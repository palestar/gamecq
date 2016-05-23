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

/////////////////////////////////////////////////////////////////////////////
// Copyright (C) 1998 by Shane Martin
// All rights reserved
//
// Distribute freely, except: don't remove my name from the source or
// documentation (don't take credit for my work), mark your changes (don't
// get me blamed for your possible bugs), don't alter or remove this
// notice.
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Send bug reports, bug fixes, enhancements, requests, flames, etc., and
// I'll try to keep a version up to date.  I can be reached as follows:
//    shane.kim@kaiserslautern.netsurf.de
/////////////////////////////////////////////////////////////////////////////

// last revised: 24 Apr 98
// Registry.cpp : implementation file
//
// Description:
// CBCGRegistry is a wrapper for the Windows Registry API.  It allows
//  easy modification of the Registry with easy to remember terms like
//  Read, Write, Open, and Close.

#include "stdafx.h"
#include "BCGRegistry.h"

#define READ_ONLY_KEYS	(KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS | KEY_NOTIFY)

IMPLEMENT_DYNCREATE(CBCGRegistry, CObject)

CBCGRegistry::CBCGRegistry()
{
}

CBCGRegistry::CBCGRegistry(BOOL bAdmin, BOOL bReadOnly) :
	m_bReadOnly (bReadOnly)
{
	m_hKey = bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	m_bAdmin = bAdmin;
	m_dwUserData = 0;
}

CBCGRegistry::~CBCGRegistry()
{
	Close();
}

BOOL CBCGRegistry::VerifyKey (LPCTSTR pszPath)
{
	ASSERT (m_hKey);
	
	HKEY hKey; // New temporary hKey
	CString strPath = pszPath;
	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}
	
	LONG ReturnValue = RegOpenKeyEx (m_hKey, strPath, 0L,
		m_bReadOnly ? READ_ONLY_KEYS : KEY_ALL_ACCESS, &hKey);
	
	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;
	
	if(ReturnValue == ERROR_SUCCESS)
	{
		//If the key exists, then close it.
		RegCloseKey (hKey);
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGRegistry::VerifyValue (LPCTSTR pszValue)
{
	ASSERT(m_hKey);
	LONG lReturn = RegQueryValueEx(m_hKey, pszValue, NULL,
		NULL, NULL, NULL);

	m_Info.lMessage = lReturn;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;

	if(lReturn == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CBCGRegistry::CreateKey (LPCTSTR pszPath)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT (pszPath != NULL);
	HKEY hKeySave = m_hKey;

	CString strPath = pszPath;
	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}

	DWORD dw;

	LONG ReturnValue = RegCreateKeyEx (m_hKey, strPath, 0L, NULL,
		REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
		&m_hKey, &dw);

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;

	TRACE(_T("Can't create registry key: %s\n"), strPath);
	m_hKey = hKeySave;
	return FALSE;
}

BOOL CBCGRegistry::Open (LPCTSTR pszPath)
{
	ASSERT (pszPath != NULL);

	HKEY hKeySave = m_hKey;
	m_sPath = pszPath;

	CString strPath = pszPath;
	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}

	LONG ReturnValue = RegOpenKeyEx (m_hKey, strPath, 0L,
		m_bReadOnly ? READ_ONLY_KEYS : KEY_ALL_ACCESS, &m_hKey);

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = 0L;
	m_Info.dwType = 0L;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;

	m_hKey = hKeySave;
	return FALSE;
}

void CBCGRegistry::Close()
{
	if (m_hKey)
	{
		RegCloseKey (m_hKey);
		m_hKey = NULL;
	}
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, int iVal)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	DWORD dwValue;

	ASSERT(m_hKey);
	
	dwValue = (DWORD)iVal;
	LONG ReturnValue = RegSetValueEx (m_hKey, pszKey, 0L, REG_DWORD,
		(CONST BYTE*) &dwValue, sizeof(DWORD));

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = sizeof(DWORD);
	m_Info.dwType = REG_DWORD;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, DWORD dwVal)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT(m_hKey);
	LONG ReturnValue = RegSetValueEx (m_hKey, pszKey, 0L, REG_DWORD,
		(CONST BYTE*) &dwVal, sizeof(DWORD));

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, LPCTSTR pszData)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT(m_hKey);
	ASSERT(pszData);
	ASSERT(AfxIsValidAddress(pszData, _tcslen(pszData), FALSE));

	LONG ReturnValue = RegSetValueEx (m_hKey, pszKey, 0L, REG_SZ,
		(CONST BYTE*) pszData, (DWORD) (_tcslen(pszData) + 1) * (sizeof *pszData));

	m_Info.lMessage = ReturnValue;
	m_Info.dwSize = (DWORD)_tcslen(pszData) + 1;
	m_Info.dwType = REG_SZ;

	if(ReturnValue == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CStringList& scStringList)
{
	return Write (pszKey, (CObject&) scStringList);
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CObList& list)
{
	return Write (pszKey, (CObject&) list);
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CObject& obj)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			obj.Serialize (ar);
			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CObject* pObj)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);
			ar << pObj;
			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CByteArray& bcArray)
{
	return Write (pszKey, (CObject&) bcArray);
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CDWordArray& dwcArray)
{
	return Write (pszKey, (CObject&) dwcArray);
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CWordArray& wcArray)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			ar << (int) wcArray.GetSize ();
			for (int i = 0; i < wcArray.GetSize (); i ++)
			{
				ar << wcArray [i];
			}

			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, CStringArray& scArray)
{
	return Write (pszKey, (CObject&) scArray);
}

BOOL CBCGRegistry::Write(LPCTSTR pszKey, const CRect& rect)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	BOOL bRes = FALSE;
	try
	{
		CMemFile file;

		{
			CArchive ar (&file, CArchive::store);

			ar << rect;
			ar.Flush ();
		}

		DWORD dwDataSize = (DWORD) file.GetLength ();
		LPBYTE lpbData = file.Detach ();

		if (lpbData == NULL)
		{
			return FALSE;
		}

		bRes = Write (pszKey, lpbData, (UINT) dwDataSize);
		free (lpbData);
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Write ()!\n"));
		return FALSE;
	}

	return bRes;
}

BOOL CBCGRegistry::Write(LPCTSTR pszKey, LPPOINT& lpPoint)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT(m_hKey);
	const int iMaxChars = 20;
	CDWordArray dwcArray;
	BYTE* byData = (BYTE*)::calloc(iMaxChars, sizeof(TCHAR));
	ASSERT(byData);

	dwcArray.SetSize(5);
	dwcArray.SetAt(0, lpPoint->x);
	dwcArray.SetAt(1, lpPoint->y);

	CMemFile file(byData, iMaxChars, 16);
	CArchive ar(&file, CArchive::store);
	ASSERT(dwcArray.IsSerializable());
	dwcArray.Serialize(ar);
	ar.Close();
	const DWORD dwLen = (const DWORD) file.GetLength();
	ASSERT(dwLen < iMaxChars);
	LONG lReturn = RegSetValueEx(m_hKey, pszKey, 0, REG_BINARY,
		file.Detach(), dwLen);
	
	m_Info.lMessage = lReturn;
	m_Info.dwSize = dwLen;
	m_Info.dwType = REG_POINT;

	if(byData)
	{
		free(byData);
		byData = NULL;
	}

	if(lReturn == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGRegistry::Write (LPCTSTR pszKey, LPBYTE pData, UINT nBytes)
{
	if (m_bReadOnly)
	{
		ASSERT (FALSE);
		return FALSE;
	}

	ASSERT (m_hKey != NULL);
	ASSERT (pszKey != NULL);
	ASSERT (pData != NULL);
	ASSERT (AfxIsValidAddress (pData, nBytes, FALSE));

	LONG lResult = ::RegSetValueEx (m_hKey, pszKey, NULL, REG_BINARY,
									pData, nBytes);

	m_Info.lMessage = lResult;
	m_Info.dwSize = nBytes;
	m_Info.dwType = REG_BINARY;

	return (lResult == ERROR_SUCCESS);
}

BOOL CBCGRegistry::Read(LPCTSTR pszKey, int& iVal)
{
	ASSERT(m_hKey);

	DWORD dwType;
	DWORD dwSize = sizeof (DWORD);
	DWORD dwDest;

	LONG lReturn = RegQueryValueEx (m_hKey, (LPTSTR) pszKey, NULL,
		&dwType, (BYTE *) &dwDest, &dwSize);

	m_Info.lMessage = lReturn;
	m_Info.dwType = dwType;
	m_Info.dwSize = dwSize;

	if(lReturn == ERROR_SUCCESS)
	{
		iVal = (int)dwDest;
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, DWORD& dwVal)
{
	ASSERT(m_hKey);

	DWORD dwType;
	DWORD dwSize = sizeof (DWORD);
	DWORD dwDest;

	LONG lReturn = RegQueryValueEx (m_hKey, (LPTSTR) pszKey, NULL, 
		&dwType, (BYTE *) &dwDest, &dwSize);

	m_Info.lMessage = lReturn;
	m_Info.dwType = dwType;
	m_Info.dwSize = dwSize;

	if(lReturn == ERROR_SUCCESS)
	{
		dwVal = dwDest;
		return TRUE;
	}

	return FALSE;
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CString& sVal)
{
    ASSERT (m_hKey != NULL); 
    ASSERT (pszKey != NULL); 
	
    UINT  nBytes = 0; 
    DWORD dwType = 0; 
    DWORD dwCount = 0;
	
    LONG lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType, 
		NULL, &dwCount); 
	
    if (lResult == ERROR_SUCCESS && dwCount > 0)
    { 
		nBytes = dwCount; 
		ASSERT (dwType == REG_SZ || dwType == REG_EXPAND_SZ);
		
		BYTE* pData = new BYTE [nBytes + 1];
		
		lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType, 
			pData, &dwCount); 
		
		if (lResult == ERROR_SUCCESS &&  dwCount > 0) 
		{ 
			ASSERT (dwType == REG_SZ || dwType == REG_EXPAND_SZ);
			sVal = (TCHAR*)pData; 
		} 
		else
		{
			sVal.Empty ();
		}
		
		delete [] pData;
		pData = NULL; 
    } 
	else
	{
		sVal.Empty ();
	}
	
    m_Info.lMessage = lResult; 
    m_Info.dwType = dwType;
    m_Info.dwSize = nBytes; 
	
    return lResult == ERROR_SUCCESS;
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CStringList& scStringList)
{
	scStringList.RemoveAll ();
	return Read (pszKey, (CObject&) scStringList);
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CByteArray& bcArray)
{
	bcArray.RemoveAll();
	return Read (pszKey, (CObject&) bcArray);
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CDWordArray& dwcArray)
{
	dwcArray.RemoveAll ();
	dwcArray.SetSize (0);

	return Read (pszKey, (CObject&) dwcArray);
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CWordArray& wcArray)
{
	wcArray.SetSize (0);

	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		int iSize;
		ar >> iSize;

		wcArray.SetSize (iSize);
		for (int i = 0; i < iSize; i ++)
		{
			ar >> wcArray [i];
		}

		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGRegistry::Read ()!\n"));
	}

	delete pData;
	return bSucess;
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CStringArray& scArray)
{
	scArray.RemoveAll ();
	return Read (pszKey, (CObject&) scArray);
}

BOOL CBCGRegistry::Read(LPCTSTR pszKey, CRect& rect)
{
	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		ar >> rect;
		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGRegistry::Read ()!\n"));
	}

	delete pData;
	return bSucess;
}

BOOL CBCGRegistry::Read(LPCTSTR pszKey, LPPOINT& lpPoint)
{
	ASSERT(m_hKey);
	const int iMaxChars = 20;
	CDWordArray dwcArray;
	DWORD dwType;
	DWORD dwData = iMaxChars;
	BYTE* byData = (BYTE*)::calloc(iMaxChars, sizeof(TCHAR));
	ASSERT(byData);

	LONG lReturn = RegQueryValueEx(m_hKey, pszKey, NULL, &dwType,
		byData, &dwData);

	if(lReturn == ERROR_SUCCESS && dwType == REG_BINARY)
	{
		ASSERT(dwData < iMaxChars);
		CMemFile file(byData, dwData);
		CArchive ar(&file, CArchive::load);
		ar.m_bForceFlat = FALSE;
		ASSERT(ar.IsLoading());
		ASSERT(dwcArray.IsSerializable());
		dwcArray.RemoveAll();
		dwcArray.SetSize(5);
		dwcArray.Serialize(ar);
		ar.Close();
		byData = file.Detach ();
		lpPoint->x = dwcArray.GetAt(0);
		lpPoint->y = dwcArray.GetAt(1);
	}

	m_Info.lMessage = lReturn;
	m_Info.dwType = REG_POINT;
	m_Info.dwSize = sizeof(POINT);

	if(byData)
	{
		free(byData);
		byData = NULL;
	}

	if(lReturn == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, BYTE** ppData, UINT* pBytes)
{
	ASSERT (m_hKey != NULL);
	ASSERT (pszKey != NULL);
	ASSERT(ppData != NULL);
	ASSERT(pBytes != NULL);
	*ppData = NULL;
	*pBytes = 0;

	DWORD dwType, dwCount;
	LONG lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType,
		NULL, &dwCount);

	if (lResult == ERROR_SUCCESS && dwCount > 0)
	{
		*pBytes = dwCount;
		ASSERT (dwType == REG_BINARY);

		*ppData = new BYTE [*pBytes];

		lResult = ::RegQueryValueEx (m_hKey, pszKey, NULL, &dwType,
			*ppData, &dwCount);

		if (lResult == ERROR_SUCCESS)
		{
			ASSERT (dwType == REG_BINARY);
		}
		else
		{
			delete [] *ppData;
			*ppData = NULL;
		}
	}

	m_Info.lMessage = lResult;
	m_Info.dwType = REG_BINARY;
	m_Info.dwSize = *pBytes;

	return (lResult == ERROR_SUCCESS);
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CObList& list)
{
	while (!list.IsEmpty ())
	{
		delete list.RemoveHead ();
	}

	return Read (pszKey, (CObject&) list);
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CObject& obj)
{
	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		obj.Serialize (ar);
		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGRegistry::Read ()!\n"));
	}

	delete pData;
	return bSucess;
}

BOOL CBCGRegistry::Read (LPCTSTR pszKey, CObject*& pObj)
{
	BOOL	bSucess = FALSE;
	BYTE*	pData = NULL;
	UINT	uDataSize;

	if (!Read (pszKey, &pData, &uDataSize))
	{
		ASSERT (pData == NULL);
		return FALSE;
	}

	ASSERT (pData != NULL);

	try
	{
		CMemFile file (pData, uDataSize);
		CArchive ar (&file, CArchive::load);

		ar >> pObj;

		bSucess = TRUE;
	}
	catch (CMemoryException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("Memory exception in CBCGRegistry::Read ()!\n"));
	}
	catch (CArchiveException* pEx)
	{
		pEx->Delete ();
		TRACE(_T("CArchiveException exception in CBCGRegistry::Read ()!\n"));
	}

	delete pData;
	return bSucess;
}

BOOL CBCGRegistry::DeleteValue (LPCTSTR pszValue)
{
	ASSERT(m_hKey);
	LONG lReturn = RegDeleteValue(m_hKey, pszValue);


	m_Info.lMessage = lReturn;
	m_Info.dwType = 0L;
	m_Info.dwSize = 0L;

	if(lReturn == ERROR_SUCCESS)
		return TRUE;

	return FALSE;
}

BOOL CBCGRegistry::DeleteKey (LPCTSTR pszPath, BOOL bAdmin)
{
	if (m_bReadOnly)
	{
		return FALSE;
	}
	
	ASSERT (pszPath != NULL);
	
	CString strPath = pszPath;

	int iPathLen = strPath.GetLength ();
	if (iPathLen > 0 && strPath [iPathLen - 1] == _T('\\'))
	{
		strPath = strPath.Left (iPathLen - 1);
	}
	
	// open the key
	HKEY hSubKey;
	LONG lReturn = ::RegOpenKeyEx (bAdmin ? HKEY_LOCAL_MACHINE :
	HKEY_CURRENT_USER,
		strPath, 0L, KEY_ALL_ACCESS, &hSubKey);
	
	if(lReturn != ERROR_SUCCESS)
		return FALSE;
	
	// first, delete all subkeys (else it won't work on NT!)
	for( DWORD dwSubKeys = 1; dwSubKeys > 0; )
	{
		dwSubKeys = 0;
		
		// first get an info about this subkey ...
		DWORD dwSubKeyLen;
		if( ::RegQueryInfoKey( hSubKey, 0,0,0, &dwSubKeys, &dwSubKeyLen,
			0,0,0,0,0,0) != ERROR_SUCCESS)
		{
			::RegCloseKey(hSubKey);
			return FALSE;
		}
		
		if( dwSubKeys > 0 )
		{
			// call DeleteKey() recursivly
			LPTSTR szSubKeyName = new TCHAR[dwSubKeyLen + 1];
			
			if( ::RegEnumKey( hSubKey, 0, szSubKeyName, dwSubKeyLen+1) !=
				ERROR_SUCCESS
				|| ! DeleteKey( strPath + "\\" + szSubKeyName, bAdmin ) )
			{
				delete szSubKeyName;
				::RegCloseKey(hSubKey);
				return FALSE;
			}
			
			delete szSubKeyName;
		}
	}
	::RegCloseKey(hSubKey);
	
	// finally delete the whole key
	lReturn = ::RegDeleteKey (bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
		strPath);
	m_Info.lMessage = lReturn;
	m_Info.dwType = 0L;
	m_Info.dwSize = 0L;
	
	if(lReturn == ERROR_SUCCESS)
		return TRUE;
	
	return FALSE;
}

BOOL CBCGRegistry::ReadSubKeys(CStringArray& SubKeys)
{
	BOOL result = TRUE;
	DWORD rc = ERROR_SUCCESS;
	TCHAR szSubKey[ 1024  ] = _T("\0");
	DWORD length = sizeof( szSubKey );

	ASSERT(m_hKey);

	int index = 0;
	rc = RegEnumKeyEx(m_hKey, index, szSubKey, &length, NULL, NULL, NULL, NULL);

	if( rc == ERROR_NO_MORE_ITEMS) {
		result = false;
	}
	else while(rc == ERROR_SUCCESS) {
		SubKeys.Add( szSubKey );		
		length = sizeof( szSubKey );
		index++;
		rc = RegEnumKeyEx(m_hKey, index, szSubKey, &length, NULL, NULL, NULL, NULL);		
		
	} // while
	return( result );
}


BOOL CBCGRegistry::ReadKeyValues(CStringArray &Values)
{
	DWORD rc = ERROR_SUCCESS;
	BOOL result = FALSE;
	TCHAR szValue[ 1024 ];
	DWORD length = sizeof( szValue );
	int index = 0;

	ASSERT(m_hKey);

	rc = RegEnumValue(m_hKey, index, szValue, &length, NULL, NULL, NULL, NULL);
	if( rc == ERROR_NO_MORE_ITEMS) {
	  result = FALSE;
	}
	else while( rc == ERROR_SUCCESS )  {
		result = TRUE;
		Values.Add( szValue );
		length = sizeof( szValue );
		index++;
		rc = RegEnumValue(m_hKey, index, szValue, &length, NULL, NULL, NULL, NULL);		
	}
	return( result );

}

//////////////////////////////////////////////////////////////////////////////
// CBCGRegistrySP - Helper class that manages "safe" CBCGRegistry pointer

CRuntimeClass* CBCGRegistrySP::m_pRTIDefault = NULL;

BOOL CBCGRegistrySP::SetRuntimeClass (CRuntimeClass* pRTI)
{
	if (pRTI != NULL &&
		!pRTI->IsDerivedFrom (RUNTIME_CLASS (CBCGRegistry)))
	{
		ASSERT (FALSE);
		return FALSE;
	}

	m_pRTIDefault = pRTI;
	return TRUE;
}

CBCGRegistry& CBCGRegistrySP::Create (BOOL bAdmin, BOOL bReadOnly)
{
	if (m_pRegistry != NULL)
	{
		ASSERT (FALSE);
		ASSERT_VALID (m_pRegistry);

		return *m_pRegistry;
	}

	if (m_pRTIDefault == NULL)
	{
		m_pRegistry = new CBCGRegistry;
	}
	else
	{
		ASSERT (m_pRTIDefault->IsDerivedFrom (RUNTIME_CLASS (CBCGRegistry)));
		m_pRegistry = DYNAMIC_DOWNCAST (CBCGRegistry, 
										m_pRTIDefault->CreateObject ());
	}

	ASSERT_VALID (m_pRegistry);

	m_pRegistry->m_bReadOnly = bReadOnly;
	m_pRegistry->m_hKey = bAdmin ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER;
	m_pRegistry->m_bAdmin = bAdmin;
	m_pRegistry->m_dwUserData = m_dwUserData;

	return *m_pRegistry;
}

