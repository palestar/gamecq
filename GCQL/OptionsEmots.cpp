// OptionsEmots.cpp : implementation file
//

#include "stdafx.h"
#include "gcql.h"
#include "Options.h"
#include "OptionsEmots.h"
#include "EditEmotion.h"

#include "Standard/Settings.h"



#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionsEmots property page

IMPLEMENT_DYNCREATE(COptionsEmots, CPropertyPage)

COptionsEmots::COptionsEmots() : CPropertyPage(COptionsEmots::IDD)
{
	//{{AFX_DATA_INIT(COptionsEmots)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

COptionsEmots::~COptionsEmots()
{
}

void COptionsEmots::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COptionsEmots)
	DDX_Control(pDX, IDC_LIST1, m_EmotionList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COptionsEmots, CPropertyPage)
	//{{AFX_MSG_MAP(COptionsEmots)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnEditEmotion)
	ON_BN_CLICKED(IDC_BUTTON1, OnAddEmotion)
	ON_BN_CLICKED(IDC_BUTTON3, OnRemoveEmotion)
	ON_BN_CLICKED(IDC_BUTTON4, OnDefaults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionsEmots message handlers

BOOL COptionsEmots::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	COptions::RestoreDefaultEmotions();

	m_EmotionList.ModifyStyle( LVS_ICON|LVS_SMALLICON|LVS_LIST, 
		LVS_SHOWSELALWAYS|LVS_SHAREIMAGELISTS|LVS_REPORT|LVS_SINGLESEL);
	m_EmotionList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	CRect rect;
	m_EmotionList.GetClientRect( &rect );

	int w = rect.Width() / 3;
	m_EmotionList.InsertColumn( 0, _T("Name"), LVCFMT_LEFT, w );
	m_EmotionList.InsertColumn( 1, _T("Text"), LVCFMT_LEFT, (w * 2) - 15 );

	CGCQLApp::gotoHomeDirectory();
	Settings settings( CGCQLApp::sConfigName );

	int count = settings.get( "EmotionNum", (dword)0 );
	count = (count / 2) * 2;			// make sure the number is even

	for(int i=0;i<count;i+=2)
	{
		int item = m_EmotionList.InsertItem( 0, CString( settings.get( CharString().format("Emotion%d", i), "" ) ) );
		m_EmotionList.SetItemText( item, 1, CString( settings.get( CharString().format("Emotion%d", i + 1), "" ) ) );
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COptionsEmots::OnOK() 
{
	CGCQLApp::gotoHomeDirectory();
	Settings settings( CGCQLApp::sConfigName );
	for(int i=0;i<m_EmotionList.GetItemCount();i++)
	{
		settings.put( CharString().format("Emotion%d",(i * 2)), CharString(m_EmotionList.GetItemText(i,0) ) );
		settings.put( CharString().format("Emotion%d",(i * 2) + 1), CharString( m_EmotionList.GetItemText(i,1) ) );
	}
	settings.put( "EmotionNum", m_EmotionList.GetItemCount() * 2 );
	
	CPropertyPage::OnOK();
}

void COptionsEmots::OnEditEmotion(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	int selected = m_EmotionList.GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	CEditEmotion emotion;
	emotion.m_Name = m_EmotionList.GetItemText( selected, 0 );
	emotion.m_Text = m_EmotionList.GetItemText( selected, 1 );

	if ( emotion.DoModal() == IDOK )
	{
		m_EmotionList.SetItemText( selected, 0, emotion.m_Name );
		m_EmotionList.SetItemText( selected, 1, emotion.m_Text );
	}
}

void COptionsEmots::OnAddEmotion() 
{
	CEditEmotion newEmotion;
	if ( newEmotion.DoModal() == IDOK )
	{
		int item = m_EmotionList.InsertItem( 0, newEmotion.m_Name );
		m_EmotionList.SetItemText( item, 1, newEmotion.m_Text );
	}
}

void COptionsEmots::OnRemoveEmotion() 
{
	int selected = m_EmotionList.GetNextItem( -1, LVNI_SELECTED );
	if ( selected < 0 )
		return;

	m_EmotionList.DeleteItem( selected );
}

void COptionsEmots::OnDefaults() 
{
	if ( MessageBox( _T("This will restore all emotes to their defaults; are you sure?"), _T("Confirm"), MB_YESNO ) != IDYES )
		return;

	CGCQLApp::gotoHomeDirectory();
	Settings settings( CGCQLApp::sConfigName );
	settings.put( "EmotionNum", (dword)0 );

	COptions::RestoreDefaultEmotions();

	// update the list control
	m_EmotionList.DeleteAllItems();

	int count = settings.get( "EmotionNum", (dword)0 );
	count = (count / 2) * 2;			// make sure the number is even

	for(int i=0;i<count;i+=2)
	{
		int item = m_EmotionList.InsertItem( 0, CString( settings.get( CharString().format("Emotion%d", i), "" ) ) );
		m_EmotionList.SetItemText( item, 1, CString( settings.get( CharString().format("Emotion%d", i + 1), "" ) ) );
	}
}
