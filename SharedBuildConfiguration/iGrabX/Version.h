#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABX__2_1_7)
constexpr unsigned int k_iGrabXLibraryVersion = 0x00020107;
#define FRAMEGRABBER__IGRABX__VERSION_STRING _T("2.1.7")
#elif defined(FRAMEGRABBER__IGRABX__2_1_10)
constexpr unsigned int k_iGrabXLibraryVersion = 0x00020110;
#define FRAMEGRABBER__IGRABX__VERSION_STRING _T("2.1.10")
#elif defined(FRAMEGRABBER__IGRABX__3_1_3)
constexpr unsigned int k_iGrabXLibraryVersion = 0x00030103;
#define FRAMEGRABBER__IGRABX__VERSION_STRING _T("3.1.3")
#elif defined(FRAMEGRABBER__IGRABX__3_1_5)
constexpr unsigned int k_iGrabXLibraryVersion = 0x00030105;
#define FRAMEGRABBER__IGRABX__VERSION_STRING _T("3.1.5")
#elif defined(FRAMEGRABBER__IGRABX__3_1_6)
constexpr unsigned int k_iGrabXLibraryVersion = 0x00030106;
#define FRAMEGRABBER__IGRABX__VERSION_STRING _T("3.1.6")
#else
#error "Undefined iGrabX Library Version"
#endif
