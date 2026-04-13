"GameMenu" [$WIN32]
{
	"ResumeGameButton"
	{
		"label"			"#MMenu_ResumeGame"
		"command"		"ResumeGame"
		"OnlyInGame"	"1"
		"subimage" "icon_resume"
	}
	"ServerBrowserButton"
	{
		"label" "#MMenu_BrowseServers" 
		"command" "OpenServerBrowser"
		"subimage" "glyph_spy_glass"
		"OnlyAtMenu" "1"
	}
	"ServerBrowserIngameButton"
	{
		"label" "#MMenu_BrowseServers" 
		"command" "OpenServerBrowser"
		"subimage" "glyph_multiplayer"
		"OnlyInGame" "1"
	}
	"CreateServerButton"
	{
		"label" "#GameUI_GameMenu_CreateServer"
		"command" "modcreateserver"
		"subimage" "glyph_create"
		"OnlyAtMenu" "1"
	}
	"OptionsButton"
	{
		"label" "#GameUI_GameMenu_Options" 
		"command" "OpenOptionsDialog"
		"subimage" "glyph_options"
	} 
	"ReplayBrowserButton"
	{
		"label" ""
		"command" "engine replay_reloadbrowser"
		"subimage" "glyph_tv"
		"tooltip" "#GameUI_GameMenu_ReplayDemos"
	}
	"CreditsButton"
	{
		"label" ""
		"command" "openmodcredits"
		"subimage" "glyph_steamworkshop"
		"tooltip" "#Gameui_Gamemenu_Credits"
	}
	
	// These buttons get positioned by the MainMenu.res	
	"AdvOptionsButton"
	{
		"label" ""
		"command" "opentf2options"
		"subimage" "glyph_options"
		"tooltip" "#MMenu_AdvOptions"
	}
	"AchievementsButton"
	{
		"label" ""
		"command" "OpenAchievementsDialog"
		"subimage" "glyph_achievements"
		"tooltip" "#TFGRUB_MMenu_Achievements"
	}	
	"CharacterSetupButton"
	{
		"label" "#MMenu_CharacterSetup"
		"command" "engine open_charinfo"
		"subimage" "glyph_items"
	}

	// These buttons are only shown while in-game
	// and also are positioned by the .res file
	"CallVoteButton"
	{
		"label"			""
		"command"		"callvote"
		"OnlyInGame"	"1"
		"subimage" "icon_checkbox"
		"tooltip" "#MMenu_CallVote"
	}
	"MutePlayersButton"
	{
		"label"			""
		"command"		"OpenPlayerListDialog"
		"OnlyInGame"	"1"
		"subimage" "glyph_muted"
		"tooltip" "#MMenu_MutePlayers"
	}
	"RequestCoachButton"
	{
		"label"			""
		"command"		"engine cl_coach_find_coach"
		"OnlyInGame"	"1"
		"subimage" "icon_whistle"
		"tooltip" "#MMenu_RequestCoach"
	}
	"LegacyCreateServer"
	{
		"label" ""
		"command" "OpenCreateMultiplayerGameDialog"
		"subimage" "glyph_create"
		"tooltip"	"Legacy Create Server Menu"
		"OnlyAtMenu" "1"
	}
	"ServerFinderButton"
	{
		"label"	""
		"command" 	"engine serverfinderdialog"
		"subimage" "glyph_multiplayer"
		"tooltip"	"#Serverfinder_Title"
		"OnlyAtMenu" "1"
	}
	"QuitButton_New"
	{
		"label" "#TF_Quit_Title"
		"command" "engine showquitconfirmdialog"
		"subimage" "glyph_quit"
		"OnlyAtMenu" "1"
	}
	"DisconnectButton_New"
	{
		"label" "#GameUI_GameMenu_Disconnect"
		"command" "engine showdisconnectconfirmdialog"
		"subimage" "glyph_quit"
		"OnlyInGame" "1"
	}
}
