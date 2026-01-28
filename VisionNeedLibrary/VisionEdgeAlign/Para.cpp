//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Para.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h" //kircheis_NGRVAF
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
using namespace VisionEdgeAlign;

LPCTSTR VisionEdgeAlign::GetDirectionName(long direction)
{
    switch (direction)
    {
        case UP:
            return _T("Top");
        case DOWN:
            return _T("Bottom");
        case LEFT:
            return _T("Left");
        case RIGHT:
            return _T("Right");
    }

    ASSERT(!_T("??"));
    return _T("");
}

EdgeSearchRoiPara::EdgeSearchRoiPara()
    : m_searchLength_um(500.f)
    , m_persent_start(0.f)
    , m_persent_end(100.f)
{
}

void EdgeSearchRoiPara::Initialize()
{
    m_searchLength_um = 500.f;
    m_persent_start = 0.f;
    m_persent_end = 100.f;
}

BOOL EdgeSearchRoiPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("Search Length")].Link(bSave, m_searchLength_um);
    db[_T("Percent Start")].Link(bSave, m_persent_start);
    db[_T("Percent End")].Link(bSave, m_persent_end);

    return TRUE;
}

Ipvm::Rect32s EdgeSearchRoiPara::ToRect(const VisionScale& scale, long direction, const Ipvm::Point32r2& paneCenter_px,
    float packageSizeX_px, float packageSizeY_px) const
{
    Ipvm::Rect32s roi(0, 0, 0, 0);

    switch (direction)
    {
        case LEFT:
            if (true)
            {
                float midX = paneCenter_px.m_x - packageSizeX_px * 0.5f;
                float start = paneCenter_px.m_y - packageSizeY_px * 0.5f + packageSizeY_px * m_persent_start / 100.f;
                float end = paneCenter_px.m_y - packageSizeY_px * 0.5f + packageSizeY_px * m_persent_end / 100.f;

                roi.m_left = CAST_INT32T(midX - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_right = CAST_INT32T(midX + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_top = (long)min(start, end);
                roi.m_bottom = (long)max(start, end);
            }
            break;

        case RIGHT:
            if (true)
            {
                float midX = paneCenter_px.m_x + packageSizeX_px * 0.5f;
                float start = paneCenter_px.m_y - packageSizeY_px * 0.5f + packageSizeY_px * m_persent_start / 100.f;
                float end = paneCenter_px.m_y - packageSizeY_px * 0.5f + packageSizeY_px * m_persent_end / 100.f;

                roi.m_left = CAST_INT32T(midX - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_right = CAST_INT32T(midX + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_top = (long)min(start, end);
                roi.m_bottom = (long)max(start, end);
            }
            break;

        case UP:
            if (true)
            {
                float midY = paneCenter_px.m_y - packageSizeY_px * 0.5f;
                float start = paneCenter_px.m_x - packageSizeX_px * 0.5f + packageSizeX_px * m_persent_start / 100.f;
                float end = paneCenter_px.m_x - packageSizeX_px * 0.5f + packageSizeX_px * m_persent_end / 100.f;

                roi.m_top = CAST_INT32T(midY - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_bottom = CAST_INT32T(midY + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_left = (long)min(start, end);
                roi.m_right = (long)max(start, end);
            }
            break;

        case DOWN:
            if (true)
            {
                float midY = paneCenter_px.m_y + packageSizeY_px * 0.5f;
                float start = paneCenter_px.m_x - packageSizeX_px * 0.5f + packageSizeX_px * m_persent_start / 100.f;
                float end = paneCenter_px.m_x - packageSizeX_px * 0.5f + packageSizeX_px * m_persent_end / 100.f;

                roi.m_top = CAST_INT32T(midY - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_bottom = CAST_INT32T(midY + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_left = (long)min(start, end);
                roi.m_right = (long)max(start, end);
            }
            break;
    }

    return roi;
}

Para::Para(VisionProcessing& parent)
    : m_alignImageFrameIndex(parent, FALSE)
    , m_emptyFrameIndex(parent, FALSE)
    , m_ImageProcMangePara(parent)
{
    Init();
}

Para::~Para()
{
}

BOOL Para::LinkDataBase(BOOL bSave, float packageSizeX_um, float packageSizeY_um, CiDataBase& db)
{
    CString strTemp;
    long nSize(0);

    if (!bSave)
    {
        Init();
    }

    long version = 2;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    //{{ //kircheis_3DEmpty
    if (!db[_T("Empty Check")].Link(bSave, m_emptyInsp))
        m_emptyInsp = FALSE;

    db[_T("{CB42C73E-2EA8-4085-954D-D45055736713}")].Link(bSave, m_2D_emptyThresholdValue);
    db[_T("{669346C1-AE0C-4544-801B-8DB15319A1D7}")].Link(bSave, m_2D_emptyMinBlobCount);
    db[_T("{9FAA88D2-24AB-4478-B603-3C6F6AFF6C93}")].Link(bSave, m_2D_emptyReverseThreshold);
    db[_T("{6D0A4DF7-342A-4492-9A77-5362ECB2D3C8}")].Link(bSave, m_2D_emptyIgnoreROINum);
    db[_T("{FA65BD09-5004-4F54-B761-558FB03929C1}")].Link(bSave, m_2D_emptyUseFilterBlobFullROI);

    if (!bSave)
    {
        m_2D_emptyIgnoreROIs_BCU.resize(m_2D_emptyIgnoreROINum);
    }

    if (version == 0)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }
    else
    {
        db[_T("{85769699-6A51-4607-8E34-604544CC5480}")].Link(bSave, m_2D_emptyMinBlobSize_umSqure);
        LinkEx(bSave, db[_T("{9754BE9F-AB37-47E5-92A8-A31A4906BCDF}")], m_2D_emptyBlobROI_BCU);

        for (long n = 0; n < m_2D_emptyIgnoreROINum; n++)
        {
            strTemp.Format(_T("{1833DCB9-620F-465C-8603-A239849DB4F7}_%d"), n);
            if (!LinkEx(bSave, db[strTemp], m_2D_emptyIgnoreROIs_BCU[n]))
            {
                m_2D_emptyIgnoreROIs_BCU[n] = Ipvm::Rect32r{};
            }
        }
    }

    if (!db[_T("Double Check")].Link(bSave, m_3D_doubleInsp))
        m_3D_doubleInsp = FALSE;
    if (!db[_T("Height Of Tray Wall From Mold")].Link(bSave, m_3D_heightOfTrayWallFromMold))
        m_3D_heightOfTrayWallFromMold = 0.f;
    if (!db[_T("Empty Check Mold Area Ratio Spec")].Link(bSave, m_3D_emptyCheckMoldAreaRatioSpec))
        m_3D_emptyCheckMoldAreaRatioSpec = 0.45f;

    // 2D Frame Index
    {
        if (!m_alignImageFrameIndex.LinkDataBase(bSave, db[_T("Edge_AlignImageFrmaeIndex")]))
            m_alignImageFrameIndex.setFrameListIndex(0);
        if (!m_emptyFrameIndex.LinkDataBase(bSave, db[_T("{6D117A4E-6642-47A2-A0E1-64B7184A2D70}")]))
            m_emptyFrameIndex.setFrameListIndex(0);
    }

    // Edge Detect Parameter
    if (!db[_T("Edge_Direction")].Link(bSave, m_nEdgeDirection))
        m_nEdgeDirection = 0;
    if (!db[_T("Search_Direction")].Link(bSave, m_nSearchDirection))
        m_nSearchDirection = 0;
    if (!db[_T("Edge_Detect_Mode")].Link(bSave, m_nEdgeDetectMode))
        m_nEdgeDetectMode = EdgeDetectMode_BestEdge;
    if (!db[_T("RoughAlign_Edge_Length")].Link(bSave, m_nRoughAlignEdgeLength))
        m_nRoughAlignEdgeLength = 10;
    //if (!db[_T("RoughAlign_Edge_Samplerate")].Link(bSave, m_fEdgeSamplerate)) m_fEdgeSamplerate = 100.f;
    if (!db[_T("RoughAlign_Edge_Gap")].Link(bSave, m_fEdgeAlignGap_um))
        m_fEdgeAlignGap_um = 40.f;
    if (!db[_T("m_fFirstEdgeMinThreshold")].Link(bSave, m_fFirstEdgeMinThreshold))
        m_fFirstEdgeMinThreshold = 10.f;

    // Edge Detect ROI Parameter
    long count_l = (long)m_searchPara[LEFT].size();
    long count_t = (long)m_searchPara[UP].size();
    long count_r = (long)m_searchPara[RIGHT].size();
    long count_b = (long)m_searchPara[DOWN].size();

    if (!db[_T("Left_Edge_Detect_ROI_Num")].Link(bSave, count_l))
        count_l = 1;
    if (!db[_T("Top_Edge_Detect_ROI_Num")].Link(bSave, count_t))
        count_t = 1;
    if (!db[_T("Right_Edge_Detect_ROI_Num")].Link(bSave, count_r))
        count_r = 1;
    if (!db[_T("Bottom_Edge_Detect_ROI_Num")].Link(bSave, count_b))
        count_b = 1;
    if (!db[_T("LowTop_Edge_Detect_ROI_Num")].Link(bSave, m_nLowTopCount))
        m_nLowTopCount = 0;

    // Edge Detect 갯수 저장
    if (!bSave)
    {
        m_searchPara[LEFT].clear();
        m_searchPara[UP].clear();
        m_searchPara[RIGHT].clear();
        m_searchPara[DOWN].clear();

        m_searchPara[LEFT].resize(count_l);
        m_searchPara[UP].resize(count_t);
        m_searchPara[RIGHT].resize(count_r);
        m_searchPara[DOWN].resize(count_b);

        if (!db[_T("LowTop_Edge_Detect_ROI_Size")].Link(bSave, nSize))
            nSize = 0;
        m_vecrtLowTopSearchROI_BCU.clear();
        m_vecrtLowTopSearchROI_BCU.resize(nSize);
    }
    else
    {
        nSize = (long)(m_vecrtLowTopSearchROI_BCU.size());
        if (!db[_T("LowTop_Edge_Detect_ROI_Size")].Link(bSave, nSize))
            nSize = 0;
    }

    if (packageSizeX_um > 0.f && packageSizeY_um > 0.f)
    {
        Ipvm::Rect32r tempRoi{};
        Ipvm::Rect32r tempfRoi{};

        switch (version)
        {
            case 0:
                // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
                return FALSE;

            case 1:
                // m_searchPara[LEFT] 저장
                for (long i = 0; i < long(m_searchPara[LEFT].size()); i++)
                {
                    auto& sPara = m_searchPara[LEFT][i];
                    strTemp.Format(_T("Left_Edge_Detect_ROI_%d"), i);
                    if (LinkEx(bSave, db[strTemp], tempfRoi))
                    {
                        sPara.m_searchLength_um = tempfRoi.Width();
                        sPara.m_persent_start = (tempfRoi.m_top + packageSizeY_um * 0.5f) / packageSizeY_um * 100.f;
                        sPara.m_persent_end = (tempfRoi.m_bottom + packageSizeY_um * 0.5f) / packageSizeY_um * 100.f;
                    }
                }

                // m_searchPara[UP] 저장
                for (long i = 0; i < long(m_searchPara[UP].size()); i++)
                {
                    auto& sPara = m_searchPara[UP][i];
                    strTemp.Format(_T("Top_Edge_Detect_ROI_%d"), i);
                    if (LinkEx(bSave, db[strTemp], tempfRoi))
                    {
                        sPara.m_searchLength_um = tempfRoi.Height();
                        sPara.m_persent_start = (tempfRoi.m_left + packageSizeX_um * 0.5f) / packageSizeX_um * 100.f;
                        sPara.m_persent_end = (tempfRoi.m_right + packageSizeX_um * 0.5f) / packageSizeX_um * 100.f;
                    }
                }

                // m_searchPara[RIGHT] 저장
                for (long i = 0; i < long(m_searchPara[RIGHT].size()); i++)
                {
                    auto& sPara = m_searchPara[RIGHT][i];
                    strTemp.Format(_T("Right_Edge_Detect_ROI_%d"), i);
                    if (LinkEx(bSave, db[strTemp], tempfRoi))
                    {
                        sPara.m_searchLength_um = tempfRoi.Width();
                        sPara.m_persent_start = (tempfRoi.m_top + packageSizeY_um * 0.5f) / packageSizeY_um * 100.f;
                        sPara.m_persent_end = (tempfRoi.m_bottom + packageSizeY_um * 0.5f) / packageSizeY_um * 100.f;
                    }
                }

                // m_searchPara[DOWN] 저장
                for (long i = 0; i < long(m_searchPara[DOWN].size()); i++)
                {
                    auto& sPara = m_searchPara[DOWN][i];
                    strTemp.Format(_T("Bottom_Edge_Detect_ROI_%d"), i);
                    if (LinkEx(bSave, db[strTemp], tempfRoi))
                    {
                        sPara.m_searchLength_um = tempfRoi.Height();
                        sPara.m_persent_start = (tempfRoi.m_left + packageSizeX_um * 0.5f) / packageSizeX_um * 100.f;
                        sPara.m_persent_end = (tempfRoi.m_right + packageSizeX_um * 0.5f) / packageSizeX_um * 100.f;
                    }
                }

                // m_vecrtLowTopEdgeDetectROI 저장
                for (long i = 0; i < long(m_vecrtLowTopSearchROI_BCU.size()); i++)
                {
                    strTemp.Format(_T("LowTop_Edge_Detect_ROI_%d"), i);
                    if (!LinkEx(bSave, db[strTemp], m_vecrtLowTopSearchROI_BCU[i]))
                    {
                        m_vecrtLowTopSearchROI_BCU[i] = Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f);
                    }
                }
                break;

            default:
                // m_searchPara[LEFT] 저장
                for (long i = 0; i < long(m_searchPara[LEFT].size()); i++)
                {
                    strTemp.Format(_T("Left_Edge_Detect_ROI_%d"), i);
                    m_searchPara[LEFT][i].LinkDataBase(bSave, db[strTemp]);
                }

                // m_searchPara[UP] 저장
                for (long i = 0; i < long(m_searchPara[UP].size()); i++)
                {
                    strTemp.Format(_T("Top_Edge_Detect_ROI_%d"), i);
                    m_searchPara[UP][i].LinkDataBase(bSave, db[strTemp]);
                }

                // m_searchPara[RIGHT] 저장
                for (long i = 0; i < long(m_searchPara[RIGHT].size()); i++)
                {
                    strTemp.Format(_T("Right_Edge_Detect_ROI_%d"), i);
                    m_searchPara[RIGHT][i].LinkDataBase(bSave, db[strTemp]);
                }

                // m_searchPara[DOWN] 저장
                for (long i = 0; i < long(m_searchPara[DOWN].size()); i++)
                {
                    strTemp.Format(_T("Bottom_Edge_Detect_ROI_%d"), i);
                    m_searchPara[DOWN][i].LinkDataBase(bSave, db[strTemp]);
                }

                // m_vecrtLowTopEdgeDetectROI 저장
                for (long i = 0; i < long(m_vecrtLowTopSearchROI_BCU.size()); i++)
                {
                    strTemp.Format(_T("LowTop_Edge_Detect_ROI_%d"), i);
                    if (!LinkEx(bSave, db[strTemp], m_vecrtLowTopSearchROI_BCU[i]))
                    {
                        m_vecrtLowTopSearchROI_BCU[i] = Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f);
                    }
                }
                break;
        }
    }

    m_ImageProcMangePara.LinkDataBase(bSave, db[_T("{93CB31F7-A6D4-4E98-AC90-24BCB2B048EE}")]);

    strTemp.Empty();

    //{{//kircheis_NGRVAF
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP)
    {
        if (!db[_T("For_NGRVAF_IsValidPlaneRefInfo")].Link(bSave, m_bIsValidPlaneRefInfo))
            m_bIsValidPlaneRefInfo = FALSE;

        if (!db[_T("For_NGRVAF_FrameID")].Link(bSave, m_nNGRVAF_FrameID))
            m_nNGRVAF_FrameID = 0;

        m_vecptRefPos_UM.resize(4);
        for (long idx = 0; idx < 4; idx++)
        {
            strTemp.Format(_T("For_NGRVAF_ptRefPos_UM%d"), idx);
            if (!db[strTemp].Link(bSave, m_vecptRefPos_UM[idx]))
            {
                int nX = -50 + ((idx % 2) * 100); //왜 이렇게 했을지 궁금하면 500원!!
                int nY = -50 + ((idx / 2) * 100);
                m_vecptRefPos_UM[idx].Set(nX * 100, nY * 100);
            }
        }

        if (!db[_T("For_NGRVAF_AFImgSizeX")].Link(bSave, m_nAFImgSizeX))
            m_nAFImgSizeX = 150;
        if (!db[_T("For_NGRVAF_AFImgSizeY")].Link(bSave, m_nAFImgSizeY))
            m_nAFImgSizeY = 150;

        static const long nSizeOfByte = sizeof(BYTE);
        long nImageSize = m_nAFImgSizeX * m_nAFImgSizeY * 3;
        m_vecbyAFImage.resize(nImageSize);
        BYTE* pbyAFImage = &m_vecbyAFImage[0];
        if (!db[_T("For_NGRVAF_AFImage")].LinkArray(bSave, pbyAFImage, nImageSize))
            memset(pbyAFImage, 0, nImageSize * nSizeOfByte);
    }
    //}}

    return TRUE;
}

void Para::Init()
{
    m_nSearchDirection = 0;
    m_nEdgeDirection = 0;
    m_nRoughAlignEdgeLength = 10;
    m_nEdgeDetectMode = EdgeDetectMode_BestEdge;
    m_fFirstEdgeMinThreshold = 10.f;
    m_nLowTopCount = 0;

    for (long direction = 0; direction < 4; direction++)
    {
        m_searchPara[direction].resize(1);
        m_searchPara[direction][0].Initialize();
    }

    m_vecrtLowTopSearchROI_BCU.clear();

    m_ImageProcMangePara.Init();

    //m_fEdgeSamplerate = 100.f;
    m_fEdgeAlignGap_um = 40.f;

    //{{  //kircheis_3DEmpty
    m_emptyInsp = FALSE;

    m_2D_emptyMinBlobSize_umSqure = 0;
    m_2D_emptyThresholdValue = 128;
    m_2D_emptyMinBlobCount = 0;
    m_2D_emptyBlobROI_BCU = Ipvm::Rect32r(-1500.f, -1500.f, 1500.f, 1500.f);
    m_2D_emptyReverseThreshold = FALSE;
    m_2D_emptyIgnoreROINum = 0;
    m_2D_emptyIgnoreROIs_BCU.clear();
    m_2D_emptyUseFilterBlobFullROI = TRUE;

    m_3D_doubleInsp = FALSE;
    m_3D_heightOfTrayWallFromMold = 0.f;
    m_3D_emptyCheckMoldAreaRatioSpec = 0.45f;

    m_alignImageFrameIndex.setFrameIndex(0);
    m_emptyFrameIndex.setFrameIndex(0);

    //{{//kircheis_NGRVAF
    m_bIsValidPlaneRefInfo = FALSE;
    m_nNGRVAF_FrameID = 0;
    m_vecptRefPos_UM.clear();
    m_nAFImgSizeX = 0;
    m_nAFImgSizeY = 0;
    m_vecbyAFImage.clear();
    //}}

    m_nNGRVRefPosCount = 0;
    m_vecrtNGRVRefPosROI.clear();
}

std::vector<CString> Para::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Module 특성 변수 //채워야 함
    CString strDirection;

    CString arStrDirectionName[] = {_T("Top"), _T("Bottom"), _T("Left"), _T("Right"), _T("LowTop")};

    //Algorithm Parameter 정리

    /********************************************************************* Empty **********************************************************************/
    strCategory.Format(_T("Empty"));

    // Empty
    strGroup.Format(_T("Empty"));

    strParaName.Format(_T("Empty Check"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_emptyInsp));
    strParaName.Format(_T("Empty Image Frame Index"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_emptyFrameIndex.getFrameIndex()));
    strParaName.Format(_T("Image Threshold"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_2D_emptyThresholdValue));
    strParaName.Format(_T("Ignore ROI"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_2D_emptyIgnoreROINum));
    strParaName.Format(_T("Min Blob Size (um^2)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_2D_emptyMinBlobSize_umSqure));
    strParaName.Format(_T("Min Blob Count"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_2D_emptyMinBlobCount));
    strParaName.Format(_T("Reverse Threshold"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_2D_emptyReverseThreshold));
    strParaName.Format(_T("Use Blob Filter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_2D_emptyUseFilterBlobFullROI));

    // Run
    strGroup.Format(_T("Empty Result"));

    // Empty ROI
    strGroup.Format(_T("ROI"));

    std::vector<CString> vecStrRoiInfoBuffer(0);

    strParaName.Format(_T("Empty Check Blob ROI"));
    vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, m_2D_emptyBlobROI_BCU);
    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());

    //Ignore ROI
    for (long idx = 0; idx < m_2D_emptyIgnoreROINum; idx++)
    {
        strParaName.Format(_T("Ignore ROI %d"), idx);
        vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, m_2D_emptyIgnoreROIs_BCU[idx]);
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
    }

    /********************************************************************* Edge Align **********************************************************************/
    strCategory.Format(_T("Edge Align"));

    // Edge align parameter
    strGroup.Format(_T("Edge Align Parameter"));

    // Edge Align의 경우에는 2D 3D 항목이 파라미터군을 제외하면 동일하게 생성된다. > 해당 항목에 대해서 분기해준 뒤 데이터 정리
    //if (strVisionName == arStrModuleName[0] || strVisionName == arStrModuleName[2])
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
        || SystemConfig::GetInstance().GetVisionType()
            == VISIONTYPE_SWIR_INSP) //kircheis_SideVision고려 //kircheis_SWIR
    {
        // 2D Edge Align
        //Image Combine
        std::vector<CString> vecstrImageCombineParameters(0);
        vecstrImageCombineParameters = m_ImageProcMangePara.ExportImageCombineParaToText(
            strVisionName, strInspectionModuleName, _T("Edge Align-Image Combine"));
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

        strParaName.Format(_T("Align Image Frame Index"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_alignImageFrameIndex.getFrameIndex()));
    }

    else
    {
        // 3D Edge Align
        strParaName.Format(_T("Empty Check"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_emptyInsp));
        strParaName.Format(_T("Double Check"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_3D_doubleInsp));
        strParaName.Format(_T("Empty Check Ratio"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_3D_emptyCheckMoldAreaRatioSpec));
        strParaName.Format(_T("Height of tray wall from mold"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_3D_heightOfTrayWallFromMold));
    }

    // Edge Detect Parameter
    strGroup.Format(_T("Edge Detect Parameter"));

    strParaName.Format(_T("Search Direction"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nSearchDirection));
    strParaName.Format(_T("Edge Direction"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nEdgeDirection));
    strParaName.Format(_T("Edge Detect Mode"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_nEdgeDetectMode));
    strParaName.Format(_T("FirstEdge Min Value"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fFirstEdgeMinThreshold));
    strParaName.Format(_T("Detected EdgeAlign Gap(um)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_fEdgeAlignGap_um));

    // Edge Detect ROI Parameter
    strGroup.Format(_T("Edge Detect ROI Parameter"));

    for (long nDirection = 0; nDirection < 5; nDirection++)
    {
        strParaName = arStrDirectionName[nDirection];
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, (long)m_searchPara[nDirection].size()));
    }

    // Auto Focus Image Set
    strGroup.Format(_T("Auto Focus Image Set"));
    for (long idx = 0; idx < m_nNGRVRefPosCount; idx++)
    {
        strParaName.Format(_T("AF Pos %d"), idx);
        vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, m_vecrtNGRVRefPosROI[idx]);
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
    }

    // ROI
    strGroup.Format(_T("ROI"));
    for (long nDirection = 0; nDirection < 5; nDirection++)
    {
        for (long Count = 0; Count < m_searchPara[nDirection].size(); Count++)
        {
            CString sCount;
            sCount.Format(_T("_%d"),
                Count + 1); // 실제로는 0부터 시작되지만 표시는 1부터 시작되므로 표시되는 ROI의 이름을 한 단계씩 올린다.
            strParaName = arStrDirectionName[nDirection] + sCount;
            strParaNameAux.Format(_T("S.I. (um)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, strParaNameAux, m_searchPara[nDirection][Count].m_searchLength_um));
            strParaNameAux.Format(_T("Start (%%)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, strParaNameAux, m_searchPara[nDirection][Count].m_persent_start));
            strParaNameAux.Format(_T("End (%%)"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, strParaNameAux, m_searchPara[nDirection][Count].m_persent_end));
        }
    }
    /***************************************************************************************************************************************************/
    return vecstrAlgorithmParameters;
}