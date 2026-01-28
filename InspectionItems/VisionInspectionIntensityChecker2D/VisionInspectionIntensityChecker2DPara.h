#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class VisionInspectionIntensityChecker2DPara
{
public:
    VisionInspectionIntensityChecker2DPara(VisionProcessing& parent);
    ~VisionInspectionIntensityChecker2DPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    void Init();

    long m_nROI_IntensityAlgorithmMode;

    long m_Max_ROI_Num;
    long m_Max_Frame_Num;

    std::vector<std::vector<long>> m_vec2nROIFrameNum;
    std::vector<Ipvm::Rect32s> m_vecROI;

    VisionInspFrameIndex m_calcFrameIndex;
};
