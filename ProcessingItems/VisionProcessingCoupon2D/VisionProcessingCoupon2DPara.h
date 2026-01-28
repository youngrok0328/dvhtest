#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class VisionProcessingCoupon2DPara
{
public:
    VisionProcessingCoupon2DPara(VisionProcessing& parent);
    ~VisionProcessingCoupon2DPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Init();

    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    ImageProcPara m_ImageProcManagePara;
    VisionInspFrameIndex m_calcFrameIndex;

    long m_nCouponThreshold;
    Ipvm::Rect32s m_CouponROI;

    Ipvm::Image8u m_imageInsp;

    long m_nMinBlobSize;
    long m_nMinBlobCount;
};