// ProcessServer.cpp : implementation file
//

#include "stdafx.h"
#include "ProcessServer.h"
#include "ProcessLog.h"
#include "ProcessSearchLogs.h"

#include "Process.h"
#include "EditDialog.h"

#include "File/FileDisk.h"

#ifdef _DEBUG
//#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProcessServer dialog


CProcessServer::CProcessServer(CWnd* pParent /*=NULL*/)
	: CDialog(CProcessServer::IDD, pParent), m_bUpdating( false )
{
	//{{AFX_DATA_INIT(CProcessServer)
	m_sCPU = _T("");
	m_sMemory = _T("");
	//}}AFX_DATA_INIT
}


void CProcessServer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessServer)
	DDX_Control(pDX, IDC_PROGRESS2, m_cMemory);
	DDX_Control(pDX, IDC_PROGRESS1, m_cCPU);
	DDX_Control(pDX, IDC_BUTTON17, m_TerminateProcess);
	DDX_Control(pDX, IDC_BUTTON13, m_StopProcess);
	DDX_Control(pDX, IDC_BUTTON12, m_RestartProcess);
	DDX_Control(pDX, IDC_BUTTON11, m_StartProcess);
	DDX_Control(pDX, IDC_BUTTON15, m_ProcessLog);
	DDX_Control(pDX, IDC_BUTTON14, m_ConfigureProcess);
	DDX_Control(pDX, IDC_BUTTON16, m_DeleteProcess);
	DDX_Control(pDX, IDC_BUTTON2, m_EditProcess);
	DDX_Control(pDX, IDC_LIST1, m_ProcessList);
	DDX_Text(pDX, IDC_CPU, m_sCPU);
	DDX_Text(pDX, IDC_MEMORY, m_sMemory);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessServer, CDialog)
	//{{AFX_MSG_MAP(CProcessServer)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_BUTTON3, OnRestart)
	ON_BN_CLICKED(IDC_BUTTON8, OnConfigure)
	ON_BN_CLICKED(IDC_BUTTON9, OnLog)
	ON_BN_CLICKED(IDC_BUTTON10, OnAddProcess)
	ON_BN_CLICKED(IDC_BUTTON2, OnEditProcess)
	ON_BN_CLICKED(IDC_BUTTON16, OnDeleteProcess)
	ON_BN_CLICKED(IDC_BUTTON14, OnConfigureProcess)
	ON_BN_CLICKED(IDC_BUTTON15, OnProcessLog)
	ON_BN_CLICKED(IDC_BUTTON11, OnStartProcess)
	ON_BN_CLICKED(IDC_BUTTON12, OnRestartProcess)
	ON_BN_CLICKED(IDC_BUTTON13, OnStopProcess)
	ON_BN_CLICKED(IDC_BUTTON4, OnStartAll)
	ON_BN_CLICKED(IDC_BUTTON6, OnRestartAll)
	ON_BN_CLICKED(IDC_BUTTON5, OnStopAll)
	ON_BN_CLICKED(IDC_BUTTON1, OnUpdate)
	ON_BN_CLICKED(IDC_BUTTON17, OnTerminateProcess)
	ON_NOTIFY(NM_CLICK, IDC_LIST1, OnSelectProcess)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, OnSelectProcess2)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST1, OnProcessLog2)
	ON_BN_CLICKED(IDC_BUTTON18, OnSearchLogs)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessServer message handlers

ProcessClient::Process CProcessServer::getProcess()
{
	int selected = m_ProcessList.GetNextItem( -1, LVNI_SELECTED );
	if ( selected >= 0 )
	{
		int i = (int)m_ProcessList.GetItemData( selected );
		if ( i >= 0 && i < m_Processes.size() )
			return m_Processes[ i ];
	}

	return ProcessClient::Process();
}

dword CProcessServer::getProcessId()
{
	return getProcess().processId;
}

//----------------------------------------------------------------------------

BOOL CProcessServer::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_hIcon = AfxGetApp()->LoadIcon( IDR_MAINFRAME );
	SetIcon(m_hIcon, TRUE);   // Set big icon
	SetIcon(m_hIcon, FALSE);  // Set small icon

	CRect rect;
	// Grab a reference to the output list
	m_ProcessList.GetWindowRect( &rect );
	m_ProcessList.SetExtendedStyle( LVS_EX_FULLROWSELECT );

	int cwidth = rect.Width() / 37;
	m_ProcessList.InsertColumn( 0, _T("Name"),		LVCFMT_LEFT		, cwidth * 10, 0 ); 		
	m_ProcessList.InsertColumn( 1, _T("Executable"),LVCFMT_LEFT		, cwidth * 7, 1 ); 		
	m_ProcessList.InsertColumn( 2, _T("Arguments"),	LVCFMT_LEFT		, cwidth * 8, 2 ); 		
	m_ProcessList.InsertColumn( 3, _T("Running"),	LVCFMT_LEFT		, cwidth * 4, 3 ); 		
	m_ProcessList.InsertColumn( 4, _T("Disabled"),	LVCFMT_LEFT		, cwidth * 4, 4 ); 	
	m_ProcessList.InsertColumn( 5, _T("RunOnce"),	LVCFMT_LEFT		, cwidth * 4, 5 ); 	

	OnUpdate();

	SetTimer( 0x1, 1000 * 5, NULL );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessServer::OnClose() 
{
	CDialog::OnClose();
	DestroyWindow();
}

void CProcessServer::PostNcDestroy() 
{
	CDialog::PostNcDestroy();
}

void CProcessServer::OnRestart() 
{
	if ( MessageBox( _T("Are you sure you wish to restart?"), _T("Confirm"), MB_YESNO ) == IDYES )
	{
		m_Client.exit();
		//OnClose();
	}
}

void CProcessServer::OnConfigure() 
{
	CharString config;
	if ( m_Client.getConfig( 0, config ) )
	{
		CEditDialog dialog;
		dialog.m_Text = CString( config );

		if ( dialog.DoModal() == IDOK )
		{
			config = dialog.m_Text;
			if (! m_Client.putConfig( 0, config ) )
				MessageBox( _T("Failed to put configuration!") );
		}
	}
	else
		MessageBox( _T("Failed to get configuration!") );
}

void CProcessServer::OnLog() 
{
	CProcessLog * pDialog = new CProcessLog( &m_Client, 0 );
	pDialog->Create( CProcessLog::IDD, this );

	CString title;
	GetWindowText( title );
	title += _T(" Log");

	pDialog->SetWindowText( title );
}

void CProcessServer::OnAddProcess() 
{
	CProcess dialog;
	if ( dialog.DoModal() == IDOK )
	{
		ProcessClient::Process proc;
		proc.processId = 0;
		proc.name = dialog.m_Name;
		proc.executable = dialog.m_Exec;
		proc.arguments = dialog.m_Arguments;
		proc.config = dialog.m_Config;
		proc.log = dialog.m_Log;

		proc.flags = 0;
		if ( dialog.m_bRunOnce )
			proc.flags |= ProcessClient::PF_RUNONCE;
		if ( dialog.m_Disabled )
			proc.flags |= ProcessClient::PF_DISABLED;

		if (! m_Client.addProcess( proc ) )
			MessageBox( _T("Failed to add process!") );
		OnUpdate();
	}
}

void CProcessServer::OnEditProcess() 
{
	ProcessClient::Process proc = getProcess();
	
	CProcess dialog;
	dialog.m_Name = (CString)proc.name;
	dialog.m_Exec = (CString)proc.executable;
	dialog.m_Arguments = (CString)proc.arguments;
	dialog.m_Config = (CString)proc.config;
	dialog.m_Log = (CString)proc.log;
	dialog.m_Disabled = (proc.flags & ProcessClient::PF_DISABLED) != 0;
	dialog.m_bRunOnce = (proc.flags & ProcessClient::PF_RUNONCE) != 0;

	if ( dialog.DoModal() == IDOK )
	{
		proc.name = dialog.m_Name;
		proc.executable = dialog.m_Exec;
		proc.arguments = dialog.m_Arguments;
		proc.config = dialog.m_Config;
		proc.log = dialog.m_Log;

		proc.flags = 0;
		if ( dialog.m_bRunOnce )
			proc.flags |= ProcessClient::PF_RUNONCE;
		if ( dialog.m_Disabled )
			proc.flags |= ProcessClient::PF_DISABLED;

		if (! m_Client.setProcess( proc ) )
			MessageBox( _T("Failed to edit process!") );

		OnUpdate();
	}
}

void CProcessServer::OnDeleteProcess() 
{
	if ( MessageBox( _T("Delete Process?"), _T("Confirm"), MB_YESNO ) == IDYES )
		if (! m_Client.deleteProcess( getProcessId() ) )
			MessageBox( _T("Failed to delete process!") );

	OnUpdate();
}

void CProcessServer::OnConfigureProcess() 
{
	CharString config;
	if ( m_Client.getConfig( getProcessId(), config ) )
	{
		CEditDialog dialog;
		dialog.m_Text = (CString)config;

		if ( dialog.DoModal() == IDOK )
		{
			config = dialog.m_Text;
			if (! m_Client.putConfig( getProcessId(), config ) )
				MessageBox( _T("Failed to put configuration!") );
		}
	}
	else
		MessageBox( _T("Failed to get configuration!") );
}

void CProcessServer::OnProcessLog() 
{
	CProcessLog * pDialog = new CProcessLog( &m_Client,  getProcessId() );
	pDialog->Create( CProcessLog::IDD, this );
	pDialog->SetWindowText( (CString)getProcess().log );
}

void CProcessServer::OnProcessLog2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnProcessLog();
	*pResult = 0;
}

void CProcessServer::OnStartProcess() 
{
	if (! m_Client.startProcess( getProcessId() ) )
		MessageBox( _T("Failed to start process!") );
	OnUpdate();
}

void CProcessServer::OnRestartProcess() 
{
	if (! m_Client.restartProcess( getProcessId() ) )
		MessageBox( _T("Failed to restart process!") );
	OnUpdate();
}

void CProcessServer::OnStopProcess() 
{
	if (! m_Client.stopProcess( getProcessId() ) )
		MessageBox( _T("Failed to stop process!") );
	OnUpdate();
}

void CProcessServer::OnStartAll() 
{
	m_Client.startAll();
	OnUpdate();
}

void CProcessServer::OnRestartAll() 
{
	if ( MessageBox( _T("Are you sure you wish to restart?"), _T("Confirm"), MB_YESNO ) == IDYES )
		m_Client.restartAll();
	OnUpdate();
}

void CProcessServer::OnStopAll() 
{
	if ( MessageBox( _T("Are you sure you wish to stop?"), _T("Confirm"), MB_YESNO ) == IDYES )
		m_Client.stopAll();
	OnUpdate();
}

static int CompareProcesses( ProcessClient::Process p1, ProcessClient::Process p2 )
{
	return strcmp( p1.name, p2.name );
}

void CProcessServer::OnUpdate() 
{
	if ( m_bUpdating )
		return;
	m_bUpdating = true;

	if ( m_Client.getProcessList( m_Processes ) )
	{
		// sort by name...
		m_Processes.qsort( CompareProcesses );

		for(int i=0;i<m_Processes.size();i++)
		{
			ProcessClient::Process & process = m_Processes[ i ];

			if ( i >= m_ProcessList.GetItemCount() )
				m_ProcessList.InsertItem( i, (CString)process.name );
			else
				m_ProcessList.SetItemText( i, 0, (CString)process.name );

			m_ProcessList.SetItemData( i, i );
			m_ProcessList.SetItemText( i, 1, (CString)process.executable );
			m_ProcessList.SetItemText( i, 2, (CString)process.arguments );
			m_ProcessList.SetItemText( i, 3, process.flags & ProcessClient::PF_RUNNING ? _T("Yes") : _T("No") );
			m_ProcessList.SetItemText( i, 4, process.flags & ProcessClient::PF_DISABLED ? _T("Yes") : _T("No") );
			m_ProcessList.SetItemText( i, 5, process.flags & ProcessClient::PF_RUNONCE ? _T("Yes") : _T("No") );
		}
	}
	else
	{
		m_Processes.release();
	}

	// remove extra list items..
	while( m_ProcessList.GetItemCount() > m_Processes.size() )
		m_ProcessList.DeleteItem( m_Processes.size() );

	ProcessClient::Status status;
	if ( m_Client.getStatus( status ) )
	{
		m_cCPU.SetPos( status.cpuUsage );
		m_sCPU.Format( _T("%d%%"), status.cpuUsage );
		m_cMemory.SetPos( status.memoryUsage );
		m_sMemory.Format( _T("%d%%"), status.memoryUsage );
		UpdateData( false );

		SetWindowText( CharString().format( "%s - CPU: %s", m_sName, m_sCPU ) );
	}

	// try to keep the connection up and running..
	m_Client.update();
	if (! m_Client.connected() )
		m_Client.reconnect();
	if ( m_Client.connected() && !m_Client.loggedIn() )
		m_Client.login( m_Client.sessionId() );

	m_bUpdating = false;
}

void CProcessServer::OnSelectProcess(NMHDR* pNMHDR, LRESULT* pResult) 
{
	*pResult = 0;

	int selected = m_ProcessList.GetNextItem( -1, LVNI_SELECTED );
	m_TerminateProcess.EnableWindow( selected >= 0 );
	m_StopProcess.EnableWindow( selected >= 0 );
	m_RestartProcess.EnableWindow( selected >= 0 );
	m_StartProcess.EnableWindow( selected >= 0 );
	m_ProcessLog.EnableWindow( selected >= 0 );
	m_ConfigureProcess.EnableWindow( selected >= 0 );
	m_DeleteProcess.EnableWindow( selected >= 0 );
	m_EditProcess.EnableWindow( selected >= 0 );
}

void CProcessServer::OnSelectProcess2(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnSelectProcess( pNMHDR, pResult );
}

void CProcessServer::OnTerminateProcess() 
{
	if ( MessageBox( _T("Are you sure you wish to terminate this process?"), _T("Confirm"), MB_YESNO ) == IDYES )
		if (! m_Client.terminateProcess( getProcessId() ) )
			MessageBox( _T("Failed to terminate process!") );
	OnUpdate();
}

void CProcessServer::OnSearchLogs() 
{
	// changes this to modal from modless, since it would cause a crash if the CProcessServer dialog was closed
	// while the search dialog was still open as a modless dialog -rl
	CProcessSearchLogs( &m_Client, this ).DoModal();
}

void CProcessServer::OnTimer(UINT nIDEvent) 
{
	OnUpdate();
	//CDialog::OnTimer(nIDEvent);
}
