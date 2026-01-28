#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../VisionCommon/ImageProcPara.h"
#include "../VisionCommon/VisionInspFrameIndex.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class ImageProcPara;
class VisionProcessing;
class VisionScale;

//HDR_6_________________________________ Header body
//
namespace VisionEdgeAlign
{
__VISION_EDGE_ALIGN_CLASS LPCTSTR GetDirectionName(long direction);

class __VISION_EDGE_ALIGN_CLASS EdgeSearchRoiPara
{
public:
    EdgeSearchRoiPara();

    void Initialize();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    Ipvm::Rect32s ToRect(const VisionScale& scale, long direction, const Ipvm::Point32r2& paneCenter_px,
        float packageSizeX_px, float packageSizeY_px) const;

    float m_searchLength_um; // Search Length
    float m_persent_start; // 시작점
    float m_persent_end; // 끝점
};

class __VISION_EDGE_ALIGN_CLASS Para
{
public:
    Para(VisionProcessing& parent);
    ~Para();

    BOOL LinkDataBase(BOOL bSave, float packageSizeX_um, float packageSizeY_um, CiDataBase& db);

    void Init();

    //EDGEe Align Para
    VisionInspFrameIndex m_alignImageFrameIndex;
    long m_nEdgeDirection;
    long m_nSearchDirection;
    long m_nEdgeDetectMode;
    float m_fFirstEdgeMinThreshold;

    long m_nRoughAlignEdgeLength;

    float m_fEdgeSamplerate;
    float m_fEdgeAlignGap_um;

    long m_nLowTopCount;

    //kircheis_USI :: ROI 변수들을 RearAux로 해서 Low Top Dist를 뺀 나머지를 넣자
    //						 :: 갯수는 동일하게 가져가고 각기 다른 위치/크기를 저장 가능 하도록....
    //						 :: Ignore Length로 넣어서 Align 검증할 떄 참조하게 하자 (Front Cam Ignore Length/Rear Cam Ignore Length
    std::vector<EdgeSearchRoiPara> m_searchPara[4]; // UP, DOWN, LEFT, RIGHT
    std::vector<Ipvm::Rect32r> m_vecrtLowTopSearchROI_BCU;

    ImageProcPara m_ImageProcMangePara;

    BOOL m_emptyInsp;
    VisionInspFrameIndex m_emptyFrameIndex;
    float m_2D_emptyMinBlobSize_umSqure;
    long m_2D_emptyMinBlobCount;
    long m_2D_emptyThresholdValue;
    Ipvm::Rect32r m_2D_emptyBlobROI_BCU;
    BOOL m_2D_emptyReverseThreshold;
    long m_2D_emptyIgnoreROINum;
    std::vector<Ipvm::Rect32r> m_2D_emptyIgnoreROIs_BCU;
    BOOL m_2D_emptyUseFilterBlobFullROI;

    BOOL m_3D_doubleInsp;
    float m_3D_heightOfTrayWallFromMold;
    float m_3D_emptyCheckMoldAreaRatioSpec;

    //{{//kircheis_NGRVAF
    BOOL m_bIsValidPlaneRefInfo;
    long m_nNGRVAF_FrameID;
    std::vector<Ipvm::Point32s2> m_vecptRefPos_UM;
    long m_nAFImgSizeX;
    long m_nAFImgSizeY;
    std::vector<BYTE>
        m_vecbyAFImage; //원래 BYTE*로 만들려 했으나 메모리 관리의 문제로 인해 vector화. 필요시 BYTE*로 변환 사용할 것
    //}}

    long m_nNGRVRefPosCount;
    std::vector<Ipvm::Rect32s> m_vecrtNGRVRefPosROI;

    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe
};
} // namespace VisionEdgeAlign
