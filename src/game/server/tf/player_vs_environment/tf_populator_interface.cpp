//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements populator interface entity.  Used for map
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "tf_population_manager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar tf_populator_debug;


class CPointPopulatorInterface : public CPointEntity
{
	DECLARE_CLASS( CPointPopulatorInterface, CPointEntity );
	DECLARE_ENT_SCRIPTDESC();
public:

	// Input handlers
	void InputPauseBotSpawning(inputdata_t &inputdata);
	void InputUnpauseBotSpawning(inputdata_t &inputdata);
	void InputChangeBotAttributes(inputdata_t &inputdata);
	void InputChangeDefaultEventAttributes(inputdata_t &inputdata);
	void InputGoToWave(inputdata_t& inputdata);
	void InputStartWave(inputdata_t &);
	void InputEndWave(inputdata_t& inputdata);

	//Converted to standalones
	void ChangeBotAttributes( const char* pszEventNam );
	void ChangeDefaultEventAttributes( const char* pszEventNam );
	void GoToWave( int waveNumber, float fCleanMoneyPercent = -1.0f );
	void StartWave ( void );

	// Vscript handlers - August101
	int	 ScriptGetWaveNumber(void) { return g_pPopulationManager->GetWaveNumber(); }
	int  ScriptGetWaveTotal(void) { return g_pPopulationManager->GetTotalWaveCount(); }
	bool ScriptIsSpawningPaused(void) { return g_pPopulationManager->IsSpawningPaused(); }
	//Standalone Conversion - Shared with I/O
	void ScriptPauseSpawning(void) { g_pPopulationManager->PauseSpawning(); }
	void ScriptUnpauseSpawning(void) { g_pPopulationManager->UnpauseSpawning(); }
	void ScriptStartWave(void) { g_pPopulationManager->StartCurrentWave(); }
	void ScriptEndWave(bool bSuccess) { g_pPopulationManager->ScriptEndWave(bSuccess); }
	
	DECLARE_DATADESC();
};

BEGIN_DATADESC( CPointPopulatorInterface )

	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID, "PauseBotSpawning", InputPauseBotSpawning ),
	DEFINE_INPUTFUNC( FIELD_VOID, "UnpauseBotSpawning", InputUnpauseBotSpawning ),
	DEFINE_INPUTFUNC( FIELD_STRING, "ChangeBotAttributes", InputChangeBotAttributes ),
	DEFINE_INPUTFUNC( FIELD_STRING, "ChangeDefaultEventAttributes", InputChangeDefaultEventAttributes ),
	DEFINE_INPUTFUNC( FIELD_STRING,  "GoToWave", InputGoToWave ),
	DEFINE_INPUTFUNC( FIELD_VOID,    "StartWave", InputStartWave ),
	DEFINE_INPUTFUNC( FIELD_BOOLEAN, "EndWave", InputEndWave ),

END_DATADESC()

	// Vscript - August101
	BEGIN_ENT_SCRIPTDESC_ROOT(CPointPopulatorInterface, "Interface for MvM populator")
	DEFINE_SCRIPTFUNC_NAMED(ScriptGetWaveNumber, "GetWaveNumber", "Get Current Wave number")
	DEFINE_SCRIPTFUNC_NAMED(ScriptGetWaveTotal, "GetWaveTotalCount", "Get Total Wave in current mission")
	DEFINE_SCRIPTFUNC(ScriptIsSpawningPaused, "is spawnwaves paused?")
	//Standalone Conversion - Shared with I/O
	DEFINE_SCRIPTFUNC_NAMED(ScriptStartWave, "StartWave", "Forcibly Start current wave")
	DEFINE_SCRIPTFUNC_NAMED(GoToWave, "JumpToWave", "Arugments - (waveNum, CurrencyPercent); Set the current Wave")
	DEFINE_SCRIPTFUNC_NAMED(ScriptEndWave, "EndWave", "Arugments - (victory); Force finishes the current Wave")
	DEFINE_SCRIPTFUNC_NAMED(ScriptPauseSpawning, "PauseBotSpawning", "Pause spawnwaves")
	DEFINE_SCRIPTFUNC_NAMED(ScriptUnpauseSpawning, "UnpauseBotSpawning", "Resume spawnwaves")
	DEFINE_SCRIPTFUNC_NAMED(ChangeBotAttributes, "ChangeBotAttributes", "Arugments - (attribute); ChangeBotAttributes input as a vscript function")
	DEFINE_SCRIPTFUNC_NAMED(ChangeDefaultEventAttributes, "ChangeDefaultEventAttributes", "Arugments - (attribute); ChangeDefaultEventAttributes input as a vscript function")
	END_SCRIPTDESC();

LINK_ENTITY_TO_CLASS( point_populator_interface, CPointPopulatorInterface );


void CPointPopulatorInterface::InputPauseBotSpawning( inputdata_t &inputdata )
{
	Assert( g_pPopulationManager );
	if( g_pPopulationManager )
	{
		g_pPopulationManager->PauseSpawning();
	}
}

void CPointPopulatorInterface::InputUnpauseBotSpawning( inputdata_t &inputdata )
{
	Assert( g_pPopulationManager );
	if( g_pPopulationManager )
	{
		g_pPopulationManager->UnpauseSpawning();
	}
}


void CPointPopulatorInterface::InputGoToWave( inputdata_t &inputdata )
{

	char		token[128];
	const char	*p = STRING( inputdata.value.StringID() );
	int			nMoneyPercent = 0;
	int			nWave = 0;

	// get the team
	p = nexttoken( token, p, ' ' );
	if ( token[0] )
	{
		nMoneyPercent = Q_atoi( token );
	}

	// get the time
	p = nexttoken( token, p, ' ' );
	if ( token[0] )
	{
		nWave = Q_atoi( token );
	}

	if ( nWave != 0 )
	{
		GoToWave( nWave, nMoneyPercent );
	}

	/*
	if ( inputdata.value.Int() <= 1 )
	{
		Msg( "Populator Input: Missing wave number\n" );
		return;
	}

	float fCleanMoneyPercent = -1.0f;
	if ( inputdata.value.Int() >= 3 )
	{
		fCleanMoneyPercent = atof( args.Arg(2) );
	}

	uint32 desiredWave = (uint32)Max( atoi( inputdata.value.Int() ) - 1, 0) ;
	GoToWave( desiredWave, fCleanMoneyPercent );
	*/
}

void CPointPopulatorInterface::GoToWave( int waveNumber, float fCleanMoneyPercent )
{
	g_pPopulationManager->JumpToWave( (uint32) waveNumber, fCleanMoneyPercent );
}

void CPointPopulatorInterface::InputStartWave( inputdata_t & ) { StartWave(); }
void CPointPopulatorInterface::StartWave( void ) {	g_pPopulationManager->StartCurrentWave(); }

void CPointPopulatorInterface::InputEndWave( inputdata_t &inputdata ) 
{ 
	g_pPopulationManager->ScriptEndWave( inputdata.value.Bool() ); 
}

void CPointPopulatorInterface::InputChangeBotAttributes( inputdata_t &inputdata )
{
	const char* pszEventName = inputdata.value.String();
	ChangeBotAttributes ( pszEventName );
}

//Turned into standalone
void CPointPopulatorInterface::ChangeBotAttributes( const char* pszEventName )
{

	if ( tf_populator_debug.GetBool() && g_pPopulationManager && !g_pPopulationManager->HasEventChangeAttributes( pszEventName ) )
	{
		Warning( "ChangeBotAttributes: Failed to find event [%s] in the pop file\n", pszEventName );
		return;
	}

	if ( TFGameRules()->IsMannVsMachineMode() )
	{
		CUtlVector< CTFBot* > botVector;
		CollectPlayers( &botVector, TF_TEAM_PVE_INVADERS, COLLECT_ONLY_LIVING_PLAYERS );
		for ( int i=0; i<botVector.Count(); ++i )
		{
			// MVM Versus - Mannhattan crash patch
			CTFBot* pBot = ToTFBot(botVector[i]);
			if( !pBot )
				continue;
			const CTFBot::EventChangeAttributes_t* pEvent = pBot->GetEventChangeAttributes( pszEventName );
			if ( pEvent )
			{
				botVector[i]->OnEventChangeAttributes( pEvent );
			}
		}
	}
}

void CPointPopulatorInterface::InputChangeDefaultEventAttributes(inputdata_t &inputdata)
{
	const char* pszEventName = inputdata.value.String();
	ChangeDefaultEventAttributes( pszEventName );
}

//Turned into standalone
void CPointPopulatorInterface::ChangeDefaultEventAttributes( const char* pszEventName )
{

	if ( tf_populator_debug.GetBool() && g_pPopulationManager && !g_pPopulationManager->HasEventChangeAttributes( pszEventName ) )
	{
		Warning( "ChangeBotAttributes: Failed to find event [%s] in the pop file\n", pszEventName );
		return;
	}

	if ( g_pPopulationManager )
	{
		g_pPopulationManager->SetDefaultEventChangeAttributesName( pszEventName );
	}	
}