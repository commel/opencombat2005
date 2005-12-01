#include ".\soldiermanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#include <misc\Stack.h>
#include <misc\Color.h>
#include <graphics\AnimationManager.h>
#include <objects\WeaponManager.h>
#include <misc\readline.h>

struct _SoldierState {
	char Name[MAX_NAME];
	char Animation[MAX_NAME];
};

class SoldierTemplate {
public:
	SoldierTemplate() {
		CanFire = false;
		CanMove = false;
		CanMoveFast = false;
		CanSneak = false;
		CanDefend = false;
		CanAmbush = false;
		CanSmoke = false;
	}

	~SoldierTemplate() {}

	float WalkingSpeed;
	float WalkingAcceleration;
	float RunningSpeed;
	float RunningAcceleration;
	float CrawlingSpeed;
	float SneakingSpeed;
	float SneakingAcceleration;

	bool CanFire;
	bool CanMove;
	bool CanMoveFast;
	bool CanSneak;
	bool CanDefend;
	bool CanAmbush;
	bool CanSmoke;

	char Name[MAX_NAME];

	char PrimaryWeapon[MAX_NAME];
	int PrimaryWeaponNumClips;

	Array<_SoldierState> States;
};

#define PSF_SOLDIERS	0x01
#define PSF_SOLDIER		0x02
#define PSF_STATES		0x04
#define PSF_STATE		0x08

enum ParserState
{
	PS_UNKNOWN,
	PS_SOLDIERS,
	PS_SOLDIER,
	PS_STATES,
	PS_STATE,
	PS_NAME,
	PS_PRIMARY_WEAPON,
	PS_PRIMARY_WEAPON_NUM_CLIPS,

	PS_ANIMATION,

	PS_WALKING,
	PS_RUNNING,
	PS_CRAWLING,
	PS_SNEAKING,

	PS_AWALKING,
	PS_ARUNNING,
	PS_ACRAWLING,
	PS_ASNEAKING,

	PS_CAN_MOVE,
	PS_CAN_MOVE_FAST,
	PS_CAN_FIRE,
	PS_CAN_DEFEND,
	PS_CAN_AMBUSH,
	PS_CAN_SMOKE,
	PS_CAN_SNEAK,

	PS_ATTRIBUTES,
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
	{ L"Soldiers",		PS_SOLDIERS},
	{ L"Soldier",		PS_SOLDIER},
	{ L"States",		PS_STATES },
	{ L"State",			PS_STATE },
	{ L"Name",			PS_NAME },
	{ L"PrimaryWeapon",		PS_PRIMARY_WEAPON },
	{ L"PrimaryWeaponNumClips",	PS_PRIMARY_WEAPON_NUM_CLIPS},


	{ L"Animation",		PS_ANIMATION},

	{ L"WalkingSpeed",	PS_WALKING},
	{ L"RunningSpeed",	PS_RUNNING},
	{ L"CrawlingSpeed",	PS_CRAWLING},
	{ L"SneakingSpeed",	PS_SNEAKING},

	{ L"WalkingAcceleration",	PS_AWALKING},
	{ L"RunningAcceleration",	PS_ARUNNING},
	{ L"CrawlingAcceleration",	PS_ACRAWLING},
	{ L"SneakingAcceleration",	PS_ASNEAKING},

	{ L"CanMove",		PS_CAN_MOVE},
	{ L"CanMoveFast",	PS_CAN_MOVE_FAST},
	{ L"CanFire",		PS_CAN_FIRE},
	{ L"CanDefend",		PS_CAN_DEFEND},
	{ L"CanAmbush",		PS_CAN_AMBUSH},
	{ L"CanSmoke",		PS_CAN_SMOKE},
	{ L"CanSneak",		PS_CAN_SNEAK},

	{ L"Attributes",	PS_ATTRIBUTES},
	
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class SoldiersContentHandler : public SAXContentHandlerImpl  
{
public:
	SoldiersContentHandler(Array<SoldierTemplate> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~SoldiersContentHandler() {}
        
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
		UNREFERENCED_PARAMETER(pAttributes);

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				_stack.Push(new StackState(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_SOLDIERS:
					_parserFlags ^= PSF_SOLDIERS;
					break;
				case PS_SOLDIER:
					_parserFlags ^= PSF_SOLDIER;
					_currentSoldier = new SoldierTemplate();
					break;
				case PS_STATES:
					_parserFlags ^= PSF_STATES;
					break;
				case PS_STATE:
					_parserFlags ^= PSF_STATE;
					_currentSoldierState = new _SoldierState();
					break;
				case PS_CAN_MOVE:
					_currentSoldier->CanMove = true;
					break;
				case PS_CAN_MOVE_FAST:
					_currentSoldier->CanMoveFast = true;
					break;
				case PS_CAN_FIRE:
					_currentSoldier->CanFire = true;
					break;
				case PS_CAN_SMOKE:
					_currentSoldier->CanSmoke = true;
					break;
				case PS_CAN_DEFEND:
					_currentSoldier->CanDefend = true;
					break;
				case PS_CAN_AMBUSH:
					_currentSoldier->CanAmbush = true;
					break;
				case PS_CAN_SNEAK:
					_currentSoldier->CanSneak = true;
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
				case PS_SOLDIERS:
					_parserFlags ^= PSF_SOLDIERS;
					break;

				case PS_SOLDIER:
					_parserFlags ^= PSF_SOLDIER;
					_attrs->Add(_currentSoldier);
					_currentSoldier = NULL;
					break;

				case PS_STATES:
					_parserFlags ^= PSF_STATES;
					break;

				case PS_STATE:
					_parserFlags ^= PSF_STATE;
					_currentSoldier->States.Add(_currentSoldierState);
					break;

			case PS_CAN_MOVE:
					_currentSoldier->CanMove = true;
					break;
				case PS_CAN_MOVE_FAST:
					_currentSoldier->CanMoveFast = true;
					break;
				case PS_CAN_FIRE:
					_currentSoldier->CanFire = true;
					break;
				case PS_CAN_SMOKE:
					_currentSoldier->CanSmoke = true;
					break;
				case PS_CAN_DEFEND:
					_currentSoldier->CanDefend = true;
					break;
				case PS_CAN_AMBUSH:
					_currentSoldier->CanAmbush = true;
					break;
				case PS_CAN_SNEAK:
					_currentSoldier->CanSneak = true;
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

				if(_parserFlags & PSF_STATE) {
					strcpy(_currentSoldierState->Name, fileName);
				} else {
					strcpy(_currentSoldier->Name, fileName);
				}
			}
			break;

		case PS_PRIMARY_WEAPON:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				strcpy(_currentSoldier->PrimaryWeapon, fileName);
			}
			break;

		case PS_PRIMARY_WEAPON_NUM_CLIPS:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentSoldier->PrimaryWeaponNumClips = atoi(fileName);
			}
			break;

		case PS_ANIMATION:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentSoldierState->Animation, "%s", fileName);
			}
			break;
		
		case PS_WALKING:
			_currentSoldier->WalkingSpeed = ToFloat(pwchChars, cchChars);
			break;

		case PS_RUNNING:
			_currentSoldier->RunningSpeed = ToFloat(pwchChars, cchChars);
			break;

		case PS_CRAWLING:
			_currentSoldier->CrawlingSpeed = ToFloat(pwchChars, cchChars);
			break;

		case PS_SNEAKING:
			_currentSoldier->SneakingSpeed = ToFloat(pwchChars, cchChars);
			break;
		
		case PS_AWALKING:
			_currentSoldier->WalkingAcceleration= ToFloat(pwchChars, cchChars);
			break;

		case PS_ARUNNING:
			_currentSoldier->RunningAcceleration= ToFloat(pwchChars, cchChars);
			break;

		case PS_ASNEAKING:
			_currentSoldier->SneakingAcceleration= ToFloat(pwchChars, cchChars);
			break;

		case PS_CAN_MOVE:
			_currentSoldier->CanMove = true;
			break;

		case PS_CAN_MOVE_FAST:
			_currentSoldier->CanMoveFast = true;
			break;

		case PS_CAN_FIRE:
			_currentSoldier->CanFire = true;
			break;

		case PS_CAN_SMOKE:
			_currentSoldier->CanSmoke = true;
			break;

		case PS_CAN_DEFEND:
			_currentSoldier->CanDefend = true;
			break;

		case PS_CAN_AMBUSH:
			_currentSoldier->CanAmbush = true;
			break;

		case PS_CAN_SNEAK:
			_currentSoldier->CanDefend = true;
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
	  SoldierTemplate *_currentSoldier;
	  _SoldierState *_currentSoldierState;
	  Array<SoldierTemplate> *_attrs;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

SoldierManager::SoldierManager(void)
{
}

SoldierManager::~SoldierManager(void)
{
}

void
SoldierManager::LoadSoldiers(char *fileName, char *soldierNames)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		SoldiersContentHandler *pMc = new SoldiersContentHandler(&_soldiers);
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

    // Now read in the soldier names file
	FILE *fp = fopen(soldierNames, "r");
	char buffer[MAX_NAME];
	int nread;
	srand(time(NULL));
	while((nread = readline(fp, buffer, MAX_NAME)) > 0) {
		if(buffer[0] == '#') {
			break;
		}

		buffer[nread] = '\0';
		if(buffer[nread-1] == '\r' || buffer[nread-1] == '\n') {
			buffer[nread-1] = '\0';
		}
		assert(strlen(buffer) < 32);
		_soldierNames.Add(strdup(buffer));
	}
}

Soldier *
SoldierManager::CreateSoldier(char *soldierType, AnimationManager *animationManager, WeaponManager *weaponManager)
{
	for(int i = 0; i < _soldiers.Count; ++i) {
		SoldierTemplate *t = _soldiers.Items[i];
		if(strcmp(t->Name, soldierType) == 0) {
			// Create a soldier of this type
			Soldier *s = new Soldier();
			strcpy(s->_name, soldierType);

			// Give this soldier a personal name
			strcpy(s->_personalName, _soldierNames.Items[rand()%_soldierNames.Count]);

			// Get the primary weapon
			s->_weapons[0] = weaponManager->GetWeapon(t->PrimaryWeapon);
			s->_currentWeaponIdx = 0;
			s->_numWeapons = 1;
			s->_weaponsNumClips[0] = t->PrimaryWeaponNumClips;

			// Read in the action state flags
			s->_canAmbush = t->CanAmbush;
			s->_canDefend = t->CanDefend;
			s->_canFire = t->CanFire;
			s->_canMove = t->CanMove;
			s->_canMoveFast = t->CanMoveFast;
			s->_canSmoke = t->CanSmoke;
			s->_canSneak = t->CanSneak;
			s->_currentHeading = South;

			// Now read in the speeds, accelerations, and animations for each state
			s->_animations[Soldier::AnimationState::Standing] = GetAnimation(animationManager, "Standing", t);
			s->_animations[Soldier::AnimationState::StandingFiring] = GetAnimation(animationManager, "Standing Firing", t);
			s->_animations[Soldier::AnimationState::StandingReloading] = GetAnimation(animationManager, "Standing Reloading", t);
			s->_animations[Soldier::AnimationState::Prone] = GetAnimation(animationManager, "Prone", t);
			s->_animations[Soldier::AnimationState::ProneFiring] = GetAnimation(animationManager, "Prone Firing", t);
			s->_animations[Soldier::AnimationState::ProneReloading] = GetAnimation(animationManager, "Prone Reloading", t);
			s->_animations[Soldier::AnimationState::DyingBlownUp] = GetAnimation(animationManager, "Dying Blown Up", t);
			s->_animations[Soldier::AnimationState::DyingBackward] = GetAnimation(animationManager, "Dying Backward", t);
			s->_animations[Soldier::AnimationState::DyingForward] = GetAnimation(animationManager, "Dying Forward", t);
			s->_animations[Soldier::AnimationState::Dead] = GetAnimation(animationManager, "Dead", t);
			s->_animations[Soldier::AnimationState::StandingUp] = GetAnimation(animationManager, "Standing Up", t);
			
			s->_animations[Soldier::AnimationState::LyingDown] = GetAnimation(animationManager, "Standing Up", t);
			s->_animations[Soldier::AnimationState::LyingDown]->SetReverse(true);

			s->_walkingAccel = t->WalkingAcceleration;
			s->_animations[Soldier::AnimationState::Walking] = GetAnimation(animationManager, "Walking", t);

			s->_crawlingAccel = t->SneakingAcceleration;
			s->_animations[Soldier::AnimationState::Sneaking] = GetAnimation(animationManager, "Sneaking", t);
			
			s->_runningAccel = t->RunningAcceleration;
			s->_animations[Soldier::AnimationState::Running] = GetAnimation(animationManager, "Running", t);

			return s;
		}
	}
	return NULL;
}

Animation *
SoldierManager::GetAnimation(AnimationManager *animationManager, char *name, SoldierTemplate *tplate)
{
	for(int i = 0; i < tplate->States.Count; ++i) {
		if(strcmp(name, tplate->States.Items[i]->Name) == 0) {
			return animationManager->GetAnimation(tplate->States.Items[i]->Animation);
		}
	}
	return NULL;
}
