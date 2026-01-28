#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//

enum eGeometryInspType //mc_Side Vision의 경우 선택할 수 있도록 한다, Default는 Both
{
    GeometryInspType_Both,
    GeometryInspType_Front_Vision,
    GeometryInspType_Rear_Vision,
};

class VisionInspectionGeometryPara
{
public:
    VisionInspectionGeometryPara(VisionProcessing& parent);
    ~VisionInspectionGeometryPara(void);

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    long m_nInspectionNumber;
    long m_nGeometryInspType;

    std::vector<CString> m_vecstrInspectionName;
    std::vector<long> m_vecnInspectionType;
    std::vector<long> m_vecnInspectionDistanceResult;
    std::vector<long> m_vecnInspectionCircleResult;
    std::vector<float> m_vecfReferenceSpec;

    // 0번 파라미터
    std::vector<std::vector<std::vector<long>>> m_vecInfoParameter_0_Ref;
    std::vector<std::vector<std::vector<long>>> m_vecInfoParameter_0_Tar;

    // 1번 파라미터
    std::vector<std::vector<std::vector<long>>> m_vecInfoParameter_1_Ref;
    std::vector<std::vector<std::vector<long>>> m_vecInfoParameter_1_Tar;

    // 2번 파라미터
    std::vector<std::vector<std::vector<long>>> m_vecInfoParameter_2_Ref;
    std::vector<std::vector<std::vector<long>>> m_vecInfoParameter_2_Tar;

    std::vector<std::vector<CString>> m_vecstrinfoName_Ref;
    std::vector<std::vector<CString>> m_vecstrinfoName_Tar;

    std::vector<std::vector<Ipvm::Rect32r>> m_vecrtInspectionROI_Ref_BCU;
    std::vector<std::vector<Ipvm::Rect32r>> m_vecrtInspectionROI_Tar_BCU;

    ImageProcPara m_ImageProcMangePara;

private:
    void SaveVec2_FRect(BOOL bSave, CString strBaseKeyName, CiDataBase& db,
        std::vector<std::vector<Ipvm::Rect32r>>& m_vecrtInspectionROI);
    void SaveVec2_String(
        BOOL bSave, CString strBaseKeyName, CiDataBase& db, std::vector<std::vector<CString>>& vecstrinfoName);
    void SaveVec3_Long(BOOL bSave, CString strBaseKeyName, CiDataBase& db,
        std::vector<std::vector<std::vector<long>>>& vecInfoParameter);
};
