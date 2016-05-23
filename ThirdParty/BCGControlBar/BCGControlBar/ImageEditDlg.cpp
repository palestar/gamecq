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

// ImageEditDlg.cpp : implementation file
//

#include "stdafx.h"
#include <afxpriv.h>
#include "bcgbarres.h"
#include "globals.h"
#include "ImageEditDlg.h"
#include "bcglocalres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static void Create16ColorsStdPalette (CPalette& pal);

/////////////////////////////////////////////////////////////////////////////
// CBCGImageEditDlg dialog


#pragma warning (disable : 4355)

CBCGImageEditDlg::CBCGImageEditDlg(CBitmap* pBitmap, CWnd* pParent /*=NULL*/,
								   int nBitsPixel /* = -1 */)
	: CDialog(CBCGImageEditDlg::IDD, pParent),
	m_pBitmap (pBitmap),
	m_wndLargeDrawArea (this)
{
	ASSERT_VALID (m_pBitmap);

	BITMAP bmp;
	m_pBitmap->GetBitmap (&bmp);

	m_sizeImage = CSize (bmp.bmWidth, bmp.bmHeight);
	m_nBitsPixel = (nBitsPixel == -1) ? bmp.bmBitsPixel : nBitsPixel;

	ASSERT (m_nBitsPixel >= 4);	// Monochrome bitmaps are not supported

	//{{AFX_DATA_INIT(CBCGImageEditDlg)
	//}}AFX_DATA_INIT
}

#pragma warning (default : 4355)

void CBCGImageEditDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CBCGImageEditDlg)
	DDX_Control(pDX, IDC_BCGBARRES_COLORS, m_wndColorPickerLocation);
	DDX_Control(pDX, IDC_BCGBARRES_PALETTE, m_wndPaletteBarLocation);
	DDX_Control(pDX, IDC_BCGBARRES_PREVIEW_AREA, m_wndPreview);
	DDX_Control(pDX, IDC_BCGBARRES_DRAW_AREA, m_wndLargeDrawArea);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CBCGImageEditDlg, CDialog)
	//{{AFX_MSG_MAP(CBCGImageEditDlg)
	ON_WM_PAINT()
	ON_COMMAND(ID_BCG_TOOL_CLEAR, OnBcgToolClear)
	ON_COMMAND(ID_BCG_TOOL_COPY, OnBcgToolCopy)
	ON_COMMAND(ID_BCG_TOOL_PASTE, OnBcgToolPaste)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_PASTE, OnUpdateBcgToolPaste)
	ON_COMMAND(ID_BCG_TOOL_ELLIPSE, OnBcgToolEllipse)
	ON_COMMAND(ID_BCG_TOOL_FILL, OnBcgToolFill)
	ON_COMMAND(ID_BCG_TOOL_LINE, OnBcgToolLine)
	ON_COMMAND(ID_BCG_TOOL_PEN, OnBcgToolPen)
	ON_COMMAND(ID_BCG_TOOL_PICK, OnBcgToolPick)
	ON_COMMAND(ID_BCG_TOOL_RECT, OnBcgToolRect)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_ELLIPSE, OnUpdateBcgToolEllipse)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_FILL, OnUpdateBcgToolFill)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_LINE, OnUpdateBcgToolLine)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_PEN, OnUpdateBcgToolPen)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_PICK, OnUpdateBcgToolPick)
	ON_UPDATE_COMMAND_UI(ID_BCG_TOOL_RECT, OnUpdateBcgToolRect)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
	ON_COMMAND(IDC_BCGBARRES_COLORS, OnColors)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBCGImageEditDlg message handlers

BOOL CBCGImageEditDlg::OnInitDialog() 
{
	const int iBorderWidth = 10;
	const int iBorderHeight = 5;
	const int iPreviewBorderSize = 4;

	CDialog::OnInitDialog();

	m_wndLargeDrawArea.SetBitmap (m_pBitmap);

	//------------------------
	// Create the palette bar:
	//------------------------	
	{
		CBCGLocalResource locaRes;

		CRect rectPaletteBar;
		m_wndPaletteBarLocation.GetClientRect (&rectPaletteBar);
		m_wndPaletteBarLocation.MapWindowPoints (this, &rectPaletteBar);
		rectPaletteBar.DeflateRect (2, 2);

		m_wndPaletteBar.EnableLargeIcons (FALSE);
		m_wndPaletteBar.Create (this);

		const UINT uiToolbarHotID = globalData.Is32BitIcons () ? IDR_BCGRES_PALETTE32 : 0;

		m_wndPaletteBar.LoadToolBar (	IDR_BCGRES_PALETTE, 0, 0, 
										TRUE /* Locked bar */, 0, 0, uiToolbarHotID);

		m_wndPaletteBar.SetBarStyle(m_wndPaletteBar.GetBarStyle() |
			CBRS_TOOLTIPS | CBRS_FLYBY);
			
		m_wndPaletteBar.SetBarStyle (
			m_wndPaletteBar.GetBarStyle () & 
				~(CBRS_GRIPPER | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

		m_wndPaletteBar.SetBorders (iBorderWidth, iBorderHeight, 
									iBorderWidth, iBorderHeight);

		const int nButtonWidth = m_wndPaletteBar.GetButtonSize ().cx;
		m_wndPaletteBar.WrapToolBar (nButtonWidth * 3);
		m_wndPaletteBar.MoveWindow (rectPaletteBar);

		m_wndPaletteBar.SetWindowPos (&wndTop, -1, -1, -1, -1,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

		m_wndPaletteBar.SetOwner (this);

		// All commands will be routed via this dialog, not via the parent frame:
		m_wndPaletteBar.SetRouteCommandsViaFrame (FALSE);
	}

	// Create color picker:
	{
		CRect rectColorBar;
		m_wndColorPickerLocation.GetClientRect (&rectColorBar);
		m_wndColorPickerLocation.MapWindowPoints (this, &rectColorBar);
		rectColorBar.DeflateRect (2, 2);

		int nColumns = 4;

		m_wndColorBar.m_bInternal = TRUE;

		// If bitmap has 256 or less colors, create 16 colors palette:
		CPalette pal;
		if (m_nBitsPixel <= 8)
		{
			Create16ColorsStdPalette (pal);
		}
		else
		{
			m_wndColorBar.EnableOtherButton (_T("Other"));

			nColumns = 5;
			m_wndColorBar.SetVertMargin (1);
			m_wndColorBar.SetHorzMargin (1);
		}
		
		m_wndColorBar.CreateControl (this, rectColorBar, IDC_BCGBARRES_COLORS,
			nColumns, m_nBitsPixel <= 8 ? &pal : NULL);

		m_wndColorBar.SetColor (RGB (0, 0, 0));
	}
	
	//---------------------
	// Define preview area:
	//---------------------
	m_wndPreview.GetClientRect (&m_rectPreviewImage);
	m_wndPreview.MapWindowPoints (this, &m_rectPreviewImage);

	m_rectPreviewImage.left = (m_rectPreviewImage.left + m_rectPreviewImage.right - m_sizeImage.cx) / 2;
	m_rectPreviewImage.right = m_rectPreviewImage.left + m_sizeImage.cx;

	m_rectPreviewImage.top = (m_rectPreviewImage.top + m_rectPreviewImage.bottom - m_sizeImage.cy) / 2;
	m_rectPreviewImage.bottom = m_rectPreviewImage.top + m_sizeImage.cy;

	m_rectPreviewFrame = m_rectPreviewImage;
	m_rectPreviewFrame.InflateRect (iPreviewBorderSize, iPreviewBorderSize);

	m_wndLargeDrawArea.m_rectParentPreviewArea = m_rectPreviewImage;
	m_wndLargeDrawArea.ModifyStyle (WS_TABSTOP, 0);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
//********************************************************************************
void CBCGImageEditDlg::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	dc.FillRect (m_rectPreviewImage, &globalData.brBtnFace);

	CBitmap* pbmOld = NULL;
	CDC dcMem;
		
	dcMem.CreateCompatibleDC (&dc);
	pbmOld = dcMem.SelectObject (m_pBitmap);

	dc.BitBlt (m_rectPreviewImage.left, m_rectPreviewImage.top,
				m_sizeImage.cx, m_sizeImage.cy, &dcMem,
				0, 0, SRCCOPY);

	CRect rectBottom = m_rectPreviewFrame;
	rectBottom.top += m_sizeImage.cy + 2;

	dc.FillRect (rectBottom, &globalData.brBtnFace);

	dc.Draw3dRect (&m_rectPreviewFrame,
					globalData.clrBtnHilite,
					globalData.clrBtnShadow);

	dcMem.SelectObject(pbmOld);
	dcMem.DeleteDC();
}
//****************************************************************************************
LRESULT CBCGImageEditDlg::OnKickIdle(WPARAM, LPARAM)
{
	m_wndPaletteBar.OnUpdateCmdUI ((CFrameWnd*) this, TRUE);
    return 0;
}
//********************************************************************************
void CBCGImageEditDlg::OnColors() 
{
	COLORREF color = m_wndColorBar.GetColor ();
	if (color == RGB (192, 192, 192))
	{
		color = globalData.clrBtnFace;
	}

	m_wndLargeDrawArea.SetColor (color);
}
//********************************************************************************
BOOL CBCGImageEditDlg::OnPickColor (COLORREF color)
{
	m_wndColorBar.SetColor (color);
	m_wndLargeDrawArea.SetColor (color);

	//-----------------------------------------
	// Move to the pen mode (not so good :-(!):
	//-----------------------------------------
	m_wndLargeDrawArea.SetMode (CImagePaintArea::IMAGE_EDIT_MODE_PEN);
	return TRUE;
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolClear() 
{
	CWindowDC	dc (this);
	CDC 		memDC;	

	memDC.CreateCompatibleDC (&dc);
	
	CBitmap* pOldBitmap = memDC.SelectObject (m_pBitmap);

	CRect rect (0, 0, m_sizeImage.cx, m_sizeImage.cy);
	memDC.FillRect (&rect, &globalData.brBtnFace);

	memDC.SelectObject (pOldBitmap);

	InvalidateRect (m_rectPreviewImage);
	m_wndLargeDrawArea.Invalidate ();
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolCopy() 
{
	CBCGLocalResource locaRes;

	if (m_pBitmap == NULL)
	{
		return;
	}

	try
	{
		CWindowDC dc (this);

		//----------------------
		// Create a bitmap copy:
		//----------------------
		CDC memDCDest;
		memDCDest.CreateCompatibleDC (NULL);
		
		CDC memDCSrc;
		memDCSrc.CreateCompatibleDC (NULL);
		
		CBitmap bitmapCopy;
		if (!bitmapCopy.CreateCompatibleBitmap (&dc, m_sizeImage.cx, m_sizeImage.cy))
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			return;
		}

		CBitmap* pOldBitmapDest = memDCDest.SelectObject (&bitmapCopy);
		CBitmap* pOldBitmapSrc = memDCSrc.SelectObject (m_pBitmap);

		memDCDest.BitBlt (0, 0, m_sizeImage.cx, m_sizeImage.cy,
						&memDCSrc, 0, 0, SRCCOPY);

		memDCDest.SelectObject (pOldBitmapDest);
		memDCSrc.SelectObject (pOldBitmapSrc);

		if (!OpenClipboard ())
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			return;
		}

		if (!::EmptyClipboard ())
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			::CloseClipboard ();
			return;
		}


		HANDLE hclipData = ::SetClipboardData (CF_BITMAP, bitmapCopy.Detach ());
		if (hclipData == NULL)
		{
			AfxMessageBox (IDP_BCGBARRES_CANT_COPY_BITMAP);
			TRACE (_T("CBCGImageEditDlg::Copy() error. Error code = %x\n"), GetLastError ());
		}

		::CloseClipboard ();
	}
	catch (...)
	{
		CBCGLocalResource locaRes;
		AfxMessageBox (IDP_BCGBARRES_INTERLAL_ERROR);
	}
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolPaste() 
{
	CBCGLocalResource locaRes;

	COleDataObject data;
	if (!data.AttachClipboard ())
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	if (!data.IsDataAvailable (CF_BITMAP))
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	tagSTGMEDIUM dataMedium;
	if (!data.GetData (CF_BITMAP, &dataMedium))
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	CBitmap* pBmpClip = CBitmap::FromHandle (dataMedium.hBitmap);
	if (pBmpClip == NULL)
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	BITMAP bmp;
	pBmpClip->GetBitmap (&bmp);

	CDC memDCDst;
	CDC memDCSrc;

	memDCSrc.CreateCompatibleDC (NULL);
	memDCDst.CreateCompatibleDC (NULL);
	
	CBitmap* pSrcOldBitmap = memDCSrc.SelectObject (pBmpClip);
	if (pSrcOldBitmap == NULL)
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		return;
	}

	CBitmap* pDstOldBitmap = memDCDst.SelectObject (m_pBitmap);
	if (pDstOldBitmap == NULL)
	{
		AfxMessageBox (IDP_BCGBARRES_CANT_PASTE_BITMAP);
		
		memDCSrc.SelectObject (pSrcOldBitmap);
		return;
	}

	memDCDst.FillRect (CRect (0, 0, m_sizeImage.cx, m_sizeImage.cy), 
						&globalData.brBtnFace);

	int x = max (0, (m_sizeImage.cx - bmp.bmWidth) / 2);
	int y = max (0, (m_sizeImage.cy - bmp.bmHeight) / 2);

	CBCGToolBarImages::TransparentBlt (memDCDst.GetSafeHdc (),
		x, y, m_sizeImage.cx, m_sizeImage.cy,
		&memDCSrc, 0, 0, RGB (192, 192, 192));

	memDCDst.SelectObject (pDstOldBitmap);
	memDCSrc.SelectObject (pSrcOldBitmap);

	InvalidateRect (m_rectPreviewImage);
	m_wndLargeDrawArea.Invalidate ();
}
//********************************************************************************
void CBCGImageEditDlg::OnUpdateBcgToolPaste(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(::IsClipboardFormatAvailable (CF_BITMAP));
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolEllipse() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_ELLIPSE);
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolFill() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_FILL);
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolLine() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_LINE);
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolPen() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_PEN);
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolPick() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_COLOR);
}
//********************************************************************************
void CBCGImageEditDlg::OnBcgToolRect() 
{
	SetMode (CImagePaintArea::IMAGE_EDIT_MODE_RECT);
}
//********************************************************************************
void CBCGImageEditDlg::OnUpdateBcgToolEllipse(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_ELLIPSE);
}
//********************************************************************************
void CBCGImageEditDlg::OnUpdateBcgToolFill(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_FILL);
}
//********************************************************************************
void CBCGImageEditDlg::OnUpdateBcgToolLine(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_LINE);
}
//********************************************************************************
void CBCGImageEditDlg::OnUpdateBcgToolPen(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_PEN);
}
//********************************************************************************
void CBCGImageEditDlg::OnUpdateBcgToolPick(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_COLOR);
}
//********************************************************************************
void CBCGImageEditDlg::OnUpdateBcgToolRect(CCmdUI* pCmdUI) 
{
	pCmdUI->SetCheck (GetMode () == CImagePaintArea::IMAGE_EDIT_MODE_RECT);
}
//********************************************************************************
INT_PTR CBCGImageEditDlg::DoModal() 
{
	CBCGLocalResource locaRes;
	return CDialog::DoModal();
}
//********************************************************************************
void Create16ColorsStdPalette (CPalette& pal)
{
	const int nStdColorCount = 20;
	CPalette* pPalDefault = CPalette::FromHandle ((HPALETTE) ::GetStockObject (DEFAULT_PALETTE));
	if (pPalDefault == NULL)
	{
		ASSERT (FALSE);
		return;
	}

	const int nColors = 16;
	UINT nSize = sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * nColors);
	LOGPALETTE *pLP = (LOGPALETTE *) new BYTE[nSize];

	pLP->palVersion = 0x300;
	pLP->palNumEntries = (USHORT) nColors;

	pal.CreatePalette (pLP);

	delete[] pLP;

	PALETTEENTRY palEntry;
	int iDest = 0;

	for (int i = 0; i < nStdColorCount; i++)
	{
		if (i < 8 || i >= 12)
		{
			pPalDefault->GetPaletteEntries (i, 1, &palEntry);
			pal.SetPaletteEntries (iDest++, 1, &palEntry);
		}
	}
}
