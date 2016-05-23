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


// BCGControlBar.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxdllx.h>
#include "bcgcontrolbar.h"
#include "bcgsound.h"
#include "bcglocalres.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static AFX_EXTENSION_MODULE BCGControlBarDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("BCGCONTROLBAR.DLL Initializing!\n");

		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(BCGControlBarDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		#ifndef _BCGCB_IN_OTHER_DLL
			new CDynLinkLibrary(BCGControlBarDLL);
		#endif	// _BCGCB_IN_OTHER_DLL
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("BCGCONTROLBAR.DLL Terminating!\n");
		BCGCBCleanUp ();
		// Terminate the library before destructors are called
		AfxTermExtensionModule(BCGControlBarDLL);
	}
	return 1;   // ok
}

///////////////////////////////////////////////////////////////////////
// CBCGLocalResource implementation:

CBCGLocalResource::CBCGLocalResource()
{
	m_hInstOld = AfxGetResourceHandle ();
	ASSERT (m_hInstOld != NULL);

	AfxSetResourceHandle (BCGControlBarDLL.hResource);
}

CBCGLocalResource::~CBCGLocalResource()
{
	AfxSetResourceHandle (m_hInstOld);
}

void BCGCBSetResourceHandle (HINSTANCE hinstResDLL)
{
	BCGControlBarDLL.hResource = (hinstResDLL == NULL) ? 
		BCGControlBarDLL.hModule : hinstResDLL;
}

#ifdef _BCGCB_IN_OTHER_DLL
	__declspec(dllexport) void BCGControlBarDllInitialize ()
	{
		new CDynLinkLibrary(BCGControlBarDLL);
	}
#endif	// _BCGCB_IN_OTHER_DLL
