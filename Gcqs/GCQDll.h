/*
	GCQDll.h
	(c)1998 Palestar, Richard Lyle
*/

#if defined(_WIN32)

//----------------------------------------------------------------------------
// NOTE: no include guards, this is because DLL will be defined multiple times

#include "Standard/Dll.h"

#undef DLL

#ifdef GCQS_DLL
#define DLL			DLL_EXPORT
#else

#define DLL			DLL_IMPORT

#ifdef _DEBUG
#pragma comment(lib,"GCQSD.lib") 
#else
#pragma comment(lib,"GCQS.lib") 
#endif

#endif

#else

#undef DLL
#define DLL

#endif


//----------------------------------------------------------------------------
// EOF
