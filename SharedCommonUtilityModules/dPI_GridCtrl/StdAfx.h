#pragma once
#define _CRTDBG_MAP_ALLOC

# if (_MSC_VER > 1000)
#  pragma warning (disable : 4355 4284 4231 4511 4512 4097 4786 4800 4018 4146 4244 4514 4127 4100 4663)
#  pragma warning (disable : 4245 4514 4660) 
#  if (_MSC_VER > 1200)
#   pragma warning (disable : 4521 4522)
#  endif  
# endif

#define WINVER 0x0601

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC 핵심 및 표준 구성 요소입니다.
#include <afxext.h>         // MFC 확장입니다.
#include <afxcmn.h>			// Windows 공용 컨트롤에 대한 MFC 지원입니다.

//#include "../../Preprocessor.h"

#define __DPI_GridCtrlDEF_EXPORTS__
