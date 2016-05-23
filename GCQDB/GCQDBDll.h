/*
	GCQDBDll.h
	(c)2004 Palestar, Richard Lyle
*/

#ifdef _WIN32

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#include "Standard/Dll.h"

#undef DLL

#ifdef GCQDB_DLL
#define DLL			DLL_EXPORT
#else
#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"GCQDBD.lib") 
#else
#pragma comment(lib,"GCQDB.lib") 
#endif

#endif

#else

#undef DLL
#define DLL

#endif

//----------------------------------------------------------------------------
// EOF
