//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"

#include <vgui_controls/Label.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ListPanel.h>
#include <vgui_controls/PanelListPanel.h>
#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IInput.h>
#include "ienginevgui.h"
#include "gf_advcreditspanel.h"
#include "filesystem.h"
#include "fmtstr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

CPFAdvCreditsPanel *g_pPFAdvCreditsPanel = NULL;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPFAdvCreditsPanel *GPFAdvCreditsPanel()
{
	if ( NULL == g_pPFAdvCreditsPanel )
	{
		g_pPFAdvCreditsPanel = new CPFAdvCreditsPanel();
	}
	return g_pPFAdvCreditsPanel;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DestroyPFAdvCreditsPanel()
{
	if ( NULL != g_pPFAdvCreditsPanel )
	{
		delete g_pPFAdvCreditsPanel;
		g_pPFAdvCreditsPanel = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPFAdvCreditsPanel::CPFAdvCreditsPanel() : BaseClass( NULL, "PFAdvCreditsPanel", 
													 vgui::scheme()->LoadSchemeFromFile( "Resource/ClientScheme.res", "ClientScheme" ) )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
void CPFAdvCreditsPanel::Init( void )
{
	vgui::VPANEL pParent = enginevgui->GetPanel( PANEL_GAMEUIDLL );
	SetParent( pParent );
	MakePopup( true );

	m_pListPanel = new PanelListPanel( this, "PanelListPanel" );

	// Check for credits file
	KeyValues* kv = new KeyValues( "Credits" );
	if ( kv->LoadFromFile( filesystem, "scripts/credits.txt", "MOD" ) )
		m_pCreditsKV = kv;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CPFAdvCreditsPanel::CPFAdvCreditsPanel( vgui::Panel *parent ) : BaseClass( parent, "PFAdvCreditsPanel", 
																		  vgui::scheme()->LoadSchemeFromFile( "Resource/ClientScheme.res", "ClientScheme" ) )
{
	Init();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPFAdvCreditsPanel::~CPFAdvCreditsPanel()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvCreditsPanel::PerformLayout()
{
	BaseClass::PerformLayout();
}


//-----------------------------------------------------------------------------
// Purpose: Applies scheme settings
//-----------------------------------------------------------------------------
void CPFAdvCreditsPanel::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetProportional( true );
	LoadControlSettings( "Resource/UI/creditsdialog.res" );
	CreateControls();
}

//-----------------------------------------------------------------------------
// Purpose: Command handler
//-----------------------------------------------------------------------------
void CPFAdvCreditsPanel::OnCommand( const char* command )
{
	if ( 0 == Q_stricmp( command, "Close" ) )
	{
		SetVisible( false );
		DestroyControls();
	}
}

static const char* s_CreditRoles[][ 2 ] =
{
	{ "developers", "#PF_Credit_Title_Developers" },
	{ "contributors", "#PF_Credit_Title_Contributors" },
	{ "playtesters", "#PF_Credit_Title_Testers" },
	{ "special", "#PF_Credit_Title_Special" },
	{ "software", "#PF_Credit_Title_Software" },
};

// excludes "other" as we handle that differently
static const char* s_CreditKeyStrings[][ 2 ] =
{
	{ "lead", "#PF_Credit_Lead" },
	{ "code", "#PF_Credit_Code" },
	{ "models", "#PF_Credit_Models" },
	{ "textures", "#PF_Credit_Textures" },
	{ "animation", "#PF_Credit_Animation" },
	{ "particles", "#PF_Credit_Particles" },
	{ "mapper", "#PF_Credit_Mapper" },
	{ "tester", "#PF_Credit_Tester" },
	{ "concepts", "#PF_Credit_Concepts"}
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvCreditsPanel::CreateControls()
{
	DestroyControls();

	// Create credit entries
	if ( m_pCreditsKV )
	{
		Label* pCreditEntry = nullptr;
		Label* pTitle = nullptr;
		IScheme* pScheme = scheme()->GetIScheme( GetScheme() );

		for ( KeyValues* kv = m_pCreditsKV->GetFirstSubKey(); kv; kv = kv->GetNextKey() )
		{
			char szTitleName[ 32 ];
			bool bRoleFound = false;
			for ( int i = 0; i < ARRAYSIZE( s_CreditRoles ) && !bRoleFound; ++i )
			{
				if( !Q_stricmp( kv->GetName(), s_CreditRoles[ i ][ 0 ] ) )
				{
					bRoleFound = true;
					Q_snprintf( szTitleName, sizeof( szTitleName ), s_CreditRoles[ i ][ 1 ] );
				}
			}
			if ( !bRoleFound )
				continue;

			pTitle = new Label( this, "Title", "" );
			pTitle->SetText( g_pVGuiLocalize->Find( szTitleName ) );

			pTitle->SetBorder( pScheme->GetBorder( "OptionsCategoryBorder" ) );
			pTitle->SetFont( pScheme->GetFont( "HudFontSmallBold", true ) );
			pTitle->SetFgColor( pScheme->GetColor( "AdvTextDefault", COLOR_WHITE ) );
			pTitle->SetSize( m_iControlWidth, m_iTitleHeight );
			m_pListPanel->AddItem( pTitle, pTitle );

			for ( KeyValues* kvDevs = kv->GetFirstSubKey(); kvDevs; kvDevs = kvDevs->GetNextKey() )
			{
				const char* name = kvDevs->GetName();
				if ( name )
				{
					char roles[ 1024 ] = "";
					bool bMultipleRoles = false;
					for ( int i = 0; i < ARRAYSIZE( s_CreditKeyStrings ); ++i )
					{
						if ( kvDevs->GetString( s_CreditKeyStrings[ i ][ 0 ], nullptr ) )
						{
							char szStringToLocalize[ 64 ];
							Q_snprintf( szStringToLocalize, sizeof( szStringToLocalize ), s_CreditKeyStrings[ i ][ 1 ] );

							char szLocalized[ 512 ];
							g_pVGuiLocalize->ConvertUnicodeToANSI( g_pVGuiLocalize->Find( szStringToLocalize ), szLocalized, sizeof( szLocalized ) );
							Q_strcat( roles, bMultipleRoles ? VarArgs( ", %s", szLocalized ) : szLocalized, sizeof( roles ) );
							bMultipleRoles = true;
						}
					}
					// Check for other subkeys
					for ( KeyValues* pSubKey = kvDevs->GetFirstSubKey(); pSubKey; pSubKey = pSubKey->GetNextKey() )
					{
						if ( !Q_stricmp( pSubKey->GetName(), "localization" ) )
						{
							bool bFound = false;
							for ( KeyValues* kvLocalizations = pSubKey->GetFirstSubKey(); kvLocalizations; kvLocalizations = kvLocalizations->GetNextKey() )
							{
								const char* name = kvLocalizations->GetName();
								if ( name )
								{
									// Construct a "#GameUI_Language_" string so we can find the translation
									char szUILanguageString[ 128 ] = "";
									Q_snprintf( szUILanguageString, sizeof( szUILanguageString ), "#GameUI_Language_%s", name );

									wchar_t wzLocalizationCredit[ 128 ];
									g_pVGuiLocalize->ConstructString( wzLocalizationCredit, sizeof( wzLocalizationCredit ), g_pVGuiLocalize->Find( "#PF_Credit_Localization_fmt" ), 1, g_pVGuiLocalize->Find( szUILanguageString ) );
									char szLocalizationCredit[ 128 ];
									g_pVGuiLocalize->ConvertUnicodeToANSI( wzLocalizationCredit, szLocalizationCredit, sizeof( szLocalizationCredit ) );

									Q_strcat( roles, bMultipleRoles ? VarArgs( ", %s", szLocalizationCredit ) : szLocalizationCredit, sizeof( roles ) );

									bMultipleRoles = true;
									bFound = true;
								}
							}
							// "localization" entry found, but no keyvalues
							if ( !bFound )
							{
								char szLocalized[ 128 ];
								g_pVGuiLocalize->ConvertUnicodeToANSI( g_pVGuiLocalize->Find( "#PF_Credit_Localization" ), szLocalized, sizeof( szLocalized ) );
								Q_strcat( roles, bMultipleRoles ? VarArgs( ", %s", szLocalized ) : szLocalized, sizeof( roles ) );
							}
							bMultipleRoles = true;
						}
						else if ( !Q_stricmp( pSubKey->GetName(), "other" ) )
						{
							for ( KeyValues* kvOtherRoles = pSubKey->GetFirstSubKey(); kvOtherRoles; kvOtherRoles = kvOtherRoles->GetNextKey() )
							{
								const char* name = kvOtherRoles->GetName();
								if ( name )
								{

									// Get our current language
									char uilanguage[ 64 ];
									engine->GetUILanguage( uilanguage, sizeof( uilanguage ) );

									char szOtherCredit[ 256 ];
									bool bFound = false;
									// We've found an "other" entry in our current language
									if ( !Q_stricmp( name, uilanguage ) )
									{
										Q_strncpy( szOtherCredit, pSubKey->GetString( name, "" ), sizeof( szOtherCredit ) );
										bFound = true;
									}
									else
									{
										// Check if we have an english entry
										if ( kvOtherRoles->GetString( "English", nullptr ) )
										{
											Q_strcat( szOtherCredit, pSubKey->GetString( "English", "" ), sizeof( szOtherCredit ) );
											bFound = true;
										}
									}

									// We've found an "other" entry, add it
									if ( bFound )
									{
										Q_strcat( roles, bMultipleRoles ? VarArgs( ", %s", szOtherCredit ) : szOtherCredit, sizeof( roles ) );
										bMultipleRoles = true;
									}
								}
							}
						}
					}

					pCreditEntry = new Label( this, "CreditName", bMultipleRoles ? VarArgs( "%s - %s", name, roles ) : name );
					pCreditEntry->SetFont( pScheme->GetFont( "HudFontSmallestBold", true ) );
					pCreditEntry->SetFgColor( pScheme->GetColor( "AdvTextDefault", COLOR_WHITE ) );
					pCreditEntry->SetSize( m_iControlWidth, m_iControlHeight );
					m_pListPanel->AddItem( pCreditEntry, pCreditEntry);
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPFAdvCreditsPanel::DestroyControls()
{
	m_pListPanel->DeleteAllItems();
}

//-----------------------------------------------------------------------------
// Purpose: Shows this dialog as a modal dialog
//-----------------------------------------------------------------------------
void CPFAdvCreditsPanel::ShowModal()
{
	InvalidateLayout( false, true );
	SetVisible( true );
	MoveToFront();
}

CON_COMMAND( OpenCreditsDialog, "Shows the credits dialog" )
{
	GPFAdvCreditsPanel()->ShowModal();
}