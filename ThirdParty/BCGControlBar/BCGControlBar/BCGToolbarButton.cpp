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

// BCGToolbarButton.cpp: implementation of the CBCGToolbarButton class.
//
//////////////////////////////////////////////////////////////////////

#include "Stdafx.h"
#include "menuhash.h"
#include "BCGToolbar.h"
#include "BCGToolbarButton.h"
#include "BCGToolBarImages.h"
#include "BCGCommandManager.h"
#include "globals.h"
#include "BCGFrameWnd.h"
#include "BCGMDIFrameWnd.h"
#include "BCGOleIPFrameWnd.h"
#include "BCGOleDocIPFrameWnd.h"
#include "BCGUserToolsManager.h"
#include "BCGUserTool.h"
#include "BCGToolbarMenuButton.h"
#include "BCGVisualManager.h"
#include "BCGWorkspace.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CBCGWorkspace* g_pWorkspace;

IMPLEMENT_SERIAL(CBCGToolbarButton, CObject, VERSIONABLE_SCHEMA | 1)

CLIPFORMAT CBCGToolbarButton::m_cFormat = 0;
CString	 CBCGToolbarButton::m_strClipboardFormatName;
BOOL CBCGToolbarButton::m_bWrapText = TRUE;

static const int nTextMargin = 3;
static const int nSeparatorWidth = 8;
static const CString strDummyAmpSeq = _T("\001\001");

CList<UINT, UINT> CBCGToolbarButton::m_lstProtectedCommands;
BOOL CBCGToolbarButton::m_bUpdateImages = TRUE;

#define IMAGE_MARGIN	4

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBCGToolbarButton::CBCGToolbarButton()
{
	Initialize ();
}
//*********************************************************************************
CBCGToolbarButton::CBCGToolbarButton(UINT uiID, int iImage, LPCTSTR lpszText, BOOL bUserButton,
									 BOOL bLocked)
{
	Initialize ();

	m_bLocked = bLocked;

	m_nID = uiID;
	m_bUserButton = bUserButton;
	SetImage (iImage);

	m_strText = (lpszText == NULL) ? _T("") : lpszText;

	if (m_nID != 0 && !m_bLocked)
	{
		if (m_bUserButton)
		{
			if (m_iUserImage != -1)
			{
				CMD_MGR.SetCmdImage (m_nID, m_iUserImage, TRUE);
			}
			else
			{
				m_iUserImage = CMD_MGR.GetCmdImage (m_nID, TRUE);
			}
		}
		else
		{
			if (m_iImage != -1)
			{
				CMD_MGR.SetCmdImage (m_nID, m_iImage, FALSE);
			}
			else
			{
				m_iImage = CMD_MGR.GetCmdImage (m_nID, FALSE);
			}
		}
	}
}
//*********************************************************************************
void CBCGToolbarButton::Initialize ()
{
	m_nID = 0;
	m_nStyle = TBBS_BUTTON;
	m_iImage = -1;
	m_iUserImage = -1;
	m_bUserButton = FALSE;
	m_bDragFromCollection = FALSE;
	m_bText = FALSE;
	m_bImage = TRUE;
	m_bWrap = FALSE;
	m_bWholeText = TRUE;
	m_bLocked = FALSE;
	m_bIsHidden = FALSE;
	m_bTextBelow = FALSE;

	m_rect.SetRectEmpty ();
	m_sizeText = CSize (0, 0);
	m_bDisableFill = FALSE;
	m_bExtraSize = FALSE;
	m_bHorz = TRUE;
	m_bVisible = TRUE;
}
//*********************************************************************************
CBCGToolbarButton::~CBCGToolbarButton()
{
}
//*********************************************************************************
void CBCGToolbarButton::CopyFrom (const CBCGToolbarButton& src)
{
	m_nID			= src.m_nID;
	m_bLocked		= src.m_bLocked;
	m_bUserButton	= src.m_bUserButton;
	m_nStyle		= src.m_nStyle;
	SetImage (src.m_bUserButton ? src.m_iUserImage : src.m_iImage);
	m_strText		= src.m_strText;
	m_bText			= src.m_bText;
	m_bImage		= src.m_bImage;
	m_bWrap			= src.m_bWrap;
	m_strTextCustom	= src.m_strTextCustom;
	m_bVisible		= src.m_bVisible;
					
	m_bDragFromCollection = FALSE;
}					
//***************************************************************************************
void CBCGToolbarButton::Serialize (CArchive& ar)
{
	CObject::Serialize (ar);

	if (ar.IsLoading ())
	{
		int iImage;

		ar >> m_nID;
		ar >> m_nStyle;	
		ar >> iImage;	
		ar >> m_strText;	
		ar >> m_bUserButton;
		ar >> m_bDragFromCollection;
		ar >> m_bText;
		ar >> m_bImage;

		if (g_menuHash.IsActive () ||
			(g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60400))
		{
			ar >> m_bVisible;
		}

		SetImage (iImage);
	}
	else
	{
		ar << m_nID;		
		ar << m_nStyle;	
		ar << GetImage ();
		ar << m_strText;
		ar << m_bUserButton;
		ar << m_bDragFromCollection;
		ar << m_bText;
		ar << m_bImage;

		if (g_menuHash.IsActive () ||
			(g_pWorkspace != NULL && g_pWorkspace->GetDataVersion () >= 0x60400))
		{
			ar << m_bVisible;
		}
	}
}
//***************************************************************************************
CLIPFORMAT CBCGToolbarButton::GetClipboardFormat ()
{
	if (m_cFormat == 0)	// Not registered yet
	{
		CString strFormat = m_strClipboardFormatName;

		if (strFormat.IsEmpty ())
		{
			strFormat.Format (_T("BCGToolbarButton%lx"), AfxGetInstanceHandle ());
						// Format should be unique per application
		}

		m_cFormat = (CLIPFORMAT)::RegisterClipboardFormat (strFormat);
		ASSERT (m_cFormat != NULL);
	}

	return m_cFormat;
}
//***************************************************************************************
CBCGToolbarButton* CBCGToolbarButton::CreateFromOleData  (COleDataObject* pDataObject)
{
	ASSERT (pDataObject != NULL);
	ASSERT (pDataObject->IsDataAvailable (CBCGToolbarButton::m_cFormat));

	CBCGToolbarButton* pButton = NULL;

	try
	{
		//-------------------------------------
		// Get file refering to clipboard data:
		//-------------------------------------
		CFile* pFile = pDataObject->GetFileData (GetClipboardFormat ());
		if (pFile == NULL)
		{
			return FALSE;
		}

		//-------------------------------------------------------
		// Connect the file to the archive and read the contents:
		//-------------------------------------------------------
		CArchive ar (pFile, CArchive::load);

		//----------------------------------------
		// First, read run-time class information:
		//----------------------------------------
		CRuntimeClass* pClass = ar.ReadClass ();
		ASSERT (pClass != NULL);

		if (pClass != NULL)
		{
			pButton = (CBCGToolbarButton*) pClass->CreateObject ();
			ASSERT (pButton != NULL);

			if (pButton != NULL)
			{
				pButton->Serialize (ar);
			}
		}

		ar.Close ();
		delete pFile;

		return pButton;
	}
	catch (COleException* pEx)
	{
		TRACE(_T("CBCGToolbarButton::CreateFromOleData. OLE exception: %x\r\n"),
			pEx->m_sc);
		pEx->Delete ();
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGToolbarButton::CreateFromOleData. Archive exception\r\n"));
		pEx->Delete ();
	}
	catch (CNotSupportedException *pEx)
	{
		TRACE(_T("CBCGToolbarButton::CreateFromOleData. \"Not Supported\" exception\r\n"));
		pEx->Delete ();
	}

	if (pButton != NULL)
	{
		delete pButton;
	}

	return NULL;
}
//***************************************************************************************
void CBCGToolbarButton::OnDraw (CDC* pDC, const CRect& rect, CBCGToolBarImages* pImages,
								BOOL bHorz, BOOL bCustomizeMode, BOOL bHighlight,
								BOOL bDrawBorder, BOOL bGrayDisabledButtons)
{
	ASSERT_VALID (pDC);
	ASSERT_VALID (this);

	m_bHorz = bHorz;

	//----------------------
	// Fill button interior:
	//----------------------
	FillInterior (pDC, rect, bHighlight);

	BOOL bHot = bHighlight;
	CSize sizeImage = (pImages == NULL) ? CSize (0, 0) : pImages->GetImageSize (TRUE);

	CBCGUserTool* pUserTool = NULL;
	if (g_pUserToolsManager != NULL && !m_bUserButton)
	{
		pUserTool = g_pUserToolsManager->FindTool (m_nID);
	}

	CRect rectInternal = rect;
	CSize sizeExtra = m_bExtraSize ?
		CBCGVisualManager::GetInstance ()->GetButtonExtraBorder () : CSize (0, 0);
	rectInternal.DeflateRect (sizeExtra.cx / 2, sizeExtra.cy / 2);

	int x = rectInternal.left;
	int y = rectInternal.top;

	int iTextLen = 0;

	CString strWithoutAmp = m_strText;
	strWithoutAmp.Replace (_T("&&"), strDummyAmpSeq);
	strWithoutAmp.Remove (_T('&'));
	strWithoutAmp.Replace (strDummyAmpSeq, _T("&"));

	CSize sizeText = pDC->GetTextExtent (strWithoutAmp);

	if (IsDrawText () && !(m_bTextBelow && bHorz))
	{
		int nMargin = IsDrawImage () ? 0 : nTextMargin;
		iTextLen = sizeText.cx + nMargin;
	}

	int dx = 0;
	int dy = 0;

	if (m_bTextBelow && bHorz)
	{
		ASSERT (bHorz);

		dx = rectInternal.Width ();
		dy = sizeImage.cy + 2 * nTextMargin;
	}
	else
	{
		dx = bHorz ? rectInternal.Width () - iTextLen : rectInternal.Width ();
		dy = bHorz ? rectInternal.Height () : rectInternal.Height () - iTextLen;
	}

	// determine offset of bitmap (centered within button)
	CPoint ptImageOffset;
	ptImageOffset.x = (dx - sizeImage.cx) / 2;
	ptImageOffset.y = (dy - sizeImage.cy) / 2;

	CPoint ptTextOffset (nTextMargin, nTextMargin);

	if (IsDrawText () && !(m_bTextBelow && bHorz))
	{
		TEXTMETRIC tm;
		pDC->GetTextMetrics (&tm);

		if (bHorz)
		{
			ptImageOffset.x -= nTextMargin;
			ptTextOffset.y = (dy - tm.tmHeight - 1) / 2;
		}
		else
		{
			ptImageOffset.y -= nTextMargin;
			ptTextOffset.x = (dx - tm.tmHeight + 1) / 2;
		}
	}

	CPoint ptImageOffsetInButton (0, 0);
	BOOL bPressed = FALSE;

	BOOL bDrawImageShadow = 
		bHighlight && !bCustomizeMode &&
		!IsDroppedDown () &&
		CBCGVisualManager::GetInstance ()->IsShadowHighlightedImage () &&
		!globalData.IsHighContastMode () &&
		((m_nStyle & TBBS_PRESSED) == 0) &&
		((m_nStyle & TBBS_CHECKED) == 0) &&
		((m_nStyle & TBBS_DISABLED) == 0);

	if ((m_nStyle & (TBBS_PRESSED | TBBS_CHECKED)) && !bCustomizeMode &&
		!CBCGVisualManager::GetInstance ()->IsShadowHighlightedImage () &&
		CBCGVisualManager::GetInstance ()->IsOffsetPressedButton ())
	{
		// pressed in or checked
		ptImageOffset.Offset (1, 1);
		bPressed = TRUE;

		ptTextOffset.y ++;

		if (bHorz)
		{
			ptTextOffset.x ++;
		}
		else
		{
			ptTextOffset.x --;
		}
	}

	BOOL bFadeImage = !bHighlight && CBCGVisualManager::GetInstance ()->IsFadeInactiveImage ();
	BOOL bImageIsReady = FALSE;

	if ((m_nStyle & TBBS_PRESSED) || !(m_nStyle & TBBS_DISABLED) ||
		bCustomizeMode)
	{
		if (IsDrawImage () && pImages != NULL)
		{
			if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon (pDC,
					CRect (CPoint (x + ptImageOffset.x, y + ptImageOffset.y),
					sizeImage));
			}
			else
			{
				CPoint pt = ptImageOffset;

				if (bDrawImageShadow)
				{
					const int nRatio = CBCGToolBar::IsLargeIcons () ? 2 : 1;

					pt.Offset (nRatio, nRatio);

					pImages->Draw (pDC, x + pt.x, 
										y + pt.y, GetImage (),
										FALSE, FALSE, FALSE, TRUE);
					pt.Offset (-2 * nRatio, -2 * nRatio);
				}

				pImages->Draw (pDC, x + pt.x, y + pt.y, GetImage (),
					FALSE, FALSE, FALSE, FALSE, bFadeImage);
			}
		}

		bImageIsReady = TRUE;
	}

	BOOL bDisabled = (bCustomizeMode && !IsEditable ()) ||
		(!bCustomizeMode && (m_nStyle & TBBS_DISABLED));

	if (!bImageIsReady)
	{
		if (IsDrawImage () && pImages != NULL)
		{
			if (pUserTool != NULL)
			{
				pUserTool->DrawToolIcon (pDC,
					CRect (CPoint (x + ptImageOffset.x, y + ptImageOffset.y),
					sizeImage));
			}
			else
			{
				if (bDrawImageShadow)
				{
					const int nRatio = CBCGToolBar::IsLargeIcons () ? 2 : 1;

					ptImageOffset.Offset (nRatio, nRatio);

					pImages->Draw (pDC, x + ptImageOffset.x, 
										y + ptImageOffset.y, GetImage (),
										FALSE, FALSE, FALSE, TRUE);
					ptImageOffset.Offset (-2 * nRatio, -2 * nRatio);
				}

				pImages->Draw (pDC, x + ptImageOffset.x, y + ptImageOffset.y, GetImage (),
								FALSE, bDisabled && bGrayDisabledButtons,
								FALSE, FALSE, bFadeImage);
			}
		}
	}

	if ((m_bTextBelow && bHorz) || IsDrawText ())
	{
		//--------------------
		// Draw button's text:
		//--------------------
		CBCGVisualManager::BCGBUTTON_STATE state = CBCGVisualManager::ButtonsIsRegular;

		if (bHighlight)
		{
			state = CBCGVisualManager::ButtonsIsHighlighted;
		}
		else if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			state = CBCGVisualManager::ButtonsIsPressed;
		}

		COLORREF clrText = CBCGVisualManager::GetInstance ()->GetToolbarButtonTextColor (
			this, state);

		pDC->SetTextColor (clrText);
		CString strText = m_strText;
		CRect rectText = rectInternal;
		UINT uiTextFormat = 0;

		if (m_bTextBelow && bHorz)
		{
			ASSERT (bHorz);

			ptTextOffset.y += sizeImage.cy + nTextMargin;
			uiTextFormat = DT_CENTER;

			if (m_bWrapText)
			{
				uiTextFormat |= DT_WORDBREAK;
			}

			rectText.left = (rectInternal.left + rectInternal.right - m_sizeText.cx) / 2 + ptTextOffset.x;
			rectText.right = (rectInternal.left + rectInternal.right + m_sizeText.cx) / 2;
		}
		else
		{
			if (IsDrawImage ())
			{
				const int nExtra = CBCGToolBar::IsLargeIcons () ? 2 * nTextMargin : 0;

				if (bHorz)
				{
					ptTextOffset.x += sizeImage.cx + nExtra;
				}
				else
				{
					ptTextOffset.y += sizeImage.cy + nExtra;
				}
			
				rectText.left = x + ptTextOffset.x + nTextMargin;
			}
			else
			{
				rectText.left = x + nTextMargin + 1;
			}

			uiTextFormat = DT_SINGLELINE;
		}

		if (bHorz)
		{
			rectText.top += ptTextOffset.y;

			if (m_bTextBelow && m_bExtraSize)
			{
				rectText.OffsetRect (0,
					CBCGVisualManager::GetInstance ()->GetButtonExtraBorder ().cy / 2);
			}

			pDC->DrawText (strText, &rectText, uiTextFormat);
		}
		else
		{
			rectText = rectInternal;
			rectText.top += ptTextOffset.y;

			rectText.left = rectText.CenterPoint ().x - sizeText.cy / 2;
			rectText.right = rectText.left + sizeText.cy;
			rectText.top += max (0, (rectText.Height () - sizeText.cx) / 2);

			rectText.SwapLeftRight ();

			uiTextFormat = DT_NOCLIP | DT_SINGLELINE;

			strText.Replace (_T("&&"), strDummyAmpSeq);
			int iAmpIndex = strText.Find (_T('&'));	// Find a SINGLE '&'
			strText.Remove (_T('&'));
			strText.Replace (strDummyAmpSeq, _T("&&"));

			if (iAmpIndex >= 0)
			{
				//-----------------------------------------
				// Calculate underlined character position:
				//-----------------------------------------
				CRect rectSubText;
				rectSubText.SetRectEmpty ();
				CString strSubText = strText.Left (iAmpIndex + 1);

				pDC->DrawText (strSubText, &rectSubText, uiTextFormat | DT_CALCRECT);
				int y1 = rectSubText.right;

				rectSubText.SetRectEmpty ();
				strSubText = strText.Left (iAmpIndex);

				pDC->DrawText (strSubText, &rectSubText, uiTextFormat | DT_CALCRECT);
				int y2 = rectSubText.right;

				pDC->DrawText (strText, &rectText, uiTextFormat);

				int x = rect.CenterPoint ().x - sizeText.cy / 2;

				CPen* pOldPen = NULL;
				CPen pen (PS_SOLID, 1, pDC->GetTextColor ());

				if (pDC->GetTextColor () != 0)
				{
					pOldPen = pDC->SelectObject (&pen);
				}

				pDC->MoveTo (x, rectText.top + y1);
				pDC->LineTo (x, rectText.top + y2);

				if (pOldPen != NULL)
				{
					pDC->SelectObject (pOldPen);
				}
			}
			else
			{
				pDC->DrawText (strText, &rectText, uiTextFormat);
			}
		}
	}

	//--------------------
	// Draw button border:
	//--------------------
	if (!bCustomizeMode && HaveHotBorder () && bDrawBorder)
	{
		if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rect, CBCGVisualManager::ButtonsIsPressed);
		}
		else if (bHot && !(m_nStyle & TBBS_DISABLED) &&
			!(m_nStyle & (TBBS_CHECKED | TBBS_INDETERMINATE)))
		{
			CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
				this, rect, CBCGVisualManager::ButtonsIsHighlighted);
		}
	}
}
//***********************************************************************************
SIZE CBCGToolbarButton::OnCalculateSize (
								CDC* pDC,
								const CSize& sizeDefault,
								BOOL bHorz)
{
	ASSERT_VALID (pDC);

	if(!IsVisible())
		return CSize(0,0);

	CSize size = sizeDefault;

	if (m_nStyle & TBBS_SEPARATOR)
	{
		if (bHorz)
		{
			size.cx = m_iImage > 0 ? m_iImage : nSeparatorWidth;
		}
		else
		{
			size.cy = nSeparatorWidth;
		}
	}
	else
	{
		BOOL bHasImage = TRUE;

		if (!IsDrawImage () || GetImage () < 0)
		{
			bHasImage = FALSE;

			CSize sizeExtra = m_bExtraSize ? 
				CBCGVisualManager::GetInstance ()->GetButtonExtraBorder () : CSize (0, 0);

			if (bHorz)
			{
				size.cx = sizeExtra.cx;
			}
			else
			{
				size.cy = sizeExtra.cy;
			}
		}

		m_sizeText = CSize (0, 0);

		if (!m_strText.IsEmpty ())
		{
			if (m_bTextBelow && bHorz)
			{
				//----------------------------------------------------------
				// Try format text that it ocuppies no more tow lines an its
				// width less than 3 images:
				//----------------------------------------------------------
				CRect rectText (0, 0, 
					sizeDefault.cx * 3, sizeDefault.cy);

				UINT uiTextFormat = DT_CENTER | DT_CALCRECT;
				if (m_bWrapText)
				{
					uiTextFormat |= DT_WORDBREAK;
				}

				pDC->DrawText (	m_strText, rectText, uiTextFormat);
				m_sizeText = rectText.Size ();
				m_sizeText.cx += 2 * nTextMargin;

				size.cx = max (size.cx, m_sizeText.cx) + 4 * nTextMargin;
				size.cy += m_sizeText.cy + CY_BORDER; 
			}
			else if (IsDrawText ())
			{
				CString strWithoutAmp = m_strText;
				strWithoutAmp.Replace (_T("&&"), strDummyAmpSeq);
				strWithoutAmp.Remove (_T('&'));
				strWithoutAmp.Replace (strDummyAmpSeq, _T("&"));

				int nTextExtra = bHasImage ? 2 * nTextMargin : 3 * nTextMargin;
				int iTextLen = pDC->GetTextExtent (strWithoutAmp).cx + nTextExtra;

				if (bHorz)
				{
					size.cx += iTextLen;
				}
				else
				{
					size.cy += iTextLen;
				}
			}
		}
	}

	return size;
}
//************************************************************************************
BOOL CBCGToolbarButton::PrepareDrag (COleDataSource& srcItem)
{
	if (!CanBeStored ())
	{
		return TRUE;
	}

	try
	{
		CSharedFile globFile;
		CArchive ar (&globFile,CArchive::store);

		//---------------------------------
		// Save run-time class information:
		//---------------------------------
		CRuntimeClass* pClass = GetRuntimeClass ();
		ASSERT (pClass != NULL);

		ar.WriteClass (pClass);

		//---------------------
		// Save button context:
		//---------------------
		Serialize (ar);
		ar.Close();

		srcItem.CacheGlobalData (GetClipboardFormat (), globFile.Detach());
	}
	catch (COleException* pEx)
	{
		TRACE(_T("CBCGToolbarButton::PrepareDrag. OLE exception: %x\r\n"),
			pEx->m_sc);
		pEx->Delete ();
		return FALSE;
	}
	catch (CArchiveException* pEx)
	{
		TRACE(_T("CBCGToolbarButton::PrepareDrag. Archive exception\r\n"));
		pEx->Delete ();
		return FALSE;
	}

	return TRUE;
}
//****************************************************************************************
void CBCGToolbarButton::SetImage (int iImage)
{
	if (m_nStyle & TBBS_SEPARATOR)
	{
		m_iImage = iImage;	// Actualy, separator width!
		return;
	}

	if (m_bUserButton)
	{
		m_iUserImage = iImage;
	}
	else
	{
		m_iImage = iImage;
	}

	if (!m_bLocked)
	{
		if (m_nID != 0 && iImage != -1)
		{
			if (m_bUpdateImages || m_bUserButton)
			{
				CMD_MGR.SetCmdImage (m_nID, iImage, m_bUserButton);
			}
		}
		else if (m_nID != 0)
		{
			m_iImage = CMD_MGR.GetCmdImage (m_nID, FALSE);
			m_iUserImage = CMD_MGR.GetCmdImage (m_nID, TRUE);

			if (m_iImage == -1 && !m_bUserButton)
			{
				m_bUserButton = TRUE;
			}
			else if (m_iImage == -1 && m_bUserButton)
			{
				m_bUserButton = FALSE;
			}
		}
	}

    if ((!m_bUserButton && m_iImage < 0) ||
        (m_bUserButton && m_iUserImage < 0))
    {
        m_bImage = FALSE;
        m_bText  = TRUE;
    }
}

/////////////////////////////////////////////////////////////////////////////
// CBCGToolbarButton diagnostics

#ifdef _DEBUG
void CBCGToolbarButton::AssertValid() const
{
	CObject::AssertValid();
}
//******************************************************************************************
void CBCGToolbarButton::Dump(CDumpContext& dc) const
{
	CObject::Dump (dc);

	CString strId;
	strId.Format (_T("%x"), m_nID);

	dc << "[" << strId << " " << m_strText << "]";
	dc << "\n";
}

#endif

int CBCGToolbarButton::OnDrawOnCustomizeList (
	CDC* pDC, const CRect& rect, BOOL bSelected)
{
	ASSERT_VALID (this);
	ASSERT_VALID (pDC);

	BOOL bText = m_bText;
	m_bText = FALSE;

	int iWidth = 0;

	CBCGToolBarImages* pImages = CBCGToolBar::GetImages ();
	if (m_bUserButton)
	{
		pImages = CBCGToolBar::GetUserImages ();
	}
	else
	{
		CBCGToolBarImages* pMenuImages = CBCGToolBar::GetMenuImages ();
		if (pMenuImages != NULL && pMenuImages->GetCount () == pImages->GetCount ())
		{
			pImages = pMenuImages;
		}
	}
	
	CBCGUserTool* pUserTool = NULL;
	if (g_pUserToolsManager != NULL && !m_bUserButton)
	{
		pUserTool = g_pUserToolsManager->FindTool (m_nID);
	}

	CSize sizeMenuImage = CBCGToolBar::GetMenuImageSize ();

	int nMargin = 3;
	CSize sizeButton = CSize (	sizeMenuImage.cx + 2 * nMargin,
								sizeMenuImage.cy + 2 * nMargin);

	CRect rectFill = rect;

	if (bSelected && 
		!CBCGVisualManager::GetInstance ()->IsHighlightWholeMenuItem () &&
		GetImage () >= 0 && pImages != NULL)
	{
		rectFill.left += sizeButton.cx;

		CRect rectLeftBtn = rect;
		rectLeftBtn.right = rectFill.left;

		CBCGVisualManager::GetInstance ()->OnFillButtonInterior (
			pDC, this, rectLeftBtn, CBCGVisualManager::ButtonsIsHighlighted);

		CBCGVisualManager::GetInstance ()->OnDrawButtonBorder (pDC,
			this, rectLeftBtn, CBCGVisualManager::ButtonsIsHighlighted);
	}

	COLORREF clrText = CBCGVisualManager::GetInstance ()->OnFillCommandsListBackground 
			(pDC, rectFill, bSelected);

	CRect rectText = rect;
	rectText.left += sizeMenuImage.cx + 2 * IMAGE_MARGIN + 2;

	iWidth = sizeButton.cx;

	//-------------------
	// Draw button image:
	//-------------------
	if (GetImage () >= 0 && pImages != NULL)
	{
		if (pUserTool != NULL)
		{
			CRect rectImage = rect;
			rectImage.right = rectImage.left + sizeButton.cx;

			pUserTool->DrawToolIcon (pDC, rectImage);
		}
		else
		{
			BOOL bFadeImage = !bSelected && CBCGVisualManager::GetInstance ()->IsFadeInactiveImage ();
			BOOL bDrawImageShadow = 
				bSelected && 
				CBCGVisualManager::GetInstance ()->IsShadowHighlightedImage () &&
				!globalData.IsHighContastMode ();

			CBCGDrawState ds;
			pImages->PrepareDrawImage (ds,
				CSize (0, 0), 
				bFadeImage);

			CPoint pt = rect.TopLeft ();
			pt.x += nMargin;
			pt.y += nMargin;

			if (bDrawImageShadow)
			{
				pt.Offset (1, 1);

				pImages->Draw (pDC, pt.x, 
									pt.y, GetImage (),
									FALSE, FALSE, FALSE, TRUE);
				pt.Offset (-2, -2);
			}

			pImages->Draw (pDC, pt.x, pt.y, GetImage (),
				FALSE, FALSE, FALSE, FALSE, bFadeImage);

			pImages->EndDrawImage (ds);
		}
	}

	//-------------------
	// Draw button text:
	//-------------------
	if (!m_strText.IsEmpty ())
	{
		COLORREF clrTextOld = pDC->SetTextColor (clrText);

		pDC->SetBkMode (TRANSPARENT);
		pDC->DrawText (m_strText, rectText, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
		pDC->SetTextColor (clrTextOld);

		int iTextWidth = min (rectText.Width (), pDC->GetTextExtent (m_strText).cx);
		iWidth += iTextWidth;
	}

	m_bText = bText;
	return iWidth;
}
//*************************************************************************************
BOOL CBCGToolbarButton::OnToolHitTest(const CWnd* pWnd, TOOLINFO* pTI)
{
	CFrameWnd* pTopFrame = (pWnd == NULL) ? 
		(CFrameWnd*) AfxGetMainWnd () : 
		BCGGetTopLevelFrame (pWnd);

	CBCGMDIFrameWnd* pMainFrame = DYNAMIC_DOWNCAST (CBCGMDIFrameWnd, pTopFrame);
	if (pMainFrame != NULL)
	{
		return pMainFrame->OnMenuButtonToolHitTest (this, pTI);
	}
	else	// Maybe, SDI frame...
	{
		CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pTopFrame);
		if (pFrame != NULL)
		{
			return pFrame->OnMenuButtonToolHitTest (this, pTI);
		}
		else	// Maybe, OLE frame...
		{
			CBCGOleIPFrameWnd* pOleFrame = 
				DYNAMIC_DOWNCAST (CBCGOleIPFrameWnd, pFrame);
			if (pOleFrame != NULL)
			{
				return pOleFrame->OnMenuButtonToolHitTest (this, pTI);
			}
		}
	}

	CBCGFrameWnd* pFrame = DYNAMIC_DOWNCAST (CBCGFrameWnd, pTopFrame);
	if (pFrame != NULL)
	{
		return pFrame->OnMenuButtonToolHitTest (this, pTI);
	}

	return FALSE;
}
//*************************************************************************************
BOOL CBCGToolbarButton::ExportToMenuButton (CBCGToolbarMenuButton& menuButton) const
{
	//-----------------------------------------------------
	// Text may be undefined, bring it from the tooltip :-(
	//-----------------------------------------------------
	if (m_strText.IsEmpty () && m_nID != 0)
	{
		CString strMessage;
		int iOffset;

		if (strMessage.LoadString (m_nID) &&
			(iOffset = strMessage.Find (_T('\n'))) != -1)
		{
			menuButton.m_strText = strMessage.Mid (iOffset + 1);
		}
	}

	return TRUE;
}
//*******************************************************************************
void CBCGToolbarButton::SetProtectedCommands (const CList<UINT, UINT>& lstCmds)
{
	m_lstProtectedCommands.RemoveAll ();
	m_lstProtectedCommands.AddTail ((CList<UINT,UINT>*) &lstCmds);
}
//********************************************************************************
void CBCGToolbarButton::SetClipboardFormatName (LPCTSTR lpszName)
{
	ASSERT (lpszName != NULL);
	ASSERT (m_cFormat == 0);

	m_strClipboardFormatName = lpszName;
}
//********************************************************************************
void CBCGToolbarButton::FillInterior (CDC* pDC, const CRect& rect,
									  BOOL bHighlight)
{
	if (m_bDisableFill)
	{
		return;
	}

	CBCGVisualManager::BCGBUTTON_STATE state = CBCGVisualManager::ButtonsIsRegular;

	if (!CBCGToolBar::IsCustomizeMode () ||
		CBCGToolBar::IsAltCustomizeMode () || m_bLocked)
	{
		if (bHighlight)
		{
			state = CBCGVisualManager::ButtonsIsHighlighted;
		}
		else if (m_nStyle & (TBBS_PRESSED | TBBS_CHECKED))
		{
			//-----------------------
			// Pressed in or checked:
			//-----------------------
			state = CBCGVisualManager::ButtonsIsPressed;
		}
	}

	CBCGVisualManager::GetInstance ()->OnFillButtonInterior (pDC, this, rect, state);
}
//************************************************************************************
void CBCGToolbarButton::ResetImageToDefault ()
{
	if (m_bUserButton || (int) m_nID <= 0)
	{
		return;
	}

	if (g_pUserToolsManager != NULL &&
		g_pUserToolsManager->FindTool (m_nID) != NULL)
	{
		// User tool has its own image
		return;
	}

	BOOL bWasImage = m_bImage;

	int iImage = CBCGToolBar::GetDefaultImage (m_nID);
	if (iImage >= 0)
	{
		SetImage (iImage);
	}
	else if (bWasImage)
	{
		m_bImage = FALSE;
		m_bText = TRUE;

		if (m_strText.IsEmpty ())
		{
			CString strMessage;
			int iOffset;

			if (strMessage.LoadString (m_nID) &&
				(iOffset = strMessage.Find (_T('\n'))) != -1)
			{
				m_strText = strMessage.Mid (iOffset + 1);
			}
		}
	}
}
//********************************************************************************
BOOL CBCGToolbarButton::CompareWith (const CBCGToolbarButton& other) const
{
	return m_nID == other.m_nID;
}
//********************************************************************************
void CBCGToolbarButton::OnChangeParentWnd (CWnd* pWndParent)
{
	m_bExtraSize = FALSE;

	if (pWndParent == NULL)
	{
		return;
	}

	CBCGToolBar* pParentBar = DYNAMIC_DOWNCAST (CBCGToolBar, pWndParent);
	if (pParentBar != NULL && pParentBar->IsButtonExtraSizeAvailable ())
	{
		m_bExtraSize = TRUE;
	}
}
