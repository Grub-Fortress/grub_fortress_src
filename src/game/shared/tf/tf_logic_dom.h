//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Entities for use in the Domination TF:Grub game mode. (Modified PD logic)
//
//=========================================================================//
#ifndef DOMINATION_H
#define DOMINATION_H
#pragma once

#include "cbase.h"
#include "tf_logic_robot_destruction.h"

#ifdef CLIENT_DLL
	#define CTFDomLogic C_TFDomLogic
#endif

//-----------------------------------------------------------------------------
class CTFDomLogic : public CTFRobotDestructionLogic
{
public:
#ifdef GAME_DLL
	DECLARE_DATADESC();
#endif // GAME_DLL
	DECLARE_CLASS( CTFDomLogic, CTFRobotDestructionLogic )
	DECLARE_NETWORKCLASS();

	virtual EType GetType() const { return TYPE_DOMINATION; }

	virtual void	PlaySoundInfoForScoreEvent( CTFPlayer* pPlayer, bool bPositive, int nNewScore, int nTeam, RDScoreMethod_t eMethod = SCORE_UNDEFINED ) OVERRIDE;

	CTFDomLogic();
	static CTFDomLogic* GetDomLogic();

#ifdef GAME_DLL
	virtual void Precache() OVERRIDE;

	virtual void FireGameEvent( IGameEvent *pEvent ) OVERRIDE;

	void InputScoreRedPoints( inputdata_t& inputdata );
	void InputScoreBluePoints( inputdata_t& inputdata );
	void InputEnableMaxScoreUpdating( inputdata_t& inputdata );
	void InputDisableMaxScoreUpdating( inputdata_t& inputdata );
	void InputSetCountdownTimer( inputdata_t& inputdata );
	void InputSetCountdownImage( inputdata_t& inputdata );

	void CountdownThink( void );
	virtual void TeamWin( int nTeam ) OVERRIDE;

#endif // GAME_DLL

	string_t GetCountdownImage( void ) OVERRIDE { return m_iszCountdownImage; }
	virtual bool IsUsingCustomCountdownImage( void ) OVERRIDE{ return m_bUsingCountdownImage; }

private:
#ifdef GAME_DLL
	virtual void OnRedScoreChanged() OVERRIDE;
	virtual void OnBlueScoreChanged() OVERRIDE;
	
	void EvaluatePlayerCount();

	void SetCountdownImage( string_t iszCountdownImage ) { m_iszCountdownImage = iszCountdownImage; }

	int m_nMinPoints;
	int m_nPointsPerPlayer;
	bool m_bMaxScoreUpdatingAllowed;

	COutputFloat m_OnRedScoreChanged;
	COutputFloat m_OnBlueScoreChanged;

	COutputEvent m_OnCountdownTimerExpired;
#endif // GAME_DLL
	CNetworkVar( bool, m_bUsingCountdownImage );

#ifdef CLIENT_DLL
	char		m_iszCountdownImage[MAX_PATH];
#else
	CNetworkVar( string_t, m_iszCountdownImage );
#endif
};

#endif// DOMINATION_H
