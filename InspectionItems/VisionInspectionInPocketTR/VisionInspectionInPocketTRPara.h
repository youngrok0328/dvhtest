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
class VisionInspectionInPocketTRPara
{
public:
    VisionInspectionInPocketTRPara(VisionProcessing& parent);
    ~VisionInspectionInPocketTRPara(void);

    void Init();

    ImageProcPara m_ImageProcManageParaMisPlace;
    VisionInspFrameIndex m_calcFrameIndexMisPlace;
    ImageProcPara m_ImageProcManageParaOrientation;
    VisionInspFrameIndex m_calcFrameIndexOrientation;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe
};
