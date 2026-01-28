#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class IllumInfo2D;
class VisionProcessing;
class VisionReusableMemory;
class ImageProcPara;

//HDR_6_________________________________ Header body
//
INT_PTR IMAGE_COMBINE_MODULE Combine_SetParameter(
    VisionProcessing& proc, long baseFrameIndex, bool isRaw, ImageProcPara* pProcManagePara, Ipvm::Rect32s rtPaneROI);
INT_PTR IMAGE_COMBINE_MODULE Combine_SetParameter(
    VisionProcessing& proc, long baseFrameIndex, bool isRaw, ImageProcPara* pProcManagePara);