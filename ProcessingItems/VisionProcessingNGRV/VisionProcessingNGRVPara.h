#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point8u3.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class VisionProcessingNGRVPara
{
public:
    VisionProcessingNGRVPara(VisionProcessing& parent);
    ~VisionProcessingNGRVPara(void);

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    long m_defectNumber;

    std::vector<CString> m_vecItemID;
    std::vector<CString> m_vecDefectName;
    std::vector<CString> m_vecDefectCode;
    std::vector<long> m_vecDisposition;
    std::vector<long> m_vecDecision;
    std::vector<Ipvm::Point32s2> m_vecStitchCount;
    std::vector<long> m_vecAFtype;
    std::vector<float> m_vecGrabHeightOffset_um;
    std::vector<std::vector<BOOL>> m_vec2IllumUsage;
    std::vector<Ipvm::Rect32s> m_vecGrabPosROI;

    ImageProcPara m_ImageProcMangePara;

    /*long m_mainImageSizeX;
    long m_mainImageSizeY;

    Ipvm::Point32r2 m_mainImageScale;

    Ipvm::Image8u m_imageColor[3];  

    Ipvm::Image8u3 m_imageReference;
    Ipvm::Point8u3 m_imageReferencePoints;*/

private:
    void SaveVec2_FRect(BOOL bSave, CString strBaseKeyName, CiDataBase& db,
        std::vector<std::vector<Ipvm::Rect32r>>& m_vecrtInspectionROI);
    void SaveVec2_String(
        BOOL bSave, CString strBaseKeyName, CiDataBase& db, std::vector<std::vector<CString>>& vecstrinfoName);
    void SaveVec3_Long(BOOL bSave, CString strBaseKeyName, CiDataBase& db,
        std::vector<std::vector<std::vector<long>>>& vecInfoParameter);
    Ipvm::Rect32s ConvertGrabROI_Position_Limit_imageSize(
        const Ipvm::Rect32s& srcROI, const long& imageSizeX, const long& imageSizeY);

private:
    long m_imageSizeX;
    long m_imageSizeY;

public:
    void SetNGRV_SingleRun_RefimageSize(const long& imageSizeX, const long& imageSizeY);
};
