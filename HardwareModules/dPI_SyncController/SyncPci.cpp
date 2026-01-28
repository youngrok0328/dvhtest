#include "stdafx.h" // Visual C++에서는 이 주석을 해제한다(단 Precomipiled header를 사용하지 않는 경우에는 예외). 
#include <windows.h>
#define _COMIUSB_LOADER_C_
#include "SyncPci.h"

#ifdef __cplusplus
extern "C"{
#endif

typedef struct{
    const char *name;
    FARPROC* ptr;
}LIB_FUNCT;

HINSTANCE hLibrary=NULL;

BOOL SyncPci_LoadDll(ULONG unPciType)
{
	if (unPciType < 0) return FALSE;

	LIB_FUNCT Func[] = {
						{"SyncPci_LoadDevice",		(FARPROC*)&SyncPci_LoadDevice},
						{"SyncPci_UnloadDevice",	(FARPROC*)&SyncPci_UnloadDevice},
						{"SyncPci_WriteMicomIo",	(FARPROC*)&SyncPci_WriteMicomIo},
						{"SyncPci_WriteHandlerIo",	(FARPROC*)&SyncPci_WriteHandlerIo},
						{"SyncPci_ReadMicomIo",		(FARPROC*)&SyncPci_ReadMicomIo},
						{"SyncPci_ReadHandlerIo",	(FARPROC*)&SyncPci_ReadHandlerIo},
						{"SyncPci_WriteAvrParam",	(FARPROC*)&SyncPci_WriteAvrParam},
						{"SyncPci_ReadHandler",		(FARPROC*)&SyncPci_ReadHandler},
						{"SyncPci_WriteHandler",	(FARPROC*)&SyncPci_WriteHandler},
						{"SyncPci_ReadAvrStat",		(FARPROC*)&SyncPci_ReadAvrStat},
						{"SyncPci_WriteAvrRst",		(FARPROC*)&SyncPci_WriteAvrRst},
						{"SyncPci_ReadHandlerOut",	(FARPROC*)&SyncPci_ReadHandlerOut},
						{"SyncPci_Write",			(FARPROC*)&SyncPci_Write},
						{"SyncPci_Read",			(FARPROC*)&SyncPci_Read},
						{NULL, NULL}
					   };
	LIB_FUNCT *f;
	
	if((hLibrary = LoadLibrary(_T("SyncPci.dll"))) == NULL)
    	return FALSE;
	
	f = &Func[0];
	while(f->ptr != NULL)
	{
		*(f->ptr) = GetProcAddress ((HMODULE)hLibrary, f->name);
		f++;
	}
	return TRUE;
}

void SyncPci_UnloadDll(void)
{
	if (hLibrary)
		FreeLibrary(hLibrary);
}

#ifdef __cplusplus
}
#endif