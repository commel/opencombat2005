#include ".\animationmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <stdio.h>
#include <math.h>
#include <misc\Stack.h>
#include <misc\Color.h>

struct AnimationAttributes 
{
	char Name[MAX_NAME];
	char GraphicsFile[MAX_NAME];
	int nDirections;
	int nFrames;
	int Width;
	int Height;
	int Time;
	unsigned int TransparentColor;
};

#define PSF_ANIMATIONS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,	
	PS_ANIMATIONS,
	PS_ANIMATION,
	PS_GRAPHIC,
	PS_DIRECTIONS,
	PS_FRAMES,
	PS_WIDTH,
	PS_HEIGHT,
	PS_TIME, 
	PS_TRANSPARENT_COLOR
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
	{ L"Animations",	PS_ANIMATIONS},
	{ L"Animation",		PS_ANIMATION},
	{ L"Graphic",		PS_GRAPHIC},
	{ L"Directions",	PS_DIRECTIONS},
	{ L"Frames",		PS_FRAMES},
	{ L"Width",			PS_WIDTH },
	{ L"Height",		PS_HEIGHT},
	{ L"Time",			PS_TIME},
	{ L"TransparentColor", PS_TRANSPARENT_COLOR},
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class AnimationsContentHandler : public SAXContentHandlerImpl  
{
public:
	AnimationsContentHandler(Array<AnimationAttributes> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~AnimationsContentHandler() {}
        
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
				case PS_ANIMATIONS:
					_parserFlags ^= PSF_ANIMATIONS;
					break;
				case PS_ANIMATION:
					_currentAnimationAttr = new AnimationAttributes();
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
				case PS_ANIMATIONS:
					_parserFlags ^= PSF_ANIMATIONS;
					break;

				case PS_ANIMATION:
					_attrs->Add(_currentAnimationAttr);
					_currentAnimationAttr = NULL;
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
				strcpy(_currentAnimationAttr->Name, fileName);
			}
			break;

		case PS_GRAPHIC:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				sprintf(_currentAnimationAttr->GraphicsFile, "%s", fileName);
			}
			break;

		case PS_DIRECTIONS:
			_currentAnimationAttr->nDirections = ToInt(pwchChars, cchChars);
			break;

		case PS_FRAMES:
			_currentAnimationAttr->nFrames = ToInt(pwchChars, cchChars);
			break;

		case PS_TRANSPARENT_COLOR:
			_currentAnimationAttr->TransparentColor = (unsigned int) ToInt(pwchChars, cchChars);
			break;

		case PS_WIDTH:
			_currentAnimationAttr->Width = ToInt(pwchChars, cchChars);
			break;

		case PS_HEIGHT:
			_currentAnimationAttr->Height = ToInt(pwchChars, cchChars);
			break;

		case PS_TIME:
			_currentAnimationAttr->Time = ToInt(pwchChars, cchChars);
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
  	  AnimationAttributes *_currentAnimationAttr;
	  Array<AnimationAttributes> *_attrs;
	  Stack<State> _stack;
	  unsigned int _parserFlags;
};

AnimationManager::AnimationManager(void)
{
}

AnimationManager::~AnimationManager(void)
{
}

void
AnimationManager::LoadAnimations(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	// Create a destination for the parsed output
	Array<AnimationAttributes> dest;

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		AnimationsContentHandler *pMc = new AnimationsContentHandler(&dest);
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

   // Okay, now iterate through all of the animation attributes and create
   // our frames
   for(int i = 0; i < dest.Count; ++i) {
		Animation *a = new Animation(dest.Items[i]->Name);

		// Create the source TGA file
		TGA *tga = TGA::Create(dest.Items[i]->GraphicsFile);
		_sourceImages.Add(tga);

		// Parse the transparent color
		Color c;
		c.Parse(dest.Items[i]->TransparentColor);

		// Create all of the frames
		for(int dir = 0; dir < dest.Items[i]->nDirections; ++dir) {
			for(int n = 0; n < dest.Items[i]->nFrames; ++n) {
				Frame *frame = new Frame(tga, dest.Items[i]->Time, dest.Items[i]->Width, dest.Items[i]->Height, 
					n*dest.Items[i]->Width, dir*dest.Items[i]->Height, &c);
				a->AddFrame(frame, (Direction) dir);
			}
		}
		_animations.Add(a);
   }
}

Animation *
AnimationManager::GetAnimation(char *animationName)
{
	for(int i = 0; i < _animations.Count; ++i) {
		if(strcmp(animationName, _animations.Items[i]->GetName()) == 0) {
			Animation *a = _animations.Items[i]->Clone();
			return a;
		}
	}
	return NULL;
}
