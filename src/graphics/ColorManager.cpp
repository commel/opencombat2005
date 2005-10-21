#include "ColorManager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>
#include <misc\Stack.h>
#include <stdio.h>
#include <direct.h>
#include <misc\Array.h>

ColorManager::ColorManager()
{
	_nColors = 0;
}

ColorManager::~ColorManager()
{
	// XXX/GWS: Todo
}

void 
ColorManager::CopyColor(char *name, Color *dest)
{
	for(int i = 0; i < _nColors; ++i) {
		if(strcmp(name, _names[i]) == 0) {
			dest->alpha = _colors[i].alpha;
			dest->red = _colors[i].red;	
			dest->green = _colors[i].green;
			dest->blue = _colors[i].blue;
			return;
		}
	}
}

#define PSF_COLOR		0x01

class ColorAttributes {
public:
	char Name[32];
	int a, r, g, b;
};

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,
	PS_ALPHA,
	PS_RED,
	PS_GREEN,
	PS_BLUE,
	PS_COLOR,
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
	{ L"Name",			PS_NAME},
	{ L"Alpha",			PS_ALPHA },
	{ L"Red",			PS_RED},
	{ L"Green",			PS_GREEN},
	{ L"Blue",			PS_BLUE},
	{ L"Color",			PS_COLOR},

	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class ColorContentHandler : public SAXContentHandlerImpl  
{
public:
	ColorContentHandler(Array<ColorAttributes> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~ColorContentHandler() {}
        
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
				case PS_COLOR:
					_currentAttr = new ColorAttributes();
					_parserFlags ^= PSF_COLOR;
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
				case PS_COLOR:
					if(_attrs != NULL) {
						_attrs->Add(_currentAttr);
						_currentAttr = NULL;
					}
					_parserFlags ^= PSF_COLOR;
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
				strcpy(_currentAttr->Name, fileName);
			}
			break;

		case PS_ALPHA:
			_currentAttr->a = ToInt(pwchChars, cchChars);
			break;

		case PS_RED:
			_currentAttr->r = ToInt(pwchChars, cchChars);
			break;

		case PS_GREEN:
			_currentAttr->g = ToInt(pwchChars, cchChars);
			break;

		case PS_BLUE:
			_currentAttr->b = ToInt(pwchChars, cchChars);
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
	  ColorAttributes *_currentAttr;
	  Array<ColorAttributes> *_attrs;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

void
ColorManager::Load(char *configFile)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	Array<ColorAttributes> dest;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, __uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		ColorContentHandler *pMc = new ColorContentHandler(&dest);
		hr = pRdr->putContentHandler(pMc);

	    static wchar_t URL[1000];
		mbstowcs( URL, configFile, 999 );
		wprintf(L"\nParsing document: %s\n", URL);
      
	    hr = pRdr->parseURL((unsigned short *)URL);
		printf("\nParse result code: %08x\n\n",hr);
   
		pRdr->Release();
   }
   else 
   {
      printf("\nError %08X\n\n", hr);
   }

   // Great, now I have parsed all of the entries, let's create all of the widgets
   for(int i = 0; i < dest.Count; ++i) {
	    ColorAttributes *attr = dest.Items[i];
	    _colors[_nColors].alpha = (unsigned char)attr->a;
		_colors[_nColors].red = (unsigned char)attr->r;
		_colors[_nColors].green = (unsigned char)attr->g;
		_colors[_nColors].blue = (unsigned char)attr->b;
		strcpy(_names[_nColors++], attr->Name);
	    delete attr;
   }
}

