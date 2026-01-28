#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#if defined(FRAMEGRABBER__IGRABX__2_1_7)
#include <C:/Program Files/IntekPlus/iGrabX v2.1.7/include/iManX.h>
#elif defined(FRAMEGRABBER__IGRABX__2_1_10)
#include <C:/Program Files/IntekPlus/iGrabX v2.1.10/include/iManX.h>
#elif defined(FRAMEGRABBER__IGRABX__3_1_3)
#include <C:/Program Files/IntekPlus/iGrabX v3.1.3/include/iManX.h>
#elif defined(FRAMEGRABBER__IGRABX__3_1_5)
#include <C:/Program Files/IntekPlus/iGrabX v3.1.5/include/iManX.h>
#elif defined(FRAMEGRABBER__IGRABX__3_1_6)
#include <C:/Program Files/IntekPlus/iGrabX v3.1.6/include/iManX.h>
#else
#error "Undefined iGrabX Library Version"
#endif

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABX__2_1_7)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabX v2.1.7/lib/iManX64.lib")
#elif defined(FRAMEGRABBER__IGRABX__2_1_10)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabX v2.1.10/lib/iManX64.lib")
#elif defined(FRAMEGRABBER__IGRABX__3_1_3)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabX v3.1.3/lib/iManX64.lib")
#elif defined(FRAMEGRABBER__IGRABX__3_1_5)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabX v3.1.5/lib/iManX64.lib")
#elif defined(FRAMEGRABBER__IGRABX__3_1_6)
#pragma comment(lib, "C:/Program Files/IntekPlus/iGrabX v3.1.6/lib/iManX64.lib")
#else
#error "Undefined iGrabX Library Version"
#endif
