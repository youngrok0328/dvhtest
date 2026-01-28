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
class VisionInspectionDie2DPara
{
public:
    VisionInspectionDie2DPara(VisionProcessing& parent);
    ~VisionInspectionDie2DPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    ImageProcPara m_ImageProcManagePara;

    long m_nEdgeDirection;
    long m_nSearchDirection;
    long m_nEdgeDetectMode;
    long m_nFirstEdgeValue;
    long m_nEdgeSearchWidthRatio;
    long m_nEdgeSearchLength_um;
    long m_nEdgeDetectGap_um;
};
