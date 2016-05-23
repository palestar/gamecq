// ColorPage2.cpp : implementation file
//
// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#include "stdafx.h"

#ifndef BCG_NO_COLOR

#include "bcgcontrolbar.h"
#include "BCGColorDialog.h"
#include "bcgbarres.h"
#include "ColorPage2.h"
#include "BCGDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static double Int2HLS (UINT n)
{
	return min (1., (double) (.5 + n) / 255.);
}

static UINT HLS2Int (double n)
{
	return min (255, (UINT) (.5 + n * 255.));
}

/////////////////////////////////////////////////////////////////////////////
// CColorPage2 property page

IMPLEMENT_DYNCREATE(CColorPage2, CPropertyPage)

CColorPage2::CColorPage2() : CPropertyPage(CColorPage2::IDD)
{
	//{{AFX_DATA_INIT(CColorPage2)
	m_r = 0;
	m_b = 0;
	m_g = 0;
	m_l = 0;
	m_h = 0;
	m_s = 0;
	//}}AFX_DATA_INIT

	m_pDialog = NULL;
	m_bIsReady = FALSE;
	m_bInUpdate = FALSE;
}

void CColorPage2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorPage2)
	DDX_Control(pDX, IDC_BCGBARRES_LUMINANCEPLACEHOLDER, m_wndLuminance);
	DDX_Control(pDX, IDC_BCGBARRES_COLOURPLACEHOLDER, m_wndColorPicker);
	DDX_Text(pDX, IDC_BCGBARRES_R, m_r);
	DDV_MinMaxInt(pDX, m_r, 0, 255);
	DDX_Text(pDX, IDC_BCGBARRES_B, m_b);
	DDV_MinMaxInt(pDX, m_b, 0, 255);
	DDX_Text(pDX, IDC_BCGBARRES_G, m_g);
	DDV_MinMaxInt(pDX, m_g, 0, 255);
	DDX_Text(pDX, IDC_BCGBARRES_L, m_l);
	DDV_MinMaxUInt(pDX, m_l, 0, 255);
	DDX_Text(pDX, IDC_BCGBARRES_H, m_h);
	DDV_MinMaxUInt(pDX, m_h, 0, 255);
	DDX_Text(pDX, IDC_BCGBARRES_S, m_s);
	DDV_MinMaxUInt(pDX, m_s, 0, 255);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorPage2, CPropertyPage)
	//{{AFX_MSG_MAP(CColorPage2)
	ON_EN_CHANGE(IDC_BCGBARRES_B, OnRGBChanged)
	ON_EN_CHANGE(IDC_BCGBARRES_H, OnHLSChanged)
	ON_BN_CLICKED(IDC_BCGBARRES_LUMINANCEPLACEHOLDER, OnLuminance)
	ON_BN_CLICKED(IDC_BCGBARRES_COLOURPLACEHOLDER, OnColour)
	ON_EN_CHANGE(IDC_BCGBARRES_G, OnRGBChanged)
	ON_EN_CHANGE(IDC_BCGBARRES_R, OnRGBChanged)
	ON_EN_CHANGE(IDC_BCGBARRES_L, OnHLSChanged)
	ON_EN_CHANGE(IDC_BCGBARRES_S, OnHLSChanged)
	ON_BN_DOUBLECLICKED(IDC_BCGBARRES_COLOURPLACEHOLDER, OnDoubleClickedColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorPage2 message handlers

BOOL CColorPage2::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_wndColorPicker.SetPalette (m_pDialog->GetPalette ());
	m_wndColorPicker.SetType(CBCGColorPickerCtrl::PICKER);

	double hue, luminance, saturation;

	m_wndColorPicker.GetHLS(&hue, &luminance, &saturation);
	
	m_wndLuminance.SetPalette (m_pDialog->GetPalette ());
	m_wndLuminance.SetType(CBCGColorPickerCtrl::LUMINANCE);
	m_wndLuminance.SetHLS(hue, luminance, saturation);
	m_wndLuminance.SetLuminanceBarWidth(14);

	// Initialize spin controls:
	for (UINT uiID = IDC_BCGBARRES_SPIN1; uiID <= IDC_BCGBARRES_SPIN6; uiID++)
	{
		CSpinButtonCtrl* pWnd = (CSpinButtonCtrl*) GetDlgItem (uiID);
		if (pWnd == NULL)
		{
			VERIFY (FALSE);
			break;
		}

		pWnd->SetRange (0, 255);
	}

	m_bIsReady = TRUE;
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CColorPage2::OnRGBChanged() 
{
	if (m_bInUpdate || !m_bIsReady || !UpdateData ())
	{
		return;
	}

	m_bInUpdate = TRUE;
	COLORREF color = RGB (m_r, m_g, m_b);

	m_pDialog->SetNewColor (color);
	m_pDialog->SetPageOne ((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	double hue;
	double luminance;
	double saturation;
	CBCGDrawManager::RGBtoHSL (color, &hue, &saturation, &luminance);

	m_h = HLS2Int (hue);
	m_l = HLS2Int (luminance);
	m_s = HLS2Int (saturation);

	UpdateData(FALSE);

	m_wndColorPicker.SetHLS(hue, luminance, saturation, TRUE);
	m_wndLuminance.SetHLS(hue, luminance, saturation, TRUE);

	m_bInUpdate = FALSE;
}

void CColorPage2::OnHLSChanged() 
{
	if (m_bInUpdate || !m_bIsReady || !UpdateData ())
	{
		return;
	}

	m_bInUpdate = TRUE;

	double dblH = Int2HLS (m_h);
	double dblS = Int2HLS (m_s);
	double dblL = Int2HLS (m_l);

	COLORREF color = CBCGDrawManager::HLStoRGB_ONE (dblH, dblL, dblS);

	m_r = GetRValue (color);
	m_g = GetGValue (color);
	m_b = GetBValue (color);

	UpdateData(FALSE);

	m_pDialog->SetNewColor (color);
	m_pDialog->SetPageOne ((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	m_wndColorPicker.SetHLS (dblH, dblL, dblS, TRUE);
	m_wndLuminance.SetHLS (dblH, dblL, dblS, TRUE);

	m_bInUpdate = FALSE;
}

void CColorPage2::Setup (BYTE R, BYTE G, BYTE B)
{
	double hue;
	double luminance;
	double saturation;
	CBCGDrawManager::RGBtoHSL((COLORREF)RGB(R, G, B), &hue, &saturation, &luminance);

	m_wndColorPicker.SetHLS(hue, luminance, saturation);
	m_wndLuminance.SetHLS(hue, luminance, saturation);

	m_r = R;
	m_g = G;
	m_b = B;
	
	m_h = HLS2Int (hue);
	m_l = HLS2Int (luminance);
	m_s = HLS2Int (saturation);

	if (GetSafeHwnd () != NULL)
	{
		UpdateData(FALSE);
	}
}

void CColorPage2::OnLuminance() 
{
	m_bInUpdate = TRUE;

	double luminance = m_wndLuminance.GetLuminance();
	m_wndColorPicker.SetLuminance(luminance);

	double H,L,S;
	m_wndColorPicker.GetHLS(&H,&L,&S);
	m_h = HLS2Int (H);
	m_l = HLS2Int (L);
	m_s = HLS2Int (S);

	COLORREF color = CBCGDrawManager::HLStoRGB_ONE(H, L, S);

	m_pDialog->SetNewColor (color);

	m_r = GetRValue (color);
	m_g = GetGValue (color);
	m_b = GetBValue (color);

	m_pDialog->SetPageOne ((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	UpdateData(FALSE);
	m_bInUpdate = FALSE;
}

void CColorPage2::OnColour() 
{
	m_bInUpdate = TRUE;
	COLORREF ref = m_wndColorPicker.GetRGB ();

	m_r = GetRValue(ref);
	m_g = GetGValue(ref);
	m_b = GetBValue(ref);

	double saturation = m_wndColorPicker.GetSaturation();
	double hue = m_wndColorPicker.GetHue();

	m_wndLuminance.SetHue(hue);
	m_wndLuminance.SetSaturation(saturation);
	m_wndLuminance.Invalidate();

	double H,L,S;
	m_wndColorPicker.GetHLS(&H,&L,&S);
	m_h = HLS2Int (H);
	m_l = HLS2Int (L);
	m_s = HLS2Int (S);

	// Set actual color.
	m_pDialog->SetNewColor(CBCGDrawManager::HLStoRGB_ONE(H, L, S));
	m_pDialog->SetPageOne((BYTE) m_r, (BYTE) m_g, (BYTE) m_b);

	UpdateData(FALSE);
	m_bInUpdate = FALSE;
}

void CColorPage2::OnDoubleClickedColor() 
{
	m_pDialog->EndDialog (IDOK);
}

#endif // BCG_NO_COLOR
