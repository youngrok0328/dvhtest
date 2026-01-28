#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SurfaceCustomROI.h"
#include "SurfaceCustomROIItem.h"
#include "SurfaceItem.h"

//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class VisionInspectionSurfacePara
{
public:
    VisionInspectionSurfacePara(VisionProcessing& parent);
    ~VisionInspectionSurfacePara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db, LPCTSTR moduleName);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe
    double GetCriteriaValue(long CriteriaIdx, double i_InputValue);

    CSurfaceItem m_SurfaceItem;
    CSurfaceCustomROI m_SurfaceCutemRoi;
    ImageProcPara m_ImageProcMangePara;
    Ipvm::Image8u m_SurfaceBitmapMask;
};
