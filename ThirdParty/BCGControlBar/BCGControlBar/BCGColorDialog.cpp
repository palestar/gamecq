// BCGColorDialog.cpp : implementation file
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.
//

#include "stdafx.h"

#include "bcgcontrolbar.h"
#include "bcglocalres.h"
#include "BCGColorDialog.h"
#include "PropertySheetCtrl.h"
#include "globals.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScreenWnd window

class CScreenWnd : public CWnd
{
// Construction
public:
	CScreenWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CScreenWnd)
	public:
	virtual BOOL Create(CBCGColorDialog* pColorDlg);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CScreenWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CScreenWnd)
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	CBCGColorDialog*	m_pColorDlg;
};


/////////////////////////////////////////////////////////////////////////////
// CBCGColorDialog dialog

CBCGColorDialog::CBCGColorDialog (COLORREF clrInit, DWORD /*dwFlags - reserved */, 
					CWnd* pParentWnd, HPALETTE hPal)
	: CBCGDialog(CBCGColorDialog::IDD, pParentWnd)
{
	//{{AFX_DATA_INIT(CBCGColorDialog)
	//}}AFX_DATA_INIT

	m_pColourSheetOne = NULL;
	m_pColourSheetTwo = NULL;

	m_CurrentColor = m_NewColor = clrInit;
	m_pPropSheet = NULL;
	m_bIsMyPalette = TRUE;
	m_pPalette = NULL;
	
	if (hPal != NULL)
	{
		m_pPalette = CPalette::FromHandle (hPal);
		m_bIsMyPalette = FALSE;
	}

	m_bPickerMode = FALSE;
}

CBCGColorDialog::~CBCGColorDialog ()
{
	if (m_pColourSheetOne != NULL)
	{
		delete m_pColourSheetOne;
	}

	if (m_pColourSheetTwo != NULL)
	{
		delete m_pColourSheetTwo;
	}
}

void CBCGColorDialog::DoDataExchange(CDataExchange* pDX)
{
	CBCGDialog::DoDataExchange(pDX);
	
	//{{AFX_DATA_MAP(CBCGColorDialog)
	DDX_Control(pDX, IDC_BCGBARRES_COLOR_SELECT, m_btnColorSelect);
	DDX_Control(pDX, IDC_BCGBARRES_STATICPLACEHOLDER, m_wndStaticPlaceHolder);
	DDX_Control(pDX, IDC_BCGBARRES_COLOURPLACEHOLDER, m_wndColors);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CBCGColorDialog, CBCGDialog)
	//{{AFX_MSG_MAP(CBCGColorDialog)
	ON_WM_DESTROY()
	ON_WM_SYSCOLORCHANGE()
	ON_BN_CLICKED(IDC_BCGBARRES_COLOR_SELECT, OnColorSelect)
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGColorDialog message handlers

BOOL CBCGColorDialog::OnInitDialog()
{
	CBCGLocalResource locaRes;

	CBCGDialog::OnInitDialog();

	if (AfxGetMainWnd () != NULL && 
		(AfxGetMainWnd ()->GetExStyle () & WS_EX_LAYOUTRTL))
	{
		ModifyStyleEx (0, WS_EX_LAYOUTRTL);
	}

	if (globalData.m_nBitsPerPixel < 8)	// 16 colors, call standard dialog
	{
		CColorDialog dlg (m_CurrentColor, CC_FULLOPEN | CC_ANYCOLOR);
		int nResult = (int) dlg.DoModal ();
		m_NewColor = dlg.GetColor ();
		EndDialog (nResult);

		return TRUE;
	}

	if (m_pPalette == NULL)
	{
		m_pPalette = new CPalette ();
		RebuildPalette ();
	}

	m_wndColors.SetType (CBCGColorPickerCtrl::CURRENT);
	m_wndColors.SetPalette (m_pPalette);

	m_wndColors.SetOriginalColor (m_CurrentColor);
	m_wndColors.SetRGB (m_NewColor);

	// Create property sheet.
	m_pPropSheet = new CPropertySheetCtrl(_T(""), this);
	ASSERT(m_pPropSheet);

	m_pColourSheetOne = new CColorPage1;
	m_pColourSheetTwo = new CColorPage2;

	// Set parent dialog.
	m_pColourSheetOne->m_pDialog = this;
	m_pColourSheetTwo->m_pDialog = this;

	m_pPropSheet->AddPage (m_pColourSheetOne);
	m_pPropSheet->AddPage (m_pColourSheetTwo);

	// Retrieve the location of the window
	CRect rectListWnd;
	m_wndStaticPlaceHolder.GetWindowRect(rectListWnd);
	ScreenToClient(rectListWnd);
	
	if (!m_pPropSheet->Create(this, WS_CHILD | WS_CLIPSIBLINGS | WS_VISIBLE, 0))
	{
		TRACE0 ("CBCGColorDialog::OnInitDialog(): Can't create the property sheet.....\n");
	}

	m_pPropSheet->SetWindowPos(NULL, rectListWnd.left, rectListWnd.top, rectListWnd.Width(),
				rectListWnd.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

	SetPageOne (GetRValue (m_CurrentColor), 
				GetGValue (m_CurrentColor),
				GetBValue (m_CurrentColor));

	SetPageTwo (GetRValue (m_CurrentColor),
				GetGValue (m_CurrentColor),
				GetBValue (m_CurrentColor));

	m_btnColorSelect.SetImage (IDB_BCGBARRES_COLOR_PICKER);

	m_hcurPicker = AfxGetApp()->LoadCursor (IDC_BCGBARRES_COLOR);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CBCGColorDialog::SetCurrentColor(COLORREF rgb)
{
	m_CurrentColor = rgb;
}

void CBCGColorDialog::SetPageOne(BYTE R, BYTE G, BYTE B)
{
	m_pColourSheetOne->m_hexpicker.SelectCellHexagon(R, G, B);
	m_pColourSheetOne->m_hexpicker_greyscale.SelectCellHexagon(R, G, B);
}

void CBCGColorDialog::SetPageTwo(BYTE R, BYTE G, BYTE B)
{
	m_pColourSheetTwo->Setup (R, G, B);
}

void CBCGColorDialog::OnDestroy() 
{
	if (m_bIsMyPalette && m_pPalette != NULL)
	{
		m_pPalette->DeleteObject ();
		delete m_pPalette;
		m_pPalette = NULL;
	}

	CBCGDialog::OnDestroy();
}

void CBCGColorDialog::SetNewColor (COLORREF rgb)
{
	m_NewColor = rgb;

	if (globalData.m_nBitsPerPixel == 8) // 256 colors
	{
		ASSERT (m_pPalette != NULL);

		UINT uiPalIndex = m_pPalette->GetNearestPaletteIndex (rgb);
		m_wndColors.SetRGB (PALETTEINDEX (uiPalIndex));
	}
	else
	{
		m_wndColors.SetRGB (rgb);
	}

	m_wndColors.Invalidate ();
	m_wndColors.UpdateWindow ();
}

void CBCGColorDialog::OnSysColorChange() 
{
	CBCGDialog::OnSysColorChange();

	globalData.UpdateSysColors ();
	
	if (m_bIsMyPalette)
	{
		if (globalData.m_nBitsPerPixel < 8)	// 16 colors, call standard dialog
		{
			ShowWindow (SW_HIDE);

			CColorDialog dlg (m_CurrentColor, CC_FULLOPEN | CC_ANYCOLOR);
			int nResult = (int) dlg.DoModal ();
			m_NewColor = dlg.GetColor ();
			EndDialog (nResult);
		}
		else
		{
			::DeleteObject (m_pPalette->Detach ());
			RebuildPalette ();

			Invalidate ();
			UpdateWindow ();
		}
	}
}

void CBCGColorDialog::RebuildPalette ()
{
	ASSERT (m_pPalette->GetSafeHandle () == NULL);

	// Create palette:
	CClientDC dc (this);

	int nColors = 256;	// Use 256 first entries
	UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
	LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

	pLP->palVersion = 0x300;
	pLP->palNumEntries = (USHORT) nColors;

	::GetSystemPaletteEntries (dc.GetSafeHdc (), 0, nColors, pLP->palPalEntry);

	m_pPalette->CreatePalette (pLP);

	delete[] pLP;
}

void CBCGColorDialog::OnColorSelect() 
{
	if (m_bPickerMode)
	{
		return;
	}

	CWinThread* pCurrThread = ::AfxGetThread ();
	if (pCurrThread == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	MSG msg;
	m_bPickerMode = TRUE;;
	::SetCursor (m_hcurPicker);

	CScreenWnd* pScreenWnd = new CScreenWnd;
	if (!pScreenWnd->Create (this))
	{
		return;
	}

	SetForegroundWindow ();
	BringWindowToTop ();

	SetCapture ();

	COLORREF colorSaved = m_NewColor;

	while (m_bPickerMode)
	{
		while (m_bPickerMode && ::PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_KEYDOWN)
			{
				switch (msg.wParam)
				{
				case VK_RETURN:
					m_bPickerMode = FALSE;
					break;

				case VK_ESCAPE:
					SetNewColor (colorSaved);
					m_bPickerMode = FALSE;
					break;
				}
			}
			else
			{
				if (!pCurrThread->PreTranslateMessage (&msg))
				{
					::TranslateMessage (&msg);
					::DispatchMessage (&msg);
				}

				pCurrThread->OnIdle (0);
			}
		}

		WaitMessage ();
	}

	ReleaseCapture ();
	pScreenWnd->DestroyWindow ();
	delete pScreenWnd;

	m_bPickerMode = FALSE;
}

BOOL CBCGColorDialog::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) 
{
	if (m_bPickerMode)
	{
		::SetCursor (m_hcurPicker);
		return TRUE;
	}
	
	return CBCGDialog::OnSetCursor(pWnd, nHitTest, message);
}

void CBCGColorDialog::OnMouseMove(UINT nFlags, CPoint point) 
{
	if (m_bPickerMode)
	{
		ClientToScreen (&point);

		CClientDC dc(NULL);
		SetNewColor (dc.GetPixel(point));
	}

	CBCGDialog::OnMouseMove(nFlags, point);
}

void CBCGColorDialog::OnLButtonDown(UINT nFlags, CPoint point) 
{
	m_bPickerMode = FALSE;

	SetPageOne (GetRValue (m_NewColor), 
				GetGValue (m_NewColor),
				GetBValue (m_NewColor));

	SetPageTwo (GetRValue (m_NewColor),
				GetGValue (m_NewColor),
				GetBValue (m_NewColor));

	CBCGDialog::OnLButtonDown(nFlags, point);
}

/////////////////////////////////////////////////////////////////////////////
// CScreenWnd

CScreenWnd::CScreenWnd()
{
}

CScreenWnd::~CScreenWnd()
{
}


BEGIN_MESSAGE_MAP(CScreenWnd, CWnd)
	//{{AFX_MSG_MAP(CScreenWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_SETCURSOR()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CScreenWnd message handlers

BOOL CScreenWnd::Create(CBCGColorDialog* pColorDlg) 
{
	CWnd* pWndDesktop = GetDesktopWindow ();
	if (pWndDesktop == NULL)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_pColorDlg = pColorDlg;
	
	CRect rectScreen;
	pWndDesktop->GetWindowRect (rectScreen);

	CString strClassName = ::AfxRegisterWndClass (
			CS_SAVEBITS,
			AfxGetApp()->LoadCursor (IDC_BCGBARRES_COLOR),
			(HBRUSH)(COLOR_BTNFACE + 1), NULL);

	return CWnd::CreateEx (0, strClassName, _T(""), WS_VISIBLE | WS_POPUP, rectScreen, NULL, 0);
}

void CScreenWnd::OnMouseMove(UINT nFlags, CPoint point) 
{
	MapWindowPoints (m_pColorDlg, &point, 1);
	m_pColorDlg->SendMessage (WM_MOUSEMOVE, nFlags, MAKELPARAM (point.x, point.y));

	CWnd::OnMouseMove(nFlags, point);
}

void CScreenWnd::OnLButtonDown(UINT nFlags, CPoint point) 
{
	MapWindowPoints (m_pColorDlg, &point, 1);
	m_pColorDlg->SendMessage (WM_LBUTTONDOWN, nFlags, MAKELPARAM (point.x, point.y));
}

BOOL CScreenWnd::OnEraseBkgnd(CDC* /*pDC*/) 
{
	return TRUE;
}

BOOL CBCGColorDialog::PreTranslateMessage(MSG* pMsg) 
{
#ifdef _UNICODE
	#define _TCF_TEXT	CF_UNICODETEXT
#else
	#define _TCF_TEXT	CF_TEXT
#endif

	if (pMsg->message == WM_KEYDOWN)
	{
		UINT nChar = (UINT) pMsg->wParam;
		BOOL bIsCtrl = (::GetAsyncKeyState (VK_CONTROL) & 0x8000);

		if (bIsCtrl && (nChar == _T('C') || nChar == VK_INSERT))
		{
			if (OpenClipboard ())
			{
				EmptyClipboard ();

				CString strText;
				strText.Format (_T("RGB (%d, %d, %d)"),
					GetRValue (m_NewColor),
					GetGValue (m_NewColor),
					GetBValue (m_NewColor));

				HGLOBAL hClipbuffer = ::GlobalAlloc (GMEM_DDESHARE, (strText.GetLength () + 1) * sizeof (TCHAR));
				LPTSTR lpszBuffer = (LPTSTR) GlobalLock (hClipbuffer);

				_tcscpy (lpszBuffer, (LPCTSTR) strText);

				::GlobalUnlock (hClipbuffer);
				::SetClipboardData (_TCF_TEXT, hClipbuffer);

				CloseClipboard();
			}
		}
	}
	
	return CBCGDialog::PreTranslateMessage(pMsg);
}
