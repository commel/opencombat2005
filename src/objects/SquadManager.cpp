#include ".\squadmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <Misc\SAXContentHandlerImpl.h>
#include <Objects\SoldierManager.h>
#include <Objects\VehicleManager.h>
#include <Objects\WeaponManager.h>
#include <Objects\Squad.h>
#include <Misc\Stack.h>
#include <stdio.h>
#include <assert.h>

#define MAX_SOLDIERS_IN_SQUAD 32
#define MAX_VEHICLES_IN_SQUAD 1

struct SquadSoldierAttributes
{
	SquadSoldierAttributes() { Slot=-1; }
	char Type[256];
	char Title[32];
	char Rank[32];
	char Camo[64];
	int Slot;
};

struct SquadVehicleAttributes
{
	SquadVehicleAttributes() { NumSoldiers=0; }
	int NumSoldiers;
	SquadSoldierAttributes Soldiers[MAX_SOLDIERS_IN_SQUAD];
	char Type[64];
};

struct SquadTemplate
{
	SquadTemplate() { NumSoldiers = NumVehicles = 0; }

	// The number of soldiers in this squad
	int  NumSoldiers;
	int  NumVehicles;

	// The types of soldiers in this squad
	SquadSoldierAttributes Soldiers[MAX_SOLDIERS_IN_SQUAD];

	// The vehicle types
	SquadVehicleAttributes Vehicles[MAX_VEHICLES_IN_SQUAD];

	// The name of this squad
	char Name[256];
	// The icon name
	char IconName[32];
};

#define PSF_SQUAD		0x01
#define PSF_SOLDIER		0x02
#define PSF_VEHICLE		0x04

enum ParserState
{
	PS_UNKNOWN,
	PS_SQUAD,
	PS_NAME,
	PS_SOLDIER,
	PS_VEHICLE,
	PS_TITLE,
	PS_RANK,
	PS_TYPE,
	PS_CAMO,
	PS_ICON
};

struct AttrState
{
	wchar_t *Name;
	ParserState State;
};

class StackState
{
public:
	StackState(ParserState s) { parserState = s; }
	virtual ~StackState() {}
	ParserState parserState;
};

static AttrState _states[] = {
	{ L"Soldier",		PS_SOLDIER},
	{ L"Name",			PS_NAME },
	{ L"Squad",			PS_SQUAD },
	{ L"Icon",			PS_ICON },
	{ L"Title",			PS_TITLE},
	{ L"Rank",			PS_RANK},
	{ L"Type",			PS_TYPE},
	{ L"Camo",			PS_CAMO},
	{ L"Vehicle",		PS_VEHICLE},

	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class SquadContentHandler : public SAXContentHandlerImpl  
{
public:
	SquadContentHandler(Array<SquadTemplate> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~SquadContentHandler() {}
        
    virtual HRESULT STDMETHODCALLTYPE startElement( 
		unsigned short __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri,
		unsigned short __RPC_FAR *pwchLocalName, int cchLocalName,
        unsigned short __RPC_FAR *pwchQName, int cchQName,
        ISAXAttributes __RPC_FAR *pAttributes)
	{
		UNREFERENCED_PARAMETER(cchNamespaceUri);
		UNREFERENCED_PARAMETER(pwchNamespaceUri);
		UNREFERENCED_PARAMETER(cchLocalName);
		UNREFERENCED_PARAMETER(cchQName);
		UNREFERENCED_PARAMETER(pwchQName);

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				_stack.Push(new StackState(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_SQUAD:
					_currentAttr = new SquadTemplate();
					_parserFlags ^= PSF_SQUAD;
					break;

				case PS_SOLDIER:
					_parserFlags ^= PSF_SOLDIER;

					// Need to pick out the 'slot' attribute
					int l;
					pAttributes->getLength(&l);
					for ( int i=0; i<l; i++ ) {
						wchar_t * ln; int lnl;
						char fileName[256];

						pAttributes->getLocalName(i,(unsigned short**)&ln,&lnl); 
						wcstombs(fileName, ln, lnl);
						fileName[lnl] = '\0';

						if(strcmp(fileName, "slot") == 0) {
							pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
							wcstombs(fileName, ln, lnl);
							fileName[lnl] = '\0';
							if(_parserFlags & PSF_VEHICLE) {
								SquadVehicleAttributes *v = &(_currentAttr->Vehicles[_currentAttr->NumVehicles]);
								v->Soldiers[v->NumSoldiers].Slot = atoi(fileName);
							}
						}
					}
					break;

				case PS_VEHICLE:
					_parserFlags ^= PSF_VEHICLE;
					break;
				}

				return S_OK;
			}
			++idx;
		}
		_stack.Push(new StackState(PS_UNKNOWN));
		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE endElement(
		unsigned short __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri, 
		unsigned short __RPC_FAR *pwchLocalName, int cchLocalName, 
		unsigned short __RPC_FAR *pwchQName, int cchQName)
	{
		UNREFERENCED_PARAMETER(cchNamespaceUri);
		UNREFERENCED_PARAMETER(pwchNamespaceUri);
		UNREFERENCED_PARAMETER(cchLocalName);
		UNREFERENCED_PARAMETER(cchQName);
		UNREFERENCED_PARAMETER(pwchQName);

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				switch(_states[idx].State) 
				{
				case PS_SQUAD:
					if(_attrs != NULL) {
						_attrs->Add(_currentAttr);
						_currentAttr = NULL;
					}
					_parserFlags ^= PSF_SQUAD;
					break;

				case PS_SOLDIER:
					_parserFlags ^= PSF_SOLDIER;
					if(_parserFlags & PSF_VEHICLE) {
						_currentAttr->Vehicles[_currentAttr->NumVehicles].NumSoldiers++;
					} else {
						_currentAttr->NumSoldiers++;
					}
					break;

				case PS_VEHICLE:
					_parserFlags ^= PSF_VEHICLE;
					_currentAttr->NumVehicles++;
					break;
				}
			}
			++idx;
		}

		delete _stack.Pop();
		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE characters(unsigned short *pwchChars, int cchChars)
	{
		// Get the current parser state
		StackState *s = (StackState *) _stack.Peek();
		switch(s->parserState)
		{

		case PS_NAME:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 32);
				strcpy(_currentAttr->Name, fileName);
			}
			break;

		case PS_ICON:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) <= 32);
				strcpy(_currentAttr->IconName, fileName);
			}
			break;

		case PS_TITLE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 32);
				if(_parserFlags & PSF_VEHICLE) {
					strcpy(_currentAttr->Vehicles[_currentAttr->NumVehicles].Soldiers[_currentAttr->Vehicles[_currentAttr->NumVehicles].NumSoldiers].Title, fileName);
				} else {
					strcpy(_currentAttr->Soldiers[_currentAttr->NumSoldiers].Title, fileName);
				}
			}
			break;

		case PS_RANK:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 32);
				if(_parserFlags & PSF_VEHICLE) {
					strcpy(_currentAttr->Vehicles[_currentAttr->NumVehicles].Soldiers[_currentAttr->Vehicles[_currentAttr->NumVehicles].NumSoldiers].Rank, fileName);
				} else {
					strcpy(_currentAttr->Soldiers[_currentAttr->NumSoldiers].Rank, fileName);
				}
			}
			break;

		case PS_TYPE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 64);
				if((_parserFlags & PSF_VEHICLE) && (_parserFlags & PSF_SOLDIER)) {
					strcpy(_currentAttr->Vehicles[_currentAttr->NumVehicles].Soldiers[_currentAttr->Vehicles[_currentAttr->NumVehicles].NumSoldiers].Type, fileName);
				} else if(_parserFlags & PSF_VEHICLE) {
					strcpy(_currentAttr->Vehicles[_currentAttr->NumVehicles].Type, fileName);
				} else if(_parserFlags & PSF_SOLDIER) {
					strcpy(_currentAttr->Soldiers[_currentAttr->NumSoldiers].Type, fileName);
				}
			}
			break;
		case PS_CAMO:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 64);
				if(_parserFlags & PSF_VEHICLE) {
					strcpy(_currentAttr->Vehicles[_currentAttr->NumVehicles].Soldiers[_currentAttr->Vehicles[_currentAttr->NumVehicles].NumSoldiers].Camo, fileName);
				} else {
					strcpy(_currentAttr->Soldiers[_currentAttr->NumSoldiers].Camo, fileName);
				}
			}
			break;
		}

		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE startDocument()
	{
		return S_OK;
	}

private:
      int idnt;
	  SquadTemplate *_currentAttr;
	  Array<SquadTemplate> *_attrs;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

SquadManager::SquadManager(void)
{
}

SquadManager::~SquadManager(void)
{
}

// Loads a group of soldiers from a configuration file
void 
SquadManager::LoadSquads(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		SquadContentHandler *pMc = new SquadContentHandler(&_squads);
		hr = pRdr->putContentHandler(pMc);

	    static wchar_t URL[1000];
		mbstowcs( URL, fileName, 999 );
		wprintf(L"\nParsing document: %s\n", URL);
      
	    hr = pRdr->parseURL((unsigned short *)URL);
		printf("\nParse result code: %08x\n\n",hr);
   
		pRdr->Release();
   }
   else 
   {
      printf("\nError %08X\n\n", hr);
   }
}

// Creates an instance of a specific soldier
Squad *
SquadManager::CreateSquad(char *squadType, SoldierManager *soldierManager, VehicleManager *vehicleManager, AnimationManager *animationManager, WeaponManager *weaponManager)
{
	for(int i = 0; i < _squads.Count; ++i) {
		if(strcmp(squadType, _squads.Items[i]->Name) == 0) {
			Squad *squad = new Squad();
			strcpy(squad->_iconName, _squads.Items[i]->IconName);
			strcpy(squad->_name, squadType);
			// XXX/GWS: Fix this random team quality thing here!
			squad->_quality = (Squad::Quality) (rand() % Squad::NumQuality);

			// Add all of the soldiers
			for(int j = 0; j < _squads.Items[i]->NumSoldiers; ++j) {
				Soldier *s = soldierManager->CreateSoldier(_squads.Items[i]->Soldiers[j].Type, animationManager, weaponManager);
				s->SetTitle(_squads.Items[i]->Soldiers[j].Title);
				s->SetRank(_squads.Items[i]->Soldiers[j].Rank);
				s->SetCamouflage(_squads.Items[i]->Soldiers[j].Camo);
				// XXX/GWS: Need better determination of the squad leader
				if(_stricmp(_squads.Items[i]->Soldiers[j].Title, "Leader") == 0 
					|| _stricmp(_squads.Items[i]->Soldiers[j].Title, "Gunner") == 0)
				{
					s->SetSquadLeader(true);
				}
				squad->_soldiers.Add(s);
				s->SetSquad(squad);
			}

			// Add all of the vehicles
			for(int j = 0; j < _squads.Items[i]->NumVehicles; ++j)
			{
				Vehicle *v = vehicleManager->GetVehicle(_squads.Items[i]->Vehicles[j].Type);
				v->SetSquadLeader(true); // XXX/GWS: Better determination here

				// Now add soldiers to this vehicle
				for(int k = 0; k < _squads.Items[i]->Vehicles[j].NumSoldiers; ++k) {
					Soldier *s = soldierManager->CreateSoldier(_squads.Items[i]->Vehicles[j].Soldiers[k].Type, animationManager, weaponManager);
					s->SetTitle(_squads.Items[i]->Vehicles[j].Soldiers[k].Title);
					s->SetRank(_squads.Items[i]->Vehicles[j].Soldiers[k].Rank);
					s->SetCamouflage(_squads.Items[i]->Vehicles[j].Soldiers[k].Camo);
					// XXX/GWS: Need better determination of the squad leader
					if(_stricmp(_squads.Items[i]->Vehicles[j].Soldiers[k].Title, "Leader") == 0 
						|| _stricmp(_squads.Items[i]->Vehicles[j].Soldiers[k].Title, "Gunner") == 0)
					{
						s->SetSquadLeader(true);
					}
					s->SetSquad(squad);
					s->SetInVechicle(true);
					v->AddCrew(s, _squads.Items[i]->Vehicles[j].Soldiers[k].Slot);
				}
				squad->_vehicles.Add(v);
				v->SetSquad(squad);
			}

			return squad;
		}
	}
	return NULL;
}

