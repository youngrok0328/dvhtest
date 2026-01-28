#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"
 
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//

enum enumSealingBoundaryType
{
    enumSealingBoundaryType_Start = 0,
    enumSealingBoundaryType_Inner = enumSealingBoundaryType_Start,
    enumSealingBoundaryType_Outer,
    enumSealingBoundaryType_End,
};

enum enumEdgeObjectType
{
    enumEdgeObjectType_Start = 0,
    enumEdgeObjectType_CoverTape = enumEdgeObjectType_Start,
    enumEdgeObjectType_LeftSealing,
    enumEdgeObjectType_RightSealing,
    enumEdgeObjectType_End,
};

class VisionInspectionOTITRPara
{
public:
    VisionInspectionOTITRPara(VisionProcessing& parent);
    ~VisionInspectionOTITRPara(void);

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    VisionInspFrameIndex m_coverTapeImageFrameIndex;
    ImageProcPara m_coverTapeImageProc;
    long m_coverTapeEdgeDetectMode = EdgeDetectMode_BestEdge;
    double m_coverTapeEdgeThreshold = 10.f;
    long m_coverTapeAlignSearchLength_um = 500;
    long m_coverTapeDetectGap_um = 100;


    VisionInspFrameIndex m_sealingImageFrameIndex;
    ImageProcPara m_sealingImageProc;
    long m_sealingEdgeDetectMode = EdgeDetectMode_BestEdge;
    double m_sealingEdgeThreshold = 10.f;
    long m_sealingAlignSearchLength_um = 500;
    long m_sealingDetectGap_um = 100;
    long m_sealingBoundaryType = enumSealingBoundaryType_Outer;
};
