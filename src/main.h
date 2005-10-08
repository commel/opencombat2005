//-----------------------------------------------------------------------------
// File: Close Combat V2.h
//
// Desc: Header file Close Combat V2 sample app
//-----------------------------------------------------------------------------
#pragma once


// XXX/GWS: Class forward declaration of private classes
class GameApplication;
class Screen;
class FontManager;

#include <Misc\StatusCallback.h>

//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
// TODO: change "DirectX AppWizard Apps" to your name or the company name
#define DXAPP_KEY        TEXT("Software\\Open Combat")

// Keeps track of previous mouse states
struct MouseState
{
	int X;
	int Y;
	BOOL bLeftDown;
	BOOL bRightDown;
};



//-----------------------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: Application class. The base class (CD3DApplication) provides the 
//       generic functionality needed in all Direct3D samples. CMyD3DApplication 
//       adds functionality specific to this sample program.
//-----------------------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication, public StatusCallback
{
    BOOL                    m_bLoadingApp;          // TRUE, if the app is loading
    CD3DFont*               m_pFont;                // Font for drawing text
    CSoundManager*          m_pSoundManager;        // DirectSound manager class

protected:
    virtual HRESULT OneTimeSceneInit();
    virtual HRESULT InitDeviceObjects();
    virtual HRESULT RestoreDeviceObjects();
    virtual HRESULT InvalidateDeviceObjects();
    virtual HRESULT DeleteDeviceObjects();
    virtual HRESULT Render();
    virtual HRESULT FrameMove();
    virtual HRESULT FinalCleanup();
    virtual HRESULT ConfirmDevice( D3DCAPS9*, DWORD, D3DFORMAT );
    VOID    Pause( bool bPause );

    HRESULT RenderText();

    HRESULT InitAudio( HWND hWnd );

	// XXX/GWS: The following is code that has been added to interface with
	//			our world
	GameApplication *	_game;
	Screen *			_screen;
	LPDIRECT3DSURFACE9	_backBuffer;
	long				_millis;
	MouseState			_oldMouseState;
	MouseState			_currentMouseState;
	FontManager *		_fontManager;
	char				_statusText[256];
	HCURSOR				_cursors[CursorInterface::CursorType::NumCursorTypes];

public:
    LRESULT MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam );
    CMyD3DApplication();
    virtual ~CMyD3DApplication();

	// Overrides parent
    virtual HRESULT Create( HINSTANCE hInstance );

	// From sound interface
	virtual void *GetImplementation() { return m_pSoundManager; }

	// From cursor interface
	virtual void ShowCursor(bool bShow, CursorInterface::CursorType type);

	// From StatusCallback
	virtual void Status(char *msg);
};

