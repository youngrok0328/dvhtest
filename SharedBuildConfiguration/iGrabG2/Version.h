#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABG2__2_0_2)
constexpr unsigned int k_iGrabG2LibraryVersion = 0x00020002;
#define FRAMEGRABBER__IGRABG2__VERSION_STRING _T("2.0.2")
#elif defined(FRAMEGRABBER__IGRABG2__2_0_3)
constexpr unsigned int k_iGrabG2LibraryVersion = 0x00020003;
#define FRAMEGRABBER__IGRABG2__VERSION_STRING _T("2.0.3")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_1)
constexpr unsigned int k_iGrabG2LibraryVersion = 0x00020101;
#define FRAMEGRABBER__IGRABG2__VERSION_STRING _T("2.1.1")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_2)
constexpr unsigned int k_iGrabG2LibraryVersion = 0x00020102;
#define FRAMEGRABBER__IGRABG2__VERSION_STRING _T("2.1.2")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_3)
constexpr unsigned int k_iGrabG2LibraryVersion = 0x00020103;
#define FRAMEGRABBER__IGRABG2__VERSION_STRING _T("2.1.3")
#elif defined(FRAMEGRABBER__IGRABG2__2_1_5)
constexpr unsigned int k_iGrabG2LibraryVersion = 0x00020105;
#define FRAMEGRABBER__IGRABG2__VERSION_STRING _T("2.1.5")
#else
#error "Undefined iGrabG2 Library Version"
#endif
