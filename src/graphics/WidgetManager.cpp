#include ".\widgetmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <stdio.h>
#include <math.h>
#include <misc\Stack.h>
#include <misc\TGA.h>
#include <graphics\Widget.h>
#include <application\Globals.h>

struct WidgetAttributes 
{
	char Name[MAX_NAME];
	char GraphicsFile[MAX_NAME];
	int Index;
};

#define PSF_WIDGETS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,	
	PS_WIDGETS,
	PS_WIDGET,
	PS_GRAPHIC,
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
	{ L"Widgets",		PS_WIDGETS},
	{ L"Widget",		PS_WIDGET},
	{ L"Graphic",		PS_GRAPHIC},
	{ L"Index",			PS_INDEX},
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class WidgetsContentHandler : public SAXContentHandlerImpl  
{
public:
	WidgetsContentHandler(Array<WidgetAttributes> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~WidgetsContentHandler() {}
        
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
				case PS_WIDGETS:
					_parserFlags ^= PSF_WIDGETS;
					break;
				case PS_WIDGET:
					_currentWidgetAttr = new WidgetAttributes();
					_currentWidgetAttr->Index = -1;
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
				case PS_WIDGETS:
					_parserFlags ^= PSF_WIDGETS;
					break;

				case PS_WIDGET:
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

		case PS_GRAPHIC:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentWidgetAttr->GraphicsFile, "%s", fileName);
			}
			break;

		case PS_INDEX:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				_currentWidgetAttr->Index = atoi(fileName);
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
  	  WidgetAttributes *_currentWidgetAttr;
	  Array<WidgetAttributes> *_attrs;
	  Stack<State> _stack;
	  unsigned int _parserFlags;
};

WidgetManager::WidgetManager(void)
{
}

WidgetManager::~WidgetManager(void)
{
}

void
WidgetManager::LoadWidgets(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	// Create a destination for the parsed output
	Array<WidgetAttributes> dest;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		WidgetsContentHandler *pMc = new WidgetsContentHandler(&dest);
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
   char fName[256];
 
   for(int i = 0; i < dest.Count; ++i) {
		// Create the source TGA file
	   sprintf(fName, "%s\\%s", g_Globals->Application.GraphicsDirectory, dest.Items[i]->GraphicsFile); 
		TGA *tga = TGA::Create(fName);
		_sourceImages.Add(tga);
		Widget *w = new Widget(dest.Items[i]->Name, tga);
		if(dest.Items[i]->Index == -1) {
			w->SetIndex(i);
		} else {
			w->SetIndex(dest.Items[i]->Index);
		}
		_widgets.Add(w);
   }
}

Widget *
WidgetManager::GetWidget(char *widgetName)
{
	for(int i = 0; i < _widgets.Count; ++i) {
		if(strcmp(widgetName, _widgets.Items[i]->GetName()) == 0) {
			Widget *w = _widgets.Items[i]->Clone();
			return w;
		}
	}
	return NULL;
}

Widget *
WidgetManager::GetWidget(int index)
{
	return GetWidget(index, true);
}

Widget *
WidgetManager::GetWidget(int index, bool clone)
{
	for(int i = 0; i < _widgets.Count; ++i) {
		if(index == _widgets.Items[i]->GetIndex()) {
			if(clone) {
				Widget *w = _widgets.Items[i]->Clone();
				return w;
			} else {
				return _widgets.Items[i];
			}
		}
	}
	return NULL;
}
