// OptionsColors.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "OptionsColors.h"

#include "GCQStyle.h"

#include "Standard/Settings.h"



#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsColors property page

IMPLEMENT_DYNCREATE(COptionsColors, CPropertyPage)

COptionsColors::COptionsColors() : CPropertyPage(COptionsColors::IDD)
{
	//{{AFX_DATA_INIT(COptionsColors)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COptionsColors::~COptionsColors()
{
}

void COptionsColors::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsColors)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsColors, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsColors)
	ON_BN_CLICKED(IDC_BUTTON13, OnDefaults)
	ON_BN_CLICKED(IDC_BUTTON12, OnApplyColors)
	ON_BN_CLICKED(IDC_BUTTON14, OnLoad)
	ON_BN_CLICKED(IDC_BUTTON15, OnSave)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsColors message handlers

BOOL COptionsColors::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_Color1.SubclassDlgItem(IDC_BUTTON1,this);
	m_Color2.SubclassDlgItem(IDC_BUTTON2,this);
	m_Color3.SubclassDlgItem(IDC_BUTTON3,this);
	m_Color4.SubclassDlgItem(IDC_BUTTON4,this);
	m_Color5.SubclassDlgItem(IDC_BUTTON5,this);
	m_Color6.SubclassDlgItem(IDC_BUTTON6,this);
	m_Color7.SubclassDlgItem(IDC_BUTTON7,this);
	m_Color8.SubclassDlgItem(IDC_BUTTON8,this);
	m_Color9.SubclassDlgItem(IDC_BUTTON9,this);
	m_Color10.SubclassDlgItem(IDC_BUTTON10,this);
	m_Color11.SubclassDlgItem(IDC_BUTTON11,this);
	
	m_Color1.currentcolor = CGCQLApp::sm_BackgroundColor1;
	m_Color2.currentcolor = CGCQLApp::sm_BackgroundColor2;
	m_Color3.currentcolor = CGCQLApp::sm_FrameColor;
	m_Color4.currentcolor = CGCQLApp::sm_ButtonFrame1;
	m_Color5.currentcolor = CGCQLApp::sm_ButtonFrame2;
	m_Color6.currentcolor = CGCQLApp::sm_ButtonHighlight;			// button highlight
	m_Color7.currentcolor = CGCQLApp::sm_GripperColor1;
	m_Color8.currentcolor = CGCQLApp::sm_GripperColor2;
	m_Color9.currentcolor = CGCQLApp::sm_TextColor;
	m_Color10.currentcolor = CGCQLApp::sm_GrayedTextColor;
	m_Color11.currentcolor = CGCQLApp::sm_StatusColor;

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsColors::OnOK() 
{
	OnApplyColors();
	CPropertyPage::OnOK();
}

#undef RGB
#define RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

void COptionsColors::OnDefaults() 
{
	m_Color1.currentcolor = RGB(32, 32, 32);
	m_Color2.currentcolor = RGB(48, 48, 48);
	m_Color3.currentcolor = RGB(64,64,64);
	m_Color4.currentcolor = RGB(16,16,16);
	m_Color5.currentcolor = RGB(32,32,32);
	m_Color6.currentcolor = RGB(64, 64, 64);
	m_Color7.currentcolor = RGB(32, 32, 32);
	m_Color8.currentcolor = RGB(255, 255, 255);
	m_Color9.currentcolor = RGB( 255, 255, 255 );
	m_Color10.currentcolor = RGB( 150, 64, 64 );
	m_Color11.currentcolor = RGB( 128, 128, 128 );

	RedrawWindow();
	OnApplyColors();
}

void COptionsColors::OnApplyColors() 
{
	CGCQLApp::sm_BackgroundColor1 = m_Color1.currentcolor;
	CGCQLApp::sm_BackgroundColor2 = m_Color2.currentcolor;
	CGCQLApp::sm_FrameColor = m_Color3.currentcolor;
	CGCQLApp::sm_ButtonFrame1 = m_Color4.currentcolor;
	CGCQLApp::sm_ButtonFrame2 = m_Color5.currentcolor;
	CGCQLApp::sm_ButtonHighlight = m_Color6.currentcolor;
	CGCQLApp::sm_GripperColor1 = m_Color7.currentcolor;
	CGCQLApp::sm_GripperColor2 = m_Color8.currentcolor;
	CGCQLApp::sm_TextColor = m_Color9.currentcolor;
	CGCQLApp::sm_GrayedTextColor = m_Color10.currentcolor;
	CGCQLApp::sm_StatusColor = m_Color11.currentcolor;

	// update the dialog colors
	//AfxGetApp()->SetDialogBkColor(CGCQLApp::sm_BackgroundColor1, CGCQLApp::sm_TextColor);

	// create a new style manager
	if (CBCGVisualManager::GetInstance () != NULL)
		 delete CBCGVisualManager::GetInstance ();
	new CGCQStyle();

	CGCQLApp::sm_pChatFrame->RedrawWindow(NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN);
	if ( CGCQLApp::sm_pWebFrame != NULL )
	{
		CGCQLApp::sm_pWebFrame->m_wndStatusBar.GetStatusBarCtrl().SetBkColor( CGCQLApp::sm_StatusColor );
		CGCQLApp::sm_pWebFrame->RedrawWindow( NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN );
	}
	if ( CGCQLApp::sm_pServerFrame != NULL )
	{
		CGCQLApp::sm_pServerFrame->m_wndStatusBar.GetStatusBarCtrl().SetBkColor( CGCQLApp::sm_StatusColor );
		CGCQLApp::sm_pServerFrame->RedrawWindow( NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN );
	}
	if ( CGCQLApp::sm_pCacheFrame != NULL )
	{
		//CGCQLApp::sm_pCacheFrame->m_wndStatusBar.GetStatusBarCtrl().SetBkColor( CGCQLApp::sm_StatusColor );
		CGCQLApp::sm_pCacheFrame->RedrawWindow( NULL,NULL,RDW_INVALIDATE|RDW_UPDATENOW|RDW_ERASE|RDW_ALLCHILDREN );
	}
}

void COptionsColors::OnLoad() 
{
	// get the working directory
	TCHAR cwd[ MAX_PATH ];
	GetCurrentDirectory( MAX_PATH, cwd );
	// append ending slash
	_tcscat( cwd, _T("\\*.skn") );

	CFileDialog load( true, _T("skn"), cwd, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("Skins (*.skn)|*.skn|All Files (*.*)|*.*||") );
	if ( load.DoModal() == IDOK )
	{
		Settings skin( "SKIN", CharString(load.GetPathName()) );
		m_Color1.currentcolor = skin.get( "Color1", RGB(32, 32, 32) );
		m_Color2.currentcolor = skin.get( "Color2", RGB(48, 48, 48) );
		m_Color3.currentcolor = skin.get( "Color3", RGB(64,64,64) );
		m_Color4.currentcolor = skin.get( "Color4", RGB(16,16,16) );
		m_Color5.currentcolor = skin.get( "Color5", RGB(32,32,32) );
		m_Color6.currentcolor = skin.get( "Color6", RGB(64, 64, 64) );
		m_Color7.currentcolor = skin.get( "Color7", RGB(32, 32, 32) );
		m_Color8.currentcolor = skin.get( "Color8", RGB(255, 255, 255) );
		m_Color9.currentcolor = skin.get( "Color9", RGB( 255, 255, 255 ) );
		m_Color10.currentcolor = skin.get( "Color10", RGB( 150, 64, 64 ) );
		m_Color11.currentcolor = skin.get( "Color11", RGB( 128, 128, 128 ) );

		RedrawWindow();
		OnApplyColors();
	}
}

void COptionsColors::OnSave() 
{
	// get the working directory
	TCHAR cwd[ MAX_PATH ];
	GetCurrentDirectory( MAX_PATH, cwd );
	// append ending slash
	_tcscat( cwd, _T("\\New Skin.skn") );

	CFileDialog save( false, _T("skn"), cwd, OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, _T("Skins (*.skn)|*.skn|All Files (*.*)|*.*||") );
	if ( save.DoModal() == IDOK )
	{
		Settings skin( "SKIN", CharString(save.GetPathName()) );
		skin.put( "Color1", m_Color1.currentcolor );
		skin.put( "Color2", m_Color2.currentcolor );
		skin.put( "Color3", m_Color3.currentcolor );
		skin.put( "Color4", m_Color4.currentcolor );
		skin.put( "Color5", m_Color5.currentcolor );
		skin.put( "Color6", m_Color6.currentcolor );
		skin.put( "Color7", m_Color7.currentcolor );
		skin.put( "Color8", m_Color8.currentcolor );
		skin.put( "Color9", m_Color9.currentcolor );
		skin.put( "Color10", m_Color10.currentcolor );
		skin.put( "Color11", m_Color11.currentcolor );
	}
}
