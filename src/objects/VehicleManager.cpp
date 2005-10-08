#include ".\vehiclemanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <Misc\SAXContentHandlerImpl.h>

#include <direct.h>
#include <stdio.h>
#include <math.h>
#include <Misc\Stack.h>
#include <Misc\TGA.h>
#include <Misc\Structs.h>
#include <Objects\Vehicle.h>
#include <Application\Globals.h>

#define MAX_TURRET_WEAPONS  8
#define MAX_HULL_WEAPONS	8

struct WeaponAttributes
{
	char Name[32];
	int Slot;
	int NumClips;
};

struct TurretAttributes
{
	TurretAttributes() { Tga = NULL; }
	char Graphic[MAX_NAME];
	TGA *Tga;
	WeaponAttributes Weapons[MAX_TURRET_WEAPONS];
	int NumWeapons;
	int RotationRate;
	Point Position;
	Point PrimaryMuzzlePosition;
};

struct HullAttributes
{
	HullAttributes() { Tga = NULL; }
	char Graphic[MAX_NAME];
	TGA *Tga;
	WeaponAttributes Weapons[MAX_HULL_WEAPONS];
	int NumWeapons;
	int RotationRate;
};

struct WreckAttributes
{
	WreckAttributes() { Tga = NULL; }
	char Graphic[MAX_NAME];
	TGA *Tga;
};

struct VehicleAttributes 
{
	char Name[MAX_NAME];
	int Index;
	HullAttributes Hull;
	TurretAttributes Turret;
	WreckAttributes Wreck;
	float MaxRoadSpeed;
	float Acceleration;
};

#define PSF_VEHICLES	0x01
#define PSF_VEHICLE		0x02
#define PSF_HULL		0x04
#define PSF_TURRET		0x08
#define PSF_WRECK		0x10

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,	
	PS_VEHICLES,
	PS_VEHICLE,
	PS_WEAPON,
	PS_HULL,
	PS_TURRET,
	PS_WRECK,
	PS_GRAPHIC,
	PS_POSITION_X,
	PS_POSITION_Y,
	PS_MUZZLE_X,
	PS_MUZZLE_Y,
	PS_ROTATION_RATE,
	PS_INDEX,
	PS_MAX_ROAD_SPEED,
	PS_ACCELERATION
};

struct AttrState
{
	wchar_t *Name;
	ParserState State;
};

class State
{
public:
	State(ParserState s) { parserState = s; }
	virtual ~State() {}
	ParserState parserState;
};

static AttrState _states[] = {
	{ L"Name",			PS_NAME },
	{ L"Vehicles",		PS_VEHICLES},
	{ L"Vehicle",		PS_VEHICLE},
	{ L"Hull",			PS_HULL},
	{ L"Turret",		PS_TURRET},
	{ L"Wreck",			PS_WRECK},
	{ L"Graphic",		PS_GRAPHIC},
	{ L"PositionX",		PS_POSITION_X},
	{ L"PositionY",		PS_POSITION_Y},
	{ L"PrimaryMuzzleX",PS_MUZZLE_X},
	{ L"PrimaryMuzzleY",PS_MUZZLE_Y},
	{ L"RotationRate",	PS_ROTATION_RATE},
	{ L"Weapon",		PS_WEAPON},
	{ L"Index",			PS_INDEX},
	{ L"Acceleration",	PS_ACCELERATION},
	{ L"MaxRoadSpeed",	PS_MAX_ROAD_SPEED},

	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class VehiclesContentHandler : public SAXContentHandlerImpl  
{
public:
	VehiclesContentHandler(Array<VehicleAttributes> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~VehiclesContentHandler() {}
        
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
				_stack.Push(new State(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_VEHICLES:
					_parserFlags ^= PSF_VEHICLES;
					break;
				case PS_VEHICLE:
					_currentVehicleAttr = new VehicleAttributes();
					_currentVehicleAttr->Index = -1;
					_currentVehicleAttr->Hull.NumWeapons = 0;
					_currentVehicleAttr->Turret.NumWeapons = 0;
					_parserFlags ^= PSF_VEHICLE;
					break;
				case PS_HULL:
					_parserFlags ^= PSF_HULL;
					break;
				case PS_TURRET:
					_parserFlags ^= PSF_TURRET;
					break;
				case PS_WRECK:
					_parserFlags ^= PSF_WRECK;
					break;
				case PS_WEAPON:
					// Need to pick out the weapon slot number
					{
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
								if(_parserFlags & PSF_HULL) {
									_currentVehicleAttr->Hull.Weapons[_currentVehicleAttr->Hull.NumWeapons].Slot = atoi(fileName);
								} else if(_parserFlags & PSF_TURRET) {
									_currentVehicleAttr->Turret.Weapons[_currentVehicleAttr->Turret.NumWeapons].Slot = atoi(fileName);
								}
							} else if(strcmp(fileName, "clips") == 0) {
								pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
								wcstombs(fileName, ln, lnl);
								fileName[lnl] = '\0';
								if(_parserFlags & PSF_HULL) {
									_currentVehicleAttr->Hull.Weapons[_currentVehicleAttr->Hull.NumWeapons].NumClips = atoi(fileName);
								} else if(_parserFlags & PSF_TURRET) {
									_currentVehicleAttr->Turret.Weapons[_currentVehicleAttr->Turret.NumWeapons].NumClips = atoi(fileName);
								}
							}
						}
					}
					break;
				}

				return S_OK;
			}
			++idx;
		}
		_stack.Push(new State(PS_UNKNOWN));
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
				case PS_VEHICLES:
					_parserFlags ^= PSF_VEHICLES;
					break;

				case PS_VEHICLE:
					_attrs->Add(_currentVehicleAttr);
					_currentVehicleAttr = NULL;
					_parserFlags ^= PSF_VEHICLE;
					break;

				case PS_HULL:
					_parserFlags ^= PSF_HULL;
					break;
				case PS_TURRET:
					_parserFlags ^= PSF_TURRET;
					break;
				case PS_WRECK:
					_parserFlags ^= PSF_WRECK;
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
		State *s = (State *) _stack.Peek();
		switch(s->parserState)
		{

		case PS_NAME:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				strcpy(_currentVehicleAttr->Name, fileName);
			}
			break;

		case PS_GRAPHIC:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				if(_parserFlags & PSF_HULL) {
					sprintf(_currentVehicleAttr->Hull.Graphic, "%s", fileName);
				} else if(_parserFlags & PSF_TURRET) {
					sprintf(_currentVehicleAttr->Turret.Graphic, "%s", fileName);
				} else if(_parserFlags & PSF_WRECK) {
					sprintf(_currentVehicleAttr->Wreck.Graphic, "%s", fileName);
				}
			}
			break;

		case PS_INDEX:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentVehicleAttr->Index = atoi(fileName);
			}
			break;

		case PS_ACCELERATION:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentVehicleAttr->Acceleration = (float)atof(fileName);
			}
			break;

		case PS_MAX_ROAD_SPEED:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentVehicleAttr->MaxRoadSpeed = (float) atof(fileName);
			}
			break;

		case PS_ROTATION_RATE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				if(_parserFlags & PSF_HULL) {
					_currentVehicleAttr->Hull.RotationRate = atoi(fileName);
				} else if(_parserFlags & PSF_TURRET) {
					_currentVehicleAttr->Turret.RotationRate = atoi(fileName);
				}
			}
			break;

		case PS_POSITION_X:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				if(_parserFlags & PSF_TURRET) {
					_currentVehicleAttr->Turret.Position.x = atoi(fileName);
				}
			}
			break;

		case PS_POSITION_Y:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				if(_parserFlags & PSF_TURRET) {
					_currentVehicleAttr->Turret.Position.y = atoi(fileName);
				}
			}
			break;

		case PS_MUZZLE_X:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				if(_parserFlags & PSF_TURRET) {
					_currentVehicleAttr->Turret.PrimaryMuzzlePosition.x = atoi(fileName);
				}
			}
			break;

		case PS_MUZZLE_Y:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				if(_parserFlags & PSF_TURRET) {
					_currentVehicleAttr->Turret.PrimaryMuzzlePosition.y = atoi(fileName);
				}
			}
			break;

		case PS_WEAPON:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				if(_parserFlags & PSF_TURRET) {
					assert(strlen(fileName) < 32);
					strcpy(_currentVehicleAttr->Turret.Weapons[_currentVehicleAttr->Turret.NumWeapons].Name, fileName);
					_currentVehicleAttr->Turret.NumWeapons++;
				} else if(_parserFlags & PSF_HULL) {
					assert(strlen(fileName) < 32);
					strcpy(_currentVehicleAttr->Hull.Weapons[_currentVehicleAttr->Hull.NumWeapons].Name, fileName);
					_currentVehicleAttr->Hull.NumWeapons++;
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
  	  VehicleAttributes *_currentVehicleAttr;
	  Array<VehicleAttributes> *_attrs;
	  Stack<State> _stack;
	  unsigned int _parserFlags;
};

VehicleManager::VehicleManager(void)
{
}

VehicleManager::~VehicleManager(void)
{
}

void
VehicleManager::Load(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	// Create a destination for the parsed output
	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		VehiclesContentHandler *pMc = new VehiclesContentHandler(&_vehicles);
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

Vehicle *
VehicleManager::GetVehicle(char *vehicleName)
{
	for(int i = 0; i < _vehicles.Count; ++i) {
		if(strcmp(vehicleName, _vehicles.Items[i]->Name) == 0) {
			Vehicle *v = new Vehicle();
		    // Okay, now iterate through all of the widget attributes and create
		    // our widgets
			// XXX/GWS: The hardcoded directory here is bad
			char fName[256];

			strcpy(v->_name, _vehicles.Items[i]->Name);
			v->_turretPosition.x = _vehicles.Items[i]->Turret.Position.x;
			v->_turretPosition.y = _vehicles.Items[i]->Turret.Position.y;
			v->_turretRotationRate = _vehicles.Items[i]->Turret.RotationRate;
			v->_hullRotationRate = _vehicles.Items[i]->Hull.RotationRate;
			v->_muzzlePosition.x = _vehicles.Items[i]->Turret.PrimaryMuzzlePosition.x;
			v->_muzzlePosition.y = _vehicles.Items[i]->Turret.PrimaryMuzzlePosition.y;
			v->_maxRoadSpeed = _vehicles.Items[i]->MaxRoadSpeed;
			v->_acceleration = _vehicles.Items[i]->Acceleration;

			// The hull graphics
			if(_vehicles.Items[i]->Hull.Tga == NULL) {
				sprintf(fName, "%s\\%s", g_Globals->Application.GraphicsDirectory, _vehicles.Items[i]->Hull.Graphic); 
				_vehicles.Items[i]->Hull.Tga = TGA::Create(fName);
			}
			v->_hullGraphics = _vehicles.Items[i]->Hull.Tga;
		
			// The turret graphic
			if(_vehicles.Items[i]->Turret.Tga == NULL) {
				sprintf(fName, "%s\\%s", g_Globals->Application.GraphicsDirectory, _vehicles.Items[i]->Turret.Graphic); 
				_vehicles.Items[i]->Turret.Tga = TGA::Create(fName);
			}
			v->_turretGraphics = _vehicles.Items[i]->Turret.Tga;
		
			// The wreck graphic
			if(_vehicles.Items[i]->Wreck.Tga == NULL) {
				sprintf(fName, "%s\\%s", g_Globals->Application.GraphicsDirectory, _vehicles.Items[i]->Wreck.Graphic); 
				_vehicles.Items[i]->Wreck.Tga = TGA::Create(fName);
			}
			v->_wreckGraphics = _vehicles.Items[i]->Wreck.Tga;

			// Now do all of the weapons
			for(int j = 0; j < _vehicles.Items[i]->Hull.NumWeapons; ++j)
			{
				v->AddWeapon(g_Globals->World.Weapons->GetWeapon(_vehicles.Items[i]->Hull.Weapons[j].Name),
					_vehicles.Items[i]->Hull.Weapons[j].Slot, _vehicles.Items[i]->Hull.Weapons[j].NumClips, true);
			}
			for(int j = 0; j < _vehicles.Items[i]->Turret.NumWeapons; ++j)
			{
				v->AddWeapon(g_Globals->World.Weapons->GetWeapon(_vehicles.Items[i]->Turret.Weapons[j].Name),
					_vehicles.Items[i]->Turret.Weapons[j].Slot, _vehicles.Items[i]->Turret.Weapons[j].NumClips, false);
			}

			// The last weapon is always an empty weapon!!!
			v->AddWeapon(g_Globals->World.Weapons->GetWeapon("Blank"), -1, 0, false);
			return v;
		}
	}
	return NULL;
}
