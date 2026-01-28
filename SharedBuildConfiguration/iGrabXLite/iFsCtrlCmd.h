#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#if defined(FRAMEGRABBER__IGRABXLITE__2_1_7)
#include <C:/Program Files/IntekPlus/iGrabXLite v2.1.7/include/iFsCtrlCmd.h>
#elif defined(FRAMEGRABBER__IGRABXLITE__2_1_8)
#include <C:/Program Files/IntekPlus/iGrabXLite v2.1.8/include/iFsCtrlCmd.h>
#else
#error "Undefined iGrabXLite Library Version"
#endif

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABXLITE__2_1_7)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXLite v2.1.7/lib/iGrabXLiteCtrlCmd64.lib")
#elif defined(FRAMEGRABBER__IGRABXLITE__2_1_8)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXLite v2.1.8/lib/iGrabXLiteCtrlCmd64.lib")
#else
#error "Undefined iGrabXLite Library Version"
#endif
