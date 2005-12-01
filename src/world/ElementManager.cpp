#include ".\elementmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <direct.h>
#include <stdio.h>
#include <math.h>
#include <misc\Stack.h>
#include <misc\TGA.h>
#include <world\Element.h>

#define PSF_ELEMENTS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,
	PS_ELEMENTS,
	PS_ELEMENT,
	PS_HEIGHT,
	PS_BLOCK_HEIGHT,
	PS_INDEX,
	PS_PASSABLE,
	PS_COVER_PRONE,
	PS_COVER_LOW,
	PS_COVER_MEDIUM,
	PS_COVER_HIGH,
	PS_PROTECTION_PRONE,
	PS_PROTECTION_LOW,
	PS_PROTECTION_MEDIUM,
	PS_PROTECTION_HIGH,
	PS_PROTECTION_TOP,
	PS_HINDRANCE_PRONE,
	PS_HINDRANCE_LOW,
	PS_HINDRANCE_MEDIUM,
	PS_HINDRANCE_HIGH,
	PS_SOLDIER_MOVE_PRONE,
	PS_SOLDIER_MOVE_CROUCH,
	PS_SOLDIER_MOVE_STANDING
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
	{ L"Elements",		PS_ELEMENTS},
	{ L"Element",		PS_ELEMENT},
	{ L"Height",		PS_HEIGHT},
	{ L"Index",			PS_INDEX},
	{ L"Block_Height",	PS_BLOCK_HEIGHT},
	{ L"Passable",		PS_PASSABLE},
	{ L"Cover_Prone",	PS_COVER_PRONE},
	{ L"Cover_Low",		PS_COVER_LOW},
	{ L"Cover_Medium",	PS_COVER_MEDIUM},
	{ L"Cover_High",	PS_COVER_HIGH},
	{ L"Protection_Prone",	PS_PROTECTION_PRONE},
	{ L"Protection_Low",	PS_PROTECTION_LOW},
	{ L"Protection_Medium",	PS_PROTECTION_MEDIUM},
	{ L"Protection_High",	PS_PROTECTION_HIGH},
	{ L"Protection_Top",	PS_PROTECTION_TOP},
	{ L"Hindrance_Prone",	PS_HINDRANCE_PRONE},
	{ L"Hindrance_Low",		PS_HINDRANCE_LOW},
	{ L"Hindrance_Mediun",	PS_HINDRANCE_MEDIUM},
	{ L"Hindrance_High",	PS_HINDRANCE_HIGH},
	{ L"Soldier_Move_Prone",PS_SOLDIER_MOVE_PRONE},
	{ L"Soldier_Move_Crouch",	PS_SOLDIER_MOVE_CROUCH},
	{ L"Soldier_Move_Standing",	PS_SOLDIER_MOVE_STANDING},
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class ElementsContentHandler : public SAXContentHandlerImpl  
{
public:
	ElementsContentHandler(Array<Element> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~ElementsContentHandler() {}
        
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
				case PS_ELEMENTS:
					_parserFlags ^= PSF_ELEMENTS;
					break;
				case PS_ELEMENT:
					_currentElementAttr = new Element();
					_currentElementAttr->Index = -1;
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
				case PS_ELEMENTS:
					_parserFlags ^= PSF_ELEMENTS;
					break;

				case PS_ELEMENT:
					_attrs->Add(_currentElementAttr);
					_currentElementAttr = NULL;
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
		int level = 0;
		switch(s->parserState)
		{

		case PS_NAME:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 32);
				strcpy(_currentElementAttr->Name, fileName);
			}
			break;

		case PS_BLOCK_HEIGHT:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 32);
				if(stricmp(fileName, "false") == 0) {
					_currentElementAttr->BlocksHeight = false;
				} else {
					_currentElementAttr->BlocksHeight = true;
				}
			}
			break;

		case PS_HEIGHT:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentElementAttr->Height = atoi(fileName);
			}
			break;

		case PS_INDEX:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentElementAttr->Index = atoi(fileName);
			}
			break;

		case PS_PASSABLE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				assert(strlen(fileName) < 32);
				if(stricmp(fileName, "false") == 0) {
					_currentElementAttr->Passable = false;
				} else {
					_currentElementAttr->Passable = true;
				}
			}
			break;

			case PS_COVER_HIGH:
				++level;
			case PS_COVER_MEDIUM:
				++level;
			case PS_COVER_LOW:
				++level;
			case PS_COVER_PRONE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentElementAttr->Cover[level] = (unsigned char) atoi(fileName);
			}
			break;

			case PS_HINDRANCE_HIGH:
				++level;
			case PS_HINDRANCE_MEDIUM:
				++level;
			case PS_HINDRANCE_LOW:
				++level;
			case PS_HINDRANCE_PRONE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentElementAttr->Hindrance[level] = (unsigned char) atoi(fileName);
			}
			break;

			case PS_PROTECTION_TOP:
				++level;
			case PS_PROTECTION_HIGH:
				++level;
			case PS_PROTECTION_MEDIUM:
				++level;
			case PS_PROTECTION_LOW:
				++level;
			case PS_PROTECTION_PRONE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentElementAttr->Protection[level] = (unsigned short) atoi(fileName);
			}
			break;

			case PS_SOLDIER_MOVE_STANDING:
				++level;
			case PS_SOLDIER_MOVE_CROUCH:
				++level;
			case PS_SOLDIER_MOVE_PRONE:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentElementAttr->Movement[level] = (float) atof(fileName);
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
  	  Element *_currentElementAttr;
	  Array<Element> *_attrs;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

ElementManager::ElementManager(void)
{
}

ElementManager::~ElementManager(void)
{
}

void
ElementManager::LoadElements(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	
	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		ElementsContentHandler *pMc = new ElementsContentHandler(&_elements);
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
   for(int i = 0; i < _elements.Count; ++i) {
		_elements.Items[i]->Index = i;
   }
}

Element *
ElementManager::GetElement(int index)
{
	assert(_elements.Items[index]->Index == index);
	return _elements.Items[index];
}
