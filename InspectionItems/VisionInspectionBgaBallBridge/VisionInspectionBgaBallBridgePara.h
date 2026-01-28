#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class VisionInspectionBgaBallBridgePara
{
public:
    VisionInspectionBgaBallBridgePara(VisionProcessing& parent);
    ~VisionInspectionBgaBallBridgePara(void);

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    ImageProcPara m_ImageProcManagePara;
    double m_fBallLinkRangeRatio;
    double m_fBallIgnoreRatio;
    double m_fROI_WidthRatio;
    long m_nThresholdValue;

    // 2nd Inspection
    BOOL m_use2ndInspection; // 2nd 검사 사용 유무 (딥러닝)
    CString m_str2ndInspCode; // 딥러닝 모델과 매칭을 위한 코드
};
