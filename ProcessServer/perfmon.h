// PerfMon.h: interface for the CPerfMon class.
//
//////////////////////////////////////////////////////////////////////
// By Mike Ryan (mike@codexia.com)
// Copyright (c) 2000, portions (c) Allen Denver
// 07.30.2000
//
// Some of the code based on Allen Denver's article "Using the Performance Data Helper Library"
//
// Free usage granted in all applications including commercial.
// Do NOT distribute without permission from me.  I can be reached
// at mike@codexia.com, http://www.codexia.com
// Please feel free to email me about this class.
//
// Compatibility:
//     Windows 98, Windows NT 4.0 SP 3 (Dlls required), Windows 2000
//
// Development Environ:
//     Visual C++ 6.0
//
// Libraries / DLLs:
//     pdh.lib (linked in)
//     pdh.dll (provided with Windows 2000, must copy in for NT 4.0)
//
//////////////////////////////////////////////////////////////////////

#ifndef _PERFMON_H
#define _PERFMON_H

#include "Standard/Array.h"

#include <pdh.h>
#include <pdhmsg.h>

#define MAX_RAW_VALUES 20

//// DEFINES FOR COUNTER NAMES ////
#define CNTR_CPU "\\Processor(_Total)\\% Processor Time" // % of cpu in use
#define CNTR_MEMINUSE_BYTES "\\Memory\\Committed Bytes" // mem in use measured in bytes
#define CNTR_MEMAVAIL_BYTES "\\Memory\\Available Bytes" // mem available measured in bytes
#define CNTR_MEMAVAIL_KB "\\Memory\\Available KBytes" // mem avail in kilobytes
#define CNTR_MEMAVAIL_MB "\\Memory\\Available MBytes" // mem avail in megabytes
#define CNTR_MEMINUSE_PERCENT "\\Memory\\% Committed Bytes In Use" // % of mem in use
#define CNTR_MEMLIMIT_BYTES "\\Memory\\Commit Limit" // commit limit on memory in bytes

// NOTE: Find other counters using the function PdhBrowseCounters() (lookup in MSDN).
// This function was not implemented in this class.

typedef struct _tag_PDHCounterStruct {
	int nIndex;				// The index of this counter, returned by AddCounter()
	LONG lValue;			// The current value of this counter
    HCOUNTER hCounter;      // Handle to the counter - given to use by PDH Library
    int nNextIndex;         // element to get the next raw value
    int nOldestIndex;       // element containing the oldes raw value
    int nRawCount;          // number of elements containing raw values
    PDH_RAW_COUNTER a_RawValue[MAX_RAW_VALUES]; // Ring buffer to contain raw values
} PDHCOUNTERSTRUCT, *PPDHCOUNTERSTRUCT;

class CPerfMon
{
public:
	CPerfMon();
	~CPerfMon();

	//// SETUP ////
	BOOL Initialize(void);
	void Uninitialize(void);

	//// COUNTERS ////
	int AddCounter(const char *pszCounterName);
	BOOL RemoveCounter(int nIndex);

	//// DATA ////
	BOOL CollectQueryData(void);
	BOOL GetStatistics(long *nMin, long *nMax, long *nMean, int nIndex);
	long GetCounterValue(int nIndex);

protected:
	//// COUNTERS ////
	PPDHCOUNTERSTRUCT GetCounterStruct(int nIndex);

	//// VALUES ////
	BOOL UpdateValue(PPDHCOUNTERSTRUCT pCounter);
	BOOL UpdateRawValue(PPDHCOUNTERSTRUCT pCounter);

	//// VARIABLES ////
	HQUERY		m_hQuery; // the query to the PDH
	Array< PPDHCOUNTERSTRUCT > 
				m_aCounters; // the current counters
	int m_nNextIndex;
};

#endif
