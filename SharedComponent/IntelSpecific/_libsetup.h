#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#ifdef __INTEKPLUS_SHARED_INTELSPECIFIC_EXPORT__
#define __INTEKPLUS_SHARED_INTELSPECIFIC_API__ __declspec(dllexport)
#else
#define __INTEKPLUS_SHARED_INTELSPECIFIC_API__ __declspec(dllimport)
#endif
