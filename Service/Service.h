#if !defined(AFX_SERVICE_H__0704EEA8_7061_11D5_BA96_00C0DF22DE85__INCLUDED_)
#define AFX_SERVICE_H__0704EEA8_7061_11D5_BA96_00C0DF22DE85__INCLUDED_

#if _MSC_VER >= 1000

#pragma once
#endif // _MSC_VER >= 1000

#include <windows.h>
#include "NTService.h"

//----------------------------------------------------------------------------

class CService : public CNTService 
{
public:
	CService();

	virtual void	Run(DWORD, LPTSTR *);
	virtual void	Stop();
	
	void			shutdown();

private:
	HANDLE			m_hStop;
	HANDLE			m_hStopProcess;
	HANDLE			m_hProcessRunning;
	HANDLE			m_hProcessServer;
};

//----------------------------------------------------------------------------

#endif // !defined(AFX_SERVICE_H__0704EEA8_7061_11D5_BA96_00C0DF22DE85__INCLUDED_)
