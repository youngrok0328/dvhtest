#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRABXLITE__2_1_7)
constexpr unsigned int k_iGrabXLiteLibraryVersion = 0x00020107;
#elif defined(FRAMEGRABBER__IGRABXLITE__2_1_8)
constexpr unsigned int k_iGrabXLiteLibraryVersion = 0x00020108;
#else
#error "Undefined iGrabXLite Library Version"
#endif
