#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABXQ__1_0_3)
constexpr unsigned int k_iGrabXQLibraryVersion = 0x00010003;
#define FRAMEGRABBER__IGRABXQ__VERSION_STRING _T("1.0.3")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_1)
constexpr unsigned int k_iGrabXQLibraryVersion = 0x00010101;
#define FRAMEGRABBER__IGRABXQ__VERSION_STRING _T("1.1.1")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_7)
constexpr unsigned int k_iGrabXQLibraryVersion = 0x00010107;
#define FRAMEGRABBER__IGRABXQ__VERSION_STRING _T("1.1.7")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_8)
constexpr unsigned int k_iGrabXQLibraryVersion = 0x00010108;
#define FRAMEGRABBER__IGRABXQ__VERSION_STRING _T("1.1.8")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_9)
constexpr unsigned int k_iGrabXQLibraryVersion = 0x00010109;
#define FRAMEGRABBER__IGRABXQ__VERSION_STRING _T("1.1.9")
#elif defined(FRAMEGRABBER__IGRABXQ__1_1_10)
constexpr unsigned int k_iGrabXQLibraryVersion = 0x00010110;
#define FRAMEGRABBER__IGRABXQ__VERSION_STRING _T("1.1.10")
#else
#error "Undefined iGrabXQ Library Version"
#endif
