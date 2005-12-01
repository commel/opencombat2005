#include ".\mapmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <stdio.h>
#include <math.h>
#include <misc\Stack.h>
#include <application\Globals.h>

#define PSF_MAP 0x01
#define PSF_VL	0x02

enum ParserState
{
	PS_UNKNOWN,
	PS_MAP,
	PS_NAME,	
	PS_BACKGROUND,
	PS_MINI,
	PS_OVERLAND,
	PS_ELEMENTS,
	PS_BUILDINGS,
	PS_VICTORY_LOCATIONS,
	PS_VL,
	PS_X,
	PS_Y,
	PS_VALUE,
	PS_LINKS_TO,
	PS_MAP_NAME,
	PS_VL_NAME,
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
	{ L"Name",			PS_NAME },
	{ L"Map",			PS_MAP},
	{ L"Overland",		PS_OVERLAND},
	{ L"Buildings",		PS_BUILDINGS},
	{ L"Background",	PS_BACKGROUND},
	{ L"Elements",		PS_ELEMENTS},
	{ L"Mini",			PS_MINI},
	{ L"VictoryLocations",	PS_VICTORY_LOCATIONS},
	{ L"VL",			PS_VL},
	{ L"X",				PS_X},
	{ L"Y",				PS_Y},
	{ L"Value",			PS_VALUE},
	{ L"LinksTo",		PS_LINKS_TO},
	{ L"MapName",		PS_MAP_NAME},
	{ L"VLName",		PS_VL_NAME},
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class MapContentHandler : public SAXContentHandlerImpl  
{
public:
	MapContentHandler() : SAXContentHandlerImpl()
	{
		idnt = 0;
		_parserFlags = 0;
	}

	virtual ~MapContentHandler() {}
        
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
				case PS_MAP:
					_parserFlags ^= PSF_MAP;
					_currentAttr = new MapAttributes();
					break;
				case PS_VL:
					_parserFlags ^= PSF_VL;
					_currentVictoryLocation = new VictoryLocation();
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
				case PS_MAP:
					_parserFlags ^= PSF_MAP;
					break;
				case PS_VL:
					_parserFlags ^= PSF_VL;
					_currentAttr->VictoryLocations.Add(_currentVictoryLocation);
					_currentVictoryLocation = NULL;
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
				if(_parserFlags & PSF_VL)
				{
					strcpy(_currentVictoryLocation->Name, fileName);
				}
				else if(_parserFlags & PSF_MAP)
				{
					strcpy(_currentAttr->Name, fileName);
				}
			}
			break;

		case PS_BACKGROUND:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentAttr->Background, "%s\\%s", g_Globals->Application.MapsDirectory, fileName);
			}
			break;

		case PS_BUILDINGS:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentAttr->Buildings, "%s\\%s", g_Globals->Application.MapsDirectory, fileName);
			}
			break;

		case PS_MINI:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentAttr->Mini, "%s\\%s", g_Globals->Application.MapsDirectory, fileName);
			}
			break;

		case PS_ELEMENTS:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentAttr->Elements, "%s\\%s", g_Globals->Application.MapsDirectory, fileName);
			}
			break;

		case PS_OVERLAND:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentAttr->Overland, "%s\\%s", g_Globals->Application.MapsDirectory, fileName);
			}
			break;

		case PS_X:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentVictoryLocation->X = atoi(fileName);
			}
			break;
		case PS_Y:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentVictoryLocation->Y = atoi(fileName);
			}
			break;
		case PS_VALUE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentVictoryLocation->Value = atoi(fileName);
			}
			break;
		case PS_MAP_NAME:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < sizeof(_currentVictoryLocation->LinksToMapName));
				strcpy(_currentVictoryLocation->LinksToMapName, fileName);
			}
			break;
		case PS_VL_NAME:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < sizeof(_currentVictoryLocation->LinksToVictoryLocationName));
				strcpy(_currentVictoryLocation->LinksToVictoryLocationName, fileName);
			}
			break;
		}

		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE startDocument()
	{
		return S_OK;
	}

	MapAttributes *GetMapAttributes() { return _currentAttr; }

private:
      int idnt;
  	  MapAttributes *_currentAttr;
	  VictoryLocation *_currentVictoryLocation;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

MapManager::MapManager(void)
{
}

MapManager::~MapManager(void)
{
}

MapAttributes *
MapManager::Parse(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);
	MapAttributes *rv = NULL;

	if(!FAILED(hr)) 
	{
		MapContentHandler *pMc = new MapContentHandler();
		hr = pRdr->putContentHandler(pMc);

	    static wchar_t URL[1000];
		mbstowcs( URL, fileName, 999 );
		wprintf(L"\nParsing document: %s\n", URL);
      
	    hr = pRdr->parseURL((unsigned short *)URL);
		printf("\nParse result code: %08x\n\n",hr);
   
		rv = pMc->GetMapAttributes();
		pRdr->Release();
		delete pMc;
   }
   else 
   {
      printf("\nError %08X\n\n", hr);
	  rv = NULL;
   }

   return rv;
}