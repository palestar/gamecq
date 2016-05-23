#include "stdafx.h"

#define MAXKEYBYTES 	56		// 448 bits max
#define NPASS           16		// SBox passes

class CPWCipher
{
	public:
				CPWCipher();
				~CPWCipher();
		DWORD	GetOutputLength( DWORD lInputLong );
		CString	Encode ( CString inp );
		CString	Decode ( CString inp );

	private:
		DWORD	*PArray;
		DWORD	(* SBoxes)[256];
		void 	Initialize( BYTE key[], int keybytes );
		void 	Priv_encipher( DWORD *xl, DWORD *xr );
		void 	Priv_decipher( DWORD *xl, DWORD *xr );

		CString	sUsername;
		CString	sComputerName;

	union aword
	{
		DWORD dword;
		BYTE byte [4];
		struct
		{
			unsigned int byte3:8;
			unsigned int byte2:8;
			unsigned int byte1:8;
			unsigned int byte0:8;
		} w;
	};

};
