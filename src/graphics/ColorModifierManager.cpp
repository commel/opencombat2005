#include "ColorModifierManager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>
#include <misc\Stack.h>
#include <stdio.h>
#include <direct.h>
#include <misc\Array.h>

// Instantiate the global color modifier
ColorModifiers g_ColorModifiers[MAX_COLOR_MODIFIERS];
int g_NumColorModifiers = 0;

ColorModifierManager::ColorModifierManager()
{
}

ColorModifierManager::~ColorModifierManager()
{
	// XXX/GWS: Todo
}

#define PSF_COLOR_MODIFIER		0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,
	PS_RED,
	PS_GREEN,
	PS_BLUE,
	PS_COLOR_MODIFIER,
	PS_BODY,
	PS_LEGS,
	PS_HEAD,
	PS_BELT,
	PS_BOOTS,
	PS_WEAPON
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
	{ L"Red",			PS_RED},
	{ L"Green",			PS_GREEN},
	{ L"Blue",			PS_BLUE},
	{ L"ColorModifier",	PS_COLOR_MODIFIER},
	{ L"Body",			PS_BODY},
	{ L"Legs",			PS_LEGS},
	{ L"Head",			PS_HEAD},
	{ L"Belt",			PS_BELT},
	{ L"Boots",			PS_BOOTS},
	{ L"Weapon",		PS_WEAPON},

	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class ColorModifierContentHandler : public SAXContentHandlerImpl  
{
public:
	ColorModifierContentHandler() : SAXContentHandlerImpl()
	{
		idnt = 0;
		_parserFlags = 0;
	}

	virtual ~ColorModifierContentHandler() {}
        
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
				case PS_COLOR_MODIFIER:
					_currentModifierIdx = g_NumColorModifiers;
					_parserFlags ^= PSF_COLOR_MODIFIER;
					break;
				case PS_BODY:
					_currentModifier = &(g_ColorModifiers[g_NumColorModifiers].Body);
					break;
				case PS_LEGS:
					_currentModifier = &(g_ColorModifiers[g_NumColorModifiers].Legs);
					break;
				case PS_HEAD:
					_currentModifier = &(g_ColorModifiers[g_NumColorModifiers].Head);
					break;
				case PS_BELT:
					_currentModifier = &(g_ColorModifiers[g_NumColorModifiers].Belt);
					break;
				case PS_BOOTS:
					_currentModifier = &(g_ColorModifiers[g_NumColorModifiers].Boots);
					break;
				case PS_WEAPON:
					_currentModifier = &(g_ColorModifiers[g_NumColorModifiers].Weapon);
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
				case PS_COLOR_MODIFIER:
					g_NumColorModifiers++;
					_parserFlags ^= PSF_COLOR_MODIFIER;
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
				strcpy(g_ColorModifiers[g_NumColorModifiers].Name, fileName);
			}
			break;

		case PS_RED:
			_currentModifier->Red = 8*ToInt(pwchChars, cchChars);
			break;

		case PS_GREEN:
			_currentModifier->Green = 8*ToInt(pwchChars, cchChars);
			break;

		case PS_BLUE:
			_currentModifier->Blue = 8*ToInt(pwchChars, cchChars);
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
	  ColorModifier *_currentModifier;
	  int _currentModifierIdx;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

void
ColorModifierManager::Load(char *configFile)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, __uuidof(ISAXXMLReader), (void **)&pRdr);
	g_NumColorModifiers = 0;

	if(!FAILED(hr)) 
	{
		ColorModifierContentHandler *pMc = new ColorModifierContentHandler();
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
}

