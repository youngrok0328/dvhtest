#ifndef _CMU_USB_H_
#define _CMU_USB_H_

#ifdef __cplusplus
extern "C"{
#endif

#ifdef COMIUSBDLL_EXPORTS
 #define COMIDAS_API _declspec(dllexport)
#else
 #ifdef _COMIUSB_LOADER_C_
  #define EXTERN
 #else
  #define EXTERN extern
 #endif
BOOL SyncPci_LoadDll(ULONG unPciType);
void SyncPci_UnloadDll(void);
EXTERN HANDLE	(*SyncPci_LoadDevice) (ULONG instance);
EXTERN void	(*SyncPci_UnloadDevice) ();
EXTERN BOOL (*SyncPci_WriteMicomIo)(HANDLE hDevice, ULONG offset, byte value);
EXTERN BOOL (*SyncPci_WriteHandlerIo)(HANDLE hDevice, ULONG offset, byte value);
EXTERN BOOL (*SyncPci_ReadMicomIo)(HANDLE hDevice, ULONG offset, byte* pValue);
EXTERN BOOL (*SyncPci_ReadHandlerIo)(HANDLE hDevice, ULONG offset, byte* pValue);
EXTERN BOOL (*SyncPci_WriteAvrParam)(HANDLE hDevice, byte addr, byte data);
EXTERN BOOL (*SyncPci_ReadHandler)(HANDLE hDevice, ULONG addr, byte *pValue);
EXTERN BOOL (*SyncPci_WriteHandler)(HANDLE hDevice, byte addr, byte data, ULONG option);
EXTERN BOOL (*SyncPci_ReadAvrStat)(HANDLE hDevice, byte *pValue);
EXTERN BOOL (*SyncPci_WriteAvrRst)(HANDLE hDevice, BOOL rst);
EXTERN BOOL	(*SyncPci_ReadHandlerOut)(HANDLE hDevice, ULONG addr, byte *value);

// 2005.08.09 추가
// CS, offset을 설정하여 모든 레지스터에 접근하는 범용 입출력 함수
EXTERN BOOL (*SyncPci_Write)(HANDLE hDevice, ULONG cs, ULONG offset, byte value);
EXTERN BOOL (*SyncPci_Read)(HANDLE hDevice, ULONG cs, ULONG offset, byte *pValue);

#endif // COMIUSBDLL_EXPORTS

#ifdef __cplusplus
}
#endif

#endif // _CMU_USB_H_
