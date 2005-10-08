#include ".\mapmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <stdio.h>
#include <math.h>
#include <misc\Stack.h>
#include <application\Globals.h>

#define PSF_MAP 0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_MAP,
	PS_NAME,	
	PS_BACKGROUND,
	PS_MINI,
	PS_OVERLAND,
	PS_ELEMENTS
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

MapAttributes _mapAttributes;

static AttrState _states[] = {
	{ L"Name",			PS_NAME },
	{ L"Map",			PS_MAP},
	{ L"Overland",		PS_OVERLAND},
	{ L"Background",	PS_BACKGROUND},
	{ L"Elements",		PS_ELEMENTS},
	{ L"Mini",			PS_MINI},
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
				_stack.Push(new State(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_MAP:
					_parserFlags ^= PSF_MAP;
					_currentAttr = &_mapAttributes;
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
				case PS_MAP:
					_parserFlags ^= PSF_MAP;
					_currentAttr = NULL;
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
				strcpy(_currentAttr->Name, fileName);
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
		}

		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE startDocument()
	{
		return S_OK;
	}

private:
      int idnt;
  	  MapAttributes *_currentAttr;
	  Stack<State> _stack;
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

	if(!FAILED(hr)) 
	{
		MapContentHandler *pMc = new MapContentHandler();
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

   return &_mapAttributes;
}