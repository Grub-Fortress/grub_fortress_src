//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ServerFinderDialog.h"

#include <stdio.h>

using namespace vgui;

#include <vgui/ILocalize.h>

#include "filesystem.h"
#include <KeyValues.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/CheckButton.h>
#include "tier1/convar.h"
#include "ServerBrowser/blacklisted_server_manager.h"
#include "ServerBrowser/IServerBrowser.h"
#include "tier1/netadr.h"
#include "vgui_controls/MessageBox.h"

// for SRC
#include <vstdlib/random.h>

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

#define RANDOM_MAP "#Serverfinder_AnyMap"
#define SERVERFINDER_CONFIG "ServerFinderConfig.vdf"

extern const char* COM_GetModDirectory();

#ifdef SERVERFINDER_FULLDEBUG
ConVar serverfinder_debug("serverfinder_debug", "2", FCVAR_NONE);
#else
ConVar serverfinder_debug("serverfinder_debug", "1", FCVAR_DEVELOPMENTONLY);
#endif

ConVar tfgrub_serverfinder_create_server("tfgrub_serverfinder_create_server", "0", FCVAR_ARCHIVE);

void LoadCommand(void)
{
	CServerFinderDialog* pCServerFinderDialog = new CServerFinderDialog(NULL);
	pCServerFinderDialog->Activate();
}

ConCommand serverfinderdialog("serverfinderdialog", LoadCommand, "", FCVAR_NONE);

static void AddFilter(CUtlVector<MatchMakingKeyValuePair_t>& vecServerFilters, const char* pchKey, const char* pchValue)
{
	// @note Tom Bui: this is to get around the linker error where we don't like strncpy in MatchMakingKeyValuePair_t's constructor!
	int idx = vecServerFilters.AddToTail();
	MatchMakingKeyValuePair_t& keyvalue = vecServerFilters[idx];
	Q_strncpy(keyvalue.m_szKey, pchKey, sizeof(keyvalue.m_szKey));
	Q_strncpy(keyvalue.m_szValue, pchValue, sizeof(keyvalue.m_szValue));
}

static bool BHasTag(const CUtlStringList& TagList, const char* tag)
{
	for (int i = 0; i < TagList.Count(); i++)
	{
		if (!Q_stricmp(TagList[i], tag))
		{
			return true;
		}
	}
	return false;
}

bool HasAppropriateTags(const CUtlStringList& TagList, const CUtlStringList& Req, const CUtlStringList& Rej)
{
	FOR_EACH_VEC(Req, idx)
	{
		if (!BHasTag(TagList, Req[idx]))
		{
			SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "Cannot find required tag %s\n", Req[idx]);
			return false;
		}
	}

	FOR_EACH_VEC(Rej, idx)
	{
		if (BHasTag(TagList, Rej[idx]))
		{
			SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "Found rejected tag %s\n", Rej[idx]);
			return false;
		}
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CServerFinderDialog::CServerFinderDialog(vgui::Panel *parent) : BaseClass(NULL, "ServerFinderConfig")
{
	m_hServerListRequest = NULL;
	m_hServerQueryRequest = NULL;
	m_iRetries = 0;
	m_timePingServerTimeout = 0.0;
	m_Mode = eServerfinderPhase0Nothing;

	SetSize(460, 460);
	//SetOKButtonText("#GameUI_Start");

	m_pOptions = new ServerFinderOptions_t();
	
	m_pMapList = new ComboBox(this, "MapList", 12, false);
	m_pMaxPlayers = new TextEntry( this, "MaxPlayers" );
	m_pMaxPing = new TextEntry(this, "MaxPing");
	m_pStatus = new Label(this, "StatusLabel", "#Serverfinder_Status_Idle");
#if defined(TF_CLIENT_DLL)
	m_pRandCrits = new ComboBox(this, "RandCrits", 12, false);
	m_pRandCrits->AddItem("#Serverfinder_RandomCrits_Enabled", NULL);
	m_pRandCrits->AddItem("#Serverfinder_RandomCrits_Default", NULL);
	m_pRandCrits->AddItem("#TF_Quickplay_RandomCrits_DontCare", NULL);

	m_pDmgSpread = new ComboBox(this, "DmgSpread", 12, false);
	m_pDmgSpread->AddItem("#TF_Quickplay_DamageSpread_Default", NULL);
	m_pDmgSpread->AddItem("#TF_Quickplay_DamageSpread_Enabled", NULL);
	m_pDmgSpread->AddItem("#TF_Quickplay_DamageSpread_DontCare", NULL);

	m_pGamemode = new ComboBox(this, "Gamemode", 13, false);
	m_pGamemode->AddItem("#Serverfinder_AnyGamemode", NULL);
	m_pGamemode->AddItem("#Gametype_CTF", NULL);
	m_pGamemode->AddItem("#Gametype_CP", NULL);
	m_pGamemode->AddItem("#Gametype_Escort", NULL);
	m_pGamemode->AddItem("#Gametype_Koth", NULL);
	m_pGamemode->AddItem("#Gametype_Bd", NULL);
	m_pGamemode->AddItem("#Gametype_Dom", NULL);
	m_pGamemode->AddItem("#Gametype_Arena", NULL);
	m_pGamemode->AddItem("#GameType_TerritorialControl", NULL);
	m_pGamemode->AddItem("#Gametype_PlayerDestruction", NULL);
	m_pGamemode->AddItem("#Gametype_SD", NULL);
	m_pGamemode->AddItem("#Gametype_RobotDestruction", NULL);
	m_pGamemode->AddItem("#GameType_Passtime", NULL);
	m_pGamemode->AddItem("#Gametype_MVM", NULL);
#endif

	m_pRespawnTimes = new ComboBox(this, "RespawnTimes", 12, false);
	m_pRespawnTimes->AddItem("#Serverfinder_RespawnTimes_Default", NULL);
	m_pRespawnTimes->AddItem("#Serverfinder_RespawnTimes_Instant", NULL);
	m_pRespawnTimes->AddItem("#Serverfinder_RespawnTimes_DontCare", NULL);

	LoadMapList();

	// create KeyValues object to load/save config options
	m_pSavedData = new KeyValues("ServerFinderConfig");

	// load the config data
	if (m_pSavedData)
	{
		m_pSavedData->LoadFromFile(g_pFullFileSystem, SERVERFINDER_CONFIG, "GAME"); // this is game-specific data, so it should live in GAME, not CONFIG
		SetParams();
	}

	GetOptions();

	LoadControlSettings("Resource/ServerFinderConfig.res");

	SetSizeable(false);
	SetDeleteSelfOnClose(true);
	MoveToCenterOfScreen();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CServerFinderDialog::~CServerFinderDialog()
{
	if (m_pSavedData)
	{
		m_pSavedData->deleteThis();
		m_pSavedData = NULL;
	}
}

void CServerFinderDialog::SaveOptionSelection(bool reload)
{
	char szMapName[64];
	Q_strncpy(szMapName, GetMapName(), sizeof(szMapName));

	char szMaxBots[256];
	m_pMaxPlayers->GetText(szMaxBots, sizeof(szMaxBots));

	char szMaxPing[256];
	m_pMaxPing->GetText(szMaxPing, sizeof(szMaxPing));

	// save the config data
	if (m_pSavedData)
	{
		// even if we have a random map selected, always save the map so we can queue for games with this map.
		m_pSavedData->SetString("map", szMapName);
		// if random map is a thing though, add a boolean to tell the ui to select the random map when it loads.
		m_pSavedData->SetBool("israndommap", IsRandomMapSelected());
		m_pSavedData->SetString("MaxPlayers", szMaxBots);
		m_pSavedData->SetString("MaxPing", szMaxPing);
#if defined(TF_CLIENT_DLL)
		m_pSavedData->SetInt("RandomCrits", (EGenericOption)m_pRandCrits->GetItemIDFromRow(m_pRandCrits->GetActiveItem()));
		m_pSavedData->SetInt("DamageSpread", (EGenericInvertedOption)m_pDmgSpread->GetItemIDFromRow(m_pDmgSpread->GetActiveItem()));
		m_pSavedData->SetInt("Gamemode", (EGenericOption)m_pGamemode->GetItemIDFromRow(m_pGamemode->GetActiveItem()));
#endif
		m_pSavedData->SetInt("RespawnTimes", (ERespawnTimes)m_pRespawnTimes->GetItemIDFromRow(m_pRespawnTimes->GetActiveItem()));

		// save config to a file
		m_pSavedData->SaveToFile(g_pFullFileSystem, SERVERFINDER_CONFIG, "GAME");

		if (reload)
		{
			// reload the saved file.
			SetParams();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CServerFinderDialog::OnClose()
{
	BaseClass::OnClose();
	
	m_vecServerJoinQueue.RemoveAll();
	SaveOptionSelection(false);

	MarkForDeletion();
}

//
// ISteamMatchmakingServerListResponse overrides
//

void CServerFinderDialog::ServerResponded(HServerListRequest hRequest, int iServer)
{
	gameserveritem_t* server = steamapicontext->SteamMatchmakingServers()->GetServerDetails(hRequest, iServer);
	if (server)
	{
		gameserveritem_ex_t foundServer;
		foundServer.server = *server;
		foundServer.m_nRealPlayers = clamp((foundServer.server.m_nPlayers - foundServer.server.m_nBotPlayers), 0, foundServer.server.m_nMaxPlayers);
		ServerResponded(foundServer);
	}
}

void CServerFinderDialog::ServerFailedToRespond(HServerListRequest hRequest, int iServer)
{
}

void CServerFinderDialog::RefreshComplete(HServerListRequest hRequest, EMatchMakingServerResponse response)
{
	DestroyServerListRequest();

	if (m_vecServerJoinQueue.Count() > 0)
	{
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "Found servers:\n");
		FOR_EACH_VEC(m_vecServerJoinQueue, id)
		{
			gameserveritem_ex_t item = m_vecServerJoinQueue[id];
			SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "%i | %s | %i/%i | %s\n",
				id,
				item.server.GetName(), 
				item.m_nRealPlayers,
				item.server.m_nMaxPlayers,
				item.server.m_NetAdr.GetConnectionAddressString());
		}

		// join a random server from the queue and ping.
		// LET'S GO GAMBLING!
		m_Mode = eServerfinderPhase2ServerPing;
		m_pStatus->SetText("#Serverfinder_Status_FoundCandidates");
		PingNextBestServer();
	}
	else
	{
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "No servers found in queue.\n");
		m_pStatus->SetText("#Serverfinder_Status_Finished_Fail");
		if ( tfgrub_serverfinder_create_server.GetBool() )
		{
			OnSearchFailure();
		}
	}
}

//
// ISteamMatchmakingPingResponse overrides
//

void CServerFinderDialog::ServerResponded(gameserveritem_t& server)
{
	// we know it's a valid server, since we checked its details before.
	// so we join it!
	// I CAN'T STOP WINNING!
	m_pStatus->SetText("#Serverfinder_Status_Finished");
	JoinServer(server);
}

void CServerFinderDialog::ServerFailedToRespond()
{
	// Rescan...
	// AWH DANGIT!
	PingNextBestServer();
}

void CServerFinderDialog::OnThink()
{
	BaseClass::OnThink();

	double currentTime = Plat_FloatTime();

	if (m_Mode == eServerfinderPhase2ServerPing)
	{
		// Do our own timeout processing
		Assert(m_hServerQueryRequest != HSERVERQUERY_INVALID);
		if (currentTime > m_timePingServerTimeout || m_hServerQueryRequest == HSERVERQUERY_INVALID)
		{
			ServerFailedToRespond();
		}
	}
}

void CServerFinderDialog::PingNextBestServer()
{
	Assert(m_Mode == eServerfinderPhase2ServerPing);

	// If we already have any ping activity going, cancel it.  (We shouldn't)
	DestroyServerQueryRequest();

	// Any more options to try?
	if (m_iRetries >= SERVERFINDER_MAX_RETRIES || m_vecServerJoinQueue.Count() < 1)
	{
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "No more servers left to ping. We failed!\n");
		m_pStatus->SetText("#Serverfinder_Status_Finished_Fail");
		if ( tfgrub_serverfinder_create_server.GetBool() )
		{
			OnSearchFailure();
		}
		return;
	}
	else
	{
		m_iRetries++;
	}

	wchar_t szFmt[128] = L"";
	wchar_t szText[512] = L"";
	wchar_t szNumRetries[16] = L"";
	_snwprintf(szNumRetries, ARRAYSIZE(szNumRetries), L"%i", m_iRetries);

	const wchar_t* pchFmt = g_pVGuiLocalize->Find("#Serverfinder_Status_Pingsearch");
	if (!pchFmt || !pchFmt[0])
		return;
	Q_wcsncpy(szFmt, pchFmt, sizeof(szFmt));

	g_pVGuiLocalize->ConstructString_safe(szText, szFmt, 1, szNumRetries);

	m_pStatus->SetText(szText);

	// Remove the next address from the queue
	int randID = RandomInt(0, m_vecServerJoinQueue.Count() - 1);
	gameserveritem_ex_t randServer = m_vecServerJoinQueue[randID];
	// prevents us from choosing it again.
	m_vecServerJoinQueue.Remove(randID);

	// Do it
	SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "Pinging %s\n", randServer.server.m_NetAdr.GetConnectionAddressString());
	m_hServerQueryRequest = steamapicontext->SteamMatchmakingServers()->PingServer(
		randServer.server.m_NetAdr.GetIP(), 
		randServer.server.m_NetAdr.GetConnectionPort(), this);
	Assert(m_hServerQueryRequest != HSERVERQUERY_INVALID);

	// Set timeout.  Since we've already pinged them once
	// fairly recently, and we are considering joining this
	// server, we don't want to wait around forever.  They
	// should reply to the ping pretty quickly or let's
	// not join them.
	m_timePingServerTimeout = Plat_FloatTime() + 1.0;
}

void CServerFinderDialog::JoinServer(gameserveritem_t& server)
{
	SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER %s: Server validation succeeded. Attempting to join %s!\n", server.GetName(), server.m_NetAdr.GetConnectionAddressString());
	// if this server is valid, join it.
	char szJoinCommand[1024];
	// create the command to execute
	Q_snprintf(szJoinCommand, sizeof(szJoinCommand), "disconnect\nconnect %s", server.m_NetAdr.GetConnectionAddressString());
	// exec
	engine->ClientCmd_Unrestricted(szJoinCommand);
	OnClose();
}

void CServerFinderDialog::OnSearchFailure()
{
	ConVar* pPassword = cvar->FindVar("sv_password");
	if (pPassword)
	{
		pPassword->SetValue("");
	}

	char szMapCommand[1024];
	// create the command to execute
	if (StringHasPrefix(GetMapName(), "mvm_"))
	{
		Q_snprintf(szMapCommand, sizeof(szMapCommand),
			"exec serverfinder_fail_mvm.cfg\nmaxplayers %i\nmp_disable_respawn_times %i\ntf_weapon_criticals %i\ntf_damage_disablespread %i\nprogress_enable\nmap %s\n",
			m_pOptions->m_iMaxPlayers,
			(m_pOptions->m_eRespawnTimes == eRespawnTimesInstant ? 1 : (m_pOptions->m_eRespawnTimes == eRespawnTimesDontCare ? random->RandomInt(0, 1) : 0)),
			(m_pOptions->m_eRandomCrits == eGenericYes ? 1 : (m_pOptions->m_eRandomCrits == eGenericDontCare ? random->RandomInt(0, 1) : 0)),
			(m_pOptions->m_eDamageSpread == eInvertedNo ? 1 : (m_pOptions->m_eDamageSpread == eInvertedDontCare ? random->RandomInt(0, 1) : 0)),
			GetMapName());
	}
	else
	{
		Q_snprintf(szMapCommand, sizeof(szMapCommand),
			"exec serverfinder_fail.cfg\ntf_bot_quota %i\nmaxplayers %i\nmp_disable_respawn_times %i\ntf_weapon_criticals %i\ntf_damage_disablespread %i\nprogress_enable\nmap %s\n",
			(m_pOptions->m_iMaxPlayers - 1),
			m_pOptions->m_iMaxPlayers,
			(m_pOptions->m_eRespawnTimes == eRespawnTimesInstant ? 1 : (m_pOptions->m_eRespawnTimes == eRespawnTimesDontCare ? random->RandomInt(0, 1) : 0)),
			(m_pOptions->m_eRandomCrits == eGenericYes ? 1 : (m_pOptions->m_eRandomCrits == eGenericDontCare ? random->RandomInt(0, 1) : 0)),
			(m_pOptions->m_eDamageSpread == eInvertedNo ? 1 : (m_pOptions->m_eDamageSpread == eInvertedDontCare ? random->RandomInt(0, 1) : 0)),
			GetMapName());
	}

	// exec
	engine->ClientCmd_Unrestricted(szMapCommand);
	OnClose();
}

void CServerFinderDialog::ServerResponded(gameserveritem_ex_t serverex)
{
	SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER FOUND! Validating.\n");

	bool invalid = false;

	if (!serverex.server.m_bHadSuccessfulResponse)
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER INVALID: Response wasn't successful\n");
	}

	// Ignore servers with bogus address. Is this even possible?
	if (!serverex.server.m_NetAdr.GetIP() || !serverex.server.m_NetAdr.GetConnectionPort())
	{
		Assert(serverex.server.m_NetAdr.GetIP() && serverex.server.m_NetAdr.GetConnectionPort());
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER INVALID: Response was invalid\n");
	}

	if (m_blackList.IsServerBlacklisted(serverex.server))
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER INVALID: Server is blacklisted\n");
	}

	if (serverex.server.m_bPassword)
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER INVALID: Server is password protected\n");
	}

	//first, are the max players invalid?
	if (serverex.server.m_nMaxPlayers > m_pOptions->m_iMaxPlayers)
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER %s: Max players are too high\n", serverex.server.GetName());
	}

	// is the server full?
	if (serverex.m_nRealPlayers >= m_pOptions->m_iMaxPlayers)
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER %s: Server is full\n", serverex.server.GetName());
	}

#ifndef SERVERFINDER_CONNECTION_TEST 
	// is the server empty?
	if (serverex.m_nRealPlayers <= 0)
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER %s: Server is empty\n", serverex.server.GetName());
	}
#endif

	// is the ping good (if we have it set)?
	if ((m_pOptions->m_iMaxPing > 0) && (serverex.server.m_nPing > m_pOptions->m_iMaxPing))
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER %s: Server ping is really fucking bad\n", serverex.server.GetName());
	}

	CUtlStringList requiredTags;
	CUtlStringList illegalTags;

	//next, parse the tags.
	switch (m_pOptions->m_eRespawnTimes)
	{
		case eRespawnTimesDefault:
			illegalTags.CopyAndAddToTail("norespawntime");
			break;

		case eRespawnTimesInstant:
			requiredTags.CopyAndAddToTail("norespawntime");
			break;

		default:
			Assert(false);
		case eRespawnTimesDontCare:
			break;
	}

#if defined(TF_CLIENT_DLL)
	switch (m_pOptions->m_eRandomCrits)
	{
		case eGenericNo:
			illegalTags.CopyAndAddToTail("nocrits");
			break;

		case eGenericYes:
			requiredTags.CopyAndAddToTail("nocrits");
			break;

		default:
			Assert(false);
		case eGenericDontCare:
			break;
	}

	switch (m_pOptions->m_eDamageSpread)
	{
		case eInvertedNo:
			illegalTags.CopyAndAddToTail("dmgspread");
			break;

		case eInvertedYes:
			requiredTags.CopyAndAddToTail("dmgspread");
			break;

		default:
			Assert(false);
		case eInvertedDontCare:
			break;
	}

	const char* serverMap = serverex.server.m_szMap;
	if (!serverMap || !serverMap[0])
	{
		invalid = true;
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER %s: Server map is invalid!\n", serverex.server.GetName());
	}

	switch (m_pOptions->m_eGamemode)
	{
	case eGamemodeAny:
		break;

	case eGamemodeCTF:
		if (!StringHasPrefix(serverMap, "ctf_"))
			invalid = true;
		break;

	case eGamemodeCP:
		if (!StringHasPrefix(serverMap, "cp_"))
			invalid = true;
		break;

	case eGamemodePayload:
		if (!StringHasPrefix(serverMap, "pl_") &&
			!StringHasPrefix(serverMap, "plr_"))
			invalid = true;
		break;

	case eGamemodeKOTH:
		if (!StringHasPrefix(serverMap, "koth_"))
			invalid = true;
		break;

	case eGamemodeBd:
		if (!StringHasPrefix(serverMap, "bd_"))
			invalid = true;
		break;

	case eGamemodeDom:
		if (!StringHasPrefix(serverMap, "dom_"))
			invalid = true;
		break;

	case eGamemodeArena:
		if (!StringHasPrefix(serverMap, "arena_"))
			invalid = true;
		break;

	case eGamemodeTC:
		if (!StringHasPrefix(serverMap, "tc_"))
			invalid = true;
		break;

	case eGamemodePD:
		if (!StringHasPrefix(serverMap, "pd_"))
			invalid = true;
		break;

	case eGamemodeSD:
		if (!StringHasPrefix(serverMap, "sd_"))
			invalid = true;
		break;

	case eGamemodeRD:
		if (!StringHasPrefix(serverMap, "rd_"))
			invalid = true;
		break;

	case eGamemodePasstime:
		if (!StringHasPrefix(serverMap, "pass_"))
			invalid = true;
		break;

	case eGamemodeMVM:
		if (!StringHasPrefix(serverMap, "mvm_"))
			invalid = true;
		break;

		// older variant of the gamemode checker		
/*
		case eGamemodeAny:
			break;
		case eGamemodeCTF:
			requiredTags.CopyAndAddToTail("ctf");
			break;
		case eGamemodeCP:
			requiredTags.CopyAndAddToTail("cp");
			break;
		case eGamemodeArena:
			requiredTags.CopyAndAddToTail("arena");
			break;
		case eGamemodeSD:
			requiredTags.CopyAndAddToTail("sd");
			break;
		case eGamemodeMVM:
			requiredTags.CopyAndAddToTail("mvm");
			break;
		case eGamemodePayload:
			requiredTags.CopyAndAddToTail("payload");
			break;
		case eGamemodeRD:
			requiredTags.CopyAndAddToTail("rd");
			break;
		case eGamemodePD:
			requiredTags.CopyAndAddToTail("pd");
			break;
		case eGamemodeTC:
			requiredTags.CopyAndAddToTail("tc");
			break;
		case eGamemodePasstime:
			requiredTags.CopyAndAddToTail("passtime");
			break;
		case eGamemodeMisc:
			requiredTags.CopyAndAddToTail("misc");
			break;
*/
		default:
			Assert(false);
	}
#endif

	CUtlStringList TagList; // auto-deletes strings on scope exit
	if (serverex.server.m_szGameTags && serverex.server.m_szGameTags[0])
	{
		V_SplitString(serverex.server.m_szGameTags, ",", TagList);
	}

	if (!HasAppropriateTags(TagList, requiredTags, illegalTags))
	{
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER %s: Server has illegal tags or is missing required tags!\n", serverex.server.GetName());
		invalid = true;
	}

	// we join the first valid server regardless of ping.
	if (!invalid)
	{
		// if a server matches our filters, we add it to the queue.
		m_vecServerJoinQueue.InsertNoSort(serverex);
	}
	else
	{
		SERVERFINDER_SPEW(SERVERFINDER_LEVEL_VALIDATION, "SERVER: Server validation failed.\n");
		if ( tfgrub_serverfinder_create_server.GetBool() )
		{
			OnSearchFailure();
		}
	}
}

void CServerFinderDialog::BeginSearch()
{ 
	m_Mode = eServerfinderPhase1ServerQueue;
	m_blackList.LoadServersFromFile(BLACKLIST_DEFAULT_SAVE_FILE, false);

	// Setup search filters
	CUtlVector<MatchMakingKeyValuePair_t> vecServerFilters;
	AddFilter(vecServerFilters, "gamedir", COM_GetModDirectory());
	// source sdk 2013 has no secure servers......
	/*if (GetUniverse() == k_EUniversePublic)
	{
		AddFilter(vecServerFilters, "secure", "1");
		AddFilter(vecServerFilters, "dedicated", "1");
	}*/
	if (!IsRandomMapSelected())
	{
		AddFilter(vecServerFilters, "map", GetMapName() );
	}

	SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "Beginning server search...\n");
	m_pStatus->SetText("#Serverfinder_Status_Searching");

	CUtlString sFilter;
	FOR_EACH_VEC(vecServerFilters, idx)
	{
		sFilter.Append(vecServerFilters[idx].m_szKey);
		sFilter.Append(" - ");
		sFilter.Append(vecServerFilters[idx].m_szValue);
		sFilter.Append("\n");
	}
	SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "Using filter: %s\n", sFilter.String());

	MatchMakingKeyValuePair_t* pFilters = vecServerFilters.Base(); // <<<< Note, this is weird, but correct.
	m_hServerListRequest = steamapicontext->SteamMatchmakingServers()->RequestInternetServerList(
		engine->GetAppID(),
		&pFilters,
		vecServerFilters.Count(),
		this);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *command - 
//-----------------------------------------------------------------------------
void CServerFinderDialog::OnCommand(const char *command)
{
	if ( !stricmp( command, "Ok" ) )
	{
		SaveOptionSelection();

#ifndef SERVERFINDER_CONNECTION_TEST
		if (engine->IsInGame())
		{
			SERVERFINDER_SPEW(SERVERFINDER_LEVEL_INTERNAL, "Already connected to server.\n");
			vgui::MessageBox* pMessageBox = new vgui::MessageBox("#Serverfinder_Title", "#Serverfinder_Error_CannotJoinConnected", GetParent());
			pMessageBox->DoModal();
			OnClose();
			return;
		}
#endif

		// SEARCH HERE.
		BeginSearch();
		
		return;
	}

	BaseClass::OnCommand( command );
}

void CServerFinderDialog::OnKeyCodeTyped(KeyCode code)
{
	// force ourselves to be closed if the escape key it pressed
	if (code == KEY_ESCAPE)
	{
		Close();
	}
	else
	{
		BaseClass::OnKeyCodeTyped(code);
	}
}

//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CServerFinderDialog::LoadMaps(const char *pszPathID)
{
	FileFindHandle_t findHandle = NULL;

	const char *pszFilename = g_pFullFileSystem->FindFirst( "maps/*.bsp", &findHandle );
	while ( pszFilename )
	{
		char mapname[256];

		const char *str = nullptr;
		char *ext = nullptr;

		// FindFirst ignores the pszPathID, so check it here
		// TODO: this doesn't find maps in fallback dirs
		Q_snprintf( mapname, sizeof(mapname), "maps/%s", pszFilename );
		if ( !g_pFullFileSystem->FileExists( mapname, pszPathID ) )
		{
			goto nextFile;
		}

		// remove the text 'maps/' and '.bsp' from the file name to get the map name
		
		str = Q_strstr( pszFilename, "maps" );
		if ( str )
		{
			Q_strncpy( mapname, str + 5, sizeof(mapname) - 1 );	// maps + \\ = 5
		}
		else
		{
			Q_strncpy( mapname, pszFilename, sizeof(mapname) - 1 );
		}
		ext = Q_strstr( mapname, ".bsp" );
		if ( ext )
		{
			*ext = 0;
		}

		// add to the map list
		m_pMapList->AddItem( mapname, new KeyValues( "data", "mapname", mapname ) );

		// get the next file
	nextFile:
		pszFilename = g_pFullFileSystem->FindNext( findHandle );
	}
	g_pFullFileSystem->FindClose( findHandle );
}



//-----------------------------------------------------------------------------
// Purpose: loads the list of available maps into the map list
//-----------------------------------------------------------------------------
void CServerFinderDialog::LoadMapList()
{
	// clear the current list (if any)
	m_pMapList->DeleteAllItems();

	// add special "name" to represent loading a randomly selected map
	m_pMapList->AddItem( RANDOM_MAP, new KeyValues( "data", "mapname", RANDOM_MAP ) );

	// iterate the filesystem getting the list of all the files
	// UNDONE: steam wants this done in a special way, need to support that
	const char *pathID = "GAME";

	// Load the GameDir maps
	LoadMaps( pathID ); 

	// set the first item to be selected
	m_pMapList->ActivateItem( 0 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CServerFinderDialog::IsRandomMapSelected()
{
	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!stricmp( mapname, RANDOM_MAP ))
	{
		return true;
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CServerFinderDialog::GetMapName()
{
	int count = m_pMapList->GetItemCount();

	// if there is only one entry it's the special "select random map" entry
	if( count <= 1 )
		return NULL;

	const char *mapname = m_pMapList->GetActiveItemUserData()->GetString("mapname");
	if (!strcmp( mapname, RANDOM_MAP ))
	{
		int which = RandomInt( 1, count - 1 );
		mapname = m_pMapList->GetItemUserData( which )->GetString("mapname");
	}

	return mapname;
}

//-----------------------------------------------------------------------------
// Purpose: Sets currently selected map in the map combobox
//-----------------------------------------------------------------------------
void CServerFinderDialog::SetMap(const char *mapName)
{
	for (int i = 0; i < m_pMapList->GetItemCount(); i++)
	{
		if (!m_pMapList->IsItemIDValid(i))
			continue;

		if (!stricmp(m_pMapList->GetItemUserData(i)->GetString("mapname"), mapName))
		{
			m_pMapList->ActivateItem(i);
			break;
		}
	}
}

int clampToPlayerCount(int violator)
{
	return clamp(violator, SERVERFINDER_MIN_PLAYERS, (MAX_PLAYERS - 1));
}

int clampToPingNum(int violator)
{
	return clamp(violator, 0, SERVERFINDER_MAX_PING);
}

void CServerFinderDialog::SetParams()
{
	const char* startMap = m_pSavedData->GetString("map", "");
	if (startMap[0])
	{
		// select the map if its non-random.
		bool bIsRandom = m_pSavedData->GetBool("israndommap", false);
		if (!bIsRandom)
		{
			SetMap(startMap);
		}
	}

	const char* startMax = m_pSavedData->GetString("MaxPlayers", "");
	if (startMax[0])
	{
		m_pOptions->m_iMaxPlayers = clampToPlayerCount(atoi(startMax));
	}

	const char* startMaxPing = m_pSavedData->GetString("MaxPing", "");
	if (startMaxPing[0])
	{
		m_pOptions->m_iMaxPing = clampToPingNum(atoi(startMaxPing));
	}

	const char* startRandomCrits = m_pSavedData->GetString("RandomCrits", "");
	if (startRandomCrits[0])
	{
		m_pOptions->m_eRandomCrits = (EGenericOption)atoi(startRandomCrits);
	}

	const char* startDamageSpread = m_pSavedData->GetString("DamageSpread", "");
	if (startDamageSpread[0])
	{
		m_pOptions->m_eDamageSpread = (EGenericInvertedOption)atoi(startDamageSpread);
	}

	const char* startGamemode = m_pSavedData->GetString("Gamemode", "");
	if (startGamemode[0])
	{
		m_pOptions->m_eGamemode = (EGamemodes)atoi(startGamemode);
	}

	const char* startRespawnTimes = m_pSavedData->GetString("RespawnTimes", "");
	if (startRespawnTimes[0])
	{
		m_pOptions->m_eRespawnTimes = (ERespawnTimes)atoi(startRespawnTimes);
	}
}

void CServerFinderDialog::GetOptions()
{
	int maxPlayers = clampToPlayerCount(m_pOptions->m_iMaxPlayers);
	if (maxPlayers > 0)
	{
		char szCount[16];
		Q_snprintf(szCount, sizeof(szCount), "%i", maxPlayers);
		m_pMaxPlayers->SetText(szCount);
	}

	// ping can be > 0 or higher
	int maxPing = clampToPingNum(m_pOptions->m_iMaxPing);

	char szCount[16];
	Q_snprintf(szCount, sizeof(szCount), "%i", maxPing);
	m_pMaxPing->SetText(szCount);

	m_pRandCrits->ActivateItem((int)m_pOptions->m_eRandomCrits);
	m_pDmgSpread->ActivateItem((int)m_pOptions->m_eDamageSpread);
	m_pGamemode->ActivateItem((int)m_pOptions->m_eGamemode);
	m_pRespawnTimes->ActivateItem((int)m_pOptions->m_eRespawnTimes);
}