//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef INSTANTACTIONGAMEDIALOG_H
#define INSTANTACTIONGAMEDIALOG_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include "tier1/netadr.h"
#include "ServerBrowser/blacklisted_server_manager.h"


//TF prefers a higher min player count. cap it to 6 so at least 3v3 is possible.
#if defined(TF_CLIENT_DLL)
#define SERVERFINDER_MIN_PLAYERS 6
#else
#define SERVERFINDER_MIN_PLAYERS 2
#endif

#define SERVERFINDER_MAX_PING 300
#define SERVERFINDER_MAX_RETRIES 10

//#ifdef _DEBUG
#define SERVERFINDER_FULLDEBUG 1
//#endif

#ifdef SERVERFINDER_FULLDEBUG
#define SERVERFINDER_CONNECTION_TEST 1
#endif

#define SERVERFINDER_LEVEL_VALIDATION 1
#define SERVERFINDER_LEVEL_INTERNAL	  2
#define SERVERFINDER_SPEW( lvl, pStrFmt, ...) \
	if ( serverfinder_debug.GetInt() >= lvl ) \
	{ \
		Msg( pStrFmt, ##__VA_ARGS__ ); \
	}

struct gameserveritem_ex_t
{
	gameserveritem_t server;
	int m_nRealPlayers;
};

enum EServerfinderMode
{
	eServerfinderPhase0Nothing,
	eServerfinderPhase1ServerQueue,
	eServerfinderPhase2ServerPing
};

enum EGenericOption
{
	eGenericYes,
	eGenericNo,
	eGenericDontCare
};

enum EGenericInvertedOption
{
	eInvertedNo,
	eInvertedYes,
	eInvertedDontCare
};

enum EGamemodes
{
	eGamemodeAny,
	eGamemodeCTF,
	eGamemodeCP,
	eGamemodePayload,
	eGamemodeKOTH,
	eGamemodeBd,
	eGamemodeDom,
	eGamemodeArena,
	eGamemodeTC,
	eGamemodePD,
	eGamemodeSD,
	eGamemodeRD,
	eGamemodePasstime,
	eGamemodeMVM
};

enum ERespawnTimes
{
	eRespawnTimesDefault,
	eRespawnTimesInstant,
	eRespawnTimesDontCare
};

class ServerFinderOptions_t
{
public:
	ServerFinderOptions_t()
	{
		m_eRandomCrits = eGenericNo;
#if defined(TF_CLIENT_DLL)
		m_eDamageSpread = eInvertedNo;
		m_eGamemode = eGamemodeAny;
#endif

		m_eRespawnTimes = eRespawnTimesDefault;
		m_iMaxPlayers = 24;
		m_iMaxPing = 0;
	}

#if defined(TF_CLIENT_DLL)
	EGenericOption m_eRandomCrits;
	EGenericInvertedOption m_eDamageSpread;
	EGamemodes m_eGamemode;
#endif

	ERespawnTimes m_eRespawnTimes;

	int m_iMaxPlayers;
	int m_iMaxPing;
};

//-----------------------------------------------------------------------------
// Purpose: dialog for launching a listenserver
//-----------------------------------------------------------------------------
class CServerFinderDialog : public vgui::Frame, public ISteamMatchmakingServerListResponse, public ISteamMatchmakingPingResponse
{
	DECLARE_CLASS_SIMPLE( CServerFinderDialog,  vgui::Frame );

public:
	CServerFinderDialog(vgui::Panel *parent);
	~CServerFinderDialog();
	
	// returns currently entered information about the server
	void SetMap(const char *name);
	void GetOptions();
	void SetParams();
	bool IsRandomMapSelected();
	const char *GetMapName();

	//
	// ISteamMatchmakingServerListResponse overrides
	//

	virtual void ServerResponded(HServerListRequest hRequest, int iServer);
	virtual void ServerFailedToRespond(HServerListRequest hRequest, int iServer);
	virtual void RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response);

	//
	// ISteamMatchmakingPingResponse overrides
	//

	virtual void ServerResponded(gameserveritem_t& server);
	virtual void ServerFailedToRespond();

	virtual void OnThink();

private:
	virtual void OnCommand( const char *command );
	virtual void OnClose();
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	
	void BeginSearch();
	void OnSearchFailure();
	void SaveOptionSelection(bool reload = true);
	void LoadMapList();
	void LoadMaps( const char *pszPathID );
	void JoinServer(gameserveritem_t& server);
	void ServerResponded(gameserveritem_ex_t serverex);
	void PingNextBestServer();

	void DestroyServerListRequest()
	{
		if (m_hServerListRequest)
		{
			steamapicontext->SteamMatchmakingServers()->ReleaseRequest(m_hServerListRequest);
			m_hServerListRequest = NULL;
		}
	}

	void DestroyServerQueryRequest()
	{
		if (m_hServerQueryRequest != HSERVERQUERY_INVALID)
		{
			steamapicontext->SteamMatchmakingServers()->CancelServerQuery(m_hServerQueryRequest);
			m_hServerQueryRequest = HSERVERQUERY_INVALID;
		}
	}

	HServerListRequest m_hServerListRequest;
	HServerQuery m_hServerQueryRequest;

	vgui::ComboBox *m_pMapList;
	vgui::TextEntry* m_pMaxPlayers;
	vgui::TextEntry* m_pMaxPing;
#if defined(TF_CLIENT_DLL)
	vgui::ComboBox *m_pRandCrits;
	vgui::ComboBox* m_pDmgSpread;
	vgui::ComboBox* m_pGamemode;
#endif
	vgui::ComboBox* m_pRespawnTimes;
	vgui::Label* m_pStatus;
	ServerFinderOptions_t *m_pOptions;

	// for loading/saving game config
	KeyValues *m_pSavedData;

	CBlacklistedServerManager m_blackList;

	CUtlSortVector<gameserveritem_ex_t> m_vecServerJoinQueue;
	double m_timePingServerTimeout;
	int m_iRetries;
	EServerfinderMode m_Mode;
};

#endif