#include ".\soundmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <direct.h>
#include <stdio.h>
#include <math.h>
#include <directx\DSUtil.h>
#include <misc\Stack.h>
#include <sound\Sound.h>
#include <application\Globals.h>

struct SoundAttributes 
{
	char Name[MAX_NAME];
	char SoundFile[MAX_NAME];
};

#define PSF_WIDGETS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,	
	PS_WIDGETS,
	PS_WIDGET,
	PS_GRAPHIC
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
	{ L"Sounds",		PS_WIDGETS},
	{ L"Sound",			PS_WIDGET},
	{ L"File",			PS_GRAPHIC},
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class SoundsContentHandler : public SAXContentHandlerImpl  
{
public:
	SoundsContentHandler(Array<SoundAttributes> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~SoundsContentHandler() {}
        
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
				case PS_WIDGETS:
					_parserFlags ^= PSF_WIDGETS;
					break;
				case PS_WIDGET:
					_currentSoundAttr = new SoundAttributes();
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
				case PS_WIDGETS:
					_parserFlags ^= PSF_WIDGETS;
					break;

				case PS_WIDGET:
					_attrs->Add(_currentSoundAttr);
					_currentSoundAttr = NULL;
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
				strcpy(_currentSoundAttr->Name, fileName);
			}
			break;

		case PS_GRAPHIC:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentSoundAttr->SoundFile, "%s", fileName);
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
  	  SoundAttributes *_currentSoundAttr;
	  Array<SoundAttributes> *_attrs;
	  Stack<StackState> _stack;
	  unsigned int _parserFlags;
};

SoundManager::SoundManager(void *soundManager)
{
	_soundManager = (CSoundManager *)soundManager;
}

SoundManager::~SoundManager(void)
{
}

void
SoundManager::LoadSounds(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	// Create a destination for the parsed output
	Array<SoundAttributes> dest;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		SoundsContentHandler *pMc = new SoundsContentHandler(&dest);
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
		// Create the source sound file
	   sprintf(fName, "%s\\%s", g_Globals->Application.SoundsDirectory, dest.Items[i]->SoundFile); 
	   Sound *s = new Sound(dest.Items[i]->Name, fName);
	   _soundManager->Create(&s->_sound, s->_soundFileName, 0, GUID_NULL, 5 );
	   _sounds.Add(s);
   }
}

Sound *
SoundManager::GetSound(char *widgetName)
{
	for(int i = 0; i < _sounds.Count; ++i) {
		if(strcmp(widgetName, _sounds.Items[i]->GetName()) == 0) {
			return _sounds.Items[i];
		}
	}
	return NULL;
}
