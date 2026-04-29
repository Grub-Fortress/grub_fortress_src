//========= Copyright Valve Corporation, All rights reserved. ============//
//
// TF Crossbow
//
//=============================================================================
#include "cbase.h"
#include "tf_weapon_rocketlauncher.h"

// Server specific.
#ifndef CLIENT_DLL

#include "tf_player.h"
#include "tf_projectile_arrow.h"

#endif

//=============================================================================
//
// Crossbow tables.
//
IMPLEMENT_NETWORKCLASS_ALIASED( TFCrossbow, DT_Crossbow )

BEGIN_NETWORK_TABLE( CTFCrossbow, DT_Crossbow )
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flRegenerateDuration ) ),
	RecvPropFloat( RECVINFO( m_flLastUsedTimestamp ) ),
#else
	SendPropFloat( SENDINFO( m_flRegenerateDuration ), 0, SPROP_NOSCALE ),
	SendPropFloat( SENDINFO( m_flLastUsedTimestamp ), 0, SPROP_NOSCALE ),
#endif
END_NETWORK_TABLE()

BEGIN_PREDICTION_DATA( CTFCrossbow )
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( tf_weapon_crossbow, CTFCrossbow );
PRECACHE_WEAPON_REGISTER( tf_weapon_crossbow );

// Server specific.
#ifndef CLIENT_DLL
BEGIN_DATADESC( CTFCrossbow )
END_DATADESC()
#endif

bool CTFCrossbow::Holster( CBaseCombatWeapon *pSwitchingTo )
{
	// Allow Crossbow to silently reload like the flaregun
	if ( m_iClip1 == 0 )
	{
		// These Values need to match the anim times since all this stuff is actually driven by animation sequence time in the base code
		float flFireDelay = ApplyFireDelay( m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeFireDelay );

		float flReloadTime = m_pWeaponInfo->GetWeaponData( m_iWeaponMode ).m_flTimeReload;
		CALL_ATTRIB_HOOK_FLOAT( flReloadTime, mult_reload_time );
		CALL_ATTRIB_HOOK_FLOAT( flReloadTime, mult_reload_time_hidden );
		CALL_ATTRIB_HOOK_FLOAT( flReloadTime, fast_reload );

		float flIdleTime = GetLastPrimaryAttackTime() + flFireDelay + flReloadTime;
		if ( GetWeaponIdleTime() < flIdleTime )
		{
			SetWeaponIdleTime( flIdleTime );
			m_flNextPrimaryAttack = flIdleTime;
		}

		IncrementAmmo();
	}

	return BaseClass::Holster( pSwitchingTo );
}
//-----------------------------------------------------------------------------
void CTFCrossbow::SecondaryAttack( void )
{
	// If this is the jarate bolt crossbow, make sure we are allowed to do it
	int iMilkBolt = 0;
	CALL_ATTRIB_HOOK_INT( iMilkBolt, fires_milk_bolt );
	if ( iMilkBolt )
	{
		CTFPlayer *pPlayer = GetTFPlayerOwner();
		if ( !pPlayer )
			return;

		if ( !CanAttack() )
			return;

		if ( m_flNextPrimaryAttack > gpGlobals->curtime )
			return;

		// Can we attack
		if ( GetProgress() >= 1.0f )
		{
			// Call Primary Attack and modify the projectile
			m_bMilkNextAttack = true;
			PrimaryAttack();
			m_flRegenerateDuration = iMilkBolt;
			m_flLastUsedTimestamp = gpGlobals->curtime;
		}
	}
}

//-----------------------------------------------------------------------------
void CTFCrossbow::ModifyProjectile( CBaseEntity* pProj )
{
#ifdef GAME_DLL
	if ( m_bMilkNextAttack )
	{
		CTFProjectile_Arrow* pMainArrow = assert_cast<CTFProjectile_Arrow*>( pProj );
		if ( pMainArrow )
		{
			pMainArrow->SetApplyMilkOnHit();
		}
	}
#endif

	m_bMilkNextAttack = false;
}
//-----------------------------------------------------------------------------
void CTFCrossbow::ItemPostFrame( void )
{
	BaseClass::ItemPostFrame();
	m_bMilkNextAttack = false;
}
//-----------------------------------------------------------------------------
float CTFCrossbow::GetProjectileSpeed( void )
{
	return RemapValClamped( 0.75f, 0.0f, 1.f, 1800, 2600 ); // Temp, if we want to ramp.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CTFCrossbow::GetProjectileGravity( void )
{
	return RemapValClamped( 0.75f, 0.0f, 1.f, 0.5, 0.1 ); // Temp, if we want to ramp.
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CTFCrossbow::IsViewModelFlipped( void )
{
	return !BaseClass::IsViewModelFlipped(); // Invert because arrows are backwards by default.
}
//-----------------------------------------------------------------------------
void CTFCrossbow::WeaponRegenerate( void )
{
	BaseClass::WeaponRegenerate();
	m_flLastUsedTimestamp = 0;
}
//-----------------------------------------------------------------------------
inline float CTFCrossbow::GetProgress( void )
{
	int iMilkBolt = 0;
	CALL_ATTRIB_HOOK_INT( iMilkBolt, fires_milk_bolt );
	if ( iMilkBolt == 0 )
		return 0;

	float meltedTime = gpGlobals->curtime - m_flLastUsedTimestamp;
	return meltedTime / m_flRegenerateDuration;
}

