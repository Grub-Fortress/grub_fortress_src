//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "tf_extra_map_entity.h"
#include "KeyValues.h"
#include "filesystem.h"

struct EntityWhiteList_t
{
	const char *pszKeyName;
	const char *pszEntName;
};

ConVar bf_teaserprops("bf_teaserprops", "1", FCVAR_REPLICATED, "Enable teaser props on mapspawn.");
ConVar bf_teaserprops_type("bf_teaserprops_type", "mvm", FCVAR_REPLICATED, "Which teaser you want to spawn with?");

// limit the entities that can be created using this method
EntityWhiteList_t g_szEntityWhiteList[] =
{
	{ "rocket", "entity_rocket" },
	{ "carrier", "entity_carrier" },
	{ "sign", "entity_sign" },
	{ "saucer", "entity_saucer" },
	//Mod Entries
	{ "bf_sign", "entity_sign" },
};

//Prop Lists
static const char* s_pszTeaserMDL_Invasion[] =
{
	"models/props_teaser/update_invasion_poster002.mdl",
	"models/props_teaser/update_invasion_poster001.mdl",
	"models/egypt/palm_tree/palm_tree.mdl",
	"models/props_spytech/control_room_console01.mdl",
	"models/props_spytech/work_table001.mdl",
};


LINK_ENTITY_TO_CLASS( entity_rocket, CExtraMapEntity_Rocket );

LINK_ENTITY_TO_CLASS( entity_carrier, CExtraMapEntity_Carrier );

LINK_ENTITY_TO_CLASS( entity_sign, CExtraMapEntity_Sign );

LINK_ENTITY_TO_CLASS( entity_saucer, CExtraMapEntity_Saucer );


BEGIN_DATADESC( CExtraMapEntity )
DEFINE_THINKFUNC( AnimThink ),
END_DATADESC();

IMPLEMENT_AUTO_LIST( IExtraMapEntityAutoList );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CExtraMapEntity::Spawn( void )
{
	Precache();
	BaseClass::Spawn();
	
	SetModel( STRING( GetModelName() ) );

	SetMoveType( MOVETYPE_NONE );
	SetSequence( 0 );

	if ( ShouldAnimate() )
	{
		ResetSequenceInfo();

		SetThink( &CExtraMapEntity::AnimThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
}

void CExtraMapEntity::AnimThink( void )
{
	StudioFrameAdvance();
	DispatchAnimEvents( this );
	SetNextThink( gpGlobals->curtime + 0.1f );
}

void CExtraMapEntity::Precache( void )
{
	BaseClass::Precache();

	bool bAllowPrecache = CBaseEntity::IsPrecacheAllowed();
	CBaseEntity::SetAllowPrecache( true );
	Precache_Internal();
	CBaseEntity::SetAllowPrecache( bAllowPrecache );
}

void CExtraMapEntity::Precache_Internal( void )
{
	PrecacheModel( STRING( GetModelName() ) );
}

const char *CExtraMapEntity::ValidateKeyName( const char *pszKey )
{
	if ( pszKey && pszKey[0] )
	{
		for ( int i = 0; i < ARRAYSIZE( g_szEntityWhiteList ); i++ )
		{
			if ( FStrEq( pszKey, g_szEntityWhiteList[i].pszKeyName ) )
			{
				return g_szEntityWhiteList[i].pszEntName;
			}
		}
	}

	return NULL;
}

void CExtraMapEntity::PrepareModelName( const char *szModelName )
{
	const char *pszTemp = GetDefaultModel();
	if ( szModelName && szModelName[0] )
	{
		pszTemp = szModelName;
	}

	SetModelName( AllocPooledString( pszTemp ) );
}

void CExtraMapEntity::SpawnExtraModel( void )
{

	const char *pszMapName = STRING( gpGlobals->mapname );
	if ( !pszMapName || !pszMapName[0] )
		return;

	// Better Fortress 2 - i loved these teasers, and i am sure everyone did back in the day...
	KeyValues *pFileKV = new KeyValues( "models" );
	/*
	if ( !pFileKV->LoadFromFile( g_pFullFileSystem, "scripts/extra_models.txt", "MOD" ) )
		return;
	*/
	if ( !pFileKV->LoadFromFile( g_pFullFileSystem, "scripts/bf_extra_models.txt", "GAME" ) )
		return;



	// See if we have an entry for this map.
	KeyValues *pMapKV = pFileKV->FindKey( pszMapName );
	if ( pMapKV )
	{
		FOR_EACH_SUBKEY( pMapKV, pSubKeyEnt )
		{
			const char *pszEntName = ValidateKeyName( pSubKeyEnt->GetName() );
			if ( !pszEntName )
				continue;
			
			FOR_EACH_SUBKEY( pSubKeyEnt, pSubKeyCount )
			{
				Vector loc = vec3_origin;
				QAngle rot( 0, 0, 0 );
				char szModelName[MAX_PATH];
				szModelName[0] = '\0';
				float flChance = 1.0f; // assume we want to show everything unless specified in the .txt file

				FOR_EACH_SUBKEY( pSubKeyCount, pSubKey )
				{
					if ( FStrEq( pSubKey->GetName(), "location" ) )
					{
						const char *pszLoc = pSubKey->GetString();
						UTIL_StringToVector( loc.Base(), pszLoc );
					}
					else if ( FStrEq( pSubKey->GetName(), "rotation" ) )
					{
						const char *pszRot = pSubKey->GetString();
						UTIL_StringToVector( rot.Base(), pszRot );
					}
					else if ( FStrEq( pSubKey->GetName(), "model" ) )
					{
						V_strcpy_safe( szModelName, pSubKey->GetString() );
					}
					else if ( FStrEq( pSubKey->GetName(), "chance" ) )
					{
						flChance = pSubKey->GetFloat();
						if ( flChance > 1.0f )
						{
							flChance = 1.0f;
						}
					}
				}

				if ( ( flChance > 0.0f ) && ( RandomFloat( 0, 1 ) < flChance ) )
				{
					//If MVM, but no MVM Sign
					if ( !V_strcmp( "mvm", bf_teaserprops_type.GetString() ) && V_strcmp( "models/props_teaser/update_eotl_poster001.mdl", szModelName ) || V_strcmp( ENTITYCARRIER_DEFAULT_MODEL, szModelName ) )
						return;

					//if Grocket, but not Grocket
					if ( !V_strcmp( "grocket", bf_teaserprops_type.GetString() ) && V_strcmp( ENTITYROCKET_DEFAULT_MODEL, szModelName ) )
						return;

					CExtraMapEntity *pExtraMapEntity = static_cast< CExtraMapEntity* >( CBaseEntity::CreateNoSpawn( pszEntName, loc, rot ) );
					if ( pExtraMapEntity )
					{
						pExtraMapEntity->PrepareModelName( szModelName );
						DispatchSpawn( pExtraMapEntity );
					}
				}
			}
		}
	}

	pFileKV->deleteThis();

}

//-----------------------------------------------------------------------------
// Purpose: Grordbort rocket model in the skybox
//-----------------------------------------------------------------------------
void CExtraMapEntity_Rocket::Spawn( void )
{
	if ( !V_strcmp( "grocket", bf_teaserprops_type.GetString() ) )
	{

		BaseClass::Spawn();

		SetSolid(SOLID_BBOX);
		SetSize(-Vector(8, 8, 0), Vector(8, 8, 16));
	}
}

void CExtraMapEntity_Rocket::Precache_Internal( void )
{
	BaseClass::Precache_Internal();
	PrecacheParticleSystem( "smoke_blackbillow_skybox" );
}

//-----------------------------------------------------------------------------
// Purpose: Mann vs. Machine carrier model in the skybox
//-----------------------------------------------------------------------------
void CExtraMapEntity_Carrier::Spawn( void )
{
	if ( !V_strcmp( "mvm", bf_teaserprops_type.GetString() ) )
	{

		BaseClass::Spawn();

		SetSolid(SOLID_BBOX);
		SetSize(-Vector(8, 8, 0), Vector(8, 8, 16));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sign models to tease future updates
//-----------------------------------------------------------------------------
void CExtraMapEntity_Sign::Spawn( void )
{
		
	BaseClass::Spawn();

	SetSolid(SOLID_NONE);
	AddEffects(EF_NOSHADOW);

}

//-----------------------------------------------------------------------------
// Purpose: Saucer models to tease future updates
//-----------------------------------------------------------------------------
void CExtraMapEntity_Saucer::Spawn( void )
{
	if ( !V_strcmp( "invasion", bf_teaserprops_type.GetString() ) )
	{

		BaseClass::Spawn();

		SetSolid(SOLID_NONE);
		AddEffects(EF_NOSHADOW);
	}
}
