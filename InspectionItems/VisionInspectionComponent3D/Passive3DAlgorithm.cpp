//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Passive3DAlgorithm.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NC_SUB(x, y) ((x == Ipvm::k_noiseValue32r || y == Ipvm::k_noiseValue32r) ? Ipvm::k_noiseValue32r : x - y)
#define NC_MUL(x, y) ((x == Ipvm::k_noiseValue32r || y == Ipvm::k_noiseValue32r) ? Ipvm::k_noiseValue32r : x * y)
#define NC_DIV(x, y) ((x == Ipvm::k_noiseValue32r || y == Ipvm::k_noiseValue32r) ? Ipvm::k_noiseValue32r : x / y)

//CPP_7_________________________________ Implementation body
//
CPassive3DAlgorithm::CPassive3DAlgorithm(sPassive_InfoDB* i_pPassiveInfoDB, SComponentAlignSpec* i_pPassiveAlignParam,
    VisionReusableMemory* i_pVisionReusableMemory, CPI_EdgeDetect* i_pEdgeDetect, CPI_Blob* i_pBlob,
    const float& ConvertGV_Gain, const float& ConvertGV_Offset, SComponentAlignResult& o_sPassiveAlignResult)
{
    m_pPassiveInfoDB = i_pPassiveInfoDB;
    m_pPassiveAlignParam = i_pPassiveAlignParam;
    m_pVisionReusableMemory = i_pVisionReusableMemory;
    m_pEdgeDetect = i_pEdgeDetect;
    m_pBlob = i_pBlob;
    m_pPassiveAlignResult = &o_sPassiveAlignResult;
    m_ConvertGV_Gain = ConvertGV_Gain;
    m_ConvertGV_Offset = ConvertGV_Offset;
}

CPassive3DAlgorithm::~CPassive3DAlgorithm(void)
{
}

bool CPassive3DAlgorithm::DoCapacitor_DetailAlign(const VisionScale& scale, const Ipvm::Image32r& i_ZMap,
    const Ipvm::Image16u& i_WIntensityMap, const Ipvm::Image8u& i_HMap, Ipvm::Image8u& AlignimageBuf)
{
    bool bRoughAlignSuccess(true), bAlignSuccess(true), bReAlignSuccess(true);

    std::vector<std::vector<FPI_RECT>> vec2frtChip_SearchROI(4);

    bRoughAlignSuccess &= DoRoughAlign(scale, i_HMap);

    if (bRoughAlignSuccess == true)
    {
        bAlignSuccess
            &= MakeDetailAlignROI_byRoughAlign(scale, m_pPassiveAlignResult->rtResChip, vec2frtChip_SearchROI);

        //Remove Noise image
        if (GetAlignimage(scale, m_pPassiveAlignResult->rtResChip, i_HMap, AlignimageBuf,
                m_pPassiveAlignParam->m_nCalc_Substrate_Area_Size_um,
                m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapX_Body_um,
                m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapY_Body_um,
                m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapX_um,
                m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapY_um, m_ConvertGV_Gain, m_ConvertGV_Offset)
            == false)
            return false;

        bAlignSuccess &= Passive_Align_byEdge(scale, AlignimageBuf, m_pPassiveAlignResult->rtResChip,
            m_pPassiveAlignResult->rtResBody, m_pPassiveAlignResult->rtValidComponent, vec2frtChip_SearchROI);
    }
    else
        bAlignSuccess = false;

    bool bisReAlign = (bRoughAlignSuccess && bAlignSuccess) ? false : true;
    if (m_pPassiveAlignParam->m_bReCalc_UseSpecROI == TRUE && bisReAlign == true)
    {
        bReAlignSuccess = DoAlignbySpecROI(scale);
        if (bReAlignSuccess == false)
            return false;

        bAlignSuccess = true;
    }

    if (bReAlignSuccess == false || bAlignSuccess == false)
        return false;

    m_pPassiveAlignResult->vecrtSubstrateArea = GetSubstrateMesureROI(scale, m_pPassiveAlignResult->rtValidComponent,
        m_pPassiveAlignParam->m_nCalc_Substrate_Area_Size_um, m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapX_Body_um,
        m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapY_Body_um, m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapX_um,
        m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapY_um);

    if (!CalcPassiveData(i_HMap, i_ZMap, i_WIntensityMap, false))
        return false;

    return true;
}

bool CPassive3DAlgorithm::GetAlignimage(const VisionScale& scale, const Ipvm::Rect32s& SearchROI,
    const Ipvm::Image8u& HMap, Ipvm::Image8u& Alignimage, const long& Substrate_Area_Size_um,
    const long& Substrate_Gap_Body_X_um, const long& Substrate_Gap_Body_Y_um, const long& Substrate_Area_GapX_um,
    const long& Substrate_Area_GapY_um, const float& ConvertGV_Gain, const float& ConvertGV_Offset)
{
    if (HMap.GetMem() == nullptr || Alignimage.GetMem() == nullptr)
        return false;

    //1. Rough Search 영역으로부터 주변 바닥 Data의 GV를 구함
    //2. 해당영역의 GV값의 반이상을 가져가서 평균을 구함
    //3. 4개의 바닥 GV에서 Min, Max를 제외한 2개영역의 평균을 구한다
    //4. 획득한 평균 GV와 Spec 높이값을 이용한 GV 환산 값을 비교하여 Threshold 값으로 이용한다
    std::vector<Ipvm::Rect32s> vecrtSubstrate = GetSubstrateMesureROI(scale, SearchROI, Substrate_Area_Size_um,
        Substrate_Gap_Body_X_um, Substrate_Gap_Body_Y_um, Substrate_Area_GapX_um, Substrate_Area_GapY_um);

    const long SubstrateCount(4);
    std::vector<double> SubstrateAvg_GV(SubstrateCount);

    for (long Substrateidx = 0; Substrateidx < SubstrateCount; Substrateidx++)
    {
        double AvgGV(0.);
        if (Ipvm::ImageProcessing::GetMean(HMap, vecrtSubstrate[Substrateidx], AvgGV) != Ipvm::Status::e_ok)
            return false;

        SubstrateAvg_GV[Substrateidx] = AvgGV;
    }

    double MaxGV(SubstrateAvg_GV[0]), MinGV(SubstrateAvg_GV[0]), SumGV(0);
    long SumCount(0);
    for (long idx = 0; idx < (long)SubstrateAvg_GV.size(); idx++)
    {
        if (SubstrateAvg_GV[idx] > MaxGV)
        {
            MaxGV = SubstrateAvg_GV[idx];
            continue;
        }

        if (SubstrateAvg_GV[idx] < MinGV)
        {
            MinGV = SubstrateAvg_GV[idx];
            continue;
        }

        SumGV += SubstrateAvg_GV[idx];
        SumCount++;
    }

    const uint8_t CalcSubstrateAvg_GV = CAST_INT8T((SumGV / SumCount) + .5f);

    const float PaasiveSpecHeight = (m_pPassiveInfoDB->fPassiveHeight_mm * 1000.f) / scale.pixelToUmZ();
    const float Factor_ConvertSpecHeight_GV(0.5f); //Spec GV의 반을 사용한다

    const uint8_t ConvertSpecHeight_GV = CAST_INT8T((PaasiveSpecHeight * ConvertGV_Gain) + ConvertGV_Offset);
    const uint8_t CalcConvertSpecHeight_GV = CAST_INT8T(ConvertSpecHeight_GV * Factor_ConvertSpecHeight_GV);

    const uint8_t ThresholdVal = max(CalcSubstrateAvg_GV, CalcConvertSpecHeight_GV);

    if (Ipvm::ImageProcessing::BinarizeGreater(HMap, SearchROI, ThresholdVal, Alignimage) != Ipvm::Status::e_ok)
        return false;

    return true;
}

bool CPassive3DAlgorithm::DoMIA_DetailAlign(const VisionScale& scale, const Ipvm::Image32r& i_ZMap,
    const Ipvm::Image16u& i_WIntensityMap, const Ipvm::Image8u& i_HMap)
{
    bool bRoughAlignSuccess(true), bAlignSuccess(true), bReAlignSuccess(true);

    std::vector<std::vector<FPI_RECT>> vec2frtChip_SearchROI(4);

    bRoughAlignSuccess &= DoRoughAlign(scale, i_HMap);
    if (bRoughAlignSuccess == true)
    {
        bAlignSuccess
            &= MakeDetailAlignROI_byRoughAlign(scale, m_pPassiveAlignResult->rtResChip, vec2frtChip_SearchROI);
        bAlignSuccess &= Passive_Align_byEdge(i_HMap, vec2frtChip_SearchROI, m_pPassiveAlignResult->rtValidComponent);
    }
    else
        bAlignSuccess = false;

    bool bisReAlign = (bRoughAlignSuccess && bAlignSuccess) ? false : true;
    if (m_pPassiveAlignParam->m_bReCalc_UseSpecROI == TRUE && bisReAlign == true)
    {
        bReAlignSuccess = DoAlignbySpecROI(scale);
        if (bReAlignSuccess == false)
            return false;

        bAlignSuccess = true;
    }

    if (bReAlignSuccess == false || bAlignSuccess == false)
        return false;

    m_pPassiveAlignResult->vecrtSubstrateArea = GetSubstrateMesureROI(scale, m_pPassiveAlignResult->rtValidComponent,
        m_pPassiveAlignParam->m_nCalc_Substrate_Area_Size_um, m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapX_Body_um,
        m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapY_Body_um, m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapX_um,
        m_pPassiveAlignParam->m_nCalc_Substrate_Area_GapY_um);

    if (!CalcPassiveData(i_HMap, i_ZMap, i_WIntensityMap, true))
        return false;

    return true;
}

bool CPassive3DAlgorithm::DoAlignbySpecROI(const VisionScale& scale)
{
    FPI_RECT frtShiftSpecROI = GetShiftSpecROI(scale, m_pPassiveInfoDB->sfrtSpecROI_px);
    m_pPassiveAlignResult->Debug_frtShiftSpecROI = frtShiftSpecROI;

    if (!CreateSpecROI(scale, frtShiftSpecROI, m_pPassiveAlignResult->rtResChip, m_pPassiveAlignResult->rtResBody,
            m_pPassiveAlignResult->vecrtResElect))
        return false;

    m_pPassiveAlignResult->rtValidComponent = m_pPassiveAlignResult->rtResChip;
    m_pPassiveAlignResult->rtValidBody = m_pPassiveAlignResult->rtResBody;

    if (m_pPassiveAlignResult->vecrtValidElect.size() >= 0)
        m_pPassiveAlignResult->vecrtValidElect.clear();

    m_pPassiveAlignResult->vecrtValidElect = m_pPassiveAlignResult->vecrtResElect;

    return true;
}

bool CPassive3DAlgorithm::DoRoughAlign(const VisionScale& scale, const Ipvm::Image8u& i_HMap)
{
    FPI_RECT frtShiftSpecROI = GetShiftSpecROI(scale, m_pPassiveInfoDB->sfrtSpecROI_px);
    m_pPassiveAlignResult->Debug_frtShiftSpecROI = frtShiftSpecROI;

    if (!CreateSpecROI(scale, frtShiftSpecROI, m_pPassiveAlignResult->rtSpecChip, m_pPassiveAlignResult->rtSpecBody,
            m_pPassiveAlignResult->vecrtSpecElect))
        return false;

    Ipvm::Rect32s rtRoughAlignSearchROI = GetPassive_Rough_SearchROI(m_pPassiveAlignResult->rtSpecChip);
    m_pPassiveAlignResult->Debug_rtRoughAlign_SearchROI = rtRoughAlignSearchROI;

    Ipvm::Rect32s rtRoughAlign;

    if (!Passive_RoughAlign_byBlobCneterGravity(scale, i_HMap, rtRoughAlignSearchROI, rtRoughAlign))
        return false;

    FPI_RECT fsrtChip = FPI_RECT(Ipvm::Point32r2((float)rtRoughAlign.m_left, (float)rtRoughAlign.m_top),
        Ipvm::Point32r2((float)rtRoughAlign.m_right, (float)rtRoughAlign.m_top),
        Ipvm::Point32r2((float)rtRoughAlign.m_left, (float)rtRoughAlign.m_bottom),
        Ipvm::Point32r2((float)rtRoughAlign.m_right, (float)rtRoughAlign.m_bottom));

    if (!CreateSpecROI(scale, fsrtChip, m_pPassiveAlignResult->rtResChip, m_pPassiveAlignResult->rtResBody,
            m_pPassiveAlignResult->vecrtResElect))
        return false;

    return true;
}

FPI_RECT CPassive3DAlgorithm::GetShiftSpecROI(const VisionScale& scale, FPI_RECT i_frtSpecROI)
{
    const auto& um2px = scale.umToPixel();

    FPI_RECT sfrtShift_SpecROI = i_frtSpecROI;
    float fShiftXPos = float(m_pPassiveAlignParam->m_fSpecROI_ShiftXPos * um2px.m_x);
    float fShiftYPos = float(m_pPassiveAlignParam->m_fSpecROI_ShiftYPos * um2px.m_y);

    sfrtShift_SpecROI.Move(fShiftXPos, fShiftYPos);

    return sfrtShift_SpecROI;
}

bool CPassive3DAlgorithm::CreateSpecROI(const VisionScale& scale, FPI_RECT i_frtShift_SpecROI, Ipvm::Rect32s& o_SpecROI,
    Ipvm::Rect32s& o_SpecROI_Body, std::vector<Ipvm::Rect32s>& o_vecSpecElectrodeROI)
{
    if (m_pPassiveInfoDB == nullptr)
        return false;

    const auto& mm2px = scale.mmToPixel();

    if (m_pPassiveInfoDB->fPassiveAngle == 90.f) //Vertical Type
    {
        Ipvm::Rect32s rtSpecROI;
        float fPassive_SpecWidth = m_pPassiveInfoDB->fPassiveSpecWidth_mm * mm2px.m_x;
        float fPassive_SpecLength = m_pPassiveInfoDB->fPassiveSpecLength_mm * mm2px.m_y;

        Ipvm::Point32r2 fptCenter = i_frtShift_SpecROI.GetCenter();

        rtSpecROI.m_left = (int)(fptCenter.m_x - fPassive_SpecWidth / 2.f + .5f);
        rtSpecROI.m_right = (int)(fptCenter.m_x + fPassive_SpecWidth / 2.f + .5f);
        rtSpecROI.m_top = (int)(fptCenter.m_y - fPassive_SpecLength / 2.f + .5f);
        rtSpecROI.m_bottom = (int)(fptCenter.m_y + fPassive_SpecLength / 2.f + .5f);
        o_SpecROI = rtSpecROI;

        if (m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
        {
            CreatePassiveElectrodeROI_bySpec(scale, rtSpecROI, o_vecSpecElectrodeROI);

            o_SpecROI_Body = rtSpecROI;
            o_SpecROI_Body.m_top = o_vecSpecElectrodeROI[0].m_bottom;
            o_SpecROI_Body.m_bottom = o_vecSpecElectrodeROI[1].m_top;
        }
    }
    else
    {
        // 회전된 영상에 영역을 맞춘다.
        float fRadian = (float)(-m_pPassiveInfoDB->fPassiveAngle * DEF_DEG_TO_RAD);
        Ipvm::Rect32s rtSpecROI = i_frtShift_SpecROI.Rotate(fRadian).GetCRect();
        o_SpecROI = rtSpecROI;

        if (m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
            CreatePassiveElectrodeROI_bySpec(scale, rtSpecROI, o_vecSpecElectrodeROI);
        {
            o_SpecROI_Body = rtSpecROI;
            o_SpecROI_Body.m_left = o_vecSpecElectrodeROI[0].m_right;
            o_SpecROI_Body.m_right = o_vecSpecElectrodeROI[1].m_left;
        }
    }

    return true;
}

bool CPassive3DAlgorithm::CreatePassiveElectrodeROI_bySpec(
    const VisionScale& scale, Ipvm::Rect32s i_rtSpecROI, std::vector<Ipvm::Rect32s>& o_vecSpecElectrodeROI)
{
    const auto& mm2px = scale.mmToPixel();

    o_vecSpecElectrodeROI.clear();
    o_vecSpecElectrodeROI.resize(2);

    if (m_pPassiveInfoDB->fPassiveAngle == 90.f) // Vertical Type
    {
        float fPassiveElect_SpecWidth = m_pPassiveInfoDB->fPassiveElectrodeWidth_mm * mm2px.m_y;
        o_vecSpecElectrodeROI[0] = o_vecSpecElectrodeROI[1] = i_rtSpecROI;
        o_vecSpecElectrodeROI[0].m_bottom = (int)(i_rtSpecROI.m_top + fPassiveElect_SpecWidth + .5f);
        o_vecSpecElectrodeROI[1].m_top = (int)(i_rtSpecROI.m_bottom - fPassiveElect_SpecWidth + .5f);
    }
    else
    {
        float fPassiveElect_SpecWidth = m_pPassiveInfoDB->fPassiveElectrodeWidth_mm * mm2px.m_x;
        o_vecSpecElectrodeROI[0] = o_vecSpecElectrodeROI[1] = i_rtSpecROI;
        o_vecSpecElectrodeROI[0].m_right = (int)(i_rtSpecROI.m_left + fPassiveElect_SpecWidth + .5f);
        o_vecSpecElectrodeROI[1].m_left = (int)(i_rtSpecROI.m_right - fPassiveElect_SpecWidth + .5f);
    }

    return true;
}

Ipvm::Rect32s CPassive3DAlgorithm::GetPassive_Rough_SearchROI(Ipvm::Rect32s i_rtSpecROI)
{
    Ipvm::Point32r2 fptCenter
        = Ipvm::Point32r2((float)i_rtSpecROI.CenterPoint().m_x, (float)i_rtSpecROI.CenterPoint().m_y);
    Ipvm::Rect32s rtRoughSearchROI = i_rtSpecROI;

    long nPassiveSpecROI_Width(0), nPassiveSpecROI_Length(0);
    if (m_pPassiveInfoDB->fPassiveAngle == 90.f) // Vertical Type
    {
        nPassiveSpecROI_Width = i_rtSpecROI.Width();
        nPassiveSpecROI_Length = i_rtSpecROI.Height();
    }
    else
    {
        nPassiveSpecROI_Width = i_rtSpecROI.Height();
        nPassiveSpecROI_Length = i_rtSpecROI.Width();
    }

    // 탐색길이는 원 스펙 길이에서 패턴의 매칭 탐색 길이를 더함.
    long nSearchPassiveWidth = (long)(nPassiveSpecROI_Width
        + nPassiveSpecROI_Width * m_pPassiveAlignParam->m_fRoughAlign_Search_ROI_Extension_Ratio / 100.f + .5f);
    long nSearchPassiveLength = (long)(nPassiveSpecROI_Length
        + nPassiveSpecROI_Length * m_pPassiveAlignParam->m_fRoughAlign_Search_ROI_Extension_Ratio / 100.f + .5f);
    if (m_pPassiveInfoDB->fPassiveAngle == 90.f) // Vertical
    {
        rtRoughSearchROI.m_left = (int)(fptCenter.m_x - (float)nSearchPassiveWidth / 2.f + .5f);
        rtRoughSearchROI.m_top = (int)(fptCenter.m_y - (float)nSearchPassiveLength / 2.f + .5f);
        rtRoughSearchROI.m_right = (int)(fptCenter.m_x + (float)nSearchPassiveWidth / 2.f + .5f);
        rtRoughSearchROI.m_bottom = (int)(fptCenter.m_y + (float)nSearchPassiveLength / 2.f + .5f);
    }
    else
    {
        rtRoughSearchROI.m_left = (int)(fptCenter.m_x - (float)nSearchPassiveLength / 2.f + .5f);
        rtRoughSearchROI.m_top = (int)(fptCenter.m_y - (float)nSearchPassiveWidth / 2.f + .5f);
        rtRoughSearchROI.m_right = (int)(fptCenter.m_x + (float)nSearchPassiveLength / 2.f + .5f);
        rtRoughSearchROI.m_bottom = (int)(fptCenter.m_y + (float)nSearchPassiveWidth / 2.f + .5f);
    }

    return rtRoughSearchROI;
}

bool CPassive3DAlgorithm::Passive_RoughAlign_byBlobCneterGravity(const VisionScale& scale,
    const Ipvm::Image8u& i_imgHMap, Ipvm::Rect32s i_rtRoughSearchROI, Ipvm::Rect32s& o_RoughAlignResult)
{
    const auto& mm2px = scale.mmToPixel();

    Ipvm::Image8u validImage;
    Ipvm::Image8u backgroundImage;
    Ipvm::Image8u labelImage;

    if (!m_pVisionReusableMemory->GetInspByteImage(validImage))
        return false;
    if (!m_pVisionReusableMemory->GetInspByteImage(backgroundImage))
        return false;
    if (!m_pVisionReusableMemory->GetInspByteImage(labelImage))
        return false;

    Ipvm::ImageProcessing::BinarizeGreater(i_imgHMap, i_rtRoughSearchROI, 0, validImage);

    double roiMean(0.);
    if (Ipvm::ImageProcessing::GetMean(i_imgHMap, validImage, i_rtRoughSearchROI, roiMean) != Ipvm::Status::e_ok)
        return false;

    double fAvg_background_Mean(0.);
    Ipvm::ImageProcessing::BinarizeLess(i_imgHMap, i_rtRoughSearchROI, BYTE(roiMean), backgroundImage);
    Ipvm::ImageProcessing::BitwiseAnd(validImage, i_rtRoughSearchROI, backgroundImage);
    Ipvm::ImageProcessing::GetMean(i_imgHMap, backgroundImage, i_rtRoughSearchROI, fAvg_background_Mean);

    double fAvg_Obejct_Mean(0.);
    Ipvm::ImageProcessing::BinarizeGreater(i_imgHMap, i_rtRoughSearchROI, BYTE(roiMean), backgroundImage);
    Ipvm::ImageProcessing::BitwiseAnd(validImage, i_rtRoughSearchROI, backgroundImage);
    Ipvm::ImageProcessing::GetMean(i_imgHMap, backgroundImage, i_rtRoughSearchROI, fAvg_Obejct_Mean);

    float fValidElectThicknessRatio(0.2f); // 높이 Average 데이터의 20%를 사용함
    BYTE byThresholdValue = (BYTE)max(0,
        min(255,
            (long)(fAvg_background_Mean + ((fAvg_Obejct_Mean - fAvg_background_Mean) * fValidElectThicknessRatio)
                + .5f)));

    Ipvm::ImageProcessing::BinarizeGreater(i_imgHMap, i_rtRoughSearchROI, byThresholdValue, labelImage);

    float fPassiveSpecArea
        = m_pPassiveInfoDB->fPassiveSpecWidth_mm * m_pPassiveInfoDB->fPassiveSpecLength_mm * mm2px.m_x * mm2px.m_y;
    long nNoiseArea = static_cast<long>(fPassiveSpecArea * 0.01f + .5f);

    nNoiseArea = max(5, nNoiseArea);

    Ipvm::Image32s imageTemp;
    if (!m_pVisionReusableMemory->GetInspLongImage(imageTemp))
        return false;

    Ipvm::ImageProcessing::Fill(i_rtRoughSearchROI, 0, imageTemp);

    int32_t nBlobNum = BLOB_INFO_LARGE_SIZE;
    Ipvm::BlobInfo* pBlobInfo = m_pVisionReusableMemory->GetBlobInfo(1);

    if (m_pBlob->DoBlob(
            labelImage, i_rtRoughSearchROI, BLOB_INFO_LARGE_SIZE, imageTemp, pBlobInfo, nBlobNum, nNoiseArea)
        != false)
        return false;

    if (nBlobNum <= 0)
        return false;

    Ipvm::Point32r2 fptBlobCneterGravity = GetCenterGravity(imageTemp, i_rtRoughSearchROI);

    // 전극 Length 방항은 노이즈가 포함 되어 있을 수 있어 원래 크기로 다시 설정한다.
    float fPixelperMM = (mm2px.m_x + mm2px.m_y) / 2.f;

    float fPassive_SpecWidth = m_pPassiveInfoDB->fPassiveSpecWidth_mm * fPixelperMM;
    float fPassive_SpecLength = m_pPassiveInfoDB->fPassiveSpecLength_mm * fPixelperMM;

    if (m_pPassiveInfoDB->fPassiveAngle == 90.f) // 90도일때 (top, bottom) 추가, 아닐때 (left, rigth) 추가
    {
        o_RoughAlignResult.m_left = static_cast<long>(fptBlobCneterGravity.m_x - fPassive_SpecWidth / 2.f + .5f);
        o_RoughAlignResult.m_right = static_cast<long>(fptBlobCneterGravity.m_x + fPassive_SpecWidth / 2.f + .5f);

        o_RoughAlignResult.m_top = static_cast<long>(fptBlobCneterGravity.m_y - fPassive_SpecLength / 2.f + .5f);
        o_RoughAlignResult.m_bottom = static_cast<long>(fptBlobCneterGravity.m_y + fPassive_SpecLength / 2.f + .5f);
    }
    else
    {
        o_RoughAlignResult.m_left = static_cast<long>(fptBlobCneterGravity.m_x - fPassive_SpecLength / 2.f + .5f);
        o_RoughAlignResult.m_right = static_cast<long>(fptBlobCneterGravity.m_x + fPassive_SpecLength / 2.f + .5f);

        o_RoughAlignResult.m_top = static_cast<long>(fptBlobCneterGravity.m_y - fPassive_SpecWidth / 2.f + .5f);
        o_RoughAlignResult.m_bottom = static_cast<long>(fptBlobCneterGravity.m_y + fPassive_SpecWidth / 2.f + .5f);
    }

    return true;
}

Ipvm::Point32r2 CPassive3DAlgorithm::GetCenterGravity(const Ipvm::Image32s& i_image, Ipvm::Rect32s i_rtCalcROI)
{
    Ipvm::Point32r2 fptCenterGravityValue(0.f, 0.f);

    long validPixelCount(0);
    for (long y = i_rtCalcROI.m_top; y < i_rtCalcROI.m_bottom; y++)
    {
        auto* label_y = i_image.GetMem(0, y);
        for (long x = i_rtCalcROI.m_left; x < i_rtCalcROI.m_right; x++)
        {
            if (label_y[x] != 0)
            {
                fptCenterGravityValue.m_x += x;
                fptCenterGravityValue.m_y += y;
                validPixelCount++;
            }
        }
    }

    if (validPixelCount > 0)
    {
        fptCenterGravityValue.m_x /= validPixelCount;
        fptCenterGravityValue.m_y /= validPixelCount;
    }

    return fptCenterGravityValue;
}

bool CPassive3DAlgorithm::MakeDetailAlignROI_byRoughAlign(const VisionScale& scale,
    const Ipvm::Rect32s& i_rtRoughAlignROI, std::vector<std::vector<FPI_RECT>>& o_vec2frtChip_SearchROI)
{
    if (!m_pPassiveInfoDB)
        return false;

    Ipvm::Point32s2 RoughAlignCenter = i_rtRoughAlignROI.CenterPoint();
    Ipvm::Rect32s rtRoughAlignROI = i_rtRoughAlignROI;

    float fRoughAlignCenterX = (float)RoughAlignCenter.m_x;
    float fRoughAlignCenterY = (float)RoughAlignCenter.m_y;
    FPI_RECT frtDetailAlign_LeftSearchROI, frtDetailAlign_TopSearchROI, frtDetailAlign_RightSearchROI,
        frtDetailAlign_BottomSearchROI;

    const auto& um2px = scale.umToPixel();

    float fElectrode_Search_Width_Ratio = m_pPassiveAlignParam->m_nOutline_align_Electrode_Search_Width_Ratio / 100.f;
    float fElectrode_Search_Length_um = (float)m_pPassiveAlignParam->m_nOutline_align_Electrode_Search_Length_um;
    float fBody_Search_Width_Ratio = m_pPassiveAlignParam->m_nOutline_align_Body_Search_Width_Ratio / 100.f;
    float fBody_Search_Length_um = (float)m_pPassiveAlignParam->m_nOutline_align_Body_Search_Length_um;

    float fElectrode_Search_LengthOffset(0.f), fBody_Search_LengthOffset(0.f); //Direction별로 다르게 구성해야한다
    float fHalf_Electrode_Search_LengthOffset(0.f), fHalf_Body_Search_LengthOffset(0.f);

    if (m_pPassiveInfoDB->fPassiveAngle == 0.f
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA
        || m_pPassiveInfoDB->fPassiveAngle == 180.f
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        float fSpecWidth = (float)rtRoughAlignROI.Height();
        float fSpecLength = (float)rtRoughAlignROI.Width();
        float fHalfSpecWidth = fSpecWidth * .5f;
        float fHalfSpecLength = fSpecLength * .5f;

        fElectrode_Search_LengthOffset = (fElectrode_Search_Length_um * um2px.m_x) + .5f;
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_y) + .5f;

        fHalf_Electrode_Search_LengthOffset = (fElectrode_Search_LengthOffset * .5f) + .5f;
        fHalf_Body_Search_LengthOffset = (fBody_Search_LengthOffset * .5f) + .5f;

        //{{Left
        float fLeftSearchROICenX = fRoughAlignCenterX - fHalfSpecLength;
        frtDetailAlign_LeftSearchROI.fptLT = Ipvm::Point32r2((fLeftSearchROICenX - fHalf_Electrode_Search_LengthOffset),
            (fRoughAlignCenterY - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRT = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fLeftSearchROICenX + fElectrode_Search_LengthOffset, LEFT)),
            (fRoughAlignCenterY - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptLB = Ipvm::Point32r2((fLeftSearchROICenX - fHalf_Electrode_Search_LengthOffset),
            (fRoughAlignCenterY + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRB = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fLeftSearchROICenX + fElectrode_Search_LengthOffset, LEFT)),
            (fRoughAlignCenterY + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[LEFT].push_back(frtDetailAlign_LeftSearchROI);

        ////{{RIGHT
        float fRightSearchROICenX = fRoughAlignCenterX + fHalfSpecLength;
        frtDetailAlign_RightSearchROI.fptLT = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fRightSearchROICenX - fElectrode_Search_LengthOffset, RIGHT)),
            (fRoughAlignCenterY - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_RightSearchROI.fptRT
            = Ipvm::Point32r2((fRightSearchROICenX + fHalf_Electrode_Search_LengthOffset),
                (fRoughAlignCenterY - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_RightSearchROI.fptLB = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fRightSearchROICenX - fElectrode_Search_LengthOffset, RIGHT)),
            (fRoughAlignCenterY + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_RightSearchROI.fptRB
            = Ipvm::Point32r2((fRightSearchROICenX + fHalf_Electrode_Search_LengthOffset),
                (fRoughAlignCenterY - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[RIGHT].push_back(frtDetailAlign_RightSearchROI);

        //}}

        //{{TOP
        float fTopSearchROICenY = fRoughAlignCenterY - fHalfSpecWidth;
        frtDetailAlign_TopSearchROI.fptLT
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - fHalf_Body_Search_LengthOffset));
        frtDetailAlign_TopSearchROI.fptRT
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - fHalf_Body_Search_LengthOffset));
        frtDetailAlign_TopSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (CheckOverPosbyCenterPos(fRoughAlignCenterY, fTopSearchROICenY + fBody_Search_LengthOffset, UP)));
        frtDetailAlign_TopSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (CheckOverPosbyCenterPos(fRoughAlignCenterY, fTopSearchROICenY + fBody_Search_LengthOffset, UP)));

        o_vec2frtChip_SearchROI[UP].push_back(frtDetailAlign_TopSearchROI);

        //{{BOTTOM
        float fBtmSearchROICenY = fRoughAlignCenterY + fHalfSpecWidth;
        frtDetailAlign_BottomSearchROI.fptLT
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (CheckOverPosbyCenterPos(fRoughAlignCenterY, fBtmSearchROICenY - fBody_Search_LengthOffset, DOWN)));
        frtDetailAlign_BottomSearchROI.fptRT
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (CheckOverPosbyCenterPos(fRoughAlignCenterY, fBtmSearchROICenY - fBody_Search_LengthOffset, DOWN)));
        frtDetailAlign_BottomSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fBtmSearchROICenY + fHalf_Body_Search_LengthOffset));
        frtDetailAlign_BottomSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fBtmSearchROICenY + fHalf_Body_Search_LengthOffset));

        o_vec2frtChip_SearchROI[DOWN].push_back(frtDetailAlign_BottomSearchROI);
        //}}
    }
    else if (m_pPassiveInfoDB->fPassiveAngle == 90.f
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA
        || m_pPassiveInfoDB->fPassiveAngle == 270.f
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)

    {
        float fSpecWidth = (float)rtRoughAlignROI.Width();
        float fSpecLength = (float)rtRoughAlignROI.Height();
        float fHalfSpecWidth = fSpecWidth * .5f;
        float fHalfSpecLength = fSpecLength * .5f;

        fElectrode_Search_LengthOffset = (fElectrode_Search_Length_um * um2px.m_y) + .5f;
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_x) + .5f;

        fHalf_Electrode_Search_LengthOffset = (fElectrode_Search_LengthOffset * .5f) + .5f;
        fHalf_Body_Search_LengthOffset = (fBody_Search_LengthOffset * .5f) + .5f;

        //{{LEFT
        float fLeftSearchROICenX = fRoughAlignCenterX - fHalfSpecWidth;
        frtDetailAlign_LeftSearchROI.fptLT = Ipvm::Point32r2((fLeftSearchROICenX - fHalf_Body_Search_LengthOffset),
            (fRoughAlignCenterY - (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRT = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fLeftSearchROICenX + fBody_Search_LengthOffset, LEFT)),
            (fRoughAlignCenterY - (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptLB = Ipvm::Point32r2((fLeftSearchROICenX - fHalf_Body_Search_LengthOffset),
            (fRoughAlignCenterY + (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRB = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fLeftSearchROICenX + fBody_Search_LengthOffset, LEFT)),
            (fRoughAlignCenterY + (fHalfSpecLength * fBody_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[LEFT].push_back(frtDetailAlign_LeftSearchROI);

        //{{Right
        float fRightSearchROICenX = fRoughAlignCenterX + fHalfSpecWidth;
        frtDetailAlign_RightSearchROI.fptLT = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fRightSearchROICenX - fBody_Search_LengthOffset, RIGHT)),
            (fRoughAlignCenterY - (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_RightSearchROI.fptRT = Ipvm::Point32r2((fRightSearchROICenX + fHalf_Body_Search_LengthOffset),
            (fRoughAlignCenterY - (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_RightSearchROI.fptLB = Ipvm::Point32r2(
            (CheckOverPosbyCenterPos(fRoughAlignCenterX, fRightSearchROICenX - fBody_Search_LengthOffset, RIGHT)),
            (fRoughAlignCenterY + (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_RightSearchROI.fptRB = Ipvm::Point32r2((fRightSearchROICenX + fHalf_Body_Search_LengthOffset),
            (fRoughAlignCenterY + (fHalfSpecLength * fBody_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[RIGHT].push_back(frtDetailAlign_RightSearchROI);

        //}}

        //{{TOP
        float fTopSearchROICenY = fRoughAlignCenterY - fHalfSpecLength;
        frtDetailAlign_TopSearchROI.fptLT
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fTopSearchROICenY - fHalf_Electrode_Search_LengthOffset));
        frtDetailAlign_TopSearchROI.fptRT
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fTopSearchROICenY - fHalf_Electrode_Search_LengthOffset));
        frtDetailAlign_TopSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (CheckOverPosbyCenterPos(fRoughAlignCenterY, fTopSearchROICenY + fElectrode_Search_LengthOffset, UP)));
        frtDetailAlign_TopSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (CheckOverPosbyCenterPos(fRoughAlignCenterY, fTopSearchROICenY + fElectrode_Search_LengthOffset, UP)));

        o_vec2frtChip_SearchROI[UP].push_back(frtDetailAlign_TopSearchROI);

        //{{Bottom
        float fBtmSearchROICenY = fRoughAlignCenterY + fHalfSpecLength;
        frtDetailAlign_BottomSearchROI.fptLT = Ipvm::Point32r2(
            (fRoughAlignCenterX - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
            (CheckOverPosbyCenterPos(fRoughAlignCenterY, fBtmSearchROICenY - fElectrode_Search_LengthOffset, DOWN)));
        frtDetailAlign_BottomSearchROI.fptRT = Ipvm::Point32r2(
            (fRoughAlignCenterX + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
            (CheckOverPosbyCenterPos(fRoughAlignCenterY, fBtmSearchROICenY - fElectrode_Search_LengthOffset, DOWN)));
        frtDetailAlign_BottomSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fBtmSearchROICenY + fHalf_Electrode_Search_LengthOffset));
        frtDetailAlign_BottomSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fBtmSearchROICenY + fHalf_Electrode_Search_LengthOffset));

        o_vec2frtChip_SearchROI[DOWN].push_back(frtDetailAlign_BottomSearchROI);
        //}}
    }
    else if (m_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        Ipvm::Point32r2 fDir_Hor_SpecSize((float)rtRoughAlignROI.Height(), (float)rtRoughAlignROI.Width());
        Ipvm::Point32r2 fDir_Ver_SpecSize((float)rtRoughAlignROI.Width(), (float)rtRoughAlignROI.Height());

        Ipvm::Point32r2 fDir_Hor_HalfSpecSize(fDir_Hor_SpecSize.m_x * .5f, fDir_Hor_SpecSize.m_y * .5f);
        Ipvm::Point32r2 fDir_Ver_HalfSpecSize(fDir_Ver_SpecSize.m_x * .5f, fDir_Ver_SpecSize.m_y * .5f);

        //{{Left
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_y) + .5f;
        float fLeftSearchROICenX = fRoughAlignCenterX - fDir_Hor_HalfSpecSize.m_y;
        frtDetailAlign_LeftSearchROI.fptLT = Ipvm::Point32r2((fLeftSearchROICenX - (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY - (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRT = Ipvm::Point32r2((fLeftSearchROICenX + (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY - (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptLB = Ipvm::Point32r2((fLeftSearchROICenX - (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY + (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRB = Ipvm::Point32r2((fLeftSearchROICenX + (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY + (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[LEFT].push_back(frtDetailAlign_LeftSearchROI);
        //}}

        //{{RIGHT
        frtDetailAlign_RightSearchROI = frtDetailAlign_LeftSearchROI.Move(fDir_Hor_SpecSize.m_y, 0);

        o_vec2frtChip_SearchROI[RIGHT].push_back(frtDetailAlign_RightSearchROI);
        //}}

        //{{Top
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_x) + .5f;
        float fTopSearchROICenY = fRoughAlignCenterY - fDir_Ver_HalfSpecSize.m_y;
        frtDetailAlign_TopSearchROI.fptLT
            = Ipvm::Point32r2((fRoughAlignCenterX - (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRT
            = Ipvm::Point32r2((fRoughAlignCenterX + (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY + (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY + (fBody_Search_LengthOffset * .5f)));

        o_vec2frtChip_SearchROI[UP].push_back(frtDetailAlign_TopSearchROI);
        //}}

        //{{Bottom
        frtDetailAlign_BottomSearchROI = frtDetailAlign_TopSearchROI.Move(0, fDir_Ver_SpecSize.m_y);

        o_vec2frtChip_SearchROI[DOWN].push_back(frtDetailAlign_BottomSearchROI);
        //}}
    }
    else //처리할수 없는 값이면 return
        return false;

    for (long nidx = 0; nidx < o_vec2frtChip_SearchROI.size(); nidx++)
        m_pPassiveAlignResult->Debug_vecrtDetailPassiveSearchROI.push_back(
            o_vec2frtChip_SearchROI[nidx][0].GetExtCRect());

    return true;
}

float CPassive3DAlgorithm::CheckOverPosbyCenterPos(
    const float i_fCenterPos, const float i_fSearchROIPos, const long i_nROIDirection)
{
    //CenterPos는 이미 px단위
    //px단위로 계산
    long nSearchROIPos = (long)(i_fSearchROIPos + .5f);

    //SearchROI Pos가 Center를 넘어가면 Center로부터 -1px을 준다
    if (i_nROIDirection == LEFT || i_nROIDirection == UP)
    {
        if (nSearchROIPos >= i_fCenterPos)
            return i_fCenterPos - 1;
    }
    else if (i_nROIDirection == RIGHT || i_nROIDirection == DOWN)
    {
        if (nSearchROIPos <= i_fCenterPos)
            return i_fCenterPos + 1;
    }

    return i_fSearchROIPos; //아무런 조건에 부합하지 않으면 그대로 사용
}

bool CPassive3DAlgorithm::Passive_Align_byEdge(const Ipvm::Image8u& i_imageHMap,
    std::vector<std::vector<FPI_RECT>> i_vec2frtChip_SearchROI, Ipvm::Rect32s& o_rtAlignReulst)
{
    std::vector<sEdgeSearchNeedInfo> vecsEdgeSearchNeedInfo;
    GetEdgeSearchNeedInfo(i_vec2frtChip_SearchROI, vecsEdgeSearchNeedInfo);
    std::vector<std::vector<Ipvm::Point32r2>> vec2fptEdgePoints(4);

    //1. Search ROI를 이용하여 영역을 정의
    std::vector<Ipvm::LineEq32r> vecSearchROI_DirLine(4);
    for (long nSearchROIDir = 0; nSearchROIDir < i_vec2frtChip_SearchROI.size(); nSearchROIDir++)
    {
        auto frtROIInfo = i_vec2frtChip_SearchROI[nSearchROIDir][0];
        Ipvm::Point32r2 fptCalcPoints[2];

        switch (nSearchROIDir)
        {
            case UP:
                fptCalcPoints[0] = frtROIInfo.fptLT;
                fptCalcPoints[1] = frtROIInfo.fptRT;
                break;
            case DOWN:
                fptCalcPoints[0] = frtROIInfo.fptLB;
                fptCalcPoints[1] = frtROIInfo.fptRB;
                break;
        }

        std::vector<Ipvm::Point32r2> vecfptCalclinePoint;
        vecfptCalclinePoint.push_back(fptCalcPoints[0]);
        vecfptCalclinePoint.push_back(fptCalcPoints[1]);

        CPI_Geometry::LineFitting_RemoveNoise(vecfptCalclinePoint, vecSearchROI_DirLine[nSearchROIDir]);
    }

    //2. Left, Right 최외각쪽 그림자를 이용하여 EdgePoint를 탐색해본다
    for (long nDir = LEFT; nDir < RIGHT + 1; nDir++)
    {
        float fOldEdgeThreshold = m_pEdgeDetect->SetMininumThreshold(
            GetEdgeThresholdValue(m_pPassiveInfoDB->ePassiveType, nDir, m_pPassiveInfoDB->fPassiveAngle));

        auto EdgeSearchInfo = vecsEdgeSearchNeedInfo[nDir];
        Ipvm::Point32r2 fptEdgePoint(0.f, 0.f);
        Ipvm::Point32s2 ptSearchStartPos(0, 0);
        for (long n = 0; n < EdgeSearchInfo.nROISize; n++)
        {
            switch (EdgeSearchInfo.nSearchDirection)
            {
                case LEFT:
                case RIGHT:
                    ptSearchStartPos
                        = Ipvm::Point32s2(EdgeSearchInfo.ptSearchStartPos.m_x, EdgeSearchInfo.ptSearchStartPos.m_y + n);
                    break;
            }

            if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(EdgeSearchInfo.nSearchType, ptSearchStartPos,
                    EdgeSearchInfo.nSearchDirection, EdgeSearchInfo.nSearchLength, EdgeSearchInfo.nSearchWidth,
                    i_imageHMap, fptEdgePoint, FALSE))
            {
                vec2fptEdgePoints[nDir].push_back(fptEdgePoint);
            }
        }

        m_pPassiveAlignResult->Debug_vecfptPassiveEdge.insert(m_pPassiveAlignResult->Debug_vecfptPassiveEdge.end(),
            vec2fptEdgePoints[nDir].begin(), vec2fptEdgePoints[nDir].end());

        m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);

        CPI_Geometry::LineFitting_RemoveNoise(vec2fptEdgePoints[nDir], vecSearchROI_DirLine[nDir]);
    }

    //3. 탐색된 EdgePoint와 Search ROI Point를 이용하여 Histogram ROI정의
    FPI_RECT frtThresholdArea;
    if (!GetfrtROI_byCrosspoint(vecSearchROI_DirLine, frtThresholdArea))
        return false;

    //4. 히스토그램을 이용하여 Peak Threshold값 구하기
    long nPeakNum(256);
    Ipvm::Image32s histogram;
    long* pnPeakID = new long[nPeakNum];
    long* pnPeakCount = new long[nPeakNum];
    long nCountThresh
        = (long)((float)frtThresholdArea.GetExtCRect().Width() * frtThresholdArea.GetExtCRect().Height() * .05f);

    CippModules::GetHistogramPeaks(
        i_imageHMap, frtThresholdArea.GetExtCRect(), histogram, pnPeakID, pnPeakCount, nPeakNum, nCountThresh, 5);
    long nSpecROIArea
        = (long)((m_pPassiveAlignResult->rtSpecChip.Width() * m_pPassiveAlignResult->rtSpecChip.Height()) * .2f);
    std::multimap<long, long, std::greater<long>> mapPeakInfo; //오름차순정렬
    for (long nPeakidx = 0; nPeakidx < nPeakNum; nPeakidx++)
    {
        auto PeakID = pnPeakID[nPeakidx];
        auto PeakCount = pnPeakCount[nPeakidx];

        if (PeakCount > nSpecROIArea)
        {
            mapPeakInfo.insert(std::make_pair(PeakCount, PeakID));
        }
    }

    delete[] pnPeakID;
    delete[] pnPeakCount;

    //5. Histogram ROI를 이용한 영역에 대하여 이진화
    Ipvm::Image8u imageBinaryTemp;
    if (!m_pVisionReusableMemory->GetInspByteImage(imageBinaryTemp))
        return false;

    imageBinaryTemp.FillZero();
    long nMaxPeakValue(0);
    for (auto PeakInfo : mapPeakInfo)
    {
        nMaxPeakValue = PeakInfo.second; //오름차순 정렬이니 첫번째Peak가 가장많은 Peak를 보유한 개수이다
        break;
    }

    //Threshold 여유를 준다
    BYTE byMinThresholdValue = (BYTE)nMaxPeakValue - 5;
    Ipvm::ImageProcessing::BinarizeGreater(
        i_imageHMap, frtThresholdArea.GetExtCRect(), byMinThresholdValue, imageBinaryTemp);

    //6. 원본영상과 이진화영상 &&
    Ipvm::Image8u imageAndTemp;
    if (!m_pVisionReusableMemory->GetInspByteImage(imageAndTemp))
        return false;

    imageAndTemp.FillZero();
    Ipvm::ImageProcessing::BitwiseAnd(i_imageHMap, imageBinaryTemp, frtThresholdArea.GetExtCRect(), imageAndTemp);

    //7. EdgeDetect
    for (long nDir = UP; nDir < DOWN + 1; nDir++)
    {
        float fOldEdgeThreshold = m_pEdgeDetect->SetMininumThreshold(
            GetEdgeThresholdValue(m_pPassiveInfoDB->ePassiveType, nDir, m_pPassiveInfoDB->fPassiveAngle));

        auto EdgeSearchInfo = vecsEdgeSearchNeedInfo[nDir];
        Ipvm::Point32r2 fptEdgePoint(0.f, 0.f);
        Ipvm::Point32s2 ptSearchStartPos(0, 0);
        for (long n = 0; n < EdgeSearchInfo.nROISize; n++)
        {
            switch (EdgeSearchInfo.nSearchDirection)
            {
                case UP:
                case DOWN:
                    ptSearchStartPos
                        = Ipvm::Point32s2(EdgeSearchInfo.ptSearchStartPos.m_x + n, EdgeSearchInfo.ptSearchStartPos.m_y);
                    break;
            }

            if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(EdgeSearchInfo.nSearchType, ptSearchStartPos,
                    EdgeSearchInfo.nSearchDirection, EdgeSearchInfo.nSearchLength, EdgeSearchInfo.nSearchWidth,
                    imageAndTemp, fptEdgePoint, FALSE))
            {
                vec2fptEdgePoints[nDir].push_back(fptEdgePoint);
            }
        }

        m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);
    }

    std::vector<Ipvm::LineEq32r> vecLine(4);
    for (long nidx = 0; nidx < vec2fptEdgePoints.size(); nidx++)
    {
        auto EdgePoints = vec2fptEdgePoints[nidx];

        std::vector<Ipvm::Point32r2> vecfptFittingEdges;
        CPI_Geometry::LineFitting_RemoveNoise(EdgePoints, vecfptFittingEdges, vecLine[nidx], 3.f);
        m_pPassiveAlignResult->Debug_vecfptPassiveEdge.insert(
            m_pPassiveAlignResult->Debug_vecfptPassiveEdge.end(), vecfptFittingEdges.begin(), vecfptFittingEdges.end());
    }

    FPI_RECT frtAlignResult;
    if (!GetfrtROI_byCrosspoint(vecLine, frtAlignResult))
        return false;

    o_rtAlignReulst = frtAlignResult.GetExtCRect();

    return true;
}

bool CPassive3DAlgorithm::GetfrtROI_byCrosspoint(std::vector<Ipvm::LineEq32r> i_vecLine, FPI_RECT& o_frtResultROI)
{
    if (i_vecLine.size() < 0) //BUG: 크기 조건 실수
        return false;

    // Left-Top
    if (Ipvm::Geometry::GetCrossPoint(i_vecLine[LEFT], i_vecLine[UP], o_frtResultROI.fptLT) != Ipvm::Status::e_ok)
        return false;

    // Left-Bottom
    if (Ipvm::Geometry::GetCrossPoint(i_vecLine[LEFT], i_vecLine[DOWN], o_frtResultROI.fptLB) != Ipvm::Status::e_ok)
        return false;

    // Right-Top
    if (Ipvm::Geometry::GetCrossPoint(i_vecLine[RIGHT], i_vecLine[UP], o_frtResultROI.fptRT) != Ipvm::Status::e_ok)
        return false;

    // Right-Bottom
    if (Ipvm::Geometry::GetCrossPoint(i_vecLine[RIGHT], i_vecLine[DOWN], o_frtResultROI.fptRB) != Ipvm::Status::e_ok)
        return false;

    return true;
}

bool CPassive3DAlgorithm::Passive_Align_byEdge(const VisionScale& scale, const Ipvm::Image8u& i_imageHMap,
    Ipvm::Rect32s i_rtRoughAlign, Ipvm::Rect32s i_rtRoughAlign_Body, Ipvm::Rect32s& o_rtAlignReulst,
    std::vector<std::vector<FPI_RECT>> i_vec2frtChip_SearchROI)
{
    o_rtAlignReulst = i_rtRoughAlign;

    if (i_imageHMap.GetMem() == nullptr)
        return false;

    int nSearchLength(0);
    int nElectordeWidth(0);

    float mm2px = scale.mmToPixelXY();

    if (m_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Capacitor
        || m_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_Register)
    {
        nSearchLength = (long)(m_pPassiveInfoDB->fPassiveSpecWidth_mm * 0.6f * mm2px); // 노이즈 때문에 0.5로 수정
        nElectordeWidth = (long)(m_pPassiveInfoDB->fPassiveElectrodeWidth_mm * mm2px);

        if (m_pPassiveInfoDB->fPassiveAngle == 90.f || m_pPassiveInfoDB->fPassiveAngle == 270.f)
            o_rtAlignReulst = Ipvm::Rect32s(i_rtRoughAlign_Body.m_left, i_rtRoughAlign_Body.m_top - nElectordeWidth,
                i_rtRoughAlign_Body.m_right, i_rtRoughAlign_Body.m_bottom + nElectordeWidth);
        else
            o_rtAlignReulst = Ipvm::Rect32s(i_rtRoughAlign_Body.m_left - nElectordeWidth, i_rtRoughAlign_Body.m_top,
                i_rtRoughAlign_Body.m_right + nElectordeWidth, i_rtRoughAlign_Body.m_bottom);
    }
    else
        return false;

    float fTolerence = 0.02f * mm2px + .5f; // 20um

    float fCompareSize(2.f);
    Ipvm::Rect32s rtSearchROI(0, 0, 0, 0);
    std::vector<Ipvm::Point32r2> vecfptEdge;
    float Re_FindEdgeROI_Factor(0.5f); //50% Extension
    Ipvm::Rect32r frtEdge = Ipvm::Rect32r((float)i_rtRoughAlign.m_left, (float)i_rtRoughAlign.m_top,
        (float)i_rtRoughAlign.m_right, (float)i_rtRoughAlign.m_bottom);
    if (m_pPassiveInfoDB->fPassiveAngle == 90.f || m_pPassiveInfoDB->fPassiveAngle == 270.f)
    {
        // Left Side
        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[LEFT][0].GetExtCRect(), LEFT,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[LEFT][0].GetExtCRect().Width()))
        {
            frtEdge.m_left = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, LEFT);
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[LEFT][0].GetExtCRect();
            rtSearchROI.m_left -= CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_right += CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, LEFT,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                    vecfptEdge, rtSearchROI.Width()))
            {
                frtEdge.m_left = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, LEFT);
            }
        }

        // Right Side
        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[RIGHT][0].GetExtCRect(), RIGHT,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[RIGHT][0].GetExtCRect().Width()))
        {
            frtEdge.m_right = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, RIGHT);
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[RIGHT][0].GetExtCRect();
            rtSearchROI.m_left -= CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_right += CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, RIGHT,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                    vecfptEdge, rtSearchROI.Width()))
            {
                frtEdge.m_right = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, RIGHT);
            }
        }

        // Top
        long nEdgePont = (long)i_rtRoughAlign.Width();
        if (nEdgePont < 0)
            return false;

        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[UP][0].GetExtCRect(), UP,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[UP][0].GetExtCRect().Width()))
        {
            frtEdge.m_top = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, UP);
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[UP][0].GetExtCRect();
            rtSearchROI.m_top -= CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_bottom += CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, UP,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize,
                    fCompareSize, vecfptEdge, rtSearchROI.Width()))
            {
                frtEdge.m_top = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, UP);
            }
        }

        // Bottom
        nEdgePont = (long)i_rtRoughAlign.Width();
        if (nEdgePont < 0)
            return false;

        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[DOWN][0].GetExtCRect(), DOWN,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[DOWN][0].GetExtCRect().Width()))
        {
            frtEdge.m_bottom = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, DOWN);
            frtEdge.m_bottom += 1.f;
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[DOWN][0].GetExtCRect();
            rtSearchROI.m_top -= CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_bottom += CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, DOWN,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize,
                    fCompareSize, vecfptEdge, rtSearchROI.Width()))
            {
                frtEdge.m_bottom = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, DOWN);
                frtEdge.m_bottom += 1.f;
            }
        }
        // }}
    }
    else
    {
        // Top Side
        long nEdgePont = (long)i_rtRoughAlign.Width();
        if (nEdgePont < 0)
            return false;

        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[UP][0].GetExtCRect(), UP,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[UP][0].GetExtCRect().Width()))
        {
            frtEdge.m_top = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, UP);
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[UP][0].GetExtCRect();
            rtSearchROI.m_top -= CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_bottom += CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, UP,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                    vecfptEdge, rtSearchROI.Width()))
            {
                frtEdge.m_top = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, UP);
            }
        }

        // Bottom Side
        nEdgePont = (long)i_rtRoughAlign.Width();
        if (nEdgePont < 0)
            return false;

        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[DOWN][0].GetExtCRect(), DOWN,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[DOWN][0].GetExtCRect().Width()))
        {
            frtEdge.m_bottom = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, DOWN);
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[DOWN][0].GetExtCRect();
            rtSearchROI.m_top -= CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_bottom += CAST_INT32T(rtSearchROI.Height() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, DOWN,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold), fCompareSize, fCompareSize,
                    vecfptEdge, rtSearchROI.Width()))
            {
                frtEdge.m_bottom = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, DOWN);
            }
        }

        // Left
        nEdgePont = (long)i_rtRoughAlign.Height();
        if (nEdgePont < 0)
            return false;

        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[LEFT][0].GetExtCRect(), LEFT,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[LEFT][0].GetExtCRect().Height()))
        {
            frtEdge.m_left = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, LEFT);
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[LEFT][0].GetExtCRect();
            rtSearchROI.m_left -= CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_right += CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, LEFT,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize,
                    fCompareSize, vecfptEdge, rtSearchROI.Height()))
            {
                frtEdge.m_left = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, LEFT);
            }
        }

        // Right
        nEdgePont = (long)i_rtRoughAlign.Height();
        if (nEdgePont < 0)
            return false;

        if (LineEdgeSearch(i_imageHMap, i_vec2frtChip_SearchROI[RIGHT][0].GetExtCRect(), RIGHT,
                FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize, fCompareSize,
                vecfptEdge, i_vec2frtChip_SearchROI[RIGHT][0].GetExtCRect().Height()))
        {
            frtEdge.m_right = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, RIGHT);
            frtEdge.m_right += 1.f;
        }
        else
        {
            rtSearchROI = i_vec2frtChip_SearchROI[RIGHT][0].GetExtCRect();
            rtSearchROI.m_left -= CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);
            rtSearchROI.m_right += CAST_INT32T(rtSearchROI.Width() * Re_FindEdgeROI_Factor);

            if (LineEdgeSearch(i_imageHMap, rtSearchROI, RIGHT,
                    FALSE, //뭐지?? Code상으로는 FALSE면 안되는데
                    CAST_FLOAT(m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold), fCompareSize,
                    fCompareSize, vecfptEdge, rtSearchROI.Height()))
            {
                frtEdge.m_right = GetFitEdge(vecfptEdge, m_pPassiveAlignResult, fTolerence, RIGHT);
                frtEdge.m_right += 1.f;
            }
        }
        // }}
    }

    o_rtAlignReulst.m_left = (int)(frtEdge.m_left + .5f);
    o_rtAlignReulst.m_top = (int)(frtEdge.m_top + .5f);
    o_rtAlignReulst.m_right = (int)(frtEdge.m_right + .5f);
    o_rtAlignReulst.m_bottom = (int)(frtEdge.m_bottom + .5f);

    if (m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
        SetElectrodeArea(scale, o_rtAlignReulst, m_pPassiveAlignResult->vecrtValidElect);

    return TRUE;
}

float CPassive3DAlgorithm::GetFitEdge(const std::vector<Ipvm::Point32r2>& Edges,
    SComponentAlignResult* PassiveAlignResult, const float& Tolerence, const long& EdgeDir)
{
    Ipvm::LineEq32r line(0.f, 0.f, 0.f);
    std::vector<Ipvm::Point32r2> vecfptFitEdge;
    if (!RemoveNoisebyLine(Edges, vecfptFitEdge, line, Tolerence))
        vecfptFitEdge = Edges; // 실패처리하지 말고
    PassiveAlignResult->Debug_vecfptPassiveEdge.insert(
        PassiveAlignResult->Debug_vecfptPassiveEdge.end(), vecfptFitEdge.begin(), vecfptFitEdge.end());
    long nValidDataNum = (long)vecfptFitEdge.size();

    float fEdge = 0.f;
    for (long nData = 0; nData < nValidDataNum; nData++)
    {
        switch (EdgeDir)
        {
            case LEFT:
            case RIGHT:
                fEdge += vecfptFitEdge[nData].m_x;
                break;
            case UP:
            case DOWN:
                fEdge += vecfptFitEdge[nData].m_y;
                break;
        }
    }

    return fEdge / (float)nValidDataNum;
}

bool CPassive3DAlgorithm::LineEdgeSearch(const Ipvm::Image8u& image, Ipvm::Rect32s i_rtROI, int i_nDir, long nEdgeType,
    float i_fAtLeastEdgeVal, float i_fCompareSizeX, float i_fCompareSizeY, std::vector<Ipvm::Point32r2>& vecptEdge,
    long i_nEdgeNum, BOOL bFirstEdge)
{
    float fEdgeMax, fEdge;
    vecptEdge.clear();

    long nBufferSize;
    Ipvm::Rect32s rtDiv;
    std::vector<Ipvm::Rect32s> vecrtDiv(i_nEdgeNum);
    float fDivSize;
    BOOL bHorz;

    long nCompareSizeX = (long)(i_fCompareSizeX + 0.5f);
    long nCompareSizeY = (long)(i_fCompareSizeY + 0.5f);

    nCompareSizeX = (std::max<long>)(nCompareSizeX, 1);
    nCompareSizeY = (std::max<long>)(nCompareSizeY, 1);

    if (bFirstEdge)
    {
        nCompareSizeX = nCompareSizeY = 1;
    }
    if (i_nDir == UP || i_nDir == DOWN)
    {
        bHorz = FALSE;
        nBufferSize = i_rtROI.Height();
        if (nCompareSizeY >= nBufferSize)
            return false;

        fDivSize = i_rtROI.Width() / (float)i_nEdgeNum;
        fDivSize = max(fDivSize, 1.f);
        for (long i = 0; i < i_nEdgeNum; i++)
        {
            rtDiv.m_top = i_rtROI.m_top;
            rtDiv.m_bottom = i_rtROI.m_bottom;
            rtDiv.m_left = (int)(i_rtROI.m_left + fDivSize * i + .5f);
            rtDiv.m_right = (int)(i_rtROI.m_left + fDivSize * (i + 1) + .5f);

            vecrtDiv[i] = rtDiv;
        }
    }
    else
    {
        bHorz = TRUE;
        nBufferSize = i_rtROI.Width();
        if (nCompareSizeX >= nBufferSize)
            return false;
        fDivSize = i_rtROI.Height() / (float)i_nEdgeNum;
        fDivSize = max(fDivSize, 1.f);
        for (long i = 0; i < i_nEdgeNum; i++)
        {
            rtDiv.m_top = (int)(i_rtROI.m_top + fDivSize * i + .5f);
            rtDiv.m_bottom = (int)(i_rtROI.m_top + fDivSize * (i + 1) + .5f);
            rtDiv.m_left = i_rtROI.m_left;
            rtDiv.m_right = i_rtROI.m_right;

            vecrtDiv[i] = rtDiv;
        }
    }

    //Ipvm::Image8u tempImage;
    //if (!m_pVisionReusableMemory->GetInspByteImage(tempImage))
    //	return false;

    //tempImage.FillZero();

    //auto* buffer = tempImage.GetMem();

    std::vector<uint8_t> buffer;

    BOOL bRisingEdge = (nEdgeType == 1);
    float fSum1, fSum2;
    for (long i = 0; i < i_nEdgeNum; i++)
    {
        long nEdge = -1;
        rtDiv = vecrtDiv[i];

        if (bHorz)
        {
            buffer.resize(rtDiv.Width());
            //if (Ipvm::ImageProcessing::MakeProjectionProfileAxisX(image, rtDiv, buffer) != Ipvm::Status::e_ok)
            if (Ipvm::ImageProcessing::MakeProjectionProfileAxisX(image, rtDiv, &buffer[0]) != Ipvm::Status::e_ok)
                return false;
        }
        else
        {
            buffer.resize(rtDiv.Height());
            //if (Ipvm::ImageProcessing::MakeProjectionProfileAxisY(image, rtDiv, buffer) != Ipvm::Status::e_ok)
            if (Ipvm::ImageProcessing::MakeProjectionProfileAxisY(image, rtDiv, &buffer[0]) != Ipvm::Status::e_ok)
                return false;
        }

        if (i_nDir == LEFT)
        {
            fEdgeMax = 0;
            for (long k = nBufferSize - nCompareSizeX - 1; k >= nCompareSizeX; k--)
            {
                fSum1 = fSum2 = 0.f;
                for (long n = 1; n <= nCompareSizeX; n++)
                {
                    fSum1 += buffer[k + n];
                    fSum2 += buffer[k - n];
                }
                if (nEdgeType == 2)
                {
                    fEdge = CAST_FLOAT(fabs(fSum2 - fSum1) / (float)nCompareSizeX);
                }
                else
                {
                    if (bRisingEdge)
                        fEdge = (fSum2 - fSum1) / (float)nCompareSizeX;
                    else
                        fEdge = (fSum1 - fSum2) / (float)nCompareSizeX;
                }
                if (fEdge > fEdgeMax)
                {
                    fEdgeMax = fEdge;
                    nEdge = i_rtROI.m_left + k;

                    if (bFirstEdge && fEdgeMax >= i_fAtLeastEdgeVal)
                    {
                        break;
                    }
                }
            }

            if (nEdge != -1 && fEdgeMax >= i_fAtLeastEdgeVal)
            {
                vecptEdge.push_back(Ipvm::Point32r2((float)nEdge, (float)rtDiv.CenterPoint().m_y));
            }
        }
        else if (i_nDir == RIGHT)
        {
            fEdgeMax = 0;
            for (long k = nCompareSizeX; k < nBufferSize - nCompareSizeX; k++)
            {
                fSum1 = fSum2 = 0.f;
                for (long n = 1; n <= nCompareSizeX; n++)
                {
                    fSum1 += buffer[k - n];
                    fSum2 += buffer[k + n];
                }
                if (nEdgeType == 2)
                {
                    fEdge = CAST_FLOAT(fabs(fSum2 - fSum1) / (float)nCompareSizeX);
                }
                else
                {
                    if (bRisingEdge)
                        fEdge = (fSum2 - fSum1) / (float)nCompareSizeX;
                    else
                        fEdge = (fSum1 - fSum2) / (float)nCompareSizeX;
                }
                if (fEdge > fEdgeMax)
                {
                    fEdgeMax = fEdge;
                    nEdge = i_rtROI.m_left + k;

                    if (bFirstEdge && fEdgeMax >= i_fAtLeastEdgeVal)
                    {
                        break;
                    }
                }
            }

            if (nEdge != -1 && fEdgeMax >= i_fAtLeastEdgeVal)
            {
                vecptEdge.push_back(Ipvm::Point32r2((float)nEdge, (float)rtDiv.CenterPoint().m_y));
            }
        }
        else if (i_nDir == UP)
        {
            fEdgeMax = 0;
            for (long k = nBufferSize - nCompareSizeY - 1; k >= nCompareSizeY; k--)
            {
                fSum1 = fSum2 = 0.f;
                for (long n = 1; n <= nCompareSizeY; n++)
                {
                    fSum1 += buffer[k + n];
                    fSum2 += buffer[k - n];
                }
                if (nEdgeType == 2)
                {
                    fEdge = CAST_FLOAT(fabs(fSum2 - fSum1) / (float)nCompareSizeY);
                }
                else
                {
                    if (bRisingEdge)
                        fEdge = (fSum2 - fSum1) / (float)nCompareSizeY;
                    else
                        fEdge = (fSum1 - fSum2) / (float)nCompareSizeY;
                }
                if (fEdge > fEdgeMax)
                {
                    fEdgeMax = fEdge;
                    nEdge = i_rtROI.m_top + k;
                    if (bFirstEdge && fEdgeMax >= i_fAtLeastEdgeVal)
                    {
                        break;
                    }
                }
            }

            if (nEdge != -1 && fEdgeMax >= i_fAtLeastEdgeVal)
                vecptEdge.push_back(Ipvm::Point32r2((float)rtDiv.CenterPoint().m_x, (float)nEdge));
        }
        else if (i_nDir == DOWN)
        {
            fEdgeMax = 0;
            for (long k = nCompareSizeY; k < nBufferSize - nCompareSizeY; k++)
            {
                fSum1 = fSum2 = 0.f;
                for (long n = 1; n <= nCompareSizeY; n++)
                {
                    fSum1 += buffer[k - n];
                    fSum2 += buffer[k + n];
                }
                if (nEdgeType == 2)
                {
                    fEdge = CAST_FLOAT(fabs(fSum2 - fSum1) / (float)nCompareSizeY);
                }
                else
                {
                    if (bRisingEdge)
                        fEdge = (fSum2 - fSum1) / (float)nCompareSizeY;
                    else
                        fEdge = (fSum1 - fSum2) / (float)nCompareSizeY;
                }
                if (fEdge > fEdgeMax)
                {
                    fEdgeMax = fEdge;
                    nEdge = i_rtROI.m_top + k;

                    if (bFirstEdge && fEdgeMax >= i_fAtLeastEdgeVal)
                    {
                        break;
                    }
                }
            }

            if (nEdge != -1 && fEdgeMax >= i_fAtLeastEdgeVal)
                vecptEdge.push_back(Ipvm::Point32r2((float)rtDiv.CenterPoint().m_x, (float)nEdge));
        }
        else
        {
            ASSERT(0);
            return false;
        }
    }

    if (vecptEdge.size() < 4)
        return false;

    return TRUE;
}

bool CPassive3DAlgorithm::GetEdgeSearchNeedInfo(const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI,
    std::vector<sEdgeSearchNeedInfo>& o_vecsEdgeSearchNeedInfo)
{
    if (o_vecsEdgeSearchNeedInfo.size() > 0)
        o_vecsEdgeSearchNeedInfo.clear();

    sEdgeSearchNeedInfo sEdgeSearchNeedInfo;
    for (long nDir = 0; nDir < 4; nDir++)
    {
        if (m_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
            sEdgeSearchNeedInfo.nSearchType = PI_ED_DIR_RISING;
        else
            sEdgeSearchNeedInfo.nSearchType = PI_ED_DIR_FALLING;

        if (m_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
        {
            switch (nDir)
            {
                case UP:
                    sEdgeSearchNeedInfo.nSearchDirection = DOWN;
                    break;
                case DOWN:
                    sEdgeSearchNeedInfo.nSearchDirection = UP;
                    break;
                case LEFT:
                    sEdgeSearchNeedInfo.nSearchDirection = RIGHT;
                    break;
                case RIGHT:
                    sEdgeSearchNeedInfo.nSearchDirection = LEFT;
                    break;
            }
        }
        else
            sEdgeSearchNeedInfo.nSearchDirection = nDir;

        sEdgeSearchNeedInfo.rtSearchROI = i_vec2frtDetailSearchROI[nDir][0].GetExtCRect();
        Ipvm::Point32s2 EdgeSearchSize = GetEdgeSearchSize(nDir, sEdgeSearchNeedInfo.rtSearchROI);
        sEdgeSearchNeedInfo.nSearchWidth = EdgeSearchSize.m_x;
        sEdgeSearchNeedInfo.nSearchLength = EdgeSearchSize.m_y;
        sEdgeSearchNeedInfo.ptSearchStartPos = GetEdgeSearch_StartPoint(nDir, sEdgeSearchNeedInfo.rtSearchROI);

        switch (nDir)
        {
            case UP:
            case DOWN:
                sEdgeSearchNeedInfo.nROISize = sEdgeSearchNeedInfo.rtSearchROI.Width();
                break;
            case LEFT:
            case RIGHT:
                sEdgeSearchNeedInfo.nROISize = sEdgeSearchNeedInfo.rtSearchROI.Height();
                break;
        }

        o_vecsEdgeSearchNeedInfo.push_back(sEdgeSearchNeedInfo);
    }

    return true;
}

Ipvm::Point32s2 CPassive3DAlgorithm::GetEdgeSearchSize(long i_nCurDirection, Ipvm::Rect32s i_rtSearchROI)
{
    Ipvm::Point32s2 EdgeSearchSize(0, 0);

    switch (i_nCurDirection)
    {
        case UP:
        case DOWN:
            EdgeSearchSize.m_x = long((i_rtSearchROI.Width() * .1f) * 1.5f);
            EdgeSearchSize.m_y = i_rtSearchROI.Height();
            break;
        case LEFT:
        case RIGHT:
            EdgeSearchSize.m_x = long((i_rtSearchROI.Height() * .1f) * 1.5f);
            EdgeSearchSize.m_y = i_rtSearchROI.Width();
            break;
    }

    return EdgeSearchSize;
}

Ipvm::Point32s2 CPassive3DAlgorithm::GetEdgeSearch_StartPoint(long i_SearchDir, Ipvm::Rect32s i_rtSearchROI)
{
    Ipvm::Point32s2 ptSearch_StartPos(0, 0);

    switch (i_SearchDir)
    {
        case UP:
            ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
            ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
            break;
        case DOWN:
            ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
            ptSearch_StartPos.m_y = i_rtSearchROI.m_bottom;
            break;
        case LEFT:
            ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
            ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
            break;
        case RIGHT:
            ptSearch_StartPos.m_x = i_rtSearchROI.m_right;
            ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
            break;
    }

    return ptSearch_StartPos;
}

float CPassive3DAlgorithm::GetEdgeThresholdValue(
    enumPassiveTypeDefine i_enumPassiveType, long i_nDir, float i_fPassiveAngle)
{
    float fEdgeThresholdValue(0.f);

    if (i_enumPassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
        return (float)m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold;

    switch (i_nDir)
    {
        case UP:
        case DOWN:
            if (i_fPassiveAngle == 90.f || i_fPassiveAngle == 270.f) //Vertical
                fEdgeThresholdValue = (float)m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold;
            else //Horizontal
                fEdgeThresholdValue = (float)m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold;
            break;
        case LEFT:
        case RIGHT:
            if (i_fPassiveAngle == 0.f || i_fPassiveAngle == 180.f) //Horizontal
                fEdgeThresholdValue = (float)m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold;
            else //Vertical
                fEdgeThresholdValue = (float)m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold;
            break;
    }

    return fEdgeThresholdValue;
}

bool CPassive3DAlgorithm::RemoveNoisebyLine(const std::vector<Ipvm::Point32r2>& i_vecptEdge,
    std::vector<Ipvm::Point32r2>& o_vecptFitEdge, Ipvm::LineEq32r& o_line, float fTolerence)
{
    if (i_vecptEdge.size() < 2)
        return false;
    else if (i_vecptEdge.size() == 2)
    {
        o_vecptFitEdge = i_vecptEdge;

        return (0
            == CPI_Geometry::mMakeLineByTwoPoints(o_line, (float)i_vecptEdge[0].m_x, (float)i_vecptEdge[0].m_y,
                (float)i_vecptEdge[1].m_x, (float)i_vecptEdge[1].m_y));
    }

    Ipvm::LineEq32r line;
    if (!CPI_Geometry::LineFitting_RemoveNoise(i_vecptEdge, o_vecptFitEdge, line, 3.f))
        return false;

    long nEdgeOrig = (long)i_vecptEdge.size();

    o_vecptFitEdge.clear();

    // 피팅라인에서 벗어난 넘 Filtering
    for (long i = 0; i < (long)i_vecptEdge.size(); i++)
    {
        float det = CAST_FLOAT(sqrt(line.m_a * line.m_a + line.m_b * line.m_b));
        if (det < FLT_MIN)
            return false;

        float fDist = CAST_FLOAT(fabs(line.m_a * i_vecptEdge[i].m_x + line.m_b * i_vecptEdge[i].m_y + line.m_c)) / det;
        if (fabs(fDist) < fTolerence)
            o_vecptFitEdge.push_back(i_vecptEdge[i]);
    }

    if (o_vecptFitEdge.size() < 4)
        return false;

    if (o_vecptFitEdge.size() < nEdgeOrig)
    {
        auto temp = o_vecptFitEdge;
        if (!CPI_Geometry::LineFitting_RemoveNoise(temp, o_vecptFitEdge, o_line))
            return false;
    }
    else
        o_line = line;

    return true;
}

bool CPassive3DAlgorithm::SetElectrodeArea(
    const VisionScale& scale, const Ipvm::Rect32s& i_rtAlignResult, std::vector<Ipvm::Rect32s>& o_vecrtElectrodeResult)
{
    m_pPassiveAlignResult->rtValidBody = i_rtAlignResult;

    float mm2px = scale.mmToPixelXY();

    long nElectNum = (long)m_pPassiveAlignResult->vecrtResElect.size();
    o_vecrtElectrodeResult.resize(nElectNum);

    for (long nElect = 0; nElect < nElectNum; nElect++)
        o_vecrtElectrodeResult[nElect] = i_rtAlignResult;

    float fElectrodeWidth = m_pPassiveInfoDB->fPassiveElectrodeWidth_mm * mm2px;

    if (m_pPassiveInfoDB->fPassiveAngle == 90.f)
    {
        // 20190411 전극 width spec이 comp의 40% 이상 크다면 전극 width를 component의 25%로 만든다
        float fComponentWidth = (float)i_rtAlignResult.Height();

        if (fComponentWidth * 0.4 < fElectrodeWidth)
            fElectrodeWidth = fComponentWidth * 0.25f;

        m_pPassiveAlignResult->rtValidBody.m_top = (int)(i_rtAlignResult.m_top + fElectrodeWidth + .5f);
        m_pPassiveAlignResult->rtValidBody.m_bottom = (int)(i_rtAlignResult.m_bottom - fElectrodeWidth + .5f);
        o_vecrtElectrodeResult[0].m_bottom = m_pPassiveAlignResult->rtValidBody.m_top;
        o_vecrtElectrodeResult[1].m_top = m_pPassiveAlignResult->rtValidBody.m_bottom;
    }
    else
    {
        float fComponentWidth = (float)i_rtAlignResult.Width();

        if (fComponentWidth * 0.4 < fElectrodeWidth)
            fElectrodeWidth = fComponentWidth * 0.25f;

        m_pPassiveAlignResult->rtValidBody.m_left = (int)(i_rtAlignResult.m_left + fElectrodeWidth + .5f);
        m_pPassiveAlignResult->rtValidBody.m_right = (int)(i_rtAlignResult.m_right - fElectrodeWidth + .5f);
        o_vecrtElectrodeResult[0].m_right = m_pPassiveAlignResult->rtValidBody.m_left;
        o_vecrtElectrodeResult[1].m_left = m_pPassiveAlignResult->rtValidBody.m_right;
    }

    // WSI TEST
    for (long nElect = 0; nElect < (long)o_vecrtElectrodeResult.size(); nElect++)
    {
        Ipvm::Rect32s rtElect = o_vecrtElectrodeResult[nElect];

        float fCalcElectWidthRatio = m_pPassiveAlignParam->m_nCalc_Electrode_Area_Width_Ratio / 100.f;
        float fCalcElectLengthRatio = m_pPassiveAlignParam->m_nCalc_Electrode_Area_Length_Ratio / 100.f;

        if (m_pPassiveInfoDB->fPassiveAngle == 90.f)
        {
            float fElectrodeLength = (float)rtElect.Width() * fCalcElectLengthRatio;
            float fReduceElectrodeWidth = (float)rtElect.Height() * (1.f - fCalcElectWidthRatio);
            Ipvm::Point32r2 fptCenter
                = Ipvm::Point32r2((float)rtElect.CenterPoint().m_x, (float)rtElect.CenterPoint().m_y);

            o_vecrtElectrodeResult[nElect].m_left = (int)(fptCenter.m_x - fElectrodeLength / 2.f + .5f);
            o_vecrtElectrodeResult[nElect].m_right = (int)(fptCenter.m_x + fElectrodeLength / 2.f - .5f);

            if (nElect == 0)
                o_vecrtElectrodeResult[nElect].m_top
                    = (int)(o_vecrtElectrodeResult[nElect].m_top + fReduceElectrodeWidth + .5f);

            if (nElect == 1)
                o_vecrtElectrodeResult[nElect].m_bottom
                    = (int)(o_vecrtElectrodeResult[nElect].m_bottom - fReduceElectrodeWidth - .5f);
        }
        else
        {
            float fElectrodeLength = (float)rtElect.Height() * fCalcElectLengthRatio;
            float fReduceElectrodeWidth = (float)rtElect.Width() * (1.f - fCalcElectWidthRatio);
            Ipvm::Point32r2 fptCenter
                = Ipvm::Point32r2((float)rtElect.CenterPoint().m_x, (float)rtElect.CenterPoint().m_y);

            o_vecrtElectrodeResult[nElect].m_top = (int)(fptCenter.m_y - fElectrodeLength / 2.f + .5f);
            o_vecrtElectrodeResult[nElect].m_bottom = (int)(fptCenter.m_y + fElectrodeLength / 2.f - .5f);

            if (nElect == 0)
                o_vecrtElectrodeResult[nElect].m_left
                    = (int)(o_vecrtElectrodeResult[nElect].m_left + fReduceElectrodeWidth + .5f);

            if (nElect == 1)
                o_vecrtElectrodeResult[nElect].m_right
                    = (int)(o_vecrtElectrodeResult[nElect].m_right - fReduceElectrodeWidth - .5f);
        }
    }

    return true;
}

std::vector<Ipvm::Rect32s> CPassive3DAlgorithm::GetSubstrateMesureROI(const VisionScale& scale,
    const Ipvm::Rect32s& i_rtAlignResult, const long& Area_Size_um, const long& Area_GapX_From_Body_um,
    const long& Area_GapY_From_Body_um, const long& Area_GapX_um, const long& Area_GapY_um)
{
    std::vector<Ipvm::Rect32s> vecSubstrateMesureROI(4);

    if (m_pPassiveInfoDB == nullptr)
        return vecSubstrateMesureROI;

    const auto& um2px = scale.umToPixel();

    float fSubstrateMesureROI_GapX_ForBody_px = Area_GapX_From_Body_um * um2px.m_x;
    float fSubstrateMesureROI_GapY_ForBody_px = Area_GapY_From_Body_um * um2px.m_y;

    Ipvm::Point32r2 fptSubstrateMesureROI_LT
        = Ipvm::Point32r2((float)i_rtAlignResult.m_left, (float)i_rtAlignResult.m_top);
    Ipvm::Point32r2 fptSubstrateMesureROI_RT
        = Ipvm::Point32r2((float)i_rtAlignResult.m_right, (float)i_rtAlignResult.m_top);
    Ipvm::Point32r2 fptSubstrateMesureROI_LB
        = Ipvm::Point32r2((float)i_rtAlignResult.m_left, (float)i_rtAlignResult.m_bottom);
    Ipvm::Point32r2 fptSubstrateMesureROI_RB
        = Ipvm::Point32r2((float)i_rtAlignResult.m_right, (float)i_rtAlignResult.m_bottom);

    float SubstrateMesureROI_Half_Size_um = Area_Size_um * .5f;
    Ipvm::Point32r2 SubstrateMesureROI_Half_Size_px(
        SubstrateMesureROI_Half_Size_um * um2px.m_x, SubstrateMesureROI_Half_Size_um * um2px.m_y);

    // LT
    vecSubstrateMesureROI[2].m_left = int(
        ((fptSubstrateMesureROI_LT.m_x - SubstrateMesureROI_Half_Size_px.m_x) - fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[2].m_right = int(
        ((fptSubstrateMesureROI_LT.m_x + SubstrateMesureROI_Half_Size_px.m_x) - fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[2].m_top = int(
        ((fptSubstrateMesureROI_LT.m_y - SubstrateMesureROI_Half_Size_px.m_y) - fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[2].m_bottom = int(
        ((fptSubstrateMesureROI_LT.m_y + SubstrateMesureROI_Half_Size_px.m_y) - fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);
    // LB
    vecSubstrateMesureROI[3].m_left = int(
        ((fptSubstrateMesureROI_LB.m_x - SubstrateMesureROI_Half_Size_px.m_x) - fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[3].m_right = int(
        ((fptSubstrateMesureROI_LB.m_x + SubstrateMesureROI_Half_Size_px.m_x) - fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[3].m_top = int(
        ((fptSubstrateMesureROI_LB.m_y - SubstrateMesureROI_Half_Size_px.m_y) + fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[3].m_bottom = int(
        ((fptSubstrateMesureROI_LB.m_y + SubstrateMesureROI_Half_Size_px.m_y) + fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);
    // RT
    vecSubstrateMesureROI[0].m_left = int(
        ((fptSubstrateMesureROI_RT.m_x - SubstrateMesureROI_Half_Size_px.m_x) + fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[0].m_right = int(
        ((fptSubstrateMesureROI_RT.m_x + SubstrateMesureROI_Half_Size_px.m_x) + fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[0].m_top = int(
        ((fptSubstrateMesureROI_RT.m_y - SubstrateMesureROI_Half_Size_px.m_y) - fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[0].m_bottom = int(
        ((fptSubstrateMesureROI_RT.m_y + SubstrateMesureROI_Half_Size_px.m_y) - fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);
    // RB
    vecSubstrateMesureROI[1].m_left = int(
        ((fptSubstrateMesureROI_RB.m_x - SubstrateMesureROI_Half_Size_px.m_x) + fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[1].m_right = int(
        ((fptSubstrateMesureROI_RB.m_x + SubstrateMesureROI_Half_Size_px.m_x) + fSubstrateMesureROI_GapX_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[1].m_top = int(
        ((fptSubstrateMesureROI_RB.m_y - SubstrateMesureROI_Half_Size_px.m_y) + fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);
    vecSubstrateMesureROI[1].m_bottom = int(
        ((fptSubstrateMesureROI_RB.m_y + SubstrateMesureROI_Half_Size_px.m_y) + fSubstrateMesureROI_GapY_ForBody_px)
        + .5f);

    long nSubstrateMesureROI_ShiftX_px = long((Area_GapX_um * um2px.m_x) + .5f);
    long nSubstrateMesureROI_ShiftY_px = long((Area_GapY_um * um2px.m_y) * .5f);

    for (long nROICount = 0; nROICount < vecSubstrateMesureROI.size(); nROICount++)
        vecSubstrateMesureROI[nROICount]
            += Ipvm::Point32s2(nSubstrateMesureROI_ShiftX_px, nSubstrateMesureROI_ShiftY_px);

    return vecSubstrateMesureROI;
}

bool CPassive3DAlgorithm::CalcPassiveData(
    const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap, const Ipvm::Image16u& i_wInten, const bool i_bisMIAType)
{
    float fMiddleCalcRangeMin = (float)(m_pPassiveAlignParam->m_fValidZRange * .5f);
    float fMiddleCalcRangeMax = (float)(m_pPassiveAlignParam->m_fValidZRange * .5f);

    if (m_pPassiveAlignParam->bMeasureComp)
    {
        if (!GetPassive_Height(i_Hmap, i_Zmap, i_wInten, fMiddleCalcRangeMin, fMiddleCalcRangeMax, i_bisMIAType))
            return false;
    }

    if (m_pPassiveAlignParam->bMeasureElect
        && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        if (!GetPassiveElectrode_Height(i_Hmap, i_Zmap, i_wInten, fMiddleCalcRangeMin, fMiddleCalcRangeMax))
            return false;
    }

    if (m_pPassiveAlignParam->bMeasureBody
        && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        if (!GetPassive_Body_Height(i_Hmap, i_Zmap, i_wInten, fMiddleCalcRangeMin, fMiddleCalcRangeMax))
            return false;
    }

    return true;
}

bool CPassive3DAlgorithm::GetPassive_Height(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap,
    const Ipvm::Image16u& i_wInten, float i_fMinimumVailidRange, float i_fMaximumVailidRange, const bool i_bisMIAType)
{
    m_pPassiveAlignResult->fCompHeight = Ipvm::k_noiseValue32r;
    m_pPassiveAlignResult->fCompHeightRatio = Ipvm::k_noiseValue32r;

    float fSpecThickness = m_pPassiveInfoDB->fPassiveHeight_mm * 1000.f;

    float fSubstrateAvgHeight
        = GetSubstrateAvgHeight(i_Hmap, i_Zmap, i_wInten, i_fMinimumVailidRange, i_fMaximumVailidRange);

    // Component HPosition
    long nCompValidDataNum(0);
    Ipvm::Rect32s rtSearchComp = m_pPassiveAlignResult->rtValidComponent;

    float fAlignAreaPassiveHeight = CalcZData(i_Hmap, i_Zmap, rtSearchComp, i_fMinimumVailidRange,
        i_fMaximumVailidRange, nCompValidDataNum, &i_wInten, i_bisMIAType, false);
    m_pPassiveAlignResult->Debug_fAlignAreaPassiveHeight = fAlignAreaPassiveHeight;

    // Measurement Component Height
    m_pPassiveAlignResult->fCompHeight = NC_SUB(fAlignAreaPassiveHeight, fSubstrateAvgHeight);
    m_pPassiveAlignResult->fCompHeightRatio = NC_MUL(NC_DIV(m_pPassiveAlignResult->fCompHeight, fSpecThickness), 100.f);
    m_pPassiveAlignResult->fValidCompRatio
        = (float)nCompValidDataNum / (rtSearchComp.Width() * rtSearchComp.Height()) * 100.f;

    return true;
}

float CPassive3DAlgorithm::GetSubstrateAvgHeight(const Ipvm::Image8u& i_byZmap, const Ipvm::Image32r& i_Zmap,
    const Ipvm::Image16u& i_wInten, float i_fMinimumVailidRange, float i_fMaximumVailidRange)
{
    float fSubstrateAvgHeight(0.f);
    long nValidDataNum(0), nValidCount(0);
    long nSubstrateROINum = (long)m_pPassiveAlignResult->vecrtSubstrateArea.size();

    std::vector<float> vecfSubstrateHeight(nSubstrateROINum);
    std::vector<long> vecnSubstrateValidDataNum(nSubstrateROINum);

    m_pPassiveAlignResult->Debug_vecfPassive_Calc_SubstrateArea_Height_Ratio.resize(nSubstrateROINum);

    for (long nROICount = 0; nROICount < nSubstrateROINum; nROICount++)
    {
        Ipvm::Rect32s rtSearchLand = m_pPassiveAlignResult->vecrtSubstrateArea[nROICount];
        long nSearchArea = rtSearchLand.Width() * rtSearchLand.Height();

        vecfSubstrateHeight[nROICount] = CalcZForSubstrate(i_Zmap, rtSearchLand, i_fMinimumVailidRange,
            i_fMaximumVailidRange, vecnSubstrateValidDataNum[nROICount], &i_wInten);

        float fValidArea = (float)vecnSubstrateValidDataNum[nROICount] / (float)nSearchArea * 100.f;

        m_pPassiveAlignResult->Debug_vecfPassive_Calc_SubstrateArea_Height_Ratio[nROICount] = fValidArea;

        if (fValidArea >= 10.f)
        {
            fSubstrateAvgHeight += vecfSubstrateHeight[nROICount];
            nValidDataNum += vecnSubstrateValidDataNum[nROICount];
            nValidCount++;
        }
    }

    //{{//kircheis_Compo_AntiShadow
    if (nValidCount < 1) //집적도 높은 소자들의 경우 소자 주변 일반적인 획득 ROI내의 SR면 높이 Data가 없다.
    { //이럴 경우에 소자의 Search ROI 내부를 검색하여 SR 높이 Data를 끌어 모은다.
        Ipvm::Rect32s rtSearchROI = m_pPassiveAlignResult->Debug_rtRoughAlign_SearchROI;
        Ipvm::Rect32s rtCopyROI = rtSearchROI;
        rtCopyROI.InflateRect(3, 3);
        Ipvm::Rect32s rtCapROI = m_pPassiveAlignResult->rtResChip;

        Ipvm::Image8u tempByZMap;
        if (!m_pVisionReusableMemory->GetInspByteImage(tempByZMap))
            return FALSE;

        Ipvm::ImageProcessing::Copy(i_byZmap, rtCopyROI, tempByZMap);
        Ipvm::ImageProcessing::Fill(rtCapROI, 0, tempByZMap);

        long nSearchArea = rtSearchROI.Width() * rtSearchROI.Height();

        vecfSubstrateHeight[0] = CalcZForSubstrate(
            i_Zmap, rtSearchROI, i_fMinimumVailidRange, i_fMaximumVailidRange, vecnSubstrateValidDataNum[0], &i_wInten);

        float fValidArea = (float)vecnSubstrateValidDataNum[0] / (float)nSearchArea * 100.f;
        m_pPassiveAlignResult->Debug_vecfPassive_Calc_SubstrateArea_Height_Ratio[0] = fValidArea;

        if (fValidArea >= 10.f * .5f)
        {
            fSubstrateAvgHeight += vecfSubstrateHeight[0];
            nValidDataNum += vecnSubstrateValidDataNum[0];
            nValidCount++;
        }
    }
    //}}

    m_pPassiveAlignResult->Debug_vecfPassive_Calc_SubstrateArea_Height = vecfSubstrateHeight;

    return fSubstrateAvgHeight /= (float)nValidCount;
}

bool CPassive3DAlgorithm::GetPassiveElectrode_Height(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap,
    const Ipvm::Image16u& i_wInten, float i_fMinimumVailidRange, float i_fMaximumVailidRange)
{
    float fValidAreaSpec(10.f); // 최소한의 데이터만 있어도 계산.
    std::vector<float> vecfSubstrateHeight;
    for (long nROICount = 0; nROICount < (long)m_pPassiveAlignResult->vecrtSubstrateArea.size(); nROICount++)
    {
        Ipvm::Rect32s rtSubstrateArea = m_pPassiveAlignResult->vecrtSubstrateArea[nROICount];

        long nSearchArea = rtSubstrateArea.Width() * rtSubstrateArea.Height();

        long nValidDataNum(0);

        // Calculate Height
        float fSubstrateHeight = CalcZForSubstrate(
            i_Zmap, rtSubstrateArea, i_fMinimumVailidRange, i_fMaximumVailidRange, nValidDataNum, &i_wInten);

        float fValidArea = (float)nValidDataNum / (float)nSearchArea * 100.f;
        if (fValidArea >= fValidAreaSpec)
        {
            m_pPassiveAlignResult->Debug_vecfElectrode_Calc_SubstrateArea_Height.push_back(fSubstrateHeight);
            vecfSubstrateHeight.push_back(fSubstrateHeight);
        }
    }

    float fSummarySubstrateHeight(0.f);
    for (long nData = 0; nData < (long)vecfSubstrateHeight.size(); nData++)
        fSummarySubstrateHeight += vecfSubstrateHeight[nData];

    if ((long)vecfSubstrateHeight.size() <= 0)
        return false;

    float fAvgSubstrateHeight = fSummarySubstrateHeight / (float)vecfSubstrateHeight.size();

    m_pPassiveAlignResult->Debug_fSubstrate_Avg_Height_Electrode = fAvgSubstrateHeight;

    long nElectNum = (long)m_pPassiveAlignResult->vecrtValidElect.size();
    std::vector<float> vecfSacaleRatio(nElectNum);
    std::vector<long> vecnElectValidDataNum(nElectNum);

    /// Calculate Height WSI 사용 안함
    std::vector<float> vecfElectrodeAreaHeight;
    for (long nElect = 0; nElect < nElectNum; nElect++)
    {
        vecfSacaleRatio[nElect] = 1.f;

        Ipvm::Rect32s rtValidElect = m_pPassiveAlignResult->vecrtValidElect[nElect];
        //long nSearchArea = rtValidElect.Width() * rtValidElect.Height();

        long nValidDataNum = 0;

        // Calculate Height
        float fElectrodeHeight = CalcZData(i_Hmap, i_Zmap, rtValidElect, i_fMinimumVailidRange, i_fMaximumVailidRange,
            nValidDataNum, &i_wInten, false, false);

        vecfElectrodeAreaHeight.push_back(fElectrodeHeight);
        m_pPassiveAlignResult->Debug_vecfElectrode_Area_Height.push_back(fElectrodeHeight);

        //float fValidArea = (float)nValidDataNum / (float)nSearchArea * 100.f;
    }

    // Measurement Electrode_Height
    m_pPassiveAlignResult->vecfElectrodeHeight.clear();
    m_pPassiveAlignResult->vecfElectrodeHeight.resize(nElectNum, Ipvm::k_noiseValue32r);

    for (long nElect = 0; nElect < vecfElectrodeAreaHeight.size(); nElect++)
    {
        if (vecfElectrodeAreaHeight[nElect] == Ipvm::k_noiseValue32r || fAvgSubstrateHeight == Ipvm::k_noiseValue32r)
            continue;

        m_pPassiveAlignResult->vecfElectrodeHeight[nElect] = (vecfElectrodeAreaHeight[nElect] - fAvgSubstrateHeight);
        m_pPassiveAlignResult->vecfElectrodeHeight[nElect] *= vecfSacaleRatio[nElect];
    }

    // Measurement Electrode_Tilt
    std::vector<float> vecfElectrodeHeight = m_pPassiveAlignResult->vecfElectrodeHeight;
    sort(vecfElectrodeHeight.begin(), vecfElectrodeHeight.end());

    m_pPassiveAlignResult->fElectTiltHeight = NC_SUB(vecfElectrodeHeight[nElectNum - 1], vecfElectrodeHeight[0]);

    return true;
}

bool CPassive3DAlgorithm::GetPassive_Body_Height(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_Zmap,
    const Ipvm::Image16u& i_wInten, float i_fMinimumVailidRange, float i_fMaximumVailidRange)
{
    // Split Body 영역 설정.
    long nSplitNumH(2);
    long nSplitNumV(2);

    GetSplitBodyROI(nSplitNumH, nSplitNumV, m_pPassiveAlignResult->rtValidBody, m_pPassiveAlignResult->vecrtSplitBody);

    // 바닥면 다시 계산 ..
    float fValidAreaSpec(10.f); // 최소한의 데이터만 있어도 계산.

    float fLand(0.f);
    long nValidCount(0);
    long nSubstrateNum = (long)m_pPassiveAlignResult->vecrtSubstrateArea.size();
    std::vector<float> vecfSubstrateData(nSubstrateNum);
    std::vector<long> vecnSubstrateValidDataNum(nSubstrateNum);
    m_pPassiveAlignResult->Debug_vecfBody_Calc_SubstrateArea_Height_Ratio.resize(nSubstrateNum);
    for (long nSubstrateidx = 0; nSubstrateidx < nSubstrateNum; nSubstrateidx++)
    {
        Ipvm::Rect32s rtSearchLand = m_pPassiveAlignResult->vecrtSubstrateArea[nSubstrateidx];

        // Height Calculate
        float fSubstrateHeight = CalcZForSubstrate(i_Zmap, rtSearchLand, i_fMinimumVailidRange, i_fMaximumVailidRange,
            vecnSubstrateValidDataNum[nSubstrateidx], &i_wInten);
        vecfSubstrateData.push_back(fSubstrateHeight);
        m_pPassiveAlignResult->Debug_vecfBody_Calc_SubstrateArea_Height.push_back(fSubstrateHeight);

        long nSearchArea = rtSearchLand.Width() * rtSearchLand.Height();
        float fValidArea = (float)vecnSubstrateValidDataNum[nSubstrateidx] / (float)nSearchArea * 100.f;
        m_pPassiveAlignResult->Debug_vecfBody_Calc_SubstrateArea_Height_Ratio[nSubstrateidx] = fValidArea;
        if (fValidArea >= fValidAreaSpec)
        {
            fLand += m_pPassiveAlignResult->Debug_vecfBody_Calc_SubstrateArea_Height[nSubstrateidx];
            nValidCount++;
        }
    }

    // Body
    long nBodyValidDataNum(0);
    Ipvm::Rect32s rtSearchBody = m_pPassiveAlignResult->rtValidBody;
    rtSearchBody.DeflateRect(1, 1, 1, 1);

    float fBody = CalcZData(i_Hmap, i_Zmap, rtSearchBody, i_fMinimumVailidRange, i_fMaximumVailidRange,
        nBodyValidDataNum, &i_wInten, false, false);

    // 무조건 전극 높이 구할 때 계산된 바닥 정보 이용.
    BOOL bWidePlane = TRUE;
    if (m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
        fLand = m_pPassiveAlignResult->Debug_fSubstrate_Avg_Height_Electrode;

    long nSplitNum = (long)m_pPassiveAlignResult->vecrtSplitBody.size();

    nValidCount = 0;
    float fTotalBody = 0.f;
    m_pPassiveAlignResult->vecrtSplitBody.resize(nSplitNum);
    m_pPassiveAlignResult->vecfSplitBody_Height.resize(nSplitNum);
    m_pPassiveAlignResult->Debug_vecfValidSplitBodyRatio.resize(nSplitNum);
    std::vector<long> vecnBodyValidDataNum(nSplitNum);

    bool findInvalidValue(false);
    for (long nBody = 0; nBody < nSplitNum; nBody++)
    {
        Ipvm::Rect32s rtSearchSplitBody = m_pPassiveAlignResult->vecrtSplitBody[nBody];

        rtSearchSplitBody.NormalizeRect();
        if (rtSearchSplitBody.m_left <= 0 || rtSearchSplitBody.m_top <= 0 || rtSearchSplitBody.m_right <= 0
            || rtSearchSplitBody.m_bottom <= 0)
            continue;
        if (rtSearchSplitBody.Width() <= 0 || rtSearchSplitBody.Height() <= 0)
            continue;

        m_pPassiveAlignResult->vecrtSplitBody[nBody] = rtSearchSplitBody;
        long nSearchArea = rtSearchSplitBody.Width() * rtSearchSplitBody.Height();
        // Height Calculate
        m_pPassiveAlignResult->vecfSplitBody_Height[nBody] = CalcZData(i_Hmap, i_Zmap, rtSearchSplitBody,
            i_fMinimumVailidRange, i_fMaximumVailidRange, vecnBodyValidDataNum[nBody], &i_wInten, false, false);
        float fValidArea = (float)vecnBodyValidDataNum[nBody] / (float)nSearchArea * 100.f;
        m_pPassiveAlignResult->Debug_vecfValidSplitBodyRatio[nBody] = fValidArea;
        if (fValidArea >= fValidAreaSpec)
        {
            fTotalBody += m_pPassiveAlignResult->vecfSplitBody_Height[nBody];
            nValidCount++;
        }
        else
            findInvalidValue = true;
    }

    if (nValidCount > 0)
        fTotalBody /= (float)nValidCount;

    // Measurement Body_Height & Body_Tilt
    float fGV_H0 = Ipvm::k_noiseValue32r;
    float fGV_H1 = Ipvm::k_noiseValue32r;
    float fGV_H2 = Ipvm::k_noiseValue32r;
    float fGV_H3 = Ipvm::k_noiseValue32r;

    // Body_Height
    if (bWidePlane)
    {
        // 대충 계산한다.
        fGV_H0 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[0], fLand);
        fGV_H1 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[1], fLand);
        fGV_H2 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[2], fLand);
        fGV_H3 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[3], fLand);
    }
    else
    {
        fGV_H0 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[0], vecfSubstrateData[0]);
        fGV_H1 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[1], vecfSubstrateData[1]);
        fGV_H2 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[2], vecfSubstrateData[2]);
        fGV_H3 = NC_SUB(m_pPassiveAlignResult->vecfSplitBody_Height[3], vecfSubstrateData[3]);
    }

    // Body_Height는 평균으로 계산
    m_pPassiveAlignResult->fBodyHeight = NC_SUB(fBody, fLand);

    // Body_Tilt
    std::vector<float> vecfHPosBodyPart = m_pPassiveAlignResult->vecfSplitBody_Height;
    sort(vecfHPosBodyPart.begin(), vecfHPosBodyPart.end());
    m_pPassiveAlignResult->fBodyTiltHeight = NC_SUB(vecfHPosBodyPart[nSplitNum - 1], vecfHPosBodyPart[0]);

    float fMinBody1 = min(fGV_H0, fGV_H1);
    float fMinBody2 = min(fGV_H2, fGV_H3);
    float fMinBody = min(fMinBody1, fMinBody2);

    float fMaxBody1 = max(fGV_H0, fGV_H1);
    float fMaxBody2 = max(fGV_H2, fGV_H3);
    float fMaxBody = max(fMaxBody1, fMaxBody2);

    m_pPassiveAlignResult->fBodyTiltHeight = NC_SUB(fMaxBody, fMinBody);

    if (findInvalidValue)
        // 유효하지 않은 픽셀들을 발견했다면 Tilt Height는 Reject 처리하자
        m_pPassiveAlignResult->fBodyTiltHeight = Ipvm::k_noiseValue32r;

    return true;
}

void CPassive3DAlgorithm::GetSplitBodyROI(
    long i_nSpitNumH, long i_nSplitNumY, Ipvm::Rect32s i_rtValidBody, std::vector<Ipvm::Rect32s>& o_vecrtValidSplitBody)
{
    o_vecrtValidSplitBody.resize(i_nSpitNumH * i_nSpitNumH);
    float fHorzSize = i_rtValidBody.Height() / (float)i_nSpitNumH;
    float fVertSize = i_rtValidBody.Width() / (float)i_nSplitNumY;
    int cnt(0);

    // Body ROI Distance X,Y
    long nBodyDistanceX_px = CAST_LONG(CAST_FLOAT(m_pPassiveAlignParam->m_nCalc_Body_Area_GapX_um)
            * SystemConfig::GetInstance().GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT).umToPixel().m_x
        + .5f);
    long nBodyDistanceY_px = CAST_LONG(CAST_FLOAT(m_pPassiveAlignParam->m_nCalc_Body_Area_GapY_um)
            * SystemConfig::GetInstance().GetScale(enSideVisionModule::SIDE_VISIONMODULE_FRONT).umToPixel().m_y
        + .5f);

    if (m_pPassiveInfoDB->fPassiveAngle == 90.f)
    {
        for (long nX = 0; nX < i_nSplitNumY; nX++)
        {
            for (long nY = 0; nY < i_nSpitNumH; nY++)
            {
                o_vecrtValidSplitBody[cnt] = Ipvm::Rect32s((int)(i_rtValidBody.m_left + nX * fVertSize + .5f),
                    (int)(i_rtValidBody.m_top + nY * fHorzSize + .5f),
                    (int)(i_rtValidBody.m_left + (nX + 1) * fVertSize + .5f),
                    (int)(i_rtValidBody.m_top + (nY + 1) * fHorzSize + .5f));

                cnt++;
            }
        }
        //// {{ Width 방향
        // Left Top Rect
        o_vecrtValidSplitBody[0].m_left = GetROISize(o_vecrtValidSplitBody[0], 90.f, LEFT);
        o_vecrtValidSplitBody[0].m_right = GetROISize(o_vecrtValidSplitBody[0], 90.f, RIGHT);

        // Right Top Rect
        o_vecrtValidSplitBody[1].m_left = GetROISize(o_vecrtValidSplitBody[1], 90.f, LEFT);
        o_vecrtValidSplitBody[1].m_right = GetROISize(o_vecrtValidSplitBody[1], 90.f, RIGHT);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].m_left = GetROISize(o_vecrtValidSplitBody[2], 90.f, LEFT);
        o_vecrtValidSplitBody[2].m_right = GetROISize(o_vecrtValidSplitBody[2], 90.f, RIGHT);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].m_left = GetROISize(o_vecrtValidSplitBody[3], 90.f, LEFT);
        o_vecrtValidSplitBody[3].m_right = GetROISize(o_vecrtValidSplitBody[3], 90.f, RIGHT);
        //// }}

        //// {{ Length 방향
        // Left Top Rect
        o_vecrtValidSplitBody[0].m_top = GetROISize(o_vecrtValidSplitBody[0], 90.f, UP);
        o_vecrtValidSplitBody[0].m_bottom = GetROISize(o_vecrtValidSplitBody[0], 90.f, DOWN);

        // Right Top Rect
        o_vecrtValidSplitBody[1].m_top = GetROISize(o_vecrtValidSplitBody[1], 90.f, UP);
        o_vecrtValidSplitBody[1].m_bottom = GetROISize(o_vecrtValidSplitBody[1], 90.f, DOWN);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].m_top = GetROISize(o_vecrtValidSplitBody[2], 90.f, UP);
        o_vecrtValidSplitBody[2].m_bottom = GetROISize(o_vecrtValidSplitBody[2], 90.f, DOWN);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].m_top = GetROISize(o_vecrtValidSplitBody[3], 90.f, UP);
        o_vecrtValidSplitBody[3].m_bottom = GetROISize(o_vecrtValidSplitBody[3], 90.f, DOWN);

        // -------------------------------- Offset X
        // Left Top Rect
        o_vecrtValidSplitBody[0].OffsetRect(-nBodyDistanceX_px, 0);

        // Right Top Rect
        o_vecrtValidSplitBody[1].OffsetRect(-nBodyDistanceX_px, 0);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].OffsetRect(nBodyDistanceX_px, 0);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].OffsetRect(nBodyDistanceX_px, 0);

        // }}

        // Offset Y
        // Left Top Rect
        o_vecrtValidSplitBody[0].OffsetRect(0, -nBodyDistanceY_px);

        // Right Top Rect
        o_vecrtValidSplitBody[1].OffsetRect(0, nBodyDistanceY_px);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].OffsetRect(0, -nBodyDistanceY_px);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].OffsetRect(0, nBodyDistanceY_px);
    }
    else
    {
        for (long nY = 0; nY < i_nSpitNumH; nY++)
        {
            for (long nX = 0; nX < i_nSplitNumY; nX++)
            {
                o_vecrtValidSplitBody[cnt] = Ipvm::Rect32s((int)(i_rtValidBody.m_left + nX * fVertSize + .5f),
                    (int)(i_rtValidBody.m_top + nY * fHorzSize + .5f),
                    (int)(i_rtValidBody.m_left + (nX + 1) * fVertSize + .5f),
                    (int)(i_rtValidBody.m_top + (nY + 1) * fHorzSize + .5f));

                cnt++;
            }
        }

        //// {{ Width 방향
        // Left Top Rect
        o_vecrtValidSplitBody[0].m_top = GetROISize(o_vecrtValidSplitBody[0], 0.f, UP);
        o_vecrtValidSplitBody[0].m_bottom = GetROISize(o_vecrtValidSplitBody[0], 0.f, DOWN);

        // Right Top Rect
        o_vecrtValidSplitBody[1].m_top = GetROISize(o_vecrtValidSplitBody[1], 0.f, UP);
        o_vecrtValidSplitBody[1].m_bottom = GetROISize(o_vecrtValidSplitBody[1], 0.f, DOWN);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].m_top = GetROISize(o_vecrtValidSplitBody[2], 0.f, UP);
        o_vecrtValidSplitBody[2].m_bottom = GetROISize(o_vecrtValidSplitBody[2], 0.f, DOWN);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].m_top = GetROISize(o_vecrtValidSplitBody[3], 0.f, UP);
        o_vecrtValidSplitBody[3].m_bottom = GetROISize(o_vecrtValidSplitBody[3], 0.f, DOWN);
        //// }}

        //// {{ Length 방향
        // Left Top Rect
        o_vecrtValidSplitBody[0].m_left = GetROISize(o_vecrtValidSplitBody[0], 0.f, LEFT);
        o_vecrtValidSplitBody[0].m_right = GetROISize(o_vecrtValidSplitBody[0], 0.f, RIGHT);

        // Right Top Rect
        o_vecrtValidSplitBody[1].m_left = GetROISize(o_vecrtValidSplitBody[1], 0.f, LEFT);
        o_vecrtValidSplitBody[1].m_right = GetROISize(o_vecrtValidSplitBody[1], 0.f, RIGHT);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].m_left = GetROISize(o_vecrtValidSplitBody[2], 0.f, LEFT);
        o_vecrtValidSplitBody[2].m_right = GetROISize(o_vecrtValidSplitBody[2], 0.f, RIGHT);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].m_left = GetROISize(o_vecrtValidSplitBody[3], 0.f, LEFT);
        o_vecrtValidSplitBody[3].m_right = GetROISize(o_vecrtValidSplitBody[3], 0.f, RIGHT);
        //// }}

        // -------------------------------- Offset X
        // Left Top Rect
        o_vecrtValidSplitBody[0].OffsetRect(-nBodyDistanceX_px, 0);

        // Right Top Rect
        o_vecrtValidSplitBody[1].OffsetRect(nBodyDistanceX_px, 0);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].OffsetRect(-nBodyDistanceX_px, 0);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].OffsetRect(nBodyDistanceX_px, 0);
        // }}

        // Offset Y
        // Left Top Rect
        o_vecrtValidSplitBody[0].OffsetRect(0, -nBodyDistanceY_px);

        // Right Top Rect
        o_vecrtValidSplitBody[1].OffsetRect(0, -nBodyDistanceY_px);

        // Left Bottom Rect
        o_vecrtValidSplitBody[2].OffsetRect(0, nBodyDistanceY_px);

        // Right Bottom Rect
        o_vecrtValidSplitBody[3].OffsetRect(0, nBodyDistanceY_px);
    }
}

int CPassive3DAlgorithm::GetROISize(Ipvm::Rect32s i_rtSplitROI, float i_fAngle, long i_nDirection)
{
    int nValue(-1);

    Ipvm::Point32s2 ptSplitROICenter = i_rtSplitROI.CenterPoint();
    int nHalfSizeValue(-1);
    int nCenterPos(-1);
    float fROISizeRatio(-1);

    if (i_fAngle == 90.f)
    {
        switch (i_nDirection)
        {
            case LEFT:
            case RIGHT:
                nHalfSizeValue = int((i_rtSplitROI.Width() * .5f) + .5f);
                nCenterPos = ptSplitROICenter.m_x;
                fROISizeRatio = m_pPassiveAlignParam->m_nCalc_Body_Area_Width_Ratio / 100.f;
                break;
            case UP:
            case DOWN:
                nHalfSizeValue = int((i_rtSplitROI.Height() * .5f) + .5f);
                nCenterPos = ptSplitROICenter.m_y;
                fROISizeRatio = m_pPassiveAlignParam->m_nCalc_Body_Area_Length_Ratio / 100.f;
                break;
        }
    }
    else if (i_fAngle == 0.f)
    {
        switch (i_nDirection)
        {
            case UP:
            case DOWN:
                nHalfSizeValue = int((i_rtSplitROI.Height() * .5f) + .5f);
                nCenterPos = ptSplitROICenter.m_y;
                fROISizeRatio = m_pPassiveAlignParam->m_nCalc_Body_Area_Width_Ratio / 100.f;
                break;
            case LEFT:
            case RIGHT:
                nHalfSizeValue = int((i_rtSplitROI.Width() * .5f) + .5f);
                nCenterPos = ptSplitROICenter.m_x;
                fROISizeRatio = m_pPassiveAlignParam->m_nCalc_Body_Area_Length_Ratio / 100.f;
                break;
        }
    }
    else
        return nValue;

    if (i_nDirection == LEFT || i_nDirection == UP)
        nValue = nCenterPos - CAST_LONG(nHalfSizeValue * fROISizeRatio);
    else
        nValue = nCenterPos + CAST_LONG(nHalfSizeValue * fROISizeRatio);

    return nValue;
}

float CPassive3DAlgorithm::CalcZForSubstrate(const Ipvm::Image32r& i_zmap, Ipvm::Rect32s i_rtArea, float i_fMinZ,
    float i_fMaxZ, long& o_nValidDataNum, const Ipvm::Image16u* i_pwInten)
{
    float boundScale = 65535 / 255.f;
    USHORT lowerBound
        = (USHORT)min(USHRT_MAX, min(255.f, max(0, m_pPassiveAlignParam->m_fVisibilityLowerBound)) * boundScale);

    o_nValidDataNum = 0;

    double meanZ = 0.;
    long countZ = 0;

    for (long y = i_rtArea.m_top; y < i_rtArea.m_bottom; y++)
    {
        const float* zmap = i_zmap.GetMem(0, y);
        const WORD* weight = i_pwInten->GetMem(0, y);

        for (long x = i_rtArea.m_left; x < i_rtArea.m_right; x++)
        {
            if (zmap[x] == Ipvm::k_noiseValue32r || weight[x] <= lowerBound)
            {
                continue;
            }

            meanZ += zmap[x];
            countZ++;
        }
    }

    if (countZ == 0)
    {
        return Ipvm::k_noiseValue32r;
    }

    meanZ /= countZ;

    float fMinValZ = (float)(meanZ - i_fMinZ);
    float fMaxValZ = (float)(meanZ + i_fMaxZ);

    long nTotalCnt = 0;
    float fTotalInten = 0.0f;
    float fTotalWeightZ = 0.0f;

    for (long y = i_rtArea.m_top; y < i_rtArea.m_bottom; y++)
    {
        const float* zmap = i_zmap.GetMem(0, y);
        const WORD* weight = i_pwInten->GetMem(0, y);

        for (long x = i_rtArea.m_left; x < i_rtArea.m_right; x++)
        {
            const float z = zmap[x];

            if (z >= fMinValZ && z < fMaxValZ && weight[x] > lowerBound)
            {
                o_nValidDataNum++;
                fTotalInten += (float)weight[x];
                fTotalWeightZ += z * (float)weight[x];

                nTotalCnt++;
            }
        }
    }

    if (o_nValidDataNum <= 3)
        return Ipvm::k_noiseValue32r;

    if (nTotalCnt <= 0)
        return Ipvm::k_noiseValue32r;

    return fTotalWeightZ / fTotalInten;
}

float CPassive3DAlgorithm::CalcZData(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_zmap, Ipvm::Rect32s i_rtArea,
    float i_fMinZ, float i_fMaxZ, long& o_nValidDataNum, const Ipvm::Image16u* i_pwInten, const bool i_bisMIAType,
    const bool i_bUseFilterData)
{
    o_nValidDataNum = 0;

    std::vector<Ipvm::Point32s2> vecfptValidArea;
    double meanZ(-1.);
    if (!GetMeanZValue(i_Hmap, i_zmap, i_rtArea, vecfptValidArea, meanZ, i_bisMIAType))
        return Ipvm::k_noiseValue32r;

    float fMinValZ = (float)(meanZ - i_fMinZ);
    float fMaxValZ = (float)(meanZ + i_fMaxZ);

    float fTotalInten(0.f);
    float fTotalWeightZ(0.f);

    std::vector<std::map<float, float>> vecmapfValidCompHeightValue;
    if (i_bisMIAType == true)
    {
        for (auto fptValid : vecfptValidArea)
        {
            const float zmap = i_zmap[fptValid.m_y][fptValid.m_x];
            const WORD weight = (*i_pwInten)[fptValid.m_y][fptValid.m_x];

            if (zmap >= fMinValZ && zmap < fMaxValZ && weight > 0)
            {
                o_nValidDataNum++;
                fTotalInten += (float)weight;
                fTotalWeightZ += zmap * (float)weight;

                std::map<float, float> mapCurrentData;
                mapCurrentData.insert(std::pair<float, float>(zmap, weight));

                vecmapfValidCompHeightValue.push_back(mapCurrentData);
            }
        }
    }
    else
    {
        for (long y = i_rtArea.m_top; y < i_rtArea.m_bottom; y++)
        {
            const float* zmap = i_zmap.GetMem(0, y);
            const WORD* weight = i_pwInten->GetMem(0, y);

            for (long x = i_rtArea.m_left; x < i_rtArea.m_right; x++)
            {
                const float z = zmap[x];

                if (z >= fMinValZ && z < fMaxValZ && weight[x] > 0)
                {
                    o_nValidDataNum++;
                    fTotalInten += (float)weight[x];
                    fTotalWeightZ += z * (float)weight[x];

                    std::map<float, float> mapCurrentData;
                    mapCurrentData.insert(std::pair<float, float>(z, (float)weight[x]));

                    vecmapfValidCompHeightValue.push_back(mapCurrentData);
                }
            }
        }
    }

    o_nValidDataNum = (long)vecmapfValidCompHeightValue.size();

    if (o_nValidDataNum <= 3)
        return Ipvm::k_noiseValue32r;

    float fHeightTotalValue(-1.f), fWeightTotalValue(-1.f);

    if (i_bUseFilterData == true)
    {
        //높이 연산시, 참여한 Data를 이용하여 Sorting 이후, 전체 중의 20%의 데이터는 날린다, Noise 영역이라 판단됨
        std::sort(vecmapfValidCompHeightValue.begin(), vecmapfValidCompHeightValue.end(),
            [](const std::map<float, float>& a, const std::map<float, float>& b)
            {
                return a > b;
            });

        //Sorting이후에는 내림차순이라 하위부터 날리면 된다
        const long nDataSize = (long)vecmapfValidCompHeightValue.size();
        float fRemovePercent(0.2f);
        long nRemoveDataCount = CAST_LONG(((float)nDataSize * fRemovePercent) + .5f);

        if (nDataSize > nRemoveDataCount)
        {
            //long nErasePos = nDataSize - nRemoveDataCount;
            //		for (long nidx = nDataSize - nRemoveDataCount; nidx < nDataSize; nidx++)
            //			vecmapfValidCompHeightValue.erase(vecmapfValidCompHeightValue.begin() + nErasePos);
            for (long nidx = 0; nidx <= nRemoveDataCount; nidx++)
                vecmapfValidCompHeightValue.erase(vecmapfValidCompHeightValue.begin());

            for (long n = 0; n < vecmapfValidCompHeightValue.size(); n++)
            {
                std::map<float, float> mapFitting_ValidData = vecmapfValidCompHeightValue[n];
                std::map<float, float>::iterator itrFitting_ValidData = mapFitting_ValidData.begin();
                fHeightTotalValue += itrFitting_ValidData->first * itrFitting_ValidData->second;
                fWeightTotalValue += itrFitting_ValidData->second;
            }

            o_nValidDataNum = (long)vecmapfValidCompHeightValue.size();
        }
    }
    else
    {
        fHeightTotalValue = fTotalWeightZ;
        fWeightTotalValue = fTotalInten;
    }

    return fHeightTotalValue / fWeightTotalValue;
}

bool CPassive3DAlgorithm::GetMeanZValue(const Ipvm::Image8u& i_Hmap, const Ipvm::Image32r& i_zmap,
    Ipvm::Rect32s i_rtArea, std::vector<Ipvm::Point32s2>& o_vecfptValidArea, double& o_MeanZ_Value,
    const bool i_bisMIAType)
{
    double meanZ(0.);
    long countZ(0);

    o_vecfptValidArea.clear();
    if (i_bisMIAType == true)
    {
        if (!GetZValueVailidArea(i_Hmap, i_rtArea, o_vecfptValidArea))
            return false;

        for (auto fptValid : o_vecfptValidArea) //Gray Map에서 RangeThreshold하고 255인 애들의 좌표값을 이용한다.
        {
            const float zmap = i_zmap[fptValid.m_y][fptValid.m_x];

            if (zmap == Ipvm::k_noiseValue32r)
                continue;

            meanZ += zmap;
            countZ++;
        }
    }
    else
    {
        for (long y = i_rtArea.m_top; y < i_rtArea.m_bottom; y++)
        {
            const float* zmap = i_zmap.GetMem(0, y);

            for (long x = i_rtArea.m_left; x < i_rtArea.m_right; x++)
            {
                if (zmap[x] == Ipvm::k_noiseValue32r)
                    continue;

                meanZ += zmap[x];
                countZ++;
            }
        }
    }

    if (countZ == 0)
        return false;

    o_MeanZ_Value = meanZ / countZ;

    return true;
}

bool CPassive3DAlgorithm::GetZValueVailidArea(
    const Ipvm::Image8u& i_Hmap, const Ipvm::Rect32s& i_rtArea, std::vector<Ipvm::Point32s2>& o_vecfptValidPoint)
{
    long nLowThresholdValue(-1);
    long nHighThresholdValue(-1);

    if (!GetThresholdValue_HistogramPeak(i_Hmap, i_rtArea, nLowThresholdValue, nHighThresholdValue))
        return false;

    Ipvm::Image8u imageBinaryTemp;
    if (!m_pVisionReusableMemory->GetInspByteImage(imageBinaryTemp))
        return false;
    imageBinaryTemp.FillZero();

    Ipvm::ImageProcessing::BinarizeGreaterEqualAndLessEqual(
        i_Hmap, i_rtArea, CAST_INT8T(nLowThresholdValue), CAST_INT8T(nHighThresholdValue), imageBinaryTemp);

    o_vecfptValidPoint.clear();
    for (int nY = i_rtArea.m_top; nY < i_rtArea.m_bottom; nY++)
    {
        for (int nX = i_rtArea.m_left; nX < i_rtArea.m_right; nX++)
        {
            auto BinaryGrayValue = imageBinaryTemp.GetMem(nX, nY);
            if (*BinaryGrayValue != 0)
                o_vecfptValidPoint.emplace_back(nX, nY);
        }
    }

    return true;
}

bool CPassive3DAlgorithm::GetThresholdValue_HistogramPeak(
    const Ipvm::Image8u& i_Hmap, const Ipvm::Rect32s& i_rtArea, long& o_nLowThresholdValue, long& o_nHighThresholdValue)
{
    o_nLowThresholdValue = -1;
    o_nHighThresholdValue = -1;

    Ipvm::Image32s histogram;
    long nPeakNum(256);
    std::vector<long> vecnPeakID(nPeakNum);
    std::vector<long> vecnPeakCount(nPeakNum);
    long* pnPeakID = &vecnPeakID[0];
    long* pnPeakCount = &vecnPeakCount[0];
    long nCountThresh(10); //최소 10px

    long nSubstrateGV_Margin = 5;
    long nSubstrate_AvgGrayValue
        = GetAvgGrayValue(i_Hmap, m_pPassiveAlignResult->vecrtSubstrateArea) + nSubstrateGV_Margin;

    if (!CippModules::GetHistogramPeaks(i_Hmap, i_rtArea, histogram, pnPeakID, pnPeakCount, nPeakNum, nCountThresh,
            nSubstrate_AvgGrayValue, 255)) //mc_CountThresh값만 어떻게할지..
        return false;

    long nPeakIDidx(0);
    long nPeakValue(0);
    for (long nPeakidx = 0; nPeakidx < nPeakNum; nPeakidx++)
    {
        long nCurValue = pnPeakID[nPeakidx];
        if (nCurValue > nPeakValue)
        {
            nPeakValue = nCurValue;
            nPeakIDidx = nPeakidx;
        }
    }

    long nMarginValue(5);

    o_nLowThresholdValue = pnPeakID[nPeakIDidx] - nMarginValue;
    o_nHighThresholdValue = pnPeakID[nPeakIDidx] + nMarginValue;

    delete[] pnPeakID;
    delete[] pnPeakCount;

    return true;
}

long CPassive3DAlgorithm::GetAvgGrayValue(const Ipvm::Image8u& i_Hmap, std::vector<Ipvm::Rect32s> i_vecrtSubstrateROI)
{
    double fSummaryMeanValue(0.);

    for (auto SubstrateROI : i_vecrtSubstrateROI)
    {
        double fMeanValue(0.);
        Ipvm::ImageProcessing::GetMean(i_Hmap, SubstrateROI, fMeanValue);

        fSummaryMeanValue += fMeanValue;
    }

    long nSubstrateROINum = (long)i_vecrtSubstrateROI.size();

    return long(fSummaryMeanValue / nSubstrateROINum);
}