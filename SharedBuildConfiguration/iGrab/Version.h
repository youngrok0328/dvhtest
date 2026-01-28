#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#if defined(FRAMEGRABBER__IGRAB__5_1_4)
constexpr unsigned int k_iGrabLibraryVersion = 0x00050104;
#elif defined(FRAMEGRABBER__IGRAB__5_1_7)
constexpr unsigned int k_iGrabLibraryVersion = 0x00050107;
#elif defined(FRAMEGRABBER__IGRAB__5_1_9)
constexpr unsigned int k_iGrabLibraryVersion = 0x00050109;
#elif defined(FRAMEGRABBER__IGRAB__5_1_10)
constexpr unsigned int k_iGrabLibraryVersion = 0x00050110;
#else
#error "Undefined iGrab Library Version"
#endif
