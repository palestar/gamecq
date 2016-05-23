// ColorPage1.cpp : implementation file
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

#include "bcgbarres.h"
#include "bcgcontrolbar.h"
#include "BCGColorDialog.h"
#include "ColorPage1.h"
#include "BCGDrawManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CColorPage1 property page

IMPLEMENT_DYNCREATE(CColorPage1, CPropertyPage)

CColorPage1::CColorPage1() : CPropertyPage(CColorPage1::IDD)
{
	//{{AFX_DATA_INIT(CColorPage1)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CColorPage1::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CColorPage1)
	DDX_Control(pDX, IDC_BCGBARRES_HEXPLACEHOLDER, m_hexpicker);
	DDX_Control(pDX, IDC_BCGBARRES_GREYSCALEPLACEHOLDER, m_hexpicker_greyscale);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CColorPage1, CPropertyPage)
	//{{AFX_MSG_MAP(CColorPage1)
	ON_BN_CLICKED(IDC_BCGBARRES_GREYSCALEPLACEHOLDER, OnGreyscale)
	ON_BN_CLICKED(IDC_BCGBARRES_HEXPLACEHOLDER, OnHexColor)
	ON_BN_DOUBLECLICKED(IDC_BCGBARRES_GREYSCALEPLACEHOLDER, OnDoubleClickedColor)
	ON_BN_DOUBLECLICKED(IDC_BCGBARRES_HEXPLACEHOLDER, OnDoubleClickedColor)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CColorPage1 message handlers

BOOL CColorPage1::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	
	m_hexpicker.SetPalette (m_pDialog->GetPalette ());
	m_hexpicker.SetType(CBCGColorPickerCtrl::HEX);

	m_hexpicker_greyscale.SetPalette (m_pDialog->GetPalette ());
	m_hexpicker_greyscale.SetType(CBCGColorPickerCtrl::HEX_GREYSCALE);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CColorPage1::OnGreyscale() 
{
	double H,L,S;
	m_hexpicker_greyscale.GetHLS(&H,&L,&S);

	COLORREF color = CBCGDrawManager::HLStoRGB_TWO(H, L, S);

	m_pDialog->SetNewColor (color);

	BYTE R = GetRValue (color);
	BYTE G = GetGValue (color);
	BYTE B = GetBValue (color);

	m_pDialog->SetPageTwo (R, G, B);

	m_hexpicker.SelectCellHexagon (R, G, B);
	m_hexpicker.Invalidate ();
}

void CColorPage1::OnHexColor() 
{
	COLORREF color = m_hexpicker.GetRGB ();

	BYTE R = GetRValue (color);
	BYTE G = GetGValue (color);
	BYTE B = GetBValue (color);

	double H,L,S;
	m_hexpicker.GetHLS (&H,&L,&S);

	// Set actual color.
	m_pDialog->SetNewColor (/*HLStoRGB_TWO(H, L, S)*/color);

	m_pDialog->SetPageTwo(R, G, B);

	m_hexpicker_greyscale.SelectCellHexagon (R, G, B);
	m_hexpicker_greyscale.Invalidate ();
}

void CColorPage1::OnDoubleClickedColor() 
{
	m_pDialog->EndDialog (IDOK);
}

#endif // BCG_NO_COLOR

