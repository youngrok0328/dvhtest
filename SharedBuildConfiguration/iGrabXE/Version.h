#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABXE__1_0_6)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010006;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.0.6")
#elif defined(FRAMEGRABBER__IGRABXE__1_0_7)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010007;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.0.7")
#elif defined(FRAMEGRABBER__IGRABXE__1_1_1)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010101;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.1.1")
#elif defined(FRAMEGRABBER__IGRABXE__1_1_2)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010102;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.1.2")
#elif defined(FRAMEGRABBER__IGRABXE__1_4_1)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010401;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.4.1")
#elif defined(FRAMEGRABBER__IGRABXE__1_4_2)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010402;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.4.2")
#elif defined(FRAMEGRABBER__IGRABXE__1_4_7)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010407;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.4.7")
#elif defined(FRAMEGRABBER__IGRABXE__1_4_8)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010408;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.4.8")
#elif defined(FRAMEGRABBER__IGRABXE__1_4_9)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00010409;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("1.4.9")
#elif defined(FRAMEGRABBER__IGRABXE__99_4_6)
constexpr unsigned int k_iGrabXELibraryVersion = 0x00990406;
#define FRAMEGRABBER__IGRABXE__VERSION_STRING _T("99.4.6")
#else
#error "Undefined iGrabXE Library Version"
#endif
