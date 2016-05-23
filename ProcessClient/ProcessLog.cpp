// ProcessLog.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessLog.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const int MAX_LOG_BUFFER = 16 * 1024;

/////////////////////////////////////////////////////////////////////////////
// CProcessLog dialog


CProcessLog::CProcessLog( ProcessClient * pClient, dword processId, CWnd* pParent /*=NULL*/)
	: CDialog(CProcessLog::IDD, pParent), m_pClient( pClient ), m_ProcessId( processId ), m_LogId( 0 )
{
	//{{AFX_DATA_INIT(CProcessLog)
	m_Text = _T("");
	//}}AFX_DATA_INIT
}

CProcessLog::~CProcessLog()
{
	// close the log
	m_pClient->closeLog( m_LogId );
}

void CProcessLog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessLog)
	DDX_Control(pDX, IDC_BUTTON5, m_CloseButton);
	DDX_Control(pDX, IDC_BUTTON4, m_SaveButton);
	DDX_Control(pDX, IDC_BUTTON3, m_ContinueButton);
	DDX_Control(pDX, IDC_BUTTON2, m_PauseButton);
	DDX_Control(pDX, IDC_EDIT1, m_TextControl);
	DDX_Text(pDX, IDC_EDIT1, m_Text);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessLog, CDialog)
	//{{AFX_MSG_MAP(CProcessLog)
	ON_WM_TIMER()
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON5, OnCloseFile)
	ON_BN_CLICKED(IDC_BUTTON4, OnSaveFile)
	ON_BN_CLICKED(IDC_BUTTON3, OnContinue)
	ON_BN_CLICKED(IDC_BUTTON2, OnPause)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessLog message handlers

BOOL CProcessLog::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Paused = false;
	m_Saving = false;
	m_LogId = m_pClient->openLog( m_ProcessId );

	if ( m_LogId == 0 )
	{
		m_Text = "Failed to open log...";
		UpdateData( false );
	}

	m_CloseButton.EnableWindow( false );
	m_ContinueButton.EnableWindow( false );

	SetTimer( 0x1, 1000, NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessLog::OnTimer(UINT nIDEvent) 
{
	if ( !m_Paused )
	{
		bool updates = false;
		
		// if connection got closed, our log handle will become invalid..
		if (! m_pClient->isLogValid( m_LogId ) )
			m_LogId = m_pClient->openLog( m_ProcessId );

		CharString sLine;
		while( m_pClient->popLog( m_LogId, sLine ) )
		{
			updates = true;

			// normalize the line feeds from unix..
			sLine.replace( "\r", "" );
			sLine.replace( "\n", "\r\n" );

			m_Text += CString( sLine );
			if ( m_Saving )
			{
				CharString sLineA( sLine );
				m_SaveFile.write( sLineA, sLineA.length() );
			}
		}

		// check the length of the log text, keep it under a certain size
		if ( m_Text.GetLength() > MAX_LOG_BUFFER )
			m_Text = m_Text.Right( MAX_LOG_BUFFER );

		if ( updates )
		{
			// update the dialog text
			UpdateData( false );
			// show the last line
			m_TextControl.LineScroll( m_TextControl.GetLineCount() );
		}
	}

	//CDialog::OnTimer(nIDEvent);
}

void CProcessLog::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
	delete this;
}

void CProcessLog::OnClose() 
{
	CDialog::OnClose();
	DestroyWindow();
}

void CProcessLog::OnCloseFile() 
{
	if ( m_Saving )
	{
		m_Saving = false;
		m_SaveFile.close();

		m_CloseButton.EnableWindow( false );
		m_SaveButton.EnableWindow( true );
	}
}

void CProcessLog::OnSaveFile() 
{
	CFileDialog save( false, _T("log") );
	if ( save.DoModal() == IDOK )
	{
		if ( m_SaveFile.open( CharString( save.GetPathName() ), FileDisk::WRITE ) )
		{
			CharString sTextA( m_Text );
			m_SaveFile.write( sTextA, sTextA.length() );

			m_CloseButton.EnableWindow( true );
			m_SaveButton.EnableWindow( false );

			m_Saving = true;
		}	
	}
}

void CProcessLog::OnContinue() 
{
	if ( m_Paused )
	{
		m_Paused = false;
		m_ContinueButton.EnableWindow( false );
		m_PauseButton.EnableWindow( true );
	}
}

void CProcessLog::OnPause() 
{
	if (! m_Paused )
	{
		m_Paused = true;
		m_ContinueButton.EnableWindow( true );
		m_PauseButton.EnableWindow( false );
	}
}
