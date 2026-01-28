#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
struct SInterposerRefROI_Info
{ //단위는 그때그때 다르게 사용한다. 파일을 읽은 넘을 저장하게나 Job으로 관리할 때 단위는 um. Job Open후 실제 계산에서 사용할 때는 pxl로 쓰자.
    float fOffsetX;
    float fOffsetY;
    float fWidth;
    float fLength;
};

class VisionInspectionPatch3DPara
{
public:
    VisionInspectionPatch3DPara(VisionProcessing& parent);
    ~VisionInspectionPatch3DPara(void);

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

    long m_nPatchHeightROI_size_um;
    long m_nPatchHeightROI_OffsetX_um;
    long m_nPatchHeightROI_OffsetY_um;

    std::vector<SInterposerRefROI_Info> m_vecsInterposerRefROI_Info_um; //csv로 읽거나 Job에 저장할 땐 얘를 써라

    void ReadInterposerRefInfoFromCSV(CString strFileFullPath);
    void WriteInterposerRefInfoToCSV(CString strFileFullPath);
};
