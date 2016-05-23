#ifndef __BCGCONTROLBAR_H
#define __BCGCONTROLBAR_H

// This is a part of the BCGControlBar Library
// Copyright (C) 1998-2006 BCGSoft Ltd.
// All rights reserved.
//
// This source code can be used, distributed or modified
// only under terms and conditions 
// of the accompanying license agreement.

#ifndef __AFXCMN_H__
	#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif

#if defined _AFXDLL && !defined _BCGCONTROLBAR_STATIC_
	#ifdef _BCGCONTROLBAR_
	   #define BCGCONTROLBARDLLEXPORT  _declspec(dllexport)
	#else
	   #define BCGCONTROLBARDLLEXPORT  _declspec(dllimport)
	#endif
#else
	#define BCGCONTROLBARDLLEXPORT
#endif

#include "BCGUserToolsManager.h"

inline BOOL IsStandardCommand (UINT uiCmd)
{
	return	((uiCmd >= ID_FILE_MRU_FILE1 && 
				uiCmd <= ID_FILE_MRU_FILE16)		||	// MRU commands,
			(uiCmd >= 0xF000 && uiCmd < 0xF1F0)		||	// system commands,
			((int) uiCmd >= AFX_IDM_FIRST_MDICHILD)	||	// windows commands
			(uiCmd >= ID_OLE_VERB_FIRST && uiCmd <= ID_OLE_VERB_LAST) ||		// OLE commands
			g_pUserToolsManager != NULL && uiCmd == g_pUserToolsManager->GetToolsEntryCmd ());
}

BCGCONTROLBARDLLEXPORT void BCGCBSetResourceHandle (
	HINSTANCE hinstResDLL);

BCGCONTROLBARDLLEXPORT void BCGCBCleanUp ();

extern BCGCONTROLBARDLLEXPORT CFrameWnd* g_pTopLevelFrame;

inline BCGCONTROLBARDLLEXPORT void BCGSetTopLevelFrame (CFrameWnd* pFrame)
{
	g_pTopLevelFrame = pFrame;
}

inline BCGCONTROLBARDLLEXPORT CFrameWnd* BCGGetTopLevelFrame (const CWnd* pWnd)
{
	ASSERT_VALID (pWnd);
	return g_pTopLevelFrame == NULL ? pWnd->GetTopLevelFrame () : g_pTopLevelFrame;
}

class BCGCONTROLBARDLLEXPORT CBCGMemDC
{
public:
	static BOOL	m_bUseMemoryDC;

	CBCGMemDC(CDC& dc, CWnd* pWnd) :
		m_dc (dc),
		m_bMemDC (FALSE),
		m_pOldBmp (NULL)
	{
		ASSERT_VALID(pWnd);

		pWnd->GetClientRect (m_rect);

		m_rect.right += pWnd->GetScrollPos (SB_HORZ);
		m_rect.bottom += pWnd->GetScrollPos (SB_VERT);

		if (m_bUseMemoryDC &&
			m_dcMem.CreateCompatibleDC (&m_dc) &&
			m_bmp.CreateCompatibleBitmap (&m_dc, m_rect.Width (), m_rect.Height ()))
		{
			//-------------------------------------------------------------
			// Off-screen DC successfully created. Better paint to it then!
			//-------------------------------------------------------------
			m_bMemDC = TRUE;
			m_pOldBmp = m_dcMem.SelectObject (&m_bmp);
		}
	}

	virtual ~CBCGMemDC()
	{
		if (m_bMemDC)
		{
			//--------------------------------------
			// Copy the results to the on-screen DC:
			//-------------------------------------- 
			CRect rectClip;
			int nClipType = m_dc.GetClipBox (rectClip);

			if (nClipType != NULLREGION)
			{
				if (nClipType != SIMPLEREGION)
				{
					rectClip = m_rect;
				}

				m_dc.BitBlt (rectClip.left, rectClip.top, rectClip.Width(), rectClip.Height(),
							   &m_dcMem, rectClip.left, rectClip.top, SRCCOPY);
			}

			m_dcMem.SelectObject (m_pOldBmp);
		}
	}

	CDC& GetDC ()			{	return m_bMemDC ? m_dcMem : m_dc;	}
	BOOL IsMemDC () const	{	return m_bMemDC;	}

protected:
	CDC&		m_dc;
	BOOL		m_bMemDC;
	CDC			m_dcMem;
	CBitmap		m_bmp;
	CBitmap*	m_pOldBmp;
	CRect		m_rect;
};

void BCGCONTROLBARDLLEXPORT BCGShowAboutDlg (LPCTSTR lpszAppName);
void BCGCONTROLBARDLLEXPORT BCGShowAboutDlg (UINT uiAppNameResID);

#define BCG_GET_X_LPARAM(lp)		((int)(short)LOWORD(lp))
#define BCG_GET_Y_LPARAM(lp)		((int)(short)HIWORD(lp))

#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL         0x00400000L // Right to left mirroring
#endif

#ifndef WS_EX_LAYERED
#define WS_EX_LAYERED           0x00080000
#endif

#ifndef LWA_COLORKEY
#define LWA_COLORKEY            0x00000001
#endif

#ifndef LWA_ALPHA
#define LWA_ALPHA               0x00000002
#endif

#if _MSC_VER < 1300

//------------------------------
// Windows 64 bit compatibility:
//------------------------------

#ifndef GetClassLongPtr
#define GetClassLongPtr		GetClassLong
#endif

#ifndef SetClassLongPtr
#define SetClassLongPtr		SetClassLong
#endif

#ifndef SetWindowLongPtr
#define SetWindowLongPtr	SetWindowLong
#endif

#ifndef GetWindowLongPtr
#define GetWindowLongPtr	GetWindowLong
#endif

#define	DWORD_PTR			DWORD
#define	INT_PTR				int
#define	UINT_PTR			UINT
#define	LONG_PTR			LONG

#ifndef GWLP_WNDPROC
#define	GWLP_WNDPROC		GWL_WNDPROC
#endif

#ifndef GCLP_HICON
#define	GCLP_HICON			GCL_HICON
#endif

#ifndef GCLP_HICONSM
#define	GCLP_HICONSM		GCL_HICONSM
#endif

#ifndef GCLP_HBRBACKGROUND
#define GCLP_HBRBACKGROUND	GCL_HBRBACKGROUND
#endif
                
#endif // _MSC_VER

#if _MSC_VER >= 1400
#define BCGNcHitTestType	LRESULT
#else
#define BCGNcHitTestType	UINT
#endif

#pragma warning (disable : 4996)

#endif // __BCGCONTROLBAR_H
