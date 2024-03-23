//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include <crtmemdebug.h>
#include "vgui_int.h"
#include "clientmode.h"
#include "iinput.h"
#include "iviewrender.h"
#include "ivieweffects.h"
#include "ivmodemanager.h"
#include "prediction.h"
#include "clientsideeffects.h"
#include "particlemgr.h"
#include "steam/steam_api.h"
#include "initializer.h"
#include "smoke_fog_overlay.h"
#include "view.h"
#include "ienginevgui.h"
#include "iefx.h"
#include "enginesprite.h"
#include "networkstringtable_clientdll.h"
#include "voice_status.h"
#include "filesystem.h"
#include "c_te_legacytempents.h"
#include "c_rope.h"
#include "engine/ishadowmgr.h"
#include "engine/IStaticPropMgr.h"
#include "hud_basechat.h"
#include "hud_crosshair.h"
#include "view_shared.h"
#include "env_wind_shared.h"
#include "detailobjectsystem.h"
#include "clienteffectprecachesystem.h"
#include "soundenvelope.h"
#include "c_basetempentity.h"
#include "materialsystem/imaterialsystemstub.h"
#include "VGuiMatSurface/IMatSystemSurface.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "c_soundscape.h"
#include "engine/ivdebugoverlay.h"
#include "vguicenterprint.h"
#include "iviewrender_beams.h"
#include "tier0/vprof.h"
#include "engine/IEngineTrace.h"
#include "engine/ivmodelinfo.h"
#include "physics.h"
#include "usermessages.h"
#include "gamestringpool.h"
#include "c_user_message_register.h"
#include "IGameUIFuncs.h"
#include "saverestoretypes.h"
#include "saverestore.h"
#include "physics_saverestore.h"
#include "igameevents.h"
#include "datacache/idatacache.h"
#include "datacache/imdlcache.h"
#include "kbutton.h"
#include "tier0/icommandline.h"
#include "gamerules_register.h"
#include "vgui_controls/AnimationController.h"
#include "bitmap/tgawriter.h"
#include "c_world.h"
#include "perfvisualbenchmark.h"	
#include "SoundEmitterSystem/isoundemittersystembase.h"
#include "hud_closecaption.h"
#include "colorcorrectionmgr.h"
#include "physpropclientside.h"
#include "panelmetaclassmgr.h"
#include "c_vguiscreen.h"
#include "imessagechars.h"
#include "game/client/IGameClientExports.h"
#include "client_factorylist.h"
#include "ragdoll_shared.h"
#include "rendertexture.h"
#include "view_scene.h"
#include "iclientmode.h"
#include "con_nprint.h"
#include "inputsystem/iinputsystem.h"
#include "appframework/IAppSystemGroup.h"
#include "scenefilecache/ISceneFileCache.h"
#include "tier2/tier2dm.h"
#include "tier3/tier3.h"
#include "ihudlcd.h"
#include "toolframework_client.h"
#include "hltvcamera.h"
#ifdef SDK2013CE
#include "shaderapihack.h"
#endif
#if defined( REPLAY_ENABLED )
#include "replay/replaycamera.h"
#include "replay/replay_ragdoll.h"
#include "qlimits.h"
#include "replay/replay.h"
#include "replay/ireplaysystem.h"
#include "replay/iclientreplay.h"
#include "replay/ienginereplay.h"
#include "replay/ireplaymanager.h"
#include "replay/ireplayscreenshotmanager.h"
#include "replay/iclientreplaycontext.h"
#include "replay/vgui/replayconfirmquitdlg.h"
#include "replay/vgui/replaybrowsermainpanel.h"
#include "replay/vgui/replayinputpanel.h"
#include "replay/vgui/replayperformanceeditor.h"
#endif
#include "vgui/ILocalize.h"
#include "vgui/IVGui.h"
#include "ixboxsystem.h"
#include "ipresence.h"
#include "engine/imatchmaking.h"
#include "cdll_bounded_cvars.h"
#include "matsys_controls/matsyscontrols.h"
#include "gamestats.h"
#include "particle_parse.h"
#if defined( TF_CLIENT_DLL )
#include "rtime.h"
#include "tf_hud_disconnect_prompt.h"
#include "../engine/audio/public/sound.h"
#include "tf_shared_content_manager.h"
#endif
#include "clientsteamcontext.h"
#include "renamed_recvtable_compat.h"
#include "mouthinfo.h"
#include "sourcevr/isourcevirtualreality.h"
#include "client_virtualreality.h"
#include "mumble.h"
#include "squirrel/squirrel.h"
#include "vgui_controls/SquirrelPanel.h"

// NVNT includes
#include "hud_macros.h"
#include "haptics/ihaptics.h"
#include "haptics/haptic_utils.h"
#include "haptics/haptic_msgs.h"

#if defined( TF_CLIENT_DLL )
#include "abuse_report.h"
#endif

#ifdef USES_ECON_ITEMS
#include "econ_item_system.h"
#endif // USES_ECON_ITEMS

#if defined( TF_CLIENT_DLL )
#include "econ/tool_items/custom_texture_cache.h"

#endif

#ifdef WORKSHOP_IMPORT_ENABLED
#include "fbxsystem/fbxsystem.h"
#endif

extern vgui::IInputInternal *g_InputInternal;

//=============================================================================
// HPE_BEGIN
// [dwenger] Necessary for stats display
//=============================================================================

#include "achievements_and_stats_interface.h"

//=============================================================================
// HPE_END
//=============================================================================


#ifdef PORTAL
#include "PortalRender.h"
#endif

#ifdef SIXENSE
#include "sixense/in_sixense.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IClientMode *GetClientModeNormal();

// IF YOU ADD AN INTERFACE, EXTERN IT IN THE HEADER FILE.
IVEngineClient	*engine = NULL;
IVModelRender *modelrender = NULL;
IVEfx *effects = NULL;
IVRenderView *render = NULL;
IVDebugOverlay *debugoverlay = NULL;
IMaterialSystemStub *materials_stub = NULL;
IDataCache *datacache = NULL;
IVModelInfoClient *modelinfo = NULL;
IEngineVGui *enginevgui = NULL;
INetworkStringTableContainer *networkstringtable = NULL;
ISpatialPartition* partition = NULL;
IFileSystem *filesystem = NULL;
IShadowMgr *shadowmgr = NULL;
IStaticPropMgrClient *staticpropmgr = NULL;
IEngineSound *enginesound = NULL;
IUniformRandomStream *random = NULL;
static CGaussianRandomStream s_GaussianRandomStream;
CGaussianRandomStream *randomgaussian = &s_GaussianRandomStream;
ISharedGameRules *sharedgamerules = NULL;
IEngineTrace *enginetrace = NULL;
IGameUIFuncs *gameuifuncs = NULL;
IGameEventManager2 *gameeventmanager = NULL;
ISoundEmitterSystemBase *soundemitterbase = NULL;
IInputSystem *inputsystem = NULL;
ISceneFileCache *scenefilecache = NULL;
IXboxSystem *xboxsystem = NULL;	// Xbox 360 only
IMatchmaking *matchmaking = NULL;
IUploadGameStats *gamestatsuploader = NULL;
IClientReplayContext *g_pClientReplayContext = NULL;
IVideoServices* g_pVideo = NULL;
ISquirrel* g_pSquirrel = NULL;
CUtlVector<SquirrelScript> squirrelscripts;
CUtlMap<int, void*>* squirrelhandles[SPCOUNT] = {};
int squirrelhandleid[SPCOUNT] = {};

SquirrelHandle NewSquirrelHandle(int pool, void* ptr)
{
	int id = squirrelhandleid[pool];
	if (!squirrelhandles[pool])
	{
		squirrelhandles[pool] = new CUtlMap<int, void*>();
		SetDefLessFunc(*squirrelhandles[pool]);
	}
	CUtlMap<int, void*>* handles = squirrelhandles[pool];
	handles->InsertOrReplace(id,ptr);
	++squirrelhandleid[pool];
	SquirrelHandle handle;
	handle.poolid = (id<<8) + pool;
	return handle;
}

void* CheckSquirrelHandle(SquirrelHandle handle, int pool)
{
	if(handle.p.pool != pool)
	{
		return 0;
	}
	if (!squirrelhandles[handle.p.pool] || !squirrelhandles[pool])
	{
		return 0;
	}
	if (!&squirrelhandles[handle.p.pool]->operator[](handle.poolid >> 8))
	{
		return 0;
	}
	return squirrelhandles[handle.p.pool]->operator[](handle.poolid >> 8);
}

#if defined( REPLAY_ENABLED )
IReplayManager *g_pReplayManager = NULL;
IReplayMovieManager *g_pReplayMovieManager = NULL;
IReplayScreenshotManager *g_pReplayScreenshotManager = NULL;
IReplayPerformanceManager *g_pReplayPerformanceManager = NULL;
IReplayPerformanceController *g_pReplayPerformanceController = NULL;
IEngineReplay *g_pEngineReplay = NULL;
IEngineClientReplay *g_pEngineClientReplay = NULL;
IReplaySystem *g_pReplay = NULL;
#endif

IHaptics* haptics = NULL;// NVNT haptics system interface singleton

//=============================================================================
// HPE_BEGIN
// [dwenger] Necessary for stats display
//=============================================================================

AchievementsAndStatsInterface* g_pAchievementsAndStatsInterface = NULL;

//=============================================================================
// HPE_END
//=============================================================================

IGameSystem *SoundEmitterSystem();
IGameSystem *ToolFrameworkClientSystem();

// Engine player info, no game related infos here
BEGIN_BYTESWAP_DATADESC( player_info_s )
	DEFINE_ARRAY( name, FIELD_CHARACTER, MAX_PLAYER_NAME_LENGTH ),
	DEFINE_FIELD( userID, FIELD_INTEGER ),
	DEFINE_ARRAY( guid, FIELD_CHARACTER, SIGNED_GUID_LEN + 1 ),
	DEFINE_FIELD( friendsID, FIELD_INTEGER ),
	DEFINE_ARRAY( friendsName, FIELD_CHARACTER, MAX_PLAYER_NAME_LENGTH ),
	DEFINE_FIELD( fakeplayer, FIELD_BOOLEAN ),
	DEFINE_FIELD( ishltv, FIELD_BOOLEAN ),
#if defined( REPLAY_ENABLED )
	DEFINE_FIELD( isreplay, FIELD_BOOLEAN ),
#endif
	DEFINE_ARRAY( customFiles, FIELD_INTEGER, MAX_CUSTOM_FILES ),
	DEFINE_FIELD( filesDownloaded, FIELD_INTEGER ),
END_BYTESWAP_DATADESC()

static bool g_bRequestCacheUsedMaterials = false;
void RequestCacheUsedMaterials()
{
	g_bRequestCacheUsedMaterials = true;
}

void ProcessCacheUsedMaterials()
{
	if ( !g_bRequestCacheUsedMaterials )
		return;

	g_bRequestCacheUsedMaterials = false;
	if ( materials )
	{
        materials->CacheUsedMaterials();
	}
}

// String tables
INetworkStringTable *g_pStringTableParticleEffectNames = NULL;
INetworkStringTable *g_StringTableEffectDispatch = NULL;
INetworkStringTable *g_StringTableVguiScreen = NULL;
INetworkStringTable *g_pStringTableMaterials = NULL;
INetworkStringTable *g_pStringTableInfoPanel = NULL;
INetworkStringTable *g_pStringTableClientSideChoreoScenes = NULL;
INetworkStringTable *g_pStringTableServerMapCycle = NULL;

#ifdef TF_CLIENT_DLL
INetworkStringTable *g_pStringTableServerPopFiles = NULL;
INetworkStringTable *g_pStringTableServerMapCycleMvM = NULL;
#endif

static CGlobalVarsBase dummyvars( true );
// So stuff that might reference gpGlobals during DLL initialization won't have a NULL pointer.
// Once the engine calls Init on this DLL, this pointer gets assigned to the shared data in the engine
CGlobalVarsBase *gpGlobals = &dummyvars;
class CHudChat;
class CViewRender;
extern CViewRender g_DefaultViewRender;

extern void StopAllRumbleEffects( void );

static C_BaseEntityClassList *s_pClassLists = NULL;
C_BaseEntityClassList::C_BaseEntityClassList()
{
	m_pNextClassList = s_pClassLists;
	s_pClassLists = this;
}
C_BaseEntityClassList::~C_BaseEntityClassList()
{
}

// Any entities that want an OnDataChanged during simulation register for it here.
class CDataChangedEvent
{
public:
	CDataChangedEvent() {}
	CDataChangedEvent( IClientNetworkable *ent, DataUpdateType_t updateType, int *pStoredEvent )
	{
		m_pEntity = ent;
		m_UpdateType = updateType;
		m_pStoredEvent = pStoredEvent;
	}

	IClientNetworkable	*m_pEntity;
	DataUpdateType_t	m_UpdateType;
	int					*m_pStoredEvent;
};

ISaveRestoreBlockHandler *GetEntitySaveRestoreBlockHandler();
ISaveRestoreBlockHandler *GetViewEffectsRestoreBlockHandler();

CUtlLinkedList<CDataChangedEvent, unsigned short> g_DataChangedEvents;
ClientFrameStage_t g_CurFrameStage = FRAME_UNDEFINED;


class IMoveHelper;

void DispatchHudText( const char *pszName );

static ConVar s_CV_ShowParticleCounts("showparticlecounts", "0", 0, "Display number of particles drawn per frame");
static ConVar s_cl_team("cl_team", "default", FCVAR_USERINFO|FCVAR_ARCHIVE, "Default team when joining a game");
static ConVar s_cl_class("cl_class", "default", FCVAR_USERINFO|FCVAR_ARCHIVE, "Default class when joining a game");

#ifdef HL1MP_CLIENT_DLL
static ConVar s_cl_load_hl1_content("cl_load_hl1_content", "0", FCVAR_ARCHIVE, "Mount the content from Half-Life: Source if possible");
#endif


// Physics system
bool g_bLevelInitialized;
bool g_bTextMode = false;
class IClientPurchaseInterfaceV2 *g_pClientPurchaseInterface = (class IClientPurchaseInterfaceV2 *)(&g_bTextMode + 156);

static ConVar *g_pcv_ThreadMode = NULL;

#ifdef SDK2013CE
void ApplyShaderConstantHack()
{
	CMaterialConfigWrapper Wrapper;

	Wrapper.PrintPixelConstants();
	Wrapper.SetNumPixelConstants(254);
	Wrapper.SetNumBooleanPixelConstants(16);
	Wrapper.SetNumIntegerPixelConstants(16);
	Wrapper.PrintPixelConstants();
}
#endif


void CMaterialReference::Shutdown()
{

}

//-----------------------------------------------------------------------------
// Purpose: interface for gameui to modify voice bans
//-----------------------------------------------------------------------------
class CGameClientExports : public IGameClientExports
{
public:
	// ingame voice manipulation
	bool IsPlayerGameVoiceMuted(int playerIndex)
	{
		return GetClientVoiceMgr()->IsPlayerBlocked(playerIndex);
	}

	void MutePlayerGameVoice(int playerIndex)
	{
		GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
	}

	void UnmutePlayerGameVoice(int playerIndex)
	{
		GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, false);
	}

	void OnGameUIActivated( void )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "gameui_activated" );
		if ( event )
		{
			gameeventmanager->FireEventClientSide( event );
		}
	}

	void OnGameUIHidden( void )
	{
		IGameEvent *event = gameeventmanager->CreateEvent( "gameui_hidden" );
		if ( event )
		{
			gameeventmanager->FireEventClientSide( event );
		}
	}

    //=============================================================================
    // HPE_BEGIN
    // [dwenger] Necessary for stats display
    //=============================================================================

    void CreateAchievementsPanel( vgui::Panel* pParent )
    {
        if (g_pAchievementsAndStatsInterface)
        {
            g_pAchievementsAndStatsInterface->CreatePanel( pParent );
        }
    }

    void DisplayAchievementPanel()
    {
        if (g_pAchievementsAndStatsInterface)
        {
            g_pAchievementsAndStatsInterface->DisplayPanel();
        }
    }

    void ShutdownAchievementPanel()
    {
        if (g_pAchievementsAndStatsInterface)
        {
            g_pAchievementsAndStatsInterface->ReleasePanel();
        }
    }

	int GetAchievementsPanelMinWidth( void ) const
	{
        if ( g_pAchievementsAndStatsInterface )
        {
            return g_pAchievementsAndStatsInterface->GetAchievementsPanelMinWidth();
        }

		return 0;
	}

    //=============================================================================
    // HPE_END
    //=============================================================================

	const char *GetHolidayString()
	{
		return UTIL_GetActiveHolidayString();
	}
};

EXPOSE_SINGLE_INTERFACE( CGameClientExports, IGameClientExports, GAMECLIENTEXPORTS_INTERFACE_VERSION );

class CClientDLLSharedAppSystems : public IClientDLLSharedAppSystems
{
public:
	CClientDLLSharedAppSystems()
	{
		AddAppSystem( "soundemittersystem" DLL_EXT_STRING, SOUNDEMITTERSYSTEM_INTERFACE_VERSION );
		AddAppSystem( "scenefilecache" DLL_EXT_STRING, SCENE_FILE_CACHE_INTERFACE_VERSION );
	}

	virtual int	Count()
	{
		return m_Systems.Count();
	}
	virtual char const *GetDllName( int idx )
	{
		return m_Systems[ idx ].m_pModuleName;
	}
	virtual char const *GetInterfaceName( int idx )
	{
		return m_Systems[ idx ].m_pInterfaceName;
	}
private:
	void AddAppSystem( char const *moduleName, char const *interfaceName )
	{
		AppSystemInfo_t sys;
		sys.m_pModuleName = moduleName;
		sys.m_pInterfaceName = interfaceName;
		m_Systems.AddToTail( sys );
	}

	CUtlVector< AppSystemInfo_t >	m_Systems;
};

EXPOSE_SINGLE_INTERFACE( CClientDLLSharedAppSystems, IClientDLLSharedAppSystems, CLIENT_DLL_SHARED_APPSYSTEMS );


//-----------------------------------------------------------------------------
// Helper interface for voice.
//-----------------------------------------------------------------------------
class CHLVoiceStatusHelper : public IVoiceStatusHelper
{
public:
	virtual void GetPlayerTextColor(int entindex, int color[3])
	{
		color[0] = color[1] = color[2] = 128;
	}

	virtual void UpdateCursorState()
	{
	}

	virtual bool			CanShowSpeakerLabels()
	{
		return true;
	}
};
static CHLVoiceStatusHelper g_VoiceStatusHelper;

//-----------------------------------------------------------------------------
// Code to display which entities are having their bones setup each frame.
//-----------------------------------------------------------------------------

ConVar cl_ShowBoneSetupEnts( "cl_ShowBoneSetupEnts", "0", 0, "Show which entities are having their bones setup each frame." );

class CBoneSetupEnt
{
public:
	char m_ModelName[128];
	int m_Index;
	int m_Count;
};

bool BoneSetupCompare( const CBoneSetupEnt &a, const CBoneSetupEnt &b )
{
	return a.m_Index < b.m_Index;
}

CUtlRBTree<CBoneSetupEnt> g_BoneSetupEnts( BoneSetupCompare );


void TrackBoneSetupEnt( C_BaseAnimating *pEnt )
{
#ifdef _DEBUG
	if ( IsRetail() )
		return;
		
	if ( !cl_ShowBoneSetupEnts.GetInt() )
		return;

	CBoneSetupEnt ent;
	ent.m_Index = pEnt->entindex();
	unsigned short i = g_BoneSetupEnts.Find( ent );
	if ( i == g_BoneSetupEnts.InvalidIndex() )
	{
		Q_strncpy( ent.m_ModelName, modelinfo->GetModelName( pEnt->GetModel() ), sizeof( ent.m_ModelName ) );
		ent.m_Count = 1;
		g_BoneSetupEnts.Insert( ent );
	}
	else
	{
		g_BoneSetupEnts[i].m_Count++;
	}
#endif
}

void DisplayBoneSetupEnts()
{
#ifdef _DEBUG
	if ( IsRetail() )
		return;
	
	if ( !cl_ShowBoneSetupEnts.GetInt() )
		return;

	unsigned short i;
	int nElements = 0;
	for ( i=g_BoneSetupEnts.FirstInorder(); i != g_BoneSetupEnts.LastInorder(); i=g_BoneSetupEnts.NextInorder( i ) )
		++nElements;
		
	engine->Con_NPrintf( 0, "%d bone setup ents (name/count/entindex) ------------", nElements );

	con_nprint_s printInfo;
	printInfo.time_to_live = -1;
	printInfo.fixed_width_font = true;
	printInfo.color[0] = printInfo.color[1] = printInfo.color[2] = 1;
	
	printInfo.index = 2;
	for ( i=g_BoneSetupEnts.FirstInorder(); i != g_BoneSetupEnts.LastInorder(); i=g_BoneSetupEnts.NextInorder( i ) )
	{
		CBoneSetupEnt *pEnt = &g_BoneSetupEnts[i];
		
		if ( pEnt->m_Count >= 3 )
		{
			printInfo.color[0] = 1;
			printInfo.color[1] = 0;
			printInfo.color[2] = 0;
		}
		else if ( pEnt->m_Count == 2 )
		{
			printInfo.color[0] = (float)200 / 255;
			printInfo.color[1] = (float)220 / 255;
			printInfo.color[2] = 0;
		}
		else
		{
			printInfo.color[0] = 1;
			printInfo.color[1] = 1;
			printInfo.color[2] = 1;
		}
		engine->Con_NXPrintf( &printInfo, "%25s / %3d / %3d", pEnt->m_ModelName, pEnt->m_Count, pEnt->m_Index );
		printInfo.index++;
	}

	g_BoneSetupEnts.RemoveAll();
#endif
}

//-----------------------------------------------------------------------------
// Purpose: engine to client .dll interface
//-----------------------------------------------------------------------------
class CHLClient : public IBaseClientDLL
{
public:
	CHLClient();

	virtual int						Connect(CreateInterfaceFn appSystemFactory, CGlobalVarsBase* pGlobals);
	virtual int						Init(CreateInterfaceFn appSystemFactory, CGlobalVarsBase* pGlobals);

	virtual void					PostInit();
	virtual void					Shutdown(void);

	virtual void					LevelInitPreEntity(const char* pMapName);
	virtual void					LevelInitPostEntity();
	virtual void					LevelShutdown(void);

	virtual ClientClass* GetAllClasses(void);

	virtual int						HudVidInit(void);
	virtual void					HudProcessInput(bool bActive);
	virtual void					HudUpdate(bool bActive);
	virtual void					HudReset(void);
	virtual void					HudText(const char* message);

	// Mouse Input Interfaces
	virtual void					IN_ActivateMouse(void);
	virtual void					IN_DeactivateMouse(void);
	virtual void					IN_Accumulate(void);
	virtual void					IN_ClearStates(void);
	virtual bool					IN_IsKeyDown(const char* name, bool& isdown);
	// Raw signal
	virtual int						IN_KeyEvent(int eventcode, ButtonCode_t keynum, const char* pszCurrentBinding);
	virtual void					IN_SetSampleTime(float frametime);
	// Create movement command
	virtual void					CreateMove(int sequence_number, float input_sample_frametime, bool active);
	virtual void					ExtraMouseSample(float frametime, bool active);
	virtual bool					WriteUsercmdDeltaToBuffer(int nSlot, bf_write* buf, int from, int to, bool isnewcommand) { return input->WriteUsercmdDeltaToBuffer(nSlot, buf, from, to, isnewcommand); };
	virtual void					EncodeUserCmdToBuffer(int nSlot, bf_write& buf, int slot) { input->EncodeUserCmdToBuffer(nSlot, buf, slot); };
	virtual void					DecodeUserCmdFromBuffer(int nSlot, bf_read& buf, int slot) { input->DecodeUserCmdFromBuffer(nSlot, buf, slot); };


	virtual void					View_Render(vrect_t* rect);
	virtual void					RenderView(const CViewSetup& view, int nClearFlags, int whatToDraw);
	virtual void					View_Fade(ScreenFade_t* pSF);

	virtual void					SetCrosshairAngle(const QAngle& angle);

	virtual void					InitSprite(CEngineSprite* pSprite, const char* loadname);
	virtual void					ShutdownSprite(CEngineSprite* pSprite);

	virtual int						GetSpriteSize(void) const;

	virtual void					VoiceStatus(int entindex, int iSsSlot, qboolean bTalking) { GetClientVoiceMgr()->UpdateSpeakerStatus(entindex, !!bTalking); };

	virtual void					InstallStringTableCallback(const char* tableName);

	virtual void					FrameStageNotify(ClientFrameStage_t curStage);

	virtual bool					DispatchUserMessage(int msg_type, bf_read& msg_data);

	// Save/restore system hooks
	virtual CSaveRestoreData* SaveInit(int size);
	virtual void			SaveWriteFields(CSaveRestoreData*, const char*, void*, datamap_t*, typedescription_t*, int);
	virtual void			SaveReadFields(CSaveRestoreData*, const char*, void*, datamap_t*, typedescription_t*, int);
	virtual void			PreSave(CSaveRestoreData*);
	virtual void			Save(CSaveRestoreData*);
	virtual void			WriteSaveHeaders(CSaveRestoreData*);
	virtual void			ReadRestoreHeaders(CSaveRestoreData*);
	virtual void			Restore(CSaveRestoreData*, bool);
	virtual void			DispatchOnRestore();
	virtual void			WriteSaveGameScreenshot(const char* pFilename);

	// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
	//  the appropriate close caption if running with closecaption = 1
	virtual void			EmitSentenceCloseCaption(char const* tokenstream);
	virtual void			EmitCloseCaption(char const* captionname, float duration);

	virtual CStandardRecvProxies* GetStandardRecvProxies();

	virtual bool			CanRecordDemo(char* errorMsg, int length) const;

	virtual void			OnDemoRecordStart(char const* pDemoBaseName);
	virtual void			OnDemoRecordStop();
	virtual void			OnDemoPlaybackStart(char const* pDemoBaseName);
	virtual void			OnDemoPlaybackStop();

	virtual void			RecordDemoPolishUserInput(int nCmdIndex) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };

	// Cache replay ragdolls
	virtual bool			CacheReplayRagdolls(const char* pFilename, int nStartTick) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); return true; };

	// save game screenshot writing
	virtual void			WriteSaveGameScreenshotOfSize(const char* pFilename, int width, int height) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };

	// Gets the location of the player viewpoint
	virtual bool			GetPlayerView(CViewSetup& playerView);

	virtual bool			ShouldHideLoadingPlaque(void) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); return true; };

	virtual void			InvalidateMdlCache();

	virtual void			OnActiveSplitscreenPlayerChanged(int nNewSlot) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };
	virtual void			OnSplitScreenStateChanged() { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };
	virtual void			CenterStringOff() { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };;


	virtual void			OnScreenSizeChanged(int nOldWidth, int nOldHeight) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };
	virtual IMaterialProxy* InstantiateMaterialProxy(const char* proxyName) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); return 0; };

	virtual vgui::VPANEL	GetFullscreenClientDLLVPanel(void) { /*Msg("IMPLEMENT " __FUNCTION__ " !!!\n");*/ return 0; };
	virtual void			MarkEntitiesAsTouching(IClientEntity* e1, IClientEntity* e2) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };
	virtual void			OnKeyBindingChanged(ButtonCode_t buttonCode, char const* pchKeyName, char const* pchNewBinding) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };
	virtual bool			HandleGameUIEvent(const InputEvent_t& event) {/* Msg("IMPLEMENT " __FUNCTION__ " !!!\n");*/  return false; };

public:
	void PrecacheMaterial(const char* pMaterialName);

	virtual void			SetBlurFade(float scale) { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };

	virtual void			ResetHudCloseCaption() { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); };

	virtual bool			SupportsRandomMaps() { Msg("IMPLEMENT " __FUNCTION__ " !!!\n"); return true; };

private:
	void UncacheAllMaterials( );
	void ResetStringTablePointers();

	CUtlVector< IMaterial * > m_CachedMaterials;
};


CHLClient gHLClient;
IBaseClientDLL *clientdll = &gHLClient;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CHLClient, IBaseClientDLL, CLIENT_DLL_INTERFACE_VERSION, gHLClient );


//-----------------------------------------------------------------------------
// Precaches a material
//-----------------------------------------------------------------------------
void PrecacheMaterial( const char *pMaterialName )
{
	gHLClient.PrecacheMaterial( pMaterialName );
}

//-----------------------------------------------------------------------------
// Converts a previously precached material into an index
//-----------------------------------------------------------------------------
int GetMaterialIndex( const char *pMaterialName )
{
	if (pMaterialName)
	{
		int nIndex = g_pStringTableMaterials->FindStringIndex( pMaterialName );
		Assert( nIndex >= 0 );
		if (nIndex >= 0)
			return nIndex;
	}

	// This is the invalid string index
	return 0;
}

//-----------------------------------------------------------------------------
// Converts precached material indices into strings
//-----------------------------------------------------------------------------
const char *GetMaterialNameFromIndex( int nIndex )
{
	if (nIndex != (g_pStringTableMaterials->GetMaxStrings() - 1))
	{
		return g_pStringTableMaterials->GetString( nIndex );
	}
	else
	{
		return NULL;
	}
}


//-----------------------------------------------------------------------------
// Precaches a particle system
//-----------------------------------------------------------------------------
void PrecacheParticleSystem( const char *pParticleSystemName )
{
	g_pStringTableParticleEffectNames->AddString( false, pParticleSystemName );
	g_pParticleSystemMgr->PrecacheParticleSystem( pParticleSystemName );
}


//-----------------------------------------------------------------------------
// Converts a previously precached particle system into an index
//-----------------------------------------------------------------------------
int GetParticleSystemIndex( const char *pParticleSystemName )
{
	if ( pParticleSystemName )
	{
		int nIndex = g_pStringTableParticleEffectNames->FindStringIndex( pParticleSystemName );
		if ( nIndex != INVALID_STRING_INDEX )
			return nIndex;
		DevWarning("Client: Missing precache for particle system \"%s\"!\n", pParticleSystemName );
	}

	// This is the invalid string index
	return 0;
}

//-----------------------------------------------------------------------------
// Converts precached particle system indices into strings
//-----------------------------------------------------------------------------
const char *GetParticleSystemNameFromIndex( int nIndex )
{
	if ( nIndex < g_pStringTableParticleEffectNames->GetMaxStrings() )
		return g_pStringTableParticleEffectNames->GetString( nIndex );
	return "error";
}

//-----------------------------------------------------------------------------
// Returns true if host_thread_mode is set to non-zero (and engine is running in threaded mode)
//-----------------------------------------------------------------------------
bool IsEngineThreaded()
{
	if ( g_pcv_ThreadMode )
	{
		return g_pcv_ThreadMode->GetBool();
	}
	return false;
}

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------

CHLClient::CHLClient() 
{
	// Kinda bogus, but the logic in the engine is too convoluted to put it there
	g_bLevelInitialized = false;
}


extern IGameSystem *ViewportClientSystem();

int CHLClient::Connect(CreateInterfaceFn appSystemFactory, CGlobalVarsBase* pGlobals)
{
	return true;
}


//-----------------------------------------------------------------------------
ISourceVirtualReality *g_pSourceVR = NULL;
// Purpose: Called when the DLL is first loaded.
// Input  : engineFactory - 
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::Init( CreateInterfaceFn appSystemFactory, CGlobalVarsBase *pGlobals )
{
	InitCRTMemDebug();
	MathLib_Init( 2.2f, 2.2f, 0.0f, 2.0f );


#ifdef SIXENSE
	g_pSixenseInput = new SixenseInput;
#endif

	// Hook up global variables
	gpGlobals = pGlobals;

	ConnectTier1Libraries( &appSystemFactory, 1 );
	ConnectTier2Libraries( &appSystemFactory, 1 );
	ConnectTier3Libraries( &appSystemFactory, 1 );

#ifndef NO_STEAM
	ClientSteamContext().Activate();
#endif

	// We aren't happy unless we get all of our interfaces.
	// please don't collapse this into one monolithic boolean expression (impossible to debug)
	if ( (engine = (IVEngineClient *)appSystemFactory( VENGINE_CLIENT_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (modelrender = (IVModelRender *)appSystemFactory( VENGINE_HUDMODEL_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (effects = (IVEfx *)appSystemFactory( VENGINE_EFFECTS_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (enginetrace = (IEngineTrace *)appSystemFactory( INTERFACEVERSION_ENGINETRACE_CLIENT, NULL )) == NULL )
		return false;
	if ( (render = (IVRenderView *)appSystemFactory( VENGINE_RENDERVIEW_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (debugoverlay = (IVDebugOverlay *)appSystemFactory( VDEBUG_OVERLAY_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (datacache = (IDataCache*)appSystemFactory(DATACACHE_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ((mdlcache = (IMDLCache*)appSystemFactory(MDLCACHE_INTERFACE_VERSION, NULL)) == NULL)
		return false;
	if ( (modelinfo = (IVModelInfoClient *)appSystemFactory(VMODELINFO_CLIENT_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( (enginevgui = (IEngineVGui *)appSystemFactory(VENGINE_VGUI_VERSION, NULL )) == NULL )
		return false;
	if ( (networkstringtable = (INetworkStringTableContainer *)appSystemFactory(INTERFACENAME_NETWORKSTRINGTABLECLIENT,NULL)) == NULL )
		return false;
	if ( (partition = (ISpatialPartition *)appSystemFactory(INTERFACEVERSION_SPATIALPARTITION, NULL)) == NULL )
		return false;
	if ( (shadowmgr = (IShadowMgr *)appSystemFactory(ENGINE_SHADOWMGR_INTERFACE_VERSION, NULL)) == NULL )
		return false;
	if ( (staticpropmgr = (IStaticPropMgrClient *)appSystemFactory(INTERFACEVERSION_STATICPROPMGR_CLIENT, NULL)) == NULL )
		return false;
	if ( (enginesound = (IEngineSound *)appSystemFactory(IENGINESOUND_CLIENT_INTERFACE_VERSION, NULL)) == NULL )
		return false;
	if ( (filesystem = (IFileSystem *)appSystemFactory(FILESYSTEM_INTERFACE_VERSION, NULL)) == NULL )
		return false;
	if ( (random = (IUniformRandomStream *)appSystemFactory(VENGINE_CLIENT_RANDOM_INTERFACE_VERSION, NULL)) == NULL )
		return false;
	if ( (gameuifuncs = (IGameUIFuncs * )appSystemFactory( VENGINE_GAMEUIFUNCS_VERSION, NULL )) == NULL )
		return false;
	if ( (gameeventmanager = (IGameEventManager2 *)appSystemFactory(INTERFACEVERSION_GAMEEVENTSMANAGER2,NULL)) == NULL )
		return false;
	if ( (soundemitterbase = (ISoundEmitterSystemBase *)appSystemFactory(SOUNDEMITTERSYSTEM_INTERFACE_VERSION, NULL)) == NULL )
		return false;
	if ( (inputsystem = (IInputSystem *)appSystemFactory(INPUTSYSTEM_INTERFACE_VERSION, NULL)) == NULL )
		return false;
	if ( (scenefilecache = (ISceneFileCache *)appSystemFactory( SCENE_FILE_CACHE_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( IsX360() && (xboxsystem = (IXboxSystem *)appSystemFactory( XBOXSYSTEM_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( IsX360() && (matchmaking = (IMatchmaking *)appSystemFactory( VENGINE_MATCHMAKING_VERSION, NULL )) == NULL )
		return false;

#if defined( REPLAY_ENABLED )
	if ( IsPC() && (g_pEngineReplay = (IEngineReplay *)appSystemFactory( ENGINE_REPLAY_INTERFACE_VERSION, NULL )) == NULL )
		return false;
	if ( IsPC() && (g_pEngineClientReplay = (IEngineClientReplay *)appSystemFactory( ENGINE_REPLAY_CLIENT_INTERFACE_VERSION, NULL )) == NULL )
		return false;
#endif

	if ((g_pMatSystemSurface = (IMatSystemSurface*)appSystemFactory("MatSystemSurface006", NULL)) == NULL) //MAT_SYSTEM_SURFACE_INTERFACE_VERSION
		return false;
	if ((materials = g_pMaterialSystem = (IMaterialSystem*)appSystemFactory("VMaterialSystem080", NULL)) == NULL) //MATERIAL_SYSTEM_INTERFACE_VERSION
		return false;
	if ((g_pInputStackSystem = (IInputStackSystem*)appSystemFactory("InputStackSystemVersion001", NULL)) == NULL) //INPUTSTACKSYSTEM_INTERFACE_VERSION
		return false;
	if ((g_pInputSystem = (IInputSystem*)appSystemFactory(INPUTSYSTEM_INTERFACE_VERSION, NULL)) == NULL)
		return false;


#ifdef WORKSHOP_IMPORT_ENABLED
	if ( !ConnectDataModel( appSystemFactory ) )
		return false;
	if ( InitDataModel() != INIT_OK )
		return false;
	InitFbx();
#endif

	// it's ok if this is NULL. That just means the sourcevr.dll wasn't found
	g_pSourceVR = (ISourceVirtualReality *)appSystemFactory(SOURCE_VIRTUAL_REALITY_INTERFACE_VERSION, NULL);

	factorylist_t factories;
	factories.appSystemFactory = appSystemFactory;
	factories.physicsFactory = appSystemFactory;
	FactoryList_Store( factories );

	// Yes, both the client and game .dlls will try to Connect, the soundemittersystem.dll will handle this gracefully
	if ( !soundemitterbase->Connect( appSystemFactory ) )
	{
		return false;
	}

	if ( CommandLine()->FindParm( "-textmode" ) )
		g_bTextMode = true;

	if ( CommandLine()->FindParm( "-makedevshots" ) )
		g_MakingDevShots = true;

	// Not fatal if the material system stub isn't around.
	materials_stub = (IMaterialSystemStub*)appSystemFactory( MATERIAL_SYSTEM_STUB_INTERFACE_VERSION, NULL );


	if ((g_pMaterialSystemHardwareConfig = (IMaterialSystemHardwareConfig*)appSystemFactory("MaterialSystemHardwareConfig013", NULL)) == NULL) //MATERIALSYSTEM_HARDWARECONFIG_INTERFACE_VERSION
		return false;

	// Hook up the gaussian random number generator
	s_GaussianRandomStream.AttachToStream( random );

	// Initialize the console variables.
	ConVar_Register( FCVAR_CLIENTDLL );

	g_pcv_ThreadMode = g_pCVar->FindVar( "host_thread_mode" );

	if (!Initializer::InitializeAllObjects())
		return false;

	//if (!ParticleMgr()->Init(MAX_TOTAL_PARTICLES, materials))
	//	return false;

	Msg("SiZEOF: %i\n", sizeof(ConVar));
	if (!VGui_Startup( appSystemFactory ))
		return false;

	vgui::VGui_InitMatSysInterfacesList( "ClientDLL", &appSystemFactory, 1 );

	// Add the client systems.	
	
	// Client Leaf System has to be initialized first, since DetailObjectSystem uses it
	IGameSystem::Add( GameStringSystem() );
	IGameSystem::Add( SoundEmitterSystem() );
	IGameSystem::Add( ToolFrameworkClientSystem() );
	IGameSystem::Add( ClientLeafSystem() );
	IGameSystem::Add( DetailObjectSystem() );
	IGameSystem::Add( ViewportClientSystem() );
	IGameSystem::Add( ClientEffectPrecacheSystem() );
	//IGameSystem::Add( g_pClientShadowMgr );
	IGameSystem::Add( g_pColorCorrectionMgr );	// NOTE: This must happen prior to ClientThinkList (color correction is updated there)
	IGameSystem::Add( ClientThinkList() );
	IGameSystem::Add( ClientSoundscapeSystem() );
	IGameSystem::Add( PerfVisualBenchmark() );
	IGameSystem::Add( MumbleSystem() );

#ifdef SDK2013CE
	ApplyShaderConstantHack();
#endif

#if defined( TF_CLIENT_DLL )
	IGameSystem::Add( CustomTextureToolCacheGameSystem() );
	IGameSystem::Add( TFSharedContentManager() );
	#endif

#if defined( TF_CLIENT_DLL )
	if ( g_AbuseReportMgr != NULL )
	{
		IGameSystem::Add( g_AbuseReportMgr );
	}
#endif

#if defined( CLIENT_DLL ) && defined( COPY_CHECK_STRESSTEST )
	IGameSystem::Add( GetPredictionCopyTester() );
#endif

	modemanager->Init( );

	g_pClientMode->InitViewport();

	gHUD.Init();

	g_pClientMode->Init();

	if ( !IGameSystem::InitAllSystems() )
		return false;

	g_pClientMode->Enable();

	if ( !view )
	{
		view = ( IViewRender * )&g_DefaultViewRender;
	}

	view->Init();
	vieweffects->Init();

	C_BaseTempEntity::PrecacheTempEnts();

	input->Init_All();

	VGui_CreateGlobalPanels();

	InitSmokeFogOverlay();

	// Register user messages..
	CUserMessageRegister::RegisterAll();

	//ClientVoiceMgr_Init();

	// Embed voice status icons inside chat element
	//{
	//	vgui::VPANEL parent = enginevgui->GetPanel( PANEL_CLIENTDLL );
	//	GetClientVoiceMgr()->Init( &g_VoiceStatusHelper, parent );
	//}

	if ( !PhysicsDLLInit(appSystemFactory) )
		return false;

	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetEntitySaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetPhysSaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->AddBlockHandler( GetViewEffectsRestoreBlockHandler() );

	ClientWorldFactoryInit();

	C_BaseAnimating::InitBoneSetupThreadPool();

#if defined( WIN32 ) && !defined( _X360 )
	// NVNT connect haptics sytem
	ConnectHaptics(appSystemFactory);
#endif
#ifndef _X360
	HookHapticMessages(); // Always hook the messages
#endif
	
	CSysModule* pSquirrelDLL = g_pFullFileSystem->LoadModule("squirrel", "GAMEBIN", false);
	if (pSquirrelDLL != nullptr)
	{
		CreateInterfaceFn squirrelFactory = Sys_GetFactory(pSquirrelDLL);
		if (squirrelFactory != nullptr)
		{
			g_pSquirrel = (ISquirrel*)squirrelFactory(INTERFACESQUIRREL_VERSION, NULL);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Called after client & server DLL are loaded and all systems initialized
//-----------------------------------------------------------------------------
void CHLClient::PostInit()
{
	IGameSystem::PostInitAllSystems();

#ifdef SIXENSE
	// allow sixnese input to perform post-init operations
	g_pSixenseInput->PostInit();
#endif

	g_ClientVirtualReality.StartupComplete();

#ifdef HL1MP_CLIENT_DLL
	if ( s_cl_load_hl1_content.GetBool() && steamapicontext && steamapicontext->SteamApps() )
	{
		char szPath[ MAX_PATH*2 ];
		int ccFolder= steamapicontext->SteamApps()->GetAppInstallDir( 280, szPath, sizeof(szPath) );
		if ( ccFolder > 0 )
		{
			V_AppendSlash( szPath, sizeof(szPath) );
			V_strncat( szPath, "hl1", sizeof( szPath ) );

			g_pFullFileSystem->AddSearchPath( szPath, "HL1" );
			g_pFullFileSystem->AddSearchPath( szPath, "GAME" );
		}
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called when the client .dll is being dismissed
//-----------------------------------------------------------------------------
void CHLClient::Shutdown( void )
{
    if (g_pAchievementsAndStatsInterface)
    {
        g_pAchievementsAndStatsInterface->ReleasePanel();
    }

#ifdef SIXENSE
	g_pSixenseInput->Shutdown();
	delete g_pSixenseInput;
	g_pSixenseInput = NULL;
#endif

	C_BaseAnimating::ShutdownBoneSetupThreadPool();
	ClientWorldFactoryShutdown();

	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetViewEffectsRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetPhysSaveRestoreBlockHandler() );
	g_pGameSaveRestoreBlockSet->RemoveBlockHandler( GetEntitySaveRestoreBlockHandler() );

	ClientVoiceMgr_Shutdown();

	Initializer::FreeAllObjects();

	g_pClientMode->Disable();
	g_pClientMode->Shutdown();

	input->Shutdown_All();
	C_BaseTempEntity::ClearDynamicTempEnts();
	TermSmokeFogOverlay();
	view->Shutdown();
	g_pParticleSystemMgr->UncacheAllParticleSystems();
	UncacheAllMaterials();

	IGameSystem::ShutdownAllSystems();
	
	gHUD.Shutdown();
	VGui_Shutdown();
	
	ParticleMgr()->Term();
	
	ClearKeyValuesCache();

#ifndef NO_STEAM
	ClientSteamContext().Shutdown();
#endif

#ifdef WORKSHOP_IMPORT_ENABLED
	ShutdownDataModel();
	DisconnectDataModel();
	ShutdownFbx();
#endif
	
	// This call disconnects the VGui libraries which we rely on later in the shutdown path, so don't do it
//	DisconnectTier3Libraries( );
	DisconnectTier2Libraries( );
	ConVar_Unregister();
	DisconnectTier1Libraries( );

	gameeventmanager = NULL;

#if defined( WIN32 ) && !defined( _X360 )
	// NVNT Disconnect haptics system
	DisconnectHaptics();
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//  Called when the game initializes
//  and whenever the vid_mode is changed
//  so the HUD can reinitialize itself.
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::HudVidInit( void )
{
	gHUD.VidInit();

	GetClientVoiceMgr()->VidInit();

	

	return 1;
}

//-----------------------------------------------------------------------------
// Method used to allow the client to filter input messages before the 
// move record is transmitted to the server
//-----------------------------------------------------------------------------
void CHLClient::HudProcessInput( bool bActive )
{
	g_pClientMode->ProcessInput( bActive );
}

//-----------------------------------------------------------------------------
// Purpose: Called when shared data gets changed, allows dll to modify data
// Input  : bActive - 
//-----------------------------------------------------------------------------
void CHLClient::HudUpdate( bool bActive )
{
	float frametime = gpGlobals->frametime;

#if defined( TF_CLIENT_DLL )
	CRTime::UpdateRealTime();
#endif

	GetClientVoiceMgr()->Frame( frametime );

	gHUD.UpdateHud( bActive );

	{
		C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, false ); 
		IGameSystem::UpdateAllSystems( frametime );
	}

	// run vgui animations
	vgui::GetAnimationController()->UpdateAnimations( Plat_FloatTime() );

	hudlcd->SetGlobalStat( "(time_int)", VarArgs( "%d", (int)gpGlobals->curtime ) );
	hudlcd->SetGlobalStat( "(time_float)", VarArgs( "%.2f", gpGlobals->curtime ) );

	// I don't think this is necessary any longer, but I will leave it until
	// I can check into this further.
	C_BaseTempEntity::CheckDynamicTempEnts();

#ifdef SIXENSE
	// If we're not connected, update sixense so we can move the mouse cursor when in the menus
	if( !engine->IsConnected() || engine->IsPaused() )
	{
		g_pSixenseInput->SixenseFrame( 0, NULL ); 
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Called to restore to "non"HUD state.
//-----------------------------------------------------------------------------
void CHLClient::HudReset( void )
{
	gHUD.VidInit();
	PhysicsReset();
}

//-----------------------------------------------------------------------------
// Purpose: Called to add hud text message
//-----------------------------------------------------------------------------
void CHLClient::HudText( const char * message )
{
	DispatchHudText( message );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Output : ClientClass
//-----------------------------------------------------------------------------
ClientClass *CHLClient::GetAllClasses( void )
{
	return g_pClientClassHead;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_ActivateMouse( void )
{
	input->ActivateMouse();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_DeactivateMouse( void )
{
	input->DeactivateMouse();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_Accumulate ( void )
{
	input->AccumulateMouse(0);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::IN_ClearStates ( void )
{
	input->ClearStates();
}

//-----------------------------------------------------------------------------
// Purpose: Engine can query for particular keys
// Input  : *name - 
//-----------------------------------------------------------------------------
bool CHLClient::IN_IsKeyDown( const char *name, bool& isdown )
{
	kbutton_t *key = input->FindKey( name );
	if ( !key )
	{
		return false;
	}
	
	isdown = ( key->state & 1 ) ? true : false;

	// Found the key by name
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Engine can issue a key event
// Input  : eventcode - 
//			keynum - 
//			*pszCurrentBinding - 
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::IN_KeyEvent( int eventcode, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	return input->KeyEvent( eventcode, keynum, pszCurrentBinding );
}

void CHLClient::ExtraMouseSample( float frametime, bool active )
{
	Assert( C_BaseEntity::IsAbsRecomputationsEnabled() );
	Assert( C_BaseEntity::IsAbsQueriesValid() );

	C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, false ); 

	MDLCACHE_CRITICAL_SECTION();
	input->ExtraMouseSample( frametime, active );
}

void CHLClient::IN_SetSampleTime( float frametime )
{
	input->Joystick_SetSampleTime( frametime );
	input->IN_SetSampleTime( frametime );

#ifdef SIXENSE
	g_pSixenseInput->ResetFrameTime( frametime );
#endif
}
//-----------------------------------------------------------------------------
// Purpose: Fills in usercmd_s structure based on current view angles and key/controller inputs
// Input  : frametime - timestamp for last frame
//			*cmd - the command to fill in
//			active - whether the user is fully connected to a server
//-----------------------------------------------------------------------------
void CHLClient::CreateMove ( int sequence_number, float input_sample_frametime, bool active )
{

	Assert( C_BaseEntity::IsAbsRecomputationsEnabled() );
	Assert( C_BaseEntity::IsAbsQueriesValid() );

	C_BaseAnimating::AutoAllowBoneAccess boneaccess( true, false ); 

	MDLCACHE_CRITICAL_SECTION();
	input->CreateMove( sequence_number, input_sample_frametime, active );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHLClient::View_Render( vrect_t *rect )
{
	VPROF( "View_Render" );

	// UNDONE: This gets hit at startup sometimes, investigate - will cause NaNs in calcs inside Render()
	if ( rect->width == 0 || rect->height == 0 )
		return;

	view->Render( rect );
	UpdatePerfStats();
}


//-----------------------------------------------------------------------------
// Gets the location of the player viewpoint
//-----------------------------------------------------------------------------
bool CHLClient::GetPlayerView( CViewSetup &playerView )
{
	playerView = *view->GetPlayerViewSetup();
	return true;
}

//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
void CHLClient::InvalidateMdlCache()
{
	C_BaseAnimating *pAnimating;
	for ( C_BaseEntity *pEntity = ClientEntityList().FirstBaseEntity(); pEntity; pEntity = ClientEntityList().NextBaseEntity(pEntity) )
	{
		pAnimating = dynamic_cast<C_BaseAnimating *>(pEntity);
		if ( pAnimating )
		{
			pAnimating->InvalidateMdlCache();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSF - 
//-----------------------------------------------------------------------------
void CHLClient::View_Fade( ScreenFade_t *pSF )
{
	if ( pSF != NULL )
		vieweffects->Fade( *pSF );
}

int Squirrel_GetConvar(SquirrelScript script)
{
	const char* convarname;
	if (!g_pSquirrel->GetArgs(script, "s", &convarname))
	{
		return 0;
	}
	ConVarRef conv(convarname);
	SquirrelValue ret;
	ret.type = SQUIRREL_STRING;
	ret.val_string = conv.GetString();
	g_pSquirrel->PushValue(script, ret);
	return 1;
}

int Squirrel_PrintToClient(SquirrelScript script)
{
	const char* toprint;
	if (!g_pSquirrel->GetArgs(script, "s", &toprint))
	{
		return 0;
	}
	Msg(toprint);
	return 0;
}

int Squirrel_ExecuteConsoleCommand(SquirrelScript script)
{
	const char* cmd;
	if (!g_pSquirrel->GetArgs(script, "s", &cmd))
	{
		return 0;
	}
	engine->ClientCmd(cmd);
	return 0;
}

int Squirrel_VGUICreatePanel(SquirrelScript script)
{
	const char* userdata;
	if (!g_pSquirrel->GetArgs(script, "s", &userdata))
	{
		return 0;
	}
	vgui::Panel* viewport = g_pClientMode->GetViewport();
	vgui::SquirrelPanel* panel = new vgui::SquirrelPanel(script,userdata);
	SquirrelHandle hand = NewSquirrelHandle(SP_VGUI, panel);
	panel->handle = hand;
	panel->SetParent(viewport);
	panel->SetPaintBackgroundEnabled(true);
	panel->SetPaintBackgroundType(2);
	panel->SetAlpha(255);
	panel->SetVisible(true);
	panel->SetEnabled(true);
	SquirrelValue ret;
	ret.type = SQUIRREL_USERDATA;
	ret.val_userdata = hand;
	g_pSquirrel->PushValue(script, ret);
	return 1;
}

int Squirrel_VGUISetBounds(SquirrelScript script)
{
	SquirrelHandle hand;
	int x, y, w, t;
	if (!g_pSquirrel->GetArgs(script, "uiiii", &hand,&x,&y,&w,&t))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetBounds(x, y, w, t);
	return 0;
}

int Squirrel_VGUIRequestFocus(SquirrelScript script)
{
	SquirrelHandle hand;
	if (!g_pSquirrel->GetArgs(script, "u", &hand))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->RequestFocus();
	return 0;
}

int Squirrel_VGUIMakePopup(SquirrelScript script)
{
	SquirrelHandle hand;
	if (!g_pSquirrel->GetArgs(script, "u", &hand))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->MakePopup();
	return 0;
}

int Squirrel_VGUISetParent(SquirrelScript script)
{
	SquirrelHandle hand, parent;
	if (!g_pSquirrel->GetArgs(script, "uu", &hand, &parent))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	void* parentptr = CheckSquirrelHandle(parent, SP_VGUI);
	if (!ptr || !parentptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetParent((vgui::SquirrelPanel*)(parentptr));
	return 0;
}

int Squirrel_VGUISetPaintFunction(SquirrelScript script)
{
	SquirrelHandle hand;
	const char* func;
	if (!g_pSquirrel->GetArgs(script, "us", &hand, &func))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetPaintFunction(func);
	return 0;
}

int Squirrel_SurfaceDrawLine(SquirrelScript script)
{
	int x1, y1, x2, y2;
	if (!g_pSquirrel->GetArgs(script, "iiii", &x1, &y1,&x2,&y2))
	{
		return 0;
	}
	vgui::surface()->DrawLine(x1, y1, x2, y2);
	return 0;
}

int Squirrel_SurfaceDrawRect(SquirrelScript script)
{
	int x1, y1, x2, y2;
	if (!g_pSquirrel->GetArgs(script, "iiii", &x1, &y1, &x2, &y2))
	{
		return 0;
	}
	vgui::surface()->DrawOutlinedRect(x1, y1, x2, y2);
	return 0;
}

int Squirrel_SurfaceFillRect(SquirrelScript script)
{
	int x1, y1, x2, y2;
	if (!g_pSquirrel->GetArgs(script, "iiii", &x1, &y1, &x2, &y2))
	{
		return 0;
	}
	vgui::surface()->DrawFilledRect(x1, y1, x2, y2);
	return 0;
}


int Squirrel_SurfaceSetColor(SquirrelScript script)
{
	int r, g, b, a;
	if (!g_pSquirrel->GetArgs(script, "iiii", &r, &g,&b,&a))
	{
		return 0;
	}
	vgui::surface()->DrawSetColor(r,g,b,a);
	return 0;
}

int Squirrel_SurfaceDrawText(SquirrelScript script)
{
	const char* text;
	wchar wtext[1024];
	if (!g_pSquirrel->GetArgs(script, "s", &text))
	{
		return 0;
	}
	Q_strtowcs(text, -1, wtext, 1024);
	vgui::surface()->DrawPrintText(wtext,wcslen(wtext));
	return 0;
}

int Squirrel_SurfaceSetTextPos(SquirrelScript script)
{
	int x, y;
	if (!g_pSquirrel->GetArgs(script, "ii", &x, &y))
	{
		return 0;
	}
	vgui::surface()->DrawSetTextPos(x, y);
	return 0;
}


int Squirrel_SurfaceSetTextColor(SquirrelScript script)
{
	int r, g, b, a;
	if (!g_pSquirrel->GetArgs(script, "iiii", &r,&g,&b,&a))
	{
		return 0;
	}
	vgui::surface()->DrawSetTextColor(r, g, b, a);
	return 0;
}

int Squirrel_SurfaceSetTextFont(SquirrelScript script)
{
	const char* scheme, *fontname;
	if (!g_pSquirrel->GetArgs(script, "ss", &scheme, &fontname))
	{
		return 0;
	}
	vgui::surface()->DrawSetTextFont(vgui::scheme()->GetIScheme(vgui::scheme()->GetScheme(scheme))->GetFont(fontname));
	return 0;
}

int Squirrel_VGUISetKeyBoardInputEnabled(SquirrelScript script)
{
	SquirrelHandle hand;
	bool enabled;
	if (!g_pSquirrel->GetArgs(script, "ub", &hand, &enabled))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetKeyBoardInputEnabled(enabled);
	return 0;
}

int Squirrel_VGUISetMouseInputEnabled(SquirrelScript script)
{
	SquirrelHandle hand;
	bool enabled;
	if (!g_pSquirrel->GetArgs(script, "ub", &hand, &enabled))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetMouseInputEnabled(enabled);
	return 0;
}

int Squirrel_VGUISetVisible(SquirrelScript script)
{
	SquirrelHandle hand;
	bool enabled;
	if (!g_pSquirrel->GetArgs(script, "ub", &hand, &enabled))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetVisible(enabled);
	return 0;
}


int Squirrel_VGUISetOnMouseDownFunction(SquirrelScript script)
{
	SquirrelHandle hand;
	const char* func;
	if (!g_pSquirrel->GetArgs(script, "us", &hand, &func))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetOnMouseDownFunction(func);
	return 0;
}

int Squirrel_VGUISetOnMouseUpFunction(SquirrelScript script)
{
	SquirrelHandle hand;
	const char* func;
	if (!g_pSquirrel->GetArgs(script, "us", &hand, &func))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetOnMouseUpFunction(func);
	return 0;
}

int Squirrel_VGUISetOnKeyDownFunction(SquirrelScript script)
{
	SquirrelHandle hand;
	const char* func;
	if (!g_pSquirrel->GetArgs(script, "us", &hand, &func))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetOnKeyDownFunction(func);
	return 0;
}


int Squirrel_VGUISetOnKeyUpFunction(SquirrelScript script)
{
	SquirrelHandle hand;
	const char* func;
	if (!g_pSquirrel->GetArgs(script, "us", &hand, &func))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetOnKeyUpFunction(func);
	return 0;
}

int Squirrel_VGUISetPosition(SquirrelScript script)
{
	SquirrelHandle hand;
	int x, y;
	if (!g_pSquirrel->GetArgs(script, "uii", &hand, &x, &y))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	((vgui::SquirrelPanel*)ptr)->SetPos(x, y);
	return 0;
}

int Squirrel_VGUIGetUserdata(SquirrelScript script)
{
	SquirrelHandle hand;
	if (!g_pSquirrel->GetArgs(script, "u", &hand))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	SquirrelValue ret;
	ret.type = SQUIRREL_STRING;
	ret.val_string = ((vgui::SquirrelPanel*)ptr)->GetUserdata();
	g_pSquirrel->PushValue(script, ret);
	return 1;
}

void Squirrel_GetModelListRecur(SquirrelScript script, FileFindHandle_t findHandle, const char* searchPath)
{
	SquirrelValue ret;
	ret.type = SQUIRREL_STRING;
	const char* pszFileName = g_pFullFileSystem->FindNext(findHandle);
	while (pszFileName)
	{
		if (pszFileName[0] == '.')
		{
			pszFileName = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}
		if (g_pFullFileSystem->FindIsDirectory(findHandle))
		{
			char newPath[MAX_PATH];
			V_strncpy(newPath, searchPath,MAX_PATH);
			V_AppendSlash(newPath, MAX_PATH);
			V_strncat(newPath, pszFileName,MAX_PATH);
			char newPathSearch[MAX_PATH];
			V_strncpy(newPathSearch, newPath, MAX_PATH);
			V_AppendSlash(newPathSearch, MAX_PATH);
			V_strncat(newPathSearch, "*.mdl", MAX_PATH);
			FileFindHandle_t newhandle;
			pszFileName = g_pFullFileSystem->FindFirst(newPathSearch, &newhandle);
			Squirrel_GetModelListRecur(script, newhandle, newPath);
			g_pFullFileSystem->FindClose(newhandle);
			continue;
		}
		char pFilePath[MAX_PATH];
		V_strncpy(pFilePath, searchPath,MAX_PATH);
		V_AppendSlash(pFilePath, MAX_PATH);
		strncat(pFilePath, pszFileName, MAX_PATH);
		ret.val_string = pFilePath;
		g_pSquirrel->PushValue(script, ret);
		g_pSquirrel->AppendToArray(script);
		pszFileName = g_pFullFileSystem->FindNext(findHandle);
	}
}

int Squirrel_GetModelList(SquirrelScript script)
{

	g_pSquirrel->PushArray(script);
	FileFindHandle_t newhandle;
	const char* pszFileName = g_pFullFileSystem->FindFirst("models/*.mdl", &newhandle);
	if (pszFileName && newhandle)
	{
		SquirrelValue ret;
		ret.type = SQUIRREL_STRING;
		char pFilePath[MAX_PATH];
		V_strncpy(pFilePath, "models/", MAX_PATH);
		strncat(pFilePath, pszFileName, MAX_PATH);
		ret.val_string = pFilePath;
		g_pSquirrel->PushValue(script, ret);
		g_pSquirrel->AppendToArray(script);
		Squirrel_GetModelListRecur(script, newhandle, "models");
	}
	
	return 1;
}


int Squirrel_VGUIGetSize(SquirrelScript script)
{
	SquirrelHandle hand;
	if (!g_pSquirrel->GetArgs(script, "u", &hand))
	{
		return 0;
	}
	void* ptr = CheckSquirrelHandle(hand, SP_VGUI);
	if (!ptr)
	{
		return 0;
	}
	int wide, tall;
	((vgui::SquirrelPanel*)ptr)->GetSize(wide, tall);
	SquirrelValue ret;
	ret.type = SQUIRREL_INT;
	ret.val_int = wide;
	g_pSquirrel->PushArray(script);
	g_pSquirrel->PushValue(script, ret);
	g_pSquirrel->AppendToArray(script);
	ret.val_int = tall;
	g_pSquirrel->PushValue(script, ret);
	g_pSquirrel->AppendToArray(script);
	return 1;
}


void LoadMod(const char* path)
{
	int len = strlen(path);
	if (len < 4 || !(path[len - 4] == '.' && path[len - 3] == 'n' && path[len - 2] == 'u' && path[len - 1] == 't'))
		return;

	CUtlBuffer codebuffer;

	codebuffer.Clear();

	if (g_pFullFileSystem->ReadFile(path, NULL, codebuffer))
	{
		SquirrelScript script = g_pSquirrel->LoadScript((const char*)(codebuffer.Base()));
		if (!script)
		{
			return;
		}
		g_pSquirrel->AddFunction(script, "ExecuteConsoleCommand", Squirrel_ExecuteConsoleCommand);
		g_pSquirrel->AddFunction(script, "GetConvar", Squirrel_GetConvar);
		g_pSquirrel->AddFunction(script, "PrintToClient", Squirrel_PrintToClient);
		g_pSquirrel->AddFunction(script, "VGUICreatePanel", Squirrel_VGUICreatePanel);
		g_pSquirrel->AddFunction(script, "VGUISetBounds", Squirrel_VGUISetBounds);
		g_pSquirrel->AddFunction(script, "VGUISetParent", Squirrel_VGUISetParent);
		g_pSquirrel->AddFunction(script, "VGUISetPaintFunction", Squirrel_VGUISetPaintFunction);
		g_pSquirrel->AddFunction(script, "VGUIRequestFocus", Squirrel_VGUIRequestFocus);
		g_pSquirrel->AddFunction(script, "VGUIMakePopup", Squirrel_VGUIMakePopup);
		g_pSquirrel->AddFunction(script, "SurfaceDrawLine", Squirrel_SurfaceDrawLine);
		g_pSquirrel->AddFunction(script, "SurfaceSetColor", Squirrel_SurfaceSetColor);
		g_pSquirrel->AddFunction(script, "SurfaceDrawText", Squirrel_SurfaceDrawText);
		g_pSquirrel->AddFunction(script, "SurfaceDrawRect", Squirrel_SurfaceDrawRect);
		g_pSquirrel->AddFunction(script, "SurfaceFillRect", Squirrel_SurfaceFillRect);
		g_pSquirrel->AddFunction(script, "SurfaceSetTextColor", Squirrel_SurfaceSetTextColor);
		g_pSquirrel->AddFunction(script, "SurfaceSetTextPos", Squirrel_SurfaceSetTextPos);
		g_pSquirrel->AddFunction(script, "SurfaceSetTextFont", Squirrel_SurfaceSetTextFont);
		g_pSquirrel->AddFunction(script, "VGUISetOnMouseDownFunction", Squirrel_VGUISetOnMouseDownFunction);
		g_pSquirrel->AddFunction(script, "VGUISetOnMouseUpFunction", Squirrel_VGUISetOnMouseUpFunction);
		g_pSquirrel->AddFunction(script, "VGUISetOnKeyDownFunction", Squirrel_VGUISetOnKeyDownFunction);
		g_pSquirrel->AddFunction(script, "VGUISetOnKeyUpFunction", Squirrel_VGUISetOnKeyUpFunction);
		g_pSquirrel->AddFunction(script, "VGUISetKeyBoardInputEnabled", Squirrel_VGUISetKeyBoardInputEnabled);
		g_pSquirrel->AddFunction(script, "VGUISetMouseInputEnabled", Squirrel_VGUISetMouseInputEnabled);
		g_pSquirrel->AddFunction(script, "VGUISetVisible", Squirrel_VGUISetVisible);
		g_pSquirrel->AddFunction(script, "VGUISetPosition", Squirrel_VGUISetPosition);
		g_pSquirrel->AddFunction(script, "VGUIGetUserdata", Squirrel_VGUIGetUserdata);
		g_pSquirrel->AddFunction(script, "VGUIGetSize", Squirrel_VGUIGetSize);
		g_pSquirrel->AddFunction(script, "GetModelList", Squirrel_GetModelList);


		SquirrelValue returned = g_pSquirrel->CallFunction(script, "OnModStart", "");
		switch (returned.type)
		{
		case SQUIRREL_INT:
			Msg("Squirrel %s returned int : %i\n", path, returned.val_int);
			break;
		case SQUIRREL_BOOL:
			Msg("Squirrel %s returned bool : %s\n", path, returned.val_bool ? "true" : "false");
			break;
		case SQUIRREL_FLOAT:
			Msg("Squirrel %s returned float : %f\n", path, returned.val_float);
			break;
		case SQUIRREL_STRING:
			Msg("Squirrel %s returned string : %s\n", path, returned.val_string);
			break;
		default:
			Msg("Squirrel %s failed to execute/return a value\n",path);
			break;
		}
		squirrelscripts.AddToTail(script);
	}
}

void LoadFilesInDirectory(const char* modname, const char* folder, const char* filename)
{
	FileFindHandle_t findHandle;
	char searchPath[MAX_PATH];
	strcpy(searchPath, "mods/");
	strncat(searchPath, folder, MAX_PATH);
	strncat(searchPath, "/*", MAX_PATH);
	const char* pszFileName = g_pFullFileSystem->FindFirst(searchPath, &findHandle);
	char pszFileNameNoExt[MAX_PATH];
	while (pszFileName)
	{
		if (pszFileName[0] == '.')
		{
			pszFileName = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}
		if (g_pFullFileSystem->FindIsDirectory(findHandle))
		{
			pszFileName = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}
		V_StripExtension(pszFileName, pszFileNameNoExt, MAX_PATH);
		if (V_strcmp(filename, pszFileNameNoExt) == 0)
		{
			char pFilePath[MAX_PATH];
			strcpy(pFilePath, "mods/");
			strncat(pFilePath, folder, MAX_PATH);
			V_AppendSlash(pFilePath, MAX_PATH);
			strncat(pFilePath, pszFileName, MAX_PATH);
			LoadMod(pFilePath);
		}
		pszFileName = g_pFullFileSystem->FindNext(findHandle);
	}
	g_pFullFileSystem->FindClose(findHandle);
}



//-----------------------------------------------------------------------------
// Purpose: Per level init
//-----------------------------------------------------------------------------
void CHLClient::LevelInitPreEntity( char const* pMapName )
{
	// HACK: Bogus, but the logic is too complicated in the engine
	if (g_bLevelInitialized)
		return;
	g_bLevelInitialized = true;

	input->LevelInit();

	vieweffects->LevelInit();
	
	//Tony; loadup per-map manifests.
	ParseParticleEffectsMap( pMapName, true );
	
	// Tell mode manager that map is changing
	modemanager->LevelInit( pMapName );
	ParticleMgr()->LevelInit();

	hudlcd->SetGlobalStat( "(mapname)", pMapName );

	C_BaseTempEntity::ClearDynamicTempEnts();
	clienteffects->Flush();
	view->LevelInit();
	tempents->LevelInit();
	ResetToneMapping(1.0);

	IGameSystem::LevelInitPreEntityAllSystems(pMapName);

#ifdef USES_ECON_ITEMS
	GameItemSchema_t *pItemSchema = ItemSystem()->GetItemSchema();
	if ( pItemSchema )
	{
		pItemSchema->BInitFromDelayedBuffer();
	}
#endif // USES_ECON_ITEMS

	ResetWindspeed();

#if !defined( NO_ENTITY_PREDICTION )
	// don't do prediction if single player!
	// don't set direct because of FCVAR_USERINFO
	if ( gpGlobals->maxClients > 1 )
	{
		if ( !cl_predict->GetInt() )
		{
			engine->ClientCmd( "cl_predict 1" );
		}
	}
	else
	{
		if ( cl_predict->GetInt() )
		{
			engine->ClientCmd( "cl_predict 0" );
		}
	}
#endif

	// Check low violence settings for this map
	g_RagdollLVManager.SetLowViolence( pMapName );

	gHUD.LevelInit();

#if defined( REPLAY_ENABLED )
	// Initialize replay ragdoll recorder
	if ( !engine->IsPlayingDemo() )
	{
		CReplayRagdollRecorder::Instance().Init();
	}
#endif
	if (squirrelhandles[SP_VGUI])
	{
		for (unsigned int i = 0; i < squirrelhandles[SP_VGUI]->Count(); i++)
		{
			((vgui::SquirrelPanel*)squirrelhandles[SP_VGUI]->Element(i))->MarkForDeletion();
		}
		squirrelhandles[SP_VGUI]->RemoveAll();
		squirrelhandleid[SP_VGUI] = 0;
	}
	for (int i = 0; i < squirrelscripts.Count(); ++i)
	{
		g_pSquirrel->ShutdownScript(squirrelscripts[i]);
	}
	
	squirrelscripts.RemoveAll();

	FileFindHandle_t findHandle;
	const char* pszFileName = g_pFullFileSystem->FindFirst("mods/*", &findHandle);
	while (pszFileName)
	{
		if (pszFileName[0] == '.')
		{
			pszFileName = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}
		if (g_pFullFileSystem->FindIsDirectory(findHandle))
		{
			LoadFilesInDirectory(pszFileName, pszFileName, "cl_main");
			pszFileName = g_pFullFileSystem->FindNext(findHandle);
			continue;
		}
		pszFileName = g_pFullFileSystem->FindNext(findHandle);
	}
	
	for (int i = 0; i < squirrelscripts.Count(); i++)
	{
		g_pSquirrel->CallFunction(squirrelscripts[i], "LevelInitPreEntity", "");
	}
}


//-----------------------------------------------------------------------------
// Purpose: Per level init
//-----------------------------------------------------------------------------
void CHLClient::LevelInitPostEntity( )
{
	IGameSystem::LevelInitPostEntityAllSystems();
	C_PhysPropClientside::RecreateAll();
	internalCenterPrint->Clear();
}

//-----------------------------------------------------------------------------
// Purpose: Reset our global string table pointers
//-----------------------------------------------------------------------------
void CHLClient::ResetStringTablePointers()
{
	g_pStringTableParticleEffectNames = NULL;
	g_StringTableEffectDispatch = NULL;
	g_StringTableVguiScreen = NULL;
	g_pStringTableMaterials = NULL;
	g_pStringTableInfoPanel = NULL;
	g_pStringTableClientSideChoreoScenes = NULL;
	g_pStringTableServerMapCycle = NULL;

#ifdef TF_CLIENT_DLL
	g_pStringTableServerPopFiles = NULL;
	g_pStringTableServerMapCycleMvM = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Per level de-init
//-----------------------------------------------------------------------------
void CHLClient::LevelShutdown( void )
{
	// HACK: Bogus, but the logic is too complicated in the engine
	if (!g_bLevelInitialized)
		return;

	g_bLevelInitialized = false;

	// Disable abs recomputations when everything is shutting down
	CBaseEntity::EnableAbsRecomputations( false );

	// Level shutdown sequence.
	// First do the pre-entity shutdown of all systems
	IGameSystem::LevelShutdownPreEntityAllSystems();

	C_PhysPropClientside::DestroyAll();

	modemanager->LevelShutdown();

	// Remove temporary entities before removing entities from the client entity list so that the te_* may
	// clean up before hand.
	tempents->LevelShutdown();

	// Now release/delete the entities
	cl_entitylist->Release();

	C_BaseEntityClassList *pClassList = s_pClassLists;
	while ( pClassList )
	{
		pClassList->LevelShutdown();
		pClassList = pClassList->m_pNextClassList;
	}

	// Now do the post-entity shutdown of all systems
	IGameSystem::LevelShutdownPostEntityAllSystems();

	view->LevelShutdown();
	beams->ClearBeams();
	ParticleMgr()->RemoveAllEffects();
	
	StopAllRumbleEffects();

	gHUD.LevelShutdown();

	internalCenterPrint->Clear();

	messagechars->Clear();

#ifndef TF_CLIENT_DLL
	// don't want to do this for TF2 because we have particle systems in our
	// character loadout screen that can be viewed when we're not connected to a server
	g_pParticleSystemMgr->UncacheAllParticleSystems();
#endif
	UncacheAllMaterials();

#ifdef _XBOX
	ReleaseRenderTargets();
#endif

	// string tables are cleared on disconnect from a server, so reset our global pointers to NULL
	ResetStringTablePointers();

#if defined( REPLAY_ENABLED )
	// Shutdown the ragdoll recorder
	CReplayRagdollRecorder::Instance().Shutdown();
	CReplayRagdollCache::Instance().Shutdown();
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Engine received crosshair offset ( autoaim )
// Input  : angle - 
//-----------------------------------------------------------------------------
void CHLClient::SetCrosshairAngle( const QAngle& angle )
{
	CHudCrosshair *crosshair = GET_HUDELEMENT( CHudCrosshair );
	if ( crosshair )
	{
		crosshair->SetCrosshairAngle( angle );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Helper to initialize sprite from .spr semaphor
// Input  : *pSprite - 
//			*loadname - 
//-----------------------------------------------------------------------------
void CHLClient::InitSprite( CEngineSprite *pSprite, const char *loadname )
{
	if ( pSprite )
	{
		pSprite->Init( loadname );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSprite - 
//-----------------------------------------------------------------------------
void CHLClient::ShutdownSprite( CEngineSprite *pSprite )
{
	if ( pSprite )
	{
		pSprite->Shutdown();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Tells engine how much space to allocate for sprite objects
// Output : int
//-----------------------------------------------------------------------------
int CHLClient::GetSpriteSize( void ) const
{
	return sizeof( CEngineSprite );
}


//-----------------------------------------------------------------------------
// Called when the string table for materials changes
//-----------------------------------------------------------------------------
void OnMaterialStringTableChanged( void *object, INetworkStringTable *stringTable, int stringNumber, const char *newString, void const *newData )
{
	// Make sure this puppy is precached
	gHLClient.PrecacheMaterial( newString );
	RequestCacheUsedMaterials();
}


//-----------------------------------------------------------------------------
// Called when the string table for particle systems changes
//-----------------------------------------------------------------------------
void OnParticleSystemStringTableChanged( void *object, INetworkStringTable *stringTable, int stringNumber, const char *newString, void const *newData )
{
	// Make sure this puppy is precached
	//g_pParticleSystemMgr->PrecacheParticleSystem( newString );
	RequestCacheUsedMaterials();
}


//-----------------------------------------------------------------------------
// Called when the string table for VGUI changes
//-----------------------------------------------------------------------------
void OnVguiScreenTableChanged( void *object, INetworkStringTable *stringTable, int stringNumber, const char *newString, void const *newData )
{
	// Make sure this puppy is precached
	vgui::Panel *pPanel = PanelMetaClassMgr()->CreatePanelMetaClass( newString, 100, NULL, NULL );
	if ( pPanel )
		PanelMetaClassMgr()->DestroyPanelMetaClass( pPanel );
}

//-----------------------------------------------------------------------------
// Purpose: Preload the string on the client (if single player it should already be in the cache from the server!!!)
// Input  : *object - 
//			*stringTable - 
//			stringNumber - 
//			*newString - 
//			*newData - 
//-----------------------------------------------------------------------------
void OnSceneStringTableChanged( void *object, INetworkStringTable *stringTable, int stringNumber, const char *newString, void const *newData )
{
}

//-----------------------------------------------------------------------------
// Purpose: Hook up any callbacks here, the table definition has been parsed but 
//  no data has been added yet
//-----------------------------------------------------------------------------
void CHLClient::InstallStringTableCallback( const char *tableName )
{
	// Here, cache off string table IDs
	if (!Q_strcasecmp(tableName, "VguiScreen"))
	{
		// Look up the id 
		g_StringTableVguiScreen = networkstringtable->FindTable( tableName );

		// When the material list changes, we need to know immediately
		g_StringTableVguiScreen->SetStringChangedCallback( NULL, OnVguiScreenTableChanged );
	}
	else if (!Q_strcasecmp(tableName, "Materials"))
	{
		// Look up the id 
		g_pStringTableMaterials = networkstringtable->FindTable( tableName );

		// When the material list changes, we need to know immediately
		g_pStringTableMaterials->SetStringChangedCallback( NULL, OnMaterialStringTableChanged );
	}
	else if ( !Q_strcasecmp( tableName, "EffectDispatch" ) )
	{
		g_StringTableEffectDispatch = networkstringtable->FindTable( tableName );
	}
	else if ( !Q_strcasecmp( tableName, "InfoPanel" ) )
	{
		g_pStringTableInfoPanel = networkstringtable->FindTable( tableName );
	}
	else if ( !Q_strcasecmp( tableName, "Scenes" ) )
	{
		g_pStringTableClientSideChoreoScenes = networkstringtable->FindTable( tableName );
		g_pStringTableClientSideChoreoScenes->SetStringChangedCallback( NULL, OnSceneStringTableChanged );
	}
	else if ( !Q_strcasecmp( tableName, "ParticleEffectNames" ) )
	{
		g_pStringTableParticleEffectNames = networkstringtable->FindTable( tableName );
		networkstringtable->SetAllowClientSideAddString( g_pStringTableParticleEffectNames, true );
		// When the particle system list changes, we need to know immediately
		g_pStringTableParticleEffectNames->SetStringChangedCallback( NULL, OnParticleSystemStringTableChanged );
	}
	else if ( !Q_strcasecmp( tableName, "ServerMapCycle" ) )
	{
		g_pStringTableServerMapCycle = networkstringtable->FindTable( tableName );
	}
#ifdef TF_CLIENT_DLL
	else if ( !Q_strcasecmp( tableName, "ServerPopFiles" ) )
	{
		g_pStringTableServerPopFiles = networkstringtable->FindTable( tableName );
	}
	else if ( !Q_strcasecmp( tableName, "ServerMapCycleMvM" ) )
	{
		g_pStringTableServerMapCycleMvM = networkstringtable->FindTable( tableName );
	}
#endif

	InstallStringTableCallback_GameRules();
}


//-----------------------------------------------------------------------------
// Material precache
//-----------------------------------------------------------------------------
void CHLClient::PrecacheMaterial( const char *pMaterialName )
{
	Assert( pMaterialName );

	int nLen = Q_strlen( pMaterialName );
	char *pTempBuf = (char*)stackalloc( nLen + 1 );
	memcpy( pTempBuf, pMaterialName, nLen + 1 );
	char *pFound = Q_strstr( pTempBuf, ".vmt\0" );
	if ( pFound )
	{
		*pFound = 0;
	}
		
	IMaterial *pMaterial = materials->FindMaterial( pTempBuf, TEXTURE_GROUP_PRECACHED );
	if ( !IsErrorMaterial( pMaterial ) )
	{
		pMaterial->IncrementReferenceCount();
		m_CachedMaterials.AddToTail( pMaterial );
	}
	else
	{
		if (IsOSX())
		{
			printf("\n ##### CHLClient::PrecacheMaterial could not find material %s (%s)", pMaterialName, pTempBuf );
		}
	}
}

void CHLClient::UncacheAllMaterials( )
{
	for (int i = m_CachedMaterials.Count(); --i >= 0; )
	{
		m_CachedMaterials[i]->DecrementReferenceCount();
	}
	m_CachedMaterials.RemoveAll();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszName - 
//			iSize - 
//			*pbuf - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHLClient::DispatchUserMessage( int msg_type, bf_read &msg_data )
{
	return usermessages->DispatchUserMessage( msg_type, msg_data );
}


void SimulateEntities()
{
	VPROF_BUDGET("Client SimulateEntities", VPROF_BUDGETGROUP_CLIENT_SIM);

	// Service timer events (think functions).
  	ClientThinkList()->PerformThinkFunctions();

	// TODO: make an ISimulateable interface so C_BaseNetworkables can simulate?
	{
		VPROF_("C_BaseEntity::Simulate", 1, VPROF_BUDGETGROUP_CLIENT_SIM, false, BUDGETFLAG_CLIENT);
		C_BaseEntityIterator iterator;
		C_BaseEntity *pEnt;
		while ( (pEnt = iterator.Next()) != NULL )
		{
			pEnt->Simulate();
		}
	}
}


bool AddDataChangeEvent( IClientNetworkable *ent, DataUpdateType_t updateType, int *pStoredEvent )
{
	VPROF( "AddDataChangeEvent" );

	Assert( ent );
	// Make sure we don't already have an event queued for this guy.
	if ( *pStoredEvent >= 0 )
	{
		Assert( g_DataChangedEvents[*pStoredEvent].m_pEntity == ent );

		// DATA_UPDATE_CREATED always overrides DATA_UPDATE_CHANGED.
		if ( updateType == DATA_UPDATE_CREATED )
			g_DataChangedEvents[*pStoredEvent].m_UpdateType = updateType;
	
		return false;
	}
	else
	{
		*pStoredEvent = g_DataChangedEvents.AddToTail( CDataChangedEvent( ent, updateType, pStoredEvent ) );
		return true;
	}
}


void ClearDataChangedEvent( int iStoredEvent )
{
	if ( iStoredEvent != -1 )
		g_DataChangedEvents.Remove( iStoredEvent );
}


void ProcessOnDataChangedEvents()
{
	VPROF_("ProcessOnDataChangedEvents", 1, VPROF_BUDGETGROUP_CLIENT_SIM, false, BUDGETFLAG_CLIENT);
	FOR_EACH_LL( g_DataChangedEvents, i )
	{
		CDataChangedEvent *pEvent = &g_DataChangedEvents[i];

		// Reset their stored event identifier.		
		*pEvent->m_pStoredEvent = -1;

		// Send the event.
		IClientNetworkable *pNetworkable = pEvent->m_pEntity;
		pNetworkable->OnDataChanged( pEvent->m_UpdateType );
	}

	g_DataChangedEvents.Purge();
}


void UpdateClientRenderableInPVSStatus()
{
	// Vis for this view should already be setup at this point.

	// For each client-only entity, notify it if it's newly coming into the PVS.
	CUtlLinkedList<CClientEntityList::CPVSNotifyInfo,unsigned short> &theList = ClientEntityList().GetPVSNotifiers();
	FOR_EACH_LL( theList, i )
	{
		CClientEntityList::CPVSNotifyInfo *pInfo = &theList[i];

		if ( pInfo->m_InPVSStatus & INPVS_YES )
		{
			// Ok, this entity already thinks it's in the PVS. No need to notify it.
			// We need to set the INPVS_YES_THISFRAME flag if it's in this frame at all, so we 
			// don't tell the entity it's not in the PVS anymore at the end of the frame.
			if ( !( pInfo->m_InPVSStatus & INPVS_THISFRAME ) )
			{
				if ( g_pClientLeafSystem->IsRenderableInPVS( pInfo->m_pRenderable ) )
				{
					pInfo->m_InPVSStatus |= INPVS_THISFRAME;
				}
			}
		}
		else
		{
			// This entity doesn't think it's in the PVS yet. If it is now in the PVS, let it know.
			if ( g_pClientLeafSystem->IsRenderableInPVS( pInfo->m_pRenderable ) )
			{
				pInfo->m_InPVSStatus |= ( INPVS_YES | INPVS_THISFRAME | INPVS_NEEDSNOTIFY );
			}
		}
	}	
}

void UpdatePVSNotifiers()
{
	MDLCACHE_CRITICAL_SECTION();

	// At this point, all the entities that were rendered in the previous frame have INPVS_THISFRAME set
	// so we can tell the entities that aren't in the PVS anymore so.
	CUtlLinkedList<CClientEntityList::CPVSNotifyInfo,unsigned short> &theList = ClientEntityList().GetPVSNotifiers();
	FOR_EACH_LL( theList, i )
	{
		CClientEntityList::CPVSNotifyInfo *pInfo = &theList[i];

		// If this entity thinks it's in the PVS, but it wasn't in the PVS this frame, tell it so.
		if ( pInfo->m_InPVSStatus & INPVS_YES )
		{
			if ( pInfo->m_InPVSStatus & INPVS_THISFRAME )
			{
				if ( pInfo->m_InPVSStatus & INPVS_NEEDSNOTIFY )
				{
					pInfo->m_pNotify->OnPVSStatusChanged( true );
				}
				// Clear it for the next time around.
				pInfo->m_InPVSStatus &= ~( INPVS_THISFRAME | INPVS_NEEDSNOTIFY );
			}
			else
			{
				pInfo->m_InPVSStatus &= ~INPVS_YES;
				pInfo->m_pNotify->OnPVSStatusChanged( false );
			}
		}
	}
}


void OnRenderStart()
{
	VPROF( "OnRenderStart" );
	MDLCACHE_CRITICAL_SECTION();
	MDLCACHE_COARSE_LOCK();

#ifdef PORTAL
	g_pPortalRender->UpdatePortalPixelVisibility(); //updating this one or two lines before querying again just isn't cutting it. Update as soon as it's cheap to do so.
#endif

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );
	C_BaseEntity::SetAbsQueriesValid( false );

	Rope_ResetCounters();

	// Interpolate server entities and move aiments.
	{
		PREDICTION_TRACKVALUECHANGESCOPE( "interpolation" );
		C_BaseEntity::InterpolateServerEntities();
	}

	{
		// vprof node for this bloc of math
		VPROF( "OnRenderStart: dirty bone caches");
		// Invalidate any bone information.
		C_BaseAnimating::InvalidateBoneCaches();

		C_BaseEntity::SetAbsQueriesValid( true );
		C_BaseEntity::EnableAbsRecomputations( true );

		// Enable access to all model bones except view models.
		// This is necessary for aim-ent computation to occur properly
		C_BaseAnimating::PushAllowBoneAccess( true, false, "OnRenderStart->CViewRender::SetUpView" ); // pops in CViewRender::SetUpView

		// FIXME: This needs to be done before the player moves; it forces
		// aiments the player may be attached to to forcibly update their position
		C_BaseEntity::MarkAimEntsDirty();
	}

	// Make sure the camera simulation happens before OnRenderStart, where it's used.
	// NOTE: the only thing that happens in CAM_Think is thirdperson related code.
	input->CAM_Think();

	// This will place the player + the view models + all parent
	// entities	at the correct abs position so that their attachment points
	// are at the correct location
	view->OnRenderStart();

	RopeManager()->OnRenderStart();
	
	// This will place all entities in the correct position in world space and in the KD-tree
	C_BaseAnimating::UpdateClientSideAnimations();

	partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );

	// Process OnDataChanged events.
	ProcessOnDataChangedEvents();

	// Reset the overlay alpha. Entities can change the state of this in their think functions.
	g_SmokeFogOverlayAlpha = 0;	

	// This must occur prior to SimulatEntities,
	// which is where the client thinks for c_colorcorrection + c_colorcorrectionvolumes
	// update the color correction weights.
	// FIXME: The place where IGameSystem::Update is called should be in here
	// so we don't have to explicitly call ResetColorCorrectionWeights + SimulateEntities, etc.
	g_pColorCorrectionMgr->ResetColorCorrectionWeights();

	// Simulate all the entities.
	SimulateEntities();
	PhysicsSimulate();

	C_BaseAnimating::ThreadedBoneSetup();

	{
		VPROF_("Client TempEnts", 0, VPROF_BUDGETGROUP_CLIENT_SIM, false, BUDGETFLAG_CLIENT);
		// This creates things like temp entities.
		engine->FireEvents();

		// Update temp entities
		tempents->Update();

		// Update temp ent beams...
		beams->UpdateTempEntBeams();
		
		// Lock the frame from beam additions
		SetBeamCreationAllowed( false );
	}

	// Update particle effects (eventually, the effects should use Simulate() instead of having
	// their own update system).
	{
		// Enable FP exceptions here when FP_EXCEPTIONS_ENABLED is defined,
		// to help track down bad math.
		FPExceptionEnabler enableExceptions;
		VPROF_BUDGET( "ParticleMgr()->Simulate", VPROF_BUDGETGROUP_PARTICLE_SIMULATION );
		ParticleMgr()->Simulate( gpGlobals->frametime );
	}

	// Now that the view model's position is setup and aiments are marked dirty, update
	// their positions so they're in the leaf system correctly.
	C_BaseEntity::CalcAimEntPositions();

	// For entities marked for recording, post bone messages to IToolSystems
	if ( ToolsEnabled() )
	{
		C_BaseEntity::ToolRecordEntities();
	}

#if defined( REPLAY_ENABLED )
	// This will record any ragdolls if Replay mode is enabled on the server
	CReplayRagdollRecorder::Instance().Think();
	CReplayRagdollCache::Instance().Think();
#endif

	// Finally, link all the entities into the leaf system right before rendering.
	C_BaseEntity::AddVisibleEntities();
}


void OnRenderEnd()
{
	// Disallow access to bones (access is enabled in CViewRender::SetUpView).
	C_BaseAnimating::PopBoneAccess( "CViewRender::SetUpView->OnRenderEnd" );

	UpdatePVSNotifiers();

	DisplayBoneSetupEnts();
}



void CHLClient::FrameStageNotify( ClientFrameStage_t curStage )
{
	g_CurFrameStage = curStage;

	switch( curStage )
	{
	default:
		break;

	case FRAME_RENDER_START:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_RENDER_START" );

			// Last thing before rendering, run simulation.
			OnRenderStart();
		}
		break;
		
	case FRAME_RENDER_END:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_RENDER_END" );
			OnRenderEnd();

			PREDICTION_SPEWVALUECHANGES();
		}
		break;
		
	case FRAME_NET_UPDATE_START:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_START" );
			// disabled all recomputations while we update entities
			C_BaseEntity::EnableAbsRecomputations( false );
			C_BaseEntity::SetAbsQueriesValid( false );
			Interpolation_SetLastPacketTimeStamp( engine->GetLastTimeStamp() );
			partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, true );

			PREDICTION_STARTTRACKVALUE( "netupdate" );
		}
		break;
	case FRAME_NET_UPDATE_END:
		{
			ProcessCacheUsedMaterials();

			// reenable abs recomputation since now all entities have been updated
			C_BaseEntity::EnableAbsRecomputations( true );
			C_BaseEntity::SetAbsQueriesValid( true );
			partition->SuppressLists( PARTITION_ALL_CLIENT_EDICTS, false );

			PREDICTION_ENDTRACKVALUE();
		}
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_START:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_POSTDATAUPDATE_START" );
			PREDICTION_STARTTRACKVALUE( "postdataupdate" );
		}
		break;
	case FRAME_NET_UPDATE_POSTDATAUPDATE_END:
		{
			VPROF( "CHLClient::FrameStageNotify FRAME_NET_UPDATE_POSTDATAUPDATE_END" );
			PREDICTION_ENDTRACKVALUE();
			// Let prediction copy off pristine data
			prediction->PostEntityPacketReceived();
			HLTVCamera()->PostEntityPacketReceived();
#if defined( REPLAY_ENABLED )
			ReplayCamera()->PostEntityPacketReceived();
#endif
		}
		break;
	case FRAME_START:
		{
			// Mark the frame as open for client fx additions
			SetFXCreationAllowed( true );
			SetBeamCreationAllowed( true );
			C_BaseEntity::CheckCLInterpChanged();
		}
		break;
	}
}

CSaveRestoreData *SaveInit( int size );

// Save/restore system hooks
CSaveRestoreData  *CHLClient::SaveInit( int size )
{
	return ::SaveInit(size);
}

void CHLClient::SaveWriteFields( CSaveRestoreData *pSaveData, const char *pname, void *pBaseData, datamap_t *pMap, typedescription_t *pFields, int fieldCount )
{
	CSave saveHelper( pSaveData );
	saveHelper.WriteFields( pname, pBaseData, pMap, pFields, fieldCount );
}

void CHLClient::SaveReadFields( CSaveRestoreData *pSaveData, const char *pname, void *pBaseData, datamap_t *pMap, typedescription_t *pFields, int fieldCount )
{
	CRestore restoreHelper( pSaveData );
	restoreHelper.ReadFields( pname, pBaseData, pMap, pFields, fieldCount );
}

void CHLClient::PreSave( CSaveRestoreData *s )
{
	g_pGameSaveRestoreBlockSet->PreSave( s );
}

void CHLClient::Save( CSaveRestoreData *s )
{
	CSave saveHelper( s );
	g_pGameSaveRestoreBlockSet->Save( &saveHelper );
}

void CHLClient::WriteSaveHeaders( CSaveRestoreData *s )
{
	CSave saveHelper( s );
	g_pGameSaveRestoreBlockSet->WriteSaveHeaders( &saveHelper );
	g_pGameSaveRestoreBlockSet->PostSave();
}

void CHLClient::ReadRestoreHeaders( CSaveRestoreData *s )
{
	CRestore restoreHelper( s );
	g_pGameSaveRestoreBlockSet->PreRestore();
	g_pGameSaveRestoreBlockSet->ReadRestoreHeaders( &restoreHelper );
}

void CHLClient::Restore( CSaveRestoreData *s, bool b )
{
	CRestore restore(s);
	g_pGameSaveRestoreBlockSet->Restore( &restore, b );
	g_pGameSaveRestoreBlockSet->PostRestore();
}

static CUtlVector<EHANDLE> g_RestoredEntities;

void AddRestoredEntity( C_BaseEntity *pEntity )
{
	if ( !pEntity )
		return;

	g_RestoredEntities.AddToTail( EHANDLE(pEntity) );
}

void CHLClient::DispatchOnRestore()
{
	for ( int i = 0; i < g_RestoredEntities.Count(); i++ )
	{
		if ( g_RestoredEntities[i] != NULL )
		{
			MDLCACHE_CRITICAL_SECTION();
			g_RestoredEntities[i]->OnRestore();
		}
	}
	g_RestoredEntities.RemoveAll();
}

void CHLClient::WriteSaveGameScreenshot( const char *pFilename )
{
	view->WriteSaveGameScreenshot( pFilename );
}

// Given a list of "S(wavname) S(wavname2)" tokens, look up the localized text and emit
//  the appropriate close caption if running with closecaption = 1
void CHLClient::EmitSentenceCloseCaption( char const *tokenstream )
{
	extern ConVar closecaption;
	
	if ( !closecaption.GetBool() )
		return;

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->ProcessSentenceCaptionStream( tokenstream );
	}
}


void CHLClient::EmitCloseCaption( char const *captionname, float duration )
{
	extern ConVar closecaption;

	if ( !closecaption.GetBool() )
		return;

	CHudCloseCaption *hudCloseCaption = GET_HUDELEMENT( CHudCloseCaption );
	if ( hudCloseCaption )
	{
		hudCloseCaption->ProcessCaption( captionname, duration );
	}
}

CStandardRecvProxies* CHLClient::GetStandardRecvProxies()
{
	return &g_StandardRecvProxies;
}

bool CHLClient::CanRecordDemo( char *errorMsg, int length ) const
{
	if ( GetClientModeNormal() )
	{
		return GetClientModeNormal()->CanRecordDemo( errorMsg, length );
	}

	return true;
}

void CHLClient::OnDemoRecordStart( char const* pDemoBaseName )
{
}

void CHLClient::OnDemoRecordStop()
{
}

void CHLClient::OnDemoPlaybackStart( char const* pDemoBaseName )
{
#if defined( REPLAY_ENABLED )
	// Load any ragdoll override frames from disk
	char szRagdollFile[MAX_OSPATH];
	V_snprintf( szRagdollFile, sizeof(szRagdollFile), "%s.dmx", pDemoBaseName );
	CReplayRagdollCache::Instance().Init( szRagdollFile );
#endif
}

void CHLClient::OnDemoPlaybackStop()
{
#ifdef DEMOPOLISH_ENABLED
	if ( DemoPolish_GetController().m_bInit )
	{
		DemoPolish_GetController().Shutdown();
	}
#endif

#if defined( REPLAY_ENABLED )
	CReplayRagdollCache::Instance().Shutdown();
#endif
}

// See RenderViewInfo_t
void CHLClient::RenderView( const CViewSetup &setup, int nClearFlags, int whatToDraw )
{
	VPROF("RenderView");
	view->RenderView( setup, nClearFlags, whatToDraw );
}

void ReloadSoundEntriesInList( IFileList *pFilesToReload );

#ifndef NO_STEAM

CSteamID GetSteamIDForPlayerIndex( int iPlayerIndex )
{
	player_info_t pi;
	if ( steamapicontext && steamapicontext->SteamUtils() )
	{
		if ( engine->GetPlayerInfo( iPlayerIndex, &pi ) )
		{
			if ( pi.friendsID )
			{
				return CSteamID( pi.friendsID, 1, steamapicontext->SteamUtils()->GetConnectedUniverse(), k_EAccountTypeIndividual );
			}
		}
	}
	return CSteamID();
}

#endif
