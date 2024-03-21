//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Implements all the functions exported by the GameUI dll
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"

#if !defined( _X360 )
#include <windows.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <io.h>
#include <tier0/dbg.h>
#include <direct.h>

#ifdef SendMessage
#undef SendMessage
#endif
																
#include "FileSystem.h"
#include "GameUI_Interface.h"
#include "string.h"
#include "tier0/icommandline.h"

// interface to engine
#include "EngineInterface.h"

#include "bitmap/TGALoader.h"

#include "GameConsole.h"
#include "game/client/IGameClientExports.h"
#include "materialsystem/imaterialsystem.h"
#include "ixboxsystem.h"
#include "iachievementmgr.h"
#include "IGameUIFuncs.h"
#include "IEngineVGUI.h"

// vgui2 interface
// note that GameUI project uses ..\vgui2\include, not ..\utils\vgui\include
#include "vgui/Cursor.h"
#include "tier1/KeyValues.h"
#include "vgui/ILocalize.h"
#include "vgui/IPanel.h"
#include "vgui/IScheme.h"
#include "vgui/IVGui.h"
#include "vgui/ISystem.h"
#include "vgui/ISurface.h"
#include "vgui_controls/Menu.h"
#include "vgui_controls/PHandle.h"
#include "tier3/tier3.h"
#include "matsys_controls/matsyscontrols.h"
#include "steam/steam_api.h"

#if defined( SWARM_DLL )

#include "swarm/basemodpanel.h"
#include "swarm/basemodui.h"
#include "asw_util_shared.h"
typedef vgui::Panel UI_BASEMOD_PANEL_CLASS;
inline UI_BASEMOD_PANEL_CLASS & GetUiBaseModPanelClass() { return UI_BASEMOD_PANEL_CLASS::GetSingleton(); }
inline UI_BASEMOD_PANEL_CLASS & ConstructUiBaseModPanelClass() { return * new UI_BASEMOD_PANEL_CLASS(); }
class IMatchExtSwarm *g_pMatchExtSwarm = NULL;

#else

typedef vgui::Panel UI_BASEMOD_PANEL_CLASS;
UI_BASEMOD_PANEL_CLASS* m_BASEMODPANEL_SINGLETON = 0;
inline UI_BASEMOD_PANEL_CLASS& GetUiBaseModPanelClass() { return *m_BASEMODPANEL_SINGLETON; }
inline UI_BASEMOD_PANEL_CLASS& ConstructUiBaseModPanelClass() 
{
	m_BASEMODPANEL_SINGLETON = new UI_BASEMOD_PANEL_CLASS(); 
	return *m_BASEMODPANEL_SINGLETON;
}

#endif

#ifdef _X360
#include "xbox/xbox_win32stubs.h"
#endif // _X360

#include "tier0/dbg.h"
#include "engine/IEngineSound.h"
#include "gameui_util.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IEngineVGui *enginevguifuncs = NULL;
#ifdef _X360
IXOnline  *xonline = NULL;			// 360 only
#endif
vgui::ISurface *enginesurfacefuncs = NULL;
IAchievementMgr *achievementmgr = NULL;

class CGameUI;
CGameUI *g_pGameUI = NULL;

class CLoadingDialog;
vgui::DHANDLE<CLoadingDialog> g_hLoadingDialog;
vgui::VPANEL g_hLoadingBackgroundDialog = NULL;

static CGameUI g_GameUI;

static IGameClientExports *g_pGameClientExports = NULL;
IGameClientExports *GameClientExports()
{
	return g_pGameClientExports;
}

//-----------------------------------------------------------------------------
// Purpose: singleton accessor
//-----------------------------------------------------------------------------
CGameUI &GameUI()
{
	return g_GameUI;
}


EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CGameUI, IGameUI, GAMEUI_INTERFACE_VERSION, g_GameUI);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CGameUI::CGameUI()
{
	g_pGameUI = this;
	m_bTryingToLoadFriends = false;
	m_iFriendsLoadPauseFrames = 0;
	m_iGameIP = 0;
	m_iGameConnectionPort = 0;
	m_iGameQueryPort = 0;
	m_bActivatedUI = false;
	m_szPreviousStatusText[0] = 0;
	m_bIsConsoleUI = false;
	m_bHasSavedThisMenuSession = false;
	m_bOpenProgressOnStart = false;
	m_iPlayGameStartupSound = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CGameUI::~CGameUI()
{
	g_pGameUI = NULL;
}


//-----------------------------------------------------------------------------
// Purpose: Initialization
//-----------------------------------------------------------------------------
void CGameUI::Initialize( CreateInterfaceFn factory )
{
	MEM_ALLOC_CREDIT();
	ConnectTier1Libraries( &factory, 1 );
	ConnectTier2Libraries( &factory, 1 );
	ConVar_Register( FCVAR_CLIENTDLL );
	ConnectTier3Libraries( &factory, 1 );

	enginesound = (IEngineSound *)factory(IENGINESOUND_CLIENT_INTERFACE_VERSION, NULL);
	engine = (IVEngineClient *)factory( VENGINE_CLIENT_INTERFACE_VERSION, NULL );
	g_pVGui = (vgui::IVGui*)factory(VGUI_IVGUI_INTERFACE_VERSION, NULL);
	g_pVGuiSurface = (vgui::ISurface*)factory(VGUI_SURFACE_INTERFACE_VERSION, NULL);
	g_pVGuiInput = (vgui::IInput*)factory("VGUI_Input005", NULL); // this is VGUI_INPUT_INTERFACE_VERSION
	g_pVGuiPanel = (vgui::IPanel*)factory(VGUI_PANEL_INTERFACE_VERSION, NULL);
	g_pVGuiSystem = (vgui::ISystem*)factory(VGUI_SYSTEM_INTERFACE_VERSION, NULL);
	g_pVGuiSchemeManager = (vgui::ISchemeManager*)factory(VGUI_SCHEME_INTERFACE_VERSION, NULL);
	g_pFullFileSystem = (IFileSystem*)factory(FILESYSTEM_INTERFACE_VERSION, NULL);
	g_pVGuiLocalize = (vgui::ILocalize*)factory("Localize_001", NULL); //this is LOCALIZE_INTERFACE_VERSION

	CGameUIConVarRef var( "gameui_xbox" );
	m_bIsConsoleUI = var.IsValid() && var.GetBool();

	vgui::VGui_InitInterfacesList( "GameUI", &factory, 1 );
	vgui::VGui_InitMatSysInterfacesList( "GameUI", &factory, 1 );



	bool bFailed = false;
	enginevguifuncs = (IEngineVGui *)factory( VENGINE_VGUI_VERSION, NULL );
	enginesurfacefuncs = (vgui::ISurface *)factory(VGUI_SURFACE_INTERFACE_VERSION, NULL);
	gameuifuncs = (IGameUIFuncs *)factory( VENGINE_GAMEUIFUNCS_VERSION, NULL );
	bFailed = !enginesurfacefuncs || !gameuifuncs || !enginevguifuncs;
	if ( bFailed )
	{
		Error( "CGameUI::Initialize() failed to get necessary interfaces\n" );
	}

	// setup base panel
	vgui::Panel& factoryBasePanel = ConstructUiBaseModPanelClass(); // explicit singleton instantiation

	factoryBasePanel.SetBounds( 0, 0, 640, 480 );
	factoryBasePanel.SetPaintBorderEnabled( false );
	factoryBasePanel.SetPaintBackgroundEnabled( true );
	factoryBasePanel.SetPaintEnabled( true );
	factoryBasePanel.SetVisible( true );

	factoryBasePanel.SetMouseInputEnabled( IsPC() );
	// factoryBasePanel.SetKeyBoardInputEnabled( IsPC() );
	factoryBasePanel.SetKeyBoardInputEnabled( true );

	vgui::VPANEL rootpanel = enginevguifuncs->GetPanel( PANEL_GAMEUIDLL );
	factoryBasePanel.SetParent( rootpanel );
}

void CGameUI::PostInit()
{
	if ( IsX360() )
	{
		enginesound->PrecacheSound( "UI/buttonrollover.wav", true, true );
		enginesound->PrecacheSound( "UI/buttonclick.wav", true, true );
		enginesound->PrecacheSound( "UI/buttonclickrelease.wav", true, true );
		enginesound->PrecacheSound( "player/suit_denydevice.wav", true, true );

		enginesound->PrecacheSound( "UI/menu_accept.wav", true, true );
		enginesound->PrecacheSound( "UI/menu_focus.wav", true, true );
		enginesound->PrecacheSound( "UI/menu_invalid.wav", true, true );
		enginesound->PrecacheSound( "UI/menu_back.wav", true, true );
		enginesound->PrecacheSound( "UI/menu_countdown.wav", true, true );
	}

#ifdef SWARM_DLL
	// to know once client dlls have been loaded
	BaseModUI::CUIGameData::Get()->OnGameUIPostInit();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Sets the specified panel as the background panel for the loading
//		dialog.  If NULL, default background is used.  If you set a panel,
//		it should be full-screen with an opaque background, and must be a VGUI popup.
//-----------------------------------------------------------------------------
void CGameUI::SetLoadingBackgroundDialog( vgui::VPANEL panel )
{
	g_hLoadingBackgroundDialog = panel;
}

//-----------------------------------------------------------------------------
// Purpose: connects to client interfaces
//-----------------------------------------------------------------------------
void CGameUI::Connect( CreateInterfaceFn gameFactory )
{
	g_pGameClientExports = (IGameClientExports *)gameFactory(GAMECLIENTEXPORTS_INTERFACE_VERSION, NULL);

	achievementmgr = engine->GetAchievementMgr();

	if (!g_pGameClientExports)
	{
		Error("CGameUI::Initialize() failed to get necessary interfaces\n");
	}

	m_GameFactory = gameFactory;
}


//-----------------------------------------------------------------------------
// Purpose: Searches for GameStartup*.mp3 files in the sound/ui folder and plays one
//-----------------------------------------------------------------------------
void CGameUI::PlayGameStartupSound()
{

}

//-----------------------------------------------------------------------------
// Purpose: Called to setup the game UI
//-----------------------------------------------------------------------------
void CGameUI::Start()
{
	// determine Steam location for configuration
	if ( !FindPlatformDirectory( m_szPlatformDir, sizeof( m_szPlatformDir ) ) )
		return;

	if ( IsPC() )
	{
		// setup config file directory
		char szConfigDir[512];
		Q_strncpy( szConfigDir, m_szPlatformDir, sizeof( szConfigDir ) );
		Q_strncat( szConfigDir, "config", sizeof( szConfigDir ), COPY_ALL_CHARACTERS );

		Msg( "Steam config directory: %s\n", szConfigDir );

		g_pFullFileSystem->AddSearchPath(szConfigDir, "CONFIG");
		g_pFullFileSystem->CreateDirHierarchy("", "CONFIG");

		// user dialog configuration
		vgui::system()->SetUserConfigFile("InGameDialogConfig.vdf", "CONFIG");

		g_pFullFileSystem->AddSearchPath( "platform", "PLATFORM" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Validates the user has a cdkey in the registry
//-----------------------------------------------------------------------------
void CGameUI::ValidateCDKey()
{
}

//-----------------------------------------------------------------------------
// Purpose: Finds which directory the platform resides in
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CGameUI::FindPlatformDirectory(char *platformDir, int bufferSize)
{
	platformDir[0] = '\0';

	if ( platformDir[0] == '\0' )
	{
		// we're not under steam, so setup using path relative to game
		if ( IsPC() )
		{
			if ( ::GetModuleFileName( ( HINSTANCE )GetModuleHandle( NULL ), platformDir, bufferSize ) )
			{
				char *lastslash = strrchr(platformDir, '\\'); // this should be just before the filename
				if ( lastslash )
				{
					*lastslash = 0;
					Q_strncat(platformDir, "\\platform\\", bufferSize, COPY_ALL_CHARACTERS );
					return true;
				}
			}
		}
		else
		{
			// xbox fetches the platform path from exisiting platform search path
			// path to executeable is not correct for xbox remote configuration
			if ( g_pFullFileSystem->GetSearchPath( "PLATFORM", false, platformDir, bufferSize ) )
			{
				char *pSeperator = strchr( platformDir, ';' );
				if ( pSeperator )
					*pSeperator = '\0';
				return true;
			}
		}

		Error( "Unable to determine platform directory\n" );
		return false;
	}

	return (platformDir[0] != 0);
}

//-----------------------------------------------------------------------------
// Purpose: Called to Shutdown the game UI system
//-----------------------------------------------------------------------------
void CGameUI::Shutdown()
{


#ifndef _X360
	// SteamAPI_Shutdown(); << Steam shutdown is controlled by engine
#endif
	
	ConVar_Unregister();
	DisconnectTier3Libraries();
	DisconnectTier2Libraries();
	DisconnectTier1Libraries();
}

//-----------------------------------------------------------------------------
// Purpose: just wraps an engine call to activate the gameUI
//-----------------------------------------------------------------------------
void CGameUI::ActivateGameUI()
{
	engine->ExecuteClientCmd("gameui_activate");
	// Lock the UI to a particular player
	SetGameUIActiveSplitScreenPlayerSlot( engine->GetActiveSplitScreenPlayerSlot() );
}

//-----------------------------------------------------------------------------
// Purpose: just wraps an engine call to hide the gameUI
//-----------------------------------------------------------------------------
void CGameUI::HideGameUI()
{
	engine->ExecuteClientCmd("gameui_hide");
}

//-----------------------------------------------------------------------------
// Purpose: Toggle allowing the engine to hide the game UI with the escape key
//-----------------------------------------------------------------------------
void CGameUI::PreventEngineHideGameUI()
{
	engine->ExecuteClientCmd("gameui_preventescape");
}

//-----------------------------------------------------------------------------
// Purpose: Toggle allowing the engine to hide the game UI with the escape key
//-----------------------------------------------------------------------------
void CGameUI::AllowEngineHideGameUI()
{
	engine->ExecuteClientCmd("gameui_allowescape");
}

//-----------------------------------------------------------------------------
// Purpose: Activate the game UI
//-----------------------------------------------------------------------------
void CGameUI::OnGameUIActivated()
{
	bool bWasActive = m_bActivatedUI;
	m_bActivatedUI = true;

	// Lock the UI to a particular player
	if ( !bWasActive )
	{
		SetGameUIActiveSplitScreenPlayerSlot( engine->GetActiveSplitScreenPlayerSlot() );
	}

	// pause the server in case it is pausable
	engine->ClientCmd_Unrestricted( "setpause nomsg" );

	SetSavedThisMenuSession( false );

	UI_BASEMOD_PANEL_CLASS &ui = GetUiBaseModPanelClass();
	bool bNeedActivation = true;
	if ( ui.IsVisible() )
	{
		// Already visible, maybe don't need activation
		if ( !IsInLevel() && IsInBackgroundLevel() )
			bNeedActivation = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hides the game ui, in whatever state it's in
//-----------------------------------------------------------------------------
void CGameUI::OnGameUIHidden()
{
	bool bWasActive = m_bActivatedUI;
	m_bActivatedUI = false;

	// unpause the game when leaving the UI
	engine->ClientCmd_Unrestricted( "unpause nomsg" );


	// Restore to default
	if ( bWasActive )
	{
		SetGameUIActiveSplitScreenPlayerSlot( 0 );
	}
}

//-----------------------------------------------------------------------------
// Purpose: paints all the vgui elements
//-----------------------------------------------------------------------------
void CGameUI::RunFrame()
{
	if ( IsX360() && m_bOpenProgressOnStart )
	{
		StartProgressBar();
		m_bOpenProgressOnStart = false;
	}

	int wide, tall;
#if defined( TOOLFRAMEWORK_VGUI_REFACTOR )
	// resize the background panel to the screen size
	vgui::VPANEL clientDllPanel = enginevguifuncs->GetPanel( PANEL_ROOT );

	int x, y;
	vgui::ipanel()->GetPos( clientDllPanel, x, y );
	vgui::ipanel()->GetSize( clientDllPanel, wide, tall );
	staticPanel->SetBounds( x, y, wide,tall );
#else
	vgui::surface()->GetScreenSize(wide, tall);

	GetUiBaseModPanelClass().SetSize(wide, tall);
#endif



}

//-----------------------------------------------------------------------------
// Purpose: Called when the game connects to a server
//-----------------------------------------------------------------------------
void CGameUI::OLD_OnConnectToServer(const char *game, int IP, int port)
{
	// Nobody should use this anymore because the query port and the connection port can be different.
	// Use OnConnectToServer2 instead.
	Assert( false );
	OnConnectToServer2( game, IP, port, port );
}

//-----------------------------------------------------------------------------
// Purpose: Called when the game connects to a server
//-----------------------------------------------------------------------------
void CGameUI::OnConnectToServer2(const char *game, int IP, int connectionPort, int queryPort)
{
	m_iGameIP = IP;
	m_iGameConnectionPort = connectionPort;
	m_iGameQueryPort = queryPort;

	SendConnectedToGameMessage();
}


void CGameUI::SendConnectedToGameMessage()
{

}



//-----------------------------------------------------------------------------
// Purpose: Called when the game disconnects from a server
//-----------------------------------------------------------------------------
void CGameUI::OnDisconnectFromServer( uint8 eSteamLoginFailure )
{
	m_iGameIP = 0;
	m_iGameConnectionPort = 0;
	m_iGameQueryPort = 0;

	if ( g_hLoadingBackgroundDialog )
	{
		vgui::ivgui()->PostMessage( g_hLoadingBackgroundDialog, new KeyValues("DisconnectedFromGame"), NULL );
	}

}

//-----------------------------------------------------------------------------
// Purpose: activates the loading dialog on level load start
//-----------------------------------------------------------------------------
void CGameUI::OnLevelLoadingStarted( const char *levelName, bool bShowProgressDialog )
{

	ShowLoadingBackgroundDialog();

	if ( bShowProgressDialog )
	{
		StartProgressBar();
	}

	// Don't play the start game sound if this happens before we get to the first frame
	m_iPlayGameStartupSound = 0;
}

//-----------------------------------------------------------------------------
// Purpose: closes any level load dialog
//-----------------------------------------------------------------------------
void CGameUI::OnLevelLoadingFinished(bool bError, const char *failureReason, const char *extendedReason)
{
	StopProgressBar( bError, failureReason, extendedReason );


	HideLoadingBackgroundDialog();


}

//-----------------------------------------------------------------------------
// Purpose: Updates progress bar
// Output : Returns true if screen should be redrawn
//-----------------------------------------------------------------------------
bool CGameUI::UpdateProgressBar(float progress, const char *statusText)
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameUI::SetProgressLevelName( const char *levelName )
{
	MEM_ALLOC_CREDIT();
	if ( g_hLoadingBackgroundDialog )
	{
		KeyValues *pKV = new KeyValues( "ProgressLevelName" );
		pKV->SetString( "levelName", levelName );
		vgui::ivgui()->PostMessage( g_hLoadingBackgroundDialog, pKV, NULL );
	}

	if ( g_hLoadingDialog.Get() )
	{
		// TODO: g_hLoadingDialog->SetLevelName( levelName );
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameUI::StartProgressBar()
{
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the screen should be updated
//-----------------------------------------------------------------------------
bool CGameUI::ContinueProgressBar(float progressFraction)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: stops progress bar, displays error if necessary
//-----------------------------------------------------------------------------
void CGameUI::StopProgressBar(bool bError, const char *failureReason, const char *extendedReason)
{
	return;
	// should update the background to be in a transition here
}

//-----------------------------------------------------------------------------
// Purpose: sets loading info text
//-----------------------------------------------------------------------------
bool CGameUI::SetProgressBarStatusText(const char *statusText)
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameUI::SetSecondaryProgressBar(float progress /* range [0..1] */)
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGameUI::SetSecondaryProgressBarText(const char *statusText)
{
	return;
}

//-----------------------------------------------------------------------------
// Purpose: Returns prev settings
//-----------------------------------------------------------------------------
bool CGameUI::SetShowProgressText( bool show )
{
	return true;
}


//-----------------------------------------------------------------------------
// Purpose: returns true if we're currently playing the game
//-----------------------------------------------------------------------------
bool CGameUI::IsInLevel()
{
	const char *levelName = engine->GetLevelName();
	if (levelName && levelName[0] && !engine->IsLevelMainMenuBackground())
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if we're at the main menu and a background level is loaded
//-----------------------------------------------------------------------------
bool CGameUI::IsInBackgroundLevel()
{
	const char *levelName = engine->GetLevelName();
	if (levelName && levelName[0] && engine->IsLevelMainMenuBackground())
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if we're in a multiplayer game
//-----------------------------------------------------------------------------
bool CGameUI::IsInMultiplayer()
{
	return (IsInLevel() && engine->GetMaxClients() > 1);
}

//-----------------------------------------------------------------------------
// Purpose: returns true if we're console ui
//-----------------------------------------------------------------------------
bool CGameUI::IsConsoleUI()
{
	return m_bIsConsoleUI;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if we've saved without closing the menu
//-----------------------------------------------------------------------------
bool CGameUI::HasSavedThisMenuSession()
{
	return m_bHasSavedThisMenuSession;
}

void CGameUI::SetSavedThisMenuSession( bool bState )
{
	m_bHasSavedThisMenuSession = bState;
}

//-----------------------------------------------------------------------------
// Purpose: Makes the loading background dialog visible, if one has been set
//-----------------------------------------------------------------------------
void CGameUI::ShowLoadingBackgroundDialog()
{
	if ( g_hLoadingBackgroundDialog )
	{
		vgui::VPANEL panel = GetUiBaseModPanelClass().GetVPanel();

		vgui::ipanel()->SetParent( g_hLoadingBackgroundDialog, panel );
		vgui::ipanel()->MoveToFront( g_hLoadingBackgroundDialog );
	}
}


//-----------------------------------------------------------------------------
// Purpose: Hides the loading background dialog, if one has been set
//-----------------------------------------------------------------------------
void CGameUI::HideLoadingBackgroundDialog()
{
	if ( g_hLoadingBackgroundDialog )
	{
		if ( engine->IsInGame() )
		{
			vgui::ivgui()->PostMessage( g_hLoadingBackgroundDialog, new KeyValues( "LoadedIntoGame" ), NULL );
		}
		else
		{
			vgui::ipanel()->SetVisible( g_hLoadingBackgroundDialog, false );
			vgui::ipanel()->MoveToBack( g_hLoadingBackgroundDialog );
		}

		vgui::ivgui()->PostMessage( g_hLoadingBackgroundDialog, new KeyValues("HideAsLoadingPanel"), NULL );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns whether a loading background dialog has been set
//-----------------------------------------------------------------------------
bool CGameUI::HasLoadingBackgroundDialog()
{
	return ( NULL != g_hLoadingBackgroundDialog );
}

//-----------------------------------------------------------------------------

void CGameUI::NeedConnectionProblemWaitScreen()
{
#ifdef SWARM_DLL
	BaseModUI::CUIGameData::Get()->NeedConnectionProblemWaitScreen();
#endif
}

void CGameUI::ShowPasswordUI( char const *pchCurrentPW )
{
#ifdef SWARM_DLL
	BaseModUI::CUIGameData::Get()->ShowPasswordUI( pchCurrentPW );
#endif
}

//-----------------------------------------------------------------------------
void CGameUI::SetProgressOnStart()
{
	m_bOpenProgressOnStart = true;
}

#if defined( _X360 ) && defined( _DEMO )
void CGameUI::OnDemoTimeout()
{
	GetUiBaseModPanelClass().OnDemoTimeout();
}
#endif
