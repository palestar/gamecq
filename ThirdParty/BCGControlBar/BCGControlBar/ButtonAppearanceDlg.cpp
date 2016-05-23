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

// ButtonAppearanceDlg.cpp : implementation file
//

#include "stdafx.h"

#ifndef BCG_NO_CUSTOMIZATION

#include "ButtonAppearanceDlg.h"
#include "BCGToolBarImages.h"
#include "BCGToolbarButton.h"
#include "ImageEditDlg.h"
#include "bcglocalres.h"
#include "BCGToolBar.h"
#include "BCGUserToolsManager.h"
#include "BCGUserTool.h"
#include "globals.h"
#include "CBCGToolbarCustomize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CBCGToolbarCustomize* g_pWndCustomize;

/////////////////////////////////////////////////////////////////////////////
// CButtonAppearanceDlg dialog

CButtonAppearanceDlg::CButtonAppearanceDlg(
	CBCGToolbarButton* pButton,
	CBCGToolBarImages* pImages,
	CWnd* pParent,
	int iStartImage,
	BOOL bMenuMode)
		: CDialog(CButtonAppearanceDlg::IDD, pParent),
		m_pButton (pButton),
		m_pImages (pImages),
		m_iStartImage (iStartImage),
		m_bMenuMode (bMenuMode),
		m_pUserTool (NULL)
{
	ASSERT_VALID(m_pButton);
	
	if (g_pUserToolsManager != NULL)
	{
		m_pUserTool = g_pUserToolsManager->FindTool (m_pButton->m_nID);
		if (m_pUserTool != NULL)
		{
			ASSERT_VALID (m_pUserTool);
		}
	}

	m_bUserButton = pButton->m_bUserButton || 
		(m_pUserTool == NULL &&
		CBCGToolBar::GetDefaultImage (m_pButton->m_nID) < 0);
	
	m_iSelImage = pButton->GetImage ();
	m_bImage = pButton->m_bImage;

	if (m_bMenuMode && CMD_MGR.IsMenuItemWithoutImage (pButton->m_nID))
	{
		m_bImage = FALSE;
	}

	if (m_bMenuMode || m_pButton->m_bTextBelow)
	{
		m_bText = TRUE;
	}
	else
	{
		m_bText = pButton->m_bText;
	}

	//{{AFX_DATA_INIT(CButtonAppearanceDlg)
	m_strButtonText = _T("");
	m_strButtonDescr = _T("");
	//}}AFX_DATA_INIT
}

CButtonAppearanceDlg::~CButtonAppearanceDlg ()
{
	while (!m_Buttons.IsEmpty ())
	{
		delete m_Buttons.RemoveHead ();
	}	
}

void CButtonAppearanceDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CButtonAppearanceDlg)
	DDX_Control(pDX, IDC_BCGBARRES_DEFAULT_IMAGE, m_wndDefautImageBtn);
	DDX_Control(pDX, IDC_BCGBARRES_USER_IMAGE, m_wndUserImageBtn);
	DDX_Control(pDX, IDC_BCGBARRES_DEFAULT_IMAGE_AREA, m_wndDefaultImageArea);
	DDX_Control(pDX, IDC_BCGBARRES_BUTTON_TEXT, m_wndButtonText);
	DDX_Control(pDX, IDC_BCGBARRES_ADD_IMAGE, m_wndAddImage);
	DDX_Control(pDX, IDC_BCGBARRES_IMAGE_LIST, m_wndButtonList);
	DDX_Control(pDX, IDC_BCGBARRES_EDIT_IMAGE, m_wndEditImage);
	DDX_Text(pDX, IDC_BCGBARRES_BUTTON_TEXT, m_strButtonText);
	DDX_Text(pDX, IDC_BCGBARRES_BUTTON_DESCR, m_strButtonDescr);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CButtonAppearanceDlg, CDialog)
	//{{AFX_MSG_MAP(CButtonAppearanceDlg)
	ON_BN_CLICKED(IDC_BCGBARRES_ADD_IMAGE, OnAddImage)
	ON_BN_CLICKED(IDC_BCGBARRES_EDIT_IMAGE, OnEditImage)
	ON_BN_CLICKED(IDC_BCGBARRES_IMAGE_LIST, OnImageList)
	ON_BN_CLICKED(IDC_BCGBARRES_IMAGE, OnImage)
	ON_BN_CLICKED(IDC_BCGBARRES_IMAGE_TEXT, OnImageText)
	ON_BN_CLICKED(IDC_BCGBARRES_TEXT, OnText)
	ON_BN_CLICKED(IDC_BCGBARRES_USER_IMAGE, OnUserImage)
	ON_BN_CLICKED(IDC_BCGBARRES_DEFAULT_IMAGE, OnDefaultImage)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CButtonAppearanceDlg message handlers

void CButtonAppearanceDlg::OnAddImage() 
{
	CBCGLocalResource locaRes;

	ASSERT (m_pImages != NULL);
	CSize sizeImage = m_pImages->GetImageSize ();

	try
	{
		CClientDC	dc (&m_wndButtonList);
		CBitmap 	bitmap;
		CDC 		memDC;	

		memDC.CreateCompatibleDC(&dc);
		
		if (!bitmap.CreateCompatibleBitmap (&dc, sizeImage.cx, sizeImage.cy))
		{
			AfxMessageBox (IDP_BCGBARRES_CANNT_CREATE_IMAGE);
			return;
		}

		CBitmap* pOldBitmap = memDC.SelectObject (&bitmap);

		CRect rect (0, 0, sizeImage.cx, sizeImage.cy);
		memDC.FillRect (CRect (0, 0, sizeImage.cx, sizeImage.cy),
						&globalData.brBtnFace);

		memDC.SelectObject (pOldBitmap);

		BITMAP bmp;
		::GetObject (m_pImages->GetImageWell (), sizeof (BITMAP), (LPVOID)&bmp);

		if (g_pWndCustomize != NULL)
		{
			ASSERT_VALID (g_pWndCustomize);

			if (!g_pWndCustomize->OnEditToolbarMenuImage (this, bitmap, bmp.bmBitsPixel))
			{
				return;
			}
		}
		else
		{
			CBCGImageEditDlg dlg (&bitmap, this, bmp.bmBitsPixel);
			if (dlg.DoModal () != IDOK)
			{
				return;
			}
		}

		int iImageIndex = m_pImages->AddImage ((HBITMAP) bitmap);
		if (iImageIndex < 0)
		{
			AfxMessageBox (IDP_BCGBARRES_CANNT_CREATE_IMAGE);
			return;
		}

		RebuildImageList ();
		m_wndButtonList.SelectButton (iImageIndex);
	}
	catch (...)
	{
		CBCGLocalResource locaRes;
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}
}
//********************************************************************************
void CButtonAppearanceDlg::OnEditImage() 
{
	ASSERT (m_pImages != NULL);
	ASSERT (m_iSelImage >= 0);

	CSize sizeImage = m_pImages->GetImageSize ();

	try
	{
		CClientDC	dc (&m_wndButtonList);
		CBitmap 	bitmap;
		CDC 		memDC;
		memDC.CreateCompatibleDC(&dc);
		
		if (!bitmap.CreateCompatibleBitmap (&dc, sizeImage.cx, sizeImage.cy))
		{
			return;
		}

		const COLORREF clrGrayStd = RGB (192, 192, 192);

		CBitmap* pOldBitmap = memDC.SelectObject (&bitmap);
		COLORREF clrTransparent = m_pImages->SetTransparentColor (clrGrayStd);

		memDC.FillSolidRect (CRect (0, 0, sizeImage.cx, sizeImage.cy), clrGrayStd);

		CBCGDrawState ds;
		if (!m_pImages->PrepareDrawImage (ds))
		{
			return;
		}

		m_pImages->Draw (&memDC, 0, 0, m_iSelImage);
		m_pImages->EndDrawImage (ds);

		m_pImages->SetTransparentColor (clrTransparent);

		memDC.SelectObject (pOldBitmap);

		BITMAP bmp;
		::GetObject (m_pImages->GetImageWell (), sizeof (BITMAP), (LPVOID)&bmp);

		if (g_pWndCustomize != NULL)
		{
			ASSERT_VALID (g_pWndCustomize);

			if (!g_pWndCustomize->OnEditToolbarMenuImage (this, bitmap, bmp.bmBitsPixel))
			{
				return;
			}
		}
		else
		{
			CBCGImageEditDlg dlg (&bitmap, this, bmp.bmBitsPixel);
			if (dlg.DoModal () != IDOK)
			{
				return;
			}
		}

		m_pImages->UpdateImage (m_iSelImage, (HBITMAP) bitmap);
		m_wndButtonList.Invalidate ();
	}
	catch (...)
	{
		CBCGLocalResource locaRes;
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}
}
//********************************************************************************
void CButtonAppearanceDlg::OnImageList() 
{
	CBCGToolbarButton* pSelButton = m_wndButtonList.GetSelectedButton ();
	m_iSelImage = (pSelButton == NULL) ? -1 : pSelButton->GetImage ();

	m_wndEditImage.EnableWindow (m_iSelImage >= 0 &&
		m_pImages != NULL && !m_pImages->IsReadOnly ());
}
//********************************************************************************
void CButtonAppearanceDlg::OnImage() 
{
	m_bImage = TRUE;
	m_bText = FALSE;

	EnableControls ();
}
//********************************************************************************
void CButtonAppearanceDlg::OnImageText() 
{
	m_bImage = TRUE;
	m_bText = TRUE;

	EnableControls ();
}
//********************************************************************************
void CButtonAppearanceDlg::OnText() 
{
	m_bImage = FALSE;
	m_bText = TRUE;

	EnableControls ();
}
//********************************************************************************
void CButtonAppearanceDlg::OnOK() 
{
	CBCGLocalResource locaRes;

	UpdateData ();

	int iImage = m_iSelImage;
	if (!m_bUserButton)
	{
		iImage = m_pUserTool != NULL ? 0 :
			CBCGToolBar::GetDefaultImage (m_pButton->m_nID);
	}

	if (m_bImage && iImage < 0)
	{
		CString str;
		str.LoadString (IDP_BCGBARRES_IMAGE_IS_REQUIRED);

		AfxMessageBox (str);

		m_wndButtonList.SetFocus ();
		return;
	}

	if (m_bText && m_strButtonText.IsEmpty ())
	{
		CString str;
		str.LoadString (IDP_BCGBARRES_TEXT_IS_REQUIRED);

		AfxMessageBox (str);

		m_wndButtonText.SetFocus ();
		return;
	}

	if (!m_pButton->m_bTextBelow)
	{
		m_pButton->m_bText = m_bText;
	}

	if (m_bMenuMode)
	{
		CMD_MGR.EnableMenuItemImage (m_pButton->m_nID, m_bImage);
	}
	else
	{
		m_pButton->m_bImage = m_bImage;
	}

	m_pButton->m_bUserButton = m_bUserButton;
	m_pButton->SetImage (iImage);
	m_pButton->m_strText = m_strButtonText;

	if (!m_strAccel.IsEmpty ())
	{
		m_pButton->m_strText += _T('\t');
		m_pButton->m_strText += m_strAccel;
	}

	CDialog::OnOK();
}
//********************************************************************************
BOOL CButtonAppearanceDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	if (m_pImages != NULL)
	{
		m_wndButtonList.SetImages (m_pImages);
		RebuildImageList ();
		m_wndButtonList.SelectButton (m_iSelImage);
	}
	else
	{
		m_wndButtonList.EnableWindow (FALSE);
		m_wndUserImageBtn.EnableWindow (FALSE);

		if (m_iSelImage < 0)
		{
			CWnd* pWndImage = GetDlgItem (IDC_BCGBARRES_IMAGE);
			VERIFY(pWndImage != NULL);
			pWndImage->EnableWindow (FALSE);

			CWnd* pWndImageText = GetDlgItem (IDC_BCGBARRES_IMAGE_TEXT);
			VERIFY(pWndImageText != NULL);
			pWndImageText->EnableWindow (FALSE);
		}
	}

	if (m_bUserButton && !m_pButton->IsLocked ())
	{
		m_wndUserImageBtn.SetCheck (1);
	}
	else
	{
		m_wndDefautImageBtn.SetCheck (1);
	}

	if (m_bImage)
	{
		if (m_bText)
		{
			CheckDlgButton (IDC_BCGBARRES_IMAGE_TEXT, TRUE);
		}
		else
		{
			CheckDlgButton (IDC_BCGBARRES_IMAGE, TRUE);
		}
	}
	else
	{
		ASSERT (m_bText);
		CheckDlgButton (IDC_BCGBARRES_TEXT, TRUE);
		m_bText = TRUE;

		m_wndButtonList.EnableWindow (FALSE);
	}

	int iTabOffset = m_pButton->m_strText.Find (_T('\t'));
	if (iTabOffset >= 0)
	{
		m_strButtonText = m_pButton->m_strText.Left (iTabOffset);
		m_strAccel = m_pButton->m_strText.Mid (iTabOffset + 1);
	}
	else
	{
		m_strButtonText = m_pButton->m_strText;
	}

	CFrameWnd* pWndFrame = GetParentFrame ();
	if (pWndFrame != NULL)
	{
		pWndFrame->GetMessageString (m_pButton->m_nID,
						m_strButtonDescr);
	}

	if (m_bMenuMode)
	{
		if (m_iSelImage < 0)
		{
			CWnd* pWndText = GetDlgItem (IDC_BCGBARRES_TEXT);
			VERIFY(pWndText != NULL);
			pWndText->EnableWindow (FALSE);

			CWnd* pWndImageText = GetDlgItem (IDC_BCGBARRES_IMAGE_TEXT);
			VERIFY(pWndImageText != NULL);
			pWndImageText->EnableWindow (FALSE);
		}

		CWnd* pWndImage = GetDlgItem (IDC_BCGBARRES_IMAGE);
		VERIFY(pWndImage != NULL);
		pWndImage->EnableWindow (FALSE);

	}

	if (m_pButton->m_bTextBelow)
	{
		CWnd* pWndImage = GetDlgItem (IDC_BCGBARRES_IMAGE);
		VERIFY(pWndImage != NULL);
		pWndImage->EnableWindow (FALSE);
	}

	m_wndDefaultImageArea.GetClientRect (&m_rectDefaultImage);
	m_wndDefaultImageArea.MapWindowPoints (this, &m_rectDefaultImage);

	CSize sizePreview (16, 16);

	CBCGToolBarImages* pImages = CBCGToolBar::GetImages ();
	if (pImages != NULL)
	{
		CSize sizeImage = pImages->GetImageSize ();

		sizePreview.cx = min (sizePreview.cx, sizeImage.cx);
		sizePreview.cy = min (sizePreview.cy, sizeImage.cy);
	}

	m_rectDefaultImage.right = m_rectDefaultImage.left + sizePreview.cx;
	m_rectDefaultImage.bottom = m_rectDefaultImage.top + sizePreview.cy;

	EnableControls ();	
	UpdateData (FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//********************************************************************************
void CButtonAppearanceDlg::RebuildImageList ()
{
	m_wndButtonList.RemoveButtons ();

	while (!m_Buttons.IsEmpty ())
	{
		delete m_Buttons.RemoveHead ();
	}

	int iEnd = m_pImages->GetCount () - 1;
	for (int iImage = m_iStartImage; iImage <= iEnd; iImage ++)
	{
		CBCGToolbarButton* pButton = new CBCGToolbarButton;

		pButton->SetImage (iImage);

		m_wndButtonList.AddButton (pButton);
		m_Buttons.AddTail (pButton);
	}

	m_wndButtonList.Invalidate ();
}
//********************************************************************************
void CButtonAppearanceDlg::EnableControls ()
{
	ASSERT_VALID (this);
	ASSERT_VALID (m_pButton);

	BOOL bLocked = m_pButton->IsLocked ();

	m_wndButtonText.EnableWindow (m_bText && (!m_bMenuMode || m_pUserTool == NULL) && !CBCGToolBar::m_bDisableLabelsEdit);

	m_wndButtonList.EnableWindow (m_bImage && m_pImages != NULL && m_bUserButton && !m_bMenuMode && !bLocked);
	m_wndAddImage.EnableWindow (m_bImage && m_pImages != NULL && m_bUserButton && 
		!m_bMenuMode && !m_pImages->IsReadOnly () && !bLocked);
	m_wndEditImage.EnableWindow (m_bImage && m_pImages != NULL && m_iSelImage >= 0 && 
		m_bUserButton && !m_pImages->IsReadOnly () && !bLocked);

	m_wndUserImageBtn.EnableWindow (m_bImage && m_pImages != NULL && !m_bMenuMode &&
									!bLocked);

	m_wndDefautImageBtn.EnableWindow (m_pUserTool != NULL ||
		(m_bImage && CBCGToolBar::GetDefaultImage (m_pButton->m_nID) >= 0) &&
		!m_bMenuMode && !bLocked);

	InvalidateRect (&m_rectDefaultImage);
}
//******************************************************************
void CButtonAppearanceDlg::OnUserImage() 
{
	m_iSelImage = -1;
	m_bUserButton = TRUE;
	m_wndDefautImageBtn.SetCheck (0);
	EnableControls ();

	m_wndButtonList.SelectButton (-1);
}
//******************************************************************
void CButtonAppearanceDlg::OnDefaultImage() 
{
	m_iSelImage = m_pButton->GetImage ();
	m_bUserButton = FALSE;
	m_wndUserImageBtn.SetCheck (0);
	EnableControls ();
}
//*******************************************************************
void CButtonAppearanceDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (m_pUserTool != NULL)
	{
		m_pUserTool->DrawToolIcon (&dc, m_rectDefaultImage);
		return;
	}

	if (m_pButton->IsLocked ())
	{
		BOOL bText = m_pButton->m_bText;
		BOOL bImage = m_pButton->m_bImage;

		m_pButton->m_bText = FALSE;
		m_pButton->m_bImage = TRUE;

		m_pButton->OnDraw (&dc, m_rectDefaultImage, NULL, TRUE, FALSE, FALSE, FALSE, FALSE);

		m_pButton->m_bText = bText;
		m_pButton->m_bImage = bImage;

		return;
	}

	int iImage = CBCGToolBar::GetDefaultImage (m_pButton->m_nID);
	if (iImage < 0 || !m_bImage)
	{
		return;
	}

	CBCGToolBarImages* pImages = CBCGToolBar::GetImages ();
	ASSERT (pImages != NULL);

	CBCGDrawState ds;
	pImages->PrepareDrawImage (ds, m_rectDefaultImage.Size ());
	
	pImages->Draw (&dc, m_rectDefaultImage.left, m_rectDefaultImage.top, iImage);
	pImages->EndDrawImage (ds);
}

#endif // BCG_NO_CUSTOMIZATION
