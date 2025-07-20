//========= Copyright Valve Corporation, All rights reserved. ============//
//
// TF Nail
//
//=============================================================================
#include "cbase.h"
#include "tf_projectile_nail.h"
#include "tf_gamerules.h"

#ifdef CLIENT_DLL
#include "c_basetempentity.h"
#include "c_te_legacytempents.h"
#include "c_te_effect_dispatch.h"
#include "input.h"
#include "c_tf_player.h"
#include "cliententitylist.h"
#endif

#ifdef GAME_DLL
#include "tf_player.h"
#endif


//=============================================================================
//
// TF Nail Projectile functions (Server specific).
//
#define NAIL_MODEL					"models/weapons/w_models/w_nail.mdl"
#define NAIL_DISPATCH_EFFECT		"ClientProjectile_Syringe"

LINK_ENTITY_TO_CLASS(tf_projectile_nail, CTFProjectile_Nail);
PRECACHE_REGISTER(tf_projectile_nail);

short g_sModelIndexNail;
void PrecacheNail(void *pUser)
{
	g_sModelIndexNail = modelinfo->GetModelIndex( NAIL_MODEL );
}

PRECACHE_REGISTER_FN(PrecacheNail);

//-----------------------------------------------------------------------------
// CTFProjectile_Nail
//-----------------------------------------------------------------------------
#define NAIL_GRAVITY		0.3f
#define NAIL_VELOCITY	1000.0f
// Purpose:
//-----------------------------------------------------------------------------
CTFBaseProjectile* CTFProjectile_Nail::Create(
	const Vector& vecOrigin,
	const QAngle& vecAngles,
	CTFWeaponBaseGun* pLauncher /*= NULL*/,
	CBaseEntity* pOwner /*= NULL*/,
	CBaseEntity* pScorer /*= NULL*/,
	bool bCritical /*= false */
) {
	return CTFBaseProjectile::Create("tf_projectile_nail", vecOrigin, vecAngles, pOwner, NAIL_VELOCITY, g_sModelIndexNail, NAIL_DISPATCH_EFFECT, pScorer, bCritical);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
unsigned int CTFProjectile_Nail::PhysicsSolidMaskForEntity(void) const
{
	return BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_REDTEAM | CONTENTS_BLUETEAM;
}

//-----------------------------------------------------------------------------
float CTFProjectile_Nail::GetGravity(void)
{
	return NAIL_GRAVITY;
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
void GetNailTrailParticleName( CTFPlayer *pPlayer, CAttribute_String *attrParticleName, bool bCritical )
{
	int iTeamNumber = TF_TEAM_RED;	
	if ( pPlayer )
	{
		iTeamNumber = pPlayer->GetTeamNumber();
		CTFWeaponBase *pWeapon = pPlayer->GetActiveTFWeapon();
		if ( pWeapon )
		{
			static CSchemaAttributeDefHandle pAttrDef_ParticleName( "projectile particle name" );
			CEconItemView *pItem = pWeapon->GetAttributeContainer()->GetItem();
			if ( pAttrDef_ParticleName && pItem )
			{
				if ( pItem->FindAttribute( pAttrDef_ParticleName, attrParticleName ) )
				{
					const char * pParticleName = attrParticleName->value().c_str();
					if ( iTeamNumber == TF_TEAM_BLUE && V_stristr( pParticleName, "_teamcolor_red" ))
					{
						static char pBlue[256];
						V_StrSubst( attrParticleName->value().c_str(), "_teamcolor_red", "_teamcolor_blue", pBlue, 256 );
						attrParticleName->set_value( pBlue );
					}
					return;
				}
			}
		}
	}
	
	if ( iTeamNumber == TF_TEAM_BLUE )
	{
		attrParticleName->set_value( bCritical ? "nailtrails_medic_blue_crit" : "nailtrails_medic_blue" );
	}
	else
	{
		attrParticleName->set_value( bCritical ? "nailtrails_medic_red_crit" : "nailtrails_medic_red" );
	}
	return;
}

//-----------------------------------------------------------------------------
// Purpose: For Synrgine Projectiles, Add effects
//-----------------------------------------------------------------------------
void ClientsideProjectileNailCallback( const CEffectData &data )
{
	// Get the syringe and add it to the client entity list, so we can attach a particle system to it.
	C_TFPlayer *pPlayer = dynamic_cast<C_TFPlayer*>( ClientEntityList().GetBaseEntityFromHandle( data.m_hEntity ) );
	if ( pPlayer )
	{
		C_LocalTempEntity *pNail = ClientsideProjectileCallback( data, NAIL_GRAVITY);
		if ( pNail )
		{
			CAttribute_String attrParticleName;
			
			bool bCritical = ( ( data.m_nDamageType & DMG_CRITICAL ) != 0 );
			GetNailTrailParticleName( pPlayer, &attrParticleName, bCritical );

			pNail->AddParticleEffect( attrParticleName.value().c_str() );
			pNail->AddEffects( EF_NOSHADOW );
			pNail->flags |= FTENT_USEFASTCOLLISIONS;
		}
	}
}

DECLARE_CLIENT_EFFECT( NAIL_DISPATCH_EFFECT, ClientsideProjectileNailCallback);

#endif
