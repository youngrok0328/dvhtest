#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#ifdef __DPI_SYSTEMINISLITDEF_EXPORTS__
#define DPI_SYSTEMINISLITDEF_API __declspec(dllexport)
#else
#define DPI_SYSTEMINISLITDEF_API __declspec(dllimport)
#endif
