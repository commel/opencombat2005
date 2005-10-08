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

struct ElementAttributes 
{
	char Name[32];
	int Index;
	int Height;
	bool BlocksHeight;
};

#define PSF_ELEMENTS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,
	PS_ELEMENTS,
	PS_ELEMENT,
	PS_HEIGHT,
	PS_BLOCK_HEIGHT,
	PS_INDEX
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
	{ L"Elements",		PS_ELEMENTS},
	{ L"Element",		PS_ELEMENT},
	{ L"Height",		PS_HEIGHT},
	{ L"Index",			PS_INDEX},
	{ L"Block_Height",	PS_BLOCK_HEIGHT},
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class ElementsContentHandler : public SAXContentHandlerImpl  
{
public:
	ElementsContentHandler(Array<ElementAttributes> *attrs) : SAXContentHandlerImpl()
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
				_stack.Push(new State(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_ELEMENTS:
					_parserFlags ^= PSF_ELEMENTS;
					break;
				case PS_ELEMENT:
					_currentElementAttr = new ElementAttributes();
					_currentElementAttr->Index = -1;
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
		State *s = (State *) _stack.Peek();
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
		}

		return S_OK;
	}

    virtual HRESULT STDMETHODCALLTYPE startDocument()
	{
		return S_OK;
	}

private:
      int idnt;
  	  ElementAttributes *_currentElementAttr;
	  Array<ElementAttributes> *_attrs;
	  Stack<State> _stack;
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
	// Create a destination for the parsed output
	Array<ElementAttributes> dest;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		ElementsContentHandler *pMc = new ElementsContentHandler(&dest);
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
   for(int i = 0; i < dest.Count; ++i) {
		// Create the source TGA file
		Element *e= new Element();
		e->_index = i;
		e->_height = dest.Items[i]->Height;
		e->_blocksHeight = dest.Items[i]->BlocksHeight;
		strcpy(e->_name, dest.Items[i]->Name);
		_elements.Add(e);
   }
}

Element *
ElementManager::GetElement(int index)
{
	assert(_elements.Items[index]->GetIndex() == index);
	return _elements.Items[index];
}
