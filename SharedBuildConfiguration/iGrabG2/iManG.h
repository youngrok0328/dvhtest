#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#if defined(FRAMEGRABBER__IGRABG2__2_0_2)
#include <C:/Program Files/IntekPlus/iGrabG2 v2.0.2/include/iManG.h>
#elif defined(FRAMEGRABBER__IGRABG2__2_0_3)
#include <C:/Program Files/IntekPlus/iGrabG2 v2.0.3/include/iManG.h>
#elif defined(FRAMEGRABBER__IGRABG2__2_1_1)
#include <C:/Program Files/IntekPlus/iGrabG2 v2.1.1/include/iManG.h>
#elif defined(FRAMEGRABBER__IGRABG2__2_1_2)
#include <C:/Program Files/IntekPlus/iGrabG2 v2.1.2/include/iManG.h>
#elif defined(FRAMEGRABBER__IGRABG2__2_1_3)
#include <C:/Program Files/IntekPlus/iGrabG2 v2.1.3/include/iManG.h>
#elif defined(FRAMEGRABBER__IGRABG2__2_1_5)
#include <C:/Program Files/IntekPlus/iGrabG2 v2.1.5/include/iManG.h>
#else
#error "Undefined iGrabG2 Library Version"
#endif

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABG2__2_0_2)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabG2 v2.0.2/lib/iManG64.lib")
#elif defined(FRAMEGRABBER__IGRABG2__2_0_3)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabG2 v2.0.3/lib/iManG64.lib")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_1)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabG2 v2.1.1/lib/iManG64.lib")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_2)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabG2 v2.1.2/lib/iManG64.lib")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_3)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabG2 v2.1.3/lib/iManG64.lib")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_5)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabG2 v2.1.5/lib/iManG64.lib")
#else
#error "Undefined iGrabG2 Library Version"
#endif
