#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Enum.h"
#include "ParaAlignment.hpp"
#include "ParaCarrierTapeAlign.h"
#include "ParaPocketAlign.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h" // Stitch Section Image Combine
#include "../../VisionNeedLibrary/VisionCommon/VisionInspFrameIndex.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class ImageProcPara;
class VisionProcessing;
class VisionScale;

//HDR_6_________________________________ Header body
//
class Para
{
public:
    Para(VisionProcessing& parent);
    ~Para() = default;

    //------------------------------------------------------------------
    // 저장되지 않고 Detail Setup을 위해 존재하는 값들
    //------------------------------------------------------------------
    bool m_skipEdgeAlign = false;

    //------------------------------------------------------------------

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Init();
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    ParaAlignment<ParaCarrierTapeAlign> m_carrierTapeAlign;
    ParaAlignment<ParaPocketAlign> m_pocketAlign;
};
