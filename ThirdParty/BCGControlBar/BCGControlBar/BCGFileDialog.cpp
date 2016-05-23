//*******************************************************************************
// COPYRIGHT NOTES
// ---------------
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//*******************************************************************************

// BCGFileDialog.cpp : implementation file
//

#include "stdafx.h"
#include "bcgbarres.h"
#include "bcglocalres.h"
#include "BCGFileDialog.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

class CNewItemInfo : public CObject
{
	friend class CBCGFileDialog;

	CNewItemInfo (LPCTSTR lpszName, int iIconIndex)
	{
		m_strName = lpszName;
		m_iIconIndex = iIconIndex;
	}

	CString m_strName;
	int		m_iIconIndex;
};

/////////////////////////////////////////////////////////////////////////////
// CBCGFileDialog

const int iTabCtrlId		= 200;
const int iNewListCtrlId	= 201;
const int iRecentListCtrlId	= 202;

WNDPROC CBCGFileDialog::m_wndProc;

IMPLEMENT_DYNAMIC(CBCGFileDialog, CFileDialog)

CBCGFileDialog::CBCGFileDialog (LPCTSTR lpszCaption, BOOL bNewPage, LPCTSTR lpszDefExt, LPCTSTR lpszFileName,
		DWORD dwFlags, LPCTSTR lpszFilter, CWnd* pParentWnd) :
		CFileDialog (TRUE /*bOpenFileDialog*/, lpszDefExt, lpszFileName, dwFlags, lpszFilter, pParentWnd),
		m_pImagesNew (NULL),
		m_bNewPage (bNewPage),
		m_strCaption (lpszCaption),
		m_hIconBig (NULL),
		m_hIconSmall (NULL),
		m_iLogoAreaHeight (0),
        m_iExtraWidth (0),
        m_iExtraHeight (0)
{
	m_iNewItemIndex = -1;
	m_pBmpLogo = NULL;

	CDocManager* pDocManager = AfxGetApp ()->m_pDocManager;
	if (pDocManager != NULL && lpszFilter == NULL)
	{
		CString strDefault;
		BOOL bFirst = TRUE;
		m_strFilter.Empty();
		for (POSITION pos = pDocManager->GetFirstDocTemplatePosition (); pos != NULL;)
		{
			CDocTemplate* pTemplate = pDocManager->GetNextDocTemplate (pos);
			ASSERT_VALID (pTemplate);
			
			CString strFilterExt, strFilterName;
			
			if (pTemplate->GetDocString (strFilterExt, CDocTemplate::filterExt) &&
				!strFilterExt.IsEmpty() &&
				pTemplate->GetDocString(strFilterName, CDocTemplate::filterName) &&
				!strFilterName.IsEmpty())
			{
#if _MSC_VER >= 1300
				int iStart = 0;
				m_strFilter += strFilterName;
				m_strFilter += (TCHAR)'\0';
				do
				{
					CString strExtension = strFilterExt.Tokenize( _T( ";" ), iStart );
					
					if (iStart != -1)
					{
						// a file based document template - add to filter list
						
						// If you hit the following ASSERT, your document template 
						// string is formatted incorrectly. The section of your 
						// document template string that specifies the allowable file
						// extensions should be formatted as follows:
						// .<ext1>;.<ext2>;.<ext3>
						// Extensions may contain wildcards (e.g. '?', '*'), but must
						// begin with a '.' and be separated from one another by a ';'.
						// Example:
						// .jpg;.jpeg
						ASSERT(strExtension[0] == '.');
						if ( strDefault.IsEmpty() )
						{
							// set the default extension
							strDefault = strExtension.Mid( 1 ); // skip the '.'
							m_ofn.lpstrDefExt = const_cast< LPTSTR >((LPCTSTR)strDefault);
							m_ofn.nFilterIndex = m_ofn.nMaxCustFilter + 1; // 1 based number
						}
						
						m_strFilter += (TCHAR)'*';
						m_strFilter += strExtension;
						m_strFilter += (TCHAR)';'; // Always append a ';'. The last ';' will get replaced with a '\0' later.
					}
				} while (iStart != -1);
				m_strFilter.SetAt( m_strFilter.GetLength()-1, '\0' );; // Replace the last ';' with a '\0'
				m_ofn.nMaxCustFilter++;
				
#else
				// a file based document template - add to filter list
				ASSERT(strFilterExt[0] == '.');
				if (bFirst)
				{
					strDefault = ((LPCTSTR)strFilterExt) + 1; // skip the '.'
					m_ofn.lpstrDefExt = strDefault;
					m_ofn.nFilterIndex = m_ofn.nMaxCustFilter + 1; // 1 based number
				}
				// add to filter
				m_strFilter += strFilterName;
				ASSERT(!m_strFilter.IsEmpty()); // must have a file type name
				m_strFilter += (TCHAR)'\0'; // next string please
				m_strFilter += (TCHAR)'*';
				m_strFilter += strFilterExt;
				m_strFilter += (TCHAR)'\0'; // next string please
				m_ofn.nMaxCustFilter++;
#endif
			}
			bFirst = FALSE;
		}
		
		CString allFilter;
		VERIFY(allFilter.LoadString(AFX_IDS_ALLFILTER));
		m_strFilter += allFilter;
		m_strFilter += (TCHAR)'\0'; // next string please
		m_strFilter += _T("*.*");
		m_strFilter += (TCHAR)'\0'; // last string
		m_ofn.nMaxCustFilter++;
		
		m_ofn.lpstrFilter = m_strFilter;
	}
}
//***************************************************************************************
CBCGFileDialog::~CBCGFileDialog ()
{
	while (!m_lstNewItems.IsEmpty ())
	{
		delete m_lstNewItems.RemoveHead ();
	}
}
//***************************************************************************************
static CBCGFileDialog* GetBCGFileDlg (HWND hwdParent)
{
	CFileDialog* pDlg = (CFileDialog*)CWnd::FromHandle (hwdParent);
	ASSERT (pDlg != NULL);

	CBCGFileDialog* pFD = (CBCGFileDialog*) pDlg->GetDlgItem(0);
	ASSERT (pFD != NULL);

	return pFD;
}
//***************************************************************************************
LRESULT CALLBACK CBCGFileDialog::WindowProcNew(HWND hwnd,UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY:
		{
			CBCGFileDialog* pFD = GetBCGFileDlg (hwnd);

			LPNMHDR pHdr = (LPNMHDR) lParam;
			ASSERT (pHdr != NULL);

			if (wParam == iTabCtrlId && pHdr->code == TCN_SELCHANGE)
			{
				pFD->OnTabSelchange();
			}
			else if ((wParam == iNewListCtrlId || wParam == iRecentListCtrlId) 
				&& pHdr->code == NM_DBLCLK)
			{
				pFD->OnItemDblClick();
			}
		}
		break;

	case WM_COMMAND:
		{
			if ((int) LOWORD(wParam) == IDOK)
			{
				CBCGFileDialog* pFD = GetBCGFileDlg (hwnd);
				if (pFD->GetPage () != CBCGFileDialog::BCGFileOpen)
				{
					pFD->OnItemDblClick();
					return 0;
				}
			}
		}
		break;

	case WM_PAINT:
		{
			CBCGFileDialog* pFD = GetBCGFileDlg (hwnd);
			pFD->CollectControls ();
	
			if (pFD->m_pBmpLogo != NULL)
			{
				ASSERT_VALID (pFD->m_pBmpLogo);

				CFileDialog* pDlg = (CFileDialog*)CWnd::FromHandle (hwnd);
				ASSERT (pDlg != NULL);

				CPaintDC dc (pDlg); // device context for painting
				dc.DrawState (pFD->m_rectLogo.TopLeft (),
					pFD->m_rectLogo.Size (), pFD->m_pBmpLogo,
					DSS_NORMAL);

				CRect rectFrame = pFD->m_rectLogo;
				rectFrame.InflateRect (1, 1);

				dc.Draw3dRect (rectFrame, globalData.clrBtnShadow, globalData.clrBtnLight);
			}
		}
		break;

	case WM_SIZE:
		{
			LRESULT lRes = CallWindowProc(CBCGFileDialog::m_wndProc, hwnd, message, wParam, lParam);

			CBCGFileDialog* pFD = GetBCGFileDlg (hwnd);
			ASSERT_VALID (pFD);

			CWnd* pFDParent = pFD->GetParent();
			ASSERT (pFDParent != NULL);

			CRect rectTabs;
			pFDParent->GetClientRect (rectTabs);
			rectTabs.DeflateRect (4, 4);
			rectTabs.top += pFD->m_iLogoAreaHeight;

			pFD->m_wndTab.SetWindowPos (NULL, -1, -1,
				rectTabs.Width (), rectTabs.Height (),
				SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);

			pFD->m_lstFDControls.RemoveAll ();
			pFD->CollectControls ();

			return lRes;
		}
	}

	return CallWindowProc(CBCGFileDialog::m_wndProc, hwnd, message, wParam, lParam);
}

BEGIN_MESSAGE_MAP(CBCGFileDialog, CFileDialog)
	//{{AFX_MSG_MAP(CBCGFileDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//-----------------------------------------------------
// My "classic " trick - how I can access to protected
// member m_pRecentFileList?
//-----------------------------------------------------
class CBCGApp : public CWinApp
{
	friend class CBCGFileDialog;
};

void CBCGFileDialog::OnInitDone()
{
	const int iBorderWidth = 20;
	const int iBorderHeight = 40;

	CWnd* pFD = GetParent();
	ASSERT (pFD != NULL);

	CRect rectClient;
	pFD->GetClientRect (rectClient);

	int nNewDlgWidth = rectClient.Width () + iBorderWidth * 2 + m_iExtraWidth;

	if (m_pBmpLogo != NULL)
	{
		BITMAP bmp;
		m_pBmpLogo->GetBitmap (&bmp);

		m_rectLogo = CRect (CPoint ((nNewDlgWidth - bmp.bmWidth) / 2, 8),
							CSize (bmp.bmWidth, bmp.bmHeight));
		m_iLogoAreaHeight = bmp.bmHeight + 20;
	}

	//---------------------------
	// Adjust parent window size:
	//---------------------------
	pFD->ModifyStyle (WS_THICKFRAME, WS_DLGFRAME | WS_BORDER);
	pFD->ModifyStyleEx (WS_EX_WINDOWEDGE, 0);
	pFD->SetWindowPos (NULL, -1, -1, nNewDlgWidth,
					rectClient.Height () + iBorderHeight * 2 + m_iLogoAreaHeight + m_iExtraHeight,
					SWP_NOMOVE | SWP_NOZORDER  | SWP_NOACTIVATE);

	//-------------------
	// Move all controls:
	//-------------------
	CWnd* pWndChild = pFD->GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		CRect rectCtl;
		pWndChild->GetClientRect (rectCtl);
		pWndChild->MapWindowPoints (pFD, rectCtl);
		pWndChild->SetWindowPos (NULL, 
			rectCtl.left + iBorderWidth, 
			rectCtl.top + iBorderHeight + m_iLogoAreaHeight,
			rectCtl.Width (), rectCtl.Height (), 
			SWP_NOZORDER | SWP_NOACTIVATE);

		pWndChild = pWndChild->GetNextWindow ();
	}

	//------------------------------------------
	// Create new and recent file list controls:
	//------------------------------------------
	CRect rectList (0, 0, 0, 0);
	m_wndNewList.Create (WS_BORDER | WS_TABSTOP | WS_CHILD | LVS_ICON | LVS_SINGLESEL, 
							rectList, pFD, iNewListCtrlId);
	m_wndNewList.ModifyStyleEx (0, WS_EX_CLIENTEDGE);

	if (m_pImagesNew != NULL)
	{
		m_wndNewList.SetImageList (m_pImagesNew, LVSIL_NORMAL);
	}

	int i = 0;
	for (POSITION pos = m_lstNewItems.GetHeadPosition (); pos != NULL; i ++)
	{
		CNewItemInfo* pInfo = (CNewItemInfo*) m_lstNewItems.GetNext (pos);
		ASSERT_VALID (pInfo);

		m_wndNewList.InsertItem (i, pInfo->m_strName, pInfo->m_iIconIndex);
	}

	m_wndRecentList.Create (WS_TABSTOP | WS_CHILD | WS_BORDER | LVS_SINGLESEL | LVS_REPORT, 
							rectList, pFD, iRecentListCtrlId);
	m_wndRecentList.ModifyStyleEx (0, WS_EX_CLIENTEDGE);

	m_ImagesRecent.Create (	::GetSystemMetrics (SM_CXSMICON),
							::GetSystemMetrics (SM_CYSMICON),
							ILC_COLOR8, 0, 10);
	m_wndRecentList.SetImageList (&m_ImagesRecent, LVSIL_SMALL);

	{
		CBCGLocalResource locaRes;

		CString strFile;
		strFile.LoadString (IDS_BCGBARRES_FILE);
		m_wndRecentList.InsertColumn (0, strFile, LVCFMT_LEFT, 100);

		CString strFolder;
		strFolder.LoadString (IDS_BCGBARRES_FOLDER);
		m_wndRecentList.InsertColumn (1, strFolder);
	}

	CRecentFileList* pMRUFiles = 
		((CBCGApp*) AfxGetApp ())->m_pRecentFileList;

	if (pMRUFiles != NULL)
	{
		TCHAR szCurDir [_MAX_PATH];
		::GetCurrentDirectory (_MAX_PATH, szCurDir);

		int nCurDir = lstrlen (szCurDir);
		ASSERT (nCurDir >= 0);

		szCurDir [nCurDir] = _T('\\');
		szCurDir [++ nCurDir] = _T('\0');

		//---------------
		// Add MRU files:
		//---------------
		int iNumOfFiles = 0;	// Actual added to menu
		for (int i = 0; i < pMRUFiles->GetSize (); i ++)
		{
			CString strFile = (*pMRUFiles) [i];
			if (!strFile.IsEmpty ())
			{
				CString strPath;
				CString strName;
				int iImage = -1;

				int iIndex = strFile.ReverseFind (_T('\\'));
				if (iIndex != -1)
				{
					strPath = strFile.Left (iIndex);
					strName = strFile.Mid (iIndex + 1);
				}
				else
				{
					strName = strFile;
				}

				SHFILEINFO  sfi;
				HIMAGELIST himlSmall = (HIMAGELIST) SHGetFileInfo (strFile,
                                       0,
                                       &sfi, 
                                       sizeof(SHFILEINFO), 
                                       SHGFI_SYSICONINDEX | SHGFI_SMALLICON);

				if (himlSmall != NULL)
				{
					CImageList* pImages = CImageList::FromHandle (himlSmall);
					ASSERT (pImages != NULL);

					iImage = m_ImagesRecent.Add (pImages->ExtractIcon (sfi.iIcon));
				}

				iIndex = m_wndRecentList.InsertItem (iNumOfFiles ++, strName, iImage);
				m_wndRecentList.SetItemText (iIndex, 1, strPath);

				int iPathWidth = m_wndRecentList.GetStringWidth (strPath) + 20;
				if (iPathWidth > m_wndRecentList.GetColumnWidth (1))
				{
					m_wndRecentList.SetColumnWidth (1, iPathWidth);
				}
			}
		}
	}

	//---------------------
	// Create tabs control:
	//---------------------
	CRect rectTabs;
	pFD->GetClientRect (rectTabs);
	rectTabs.DeflateRect (4, 4);
	rectTabs.top += m_iLogoAreaHeight;

	m_wndTab.Create (CBCGTabWnd::STYLE_3D, rectTabs, pFD, iTabCtrlId, 
		CBCGTabWnd::LOCATION_TOP);
	m_wndTab.m_pParent = this;

	m_wndTab.SetFont (GetFont ());
	m_wndTab.SetOwner (this);

	m_wndDummy.Create (_T(""), WS_CHILD, CRect (0, 0, 0, 0), this);

	{
		CBCGLocalResource locaRes;
		CString strTab;

		if (m_bNewPage)
		{
			m_wndTab.AddTab (&m_wndDummy, IDS_BCGBARRES_NEW_FILE, (UINT)-1);
		}

		m_wndTab.AddTab (&m_wndDummy, IDS_BCGBARRES_EXISTING, (UINT)-1);
		m_wndTab.AddTab (&m_wndDummy, IDS_BCGBARRES_RECENT, (UINT)-1);
	}

	pFD->CenterWindow ();
	pFD->SetWindowText (m_strCaption);

	//------------------
	// Set dilaog icons:
	//------------------
	if (m_hIconSmall != NULL)
	{
		pFD->SetIcon (m_hIconSmall, FALSE);
	}

	if (m_hIconBig != NULL)
	{
		pFD->SetIcon (m_hIconBig, TRUE);
	}

	//--------------------------
	// Setup parent window proc:
	//--------------------------
	m_wndProc = (WNDPROC)SetWindowLongPtr(pFD->m_hWnd, GWLP_WNDPROC, 
		(LONG_PTR) CBCGFileDialog::WindowProcNew);
}
//******************************************************************************************
void CBCGFileDialog::OnTabSelchange() 
{
	m_wndDummy,ShowWindow (SW_HIDE);

	int nPage = m_wndTab.GetActiveTab ();
	if (!m_bNewPage)
	{
		nPage ++;
	}

	switch (nPage)
	{
	case 0:
		m_nPage = BCGFileNew;
		break;

	case 1:
		m_nPage = BCGFileOpen;
		break;

	case 2:
		m_nPage = BCGFileRecent;
		break;

	default:
		ASSERT (FALSE);
	}

	//---------------------
	// Show/hide file list:
	//---------------------
	CWnd* pWnd = GetParent();
	ASSERT (pWnd != NULL);

	CWnd* pWndChild = pWnd->GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		TCHAR szClass [256];
		::GetClassName (pWndChild->GetSafeHwnd (), szClass, 255);

		CString strClass = szClass;

		if (strClass.CompareNoCase (_T("SHELLDLL_DefView")) == 0)
		{
			pWndChild->ShowWindow (m_nPage == BCGFileOpen ? SW_SHOW : SW_HIDE);
			break;
		}

		pWndChild = pWndChild->GetNextWindow ();
	}

	//--------------------------
	// Show/hide other controls:
	//--------------------------
	for (POSITION pos = m_lstFDControls.GetHeadPosition (); pos != NULL;)
	{
		pWnd = CWnd::FromHandle (m_lstFDControls.GetNext (pos));
		ASSERT (pWnd != NULL);

		pWnd->ShowWindow (m_nPage == BCGFileOpen ? SW_SHOW : SW_HIDE);
	}

	m_wndNewList.ShowWindow (m_nPage == BCGFileNew ? SW_SHOW : SW_HIDE);
	m_wndRecentList.ShowWindow (m_nPage == BCGFileRecent ? SW_SHOW : SW_HIDE);
}
//***************************************************************************************
void CBCGFileDialog::OnItemDblClick ()
{
	ASSERT (m_nPage != BCGFileOpen);

	CListCtrl& list = (m_nPage == BCGFileRecent) ? m_wndRecentList : m_wndNewList;
	int iSelIndex = list.GetNextItem (-1, LVNI_ALL | LVNI_SELECTED);

	if (iSelIndex == -1)
	{
		return;
	}

	if (m_nPage == BCGFileRecent)
	{
		CString strPath = list.GetItemText (iSelIndex, 1);
		CString strName = list.GetItemText (iSelIndex, 0);

		if (strPath.IsEmpty ())
		{
			m_strRecentFilePath = strName;
		}
		else
		{
			m_strRecentFilePath = strPath;
			m_strRecentFilePath += _T('\\');
			m_strRecentFilePath += strName;
		}
	}
	else
	{
		ASSERT (m_nPage == BCGFileNew);
		m_iNewItemIndex = iSelIndex;
	}
	
	CDialog* pWnd = (CDialog*) GetParent();
	ASSERT (pWnd != NULL);

	pWnd->EndDialog (IDOK);
}
//****************************************************************************************
void CBCGFileDialog::CollectControls ()
{
	if (!m_lstFDControls.IsEmpty ())
	{
		return;
	}

	CWnd* pWnd = GetParent();
	ASSERT (pWnd != NULL);

	CRect rectList (0, 0, 0, 0);

	CWnd* pWndChild = pWnd->GetWindow (GW_CHILD);
	while (pWndChild != NULL)
	{
		BOOL bIsFileList = FALSE;

		UINT uiID = pWndChild->GetDlgCtrlID();
		TCHAR szClass [256];
		::GetClassName (pWndChild->GetSafeHwnd (), szClass, 255);

		CString strClass = szClass;

		CRect rectCtl;
		pWndChild->GetClientRect (rectCtl);
		pWndChild->MapWindowPoints (pWnd, rectCtl);

		if (strClass.CompareNoCase (_T("SHELLDLL_DefView")) == 0)
		{
			rectList.left = rectCtl.left;
			rectList.right = rectCtl.right;
			rectList.bottom = rectCtl.bottom - 10;

			bIsFileList = TRUE;
		}

		if (strClass.CompareNoCase (_T("ToolbarWindow32")) == 0)
		{
			rectList.top = rectCtl.top;
		}

		if ((((strClass.CompareNoCase (_T("BUTTON")) != 0) ||
			uiID != IDOK &&
			uiID != IDCANCEL &&
			uiID != IDHELP) &&
			pWndChild->GetStyle () & WS_VISIBLE) &&
			uiID != iTabCtrlId &&
			uiID != iNewListCtrlId &&
			uiID != iRecentListCtrlId &&
			!bIsFileList)
		{
			m_lstFDControls.AddTail (pWndChild->GetSafeHwnd ());
		}

		pWndChild = pWndChild->GetNextWindow ();
	}

	m_wndNewList.MoveWindow (rectList);
	m_wndRecentList.MoveWindow (rectList);

	OnTabSelchange();
}
//**************************************************************************************
void CBCGFileDialog::AddNewItem (LPCTSTR lpszName, int iIconIndex)
{
	m_lstNewItems.AddTail (new CNewItemInfo (lpszName, iIconIndex));
}
//**************************************************************************************
void CBCGFileDialog::SetDlgIcon (HICON hIconBig, HICON hIconSmall/* = NULL*/)
{
	m_hIconBig = hIconBig;
	m_hIconSmall = (hIconSmall == NULL) ? m_hIconBig : hIconSmall;
}

BEGIN_MESSAGE_MAP(CBCGFDTabWnd, CBCGTabWnd)
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CBCGFDTabWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rectClient;
	GetClientRect (rectClient);
	pDC->FillRect (rectClient, &globalData.brBtnFace);

	return TRUE;
}

void CBCGFDTabWnd::FireChangeActiveTab (int /*nNewTab*/)
{
	if (m_pParent != NULL)
	{
		m_pParent->OnTabSelchange ();
	}
}
