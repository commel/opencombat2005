#include ".\soldieranimationmanager.h"
#import <msxml4.dll> raw_interfaces_only 
using namespace MSXML2;
#include <misc\SAXContentHandlerImpl.h>

#include <stdio.h>
#include <math.h>
#include <direct.h>
#include <misc\Stack.h>
#include <misc\Color.h>
#include <misc\Structs.h>
#include <graphics\MaskFrame.h>
#include <application\Globals.h>

struct AnimationAttributes 
{
	char Name[MAX_NAME];
	int nDirections;
	int nFrames;
	int Time;
	char FirstDirection[MAX_NAME];
	unsigned int TransparentColor;
};

#define PSF_ANIMATIONS	0x01

enum ParserState
{
	PS_UNKNOWN,
	PS_NAME,	
	PS_ANIMATIONS,
	PS_ANIMATION,
	PS_DIRECTIONS,
	PS_FRAMES,
	PS_TIME, 
	PS_FIRST_DIR,
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
	{ L"Directions",	PS_DIRECTIONS},
	{ L"NumFrames",		PS_FRAMES},
	{ L"Time",			PS_TIME},
	{ L"FirstDirection", PS_FIRST_DIR },
	{ L"TransparentColor", PS_TRANSPARENT_COLOR},
	{ NULL,				PS_UNKNOWN } /* Must be last */
};

class SoldierAnimationsContentHandler : public SAXContentHandlerImpl  
{
public:
	SoldierAnimationsContentHandler(Array<AnimationAttributes> *attrs) : SAXContentHandlerImpl()
	{
		idnt = 0;
		_attrs = attrs;
		_parserFlags = 0;
	}

	virtual ~SoldierAnimationsContentHandler() {}
        
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

		int idx = 0;
		while(_states[idx].Name != NULL) {
			if(wcsicmp((wchar_t*)_states[idx].Name, (wchar_t*)pwchLocalName) == 0) {
				_stack.Push(new State(_states[idx].State));

				switch(_states[idx].State) 
				{
				case PS_ANIMATIONS:
					{
						_parserFlags ^= PSF_ANIMATIONS;
						int l;
						pAttributes->getLength(&l);
						for ( int i=0; i<l; i++ ) {
							wchar_t * ln; int lnl;
							char fileName[256];

							pAttributes->getLocalName(i,(unsigned short**)&ln,&lnl); 
							wcstombs(fileName, ln, lnl);
							fileName[lnl] = '\0';

							if(strcmp(fileName, "dir") == 0) {
								pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
								wcstombs(fileName, ln, lnl);
								fileName[lnl] = '\0';
								strcpy(Directory, fileName);
							} else if(strcmp(fileName, "image") == 0) {
								pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
								wcstombs(fileName, ln, lnl);
								fileName[lnl] = '\0';
								strcpy(Image, fileName);
							} else if(strcmp(fileName, "mask") == 0) {
								pAttributes->getValue(i,(unsigned short**)&ln,&lnl); 
								wcstombs(fileName, ln, lnl);
								fileName[lnl] = '\0';
								strcpy(Mask, fileName);
							}
						}
					}
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

		case PS_FIRST_DIR:
			{
				char fileName[256];
				wcstombs(fileName, (wchar_t*)pwchChars, cchChars);
				fileName[cchChars] = '\0';
				strcpy(_currentAnimationAttr->FirstDirection, fileName);
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

	char Directory[256];
	char Image[256];
	char Mask[256];

private:
      int idnt;
  	  AnimationAttributes *_currentAnimationAttr;
	  Array<AnimationAttributes> *_attrs;
	  Stack<State> _stack;
	  unsigned int _parserFlags;
};

SoldierAnimationManager::SoldierAnimationManager(void)
{
}

SoldierAnimationManager::~SoldierAnimationManager(void)
{
}

void
SoldierAnimationManager::LoadAnimations(char *fileName)
{
	// Create the reader
	ISAXXMLReader* pRdr = NULL;
	// Create a destination for the parsed output
	Array<AnimationAttributes> dest;
	char directory[256], image[256], mask[256];

	HRESULT hr = CoCreateInstance(__uuidof(SAXXMLReader), NULL, CLSCTX_ALL, 
		__uuidof(ISAXXMLReader), (void **)&pRdr);

	if(!FAILED(hr)) 
	{
		SoldierAnimationsContentHandler *pMc = new SoldierAnimationsContentHandler(&dest);
		hr = pRdr->putContentHandler(pMc);

	    static wchar_t URL[1000];
		mbstowcs( URL, fileName, 999 );
		wprintf(L"\nParsing document: %s\n", URL);
      
	    hr = pRdr->parseURL((unsigned short *)URL);
		printf("\nParse result code: %08x\n\n",hr);
   
		pRdr->Release();
		strcpy(directory, pMc->Directory);
		strcpy(image, pMc->Image);
		strcpy(mask, pMc->Mask);

		// XXX/GWS: Need to delete the content handler
	}
   else 
   {
      printf("\nError %08X\n\n", hr);
   }

	// Let's find all of the files in the directory
	Array<char> files;
	Array<char> masks;
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;
	char searchDir[256];

	sprintf(searchDir, "%s\\%s\\%s", g_Globals->Application.GraphicsDirectory, directory, image);
	hFind = FindFirstFile(searchDir, &FindFileData);
	if(hFind == INVALID_HANDLE_VALUE) 
	{
		assert(0);
	} else {
		printf ("The first file found is %s\n", FindFileData.cFileName);
		sprintf(searchDir, "%s\\%s\\%s", g_Globals->Application.GraphicsDirectory, directory, FindFileData.cFileName);
		files.Add(strdup(searchDir));
		
		// Add the masks file
		FindFileData.cFileName[0] = 'm';
		FindFileData.cFileName[1] = 's';
		FindFileData.cFileName[2] = 'k';
		sprintf(searchDir, "%s\\%s\\%s", g_Globals->Application.GraphicsDirectory, directory, FindFileData.cFileName);
		masks.Add(strdup(searchDir));
		
		while(FindNextFile(hFind, &FindFileData) != 0) {
			sprintf(searchDir, "%s\\%s\\%s", g_Globals->Application.GraphicsDirectory, directory, FindFileData.cFileName);
			files.Add(strdup(searchDir));

			// Add the masks file
			FindFileData.cFileName[0] = 'm';
			FindFileData.cFileName[1] = 's';
			FindFileData.cFileName[2] = 'k';
			sprintf(searchDir, "%s\\%s\\%s", g_Globals->Application.GraphicsDirectory, directory, FindFileData.cFileName);
			masks.Add(strdup(searchDir));
		}
	}
	FindClose(hFind);

   // Okay, now iterate through all of the animation attributes and create
   // our frames
	int numFiles = 0;
	for(int i = 0; i < dest.Count; ++i) {
		Animation *a = new Animation(dest.Items[i]->Name);

		// Find out what our first direction is
		Direction firstDir = North;

		// We need to create one frame for each file in the directory
		// Up to the number of frames we are supposed to read in
		for(int j = 0; j < dest.Items[i]->nFrames; ++j) {
			for(int k = 0; k < dest.Items[i]->nDirections; ++k) {
				// Create the source tga
				char *fName = files.Items[numFiles];
				char *mName = masks.Items[numFiles++];
				TGA *tga = TGA::Create(fName);
				TGA *mtga = TGA::Create(mName);

				// Let's find the hotspot for this effect. It is encoded in the
				// filename.
				char *last = strrchr(fName, '.');
				*last = '\0';
				char *second = strrchr(fName, '.');
				*second = '\0';
				char *third = strrchr(fName, '.');
				int x = atoi(third+1);
				int y = atoi(second+1);
				tga->SetOrigin(x,y);
				mtga->SetOrigin(x,y);

				// Add to our sources
				_sourceImages.Add(tga);
				_sourceImages.Add(mtga);

				// Parse the transparent color
				Color c;
				c.Parse(dest.Items[i]->TransparentColor);

				MaskFrame *frame = new MaskFrame(tga, mtga, dest.Items[i]->Time, tga->GetWidth(), tga->GetHeight(), 0, 0, &c);
				a->AddFrame(frame, (Direction) (((int)firstDir+k) % NumDirections));
			
				delete fName;
			}
		}
		_animations.Add(a);
	}
}

#if 0
Animation *
SoldierAnimationManager::GetAnimation(char *animationName)
{
	for(int i = 0; i < _animations.Count; ++i) {
		if(strcmp(animationName, _animations.Items[i]->GetName()) == 0) {
			Animation *a = _animations.Items[i]->Clone();
			return a;
		}
	}
	return NULL;
}
#endif