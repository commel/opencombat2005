#include ".\buildingmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <direct.h>
#include <stdio.h>
#include <math.h>
#include <misc\Structs.h>
#include <misc\Stack.h>
#include <misc\TGA.h>
#include <world\Building.h>
#include <Application\Globals.h>

#define PSF_BUILDINGS	0x01
#define PSF_BUILDING	0x02
#define PSF_BOUNDARY	0x04
#define PSF_POSITION	0x08

enum ParserState
{
	PS_UNKNOWN,
	PS_BUILDINGS,
	PS_BUILDING,
	PS_BOUNDARY,
	PS_POINT,
	PS_POSITION,
	PS_X,
	PS_Y,
	PS_EXTERIOR_GRAPHIC,
	PS_INTERIOR_GRAPHIC,
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
	{ L"Buildings",		PS_BUILDINGS},
	{ L"Building",		PS_BUILDING},
	{ L"Boundary",		PS_BOUNDARY},
	{ L"Point",			PS_POINT},
	{ L"Position",		PS_POSITION},
	{ L"X",				PS_X},
	{ L"Y",				PS_Y},
	{ L"ExteriorGraphic", PS_EXTERIOR_GRAPHIC },
	{ L"InteriorGraphic", PS_INTERIOR_GRAPHIC },
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class BuildingsContentHandler : public SAXContentHandlerImpl  
{
public:
	BuildingsContentHandler(Array<Building> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~BuildingsContentHandler() {}
        
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
		UNREFERENCED_PARAMETER(pAttributes);

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				_stack.Push(new StackState(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_BUILDINGS:
					_parserFlags ^= PSF_BUILDINGS;
					break;
				case PS_BUILDING:
					_parserFlags ^= PSF_BUILDING;
					_currentBuilding = new Building();
					break;
				case PS_BOUNDARY:
					_parserFlags ^= PSF_BOUNDARY;
					break;
				case PS_POSITION:
					_parserFlags ^= PSF_POSITION;
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
		UNREFERENCED_PARAMETER(pwchQName);
		UNREFERENCED_PARAMETER(cchQName);

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				switch(_states[idx].State) 
				{
				case PS_BUILDINGS:
					_parserFlags ^= PSF_BUILDINGS;
					break;

				case PS_BUILDING:
					_parserFlags ^= PSF_BUILDING;
					_attrs->Add(_currentBuilding);
					_currentBuilding = NULL;
					break;
				case PS_BOUNDARY:
					_parserFlags ^= PSF_BOUNDARY;
					break;
				case PS_POSITION:
					_parserFlags ^= PSF_POSITION;
					_currentBuilding->Position.x = _x;
					_currentBuilding->Position.y = _y;
					break;
				case PS_POINT:
					if(_parserFlags & PSF_BOUNDARY)
					{
						Point *p = new Point();
						p->x = _x;
						p->y = _y;
						_currentBuilding->AddBoundaryPoint(p);
					}
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

		case PS_INTERIOR_GRAPHIC:
			{
				char fileName[256],buffer[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';

				// Create a TGA from this file
				sprintf(buffer, "%s\\%s", g_Globals->Application.MapsDirectory, fileName);
				_currentBuilding->SetInterior(TGA::Create(buffer));
			}
			break;
		
		case PS_EXTERIOR_GRAPHIC:
			// Ignore this for now, because I am not sure we need it
			break;

		case PS_X:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_x = atoi(fileName);
			}
			break;
		case PS_Y:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_y = atoi(fileName);
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
	  int _x, _y;
  	  Building *_currentBuilding;
	  Array<Building> *_attrs;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

void
BuildingManager::LoadBuildings(char *fileName, Array<Building> *buildings)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		BuildingsContentHandler *pMc = new BuildingsContentHandler(buildings);
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
