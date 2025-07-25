//========= Copyright Valve Corporation, All rights reserved. ============//
//
// TF Nail Projectile
//
//=============================================================================
#ifndef TF_PROJECTILE_NAIL_H
#define TF_PROJECTILE_NAIL_H
#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "tf_projectile_base.h"
#include "tf_weaponbase_gun.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
class CTFProjectile_Nail : public CTFBaseProjectile
{
	DECLARE_CLASS(CTFProjectile_Nail, CTFBaseProjectile);

public:
	// Creation.
	static CTFBaseProjectile* Create(const Vector& vecOrigin, const QAngle& vecAngles, CTFWeaponBaseGun* pLauncher = NULL, CBaseEntity* pOwner = NULL, CBaseEntity* pScorer = NULL, bool bCritical = false);

	virtual unsigned int PhysicsSolidMaskForEntity(void) const;
	virtual const char* GetProjectileModelName(void) { return "models/weapons/w_models/w_nail.mdl"; }
	virtual float GetGravity(void);
};


#endif	//TF_PROJECTILE_NAIL_H
