#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#if defined(FRAMEGRABBER__IGRABXQ__1_0_3)
#include <C:/Program Files/IntekPlus/iGrabXQ v1.0.3/include/iFsCtrlCmd.h>
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_1)
#include <C:/Program Files/IntekPlus/iGrabXQ v1.1.1/include/iFsCtrlCmd.h>
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_7)
#include <C:/Program Files/IntekPlus/iGrabXQ v1.1.7/include/iFsCtrlCmd.h>
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_8)
#include <C:/Program Files/IntekPlus/iGrabXQ v1.1.8/include/iFsCtrlCmd.h>
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_9)
#include <C:/Program Files/IntekPlus/iGrabXQ v1.1.9/include/iFsCtrlCmd.h>
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_10)
#include <C:/Program Files/IntekPlus/iGrabXQ v1.1.10/include/iFsCtrlCmd.h>
#else
#error "Undefined iGrabXQ Library Version"
#endif

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABXQ__1_0_3)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXQ v1.0.3/lib/iGrabXCtrlCmd64.lib")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_1)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXQ v1.1.1/lib/iGrabXCtrlCmd64.lib")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_7)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXQ v1.1.7/lib/iGrabXCtrlCmd64.lib")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_8)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXQ v1.1.8/lib/iGrabXCtrlCmd64.lib")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_9)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXQ v1.1.9/lib/iGrabXCtrlCmd64.lib")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_10)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabXQ v1.1.10/lib/iGrabXCtrlCmd64.lib")
#else
#error "Undefined iGrabXQ Library Version"
#endif
