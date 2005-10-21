//-----------------------------------------------------------------------------
// File: Open Combat.cpp
//
// Desc: DirectX window application created by the DirectX AppWizard
//-----------------------------------------------------------------------------
#define STRICT
#include <assert.h>
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <dxerr9.h>
#include <tchar.h>
#include <direct.h>
#include <DirectX\DXUtil.h>
#include <DirectX\D3DEnumeration.h>
#include <DirectX\D3DSettings.h>
#include <DirectX\D3DApp.h>
#include <DirectX\D3DFont.h>
#include <DirectX\D3DUtil.h>
#include <DirectX\DSUtil.h>
#include "resource.h"
#include "main.h"

#include <Application\GameApplication.h>
#include <Graphics\Screen.h>
#include <Misc\Color.h>
#include <Graphics\FontManager.h>
#include <Application\Globals.h>

//-----------------------------------------------------------------------------
// Defines, and constants
//-----------------------------------------------------------------------------
// This GUID must be unique for every game, and the same for 
// every instance of this app.  // {24EA8E60-1D82-4B40-B3A7-63F53B4453DE}
// The GUID allows DirectInput to remember input settings
GUID g_guidApp = { 0x24EA8E60, 0x1D82, 0x4B40, { 0xB3, 0xA7, 0x63, 0xF5, 0x3B, 0x44, 0x53, 0xDE } };

//-----------------------------------------------------------------------------
// Global access to the app (needed for the global WndProc())
//-----------------------------------------------------------------------------
CMyD3DApplication*	g_pApp  = NULL;
HINSTANCE			g_hInst = NULL;
Globals*			g_Globals;

//-----------------------------------------------------------------------------
// Name: WinMain()
// Desc: Entry point to the program. Initializes everything, and goes into a
//       message-processing loop. Idle time is used to render the scene.
//-----------------------------------------------------------------------------
INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR, INT )
{
	Globals globals;
	g_Globals = &globals;

	// Run our SelfTests
	Screen::SelfTest();

	// Get the current directory and store it
	_getcwd(globals.Application.CurrentDirectory, 256);
	sprintf(globals.Application.ConfigDirectory, "%s\\Config", globals.Application.CurrentDirectory);
	sprintf(globals.Application.GraphicsDirectory, "%s\\Graphics", globals.Application.CurrentDirectory);
	sprintf(globals.Application.MapsDirectory, "%s\\Maps", globals.Application.CurrentDirectory);
	sprintf(globals.Application.SoundsDirectory, "%s\\Sounds", globals.Application.CurrentDirectory);

	CMyD3DApplication d3dApp;
    g_pApp  = &d3dApp;
	globals.Application.Status = (StatusCallback *)&d3dApp;
	globals.Application.Cursor = (CursorInterface *)&d3dApp;

	g_hInst = hInst;
	
	if(FAILED(CoInitialize(NULL))) {
		return 0;
	}

    InitCommonControls();
	if( FAILED( d3dApp.Create( hInst ) ) ) {
        return 0;
	}

	int rv = d3dApp.Run(); 
	
	CoUninitialize();
	return rv;
}




//-----------------------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: Application constructor.   Paired with ~CMyD3DApplication()
//       Member variables should be initialized to a known state here.  
//       The application window has not yet been created and no Direct3D device 
//       has been created, so any initialization that depends on a window or 
//       Direct3D should be deferred to a later stage. 
//-----------------------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
    m_dwCreationWidth           = 600;
    m_dwCreationHeight          = 375;
    m_strWindowTitle            = TEXT( "Open Combat" );
    m_d3dEnumeration.AppUsesDepthBuffer   = TRUE;
	m_bStartFullscreen			= false;
	m_bShowCursorWhenFullscreen	= false;
	sprintf(_statusText, "Loading... Please wait");

    // Create a D3D font using d3dfont.cpp
    m_pFont                     = new CD3DFont( _T("Arial"), 12, D3DFONT_BOLD );
    m_bLoadingApp               = TRUE;
    m_pSoundManager             = NULL;

    ZeroMemory( &_currentMouseState, sizeof(MouseState) );
    ZeroMemory( &_oldMouseState, sizeof(MouseState) );

	_game = new GameApplication();
	_screen = new Screen();
	_millis = 0;
	_fontManager = new FontManager();
	g_Globals->World.Fonts = _fontManager;
}

//-----------------------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: Application destructor.  Paired with CMyD3DApplication()
//-----------------------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
}




//-----------------------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: Paired with FinalCleanup().
//       The window has been created and the IDirect3D9 interface has been
//       created, but the device has not been created yet.  Here you can
//       perform application-related initialization and cleanup that does
//       not depend on a device.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
    // TODO: perform one time initialization

    // Drawing loading status message until app finishes loading
    SendMessage( m_hWnd, WM_PAINT, 0, 0 );

    // Initialize audio
    InitAudio( m_hWnd );

	// Initialize the game
	_game->Initialize(this);

	// Set the first module
	_game->ChooseModule(GameApplication::AvailableModules::Combat);

    m_bLoadingApp = FALSE;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: InitAudio()
// Desc: Initialize DirectX audio objects
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitAudio( HWND hWnd )
{
    HRESULT hr;

    // Create a static IDirectSound in the CSound class.  
    // Set coop level to DSSCL_PRIORITY, and set primary buffer 
    // format to stereo, 22kHz and 16-bit output.
    m_pSoundManager = new CSoundManager();

    if( FAILED( hr = m_pSoundManager->Initialize( hWnd, DSSCL_PRIORITY ) ) )
        return DXTRACE_ERR( TEXT("m_pSoundManager->Initialize"), hr );

    if( FAILED( hr = m_pSoundManager->SetPrimaryBufferFormat( 2, 22050, 16 ) ) )
        return DXTRACE_ERR( TEXT("m_pSoundManager->SetPrimaryBufferFormat"), hr );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: Called during device initialization, this code checks the display device
//       for some minimum set of capabilities
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice( D3DCAPS9* pCaps, DWORD dwBehavior,
                                          D3DFORMAT Format )
{
    UNREFERENCED_PARAMETER( Format );
    UNREFERENCED_PARAMETER( dwBehavior );
    UNREFERENCED_PARAMETER( pCaps );
    
    BOOL bCapsAcceptable;

    // TODO: Perform checks to see if these display caps are acceptable.
    bCapsAcceptable = TRUE;

    if( bCapsAcceptable )         
        return S_OK;
    else
        return E_FAIL;
}

//-----------------------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: Paired with DeleteDeviceObjects()
//       The device has been created.  Resources that are not lost on
//       Reset() can be created here -- resources in D3DPOOL_MANAGED,
//       D3DPOOL_SCRATCH, or D3DPOOL_SYSTEMMEM.  Image surfaces created via
//       CreateOffScreenPlainSurface are never lost and can be created here.  Vertex
//       shaders and pixel shaders can also be created here as they are not
//       lost on Reset().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    // TODO: create device objects
    HRESULT hr;

    // Init the font
    hr = m_pFont->InitDeviceObjects( m_pd3dDevice );
    if( FAILED( hr ) )
        return DXTRACE_ERR( "m_pFont->InitDeviceObjects", hr );
    
	_fontManager->Initialize(m_pd3dDevice);
	return S_OK;
}

//-----------------------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: Paired with InvalidateDeviceObjects()
//       The device exists, but may have just been Reset().  Resources in
//       D3DPOOL_DEFAULT and any other device state that persists during
//       rendering should be set here.  Render states, matrices, textures,
//       etc., that don't change during rendering can be set once here to
//       avoid redundant state setting during Render() or FrameMove().
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    // TODO: setup render states
    // Setup a material
    D3DMATERIAL9 mtrl;
    D3DUtil_InitMaterial( mtrl, 1.0f, 0.0f, 0.0f );
    m_pd3dDevice->SetMaterial( &mtrl );

    // Set up the textures
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
    m_pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR );
    m_pd3dDevice->SetSamplerState( 0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR );

    // Set miscellaneous render states
    m_pd3dDevice->SetRenderState( D3DRS_DITHERENABLE,   FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_SPECULARENABLE, FALSE );
    m_pd3dDevice->SetRenderState( D3DRS_ZENABLE,        TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_AMBIENT,        0x000F0F0F );

    // Set the world matrix
    D3DXMATRIX matIdentity;
    D3DXMatrixIdentity( &matIdentity );
    m_pd3dDevice->SetTransform( D3DTS_WORLD,  &matIdentity );

    // Set up our view matrix. A view matrix can be defined given an eye point,
    // a point to lookat, and a direction for which way is up. Here, we set the
    // eye five units back along the z-axis and up three units, look at the
    // origin, and define "up" to be in the y-direction.
    D3DXMATRIX matView;
    D3DXVECTOR3 vFromPt   = D3DXVECTOR3( 0.0f, 0.0f, -5.0f );
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3( 0.0f, 0.0f, 0.0f );
    D3DXVECTOR3 vUpVec    = D3DXVECTOR3( 0.0f, 1.0f, 0.0f );
    D3DXMatrixLookAtLH( &matView, &vFromPt, &vLookatPt, &vUpVec );
    m_pd3dDevice->SetTransform( D3DTS_VIEW, &matView );

    // Set the projection matrix
    D3DXMATRIX matProj;
    FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
    D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, fAspect, 1.0f, 100.0f );
    m_pd3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

    // Set up lighting states
    D3DLIGHT9 light;
    D3DUtil_InitLight( light, D3DLIGHT_DIRECTIONAL, -1.0f, -1.0f, 2.0f );
    m_pd3dDevice->SetLight( 0, &light );
    m_pd3dDevice->LightEnable( 0, TRUE );
    m_pd3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE );

    // Restore the font
    m_pFont->RestoreDeviceObjects();
	_fontManager->Restore();

	// Get the back buffer
    m_pd3dDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &_backBuffer );

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FrameMove()
// Desc: Called once per frame, the call is the entry point for animating
//       the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
    // Update the game state
	long oldMillis = _millis;
	long currentMillis = GetTickCount();
	if(oldMillis != 0 && (currentMillis-oldMillis) >= 33) {
		_millis = currentMillis;
		_game->Simulate(33/*currentMillis-oldMillis*/);
	} else if(oldMillis == 0) {
		_millis = currentMillis;
	}
	
	// Check the current mouse states!
    // Respond to input. First check the mouse states
	if(_currentMouseState.bLeftDown && !_oldMouseState.bLeftDown) {
		// Mouse was up, now is down
		_game->LeftMouseDown(m_cursorPosition.x, m_cursorPosition.y);
	} else if(!_currentMouseState.bLeftDown && _oldMouseState.bLeftDown) {
		_game->LeftMouseUp(m_cursorPosition.x, m_cursorPosition.y);
	} else if(_currentMouseState.bLeftDown && _oldMouseState.bLeftDown) {
		// Mouse is dragged
		_game->LeftMouseDrag(m_cursorPosition.x, m_cursorPosition.y);
	}
	_oldMouseState.bLeftDown = _currentMouseState.bLeftDown;
	
	if(_currentMouseState.bRightDown && !_oldMouseState.bRightDown) {
		// Mouse was up, now is down
		_game->RightMouseDown(m_cursorPosition.x, m_cursorPosition.y);
	} else if(!_currentMouseState.bRightDown && _oldMouseState.bRightDown) {
		_game->RightMouseUp(m_cursorPosition.x, m_cursorPosition.y);
	} else if(_currentMouseState.bRightDown  && _oldMouseState.bRightDown) {
		// Mouse is dragged
		_game->RightMouseDrag(m_cursorPosition.x, m_cursorPosition.y);
	}
	_oldMouseState.bRightDown = _currentMouseState.bRightDown ;

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: Render()
// Desc: Called once per frame, the call is the entry point for 3d
//       rendering. This function sets up render states, clears the
//       viewport, and renders the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	// Clear the viewport
	// XXX/GWS: Remove this later
    m_pd3dDevice->Clear( 0L, NULL, D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,
                         0x000000ff, 1.0f, 0L );

	// Set the device on the screen
	_screen->SetDevice(m_pd3dDevice);
	_screen->SetCursorPosition(m_cursorPosition.x, m_cursorPosition.y);

	// We need to lock the back buffer on the device
	D3DLOCKED_RECT lockRect;
	DXTEST(_backBuffer->LockRect(&lockRect, NULL, 0));
	
	// Set the capabilties of this screen
	D3DSURFACE_DESC desc;
	DXTEST(_backBuffer->GetDesc(&desc));
	unsigned char *dest = (unsigned char *) lockRect.pBits;
	_screen->SetCapabilities(dest, desc.Width, desc.Height, desc.Format, lockRect.Pitch);

    // Begin the scene
    if( SUCCEEDED( m_pd3dDevice->BeginScene() ) )
    {
		// Now render the world
		_game->Render(_screen);
        
        // Render stats and help text  
		RenderText();

        // End the scene.
        m_pd3dDevice->EndScene();
    }

	// Now unlock the back buffer
	DXTEST(_backBuffer->UnlockRect());

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderText()
// Desc: Renders stats and help text to the scene.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
    D3DCOLOR fontColor        = D3DCOLOR_ARGB(255,255,255,0);
    char szMsg[MAX_PATH] = TEXT("");

    // Output display stats
    FLOAT fNextLine = 40.0f; 

	if(g_Globals->World.bRenderHelpText)
	{
		fNextLine = 0.0f;
		sprintf(szMsg, "F2 - Toggle Paths      F3 - Toggle Text");
	    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

		sprintf(szMsg, "F5 - Toggle Mini Map   F6 - Toggle Team Panel");
		fNextLine += 20.0f;
	    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

		sprintf(szMsg, "F7 - Toggle Unit Panel F8 - Toggle Building Outline/Elevations");
		fNextLine += 20.0f;
	    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

		sprintf(szMsg, "F9 - Toggle Elements");
		fNextLine += 20.0f;
	    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );
	}
	else if(g_Globals->World.bRenderStats)
	{
	    strcpy( szMsg, m_strDeviceStats );
		fNextLine -= 20.0f;
	    m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );

		strcpy( szMsg, m_strFrameStats );
		fNextLine -= 20.0f;
		m_pFont->DrawText( 2, fNextLine, fontColor, szMsg );
	}

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: Pause()
// Desc: Called in to toggle the pause state of the app.
//-----------------------------------------------------------------------------
VOID CMyD3DApplication::Pause( bool bPause )
{
    CD3DApplication::Pause( bPause );
}




//-----------------------------------------------------------------------------
// Name: MsgProc()
// Desc: Overrrides the main WndProc, so the sample can do custom message
//       handling (e.g. processing mouse, keyboard, or menu commands).
//-----------------------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc( HWND hWnd, UINT msg, WPARAM wParam,
                                    LPARAM lParam )
{
    switch( msg )
    {
        case WM_PAINT:
        {
            if( m_bLoadingApp )
            {
                // Draw on the window tell the user that the app is loading
                // TODO: change as needed
                HDC hDC = GetDC( hWnd );
                RECT rct;
                GetClientRect( hWnd, &rct );
                DrawText( hDC, _statusText, -1, &rct, DT_CENTER|DT_VCENTER|DT_SINGLELINE );
                ReleaseDC( hWnd, hDC );
            }
            break;
        }

		case WM_KEYUP:
		{
			_game->KeyUp(wParam);
			break;
		}

		case WM_LBUTTONDOWN:
			_currentMouseState.bLeftDown = true;
			break;
		case WM_LBUTTONUP:
			_currentMouseState.bLeftDown = false;
			break;
		case WM_RBUTTONDOWN:
			_currentMouseState.bRightDown = true;
			break;
		case WM_RBUTTONUP:
			_currentMouseState.bRightDown = false;
			break;
    }

    return CD3DApplication::MsgProc( hWnd, msg, wParam, lParam );
}

//-----------------------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: Invalidates device objects.  Paired with RestoreDeviceObjects()
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
    // TODO: Cleanup any objects created in RestoreDeviceObjects()
    m_pFont->InvalidateDeviceObjects();
    _fontManager->Invalidate();

	// Release the back buffer
	_backBuffer->Release();

	// Cleanup the screen
	_screen->Cleanup();

    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: Paired with InitDeviceObjects()
//       Called when the app is exiting, or the device is being changed,
//       this function deletes any device dependent objects.  
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    // TODO: Cleanup any objects created in InitDeviceObjects()
    m_pFont->DeleteDeviceObjects();
	_fontManager->Cleanup();
    return S_OK;
}

//-----------------------------------------------------------------------------
// Name: FinalCleanup()
// Desc: Paired with OneTimeSceneInit()
//       Called before the app exits, this function gives the app the chance
//       to cleanup after itself.
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
    // TODO: Perform any final cleanup needed
    // Cleanup D3D font
    SAFE_DELETE( m_pFont );
	delete _fontManager;

    // Cleanup DirectX audio objects
    SAFE_DELETE( m_pSoundManager );
    return S_OK;
}

void
CMyD3DApplication::Status(char *msg)
{
	strcpy(_statusText, msg);

	// Drawing loading status message until app finishes loading
    SendMessage( m_hWnd, WM_PAINT, 0, 0 );
}

void 
CMyD3DApplication::ShowCursor(bool bShow, CursorInterface::CursorType type)
{
	UNREFERENCED_PARAMETER(bShow);
	switch(type) {
		case CursorInterface::CursorType::Regular:
			SetCursor(LoadCursor( NULL, IDC_ARROW));
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) LoadCursor( NULL, IDC_ARROW));
			break;
		case CursorInterface::CursorType::MarkBlue:
			SetCursor(_cursors[CursorInterface::MarkBlue]);
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) _cursors[CursorInterface::MarkBlue]);
			break;
		case CursorInterface::CursorType::MarkPurple:
			SetCursor(_cursors[CursorInterface::MarkPurple]);
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) _cursors[CursorInterface::MarkPurple]);
			break;
		case CursorInterface::CursorType::MarkOrange:
			SetCursor(_cursors[CursorInterface::MarkOrange]);
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) _cursors[CursorInterface::MarkOrange]);
			break;
		case CursorInterface::CursorType::MarkYellow:
			SetCursor(_cursors[CursorInterface::MarkYellow]);
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) _cursors[CursorInterface::MarkYellow]);
			break;
		case CursorInterface::CursorType::MarkGreen:
			SetCursor(_cursors[CursorInterface::MarkGreen]);
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) _cursors[CursorInterface::MarkGreen]);
			break;
		case CursorInterface::CursorType::MarkRed:
			SetCursor(_cursors[CursorInterface::MarkRed]);
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) _cursors[CursorInterface::MarkRed]);
			break;
		case CursorInterface::CursorType::MarkBrown:
			SetCursor(_cursors[CursorInterface::MarkBrown]);
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) _cursors[CursorInterface::MarkBrown]);
			break;
		default:
			SetCursor(LoadCursor( NULL, IDC_ARROW));
			SetClassLong(m_hWnd,    // window handle 
						GCL_HCURSOR,      // change cursor 
						(LONG) LoadCursor( NULL, IDC_ARROW));
			break;

	}
}

HRESULT 
CMyD3DApplication::Create( HINSTANCE hInstance )
{
	_cursors[CursorInterface::MarkBlue]		= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MARK_BLUE));
	_cursors[CursorInterface::MarkPurple]	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MARK_PURPLE));
	_cursors[CursorInterface::MarkOrange]	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MARK_ORANGE));
	_cursors[CursorInterface::MarkYellow]	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MARK_YELLOW));
	_cursors[CursorInterface::MarkRed]		= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MARK_RED));
	_cursors[CursorInterface::MarkGreen]	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MARK_GREEN));
	_cursors[CursorInterface::MarkBrown]	= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MARK_BROWN));

	return CD3DApplication::Create(hInstance);
}


