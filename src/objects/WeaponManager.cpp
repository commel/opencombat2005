#include ".\weaponmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <Misc\SAXContentHandlerImpl.h>

#include <assert.h>
#include <direct.h>
#include <stdio.h>
#include <math.h>
#include <Misc\Stack.h>
#include <Objects\Weapon.h>

struct WeaponTemplate 
{
	WeaponTemplate() { ShakeGround = false; }
	char Name[32];
	char Icon[32];
	char Sound[64];
	char Animation[64];
	int NumRounds;
	int ReloadTimeClip;
	int ReloadTimeChamber;
	int RoundsPerBurst;
	int TimeToFire;
	bool ShakeGround;
};

#define PSF_WEAPONS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,	
	PS_WEAPONS,
	PS_WEAPON,
	PS_ICON,
	PS_RELOAD_TIME_CLIP,
	PS_RELOAD_TIME_CHAMBER,
	PS_NUM_ROUNDS,
	PS_ROUNDS_PER_BURST,
	PS_SOUND,
	PS_ANIMATION,
	PS_TIME_TO_FIRE
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
	{ L"Weapons",		PS_WEAPONS},
	{ L"Weapon",		PS_WEAPON},
	{ L"Icon",			PS_ICON},
	{ L"RoundsPerClip",		PS_NUM_ROUNDS},
	{ L"ReloadTimeClip",	PS_RELOAD_TIME_CLIP},
	{ L"ReloadTimeChamber",	PS_RELOAD_TIME_CHAMBER},
	{ L"RoundsPerBurst",	PS_ROUNDS_PER_BURST},
	{ L"Sound",			PS_SOUND},
	{ L"Animation",			PS_ANIMATION},
	{ L"TimeToFire",	PS_TIME_TO_FIRE},
	
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class WeaponsContentHandler : public SAXContentHandlerImpl  
{
public:
	WeaponsContentHandler(Array<WeaponTemplate> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~WeaponsContentHandler() {}
        
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
				case PS_WEAPONS:
					_parserFlags ^= PSF_WEAPONS;
					break;
				case PS_WEAPON:
					{
						_currentWeaponAttr = new WeaponTemplate();
						int l;
						pAttributes->getLength(&l);
						for ( int i=0; i<l; i++ ) 
						{
							wchar_t * ln; int lnl;
							char fileName[256];

							pAttributes->getLocalName(i,(unsigned short**)&ln,&lnl); 
							wcstombs(fileName, ln, lnl);
							fileName[lnl] = '\0';

							if(strcmp(fileName, "earthShaker") == 0) {
								pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
								wcstombs(fileName, ln, lnl);
								fileName[lnl] = '\0';
								if(strcmp(fileName, "true") == 0) {
									_currentWeaponAttr->ShakeGround = true;
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
				case PS_WEAPONS:
					_parserFlags ^= PSF_WEAPONS;
					break;

				case PS_WEAPON:
					_attrs->Add(_currentWeaponAttr);
					_currentWeaponAttr = NULL;
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
				assert(strlen(fileName) < 32);
				strcpy(_currentWeaponAttr->Name, fileName);
			}
			break;

		case PS_ICON:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 32);
				sprintf(_currentWeaponAttr->Icon, "%s", fileName);
			}
			break;

		case PS_SOUND:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 64);
				sprintf(_currentWeaponAttr->Sound, "%s", fileName);
			}
			break;

		case PS_ANIMATION:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 64);
				sprintf(_currentWeaponAttr->Animation, "%s", fileName);
			}
			break;

		case PS_TIME_TO_FIRE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentWeaponAttr->TimeToFire = atoi(fileName);
			}
			break;

		case PS_ROUNDS_PER_BURST:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentWeaponAttr->RoundsPerBurst = atoi(fileName);
			}
			break;

		case PS_NUM_ROUNDS:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentWeaponAttr->NumRounds = atoi(fileName);
			}
			break;

		case PS_RELOAD_TIME_CLIP:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentWeaponAttr->ReloadTimeClip = atoi(fileName);
			}
			break;

		case PS_RELOAD_TIME_CHAMBER:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentWeaponAttr->ReloadTimeChamber = atoi(fileName);
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
  	  WeaponTemplate *_currentWeaponAttr;
	  Array<WeaponTemplate> *_attrs;
	  Stack<State> _stack;
	  unsigned int _parserFlags;
};

WeaponManager::WeaponManager(void)
{
}

WeaponManager::~WeaponManager(void)
{
}

void
WeaponManager::LoadWeapons(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		WeaponsContentHandler *pMc = new WeaponsContentHandler(&_weapons);
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

Weapon *
WeaponManager::GetWeapon(char *weaponName)
{
	for(int i = 0; i < _weapons.Count; ++i) {
		if(strcmp(weaponName, _weapons.Items[i]->Name) == 0) {
			Weapon *w = new Weapon();
			
			w->_numRounds = _weapons.Items[i]->NumRounds;
			w->_totalRounds = _weapons.Items[i]->NumRounds;
			strcpy(w->_name, _weapons.Items[i]->Name);
			strcpy(w->_iconName, _weapons.Items[i]->Icon);
			strcpy(w->_sound, _weapons.Items[i]->Sound);
			w->SetEffect(_weapons.Items[i]->Animation);
			w->_reloadTimeChamber = _weapons.Items[i]->ReloadTimeChamber;
			w->_reloadTimeClip = _weapons.Items[i]->ReloadTimeClip;
			w->_roundsPerBurst = _weapons.Items[i]->RoundsPerBurst;
			w->_timeToFire = _weapons.Items[i]->TimeToFire;
			w->SetGroundShaker(_weapons.Items[i]->ShakeGround);
			return w;
		}
	}
	return NULL;
}
