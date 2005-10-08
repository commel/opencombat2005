#include ".\effectmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <direct.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>
#include <misc\Stack.h>
#include <misc\TGA.h>
#include <graphics\Effect.h>
#include <application\Globals.h>

struct EffectAttributes 
{
	EffectAttributes() { Dynamic=false;PlaceOnTurret=false; }
	char Name[MAX_NAME];
	char GraphicsFile[256][MAX_NAME];
	char Sound[MAX_NAME];
	int NumGraphicsFile;
	long FrameHold;
	bool Dynamic;
	bool PlaceOnTurret;
};

#define PSF_EFFECTS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,	
	PS_EFFECTS,
	PS_EFFECT,
	PS_GRAPHIC,
	PS_GRAPHICS,
	PS_FRAME_HOLD,
	PS_SOUND
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
	{ L"Effects",		PS_EFFECTS},
	{ L"Effect",		PS_EFFECT},
	{ L"Graphic",		PS_GRAPHIC},
	{ L"Graphics",		PS_GRAPHICS},
	{ L"FrameHold",		PS_FRAME_HOLD },
	{ L"Sound",			PS_SOUND },
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class EffectsContentHandler : public SAXContentHandlerImpl  
{
public:
	EffectsContentHandler(Array<EffectAttributes> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~EffectsContentHandler() {}
        
    virtual HRESULT STDMETHODCALLTYPE startElement( 
		unsigned short __RPC_FAR *pwchNamespaceUri, int cchNamespaceUri,
		unsigned short __RPC_FAR *pwchLocalName, int cchLocalName,
        unsigned short __RPC_FAR *pwchQName, int cchQName,
        ISAXAttributes __RPC_FAR *pAttributes)
	{
		UNREFERENCED_PARAMETER(cchNamespaceUri);
		UNREFERENCED_PARAMETER(pwchNamespaceUri);
		UNREFERENCED_PARAMETER(cchLocalName);
		UNREFERENCED_PARAMETER(pwchQName);
		UNREFERENCED_PARAMETER(cchQName);

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				_stack.Push(new State(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_EFFECTS:
					_parserFlags ^= PSF_EFFECTS;
					break;
				case PS_EFFECT:
						_currentWidgetAttr = new EffectAttributes();
						_currentWidgetAttr->NumGraphicsFile = 0;
						_currentWidgetAttr->Dynamic = false;
						_currentWidgetAttr->Sound[0] = '\0';
						int l;
						pAttributes->getLength(&l);
						for ( int i=0; i<l; i++ ) {
							wchar_t * ln; int lnl;
							char fileName[256];

							pAttributes->getLocalName(i,(unsigned short**)&ln,&lnl); 
							wcstombs(fileName, ln, lnl);
							fileName[lnl] = '\0';

							if(strcmp(fileName, "type") == 0) {
								pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
								wcstombs(fileName, ln, lnl);
								fileName[lnl] = '\0';
								if(strcmp(fileName, "dynamic") == 0) {
									_currentWidgetAttr->Dynamic = true;
								}
							} else if(strcmp(fileName, "place") == 0) {
								pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
								wcstombs(fileName, ln, lnl);
								fileName[lnl] = '\0';
								if(strcmp(fileName, "turret") == 0) {
									_currentWidgetAttr->PlaceOnTurret = true;
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
		UNREFERENCED_PARAMETER(pwchQName);
		UNREFERENCED_PARAMETER(cchQName);

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				switch(_states[idx].State) 
				{
				case PS_EFFECTS:
					_parserFlags ^= PSF_EFFECTS;
					break;

				case PS_EFFECT:
					_attrs->Add(_currentWidgetAttr);
					_currentWidgetAttr = NULL;
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
				strcpy(_currentWidgetAttr->Name, fileName);
			}
			break;

		case PS_SOUND:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				strcpy(_currentWidgetAttr->Sound, fileName);
			}
			break;

		case PS_GRAPHIC:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
			    sprintf(_currentWidgetAttr->GraphicsFile[_currentWidgetAttr->NumGraphicsFile++], "%s\\Effects\\%s", g_Globals->Application.GraphicsDirectory, fileName); 
				assert(_currentWidgetAttr->NumGraphicsFile <= 256);
			}
			break;

		case PS_GRAPHICS:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				GetFiles(_currentWidgetAttr, fileName);
				assert(_currentWidgetAttr->NumGraphicsFile <= 256);
			}
			break;

		case PS_FRAME_HOLD:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentWidgetAttr->FrameHold = atoi(fileName);
			}
			break;

		}

		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE startDocument()
	{
		return S_OK;
	}

	void GetFiles(EffectAttributes *attr, char *searchStr)
	{
		WIN32_FIND_DATA FindFileData;
		HANDLE hFind;
		char searchDir[256];

		sprintf(searchDir, "%s\\Effects\\%s", g_Globals->Application.GraphicsDirectory, searchStr);
		hFind = FindFirstFile(searchDir, &FindFileData);
		if(hFind == INVALID_HANDLE_VALUE) 
		{
			assert(0);
		} else {
			// Now find the base of our string
			char *p = strchr(searchDir, '*');
			*p = '\0';
			sprintf(attr->GraphicsFile[attr->NumGraphicsFile++], "%s%s", searchDir, FindFileData.cFileName);
		
			while(FindNextFile(hFind, &FindFileData) != 0) {
				sprintf(attr->GraphicsFile[attr->NumGraphicsFile++], "%s%s", searchDir, FindFileData.cFileName);
			}
		}
		FindClose(hFind);
	}

private:
      int idnt;
  	  EffectAttributes *_currentWidgetAttr;
	  Array<EffectAttributes> *_attrs;
	  Stack<State> _stack;
	  unsigned int _parserFlags;
};

EffectManager::EffectManager(void)
{
}

EffectManager::~EffectManager(void)
{
}

void
EffectManager::LoadEffects(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	// Create a destination for the parsed output
	Array<EffectAttributes> dest;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		EffectsContentHandler *pMc = new EffectsContentHandler(&dest);
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

   // Okay, now iterate through all of the widget attributes and create
   // our widgets
   // XXX/GWS: The hardcoded directory here is bad
   char fName[256];
   
   for(int i = 0; i < dest.Count; ++i) {
		// Create the source TGA file
		Effect *e = new Effect(dest.Items[i]->Name);
		e->SetSound(dest.Items[i]->Sound);
		e->SetDynamic(dest.Items[i]->Dynamic);
		e->SetPlaceOnTurret(dest.Items[i]->PlaceOnTurret);
		
		for(int j = 0; j < dest.Items[i]->NumGraphicsFile; ++j) {
			sprintf(fName, "%s", dest.Items[i]->GraphicsFile[j]); 
			TGA *tga = TGA::Create(fName);
			tga->SetTransparentColor(0,0,0);
			_sourceImages.Add(tga);

			// Let's find the hotspot for this effect. It is encoded in the
			// filename.
			char *last = strrchr(fName, '.');
			*last = '\0';
			char *second = strrchr(fName, '.');
			*second = '\0';
			char *third = strrchr(fName, '.');
			int x = atoi(third+1);
			int y = atoi(second+1);
			tga->SetOrigin(x,y);
			e->AddFrame(tga, dest.Items[i]->FrameHold);
	    }
		_effects.Add(e);
   }
}

Effect *
EffectManager::GetEffect(char *effectName)
{
	for(int i = 0; i < _effects.Count; ++i) {
		if(strcmp(effectName, _effects.Items[i]->GetName()) == 0) {
			Effect *e = _effects.Items[i]->Clone();
			return e;
		}
	}
	return NULL;
}
