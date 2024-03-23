//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#if !defined( IINPUT_H )
#define IINPUT_H
#ifdef _WIN32
#pragma once
#endif

#include "inputsystem/iinputsystem.h"

class bf_write;
class bf_read;
class CUserCmd;
class C_BaseCombatWeapon;
struct kbutton_t;

struct CameraThirdData_t
{
	float	m_flPitch;
	float	m_flYaw;
	float	m_flDist;
	float	m_flLag;
	Vector	m_vecHullMin;
	Vector	m_vecHullMax;
};

abstract_class IInput
{
public:
	// Initialization/shutdown of the subsystem
	virtual	void		Init_All( void ) = 0;
	virtual void		Shutdown_All( void ) = 0;
	// Latching button states
	virtual int			GetButtonBits( bool bResetState ) = 0;
	// Create movement command
	virtual void		CreateMove ( int sequence_number, float input_sample_frametime, bool active ) = 0;
	virtual void		ExtraMouseSample( float frametime, bool active ) = 0;
	virtual bool		WriteUsercmdDeltaToBuffer( int nSlot, bf_write *buf, int from, int to, bool isnewcommand ) = 0;
	virtual void		EncodeUserCmdToBuffer( int nSlot, bf_write& buf, int slot ) = 0;
	virtual void		DecodeUserCmdFromBuffer( int nSlot, bf_read& buf, int slot ) = 0;

	virtual CUserCmd	*GetUserCmd( int nSlot, int sequence_number ) = 0;

	virtual void		MakeWeaponSelection( C_BaseCombatWeapon *weapon ) = 0;

	// Retrieve key state
	virtual float		KeyState ( kbutton_t *key ) = 0;
	// Issue key event
	virtual int			KeyEvent( int eventcode, ButtonCode_t keynum, const char *pszCurrentBinding ) = 0;
	// Look for key
	virtual kbutton_t	*FindKey( const char *name ) = 0;

	// Issue commands from controllers
	virtual void		ControllerCommands( void ) = 0;
	// Extra initialization for some joysticks
	virtual bool		ControllerModeActive() = 0;
	virtual void		Joystick_Advanced( bool bSilent ) = 0;
	virtual void		Joystick_SetSampleTime( float frametime ) = 0;
	virtual void		IN_SetSampleTime( float frametime ) = 0;

	// Accumulate mouse delta
	virtual void		AccumulateMouse( int nSlot ) = 0;
	// Activate/deactivate mouse
	virtual void		ActivateMouse( void ) = 0;
	virtual void		DeactivateMouse( void ) = 0;

	// Clear mouse state data
	virtual void		ClearStates( void ) = 0;
	// Retrieve lookspring setting
	virtual float		GetLookSpring( void ) = 0;

	// Retrieve mouse position
	virtual void		GetFullscreenMousePos( int *mx, int *my, int *unclampedx = 0, int *unclampedy = 0 ) = 0;
	virtual void		SetFullscreenMousePos( int mx, int my ) = 0;
	virtual void		ResetMouse() = 0;
	virtual	float		GetLastForwardMove( void ) = 0;

	// Third Person camera ( TODO/FIXME:  Move this to a separate interface? )
	virtual void		CAM_Think( void ) = 0;
	virtual int			CAM_IsThirdPerson( int nSlot = -1 ) = 0;
	virtual void		CAM_GetCameraOffset( Vector& ofs ) = 0;
	virtual void		CAM_ToThirdPerson(void) = 0;
	virtual void		CAM_ToFirstPerson(void) = 0;
	virtual void		CAM_ToThirdPersonShoulder(void) = 0;
	virtual void		CAM_StartMouseMove(void) = 0;
	virtual void		CAM_EndMouseMove(void) = 0;
	virtual void		CAM_StartDistance(void) = 0;
	virtual void		CAM_EndDistance(void) = 0;
	virtual int			CAM_InterceptingMouse( void ) = 0;
	virtual	void		CAM_Command( int command ) = 0;

	// orthographic camera info	( TODO/FIXME:  Move this to a separate interface? )
	virtual void		CAM_ToOrthographic() = 0;
	virtual	bool		CAM_IsOrthographic() const = 0;
	virtual	void		CAM_OrthographicSize( float& w, float& h ) const = 0;

#if defined( HL2_CLIENT_DLL )
	// IK back channel info
	virtual void		AddIKGroundContactInfo( int entindex, float minheight, float maxheight ) = 0;
#endif

	virtual void		LevelInit( void ) = 0;

	// Causes an input to have to be re-pressed to become active
	virtual void		ClearInputButton( int bits ) = 0;

	virtual	void		CAM_SetCameraThirdData( CameraThirdData_t *pCameraData, const QAngle &vecCameraOffset ) = 0;
	virtual void		CAM_CameraThirdThink( void ) = 0;
};

extern ::IInput *input;




///-----------------------------------------------------------------------------
/// Purpose: This is input priority system, allowing various clients to
/// cause input messages / cursor control to be routed to them as opposed to
/// other clients.
///
/// NOTE: For Source1, it would be a huge change to move all input (like 
/// the code in engine/keys.cpp for example) to go through this interface. 
/// Therefore, I'm going to stick with only dealing with cursor control, 
/// which is necessary for Jen's new gameUI system to interoperate with VGui.
///-----------------------------------------------------------------------------
abstract_class IInputStackSystem : public IAppSystem
{
public:
	/// Allocates an input context, pushing it on top of the input stack, 
	/// thereby giving it top priority
	virtual InputContextHandle_t PushInputContext() = 0;

	/// Pops the top input context off the input stack, and destroys it.
	virtual void PopInputContext() = 0;

	/// Enables/disables an input context, allowing something lower on the
	/// stack to have control of input. Disabling an input context which
	/// owns mouse capture
	virtual void EnableInputContext(InputContextHandle_t hContext, bool bEnable) = 0;

	/// Allows a context to make the cursor visible;
	/// the topmost enabled context wins
	virtual void SetCursorVisible(InputContextHandle_t hContext, bool bVisible) = 0;

	/// Allows a context to set the cursor icon;
	/// the topmost enabled context wins
	virtual void SetCursorIcon(InputContextHandle_t hContext, InputCursorHandle_t hCursor) = 0;

	/// Allows a context to enable mouse capture. Disabling an input context
	/// deactivates mouse capture. Capture will occur if it happens on the
	/// topmost enabled context
	virtual void SetMouseCapture(InputContextHandle_t hContext, bool bEnable) = 0;

	/// Allows a context to set the mouse position. It only has any effect if the
	/// specified context is the topmost enabled context
	virtual void SetCursorPosition(InputContextHandle_t hContext, int x, int y) = 0;

	/// Returns true if the specified context is the topmost enabled context
	virtual bool IsTopmostEnabledContext(InputContextHandle_t hContext) const = 0;
};

extern IInputStackSystem *g_pInputStackSystem;

#endif // IINPUT_H