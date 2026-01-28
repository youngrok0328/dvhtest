//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionMark.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionMark.h"
#include "DlgVisionInspectionMarkOperator.h"
#include "InspResult.h"
#include "VisionInspectionMarkPara.h"
#include "VisionInspectionMarkSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/BlobAttribute.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Algorithm/Mathematics.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>
#include <math.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

/// ENUM_THRESHOLD_AUTOFULL Mode를 사용할때, Teaching과 Inspection의 AutoThreshold값이 +-30% 이상 차이가 발생하지 않게 Limit을 정함.
#define MAX_THRESHOLDDIFF_TEACHINSP 30
#define MK_AREATHRESHOLD_RATIO 0.3 /// 지정한 최소 Blob size의 30%까지를 Blob으로 인정.
#define MK_BLOB_WIDTHMAX 1000
#define MK_BLOB_WIDTHMIN 2
#define MK_BLOB_HEIGHTMAX 1000
#define MK_BLOB_HEIGHTMIN 2

//CPP_7_________________________________ Implementation body
//
const long ANGLETYPE_ALIGN_MARKANGLE = 2;

BOOL VisionInspectionMark::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionInspectionMark::CheckROI(Ipvm::Rect32s rtROI, long i_width, long i_height)
{
    if (rtROI.Width() <= 0 || rtROI.Height() <= 0)
        return FALSE;
    if (rtROI.m_left < 0 || rtROI.m_top < 0 || rtROI.m_right < rtROI.m_left || rtROI.m_bottom < rtROI.m_top)
        return FALSE;
    if (rtROI.m_right > i_width || rtROI.m_bottom > i_height)
        return FALSE;

    return TRUE;
}

void VisionInspectionMark::GetDebugInfo(
    const bool detailSetupMode, const Ipvm::Point32r2& imageRotateCenter, float imageAngle_deg)
{
    long nImageSizeX = getReusableMemory().GetInspImageSizeX();
    long nImageSizeY = getReusableMemory().GetInspImageSizeY();
    Ipvm::Point32r2 imageCenter(nImageSizeX * 0.5f, nImageSizeY * 0.5f);

    if (m_VisionPara->m_nMarkInspMode == Mark_InspMode_Normal)
    {
        std::vector<Ipvm::Rect32s> vecrtTemp;
        std::vector<Ipvm::Point32s2> vecptTemp;
        std::vector<long> vecnTemp;

        vecrtTemp.resize(2);
        vecrtTemp[0] = m_prtLocaterSearchROI[0];
        vecrtTemp[1] = m_prtLocaterSearchROI[1];
        SetDebugInfoItem(detailSetupMode, _T("Mark Locater Search ROI"), vecrtTemp);

        vecptTemp.resize(2);
        vecptTemp[0] = m_pptLocater[0];
        vecptTemp[1] = m_pptLocater[1];
        SetDebugInfoItem(detailSetupMode, _T("Mark Find Locater Point"), vecptTemp);

        vecnTemp.resize(1);
        vecnTemp[0] = m_VisionTempSpec->m_nCharNum;
        SetDebugInfoItem(detailSetupMode, _T("Mark Char Num"), vecnTemp, true);

        vecrtTemp.resize(1);
        vecrtTemp[0] = m_rtAllChar;
        SetDebugInfoItem(detailSetupMode, _T("Mark All Char"), vecrtTemp);

        SetDebugInfoItem(detailSetupMode, _T("Mark Search ROI"), m_result->m_vecroiCharSearch);
        SetDebugInfoItem(detailSetupMode, _T("Mark Blob ROI"), m_vecrtBlobROI);
        SetDebugInfoItem(detailSetupMode, _T("Mark Char Position"), m_result->m_vecrtCharPositionforImage);
        SetDebugInfoItem(detailSetupMode, _T("Mark User Ignore ROI"), m_VisionPara->m_vecrtUserIgnore);

        auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Mark ROI Spec Rect"));
        if (psDebugInfo != NULL)
        {
            Ipvm::Rect32s rtROIRef;

            if (psDebugInfo->pData != NULL)
            {
                psDebugInfo->Reset();
            }
            psDebugInfo->nDataNum = m_VisionTempSpec->m_nCharNum;
            Ipvm::Rect32s* pRect = new Ipvm::Rect32s[psDebugInfo->nDataNum];
            for (long i = 0; i < psDebugInfo->nDataNum; i++)
            {
                rtROIRef = m_VisionTempSpec->m_plTeachCharROI[i];
                pRect[i] = rtROIRef;
            }
            psDebugInfo->pData = pRect;
        }

        // 영훈 - Multi Mark에서 일단 제외한다. 아마 마지막 티치된 이미지만 보여질 것이다.
        SetDebugInfoItem(detailSetupMode, _T("Mark Image Spec"), m_VisionTempSpec->m_specImage, true);

        auto* surfaceLayerMask = getReusableMemory().AddSurfaceLayerMaskClass(m_strModuleName);
        surfaceLayerMask->Reset();

        //float fBodyangle = m_bodyAlignResult->fAngle;

        for (long charIndex = 0; charIndex < m_VisionTempSpec->m_nCharNum; charIndex++)
        {
            auto specROI = m_VisionTempSpec->m_plTeachCharROI[charIndex] + Ipvm::Conversion::ToPoint32s2(imageCenter);
            auto targetROI = m_result->m_vecrtCharPositionforCalcImage[charIndex];

            if ((specROI & Ipvm::Rect32s(m_VisionTempSpec->m_specImage)) != specROI)
                continue;

            Ipvm::Point32r2 targetPos;
            targetPos.m_x = targetROI.m_left + specROI.Width() * 0.5f;
            targetPos.m_y = targetROI.m_top + specROI.Height() * 0.5f;

            Ipvm::Image8u charMask(m_VisionTempSpec->m_specImage, specROI);
            surfaceLayerMask->Add(charMask, targetPos, imageRotateCenter, imageAngle_deg);
        }

        SetDebugInfoItem(detailSetupMode, _T("Find Mark Image"), surfaceLayerMask->GetMask());
    }
    else if (m_VisionPara->m_nMarkInspMode == MarkInspectionMode_Simple)
    {
        SetDebugInfoItem(detailSetupMode, _T("Inspection Blob ROI"), m_vecrtDebugROI);

        auto* surfaceLayerMask = getReusableMemory().AddSurfaceLayerMaskClass(m_strModuleName);
        surfaceLayerMask->Reset();

        if (m_SimpleAlgorithmImage.GetMem() != nullptr) //Blob 실패시 보여주기 싫어서..
        {
            SetDebugInfoItem(detailSetupMode, _T("Find Mark Image"), m_SimpleAlgorithmImage);
            surfaceLayerMask->Add(m_SimpleAlgorithmImage, (float)m_VisionPara->m_nMarkAngle_deg);
        }
    }
}

BOOL VisionInspectionMark::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side VIsion
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck time;

    ResetResult();

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    //Align 정보를 가져 온다
    m_bodyAlignResult = GetAlignInfo();
    if (m_bodyAlignResult == nullptr)
        return FALSE;

    //-----------------------------------------------------------------------
    // 검사항목이 아무것도 켜져 있다면 수행할 것이 없다.
    //-----------------------------------------------------------------------

    BOOL noInsp = TRUE;
    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_use)
            noInsp = FALSE;
    }

    if (noInsp)
    {
        return TRUE;
    }

    //-----------------------------------------------------------------------

    /// Buffer 할당.
    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return FALSE;

    //Src image를 돌린다
    Ipvm::Point32r2 bodyCenter = m_bodyAlignResult->m_center;
    float fBodyangle_deg = m_bodyAlignResult->m_angle_rad * (float)ITP_RAD_TO_DEG;
    float fMarkParaAngle_deg = (float)m_VisionPara->m_nMarkAngle_deg;
    float imageAngle_deg = fBodyangle_deg + fMarkParaAngle_deg;

    Ipvm::Image8u mainImage;
    Ipvm::Image8u rotateImage;
    Ipvm::Image8u procImage;

    if (!getReusableMemory().GetInspByteImage(mainImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(rotateImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(procImage))
        return FALSE;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(mainImage), 0, mainImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(rotateImage), 0, rotateImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(procImage), 0, procImage);

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcMangePara, mainImage))
    {
        m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());
        return FALSE;
    }

    //mc_Combine 후 Align된 영역에 따라 Ignore하고 Rotate 진행한다
    int Left = int(min(m_bodyAlignResult->fptLT.m_x, m_bodyAlignResult->fptLB.m_x) + .5f);
    int Top = int(min(m_bodyAlignResult->fptLT.m_y, m_bodyAlignResult->fptRT.m_y) + .5f);
    int Right = int(max(m_bodyAlignResult->fptRT.m_x, m_bodyAlignResult->fptRB.m_x) + .5f);
    int Bottom = int(max(m_bodyAlignResult->fptLB.m_y, m_bodyAlignResult->fptRB.m_y) + .5f);
    Ipvm::Rect32s rtBody(Left, Top, Right, Bottom);

    if (m_VisionPara->m_bAlignAreaIgnore)
        GetIgnoreImageByDebugInfo(mainImage, rtBody, mainImage);

    BOOL bInsp(FALSE);
    ResetInspItem();

    if (m_VisionPara->m_nMarkInspMode == Mark_InspMode_Normal)
    {
        // image Combine 후 기존 이미지 프로세싱 진행
        Ipvm::Rect32s rtInspROI_MapData;
        Ipvm::Point32r2 ptCenter;
        float fAngle_deg = 0.f;
        if (m_VisionPara->m_nROISettingMethod != ROISettingMethod_Manual)
        {
            rtInspROI_MapData = getScale().convert_BCUToPixel(m_VisionSpec[0]->m_rtMarkTeachROI_BCU, bodyCenter);
            ptCenter.Set(m_result->m_sMarkMapInfo_px.sTeachROI_Info.fOffsetX,
                m_result->m_sMarkMapInfo_px.sTeachROI_Info.fOffsetY);
            fAngle_deg = -m_result->m_sMarkMapInfo_px.sTeachROI_Info.fAngle;
        }
        else
        {
            rtInspROI_MapData = Ipvm::Rect32s(mainImage);
            ptCenter = bodyCenter;
            fAngle_deg = -imageAngle_deg;
        }

        Ipvm::ImageProcessing::RotateLinearInterpolation(
            mainImage, rtInspROI_MapData, ptCenter, fAngle_deg, Ipvm::Point32r2(0.f, 0.f), rotateImage);
        //Ipvm::ImageProcessing::RotateLinearInterpolation(mainImage, Ipvm::Rect32s(mainImage), bodyCenter, -imageAngle_deg, Ipvm::Point32r2(0.f, 0.f), rotateImage);
        if (!ImplProcessing(rotateImage, rtInspROI_MapData, m_VisionPara->m_nMarkPreProcMode, procImage))
        {
            m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

            return FALSE;
        }

        for (long nMarkID = 0; nMarkID < NUM_OF_MARKMULTI; nMarkID++)
        {
            long nMarkSpecIndex = m_VisionPara->m_nROISettingMethod == ROISettingMethod_Manual ? nMarkID : 0;

            m_VisionTempSpec = m_VisionSpec[nMarkSpecIndex];

            if (m_VisionSpec[nMarkSpecIndex]->m_bTeachDone)
            {
                m_result->Alloc(m_VisionTempSpec->m_nCharNum);

                Ipvm::Rect32s rtInspROI
                    = getScale().convert_BCUToPixel(m_VisionTempSpec->m_rtMarkTeachROI_BCU, bodyCenter);

                if (m_pVisionInspDlg != NULL)
                {
                    m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
                }

                bInsp = DoMarkInsp(procImage, m_psBlobInfo, rtInspROI);

                GetDebugInfo(detailSetupMode, ptCenter, fAngle_deg);

                bInsp &= UpdateResult();

                if (bInsp)
                    break;
                else
                    continue;
            }
        }
    }
    else if (m_VisionPara->m_nMarkInspMode == Mark_InspMode_Simple)
    {
        if (!ImplProcessing(mainImage, Ipvm::Rect32s(mainImage), m_VisionPara->m_nMarkPreProcMode, procImage))
        {
            m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

            return FALSE;
        }

        m_VisionTempSpec = m_VisionSpec[0];

        Ipvm::Rect32s rtInspROI = getScale().convert_BCUToPixel(m_VisionTempSpec->m_rtMarkTeachROI_BCU, bodyCenter);

        if (m_pVisionInspDlg != NULL)
            m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();

        if (m_fixedInspectionSpecs[MARK_INSPECTION_MARK_COUNT].m_use)
            bInsp = DoMarkCountInsp(procImage, m_psBlobInfo, rtInspROI);

        GetDebugInfo(detailSetupMode, bodyCenter, imageAngle_deg);

        bInsp &= UpdateResult();
    }

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return bInsp;
}

void VisionInspectionMark::ResetInspItem()
{
    VisionInspectionResult* pResult;

    for (long nInsp = MARK_INSPECTION_START; nInsp < MARK_INSPECTION_END; nInsp++)
    {
        pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[nInsp]);
        if (pResult == NULL)
            continue;

        pResult->Clear();
        pResult->Resize(1);

        // 영훈 20140630_BGAInitResult_Invalid : BGA 검사 초기 세팅을 invalid로 해놓는다.
        if (pResult && m_fixedInspectionSpecs[nInsp].m_use)
        {
            pResult->m_totalResult = INVALID;
            for (long i = 0; i < (long)pResult->m_objectErrorValues.size(); i++)
            {
                pResult->SetResult(i, pResult->m_totalResult);
            }
            pResult->SetTotalResult();
        }
    }
}

VisionAlignResult* VisionInspectionMark::GetAlignInfo()
{
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        return (VisionAlignResult*)pData;

    return NULL;
}

BOOL VisionInspectionMark::ImplProcessing(
    const Ipvm::Image8u& i_grayImage, const Ipvm::Rect32s& i_rtProcessROI, long i_nPreProcMode, Ipvm::Image8u& o_image)
{
    Ipvm::Image8u tempImage;
    if (!getReusableMemory().GetInspByteImage(tempImage))
        return FALSE;

    BOOL bResult = TRUE;

    switch (i_nPreProcMode)
    {
        case ENUM_PREPROC_NOTHING:
            if (Ipvm::ImageProcessing::Copy(i_grayImage, i_rtProcessROI, o_image) != Ipvm::Status::e_ok)
            {
                bResult = FALSE;
                break;
            }
            break;
        case ENUM_PREPROC_MEDIANSMOOTHING:
            if (Ipvm::ImageProcessing::FilterMedian3x3(i_grayImage, i_rtProcessROI, true, tempImage)
                != Ipvm::Status::e_ok)
            {
                bResult = FALSE;
            }
            if (Ipvm::ImageProcessing::FilterLowPass3x3(tempImage, i_rtProcessROI, true, o_image) != Ipvm::Status::e_ok)
            {
                bResult = FALSE;
            }
            break;
        case ENUM_PREPROC_SMOOTHINGMEDIAN:
            if (Ipvm::ImageProcessing::FilterLowPass3x3(i_grayImage, i_rtProcessROI, true, tempImage)
                != Ipvm::Status::e_ok)
            {
                bResult = FALSE;
            }
            if (Ipvm::ImageProcessing::FilterMedian3x3(tempImage, i_rtProcessROI, true, o_image) != Ipvm::Status::e_ok)
            {
                bResult = FALSE;
            }
            break;
        case ENUM_PREPROC_MEDIAN:
            if (Ipvm::ImageProcessing::FilterMedian3x3(i_grayImage, i_rtProcessROI, true, o_image)
                != Ipvm::Status::e_ok)
            {
                bResult = FALSE;
            }
            break;
        case ENUM_PREPROC_SMOOTHING:
            if (Ipvm::ImageProcessing::FilterLowPass3x3(i_grayImage, i_rtProcessROI, true, o_image)
                != Ipvm::Status::e_ok)
            {
                bResult = FALSE;
            }
            break;
        default:
            bResult = FALSE;
    }

    return bResult;
}

BOOL VisionInspectionMark::GetCharBlob(const Ipvm::Image8u& i_binImage, Ipvm::Rect32s i_rtProcessROI,
    SMarkAlgorithmParameter* i_psMarkParam, bool i_roiMerge, std::vector<TeachMergeInfo>& io_mergeInfos,
    const std::vector<Ipvm::Rect32s>& i_vecrtIgnoreNumROI, const std::vector<Ipvm::Rect32s>& i_vecrtIgnoreROI,
    Ipvm::BlobInfo* o_psMarkInfo, BlobAttribute* o_blobAttributes, Ipvm::Image32s& o_imageLabel,
    int32_t& o_nCharBlobNum)
{
    long nImageSizeX = getReusableMemory().GetInspImageSizeX();
    long nImageSizeY = getReusableMemory().GetInspImageSizeY();
    Ipvm::Point32r2 imageCenter(nImageSizeX * 0.5f, nImageSizeY * 0.5f);

    long lAreaThresh = (long)(MK_AREATHRESHOLD_RATIO
        * i_psMarkParam->nBlobSizeMin); /// 최소BlobSize의 약 30%정도 이상을 Blob으로 인정하자.

    /// Merge ROI
    long nMergeBoxNum = (long)io_mergeInfos.size();
    std::vector<Ipvm::Rect32s> vecrtMergeBox(nMergeBoxNum);
    for (long n = 0; n < nMergeBoxNum; n++)
    {
        vecrtMergeBox[n] = io_mergeInfos[n].m_roi + Ipvm::Conversion::ToPoint32s2(imageCenter);
    }

    //20141004 현건
    /// Ignore Number ROI
    long nIgnoreCnt = 0;
    for (int i = 0; i < MARK_MAX_IGNORE_AREA; i++)
    {
        if (m_VisionPara->m_vecbSelectIgnore[i] == TRUE)
        {
            nIgnoreCnt++;
        }
    }
    long nIgnoreBoxNum = nIgnoreCnt;
    std::vector<Ipvm::Rect32s> vecrtNumIgnoreBox(nIgnoreBoxNum);
    nIgnoreCnt = 0;

    for (int i = 0; i < MARK_MAX_IGNORE_AREA; i++)
    {
        if (m_VisionPara->m_vecbSelectIgnore[i] == TRUE)
        {
            vecrtNumIgnoreBox[nIgnoreCnt]
                = i_vecrtIgnoreNumROI[nIgnoreCnt] + Ipvm::Conversion::ToPoint32s2(imageCenter);
            nIgnoreCnt++;
        }
    }

    /// Ignore ROI
    nIgnoreBoxNum = (long)i_vecrtIgnoreROI.size();
    std::vector<Ipvm::Rect32s> vecrtIgnoreBox;
    vecrtIgnoreBox.resize(nIgnoreBoxNum);

    for (long n = 0; n < nIgnoreBoxNum; n++)
    {
        vecrtIgnoreBox[n] = i_vecrtIgnoreROI[n] + Ipvm::Conversion::ToPoint32s2(imageCenter);
    }

    //{{ //kircheis_AutoTeach
    std::vector<Ipvm::Rect32s> vecrtMerge = vecrtMergeBox;
    std::vector<Ipvm::Rect32s> vecrtUserNumIgnore = vecrtNumIgnoreBox;
    std::vector<Ipvm::Rect32s> vecrtUserIgnore = vecrtIgnoreBox;
    Ipvm::Point32s2 ptOffset;

    if (m_bAutoTeachMode && m_bodyAlignResult->bAvailable) //kircheis_AutoTeach
    {
        long nCenterX = (long)(m_bodyAlignResult->m_center.m_x + .5f);
        long nCenterY = (long)(m_bodyAlignResult->m_center.m_y + .5f);

        //Merge ROI
        for (long i = 0; i < long(m_VisionPara->m_teach_merge_infos.size()); i++)
        {
            ptOffset = m_VisionPara->m_teach_merge_infos[i].m_roiLtFromPackCenter;
            if (ptOffset.m_x != 0 && ptOffset.m_y != 0)
            {
                Ipvm::Point32s2 ptRoiLT = Ipvm::Point32s2(nCenterX + ptOffset.m_x, nCenterY + ptOffset.m_y);
                vecrtMerge[i] += ptRoiLT;
            }
        }

        //UserNumIgnore ROI
        long nSize = (long)(m_VisionPara->m_vecrtUserNumIgnore.size());
        long nDestSize = (long)(m_VisionPara->m_vecptUserNumIgnoreRoiLtFromPackCenter.size());
        if (nSize == nDestSize)
        {
            for (long i = 0; i < nSize; i++)
            {
                ptOffset = m_VisionPara->m_vecptUserNumIgnoreRoiLtFromPackCenter[i];
                if (ptOffset.m_x != 0 && ptOffset.m_y != 0)
                {
                    Ipvm::Point32s2 ptRoiLT = Ipvm::Point32s2(nCenterX + ptOffset.m_x, nCenterY + ptOffset.m_y);
                    vecrtUserNumIgnore[i] += ptRoiLT;
                }
            }
        }

        //UserIgnore ROI
        nSize = (long)(m_VisionPara->m_vecrtUserIgnore.size());
        nDestSize = (long)(m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter.size());
        if (nSize == nDestSize)
        {
            for (long i = 0; i < nSize; i++)
            {
                ptOffset = m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter[i];
                if (ptOffset.m_x != 0 && ptOffset.m_y != 0)
                {
                    Ipvm::Point32s2 ptRoiLT = Ipvm::Point32s2(nCenterX + ptOffset.m_x, nCenterY + ptOffset.m_y);
                    vecrtUserIgnore[i] += ptRoiLT;
                }
            }
        }
    }
    else if (m_bodyAlignResult->bAvailable)
    {
        //Merge ROI
        for (long i = 0; i < long(io_mergeInfos.size()); i++)
        {
            io_mergeInfos[i].m_roiLtFromPackCenter.m_x
                = (long)((float)(vecrtMerge[i].m_left) - m_bodyAlignResult->m_center.m_x + .5f);
            io_mergeInfos[i].m_roiLtFromPackCenter.m_y
                = (long)((float)(vecrtMerge[i].m_top) - m_bodyAlignResult->m_center.m_y + .5f);
        }

        //UserNumIgnore ROI
        long nSize = (long)vecrtUserNumIgnore.size();
        long nDestSize = (long)(m_VisionPara->m_vecptUserNumIgnoreRoiLtFromPackCenter.size());
        if (nSize != nDestSize)
            m_VisionPara->m_vecptUserNumIgnoreRoiLtFromPackCenter.resize(nSize);
        for (long i = 0; i < nSize; i++)
        {
            m_VisionPara->m_vecptUserNumIgnoreRoiLtFromPackCenter[i].m_x
                = (long)((float)(vecrtUserNumIgnore[i].m_left) - m_bodyAlignResult->m_center.m_x + .5f);
            m_VisionPara->m_vecptUserNumIgnoreRoiLtFromPackCenter[i].m_y
                = (long)((float)(vecrtUserNumIgnore[i].m_top) - m_bodyAlignResult->m_center.m_y + .5f);
        }

        //UserIgnore ROI
        nSize = (long)vecrtUserIgnore.size();
        nDestSize = (long)(m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter.size());
        if (nSize != nDestSize)
            m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter.resize(nSize);
        for (long i = 0; i < nSize; i++)
        {
            m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter[i].m_x
                = (long)((float)(vecrtUserIgnore[i].m_left) - m_bodyAlignResult->m_center.m_x + .5f);
            m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter[i].m_y
                = (long)((float)(vecrtUserIgnore[i].m_top) - m_bodyAlignResult->m_center.m_y + .5f);
        }
    }
    //}}

    o_nCharBlobNum = BLOB_INFO_SMALL_SIZE;

    if (m_pBlob->DoBlob(
            i_binImage, i_rtProcessROI, BLOB_INFO_SMALL_SIZE, o_imageLabel, o_psMarkInfo, o_nCharBlobNum, lAreaThresh)
        != 0)
    {
        return FALSE;
    }

    if (i_roiMerge)
    {
        if (!m_pBlob->GetBlobs_HitROIMerge(o_imageLabel, o_psMarkInfo, o_nCharBlobNum))
        {
            return FALSE;
        }
    }

    if (!m_pBlob->GetBlobs_MultiMergeBox(o_imageLabel, o_psMarkInfo, i_rtProcessROI, vecrtMerge, vecrtUserNumIgnore,
            vecrtUserIgnore, Ipvm::Rect32s(0, 0, 0, 0), MK_BLOB_WIDTHMAX, MK_BLOB_WIDTHMIN, MK_BLOB_HEIGHTMAX,
            MK_BLOB_HEIGHTMIN, o_nCharBlobNum))
    {
        return FALSE;
    }

    // Charector 속성 추출

    if (!m_pBlob->CalcBlobMassCenter(o_imageLabel, o_psMarkInfo, o_nCharBlobNum, o_blobAttributes))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetProfile(const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& io_binImage,
    Ipvm::Rect32s rtChar, std::vector<float>& vecfFilteredProf, BOOL bVirtical)
{
    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    /// ROI가 너무 작으면, Splite을 할 필요 없다.
    if (!CheckROI(rtChar, imageSizeX, imageSizeY))
        return FALSE;
    if (rtChar.Width() <= 5 || rtChar.Height() <= 5)
        return FALSE;

    vecfFilteredProf.clear();

    if (bVirtical)
    {
        vecfFilteredProf.resize(rtChar.Height());

        long nIndex = 0;
        for (long y = rtChar.m_top; y < rtChar.m_bottom; y++)
        {
            auto* gray_y = i_grayImage.GetMem(0, y);
            auto* bin_y = io_binImage.GetMem(0, y);
            long nCnt = 0;
            long nSumInten = 0;
            for (long x = rtChar.m_left; x < rtChar.m_right; x++)
            {
                if (bin_y[x] == 255)
                {
                    nCnt++;
                    nSumInten += gray_y[x];
                }
            }

            if (nCnt <= 0)
                vecfFilteredProf[nIndex] = 0.0f;
            else
                vecfFilteredProf[nIndex] = nSumInten / (float)nCnt;

            nIndex++;
        }
    }
    else
    {
        vecfFilteredProf.resize(rtChar.Width());

        long nIndex = 0;
        for (long x = rtChar.m_left; x < rtChar.m_right; x++)
        {
            long nCnt = 0;
            long nSumInten = 0;
            for (long y = rtChar.m_top; y < rtChar.m_bottom; y++)
            {
                if (io_binImage[y][x] == 255)
                {
                    nCnt++;
                    nSumInten += i_grayImage[y][x];
                }
            }

            if (nCnt <= 0)
                vecfFilteredProf[nIndex] = 0.0f;
            else
                vecfFilteredProf[nIndex] = nSumInten / (float)nCnt;

            nIndex++;
        }
    }

    LineFiltering(vecfFilteredProf);

    return TRUE;
}

BOOL VisionInspectionMark::LineFiltering(std::vector<float>& vecfLineProfile)
{
    long nProfileNum = (long)vecfLineProfile.size();
    if (nProfileNum < 5)
        return FALSE;

    std::vector<float> vecfTemp;
    vecfTemp.resize(nProfileNum);
    for (long nProfileIndex = 0; nProfileIndex < nProfileNum; nProfileIndex++)
    {
        float fAvg = 0.0f;
        if (nProfileIndex == 0 || nProfileIndex == (nProfileNum - 1))
            vecfTemp[nProfileIndex] = vecfLineProfile[nProfileIndex];
        else
        {
            fAvg = (vecfLineProfile[nProfileIndex - 1] + vecfLineProfile[nProfileIndex]
                       + vecfLineProfile[nProfileIndex + 1])
                / 3.0f;
            vecfTemp[nProfileIndex] = fAvg;
        }
    }

    for (long nProfileIndex = 0; nProfileIndex < nProfileNum; nProfileIndex++)
    {
        vecfLineProfile[nProfileIndex] = vecfTemp[nProfileIndex];
    }
    return TRUE;
}

BOOL VisionInspectionMark::SetSpliteChar(
    const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& io_binImage, Ipvm::Rect32s rtChar)
{
    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    long nDeflateOffset = 2;
    rtChar.DeflateRect(nDeflateOffset, nDeflateOffset);

    /// ROI가 너무 작으면, Splite을 할 필요 없다.
    if (!CheckROI(rtChar, imageSizeX, imageSizeY))
        return FALSE;
    if (rtChar.Width() <= 5 || rtChar.Height() <= 5)
        return FALSE;

    std::vector<float> vecfProfile;

    GetProfile(i_grayImage, io_binImage, rtChar, vecfProfile, m_VisionPara->m_nMarkDirection);
    long nProfileNum = (long)vecfProfile.size();

    float fMean, fMin, fMax, fStdDev;

    if (Ipvm::Mathematics::GetMean(&vecfProfile[0], nProfileNum, fMean) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (Ipvm::Mathematics::GetStdevSWithMean(&vecfProfile[0], nProfileNum, fMean, fStdDev) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (Ipvm::Mathematics::GetMinMax(&vecfProfile[0], nProfileNum, fMin, fMax) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (m_VisionPara->m_nMarkDirection == ENUM_MARKDIR_HOR)
    {
        /// Profile을 구하자
        //////// 평균-표준표차 보다 Low Value라면 Splite경계라 판단하자.
        float fThreshold = (fMean + fMin) / 2.0f;
        for (long nProfileIndex = 0; nProfileIndex < nProfileNum; nProfileIndex++)
        {
            if (fThreshold > vecfProfile[nProfileIndex])
            {
                for (long y = rtChar.m_top; y < rtChar.m_bottom; y++)
                {
                    *io_binImage.GetMem(rtChar.m_left + nProfileIndex, y) = 0;
                }
            }
        }
    }
    else
    {
        /// Profile을 구하자
        //////// 평균-표준표차 보다 Low Value라면 Splite경계라 판단하자.
        float fThreshold = (fMean + fMin) / 2.0f;
        for (long nProfileIndex = 0; nProfileIndex < nProfileNum; nProfileIndex++)
        {
            if (fThreshold > vecfProfile[nProfileIndex])
            {
                for (long x = rtChar.m_left; x < rtChar.m_right; x++)
                {
                    *io_binImage.GetMem(x, rtChar.m_top + nProfileIndex) = 0;
                }
            }
        }
    }

    return TRUE;
}

/// Average = (MAX + AVG)/2;
BOOL VisionInspectionMark::GetAvgRectWidthHeight(
    std::vector<Ipvm::Rect32s>& i_vecrtROI, float& o_fAvgWidth, float& o_fAvgHeight)
{
    long nSumHeight = 0;
    long nMaxHeight = 0;
    long nCurROIHeight = 0;
    long nSumWidth = 0;
    long nMaxWidth = 0;
    long nCurROIWidth = 0;

    long nROINum = (long)i_vecrtROI.size();

    for (long n = 0; n < nROINum; n++)
    {
        /// Width 평균과 최대
        nCurROIWidth = i_vecrtROI[n].Width();
        nSumWidth += nCurROIWidth;
        if (nCurROIWidth > nMaxWidth)
        {
            nMaxWidth = nCurROIWidth;
        }

        /// Height 평균과 최대
        nCurROIHeight = i_vecrtROI[n].Height();
        nSumHeight += nCurROIHeight;
        if (nCurROIHeight > nMaxHeight)
        {
            nMaxHeight = nCurROIHeight;
        }
    }
    float fAvgCharWidth = nSumWidth / (float)nROINum;
    o_fAvgWidth = (fAvgCharWidth + nMaxWidth) / 2;

    float fAvgCharHeight = nSumHeight / (float)nROINum;
    o_fAvgHeight = (fAvgCharHeight + nMaxHeight) / 2.0f;

    return TRUE;
}

BOOL VisionInspectionMark::SetSpliteChar(const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& io_binImage,
    std::vector<Ipvm::Rect32s>& i_vecrtSpecSpliteROI, Ipvm::BlobInfo* i_pCharBlobInfo)
{
    Ipvm::Point32r2 imageCenter(i_grayImage.GetSizeX() * 0.5f, i_grayImage.GetSizeY() * 0.5f);

    long nMarkSpliteROINum = (long)i_vecrtSpecSpliteROI.size();
    if (nMarkSpliteROINum <= 0)
        return TRUE;

    for (long nLine = 0; nLine < m_VisionTempSpec->m_nCharLineNum; nLine++)
    {
        long nStart = m_VisionTempSpec->m_veclTeachStartCharIDPerLine[nLine];
        long nEnd = nStart + m_VisionTempSpec->m_veclTeachCharNumPerLine[nLine];

        std::vector<Ipvm::Rect32s> vecrtSelectedROI;
        std::vector<Ipvm::Rect32s> vecrtNonSelectedROI;
        for (long nBlob = nStart; nBlob < nEnd; nBlob++)
        {
            Ipvm::Rect32s rtBlob = i_pCharBlobInfo[nBlob].m_roi;

            BOOL bFind = FALSE;
            for (long nSpliteROI = 0; nSpliteROI < i_vecrtSpecSpliteROI.size(); nSpliteROI++)
            {
                Ipvm::Rect32s rtSelectedROI
                    = i_vecrtSpecSpliteROI[nSpliteROI] + Ipvm::Conversion::ToPoint32s2(imageCenter);

                if (rtSelectedROI.PtInRect(rtBlob.CenterPoint()))
                {
                    bFind = TRUE;
                    break;
                }
            }

            if (bFind)
                vecrtSelectedROI.push_back(rtBlob);
            else
                vecrtNonSelectedROI.push_back(rtBlob);
        }

        if (vecrtSelectedROI.size() > 0)
        {
            if (vecrtNonSelectedROI.size() == 1)
            {
                SetSpliteChar(i_grayImage, io_binImage, vecrtNonSelectedROI[0]);
            }
            else
            {
                /// 선택되지 않은 Char들의 평균 Width, Height를 가지고 나누는데 사용하자.
                float fAvgCharWidth = 0.0f;
                float fAvgCharHeight = 0.0f;

                GetAvgRectWidthHeight(vecrtNonSelectedROI, fAvgCharWidth, fAvgCharHeight);

                if (fAvgCharWidth <= 1.0f || fAvgCharHeight <= 1.0f)
                {
                    for (long n = 0; n < vecrtSelectedROI.size(); n++)
                    {
                        SetSpliteChar(i_grayImage, io_binImage, vecrtSelectedROI[n]);
                    }
                    return TRUE;
                }

                for (long n = 0; n < vecrtSelectedROI.size(); n++)
                {
                    std::vector<float> vecfFilteredProf;
                    Ipvm::Rect32s rtChar = vecrtSelectedROI[n];

                    GetProfile(i_grayImage, io_binImage, rtChar, vecfFilteredProf, m_VisionPara->m_nMarkDirection);
                    long nProfileNum = (long)vecfFilteredProf.size();
                    if (nProfileNum < 5)
                        continue;

                    if (m_VisionPara->m_nMarkDirection == ENUM_MARKDIR_HOR)
                    {
                        long nNumChar = (long)((float)rtChar.Width() / fAvgCharWidth);
                        if (nNumChar <= 1 || rtChar.Width() < fAvgCharWidth * 1.5)
                            continue;

                        float fCharGap = rtChar.Width() / (float)nNumChar;

                        for (long i = 0; i < nNumChar - 1; i++)
                        {
                            BYTE byMinValue = 255;
                            long nMinIndex = (long)vecfFilteredProf.size();

                            for (long j = -2; j <= 2; j++)
                            {
                                long nPos = (long)(fCharGap * (float)(i + 1)) + j;
                                if (nPos <= 0 || nPos >= nProfileNum)
                                    return FALSE;
                                if (vecfFilteredProf[nPos] < byMinValue)
                                {
                                    byMinValue = (BYTE)vecfFilteredProf[nPos];
                                    nMinIndex = nPos;
                                }
                            }

                            for (long y = rtChar.m_top - 2; y < rtChar.m_bottom + 2; y++)
                            {
                                *io_binImage.GetMem(rtChar.m_left + nMinIndex, y) = 0;
                            }
                        }
                    }
                    else
                    {
                        long nNumChar = (long)((float)rtChar.Height() / fAvgCharHeight);
                        if (nNumChar <= 1 || rtChar.Height() < fAvgCharHeight * 1.5)
                            continue;

                        float fCharGap = rtChar.Height() / (float)nNumChar;

                        for (long i = 0; i < nNumChar - 1; i++)
                        {
                            BYTE byMinValue = 255;
                            long nMinIndex = (long)vecfFilteredProf.size();

                            for (long j = -2; j <= 2; j++)
                            {
                                long nPos = (long)fCharGap * (i + 1) + j;
                                if (nPos <= 0 || nPos >= nProfileNum)
                                    return FALSE;
                                if ((BYTE)vecfFilteredProf[nPos] < byMinValue)
                                {
                                    byMinValue = (BYTE)vecfFilteredProf[nPos];
                                    nMinIndex = nPos;
                                }
                            }

                            for (long x = rtChar.m_left - 2; x < rtChar.m_right + 2; x++)
                            {
                                *io_binImage.GetMem(x + nMinIndex, rtChar.m_top + nMinIndex) = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetCharBlobInfoForTeach(VisionInspectionMarkPara* i_pVisionPara,
    const Ipvm::Image8u& i_image, Ipvm::Image8u& i_grayProcessImage, Ipvm::Image8u& i_binImage,
    Ipvm::Image32s& imageLabel, Ipvm::Rect32s i_rtProcessROI, Ipvm::BlobInfo* i_pCharBlobInfo,
    BlobAttribute* i_charAttribute, int32_t& i_nCharBlobNum, long nReTeachThreshold, BOOL bSplite)
{
    UNREFERENCED_PARAMETER(nReTeachThreshold);

    ///1) PreProcessing image:: Teaching 할때는 전체 영상을 가지고 하자.
    Ipvm::Rect32s rtROI(getImageLotInsp().GetImageRect());

    if (!ImplProcessing(i_image, rtROI, i_pVisionPara->m_nMarkPreProcMode, i_grayProcessImage))
    {
        return FALSE;
    }

    //{{ //kircheis_AutoTeach
    if (m_bAutoTeachMode && m_VisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL)
    {
        BYTE nThresholdValue = (BYTE)m_VisionTempSpec->m_nMarkThreshold;
        BYTE nLowMean = 0;
        BYTE nHighMean = 0;

        Ipvm::ImageProcessing::GetThresholdOtsu(
            i_grayProcessImage, i_rtProcessROI, nLowMean, nHighMean, nThresholdValue);
        m_VisionTempSpec->m_nMarkThreshold = nThresholdValue;
    }
    //}}

    /// 2) Threshold :: AutoMode일때는 Threshold값을 따로 저장해 두자.
    BYTE thresholdValue = (BYTE)min(255, max(0, m_VisionTempSpec->m_nMarkThreshold));

    if (Ipvm::ImageProcessing::BinarizeGreaterEqual(i_grayProcessImage, i_rtProcessROI, thresholdValue, i_binImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (i_pVisionPara->m_bWhiteBackGround)
    {
        if (Ipvm::ImageProcessing::BitwiseNot(i_rtProcessROI, i_binImage) != Ipvm::Status::e_ok)
        {
            return FALSE;
        }
    }

    /// Binary영상에 Splite처리하자.
    if (bSplite)
        SetSpliteChar(i_image, i_binImage, i_pVisionPara->m_vecrtUserSplite, i_pCharBlobInfo);

    /// Blob
    i_nCharBlobNum = 0;
    if (!GetCharBlob(i_binImage, i_rtProcessROI, &i_pVisionPara->sMarkAlgoParam,
            (i_pVisionPara->m_teach_merge_num == -1), i_pVisionPara->m_teach_merge_infos,
            i_pVisionPara->m_vecrtUserNumIgnore, i_pVisionPara->m_vecrtUserIgnore, i_pCharBlobInfo, i_charAttribute,
            imageLabel,
            i_nCharBlobNum)) //kircheis_AutoTeach
        return FALSE;

    /// Blob filtering & Merge
    if (!DoCharNumberCheckTeach(
            imageLabel, i_pCharBlobInfo, i_nCharBlobNum, i_pVisionPara->sMarkAlgoParam.fMarkMergeGap))
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetMarkLineForTeach(SMarkAlgorithmParameter* i_pMarkAlgoPara,
    Ipvm::BlobInfo* i_pCharBlobInfo, BlobAttribute* i_charAttribute, long i_nCharBlobNum, short& i_nLineNum)
{
    UNREFERENCED_PARAMETER(i_pMarkAlgoPara);

    ///2) Y축으로 정렬해서 몇 Line이 있는지 확인하자. 그리고, X축으로 정렬한다.
    std::vector<long> vCharNum;
    std::vector<long> vLineStartCharID;
    if (!CalTextLineNumber(
            i_pCharBlobInfo, i_charAttribute, i_nCharBlobNum, i_nLineNum, vCharNum, vLineStartCharID, TRUE))
        return FALSE;

    //권오석 - Line별 Char 수가 몇개인지, Start Char ID는 어떤 놈인지 저장 070313 추가
    //요거 일괄적으로 할 수 있도록 정리..
    m_VisionTempSpec->m_nCharLineNum = i_nLineNum;
    m_VisionTempSpec->m_veclTeachCharNumPerLine.resize(i_nLineNum);
    m_VisionTempSpec->m_veclTeachStartCharIDPerLine.resize(i_nLineNum);

    for (short i = 0; i < i_nLineNum; i++)
    {
        m_VisionTempSpec->m_veclTeachCharNumPerLine[i] = vCharNum[i];
        m_VisionTempSpec->m_veclTeachStartCharIDPerLine[i] = vLineStartCharID[i];
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetCharOffsetFromMarkGroupCenter(
    Ipvm::BlobInfo* i_pCharBlobInfo, short i_nCharBlobNum, std::vector<Ipvm::Point32r2>& o_vecfptTeachCharOffset)
{
    if (i_nCharBlobNum <= 0)
        return FALSE;

    o_vecfptTeachCharOffset.clear();
    o_vecfptTeachCharOffset.resize(i_nCharBlobNum);

    //권오석 - CharOffset을 위해 L-T Point들의 중심과 중심으로 부터의 모든 Point들의 차이를 구한다.
    float fsumX = 0.f;
    float fsumY = 0.f;
    for (short i = 0; i < i_nCharBlobNum; i++)
    {
        fsumX += (float)i_pCharBlobInfo[i].m_roi.m_left;
        fsumY += (float)i_pCharBlobInfo[i].m_roi.m_top;
    }

    float fCenX = fsumX / (float)i_nCharBlobNum;
    float fCenY = fsumY / (float)i_nCharBlobNum;

    for (long i = 0; i < i_nCharBlobNum; i++)
    {
        o_vecfptTeachCharOffset[i].m_x = i_pCharBlobInfo[i].m_roi.m_left - fCenX;
        o_vecfptTeachCharOffset[i].m_y = i_pCharBlobInfo[i].m_roi.m_top - fCenY;
    }
    return TRUE;
}

BOOL VisionInspectionMark::MakeTeachingCharImage(Ipvm::BlobInfo* i_pCharBlobInfo, short i_nCharBlobNum,
    Ipvm::Rect32s* o_prtCharSpecROI, long* o_plCharArea, Ipvm::Rect32s* o_prtCharPosition)
{
    if (o_prtCharSpecROI == NULL || o_plCharArea == NULL)
        return FALSE;

    for (short k = 0; k < i_nCharBlobNum; k++)
    {
        Ipvm::Rect32s rtCharTemp = i_pCharBlobInfo[k].m_roi;
        o_plCharArea[k] = i_pCharBlobInfo[k].m_area;
        o_prtCharSpecROI[k] = rtCharTemp;
        o_prtCharPosition[k] = rtCharTemp;
    }

    return TRUE;
}

BOOL VisionInspectionMark::MakeLocatorForTeach(VisionInspectionMarkPara* i_pVisionPara, const Ipvm::Image8u& i_binImage,
    Ipvm::BlobInfo* i_pCharBlobInfo, BlobAttribute* i_charAttribute, short i_nCharBlobNum, int* i_pnLocID,
    Ipvm::Rect32s* i_prtLocaterSearchROI)
{
    Ipvm::Point32r2 imageCenter(i_binImage.GetSizeX() * 0.5f, i_binImage.GetSizeY() * 0.5f);

    // reference의 character ID를 얻는다.
    if (!i_pVisionPara->m_bLocatorTargetROI)
    {
        if (!GetDefaultRefID_Auto(i_binImage, i_pCharBlobInfo, i_charAttribute, i_nCharBlobNum, i_pnLocID))
            return FALSE;
    }
    else
    {
        long nCenterX = (long)(m_bodyAlignResult->m_center.m_x + .5f); //kircheis_AutoTeach
        long nCenterY = (long)(m_bodyAlignResult->m_center.m_y + .5f);
        Ipvm::Rect32s prtLocaterSearchROI[NUM_OF_LOCATOR];

        for (long i = 0; i < NUM_OF_LOCATOR; i++)
        {
            i_prtLocaterSearchROI[i]
                = m_VisionTempSpec->m_rtLocatorTargetROI[i] + Ipvm::Conversion::ToPoint32s2(imageCenter);
            prtLocaterSearchROI[i] = i_prtLocaterSearchROI[i];

            //{{//kircheis_AutoTeach
            if (m_bAutoTeachMode && m_bodyAlignResult->bAvailable)
            {
                if (m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i].m_x != 0
                    && m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i].m_y != 0)
                {
                    Ipvm::Point32s2 ptRoiLT(nCenterX + m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i].m_x,
                        nCenterY + m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i].m_y);
                    prtLocaterSearchROI[i] += ptRoiLT;
                }
            }
            else if (m_bodyAlignResult->bAvailable) //kircheis_AutoTeach
            {
                m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i].m_x
                    = (long)((float)(i_prtLocaterSearchROI[i].m_left) - m_bodyAlignResult->m_center.m_x + .5f);
                m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i].m_y
                    = (long)((float)(i_prtLocaterSearchROI[i].m_top) - m_bodyAlignResult->m_center.m_y + .5f);
            }
            //}}
        }

        if (!GetDefaultRefID_Manual(i_binImage, i_pCharBlobInfo, i_charAttribute, i_nCharBlobNum,
                prtLocaterSearchROI /*i_prtLocaterSearchROI*/, i_pnLocID))
            return FALSE;
    }

    // locater search ROI 설정
    // 주어진 옵셋만큼 서치 영역을 확장한다.
    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        m_prtLocaterSearchROI[i]
            = Ipvm::Rect32s(int32_t(i_pCharBlobInfo[i_pnLocID[i]].m_roi.m_left - i_pVisionPara->m_nLocSearchOffsetX),
                int32_t(i_pCharBlobInfo[i_pnLocID[i]].m_roi.m_top - i_pVisionPara->m_nLocSearchOffsetY),
                int32_t(i_pCharBlobInfo[i_pnLocID[i]].m_roi.m_right + i_pVisionPara->m_nLocSearchOffsetX),
                int32_t(i_pCharBlobInfo[i_pnLocID[i]].m_roi.m_bottom + i_pVisionPara->m_nLocSearchOffsetY));

        i_prtLocaterSearchROI[i] = i_pCharBlobInfo[i_pnLocID[i]].m_roi;
    }

    // locater position 계산

    Ipvm::Rect32s rtLocatorROI[NUM_OF_LOCATOR];

    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        m_pptLocater[i]
            = Ipvm::Point32s2(i_pCharBlobInfo[i_pnLocID[i]].m_roi.m_left, i_pCharBlobInfo[i_pnLocID[i]].m_roi.m_top);
    }

    if (!FindLocater(m_VisionTempSpec->m_specImage, i_binImage, m_pptLocater, TRUE))
    {
        return TRUE;
    }

    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        int nROIWidth = i_pCharBlobInfo[i_pnLocID[i]].m_roi.Width();
        int nROIHeight = i_pCharBlobInfo[i_pnLocID[i]].m_roi.Height();

        rtLocatorROI[i] = Ipvm::Rect32s(m_pptLocater[i].m_x, m_pptLocater[i].m_y, m_pptLocater[i].m_x + nROIWidth,
            m_pptLocater[i].m_y + nROIHeight);
    }
    //}}

    // Locater Angle
    float fAngle;
    Get2PntLocaterLineAngle(i_binImage, m_pptLocater, rtLocatorROI, fAngle, TRUE); //kircheis_MarkLoc
    m_VisionTempSpec->m_fLocAngleForBody = fAngle;

    float fX = (float)(m_pptLocater[0].m_x + m_pptLocater[1].m_x) / 2.0f;
    float fY = (float)(m_pptLocater[0].m_y + m_pptLocater[1].m_y) / 2.0f;
    Ipvm::Point32r2 ptLocCenter(fX, fY);
    m_VisionTempSpec->m_ptLocCenterforScreen = ptLocCenter;

    // character search ROI for bodyalign angle
    if (i_nCharBlobNum > 1) // 영훈 : Locator가 1개일 경우 Center와 LT의 오차가 발생하여 검사 Fail 발생
    {
        for (int i = 0; i < i_nCharBlobNum; i++)
        {
            Ipvm::Point32r2 ptCharLeftTop
                = Ipvm::Point32r2((float)i_pCharBlobInfo[i].m_roi.m_left, (float)i_pCharBlobInfo[i].m_roi.m_top);

            m_VisionTempSpec->m_teachCharOffset[i] = ptLocCenter - ptCharLeftTop;
        }
    }

    return TRUE;
}

BOOL VisionInspectionMark::MakeMarkSpecImageFromTeachPane(const Ipvm::Image8u& i_binImage, Ipvm::Image8u& o_specImage)
{
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(o_specImage), 0, o_specImage);

    if (Ipvm::ImageProcessing::Copy(i_binImage, Ipvm::Rect32s(i_binImage), o_specImage) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::DoTeach(const bool detailSetupMode)
{
    if (m_pVisionInspDlg != NULL)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }
    else
        return FALSE;

    // Align 결과 가져오기...
    m_bodyAlignResult = GetAlignInfo();
    if (m_bodyAlignResult == nullptr)
        return FALSE;

    // Initilize
    // 영훈 20160523 : Teaching Slot이 -111100901 같은 쓰래기 값이 들어가 있는 경우 발생, 왜 그런지 원인 파악 하지 못함
    // 만약 위와 같은 경우가 있다면 Slot은 3개 밖에 없으므로 그 이외의 값은 0으로 초기화 하도록 한다.
    long nTeachSlotBuf = 0; //kircheis_AutoTeach
    if (m_bAutoTeachMode)
    {
        nTeachSlotBuf = 0;
        InitMemory(FALSE, nTeachSlotBuf);
    }
    else
    {
        if (m_pVisionInspDlg->m_pVisionInspOpParaDlg == nullptr)
            return FALSE;
        InitMemory(FALSE, m_pVisionInspDlg->m_pVisionInspOpParaDlg->m_nTeachSlot);
        nTeachSlotBuf = m_pVisionInspDlg->m_pVisionInspOpParaDlg->m_nTeachSlot;
    }

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    // Spec은 Detail일 경우 전부 Slot "0"째 이므로 항상 자기 자신임
    // Operator Mode일 분기를 시켜 주면 된다
    const long nTeachSlot = nTeachSlotBuf; //m_pVisionInspDlg->m_pVisionInspOpParaDlg->m_nTeachSlot;//kircheis_AutoTeach

    m_VisionTempSpec = m_VisionSpec[nTeachSlot];

    Ipvm::Rect32s teachROI = getScale().convert_BCUToPixel(m_VisionTempSpec->m_rtMarkTeachROI_BCU, imageCenter);

    if (m_bAutoTeachMode && m_bodyAlignResult->bAvailable) //kircheis_AutoTeach
    {
        CalcAutoTeachRoiOffset(); //기존의 Auto Mark Teach용 ROI Offset이 저장이 안되어 있으면 이전 Teach Data 기반으로 계산해주게 한다.
        if (m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter.m_x != 0
            && m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter.m_y != 0)
        {
            long nCenterX = (long)(m_bodyAlignResult->m_center.m_x + .5f);
            long nCenterY = (long)(m_bodyAlignResult->m_center.m_y + .5f);
            Ipvm::Point32s2 ptRoiLT(nCenterX + m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter.m_x,
                nCenterY + m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter.m_y);
            teachROI += ptRoiLT;
        }
    }
    else if (m_bodyAlignResult->bAvailable)
    {
        m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter.m_x
            = (long)((float)(teachROI.m_left) - m_bodyAlignResult->m_center.m_x + .5f);
        m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter.m_y
            = (long)((float)(teachROI.m_top) - m_bodyAlignResult->m_center.m_y + .5f);
    }

    // Teach된 이미지를 Spec Image로 저장한다.
    //	CString strTemp = m_strModuleName;
    //	strTemp.Delete(0, 5);
    //	long nCurMarkNum = atoi(strTemp); // "Mark_" 지우고 저장

    // Teaching Image 초기화
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_VisionTempSpec->m_specImage), 0, m_VisionTempSpec->m_specImage);

    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return FALSE;

    //// 임시 Buffer 할당.
    Ipvm::Image8u mainImage;
    Ipvm::Image8u rotateImage;
    Ipvm::Image8u grayProcImage;
    Ipvm::Image8u thresImage;

    if (!getReusableMemory().GetInspByteImage(mainImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(rotateImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(grayProcImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(thresImage))
        return FALSE;

    //// Buffer할당할때 초기화도 같이.. 뭐가들어있을지 모른다..
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(mainImage), 0, mainImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(rotateImage), 0, rotateImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(grayProcImage), 0, grayProcImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(thresImage), 0, thresImage);

    //// 이진화하고 Blob하자.
    Ipvm::Point32r2 bodyCenter = m_bodyAlignResult->m_center;
    float fBodyangle_deg = m_bodyAlignResult->m_angle_rad * (float)ITP_RAD_TO_DEG;
    float fMarkParaAngle_deg = (float)m_VisionPara->m_nMarkAngle_deg;
    float imageAngle_deg = fBodyangle_deg + fMarkParaAngle_deg;

    Ipvm::Rect32s rtInspROI_MapData;
    Ipvm::Point32r2 ptCenter;
    float fAngle_deg = 0.f;

    if (m_VisionPara->m_nROISettingMethod != ROISettingMethod_Manual)
    {
        rtInspROI_MapData = getScale().convert_BCUToPixel(m_VisionSpec[0]->m_rtMarkTeachROI_BCU, bodyCenter);
        ptCenter.Set(
            m_result->m_sMarkMapInfo_px.sTeachROI_Info.fOffsetX, m_result->m_sMarkMapInfo_px.sTeachROI_Info.fOffsetY);
        fAngle_deg = -m_result->m_sMarkMapInfo_px.sTeachROI_Info.fAngle;
    }
    else
    {
        rtInspROI_MapData = Ipvm::Rect32s(mainImage);
        ptCenter = bodyCenter;
        fAngle_deg = -imageAngle_deg;
    }

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, false, m_VisionPara->m_ImageProcMangePara, mainImage))
    {
        return FALSE;
    }

    Ipvm::ImageProcessing::Copy(mainImage, Ipvm::Rect32s(mainImage), rotateImage);
    //Rotate 하기전에 Parameter에 따라 Ignore 수행한다.
    if (m_VisionPara->m_bAlignAreaIgnore)
    {
        GetIgnoreImageByDebugInfo(mainImage, teachROI, mainImage);
    }

    Ipvm::ImageProcessing::RotateLinearInterpolation(
        mainImage, rtInspROI_MapData, ptCenter, fAngle_deg, Ipvm::Point32r2(0.f, 0.f), rotateImage);

    //////===================== 실제 Mark Teaching 함수.=============================/////

    m_VisionTempSpec->m_bTeachDone = DoTeach(detailSetupMode, m_VisionTempSpec, Ipvm::Rect32s(mainImage), teachROI,
        rotateImage, grayProcImage, thresImage, 0);

    // 영훈 Mark Image를 저장할 때 Teaching을 한 경우에만 저장하도록 변수를 하나 만든다.
    m_VisionTempSpec->m_bSaveTeachImage = m_VisionTempSpec->m_bTeachDone;

    return m_VisionTempSpec->m_bTeachDone;
}

BOOL VisionInspectionMark::DoTeach(const bool detailSetupMode, VisionInspectionMarkSpec* i_pMarkSpec,
    Ipvm::Rect32s rtPane, const Ipvm::Rect32s& teachROI, const Ipvm::Image8u& rotateImage, Ipvm::Image8u& grayProcImage,
    Ipvm::Image8u& thresImage, long nReTeachThreshold)
{
    UNREFERENCED_PARAMETER(detailSetupMode);

    Ipvm::Rect32s imageRoi(0, 0, getReusableMemory().GetInspImageSizeX(), getReusableMemory().GetInspImageSizeY());
    Ipvm::Point32r2 imageCenter(imageRoi.Width() * 0.5f, imageRoi.Height() * 0.5f);

    int32_t nCharBlobNum = 0;
    short nLineNumber = 0;
    float fAngle;

    /// MarkSplite을 하려면 먼저 한번 Blob을 한 상태에서 진행 가능하다.
    if (m_VisionPara->m_nSelectAngleReference == Mark_Angle_Ref_Body_Align)
    {
        Ipvm::Image8u rotateSrcImg;
        Ipvm::Image8u rotateImageTmp;
        if (!getReusableMemory().GetInspByteImage(rotateImageTmp))
            return FALSE;

        Ipvm::Image32s imageLabel;
        getReusableMemory().GetInspLongImage(imageLabel);

        for (long i = 0; i < ANGLETYPE_ALIGN_MARKANGLE; i++)
        {
            if (i == 0)
                rotateSrcImg = rotateImage;
            else
                rotateSrcImg = rotateImageTmp;

            nCharBlobNum = 0;

            if (!GetCharBlobInfoForTeach(m_VisionPara, rotateSrcImg, grayProcImage, thresImage, imageLabel, teachROI,
                    m_psBlobInfo, m_blobAttributes, nCharBlobNum, nReTeachThreshold, FALSE))
                return FALSE;

            i_pMarkSpec->AllocateMemory(nCharBlobNum);
            m_result->Alloc(nCharBlobNum);

            nLineNumber = 0;
            if (!GetMarkLineForTeach(
                    &m_VisionPara->sMarkAlgoParam, m_psBlobInfo, m_blobAttributes, nCharBlobNum, nLineNumber))
                return FALSE;

            if (i == 0)
            {
                fAngle = 0.f;
                if (!CheckMarkAngle(nLineNumber, fAngle))
                    return FALSE;

                auto centerPoint = m_bodyAlignResult->m_center;

                if (Ipvm::ImageProcessing::RotateLinearInterpolation(
                        rotateImage, rtPane, centerPoint, -fAngle, Ipvm::Point32r2(0.f, 0.f), rotateImageTmp)
                    != Ipvm::Status::e_ok)
                {
                    return FALSE;
                }
            }
        }

        ///// Spec image 만듬.
        Ipvm::Image8u temp;
        if (!getReusableMemory().GetInspByteImage(temp))
            return FALSE;

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(temp), 0, temp);

        for (int nCharIdx = 0; nCharIdx < nCharBlobNum; nCharIdx++)
        {
            if (m_VisionPara->m_vecbSelectIgnore[nCharIdx])
            {
                continue;
            }

            Ipvm::Rect32s charectorROI = m_psBlobInfo[nCharIdx].m_roi;

            ///// Label이 일치하는 것만 Spec image에 복사
            long nSelectedLabel = m_psBlobInfo[nCharIdx].m_label;
            for (long y = charectorROI.m_top; y < charectorROI.m_bottom; y++)
            {
                auto* label_y = imageLabel.GetMem(0, y);
                auto* temp_y = temp.GetMem(0, y);
                auto* spec_y = i_pMarkSpec->m_specImage.GetMem(0, y);

                for (long x = charectorROI.m_left; x < charectorROI.m_right; x++)
                {
                    if (nSelectedLabel == label_y[x])
                    {
                        temp_y[x] = 255;
                        spec_y[x] = 255;
                    }
                }
            }
        }

        Ipvm::ImageProcessing::Copy(i_pMarkSpec->m_specImage, Ipvm::Rect32s(i_pMarkSpec->m_specImage), thresImage);

        //LogMaster(strLogFileName,"=====================MakeTeachingCharImage() Start");
        //// Teaching Image를 만들자.
        if (!MakeTeachingCharImage(m_psBlobInfo, (short)nCharBlobNum, i_pMarkSpec->m_plTeachCharROI,
                i_pMarkSpec->m_plTeachCharArea, &m_result->m_vecrtCharPositionforCalcImage[0]))
        {
            return FALSE;
        }
        //LogMaster(strLogFileName,"=====================MakeTeachingCharImage() End");

        //LogMaster(strLogFileName,"=====================MakeLocatorForTeach() Start");
        if (!MakeLocatorForTeach(m_VisionPara, thresImage, m_psBlobInfo, m_blobAttributes, (short)nCharBlobNum,
                i_pMarkSpec->m_pnLocID, i_pMarkSpec->m_prtLocaterTeachingBox))
        {
            return FALSE;
        }
        //LogMaster(strLogFileName,"=====================MakeLocatorForTeach() End");

        //LogMaster(strLogFileName,"=====================ImageROItoAbsoluteROI() Start");
        ///============좌표 변환:: Pane ROI를 기준으로 좌표를 변환한다.========================////
        for (long n = 0; n < i_pMarkSpec->m_nCharNum; n++)
        {
            i_pMarkSpec->m_plTeachCharROI[n]
                = i_pMarkSpec->m_plTeachCharROI[n] - Ipvm::Conversion::ToPoint32s2(imageCenter);
        }
        for (long n = 0; n < NUM_OF_LOCATOR; n++)
        {
            i_pMarkSpec->m_prtLocaterTeachingBox[n]
                = i_pMarkSpec->m_prtLocaterTeachingBox[n] - Ipvm::Conversion::ToPoint32s2(imageCenter);
        }

        i_pMarkSpec->m_ptLocCenterforScreen = i_pMarkSpec->m_ptLocCenterforScreen - imageCenter;

        ///=====================================================================================////

        //LogMaster(strLogFileName,"=====================ImageROItoAbsoluteROI() End");
        MakeMarkSpecImageFromTeachPane(temp, i_pMarkSpec->m_specImage);
    }
    else if (m_VisionPara->m_nSelectAngleReference == Mark_Angle_Ref_Teaching_Image)
    {
        /// MarkSplite을 하려면 먼저 한번 Blob을 한 상태에서 진행 가능하다.
        Ipvm::Image32s imageLabel;
        getReusableMemory().GetInspLongImage(imageLabel);

        if (m_bMarkSpliteOptionTeach)
        {
            if (!GetCharBlobInfoForTeach(m_VisionPara, rotateImage, grayProcImage, thresImage, imageLabel, teachROI,
                    m_psBlobInfo, m_blobAttributes, nCharBlobNum, nReTeachThreshold, FALSE))
            {
                return FALSE;
            }

            //{{ Char Blob 만큼 메모리 확보.
            i_pMarkSpec->AllocateMemory(nCharBlobNum);
            m_result->Alloc(nCharBlobNum);
            //}}

            if (!GetMarkLineForTeach(
                    &m_VisionPara->sMarkAlgoParam, m_psBlobInfo, m_blobAttributes, nCharBlobNum, nLineNumber))
            {
                return FALSE;
            }
        }

        nCharBlobNum = 0;

        if (!GetCharBlobInfoForTeach(m_VisionPara, rotateImage, grayProcImage, thresImage, imageLabel, teachROI,
                m_psBlobInfo, m_blobAttributes, nCharBlobNum, nReTeachThreshold, m_bMarkSpliteOptionTeach))
        {
            return FALSE;
        }

        //{{ Char Blob 만큼 메모리 확보.
        i_pMarkSpec->AllocateMemory(nCharBlobNum);
        m_result->Alloc(nCharBlobNum);
        //}}

        nLineNumber = 0;
        if (!GetMarkLineForTeach(
                &m_VisionPara->sMarkAlgoParam, m_psBlobInfo, m_blobAttributes, nCharBlobNum, nLineNumber))
        {
            return FALSE;
        }

        ///// Spec image 만듬.

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(i_pMarkSpec->m_specImage), 0, i_pMarkSpec->m_specImage);

        for (int nCharIdx = 0; nCharIdx < nCharBlobNum; nCharIdx++)
        {
            Ipvm::Rect32s charectorROI = m_psBlobInfo[nCharIdx].m_roi;

            ///// Label이 일치하는 것만 Spec image에 복사
            long nSelectedLabel = m_psBlobInfo[nCharIdx].m_label;
            for (long y = charectorROI.m_top; y < charectorROI.m_bottom; y++)
            {
                auto* label_y = imageLabel.GetMem(0, y);
                auto* src_y = i_pMarkSpec->m_specImage.GetMem(0, y);

                for (long x = charectorROI.m_left; x < charectorROI.m_right; x++)
                {
                    if (nSelectedLabel == label_y[x])
                    {
                        src_y[x] = 255;
                    }
                }
            }
        }

        Ipvm::ImageProcessing::Copy(i_pMarkSpec->m_specImage, Ipvm::Rect32s(i_pMarkSpec->m_specImage), thresImage);

        //// Teaching Image를 만들자.
        if (!MakeTeachingCharImage(m_psBlobInfo, (short)nCharBlobNum, i_pMarkSpec->m_plTeachCharROI,
                i_pMarkSpec->m_plTeachCharArea, &m_result->m_vecrtCharPositionforCalcImage[0]))
        {
            return FALSE;
        }

        if (!MakeLocatorForTeach(m_VisionPara, thresImage, m_psBlobInfo, m_blobAttributes, (short)nCharBlobNum,
                i_pMarkSpec->m_pnLocID, i_pMarkSpec->m_prtLocaterTeachingBox))
        {
            return FALSE;
        }

        ///============좌표 변환:: Pane ROI를 기준으로 좌표를 변환한다.========================////
        for (long n = 0; n < i_pMarkSpec->m_nCharNum; n++)
        {
            i_pMarkSpec->m_plTeachCharROI[n]
                = i_pMarkSpec->m_plTeachCharROI[n] - Ipvm::Conversion::ToPoint32s2(imageCenter);
        }
        for (long n = 0; n < NUM_OF_LOCATOR; n++)
        {
            i_pMarkSpec->m_prtLocaterTeachingBox[n]
                = i_pMarkSpec->m_prtLocaterTeachingBox[n] - Ipvm::Conversion::ToPoint32s2(imageCenter);
        }

        i_pMarkSpec->m_ptLocCenterforScreen = i_pMarkSpec->m_ptLocCenterforScreen - imageCenter;

        ///=====================================================================================////

        MakeMarkSpecImageFromTeachPane(thresImage, i_pMarkSpec->m_specImage);
    }

    i_pMarkSpec->m_nCharMinArea = (long)(LONG_MAX);
    for (long i = 0; i < nCharBlobNum; i++)
        i_pMarkSpec->m_nCharMinArea
            = (long)min(i_pMarkSpec->m_nCharMinArea, i_pMarkSpec->m_plTeachCharArea[i]); //kircheis_MarkAngle

    return TRUE;
}

BOOL VisionInspectionMark::CheckMarkAngle(long nLineNumber, float& fAngleChek)
{
    std::vector<Ipvm::Point32r2> vecMarkPositionXY;
    Ipvm::LineEq32r coeff;

    std::vector<float> calcAngle;

    if (nLineNumber > 1)
    {
        for (long nLinenum = 0; nLinenum < 2; nLinenum++)
        {
            vecMarkPositionXY.clear();

            if (nLinenum == 0)
            {
                for (long nMarkStart = m_VisionTempSpec->m_veclTeachStartCharIDPerLine[nLinenum];
                    nMarkStart < m_VisionTempSpec->m_veclTeachStartCharIDPerLine[nLinenum + 1]; nMarkStart++)
                {
                    if (m_VisionPara->m_vecbSelectIgnore[nMarkStart] == TRUE)
                    {
                        continue;
                    }
                    vecMarkPositionXY.emplace_back((float)m_psBlobInfo[nMarkStart].m_roi.CenterPoint().m_x,
                        (float)m_psBlobInfo[nMarkStart].m_roi.CenterPoint().m_y);
                }

                if (vecMarkPositionXY.size() == 0)
                {
                    return FALSE;
                }

                if (Ipvm::DataFitting::FitToLine((long)vecMarkPositionXY.size(), &vecMarkPositionXY[0], coeff)
                    == Ipvm::Status::e_ok)
                {
                    calcAngle.push_back(CAST_FLOAT(atan(-coeff.m_a / coeff.m_b)));
                }
            }
            else
            {
                long nEnd = (long)m_VisionTempSpec->m_veclTeachStartCharIDPerLine.size() > 2
                    ? m_VisionTempSpec->m_veclTeachStartCharIDPerLine[nLinenum + 1]
                    : (long)m_result->m_vecrtCharPositionforCalcImage.size();
                for (long nMarkStart = m_VisionTempSpec->m_veclTeachStartCharIDPerLine[nLinenum]; nMarkStart < nEnd;
                    nMarkStart++)
                {
                    if (m_VisionPara->m_vecbSelectIgnore[nMarkStart] == TRUE)
                    {
                        continue;
                    }

                    vecMarkPositionXY.emplace_back((float)m_psBlobInfo[nMarkStart].m_roi.CenterPoint().m_x,
                        (float)m_psBlobInfo[nMarkStart].m_roi.CenterPoint().m_y);
                }
                if (vecMarkPositionXY.size() == 0)
                {
                    return FALSE;
                }

                if (Ipvm::DataFitting::FitToLine((long)vecMarkPositionXY.size(), &vecMarkPositionXY[0], coeff)
                    == Ipvm::Status::e_ok)
                {
                    calcAngle.push_back(CAST_FLOAT(atan(-coeff.m_a / coeff.m_b)));
                }
            }
        }
    }
    else if (nLineNumber == 1)
    {
        vecMarkPositionXY.clear();

        for (long nMarkStart = 0; nMarkStart < m_result->m_vecrtCharPositionforCalcImage.size(); nMarkStart++)
        {
            vecMarkPositionXY.emplace_back((float)m_psBlobInfo[nMarkStart].m_roi.CenterPoint().m_x,
                (float)m_psBlobInfo[nMarkStart].m_roi.CenterPoint().m_y);
        }

        if (Ipvm::DataFitting::FitToLine((long)vecMarkPositionXY.size(), &vecMarkPositionXY[0], coeff)
            == Ipvm::Status::e_ok)
        {
            calcAngle.push_back(CAST_FLOAT(atan(-coeff.m_a / coeff.m_b)));
        }
    }

    auto fLocAngle = 0.f;
    if (calcAngle.size())
    {
        for (auto& angle : calcAngle)
            fLocAngle += angle;
        fLocAngle /= float(calcAngle.size());
    }

    if (abs(fLocAngle) > (float)ITP_HalfPI / 2.0f)
    {
        if (fLocAngle > m_VisionTempSpec->m_fLocAngleForBody)
            fLocAngle -= (float)ITP_HalfPI;
        else
            fLocAngle += (float)ITP_HalfPI;
    }

    fAngleChek = fLocAngle * (float)ITP_RAD_TO_DEG;

    // 	if (fabs(fAngleChek) > 2.f)
    // 	{
    // // 		return FALSE;
    // 	}

    return TRUE;
}

BOOL VisionInspectionMark::DoCharNumberCheckTeach(
    Ipvm::Image32s& io_label, Ipvm::BlobInfo* psCharInfo, int32_t& nCharBlobNum, float fMergeGap)
{
    if (nCharBlobNum > BLOB_INFO_SMALL_SIZE || nCharBlobNum <= 0)
    {
        return FALSE;
    }

    ///1) Blob으로 잡힌놈들을 Filtering 한다.
    if (!MergeMarkForTeach(psCharInfo, nCharBlobNum, io_label, fMergeGap))
        return FALSE;

    if (!DoAreaThreshold(psCharInfo, nCharBlobNum))
        return FALSE;

    return TRUE;
}

BOOL VisionInspectionMark::MergeMarkForTeach(
    Ipvm::BlobInfo* psCharInfo, int32_t& nCharBlobNum, Ipvm::Image32s& io_label, float fBlobDist)
{
    /// Blob외각 포인트를 이용한 거리 측정후 Merge한다. 다른쪽으로 흡수된 Blob은 area가 0이다.
    if (m_pBlob->MergeBlobsByDistance(psCharInfo, nCharBlobNum, fBlobDist, io_label))
    {
        return FALSE;
    }

    /// 위에서 Merge를 했으면 BlobInfo정보(Blob갯수, Blob Area)가 틀려졌으니까 갱신하자.
    std::vector<Ipvm::BlobInfo> sTempBlobInfo;
    Ipvm::BlobInfo sBlob;
    for (short i = 0; i < nCharBlobNum; i++)
    {
        if (psCharInfo[i].m_area != 0)
        {
            sBlob = psCharInfo[i];
            sTempBlobInfo.push_back(sBlob);
        }
    }
    nCharBlobNum = (short)sTempBlobInfo.size();
    for (short i = 0; i < nCharBlobNum; i++)
    {
        psCharInfo[i] = sTempBlobInfo[i];
    }
    return TRUE;
}

BOOL VisionInspectionMark::DoAreaThreshold(Ipvm::BlobInfo* psCharInfo, int32_t& nCharBlobNum)
{
    Ipvm::Rect32s imageRoi(0, 0, getReusableMemory().GetInspImageSizeX(), getReusableMemory().GetInspImageSizeY());
    Ipvm::Point32r2 imageCenter(imageRoi.Width() * 0.5f, imageRoi.Height() * 0.5f);

    if (psCharInfo == NULL)
        return FALSE;

    //// Ignore 영역 구하기.
    std::vector<Ipvm::Rect32s> vecrtUserIgnore;
    for (long i = 0; i < (long)m_VisionPara->m_vecrtUserIgnore.size(); i++)
    {
        Ipvm::Rect32s rtUserROI = m_VisionPara->m_vecrtUserIgnore[i] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        vecrtUserIgnore.push_back(rtUserROI);
    }

    long nCharAreaThresh = 0; //kircheis_580 MarkMT
    if (m_VisionTempSpec->m_nCharMinArea == (long)LONG_MAX || m_VisionTempSpec->m_nCharMinArea <= 0)
        nCharAreaThresh = (long)(m_VisionPara->sMarkAlgoParam.nBlobSizeMin); //kircheis_MarkAngle
    else
        nCharAreaThresh = (long)((float)m_VisionTempSpec->m_nCharMinArea * 0.9f + .5f); //kircheis_MarkAngle
    BOOL bValidLocater = !(m_pptLocater[0].m_x == 0 && m_pptLocater[0].m_y == 0);
    for (short i = 0; i < nCharBlobNum; i++)
    {
        if (nCharBlobNum <= 3)
            continue;

        Ipvm::Rect32s charectorROI = psCharInfo[i].m_roi;

        /// Blobsize가 너무 크거나, 작으면 지움.
        if (psCharInfo[i].m_area < m_VisionPara->sMarkAlgoParam.nBlobSizeMin
            || psCharInfo[i].m_area > m_VisionPara->sMarkAlgoParam.nBlobSizeMax
            || (psCharInfo[i].m_area < nCharAreaThresh && bValidLocater)) //kircheis_MarkAngle
        {
            psCharInfo[i].m_area = 0;
        }

        /// Ignore 박스에 걸친 Blob 지움
        for (long n = 0; n < (long)vecrtUserIgnore.size(); n++)
        {
            Ipvm::Rect32s rtUsetIgnore = vecrtUserIgnore[n];
            if ((rtUsetIgnore & charectorROI) == charectorROI)
            {
                psCharInfo[i].m_area = 0;
            }
        }

        /// Package영역 밖에 있는 Blob은 지움.
        Ipvm::Rect32s rtBodyROI
            = Ipvm::Rect32s(int32_t(min((long)m_bodyAlignResult->fptLT.m_x, (long)m_bodyAlignResult->fptLB.m_x)),
                int32_t(min((long)m_bodyAlignResult->fptLT.m_y, (long)m_bodyAlignResult->fptRT.m_y)),
                int32_t(max((long)m_bodyAlignResult->fptRT.m_x, (long)m_bodyAlignResult->fptRB.m_x)),
                int32_t(max((long)m_bodyAlignResult->fptLB.m_y, (long)m_bodyAlignResult->fptRB.m_y)));

        rtBodyROI &= imageRoi;

        if ((rtBodyROI & charectorROI) != charectorROI)
        {
            // Charector Blob ROI가 Body ROI를 벗어났다
            psCharInfo[i].m_area = 0;
        }
    }

    /// 위에서 Blob작은거 제거했으니깐 Blob정보가 바뀌었다. 갱신하자.
    std::vector<Ipvm::BlobInfo> sTempBlobInfo;
    Ipvm::BlobInfo sBlob;
    for (short i = 0; i < nCharBlobNum; i++)
    {
        if (psCharInfo[i].m_area != 0)
        {
            sBlob = psCharInfo[i];
            sTempBlobInfo.push_back(sBlob);
        }
    }

    nCharBlobNum = (short)sTempBlobInfo.size();
    for (short i = 0; i < nCharBlobNum; i++)
    {
        psCharInfo[i] = sTempBlobInfo[i];
    }

    return TRUE;
}

void VisionInspectionMark::SortingCharsY(
    Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute, long nCharBlobNum, BOOL bTopToBottom) //kircheis_0401
{
    if (bTopToBottom)
    {
        for (short i = 0; i < nCharBlobNum; i++)
        {
            for (short j = 0; j < nCharBlobNum - i - 1; j++)
            {
                if (charAttribute[j].m_massCenter.m_y > charAttribute[j + 1].m_massCenter.m_y)
                {
                    std::swap(psCharInfo[j], psCharInfo[j + 1]);
                    std::swap(charAttribute[j], charAttribute[j + 1]);
                }
            }
        }
    }
    else
    {
        for (short i = 0; i < nCharBlobNum; i++)
        {
            for (short j = 0; j < nCharBlobNum - i - 1; j++)
            {
                if (charAttribute[j].m_massCenter.m_y < charAttribute[j + 1].m_massCenter.m_y)
                {
                    std::swap(psCharInfo[j], psCharInfo[j + 1]);
                    std::swap(charAttribute[j], charAttribute[j + 1]);
                }
            }
        }
    }
}

BOOL VisionInspectionMark::CalTextLineNumber(Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute,
    long nCharBlobNum, short& nLineNumber, std::vector<long>& vCharNum, std::vector<long>& vLineStartCharID,
    BOOL bToptoBottom)
{
    if (nCharBlobNum == 0)
    {
        nLineNumber = 0;
        return FALSE;
    }

    //m_VisionTempSpec->m_p
    if (m_VisionPara->m_nMarkDirection == ENUM_MARKDIR_HOR)
    {
        /// Horizontal 방향이면 Y축으로 Sortting하여, Group을 형성한다.
        SortingCharsY(psCharInfo, charAttribute, nCharBlobNum, bToptoBottom);

        std::vector<long> vYPos(nCharBlobNum);
        long nSumHeight = 0;

        for (short i = 0; i < nCharBlobNum; i++)
        {
            vYPos[i] = long(charAttribute[i].m_massCenter.m_y + 0.5f);
            nSumHeight += psCharInfo[i].m_roi.m_bottom - psCharInfo[i].m_roi.m_top;
        }

        long nQuaterHeight = nSumHeight / nCharBlobNum;
        // 	long nQuaterHeight = nSumHeight / nCharBlobNum*4;//kircheis_0331

        nLineNumber = 1;
        vCharNum.push_back(1);
        vLineStartCharID.push_back(0);

        for (short i = 1; i < nCharBlobNum; i++)
        {
            if (vYPos[i] - vYPos[i - 1] < nQuaterHeight)
            {
                vCharNum[nLineNumber - 1] += 1;
            }
            else
            {
                vCharNum.push_back(1);
                vLineStartCharID.push_back(i);
                nLineNumber++;
            }
        }

        for (long i = 0; i < nLineNumber; i++)
            SortingCharsX(
                psCharInfo + vLineStartCharID[i], charAttribute + vLineStartCharID[i], (short)vCharNum[i], TRUE);
    }
    else
    {
        /// Virtical 방향이면 X축으로 Sortting하여, Group을 형성한다.
        SortingCharsX(psCharInfo, charAttribute, nCharBlobNum, bToptoBottom);

        std::vector<long> vXPos(nCharBlobNum);
        long nSumWidth = 0;

        for (short i = 0; i < nCharBlobNum; i++)
        {
            vXPos[i] = long(charAttribute[i].m_massCenter.m_x + 0.5f);
            nSumWidth += psCharInfo[i].m_roi.m_right - psCharInfo[i].m_roi.m_left;
        }

        long nQuaterWidth = nSumWidth / nCharBlobNum;
        // 	long nQuaterHeight = nSumHeight / nCharBlobNum*4;//kircheis_0331

        nLineNumber = 1;
        vCharNum.push_back(1);
        vLineStartCharID.push_back(0);

        for (short i = 1; i < nCharBlobNum; i++)
        {
            if (vXPos[i] - vXPos[i - 1] < nQuaterWidth)
            {
                vCharNum[nLineNumber - 1] += 1;
            }
            else
            {
                vCharNum.push_back(1);
                vLineStartCharID.push_back(i);
                nLineNumber++;
            }
        }

        for (long i = 0; i < nLineNumber; i++)
            SortingCharsY(
                psCharInfo + vLineStartCharID[i], charAttribute + vLineStartCharID[i], (short)vCharNum[i], TRUE);
    }

    return TRUE;
}

void VisionInspectionMark::SortingCharsX(
    Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute, long nCharBlobNum, BOOL bLeftToRight) //kircheis_0401
{
    if (bLeftToRight)
    {
        for (short i = 0; i < nCharBlobNum; i++)
        {
            for (short j = 0; j < nCharBlobNum - i - 1; j++)
            {
                if (charAttribute[j].m_massCenter.m_x > charAttribute[j + 1].m_massCenter.m_x)
                {
                    std::swap(psCharInfo[j], psCharInfo[j + 1]);
                    std::swap(charAttribute[j], charAttribute[j + 1]);
                }
            }
        }
    }
    else
    {
        for (short i = 0; i < nCharBlobNum; i++)
        {
            for (short j = 0; j < nCharBlobNum - i - 1; j++)
            {
                if (charAttribute[j].m_massCenter.m_x < charAttribute[j + 1].m_massCenter.m_x)
                {
                    std::swap(psCharInfo[j], psCharInfo[j + 1]);
                    std::swap(charAttribute[j], charAttribute[j + 1]);
                }
            }
        }
    }
}

void VisionInspectionMark::SetMarkROI(long markIndex, const Ipvm::Rect32s& roi, const float fMarkAngle)
{
    Ipvm::Rect32s imageRoi(0, 0, getReusableMemory().GetInspImageSizeX(), getReusableMemory().GetInspImageSizeY());
    Ipvm::Point32r2 imageCenter(imageRoi.Width() * 0.5f, imageRoi.Height() * 0.5f);

    m_bodyAlignResult = GetAlignInfo();
    if (m_bodyAlignResult == nullptr)
        return;
    if (markIndex < 0 || markIndex >= long(m_result->m_vecrtCharPositionforCalcImage.size())
        || markIndex >= long(m_result->m_vecrtCharPositionforImage.size()))
    {
        ASSERT(!_T("??"));
        return;
    }

    m_result->m_vecrtCharPositionforCalcImage[markIndex] = roi;

    //Src image를 돌린다
    Ipvm::Point32r2 bodyCenter = m_bodyAlignResult->m_center;
    float angle_rad = m_bodyAlignResult->m_angle_rad + (float)(m_VisionPara->m_nMarkAngle_deg * ITP_DEG_TO_RAD);

    Ipvm::Rect32s rtROIRef(m_VisionTempSpec->m_plTeachCharROI[markIndex] + Ipvm::Conversion::ToPoint32s2(imageCenter));

    Ipvm::Point32r2 tempXY(roi.m_left + rtROIRef.Width() * 0.5f, roi.m_top + rtROIRef.Height() * 0.5f);

    // 영훈 : 정확성을 위해 Float 형으로 Rotate를 시킨다.
    CPI_Geometry::RotatePoint(tempXY, angle_rad, bodyCenter);

    if (m_VisionPara->m_nROISettingMethod != ROISettingMethod_Manual)
    {
        Ipvm::Point32r2 SearchROICenter(
            m_result->m_sMarkMapInfo_px.sTeachROI_Info.fOffsetX, m_result->m_sMarkMapInfo_px.sTeachROI_Info.fOffsetY);
        CPI_Geometry::RotatePoint(tempXY, fMarkAngle * ITP_DEG_TO_RAD, SearchROICenter);
    }

    Ipvm::Point32s2 pPoint
        = Ipvm::Point32s2(long(tempXY.m_x - rtROIRef.Width() * 0.5f), long(tempXY.m_y - rtROIRef.Height() * 0.5f));
    Ipvm::Rect32s rtChage = Ipvm::Rect32s(pPoint, pPoint + Ipvm::Point32s2(rtROIRef.Width(), rtROIRef.Height()));
    rtChage.NormalizeRect();

    m_result->m_vecrtCharPositionforImage[markIndex] = rtChage;
}

struct MinMisMatchBlobInfo
{
    long nIndex;
    //제일 작은 MisMatch값을 저장할것이다.
    //이값이 크면 클수록 마크중에 같은 모양의 마크가 없다는 뜻이다.
    //나중에 이넘의 제일 큰값으로 소팅하여 선착순 절반으로만 최대 멀리 떨어져있는 포인트를 찾는다.
    long nMinMisMatch;

    MinMisMatchBlobInfo()
    {
        nIndex = -1;
        nMinMisMatch = -1;
    };
};

BOOL VisionInspectionMark::GetDefaultRefID_Auto(
    const Ipvm::Image8u& i_binImage, Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute, long nNum, int* pnRefID)
{
    //mc_사용하는곳은 없고.. 예외처리로 사용하자
    if (i_binImage.GetMem() == nullptr)
        return FALSE;

    int nMisMatchEach, nMinMisMatch;
    std::vector<MinMisMatchBlobInfo> vecsMMMBI;
    MinMisMatchBlobInfo sMMMBITemp;

    if (nNum > 1)
    {
        //한화면에 비슷한 마크가 있으면 그넘은 로케이터로 잡지않기위해서 메칭률을 확인한다.
        vecsMMMBI.clear();
        for (long i = 0; i < nNum; i++)
        {
            nMinMisMatch = INT_MAX;
            for (long j = i; j < nNum; j++)
            {
                if (i != j)
                {
                    if (abs(psCharInfo[i].m_area - psCharInfo[j].m_area)
                        > min(psCharInfo[i].m_area, psCharInfo[j].m_area) * 0.5f)
                        continue;

                    Ipvm::Rect32s rtRef = psCharInfo[i].m_roi;
                    Ipvm::Rect32s rtEach = psCharInfo[i].m_roi;

                    if (!CharMatching(m_VisionTempSpec->m_specImage, rtRef, rtEach, nMisMatchEach))
                        return FALSE;

                    if (nMinMisMatch > nMisMatchEach)
                    {
                        nMinMisMatch = nMisMatchEach;
                    }
                }
            }

            // 판단기준이 좀 모호하다... 0.03 ???
            if (nMinMisMatch != INT_MAX && (psCharInfo[i].m_area * 0.03f) < nMinMisMatch)
            {
                sMMMBITemp.nIndex = i;
                sMMMBITemp.nMinMisMatch = nMinMisMatch;
                vecsMMMBI.push_back(sMMMBITemp);
            }
        }

        if (NUM_OF_LOCATOR > (long)vecsMMMBI.size())
        {
            vecsMMMBI.clear();
            for (long i = 0; i < nNum; i++)
            {
                sMMMBITemp.nIndex = i;
                sMMMBITemp.nMinMisMatch = psCharInfo[i].m_area;
                vecsMMMBI.push_back(sMMMBITemp);
            }
        }

        long nSize = (long)vecsMMMBI.size();
        double dX, dY;
        double dDistance = 0;
        double dMaxDistance = 0;
        //---->2007.04.20 bok 평균크기의 후보중에서 가장먼것을 로케이터로 지정한다.
        for (long i = 0; i < nSize; i++)
        {
            for (long j = i + 1; j < nSize; j++)
            {
                dX = charAttribute[vecsMMMBI[i].nIndex].m_massCenter.m_x
                    - charAttribute[vecsMMMBI[j].nIndex].m_massCenter.m_x;
                dY = charAttribute[vecsMMMBI[i].nIndex].m_massCenter.m_y
                    - charAttribute[vecsMMMBI[j].nIndex].m_massCenter.m_y;
                dDistance = sqrt(pow(dX, 2) + pow(dY, 2));
                if (dMaxDistance < dDistance
                    && (abs(psCharInfo[vecsMMMBI[i].nIndex].m_area - psCharInfo[vecsMMMBI[j].nIndex].m_area)
                        <= min(psCharInfo[vecsMMMBI[i].nIndex].m_area, psCharInfo[vecsMMMBI[j].nIndex].m_area) * 0.5f))
                {
                    dMaxDistance = dDistance;
                    pnRefID[0] = vecsMMMBI[i].nIndex;
                    pnRefID[1] = vecsMMMBI[j].nIndex;
                }
            }
        }
        //<----2007.04.20 bok 평균크기의 후보중에서 가장먼것을 로케이터로 지정한다.
        //---->2007.04.20 bok 로케이터로 지정이 안되면 가장 먼것으로 한다.
        if (dMaxDistance == 0)
        {
            for (long i = 0; i < nSize; i++)
            {
                for (long j = i + 1; j < nSize; j++)
                {
                    dX = charAttribute[vecsMMMBI[i].nIndex].m_massCenter.m_x
                        - charAttribute[vecsMMMBI[j].nIndex].m_massCenter.m_x;
                    dY = charAttribute[vecsMMMBI[i].nIndex].m_massCenter.m_y
                        - charAttribute[vecsMMMBI[j].nIndex].m_massCenter.m_y;
                    dDistance = sqrt(pow(dX, 2) + pow(dY, 2));
                    if (dMaxDistance < dDistance)
                    {
                        dMaxDistance = dDistance;
                        pnRefID[0] = vecsMMMBI[i].nIndex;
                        pnRefID[1] = vecsMMMBI[j].nIndex;
                    }
                }
            }
        }
        //<----2007.04.20 bok 로케이터로 지정이 안되면 가장 먼것으로 한다.
    }
    else if (nNum == 1)
    {
        pnRefID[0] = 0;
        pnRefID[1] = 0;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetDefaultRefID_Manual(const Ipvm::Image8u& i_binImage, Ipvm::BlobInfo* psCharInfo,
    BlobAttribute* charAttribute, long nNum, Ipvm::Rect32s* prtLocaterSearchROI, int* pnRefID)
{
    if (i_binImage.GetMem() == nullptr)
        return FALSE;

    int nMisMatchEach, nMinMisMatch;
    std::vector<MinMisMatchBlobInfo> vecsMMMBI;
    std::vector<long> veclMisMatch;
    MinMisMatchBlobInfo sMMMBITemp;

    if (nNum > 1)
    {
        pnRefID[0] = -1;
        pnRefID[1] = -1;
        for (int nLocatorID = 0; nLocatorID < NUM_OF_LOCATOR; nLocatorID++)
        {
            //한화면에 비슷한 마크가 있으면 그넘은 로케이터로 잡지않기위해서 메칭률을 확인한다.
            veclMisMatch.clear();
            vecsMMMBI.clear();
            for (long i = 0; i < nNum; i++)
            {
                Ipvm::Rect32s rtLocatorTargetROI = prtLocaterSearchROI[nLocatorID];
                nMinMisMatch = INT_MAX;
                Ipvm::Rect32s rtRef = psCharInfo[i].m_roi;
                if (rtLocatorTargetROI.PtInRect(Ipvm::Point32s2(rtRef.CenterPoint())))
                {
                    for (long j = 0; j < nNum; j++)
                    {
                        if (i != j)
                        {
                            Ipvm::Rect32s rtEach = psCharInfo[j].m_roi;
                            if (rtLocatorTargetROI.PtInRect(Ipvm::Point32s2(rtEach.CenterPoint())))
                            {
                                if (!CharMatching(m_VisionTempSpec->m_specImage, rtRef, rtEach, nMisMatchEach))
                                    return FALSE;

                                if (nMinMisMatch > nMisMatchEach)
                                {
                                    nMinMisMatch = nMisMatchEach;
                                }
                            }
                        }
                    }

                    sMMMBITemp.nIndex = i;
                    sMMMBITemp.nMinMisMatch = nMinMisMatch;
                    vecsMMMBI.push_back(sMMMBITemp);
                    veclMisMatch.push_back(nMinMisMatch);
                }
            }

            if ((0 >= (long)vecsMMMBI.size() || 0 >= (long)veclMisMatch.size())
                || ((long)vecsMMMBI.size() != (long)veclMisMatch.size()))
            {
                break;
            }

            //---->2008.07.25 bok 1개면 1개가, 2개 이상이면 2번째 차이가 큰것.
            long nSize = (long)vecsMMMBI.size();

            std::sort(veclMisMatch.begin(), veclMisMatch.end());

            nMisMatchEach = veclMisMatch[(long)veclMisMatch.size() - 1];

            for (long i = 0; i < nSize; i++)
            {
                if (nMisMatchEach == vecsMMMBI[i].nMinMisMatch)
                {
                    pnRefID[nLocatorID] = vecsMMMBI[i].nIndex;
                    break;
                }
            }
            //<----2008.07.25 bok 1개면 1개가, 2개 이상이면 2번째 차이가 큰것.
        }
        //---->2008.07.25 bok 로케이터로 지정이 안되면 가장 먼것으로 한다.

        if (-1 == pnRefID[0] || -1 == pnRefID[1])
        {
            double dMaxDistance = 0;
            double dDistance;

            for (long i = 0; i < nNum; i++)
            {
                for (long j = i + 1; j < nNum; j++)
                {
                    if (i != j)
                    {
                        float diffX = charAttribute[i].m_massCenter.m_x - charAttribute[j].m_massCenter.m_x;
                        float diffY = charAttribute[i].m_massCenter.m_y - charAttribute[j].m_massCenter.m_y;
                        dDistance = sqrt(pow(diffX, 2) + pow(diffY, 2));
                        if (dMaxDistance < dDistance)
                        {
                            dMaxDistance = dDistance;
                            pnRefID[0] = i;
                            pnRefID[1] = j;
                        }
                    }
                }
            }
        }
        //<----2008.07.25 bok 로케이터로 지정이 안되면 가장 먼것으로 한다.
    }
    else if (nNum == 1)
    {
        pnRefID[0] = 0;
        pnRefID[1] = 0;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::CharMatching(
    const Ipvm::Image8u& i_theshImage, Ipvm::Rect32s rtROIRef, Ipvm::Rect32s rtROI, int& nMisMatch)
{
    const int imageSizeX = getImageLotInsp().GetImageSizeX();
    const int imageSizeY = getImageLotInsp().GetImageSizeY();

    int nx = abs(rtROI.Width() - rtROIRef.Width());
    int ny = abs(rtROI.Height() - rtROIRef.Height());

    Ipvm::Rect32s searchROI;
    searchROI.m_left = (rtROI.Width() > rtROIRef.Width()) ? rtROI.m_left : rtROI.m_left - nx;
    searchROI.m_right = searchROI.m_left + nx + 1;
    searchROI.m_top = (rtROI.Height() > rtROIRef.Height()) ? rtROI.m_top : rtROI.m_top - ny;
    searchROI.m_bottom = searchROI.m_top + ny + 1;
    searchROI &= Ipvm::Rect32s(0, 0, imageSizeX - rtROIRef.Width(), imageSizeY - rtROIRef.Height());

    // Ref의 255 픽셀 개수 계산
    int nScoreTot = 0;
    for (long y = rtROIRef.m_top; y < rtROIRef.m_bottom; y++)
    {
        auto* threh_y = i_theshImage.GetMem(0, y);
        for (long x = rtROIRef.m_left; x < rtROIRef.m_right; x++)
        {
            if (threh_y[x])
            {
                nScoreTot++;
            }
        }
    }

    // Score 계산
    int nScore = 0;
    int nScoreMax = -nScoreTot;

    BYTE nRef, nCur;
    int nROIRefWidth = rtROIRef.Width();
    int nROIRefHeight = rtROIRef.Height();

    for (long y = searchROI.m_top; y < searchROI.m_bottom; y++)
    {
        for (long x = searchROI.m_left; x < searchROI.m_right; x++)
        {
            nScore = 0;
            for (long j = 0; j < nROIRefHeight; j++)
            {
                auto* ref_y = i_theshImage.GetMem(0, j + rtROIRef.m_top);
                auto* cur_y = i_theshImage.GetMem(0, j + y);
                for (long i = 0; i < nROIRefWidth; i++)
                {
                    nRef = ref_y[i + rtROIRef.m_left];
                    nCur = cur_y[i + x];

                    if (nRef && nCur)
                        nScore++;
                    else if (nRef || nCur)
                        nScore--;
                }
            }

            if (nScore > nScoreMax)
                nScoreMax = nScore;
        }
    }

    if (nScoreTot <= 0)
        return FALSE;

    nMisMatch = 100 * (nScoreTot - nScoreMax) / nScoreTot;

    return TRUE;
}

BOOL VisionInspectionMark::GetLocaterSearchBox(Ipvm::Rect32s* o_prtLocaterSearch, Ipvm::Point32r2& o_ptLocaterCenter)
{
    Ipvm::Rect32s imageRoi(0, 0, getReusableMemory().GetInspImageSizeX(), getReusableMemory().GetInspImageSizeY());
    Ipvm::Point32r2 imageCenter(imageRoi.Width() * 0.5f, imageRoi.Height() * 0.5f);

    /// Locater Center를 좌표 변환함.
    o_ptLocaterCenter = m_VisionTempSpec->m_ptLocCenterforScreen + imageCenter;

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    for (int i = 0; i < NUM_OF_LOCATOR; i++)
    {
        o_prtLocaterSearch[i]
            = m_VisionTempSpec->m_prtLocaterTeachingBox[i] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        o_prtLocaterSearch[i].InflateRect(m_VisionPara->m_nLocSearchOffsetX, m_VisionPara->m_nLocSearchOffsetY,
            m_VisionPara->m_nLocSearchOffsetX, m_VisionPara->m_nLocSearchOffsetY);

        if (!CheckROI(o_prtLocaterSearch[i], imageSizeX, imageSizeY))
            return FALSE;
    }

    return TRUE;
}

// Locater 위치 계산
BOOL VisionInspectionMark::FindLocater(long i_nNumOfLocator, const Ipvm::Image8u& i_refImage,
    const Ipvm::Image8u& i_binImage, long i_nImageSampleRate, Ipvm::Rect32s* i_prtRefROI, Ipvm::Rect32s* i_prtSerachROI,
    Ipvm::Point32s2* o_pptLocater)
{
    if (i_nNumOfLocator <= 0 || i_nNumOfLocator > 2)
        i_nNumOfLocator = 2;

    Ipvm::Rect32s imageRoi(i_binImage);

    BOOL isLocaterExist = FALSE;

    // Image Sample Rate
    double dImageSampleRateInverse = 1.0 / i_nImageSampleRate;

    // 티칭된 로케이터의 사이즈
    Ipvm::Rect32s rtROISpec[NUM_OF_LOCATOR];
    Ipvm::Rect32s rtSearchROI[NUM_OF_LOCATOR];
    long nImgNormalX[NUM_OF_LOCATOR], nImgNormalY[NUM_OF_LOCATOR];

    Ipvm::Image8u refResize[NUM_OF_LOCATOR];
    Ipvm::Image8u srcResize[NUM_OF_LOCATOR];

    long nSearchRangeX = 0;
    long nSearchRangeY = 0;

    long nTempX[NUM_OF_LOCATOR];
    long nTempY[NUM_OF_LOCATOR];

    BOOL bResult = TRUE;

    for (long i = 0; i < i_nNumOfLocator; i++)
    {
        rtROISpec[i] = i_prtRefROI[i];
        rtROISpec[i].InflateRect(
            m_VisionPara->sMarkAlgoParam.nMarkRoiMargin, m_VisionPara->sMarkAlgoParam.nMarkRoiMargin);
        rtROISpec[i] &= imageRoi;

        rtSearchROI[i] = i_prtSerachROI[i];
        rtSearchROI[i].InflateRect(
            m_VisionPara->sMarkAlgoParam.nMarkRoiMargin, m_VisionPara->sMarkAlgoParam.nMarkRoiMargin);
        rtSearchROI[i] &= imageRoi;

        nImgNormalX[i] = rtROISpec[i].Width();
        nImgNormalY[i] = rtROISpec[i].Height();

        int nxRefResize = (int)(rtROISpec[i].Width()
            * dImageSampleRateInverse); //맘에 안들어도 소수점 변환은 하지 말도록.. 360과 검사 결과를 일치 시키기 위함임.
        int nyRefResize = (int)(rtROISpec[i].Height() * dImageSampleRateInverse);

        refResize[i].Create(nxRefResize, nyRefResize);
        srcResize[i].Create(nxRefResize, nyRefResize);

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(refResize[i]), 0, refResize[i]);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(srcResize[i]), 0, srcResize[i]);

        Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(i_refImage, rtROISpec[i]), refResize[i]);

        nTempX[i] = rtSearchROI[i].Width() - nImgNormalX[i];
        nTempY[i] = rtSearchROI[i].Height() - nImgNormalY[i];
    }

    if (i_nNumOfLocator == NUM_OF_LOCATOR)
    {
        nSearchRangeX = (nTempX[0] + nTempX[1]) / NUM_OF_LOCATOR;
        nSearchRangeY = (nTempY[0] + nTempY[1]) / NUM_OF_LOCATOR;
    }
    else
    {
        nSearchRangeX = nTempX[0];
        nSearchRangeY = nTempY[0];
    }

    Ipvm::Point32s2 ptScoreMax = Ipvm::Point32s2(-1, -1);
    double fScoreMax = -100000000.f;
    double fScoreTemp[NUM_OF_LOCATOR];
    double fScoreTotal;

    for (long y = 0; y < nSearchRangeY; y = y + i_nImageSampleRate)
    {
        for (long x = 0; x < nSearchRangeX; x = x + i_nImageSampleRate)
        {
            for (long i = 0; i < i_nNumOfLocator; i++)
            {
                Ipvm::Rect32s rtSrc = Ipvm::Rect32s(int32_t(rtSearchROI[i].m_left + x),
                    int32_t(rtSearchROI[i].m_top + y), int32_t(rtSearchROI[i].m_left + x + nImgNormalX[i]),
                    int32_t(rtSearchROI[i].m_top + y + nImgNormalY[i]));

                if (Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(i_binImage, rtSrc), srcResize[i])
                    != Ipvm::Status::e_ok)
                {
                    bResult = FALSE;
                }

                // source, reference 모두 255인 픽셀수

                if (!GetMatchScore(refResize[i], srcResize[i], fScoreTemp[i]))
                {
                    return FALSE;
                }
            }

            if (i_nNumOfLocator == NUM_OF_LOCATOR)
            {
                fScoreTotal = (fScoreTemp[0] + fScoreTemp[1]) / 2.0f;
            }
            else
            {
                fScoreTotal = fScoreTemp[0];
            }

            // score match 가 최고점인 x, y 값 저장
            if (fScoreTotal > fScoreMax)
            {
                fScoreMax = fScoreTotal;
                ptScoreMax.m_x = x + m_VisionPara->sMarkAlgoParam.nMarkRoiMargin;
                ptScoreMax.m_y = y + m_VisionPara->sMarkAlgoParam.nMarkRoiMargin;
                isLocaterExist = TRUE;
            }
        }
    }

    bResult &= isLocaterExist;

    ///MaxScore를 찾지 못했다면, Return;
    if (isLocaterExist == FALSE || ptScoreMax.m_x < 0 || ptScoreMax.m_y < 0)
        return FALSE;

    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        if (i_nNumOfLocator == NUM_OF_LOCATOR)
        {
            o_pptLocater[i]
                = Ipvm::Point32s2(rtSearchROI[i].m_left + ptScoreMax.m_x, rtSearchROI[i].m_top + ptScoreMax.m_y);
        }
        else
        {
            o_pptLocater[i]
                = Ipvm::Point32s2(rtSearchROI[0].m_left + ptScoreMax.m_x, rtSearchROI[0].m_top + ptScoreMax.m_y);
        }
    }

    return bResult;
}

BOOL VisionInspectionMark::FindLocaterReformed(long i_nNumOfLocator, const Ipvm::Image8u& i_refImage,
    const Ipvm::Image8u& i_binImage, long i_nImageSampleRate, Ipvm::Rect32s* i_prtRefROI, Ipvm::Rect32s* i_prtSerachROI,
    Ipvm::Point32s2* o_pptLocater)
{
    if (i_nNumOfLocator <= 0 || i_nNumOfLocator > 2)
        i_nNumOfLocator = 2;

    Ipvm::Rect32s imageRoi(i_binImage);

    BOOL isLocaterExist = FALSE;

    // Image Sample Rate
    double dImageSampleRateInverse = 1.0 / i_nImageSampleRate;

    // 티칭된 로케이터의 사이즈
    Ipvm::Rect32s rtROISpec[NUM_OF_LOCATOR];
    Ipvm::Rect32s rtSearchROI[NUM_OF_LOCATOR];
    long nImgNormalX[NUM_OF_LOCATOR], nImgNormalY[NUM_OF_LOCATOR];

    Ipvm::Image8u refResize[NUM_OF_LOCATOR];
    Ipvm::Image8u srcResize[NUM_OF_LOCATOR];

    long nSearchRangeX = 0;
    long nSearchRangeY = 0;

    long nTempX[NUM_OF_LOCATOR];
    long nTempY[NUM_OF_LOCATOR];

    BOOL bResult = TRUE;

    for (long i = 0; i < i_nNumOfLocator; i++)
    {
        rtROISpec[i] = i_prtRefROI[i];
        rtROISpec[i].InflateRect(
            m_VisionPara->sMarkAlgoParam.nMarkRoiMargin, m_VisionPara->sMarkAlgoParam.nMarkRoiMargin);
        rtROISpec[i] &= imageRoi;

        rtSearchROI[i] = i_prtSerachROI[i];
        rtSearchROI[i].InflateRect(
            m_VisionPara->sMarkAlgoParam.nMarkRoiMargin, m_VisionPara->sMarkAlgoParam.nMarkRoiMargin);
        rtSearchROI[i] &= imageRoi;

        nImgNormalX[i] = rtROISpec[i].Width();
        nImgNormalY[i] = rtROISpec[i].Height();

        int nxRefResize = (int)(rtROISpec[i].Width()
            * dImageSampleRateInverse); //맘에 안들어도 소수점 변환은 하지 말도록.. 360과 검사 결과를 일치 시키기 위함임.
        int nyRefResize = (int)(rtROISpec[i].Height() * dImageSampleRateInverse);

        refResize[i].Create(nxRefResize, nyRefResize);
        srcResize[i].Create(nxRefResize, nyRefResize);

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(refResize[i]), 0, refResize[i]);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(srcResize[i]), 0, srcResize[i]);

        Ipvm::Rect32s rtProcessingROI = Ipvm::Rect32s(0, 0, nxRefResize, nyRefResize);
        if (Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(i_refImage, rtROISpec[i]), refResize[i])
            != Ipvm::Status::e_ok)
        {
            bResult = FALSE;
        }

        nTempX[i] = rtSearchROI[i].Width() - nImgNormalX[i];
        nTempY[i] = rtSearchROI[i].Height() - nImgNormalY[i];
    }

    if (i_nNumOfLocator == NUM_OF_LOCATOR)
    {
        nSearchRangeX = (nTempX[0] + nTempX[1]) / NUM_OF_LOCATOR;
        nSearchRangeY = (nTempY[0] + nTempY[1]) / NUM_OF_LOCATOR;
    }
    else
    {
        nSearchRangeX = nTempX[0];
        nSearchRangeY = nTempY[0];
    }

    Ipvm::Point32s2 ptScoreMax = Ipvm::Point32s2(-1, -1);
    double fScoreMax = -100000000.f;
    double fScoreTemp[NUM_OF_LOCATOR];
    double fScoreTotal;

    //{{//kircheis_MarkLocDebug
    std::vector<Ipvm::Point32s2> vecptScoreMax(2);
    std::vector<double> vecfScoreMax(2);
    vecfScoreMax[0] = -100000000.f;
    vecfScoreMax[1] = -100000000.f;
    std::vector<BOOL> vecbIsLocaterExist(2);
    vecbIsLocaterExist[0] = FALSE;
    vecbIsLocaterExist[1] = FALSE;
    //}}

    for (long y = 0; y < nSearchRangeY; y = y + i_nImageSampleRate)
    {
        for (long x = 0; x < nSearchRangeX; x = x + i_nImageSampleRate)
        {
            for (long i = 0; i < i_nNumOfLocator; i++)
            {
                Ipvm::Rect32s rtSrc = Ipvm::Rect32s(int32_t(rtSearchROI[i].m_left + x),
                    int32_t(rtSearchROI[i].m_top + y), int32_t(rtSearchROI[i].m_left + x + nImgNormalX[i]),
                    int32_t(rtSearchROI[i].m_top + y + nImgNormalY[i]));

                if (Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(i_binImage, rtSrc), srcResize[i])
                    != Ipvm::Status::e_ok)
                {
                    bResult = FALSE;
                }

                // source, reference 모두 255인 픽셀수

                if (!GetMatchScore(refResize[i], srcResize[i], fScoreTemp[i]))
                {
                    vecptScoreMax.clear();
                    vecfScoreMax.clear();
                    vecbIsLocaterExist.clear();
                    return FALSE;
                }

                if (vecfScoreMax[i] < fScoreTemp[i]) //kircheis_MarkLoc
                {
                    vecfScoreMax[i] = fScoreTemp[i];
                    vecptScoreMax[i].m_x = x + m_VisionPara->sMarkAlgoParam.nMarkRoiMargin;
                    vecptScoreMax[i].m_y = y + m_VisionPara->sMarkAlgoParam.nMarkRoiMargin;
                    vecbIsLocaterExist[i] = TRUE;
                }
            }

            if (i_nNumOfLocator == NUM_OF_LOCATOR)
            {
                fScoreTotal = (fScoreTemp[0] + fScoreTemp[1]) / 2.0f;
            }
            else
            {
                fScoreTotal = fScoreTemp[0];
            }

            // score match 가 최고점인 x, y 값 저장
            if (fScoreTotal > fScoreMax)
            {
                fScoreMax = fScoreTotal;
                ptScoreMax.m_x = x + m_VisionPara->sMarkAlgoParam.nMarkRoiMargin;
                ptScoreMax.m_y = y + m_VisionPara->sMarkAlgoParam.nMarkRoiMargin;
                isLocaterExist = TRUE;
            }
        }
    }

    bResult &= isLocaterExist;

    ///MaxScore를 찾지 못했다면, Return;
    if (vecbIsLocaterExist[0] == FALSE || vecptScoreMax[0].m_x < 0 || vecptScoreMax[0].m_y < 0
        || (i_nNumOfLocator == NUM_OF_LOCATOR
            && (vecbIsLocaterExist[1] == FALSE || vecptScoreMax[1].m_x < 0
                || vecptScoreMax[1].m_y < 0))) //kircheis_MarkLoc
    {
        return FALSE;
    }

    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        if (i_nNumOfLocator == NUM_OF_LOCATOR)
        {
            o_pptLocater[i] = Ipvm::Point32s2(rtSearchROI[i].m_left + vecptScoreMax[i].m_x,
                rtSearchROI[i].m_top + vecptScoreMax[i].m_y); //kircheis_MarkLoc
        }
        else
        {
            o_pptLocater[i] = Ipvm::Point32s2(rtSearchROI[0].m_left + vecptScoreMax[0].m_x,
                rtSearchROI[0].m_top + vecptScoreMax[0].m_y); //kircheis_MarkLoc
        }
    }

    return bResult;
}

// Locater 위치 계산
BOOL VisionInspectionMark::FindLocater(
    const Ipvm::Image8u& i_refImage, const Ipvm::Image8u& i_binImage, Ipvm::Point32s2* o_pptLocater, BOOL bTeaching)
{
    Ipvm::Point32r2 imageCenter(i_binImage.GetSizeX() * 0.5f, i_binImage.GetSizeY() * 0.5f);
    Ipvm::Rect32s imageRoi(i_binImage);

    // 티칭된 로케이터의 사이즈
    Ipvm::Rect32s rtROISpec[NUM_OF_LOCATOR];
    Ipvm::Rect32s rtSearchROI[NUM_OF_LOCATOR];

    for (long i = 0; i < NUM_OF_LOCATOR; i++)
    {
        rtROISpec[i] = m_VisionTempSpec->m_plTeachCharROI[m_VisionTempSpec->m_pnLocID[i]];
        if (bTeaching == FALSE)
        {
            rtROISpec[i] = rtROISpec[i] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        }
        rtROISpec[i] &= imageRoi;

        rtSearchROI[i] = m_prtLocaterSearchROI[i];
        rtSearchROI[i].InflateRect(
            m_VisionPara->sMarkAlgoParam.nMarkRoiMargin, m_VisionPara->sMarkAlgoParam.nMarkRoiMargin);
        rtSearchROI[i] &= imageRoi;
    }

    long nNumOfLocator = 2;
    if (rtROISpec[0] == rtROISpec[1])
        nNumOfLocator = 1;

    //if(!FindLocater(nNumOfLocator, i_pbyRefImage, i_pbyBinImage, m_VisionPara->m_nImageSampleRate, rtROISpec, rtSearchROI, o_pptLocater))
    if (!FindLocaterReformed(nNumOfLocator, i_refImage, i_binImage, m_VisionPara->m_nImageSampleRate, rtROISpec,
            rtSearchROI, o_pptLocater))
        return FALSE;

    /// Normal size로 한번더...
    if (m_VisionPara->m_nImageSampleRate > 1)
    {
        long nExtendOffset = m_VisionPara->m_nImageSampleRate / 2 + 1;

        for (long i = 0; i < NUM_OF_LOCATOR; i++)
        {
            rtSearchROI[i] = Ipvm::Rect32s(o_pptLocater[i].m_x, o_pptLocater[i].m_y,
                o_pptLocater[i].m_x + rtROISpec[i].Width(), o_pptLocater[i].m_y + rtROISpec[i].Height());
            rtSearchROI[i].InflateRect(nExtendOffset, nExtendOffset);
        }
        //if(!FindLocater(nNumOfLocator, i_pbyRefImage, i_pbyBinImage, 1, rtROISpec, rtSearchROI, o_pptLocater))
        if (!FindLocaterReformed(nNumOfLocator, i_refImage, i_binImage, 1, rtROISpec, rtSearchROI, o_pptLocater))
            return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetMatchScore(
    const Ipvm::Image8u& refImage, const Ipvm::Image8u& sourceImage, double& o_pdScore)
{
    const long imageSizeX = refImage.GetSizeX();
    const long imageSizeY = refImage.GetSizeY();

    if (refImage.GetSizeX() != sourceImage.GetSizeX())
        return FALSE;
    if (refImage.GetSizeY() != sourceImage.GetSizeY())
        return FALSE;

    Ipvm::Image8u tempImage;
    if (!getReusableMemory().GetByteImage(tempImage, imageSizeX, imageSizeY))
    {
        return FALSE;
    }

    BOOL bResult = TRUE;
    double dScoreMatch = 0.0f;
    double dScoreDiff = 0.0f;

    if (Ipvm::ImageProcessing::BitwiseAnd(sourceImage, refImage, Ipvm::Rect32s(sourceImage), tempImage)
        != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::ImageProcessing::GetSum(tempImage, Ipvm::Rect32s(tempImage), dScoreMatch) != Ipvm::Status::e_ok)
        return FALSE;

    dScoreMatch /= 255.0f;

    // source, reference 둘 중 하나만 255인 픽셀수
    if (Ipvm::ImageProcessing::BitwiseXor(sourceImage, refImage, Ipvm::Rect32s(sourceImage), tempImage)
        != Ipvm::Status::e_ok)
        return FALSE;
    if (Ipvm::ImageProcessing::GetSum(tempImage, Ipvm::Rect32s(tempImage), dScoreDiff) != Ipvm::Status::e_ok)
        return FALSE;

    dScoreDiff /= 255.0f;

    // Src, Ref 모두 255인 픽셀수. - Src, Ref가 서로 다른 픽셀수.
    o_pdScore = dScoreMatch - dScoreDiff;

    return bResult;
}

void VisionInspectionMark::Get2PntLocaterLineAngle(const Ipvm::Image8u& i_binImage, Ipvm::Point32s2* pptLocator,
    Ipvm::Rect32s* prtLocatorROI, float& fAngle, BOOL bTeach) //kircheis_MarkLoc
{
    Ipvm::Point32r2 imageCenter(i_binImage.GetSizeX() * 0.5f, i_binImage.GetSizeY() * 0.5f);
    Ipvm::Rect32s imageRoi(i_binImage);

    Ipvm::Point32s2& pt1 = pptLocator[0];
    Ipvm::Point32s2& pt2 = pptLocator[1];

    if (pt1 == pt2)
    {
        fAngle = -100.f;

        std::vector<Ipvm::Point32r2> vecfXY;

        Ipvm::Rect32s rtROI = prtLocatorROI[0];
        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            auto* bin_y = i_binImage.GetMem(0, y);
            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (bin_y[x] == 255)
                {
                    vecfXY.emplace_back((float)x, (float)y);
                }
            }
        }

        if (vecfXY.size() <= 3)
        {
            return;
        }

        Ipvm::LineEq32r line;

        Ipvm::DataFitting::FitToLine(long(vecfXY.size()), &vecfXY[0], line);

        fAngle = (float)(-atan(line.m_a / line.m_b));
        return; //kircheis_MarkLoc
    }
    else if (bTeach) //kircheis_MarkLoc
    {
        long nCharNum = (long)m_result->m_vecrtCharPositionforCalcImage.size();

        for (long i = 0; i < nCharNum; i++)
        {
            Ipvm::Rect32s rtROI(m_result->m_vecrtCharPositionforCalcImage[i]);

            if ((rtROI & imageRoi) != rtROI)
                continue;

            if (m_VisionTempSpec->m_pnLocID[0] == i)
            {
                pt1.m_x = rtROI.m_left;
                pt1.m_y = rtROI.m_top;
            }
            else if (m_VisionTempSpec->m_pnLocID[1] == i)
            {
                pt2.m_x = rtROI.m_left;
                pt2.m_y = rtROI.m_top;
            }
        }
    }
    else
    {
        pt1 = prtLocatorROI[0].TopLeft();
        pt2 = prtLocatorROI[1].TopLeft();
    }

    Ipvm::LineEq32r oLine(Ipvm::Conversion::ToLineEq32r(
        Ipvm::LineSeg32r((float)pt1.m_x, (float)pt1.m_y, (float)pt2.m_x, (float)pt2.m_y)));

    fAngle = (float)(-atan(oLine.m_a / oLine.m_b));
}

BOOL VisionInspectionMark::DoMarkInsp(
    const Ipvm::Image8u& image, Ipvm::BlobInfo* psMarkInfo, const Ipvm::Rect32s& inspROI)
{
    UNREFERENCED_PARAMETER(psMarkInfo);

    Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
    Ipvm::Rect32s imageRoi(image);

    float fMarkAngle = 0.0f;
    Ipvm::Point32r2 ptRefLocaterCenter;
    Ipvm::Point32r2 ptLocaterCenter;

    if (!GetLocaterSearchBox(m_prtLocaterSearchROI, ptRefLocaterCenter))
        return FALSE;

    ///==============Mark Insp ==========================////
    int Left = int(min(m_bodyAlignResult->fptLT.m_x, m_bodyAlignResult->fptLB.m_x) + .5f);
    int Top = int(min(m_bodyAlignResult->fptLT.m_y, m_bodyAlignResult->fptRT.m_y) + .5f);
    int Right = int(max(m_bodyAlignResult->fptRT.m_x, m_bodyAlignResult->fptRB.m_x) + .5f);
    int Bottom = int(max(m_bodyAlignResult->fptLB.m_y, m_bodyAlignResult->fptRB.m_y) + .5f);
    Ipvm::Rect32s rtBody(Left, Top, Right, Bottom);

    int nRefThreshold = m_VisionTempSpec->m_nMarkThreshold;

    if (m_VisionPara->sMarkAlgoParam.nThresholdMode == ENUM_THRESHOLD_AUTOFULL)
    {
        BYTE nAutoThreshold = (BYTE)nRefThreshold;
        BYTE nLowMean = 0;
        BYTE nHighMean = 0;

        Ipvm::ImageProcessing::GetThresholdOtsu(image, inspROI, nLowMean, nHighMean, nAutoThreshold);

        if (nAutoThreshold > nRefThreshold)
        {
            if (abs(nAutoThreshold - nRefThreshold) > MAX_THRESHOLDDIFF_TEACHINSP)
            {
                nAutoThreshold = BYTE(nRefThreshold + MAX_THRESHOLDDIFF_TEACHINSP);
            }
        }
        else
        {
            if (abs(nAutoThreshold - nRefThreshold) > MAX_THRESHOLDDIFF_TEACHINSP)
            {
                nAutoThreshold = BYTE(nRefThreshold - MAX_THRESHOLDDIFF_TEACHINSP);
            }
        }

        nRefThreshold = nAutoThreshold;
    }

    Ipvm::Image8u thresholdImage;
    if (!getReusableMemory().GetInspByteImage(thresholdImage))
        return FALSE;

    if (Ipvm::ImageProcessing::BinarizeGreaterEqual(
            image, rtBody, (BYTE)min(255, max(0, nRefThreshold)), thresholdImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (m_VisionPara->m_bWhiteBackGround)
    {
        if (Ipvm::ImageProcessing::BitwiseNot(rtBody, thresholdImage) != Ipvm::Status::e_ok)
        {
            return FALSE;
        }
    }

    if (m_VisionPara->m_bAlignAreaIgnore)
    {
        GetIgnoreImageByDebugInfo(thresholdImage, inspROI, thresholdImage);
    }

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    if (m_VisionPara->m_nSelectAngleReference == Mark_Angle_Ref_Body_Align)
    {
        /// 1. Locater Searching
        //시퀀스
        //20140918 현건<<===============
        //Algorithm. SearchLocator안에서 Blob 후보군을 찾아내고 Specimage랑 비벼서 가장 점수 높은 Blob의 LT점을 찾는다
        Ipvm::Rect32s rtSearchLoctor;
        Ipvm::Rect32s rtROISpec;
        int32_t nScanBlobNum = 100;
        double fScore = 0;
        double fMaxScore = -(imageSizeX * imageSizeY);
        long nMaxID;

        Ipvm::Image8u tempImage;
        Ipvm::Image8u cpyImage;

        if (!getReusableMemory().GetInspByteImage(tempImage))
            return FALSE;
        if (!getReusableMemory().GetInspByteImage(cpyImage))
            return FALSE;

        if (m_VisionTempSpec->m_plTeachCharROI == NULL)
        {
            return FALSE;
        }

        for (int i = 0; i < NUM_OF_LOCATOR; i++)
        {
            nMaxID = 0;
            fScore = 0;
            nScanBlobNum = 100;

            fMaxScore = -(imageSizeX * imageSizeY);

            rtSearchLoctor = m_prtLocaterSearchROI[i];
            Ipvm::ImageProcessing::Copy(thresholdImage, rtSearchLoctor, cpyImage);

            rtSearchLoctor &= imageRoi;

            rtROISpec = m_VisionSpec[0]->m_plTeachCharROI[m_VisionTempSpec->m_pnLocID[i]]
                + Ipvm::Conversion::ToPoint32s2(imageCenter);
            rtROISpec &= imageRoi;

            Ipvm::Image32s imageLabel;
            if (!getReusableMemory().GetInspLongImage(imageLabel))
                return FALSE;

            if (m_pBlob->DoBlob(thresholdImage, rtSearchLoctor, nScanBlobNum, imageLabel, m_psBlobInfo, nScanBlobNum,
                    m_VisionPara->sMarkAlgoParam.nBlobSizeMin)
                != 0)
            {
                return FALSE;
            }

            for (long j = 0; j < nScanBlobNum; j++)
            {
                Ipvm::Rect32s rtSrc = Ipvm::Rect32s(m_psBlobInfo[j].m_roi.m_left, m_psBlobInfo[j].m_roi.m_top,
                    m_psBlobInfo[j].m_roi.m_left + rtROISpec.Width(), m_psBlobInfo[j].m_roi.m_top + rtROISpec.Height());

                Ipvm::Image8u refImage(m_VisionTempSpec->m_specImage, rtROISpec);
                Ipvm::Image8u srcImage(cpyImage, rtSrc);

                if (!GetMatchScore(refImage, srcImage, fScore))
                {
                    return FALSE;
                }

                if (fScore > fMaxScore)
                {
                    fMaxScore = fScore;
                    nMaxID = j;
                }
            }
            m_pptLocater[i] = Ipvm::Point32s2(m_psBlobInfo[nMaxID].m_roi.m_left, m_psBlobInfo[nMaxID].m_roi.m_top);
        }

        Ipvm::Rect32s rtLocatorROI[NUM_OF_LOCATOR];
        for (int i = 0; i < NUM_OF_LOCATOR; i++)
        {
            int nRefID = m_VisionTempSpec->m_pnLocID[i];
            int nROIWidth = abs(
                m_VisionTempSpec->m_plTeachCharROI[nRefID].m_right - m_VisionTempSpec->m_plTeachCharROI[nRefID].m_left);
            int nROIHeight = abs(
                m_VisionTempSpec->m_plTeachCharROI[nRefID].m_bottom - m_VisionTempSpec->m_plTeachCharROI[nRefID].m_top);

            rtLocatorROI[i] = Ipvm::Rect32s(m_pptLocater[i].m_x, m_pptLocater[i].m_y, m_pptLocater[i].m_x + nROIWidth,
                m_pptLocater[i].m_y + nROIHeight);
        }

        ptLocaterCenter.m_x = (m_pptLocater[0].m_x + m_pptLocater[1].m_x) * 0.5f;
        ptLocaterCenter.m_y = (m_pptLocater[0].m_y + m_pptLocater[1].m_y) * 0.5f;

        fMarkAngle = 0.0f;
        if (!LoopCharInsp(image, thresholdImage, ptLocaterCenter, nRefThreshold, fMarkAngle))
        {
            return FALSE;
        }

        long nCharNum = (long)m_result->m_vecrtCharPositionforCalcImage.size();

        for (long i = 0; i < nCharNum; i++)
        {
            Ipvm::Rect32s rtROI(m_result->m_vecrtCharPositionforCalcImage[i]);

            if ((rtROI & imageRoi) != rtROI)
            {
                continue;
            }

            if (m_VisionTempSpec->m_pnLocID[0] == i)
            {
                m_pptLocater[0].m_x = rtROI.m_left;
                m_pptLocater[0].m_y = rtROI.m_top;
            }
            else if (m_VisionTempSpec->m_pnLocID[1] == i)
            {
                m_pptLocater[1].m_x = rtROI.m_left;
                m_pptLocater[1].m_y = rtROI.m_top;
            }
        }

        ptLocaterCenter.m_x = (m_pptLocater[0].m_x + m_pptLocater[1].m_x) * 0.5f;
        ptLocaterCenter.m_y = (m_pptLocater[0].m_y + m_pptLocater[1].m_y) * 0.5f;
    }
    else if (m_VisionPara->m_nSelectAngleReference == Mark_Angle_Ref_Teaching_Image)
    {
        if (!FindLocater(m_VisionTempSpec->m_specImage, thresholdImage, m_pptLocater, FALSE))
            return FALSE;

        Ipvm::Rect32s rtLocatorROI[NUM_OF_LOCATOR];
        for (int i = 0; i < NUM_OF_LOCATOR; i++)
        {
            long nRefID = m_VisionTempSpec->m_pnLocID[i];
            int nROIWidth = abs(
                m_VisionTempSpec->m_plTeachCharROI[nRefID].m_right - m_VisionTempSpec->m_plTeachCharROI[nRefID].m_left);
            int nROIHeight = abs(
                m_VisionTempSpec->m_plTeachCharROI[nRefID].m_bottom - m_VisionTempSpec->m_plTeachCharROI[nRefID].m_top);

            rtLocatorROI[i] = Ipvm::Rect32s(m_pptLocater[i].m_x, m_pptLocater[i].m_y, m_pptLocater[i].m_x + nROIWidth,
                m_pptLocater[i].m_y + nROIHeight);
        }

        //Locater Center
        ptLocaterCenter.m_x = (m_pptLocater[0].m_x + m_pptLocater[1].m_x) * 0.5f;
        ptLocaterCenter.m_y = (m_pptLocater[0].m_y + m_pptLocater[1].m_y) * 0.5f;

        /// 1)  Char Insp (UnderPrint, OverPrint, Contrast)
        fMarkAngle = m_result->m_sMarkMapInfo_px.sTeachROI_Info.fAngle;

        if (!LoopCharInsp(image, thresholdImage, ptLocaterCenter, nRefThreshold, fMarkAngle))
        {
            return FALSE;
        }

        // 영훈 20160617 : Mark가 많이 돌아갔을 경우 Locator 문자가 Search ROI에 조차 잘 들어오지 않을 때가 있다.
        // 뭐 대충이라도 Locator를 찾고 그 기준으로 모든 문자를 찾은 뒤 Locator ID를 이용하여 찾은 문자들 중에서 Locator 위치를 다시 부여해주자.
        // 이 것으로 Mark Angle의 값이 정상으로 나오는 것을 확인한다.
        /// 3) Locater Angle
        Get2PntLocaterLineAngle(thresholdImage, m_pptLocater, rtLocatorROI, fMarkAngle, FALSE);

        ptLocaterCenter.m_x = (m_pptLocater[0].m_x + m_pptLocater[1].m_x) * 0.5f;
        ptLocaterCenter.m_y = (m_pptLocater[0].m_y + m_pptLocater[1].m_y) * 0.5f;
    }

    if (m_fixedInspectionSpecs[MARK_INSPECTION_MARK_ANGLE].m_use)
    {
        if (!MarkAngleInsp(fMarkAngle, inspROI))
            return FALSE;
    }

    /// 2) Blob Size
    if (m_fixedInspectionSpecs[MARK_INSPECTION_BLOB_SIZE].m_use)
    {
        if (!MarkBlobSizebyUnderOverImage())
        {
            return FALSE;
        }
    }

    /// 4) Mark Position
    if (m_fixedInspectionSpecs[MARK_INSPECTION_MARK_POS].m_use)
    {
        Ipvm::Point32r2 ptDiffLocPos = Ipvm::Point32r2(ptLocaterCenter - ptRefLocaterCenter);
        if (!MarkPositionInsp(ptDiffLocPos, inspROI))
            return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::DoMarkCountInsp(
    const Ipvm::Image8u& i_imgInsp, Ipvm::BlobInfo* i_psMarkInfo, const Ipvm::Rect32s& i_rtTeachROI)
{
    if (i_imgInsp.GetMem() == nullptr || i_rtTeachROI.IsRectEmpty() || i_psMarkInfo == NULL)
        return FALSE;

    m_vecrtDebugROI.clear();

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_MARK_COUNT]);
    if (pResult == nullptr)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr)
        return FALSE;

    pResult->Clear();
    pResult->Resize(1);

    Ipvm::Image8u MarkBlobImage;
    if (!getReusableMemory().GetInspByteImage(MarkBlobImage))
        return FALSE;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(MarkBlobImage), 0, MarkBlobImage);

    if (!GetMarkInspImage(i_imgInsp, i_rtTeachROI, MarkBlobImage))
        return FALSE;

    std::vector<Ipvm::BlobInfo> vecCurBlobInfo;
    Ipvm::Image32s ImageLabel;
    if (!GetMarkBlobInfo(MarkBlobImage, i_rtTeachROI, m_psBlobInfo, ImageLabel, vecCurBlobInfo))
    {
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("0"), _T(""), _T(""), 0.f, *pSpec, 0.f, 0.f, 0.f, REJECT); //kircheis_VSV
        pResult->SetRect(0, i_rtTeachROI); //Search 영역을 준다.
        pResult->m_totalResult = REJECT;

        pResult->UpdateTypicalValue(pSpec);
        pResult->SetTotalResult();

        return TRUE;
    }

    //FIlter. MinBlob Size랑 Max BlobSize로
    std::vector<Ipvm::BlobInfo> vecFinalBlobInfo;
    std::vector<long> vecFinalBlobInfo_Area;
    std::vector<float> vecFinalBlobInfo_Contrast;
    long nMinBlobSize = m_VisionPara->sMarkAlgoParam.nBlobSizeMin;
    long nMaxBlobSize = m_VisionPara->sMarkAlgoParam.nBlobSizeMax;
    float fMinMarkContrastForMarkCnt = (float)m_VisionPara->sMarkAlgoParam.nMinContrastForMarkCnt;
    float fContrast;

    for (const auto& BlobInfo : vecCurBlobInfo)
    {
        //여기에 Blob의 Contrast를 검증하는 Code를 넣으면 좋을거 같다. 각 Mark의 Contrast 값도 DebugInfo에다 넣어서 검사 파라미터로 제어하는게 좋을 듯
        fContrast = GetMarkContrastForMarkCount(i_imgInsp, MarkBlobImage, ImageLabel, BlobInfo.m_roi, BlobInfo.m_label);

        //추출한 Blob의 Area가 최소 Blob의 크기보다는 커야 하며, 최대 Blob의 크기보다는 작아야 인정한다
        if (BlobInfo.m_area > nMinBlobSize && BlobInfo.m_area < nMaxBlobSize && fContrast >= fMinMarkContrastForMarkCnt)
        {
            vecFinalBlobInfo.push_back(BlobInfo);
            vecFinalBlobInfo_Area.push_back(BlobInfo.m_area);
            vecFinalBlobInfo_Contrast.push_back(fContrast);
        }
    }

    SetDebugInfoItem(true, _T("Simple Mark Blob Size(Pxl)"), vecFinalBlobInfo_Area, true);
    SetDebugInfoItem(true, _T("Simple Mark Contrast(GV)"), vecFinalBlobInfo_Contrast, true);

    long nFinalBlobInfoNum = (long)vecFinalBlobInfo.size();
    if (nFinalBlobInfoNum > 0)
    {
        Ipvm::Rect32s rtResultROI;
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("0"), _T(""), _T(""), float(nFinalBlobInfoNum), *pSpec, 0.f, 0.f);
        long nResult = pResult->IsValueCheck(pSpec, float(nFinalBlobInfoNum));

        switch (nResult)
        {
            case PASS:
                for (const auto& BlobROI : vecFinalBlobInfo)
                    rtResultROI.UnionRect(rtResultROI, BlobROI.m_roi);
                pResult->SetRect(0, rtResultROI);
                break;
            case REJECT:
                pResult->SetRect(0, i_rtTeachROI);
                break;
            default:
                break;
        }
    }
    else //Filter후 Blob의 갯수가 없다면
    {
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            0, _T("0"), _T(""), _T(""), 0.f, *pSpec, 0.f, 0.f, 0.f, REJECT); //kircheis_VSV
        pResult->SetRect(0, i_rtTeachROI); //Search 영역을 준다.
        pResult->m_totalResult = REJECT;
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    for (const auto& FinalBlobInfo : vecFinalBlobInfo)
        m_vecrtDebugROI.push_back(FinalBlobInfo.m_roi);

    //{{5. Blob의 정보들로 Surface Mask 생성.. (Ignore 할꺼임)
    //PASS일때만 만들면 되지
    if (pResult->m_totalResult == PASS)
    {
        m_SimpleAlgorithmImage.Create(getImageLotInsp().GetImageSizeX(), getImageLotInsp().GetImageSizeY());
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_SimpleAlgorithmImage), 0, m_SimpleAlgorithmImage);

        for (int nCharIdx = 0; nCharIdx < nFinalBlobInfoNum; nCharIdx++)
        {
            Ipvm::Rect32s charectorROI = vecFinalBlobInfo[nCharIdx].m_roi;

            ///// Label이 일치하는 것만 Spec image에 복사
            long nSelectedLabel = vecFinalBlobInfo[nCharIdx].m_label;
            for (long y = charectorROI.m_top; y < charectorROI.m_bottom; y++)
            {
                auto* label_y = ImageLabel.GetMem(0, y);
                auto* src_y = m_SimpleAlgorithmImage.GetMem(0, y);

                for (long x = charectorROI.m_left; x < charectorROI.m_right; x++)
                {
                    if (nSelectedLabel == label_y[x])
                        src_y[x] = 255;
                }
            }
        }
    }

    return TRUE;
}

float VisionInspectionMark::GetMarkContrastForMarkCount(const Ipvm::Image8u& i_imageGray,
    const Ipvm::Image8u& i_imageThreshold, const Ipvm::Image32s& i_ImageLabel, Ipvm::Rect32s i_rtROI,
    long i_nTargetLabel)
{
    float fContrast = 0.f;

    long lSumFg, lNumFg;
    long lSumBg, lNumBg;

    i_rtROI.InflateRect(2, 2, 2, 2);
    i_rtROI &= Ipvm::Rect32s(i_imageGray);

    lSumFg = lNumFg = lSumBg = lNumBg = 0;

    for (long y = i_rtROI.m_top; y < i_rtROI.m_bottom; y++)
    {
        auto* image_y = i_imageGray.GetMem(0, y);
        auto* threshold_y = i_imageThreshold.GetMem(0, y);
        auto* label_Y = i_ImageLabel.GetMem(0, y);

        for (long x = i_rtROI.m_left; x < i_rtROI.m_right; x++)
        {
            BYTE score = image_y[x];

            if (threshold_y[x] > 0 && label_Y[x] == i_nTargetLabel)
            {
                lSumFg += score;
                lNumFg++;
            }
            else if (threshold_y[x] <= 0)
            {
                lSumBg += score;
                lNumBg++;
            }
        }
    }

    //{{ 김일남 2003.12.10
    if (lNumFg <= 0 || lNumBg <= 0)
    {
        fContrast = 255.f;
    }
    else
    {
        float fAvgFg = (float)lSumFg / lNumFg;
        float fAvgBg = (float)lSumBg / lNumBg;
        //if(fAvgFg==0. && fAvgBg==0.) return FALSE;
        //nContrast=(int)((fAvgFg-fAvgBg)/(fAvgFg+fAvgBg)*100);
        fContrast = (float)(max(fAvgFg, fAvgBg) - min(fAvgFg, fAvgBg));
    }
    //}}

    return fContrast;
}

BOOL VisionInspectionMark::GetMarkInspImage(
    const Ipvm::Image8u& i_imgInsp, const Ipvm::Rect32s& i_rtTeachROI, Ipvm::Image8u& o_ThresholdImage)
{
    if (i_imgInsp.GetMem() == nullptr || i_rtTeachROI.IsRectEmpty())
        return FALSE;

    //{{ROI 영역내 이진화

    Ipvm::Image8u CalcThresholdValueImage;
    if (!getReusableMemory().GetInspByteImage(CalcThresholdValueImage))
        return FALSE;

    if (m_VisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL)
    {
        BYTE nThresholdValue = (BYTE)m_VisionTempSpec->m_nMarkThreshold;
        BYTE nLowMean = 0;
        BYTE nHighMean = 0;

        Ipvm::ImageProcessing::Copy(i_imgInsp, Ipvm::Rect32s(i_imgInsp), CalcThresholdValueImage);

        std::vector<BYTE> vecbyIgnoreZero(0);
        long nImageSIzeX = CalcThresholdValueImage.GetSizeX();
        long nIndex(0), nIndexY(0);
        BYTE* pbyCalcImage = CalcThresholdValueImage.GetMem();
        for (long ny = i_rtTeachROI.m_top; ny < i_rtTeachROI.m_bottom; ny++)
        {
            nIndexY = nImageSIzeX * ny;
            for (long nx = i_rtTeachROI.m_left; nx < i_rtTeachROI.m_right; nx++)
            {
                nIndex = nIndexY + nx;
                if (pbyCalcImage[nIndex] > 0)
                    vecbyIgnoreZero.push_back(pbyCalcImage[nIndex]);
            }
        }
        int nBufSize = (long)vecbyIgnoreZero.size();
        if (nBufSize <= 0)
            return FALSE;

        Ipvm::Image8u OtsuImageBuf(nBufSize, 1, &vecbyIgnoreZero[0], nBufSize);

        Ipvm::ImageProcessing::GetThresholdOtsu(
            OtsuImageBuf, Ipvm::Rect32s(OtsuImageBuf), nLowMean, nHighMean, nThresholdValue);
        m_VisionTempSpec->m_nMarkThreshold = nThresholdValue;
    }

    BYTE thresholdValue = (BYTE)min(255, max(0, m_VisionTempSpec->m_nMarkThreshold));

    if (Ipvm::ImageProcessing::BinarizeGreaterEqual(i_imgInsp, i_rtTeachROI, thresholdValue, o_ThresholdImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (m_VisionPara->m_bWhiteBackGround)
    {
        Ipvm::ImageProcessing::BitwiseNot(i_rtTeachROI, o_ThresholdImage);
        if (m_VisionPara->m_bAlignAreaIgnore)
        {
            Ipvm::ImageProcessing::Copy(o_ThresholdImage, Ipvm::Rect32s(o_ThresholdImage), CalcThresholdValueImage);
            GetIgnoreImageByDebugInfo(CalcThresholdValueImage, i_rtTeachROI, o_ThresholdImage);
        }
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetIgnoreImageByDebugInfo(
    const Ipvm::Image8u& i_imgOrigin, Ipvm::Rect32s i_rtTeachROI, Ipvm::Image8u& o_imgIgnore)
{
    if (i_imgOrigin.GetMem() == nullptr || o_imgIgnore.GetMem() == nullptr)
        return FALSE;

    Ipvm::Image8u MaskImage;
    Ipvm::Image8u DilateImage;
    if (!getReusableMemory().GetInspByteImage(MaskImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(DilateImage))
        return FALSE;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(MaskImage), 0, MaskImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(DilateImage), 0, DilateImage);

    long nDataNum(0);

    void* pPadIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_PAD_ALIGN_2D, _T("Align PAD Image"), nDataNum);
    if (pPadIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pPadIgnoreimg;
        pPadIgnoreimg = (Ipvm::Image8u*)pPadIgnoreData;
        IgnoreImageDilate(*pPadIgnoreimg, i_rtTeachROI, m_VisionPara->m_nIgnoreDilateCount, DilateImage);

        Ipvm::ImageProcessing::Add(DilateImage, i_rtTeachROI, 0, MaskImage);
    }

    void* pBallIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_BGA_BALL_2D, _T("Ball Mask Image"), nDataNum);
    if (pBallIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pBallIgnoreimg;
        pBallIgnoreimg = (Ipvm::Image8u*)pBallIgnoreData;
        IgnoreImageDilate(*pBallIgnoreimg, i_rtTeachROI, m_VisionPara->m_nIgnoreDilateCount, DilateImage);

        Ipvm::ImageProcessing::Add(DilateImage, i_rtTeachROI, 0, MaskImage);
    }

    void* pLandIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_LGA_LAND_2D, _T("Land Origin Mask Image"), nDataNum);
    if (pLandIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pLandIgnoreimg;
        pLandIgnoreimg = (Ipvm::Image8u*)pLandIgnoreData;
        IgnoreImageDilate(*pLandIgnoreimg, i_rtTeachROI, m_VisionPara->m_nIgnoreDilateCount, DilateImage);

        Ipvm::ImageProcessing::Add(DilateImage, i_rtTeachROI, 0, MaskImage);
    }

    void* pCompIgnoreData
        = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_PASSIVE_2D, _T("Passive Mask Image with Pad"), nDataNum);
    if (pCompIgnoreData != nullptr && nDataNum > 0)
    {
        Ipvm::Image8u* pCompIgnoreimg;
        pCompIgnoreimg = (Ipvm::Image8u*)pCompIgnoreData;
        IgnoreImageDilate(*pCompIgnoreimg, i_rtTeachROI, m_VisionPara->m_nIgnoreDilateCount, DilateImage);

        Ipvm::ImageProcessing::Add(DilateImage, i_rtTeachROI, 0, MaskImage);
    }

    Ipvm::ImageProcessing::Subtract(i_imgOrigin, MaskImage, i_rtTeachROI, 0, o_imgIgnore);

    return TRUE;
}

BOOL VisionInspectionMark::IgnoreImageDilate(
    const Ipvm::Image8u i_Image, Ipvm::Rect32s i_rtTeachROI, long i_nDilateCount, Ipvm::Image8u& o_DilateImage)
{
    if (i_Image.GetMem() == nullptr)
        return FALSE;

    Ipvm::ImageProcessing::MorphDilate(i_Image, i_rtTeachROI, i_nDilateCount, i_nDilateCount, true, o_DilateImage);

    return TRUE;
}

BOOL VisionInspectionMark::GetMarkBlobInfo(const Ipvm::Image8u& i_MarkBlobImage, const Ipvm::Rect32s& i_rtTeachROI,
    Ipvm::BlobInfo* i_psBlobInfo, Ipvm::Image32s& o_imageLabel, std::vector<Ipvm::BlobInfo>& o_vecCurBlobInfo)
{
    if (i_MarkBlobImage.GetMem() == nullptr || i_rtTeachROI.IsRectEmpty())
        return FALSE;

    int32_t nCharBlobNum(0);
    getReusableMemory().GetInspLongImage(o_imageLabel);
    /// Blob
    if (!GetCharBlob(i_MarkBlobImage, i_rtTeachROI, &m_VisionPara->sMarkAlgoParam,
            (m_VisionPara->m_teach_merge_num == -1), m_VisionPara->m_teach_merge_infos,
            m_VisionPara->m_vecrtUserNumIgnore, m_VisionPara->m_vecrtUserIgnore, i_psBlobInfo, m_blobAttributes,
            o_imageLabel,
            nCharBlobNum)) //kircheis_AutoTeach
        return FALSE;

    if (!DoCharNumberCheckTeach(o_imageLabel, i_psBlobInfo, nCharBlobNum, m_VisionPara->sMarkAlgoParam.fMarkMergeGap))
        return FALSE;

    //{{Blob한 Image 확인 용도_Test용도임
    Ipvm::Image8u imgBlob;
    if (!getReusableMemory().GetInspByteImage(imgBlob))
        return FALSE;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imgBlob), 0, imgBlob);
    //}}

    //현재 추출한 Blob Info 저장
    if (o_vecCurBlobInfo.size() > 0)
        o_vecCurBlobInfo.clear();

    for (long nIndex = 0; nIndex < nCharBlobNum; nIndex++)
    {
        o_vecCurBlobInfo.push_back(m_psBlobInfo[nIndex]);
        Ipvm::ImageProcessing::Copy(i_MarkBlobImage, m_psBlobInfo[nIndex].m_roi, imgBlob);
    }

    //}}

    return TRUE;
}

BOOL VisionInspectionMark::MarkPositionInsp(Ipvm::Point32r2 i_ptDiffLocatorPos, const Ipvm::Rect32s& inspROI)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_MARK_POS]);
    if (pResult == nullptr)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr)
        return FALSE;

    pResult->Clear();
    pResult->Resize(1);

    float dx = i_ptDiffLocatorPos.m_x;
    float dy = i_ptDiffLocatorPos.m_y;

    float fDist = CAST_FLOAT(sqrt(float(dx * dx + dy * dy)) * getScale().pixelToUmXY());
    pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(0, _T("0"), _T(""), _T(""), fDist, *pSpec, 0.f, 0.f);
    pResult->SetTotalResult();

    Ipvm::Rect32s rtTotalROI;
    if (GetTotalROIFromSmallROIs(m_result->m_vecrtCharPositionforImage, rtTotalROI))
    {
        pResult->SetRect(0, rtTotalROI);
    }
    else
    {
        pResult->SetRect(0, inspROI);
    }

    return TRUE;
}

BOOL VisionInspectionMark::GetCharSearchROI(long i_nCharNum, Ipvm::Point32r2 i_ptLocCenter,
    const std::vector<Ipvm::Point32r2>& i_ptCharPosFromLC, std::vector<Ipvm::Rect32s>& o_vecroiCharROI)
{
    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    long nSearchOffsetX = m_VisionPara->m_nCharSearchOffsetX;
    long nSearchOffsetY = m_VisionPara->m_nCharSearchOffsetY;

    for (int i = 0; i < i_nCharNum; i++)
    {
        int HW = (int)m_VisionTempSpec->m_plTeachCharROI[i].Width();
        int HH = (int)m_VisionTempSpec->m_plTeachCharROI[i].Height();

        float dx = i_ptCharPosFromLC[i].m_x;
        float dy = i_ptCharPosFromLC[i].m_y;

        o_vecroiCharROI[i] = Ipvm::Rect32s(int32_t(i_ptLocCenter.m_x - dx - nSearchOffsetX),
            int32_t(i_ptLocCenter.m_y - dy - nSearchOffsetY), int32_t(i_ptLocCenter.m_x - dx + HW + nSearchOffsetX),
            int32_t(i_ptLocCenter.m_y - dy + HH + nSearchOffsetY));

        if (!CheckROI(o_vecroiCharROI[i], imageSizeX, imageSizeY))
            return FALSE;
    }

    return TRUE;
}

BOOL VisionInspectionMark::LoopCharInsp(const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& i_theshImage,
    Ipvm::Point32r2 i_ptLocatorCenter, int nRefThreshold, float& fMarkAngle)
{
    //{{ underprint, overprint pass 할 때까지 threshold를 바꾸면서 다시 시도한다.
    int nThresholdDeviationDir = 0;
    int nReinspectionNumberUnderCount = 0;
    int nReinspectionNumberOverCount = 0;

    long nImgThresh = nRefThreshold; /// nRefThreshold는 기준값이다.

    BOOL bFlip = FALSE;
    Ipvm::Image8u rotateSrcImg = i_grayImage;

    // 영훈 20160621 : 일단은 항상 Spec ROI 내에서 찾도록 한다.
    if (!GetCharSearchROI(m_VisionTempSpec->m_nCharNum, i_ptLocatorCenter, m_VisionTempSpec->m_teachCharOffset,
            m_result->m_vecroiCharSearch))
    {
        return FALSE;
    }

    if (m_VisionPara->m_nSelectAngleReference == Mark_Angle_Ref_Body_Align)
    {
        //{{ //kircheis_MarkAngle
        long nMarkNum = m_VisionTempSpec->m_nCharNum;

        Ipvm::Rect32s rtAllChar = m_result->m_vecroiCharSearch[0];
        Ipvm::Rect32s rtEach;
        for (long nChar = 0; nChar < nMarkNum; nChar++)
        {
            rtEach = m_result->m_vecroiCharSearch[nChar];
            rtEach.InflateRect(12, 12);
            rtAllChar |= rtEach;
            Ipvm::ImageProcessing::BinarizeGreaterEqual(
                rotateSrcImg, rtEach, (BYTE)min(255, max(0, m_VisionTempSpec->m_nMarkThreshold)), i_theshImage);

            if (m_VisionPara->m_bWhiteBackGround)
            {
                Ipvm::ImageProcessing::BitwiseNot(rtEach, i_theshImage);
            }
        }
        //}}
        int32_t i_nCharBlobNum = 0;

        Ipvm::Image32s imageLabel;
        if (!getReusableMemory().GetInspLongImage(imageLabel))
            return FALSE;

        for (long i = 0; i < ANGLETYPE_ALIGN_MARKANGLE; i++)
        {
            // Blob을 계산해서 갯수가 같으면 Blob으로 Search 영역을 만들고 그렇지 않으며 Spec을 이용하도록 수정
            if (!GetCharBlob(i_theshImage, rtAllChar, &m_VisionPara->sMarkAlgoParam,
                    (m_VisionPara->m_teach_merge_num == -1), m_VisionPara->m_teach_merge_infos,
                    m_VisionPara->m_vecrtUserNumIgnore, m_VisionPara->m_vecrtUserIgnore, m_psBlobInfo, m_blobAttributes,
                    imageLabel, i_nCharBlobNum))
            {
                i_nCharBlobNum = 0;
            }

            if (!DoCharNumberCheckTeach(
                    imageLabel, m_psBlobInfo, i_nCharBlobNum, m_VisionPara->sMarkAlgoParam.fMarkMergeGap))
            {
                i_nCharBlobNum = 0;
            }

            short nLineNumber = 0;
            if (!GetMarkLineForTeach(
                    &m_VisionPara->sMarkAlgoParam, m_psBlobInfo, m_blobAttributes, i_nCharBlobNum, nLineNumber))
            {
                return FALSE;
            }

            if (i == 0)
            {
                if (!CheckMarkAngle(nLineNumber, fMarkAngle))
                    return FALSE;

                auto centerPoint = m_bodyAlignResult->m_center;

                Ipvm::Image8u rotateImageMain;
                if (!getReusableMemory().GetInspByteImage(rotateImageMain))
                {
                    return FALSE;
                }

                if (Ipvm::ImageProcessing::RotateLinearInterpolation(i_grayImage, Ipvm::Rect32s(i_grayImage),
                        centerPoint, -fMarkAngle, Ipvm::Point32r2(0.f, 0.f), rotateImageMain)
                    != Ipvm::Status::e_ok)
                {
                    return FALSE;
                }

                rotateSrcImg = rotateImageMain;

                ///==============Mark Insp ==========================////
                Ipvm::Rect32s rtBody
                    = Ipvm::Rect32s(min((int32_t)m_bodyAlignResult->fptLT.m_x, (int32_t)m_bodyAlignResult->fptLB.m_x),
                        min((int32_t)m_bodyAlignResult->fptLT.m_y, (int32_t)m_bodyAlignResult->fptRT.m_y),
                        max((int32_t)m_bodyAlignResult->fptRT.m_x, (int32_t)m_bodyAlignResult->fptRB.m_x),
                        max((int32_t)m_bodyAlignResult->fptLB.m_y, (int32_t)m_bodyAlignResult->fptRB.m_y));

                if (Ipvm::ImageProcessing::BinarizeGreaterEqual(
                        rotateSrcImg, rtBody, (BYTE)min(255, max(0, nRefThreshold)), i_theshImage)
                    != Ipvm::Status::e_ok)
                {
                    return FALSE;
                }

                if (m_VisionPara->m_bWhiteBackGround)
                {
                    if (Ipvm::ImageProcessing::BitwiseNot(rtBody, i_theshImage) != Ipvm::Status::e_ok)
                    {
                        return FALSE;
                    }
                }
            }
        }
    }

    do
    {
        // 4.2. Char Insp.
        // Char Inspection

        /// 이태용테스트
        if (!CharInsp(rotateSrcImg, fMarkAngle, i_theshImage, m_VisionPara->sMarkAlgoParam.bUseEachCharReinsp,
                (BYTE)nImgThresh))
            return FALSE;

        float fUnderPrintTotal = 0.f;
        float fOverPrintTotal = 0.f;

        for (int j = 0; j < m_VisionTempSpec->m_nCharNum; j++)
        {
            fUnderPrintTotal += m_result->m_vecfFirstTrialUnderPrint[j];
            fOverPrintTotal += m_result->m_vecfFirstTrialOverPrint[j];
        }

        float fUnderPrintAve = fUnderPrintTotal / m_VisionTempSpec->m_nCharNum;
        float fOverPrintAve = fOverPrintTotal / m_VisionTempSpec->m_nCharNum;

        // 아래 검사 항목이 모두 패스이면 중단한다.
        BOOL bPass = TRUE;
        BOOL bUse = TRUE;
        long nResult = NOT_MEASURED;

        if (!ResultUpdate(g_szMarkInspectionName[MARK_INSPECTION_MATCH_RATE], nResult, bUse))
            return FALSE;
        if (bUse == TRUE && nResult != PASS)
            bPass = FALSE;
        if (!ResultUpdate(g_szMarkInspectionName[MARK_INSPECTION_OVER_PRINT], nResult, bUse))
            return FALSE;
        if (bUse == TRUE && nResult != PASS)
            bPass = FALSE;
        if (!ResultUpdate(g_szMarkInspectionName[MARK_INSPECTION_UNDER_PRINT], nResult, bUse))
            return FALSE;
        if (bUse == TRUE && nResult != PASS)
            bPass = FALSE;

        if (bPass || bFlip)
        {
            break;
        }
        else
        {
            if (fabs(100 - fUnderPrintAve) > fabs(fOverPrintAve - 100))
            {
                // underprint 쪽 threshold 조절 상태에서 overprint 쪽 threshold 조절 판정이 나면 break
                if (nThresholdDeviationDir == 1)
                {
                    nImgThresh -= m_VisionPara->sMarkAlgoParam.nAllCharThresholdDeviation;
                    bFlip = TRUE;
                }
                else
                {
                    // underprint_threshold 수정 모드
                    if (nThresholdDeviationDir == 0)
                        nThresholdDeviationDir = -1;
                    // underprint_Threshold 조절 횟수 증가
                    nReinspectionNumberUnderCount++;
                    // underprint_Threshold 조절
                    nImgThresh -= m_VisionPara->sMarkAlgoParam.nAllCharThresholdDeviation;
                    // underprint의 Threshold 조절 횟수 증가
                    if (nReinspectionNumberUnderCount > m_VisionPara->sMarkAlgoParam.nAllCharReinspNumber)
                        break;
                }
            }
            else if (fabs(100 - fUnderPrintAve) < fabs(fOverPrintAve - 100))
            {
                // overprint 쪽 threshold 조절 상태에서 underprint 쪽 threshold 조절 판정이 나면 break
                if (nThresholdDeviationDir == -1)
                {
                    nImgThresh += m_VisionPara->sMarkAlgoParam.nAllCharThresholdDeviation;
                    bFlip = TRUE;
                }
                else
                {
                    // overprint_threshold 수정 모드
                    if (nThresholdDeviationDir == 0)
                        nThresholdDeviationDir = 1;
                    // overprint_Threshold 조절 횟수 증가
                    nReinspectionNumberOverCount++;
                    // overprint_Threshold 조절
                    nImgThresh += m_VisionPara->sMarkAlgoParam.nAllCharThresholdDeviation;
                    // overprint의 Threshold 조절 횟수 증가
                    if (nReinspectionNumberOverCount > m_VisionPara->sMarkAlgoParam.nAllCharReinspNumber)
                        break;
                }
            }
            else
            {
                break;
            }
        }
    }
    while (m_VisionPara->sMarkAlgoParam.bUseAllCharReinsp); //kircheis1111

    return TRUE;
}

BOOL VisionInspectionMark::ResultUpdate(CString strInspName, long& nTotalResult, BOOL& bUse)
{
    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strInspName);
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);

    if (pResult == NULL || pSpec == NULL)
        return FALSE;

    pResult->SetTotalResult();

    nTotalResult = pResult->m_totalResult;
    bUse = pSpec->m_use;

    return TRUE;
}

BOOL VisionInspectionMark::CharInsp(const Ipvm::Image8u& i_grayImage, const float fMarkAngle,
    Ipvm::Image8u& i_theshImage, BOOL bReinsp, BYTE byInitThreshold)
{
    if (m_result->m_vecroiCharSearch.size() <= 0)
        return FALSE;

    static const BOOL bUseMarkMatchRateAlgo2
        = SystemConfig::GetInstance().m_bUseMarkMatchRateAlgoritm2; //kircheis_MatchRate 수정

    long nCharNum = m_VisionTempSpec->m_nCharNum;
    Ipvm::Rect32s* prtCharSearchROI = &m_result->m_vecroiCharSearch[0];

    VisionInspectionResult* pResult = NULL;
    VisionInspectionSpec* pSpec = NULL;
    pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_UNDER_PRINT]);
    if (pResult == nullptr)
        return FALSE;
    pResult->Clear();
    pResult->Resize(nCharNum);
    pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_OVER_PRINT]);
    if (pResult == nullptr)
        return FALSE;
    pResult->Clear();
    pResult->Resize(nCharNum);
    pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_MATCH_RATE]);
    if (pResult == nullptr)
        return FALSE;
    pResult->Clear();
    pResult->Resize(nCharNum);
    pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_CONTRAST]);
    if (pResult == nullptr)
        return FALSE;
    pResult->Clear();
    pResult->Resize(nCharNum);

    int nOverPrintNum(0), nUnderPrintNum(0), nMatchingRate(0);
    int nThreshJump(0);
    int nThreshJumpOver = m_VisionPara->sMarkAlgoParam.nEachCharThresholdDeviation;
    int nThreshJumpUnder = m_VisionPara->sMarkAlgoParam.nEachCharThresholdDeviation;

    long nImgThresh = byInitThreshold;

    Ipvm::Rect32s imageRoi = Ipvm::Rect32s(i_grayImage);

    // 영훈 20151116 : 글자별 리테스트 시 pbyRefImage, pbyBinImage, pbyUnderPrint, pbyOverPrint 이 변수들이 초기화 되지 않고 계속 덮어쓴다. 문제가 된다.
    // 여기서 초기화 해주도록 한다.
    Ipvm::Image8u refImage = *m_oriViewer;
    Ipvm::Image8u binImage = *m_binViewer;
    Ipvm::Image8u underPrint = *m_overViewer;
    Ipvm::Image8u overPrint = *m_underViewer;

    Ipvm::ImageProcessing::Fill(imageRoi, 0, refImage);
    Ipvm::ImageProcessing::Fill(imageRoi, 0, binImage);
    Ipvm::ImageProcessing::Fill(imageRoi, 0, underPrint);
    Ipvm::ImageProcessing::Fill(imageRoi, 0, overPrint);

    // 영훈 : 이제 검사를 실패하면 Repeat시 Search ROI도 움직여보도록 하자. 실제 Mark char간의 Pitch가 차이나서 실패할 경우가 많다.
    // 방향은 X만 하도록 한다.
    long nRepeat = m_VisionPara->sMarkAlgoParam.nEachCharReinspNumber;
    std::vector<long> vecnOffsetValue;
    vecnOffsetValue.resize(3);
    vecnOffsetValue[0] = 0;
    vecnOffsetValue[1] = -10;
    vecnOffsetValue[2] = 10;

    m_vecrtBlobROI.resize(nCharNum);

    long nDir = 0;
    long nOldDir = 0;
    long nTime = 0;
    BOOL bPass = FALSE;
    BOOL bFlip = FALSE;

    long nOverPrint = 0;
    long nUnderPrint = 0;

    CString sObjectID = NULL;

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    // non-rotation
    for (int i = 0; i < nCharNum; i++)
    {
        Ipvm::Rect32s rtProcessROI = prtCharSearchROI[i] & getImageLotInsp().GetImageRect();

        nDir = 0;
        nOldDir = 0;
        nTime = 0;
        bPass = FALSE;
        bFlip = FALSE;

        nOverPrint = 0;
        nUnderPrint = 0;

        do
        {
            Ipvm::Rect32s rtClearArea(rtProcessROI);
            rtClearArea.InflateRect(5, 5, 5, 5);
            if (!CheckROI(rtClearArea, imageSizeX, imageSizeY))
            {
                return FALSE;
            }

            // 2005.10.10 이현민
            // 이 코드의 위치가 잘못되어 있어서
            // 글자별 리테스트가 실패하였음. 수정함.
            Ipvm::ImageProcessing::Fill(rtClearArea, 0, i_theshImage);
            Ipvm::ImageProcessing::BinarizeGreaterEqual(
                i_grayImage, rtProcessROI, (BYTE)min(255, max(0, nImgThresh)), i_theshImage);
            if (m_VisionPara->m_bWhiteBackGround)
            {
                Ipvm::ImageProcessing::BitwiseNot(rtProcessROI, i_theshImage);
            }

            if (m_VisionPara->m_bUseNormalSizeInspMode)
            {
                // 영훈 20151116 : 글자별 리테스트 시 pbyRefImage, pbyBinImage, pbyUnderPrint, pbyOverPrint 이 변수들이 초기화 되지 않고 계속 덮어쓴다. 문제가 된다.
                if (!CharMatching_Blob_NormalSize(i_theshImage, rtProcessROI, fMarkAngle, i, nUnderPrintNum,
                        nOverPrintNum, nMatchingRate, refImage, binImage, underPrint, overPrint))
                {
                    return FALSE;
                }
            }
            else
            {
                if (!CharMatching_Blob(i_theshImage, rtProcessROI, fMarkAngle, i, nUnderPrintNum, nOverPrintNum,
                        nMatchingRate, refImage, binImage, underPrint, overPrint))
                {
                    return FALSE;
                }
            }

            long nlSpecArea = m_VisionTempSpec->m_plTeachCharArea[i];

            nOverPrint = (nOverPrintNum) * 100 / nlSpecArea;
            nUnderPrint = (nUnderPrintNum) * 100 / nlSpecArea;

            if (bReinsp)
            {
                if ((nOverPrint < 15 && nUnderPrint < 15 && nOverPrint + nUnderPrint < 20) || bFlip)
                {
                    bPass = TRUE;
                }
                else
                {
                    nOldDir = nDir;
                    if (nOverPrint > nUnderPrint)
                    {
                        if (nTime == 0)
                        {
                            nThreshJump = nThreshJumpOver;
                        }

                        //영훈 20140103 : Invert된 이미지는 Threshold가 방향이 다르므로 분리해준다.
                        if (m_VisionPara->m_bWhiteBackGround)
                            nDir = -1;
                        else
                            nDir = 1;
                    }
                    else
                    {
                        if (nTime == 0)
                            nThreshJump = nThreshJumpUnder;

                        if (m_VisionPara->m_bWhiteBackGround)
                            nDir = 1;
                        else
                            nDir = -1;
                    }

                    if (nOldDir != 0 && nDir != nOldDir)
                    {
                        nThreshJump /= 2;
                        nTime--; // 한번 더 threshold할 기회를 준다.
                        bFlip = TRUE;
                    }

                    nImgThresh += (nThreshJump * nDir);
                }
            }

            nTime++;

            //{{kircheis_MatchRate 수정
            if (bUseMarkMatchRateAlgo2)
                nMatchingRate = 100 - (nOverPrint + nUnderPrint);
            //}}

            nOverPrint = 100 + nOverPrint;
            nUnderPrint = 100 - nUnderPrint;

            if (nTime == 1)
            {
                m_result->m_vecfFirstTrialOverPrint[i] = (float)nOverPrint;
                m_result->m_vecfFirstTrialUnderPrint[i] = (float)nUnderPrint;
            }

            long nOverPrintResult = NOT_MEASURED;
            UpdateEachInspResult(
                g_szMarkInspectionName[MARK_INSPECTION_OVER_PRINT], i, (float)nOverPrint, nOverPrintResult);

            long nUnderPrintResult = NOT_MEASURED;
            UpdateEachInspResult(
                g_szMarkInspectionName[MARK_INSPECTION_UNDER_PRINT], i, (float)nUnderPrint, nUnderPrintResult);

            long nMatchingRateResult = NOT_MEASURED;
            UpdateEachInspResult(
                g_szMarkInspectionName[MARK_INSPECTION_MATCH_RATE], i, (float)nMatchingRate, nMatchingRateResult);

            //// 아래 검사 항목이 모두 패스이면 중단한다.
            if (nOverPrintResult == PASS && nUnderPrintResult == PASS && nMatchingRateResult == PASS)
            {
                bPass = TRUE;
                break;
            }
        }
        while (nTime < nRepeat && bReinsp && !bPass);

        // 영훈 20160802 : 다른 검사항목과 다른 방식을 사용함으로써 문제 발생
        // 위와 같이 하나 넣고 검사하고 불량이면 다시 넣고 하다보면 reject ROI 및 다른 ROI나 ID가 서로 매칭이 되지 않아버리는 문제가 발생한다.
        // 위에서는 현재 값이 불량인지 아닌지만 판단하고 Re-Inspection이 끝난 마지막 결과에 모든 값을 넣도록 한다.
        //k 좌표
        sObjectID.Format(_T("%d"), i);

        pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_UNDER_PRINT]);
        if (pResult == nullptr)
            return FALSE;
        pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == nullptr)
            return FALSE;
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            i, sObjectID, _T(""), _T(""), (float)nUnderPrint, *pSpec, 0.f, 0.f);
        pResult->SetRect(i, m_result->m_vecrtCharPositionforImage[i]);

        pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_OVER_PRINT]);
        if (pResult == nullptr)
            return FALSE;
        pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == nullptr)
            return FALSE;
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            i, sObjectID, _T(""), _T(""), (float)nOverPrint, *pSpec, 0.f, 0.f);
        pResult->SetRect(i, m_result->m_vecrtCharPositionforImage[i]);

        pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_MATCH_RATE]);
        if (pResult == nullptr)
            return FALSE;
        pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == nullptr)
            return FALSE;
        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            i, sObjectID, _T(""), _T(""), (float)nMatchingRate, *pSpec, 0.f, 0.f);
        pResult->SetRect(i, m_result->m_vecrtCharPositionforImage[i]);
    }

    for (int index = 0; index < nCharNum; index++)
    {
        Ipvm::Rect32s rtChar = m_result->m_vecrtCharPositionforImage[index];

        if (index == 0)
            m_rtAllChar = rtChar;
        else
            m_rtAllChar |= rtChar;
    }

    CharContrastInsp(i_grayImage, refImage, m_result->m_vecrtCharPositionforCalcImage);

    sObjectID.Empty();

    return TRUE;
}

BOOL VisionInspectionMark::GetTotalROIFromSmallROIs(std::vector<Ipvm::Rect32s>& i_vecrtROI, Ipvm::Rect32s& o_rtTotalROI)
{
    if ((long)i_vecrtROI.size() <= 0)
        return FALSE;

    Ipvm::Rect32s rtTempROI(0, 0, 0, 0);
    for (long i = 0; i < (long)i_vecrtROI.size(); i++)
    {
        if (i == 0)
            rtTempROI = i_vecrtROI[i];
        else
            rtTempROI |= i_vecrtROI[i];
    }

    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    if (!CheckROI(rtTempROI, imageSizeX, imageSizeY))
        return FALSE;

    o_rtTotalROI = rtTempROI;

    return TRUE;
}

BOOL VisionInspectionMark::UpdateEachInspResult(
    CString strInspName, long nCharIndex, float fCurrValue, long& nCurrResult)
{
    if (nCharIndex < 0 || nCharIndex > m_VisionTempSpec->m_nCharNum)
        return FALSE;

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strInspName);
    if (pResult == nullptr)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == nullptr)
        return FALSE;

    if (pSpec->m_use)
    {
        nCurrResult = pResult->IsValueCheck(pSpec, fCurrValue);
    }
    else
    {
        nCurrResult = PASS;
    }
    return TRUE;
}

BOOL VisionInspectionMark::CharMatching_Blob(const Ipvm::Image8u& i_theshImage, const Ipvm::Rect32s& rtROI,
    const float fMarkAngle, int index, int& nUnderPrintNum, int& nOverPrintNum, int& nMatchingRate,
    Ipvm::Image8u& refBuffer, Ipvm::Image8u& binBuffer, Ipvm::Image8u& underBuffer, Ipvm::Image8u& overBuffer)
{
    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    Ipvm::Point32r2 imageCenter(imageSizeX * 0.5f, imageSizeY * 0.5f);

    if (!CheckROI(rtROI, imageSizeX, imageSizeY))
        return FALSE;

    if (m_VisionTempSpec->m_plTeachCharArea[index] < 1)
        return FALSE;

    Ipvm::Rect32s rtROIRef = m_VisionTempSpec->m_plTeachCharROI[index] + Ipvm::Conversion::ToPoint32s2(imageCenter);

    if (!CheckROI(rtROIRef, imageSizeX, imageSizeY))
        return FALSE;

    Ipvm::Image8u refImage = m_VisionTempSpec->m_specImage;

    // ROI 영상 영역
    int nxROI = rtROI.Width();
    int nyROI = rtROI.Height();

    // image sample rate
    int nImageSampleRate = m_VisionPara->m_nImageSampleRate;
    if (nImageSampleRate < 1 || rtROIRef.Width() < 10 || rtROIRef.Height() < 10)
        nImageSampleRate = 1;

    // Ref 영상 크기
    int nxRef = rtROIRef.Width();
    int nyRef = rtROIRef.Height();

    Ipvm::Rect32s searchROI = rtROI;
    Ipvm::Image8u refeference(refImage, rtROIRef);

    if (nImageSampleRate > 1)
    {
        Ipvm::Image8u srcROI_t;
        srcROI_t.Create(nxROI / nImageSampleRate, nyROI / nImageSampleRate);
        if (Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(i_theshImage, rtROI), srcROI_t)
            != Ipvm::Status::e_ok)
        {
            return FALSE;
        }

        // sampling된 Ref 영상 크기와 영역
        Ipvm::Image8u refResize_t;
        refResize_t.Create(nxRef / nImageSampleRate, nyRef / nImageSampleRate);

        if (Ipvm::ImageProcessing::ResizeLinearInterpolation(refeference, refResize_t) != Ipvm::Status::e_ok)
        {
            return FALSE;
        }

        Ipvm::Point32s2 matchingPos;
        float score = 0.f;
        if (Ipvm::ImageProcessing::SearchTemplate(srcROI_t, refResize_t, Ipvm::Rect32s(srcROI_t), matchingPos, score)
            != Ipvm::Status::e_ok)
        {
            return FALSE;
        }

        long offset = nImageSampleRate + 1;
        searchROI.m_left = long(rtROI.m_left + matchingPos.m_x * nImageSampleRate - offset);
        searchROI.m_top = long(rtROI.m_top + matchingPos.m_y * nImageSampleRate - offset);
        searchROI.m_right = searchROI.m_left + refeference.GetSizeX() + offset * 2;
        searchROI.m_bottom = searchROI.m_top + refeference.GetSizeY() + offset * 2;
    }

    // maximum position 계산
    long lMatchingNumMax = 0;

    long nSearchSX = searchROI.m_left;
    long nSearchSY = searchROI.m_top;
    long nSearchEX = searchROI.m_right - nxRef;
    long nSearchEY = searchROI.m_bottom - nyRef;

    Ipvm::Image8u temp;
    if (!getReusableMemory().GetByteImage(temp, nxRef, nyRef))
    {
        return FALSE;
    }

    // 영훈 20160624 : Teaching 된 Mark가 검사 중인 Package에 아예 없다면 0, 0 위치에 표시되는 문제로 인해 시작 지점을 정해준다.
    Ipvm::Point32s2 point(nSearchSX, nSearchSY);

    for (int y = nSearchSY; y <= nSearchEY; y++)
    {
        for (int x = nSearchSX; x <= nSearchEX; x++)
        {
            Ipvm::Rect32s rtMatch;
            rtMatch.m_left = x;
            rtMatch.m_top = y;
            rtMatch.m_right = rtMatch.m_left + nxRef;
            rtMatch.m_bottom = rtMatch.m_top + nyRef;

            if ((rtMatch & searchROI) != rtMatch)
                continue;

            Ipvm::Image8u matchingSrc(i_theshImage, rtMatch);

            // source, reference 모두 255인 픽셀수
            double fScoreMatch = 0.;
            Ipvm::ImageProcessing::BitwiseAnd(matchingSrc, refeference, Ipvm::Rect32s(refeference), temp);
            Ipvm::ImageProcessing::GetSum(temp, Ipvm::Rect32s(temp), fScoreMatch);

            fScoreMatch /= 255.f;

            // source, reference 둘 중 하나만 255인 픽셀수
            double fScoreDiff = 0.;
            Ipvm::ImageProcessing::BitwiseXor(matchingSrc, refeference, Ipvm::Rect32s(refeference), temp);
            Ipvm::ImageProcessing::GetSum(temp, Ipvm::Rect32s(temp), fScoreDiff);
            fScoreDiff /= 255.f;

            // Src, Ref 모두 255인 픽셀수. - Src, Ref가 서로 다른 픽셀수.
            long lMatchingNum = (long)(fScoreMatch - fScoreDiff);
            if (lMatchingNum >= lMatchingNumMax)
            {
                point.m_x = x;
                point.m_y = y;
                lMatchingNumMax = lMatchingNum;
            }
        }
    }

    // matching rate
    float fArea = (float)m_VisionTempSpec->m_plTeachCharArea[index];

    nMatchingRate = (int)(lMatchingNumMax / fArea * 100.f);
    if (nMatchingRate >= 100)
        nMatchingRate = 100;
    if (nMatchingRate < 0)
        nMatchingRate = 0;

    // roi test
    SetMarkROI(index, Ipvm::Rect32s(point, point + Ipvm::Point32s2(rtROIRef.Width(), rtROIRef.Height())), fMarkAngle);

    //{{ Reference Character로부터 1픽셀의 오차는 계산에 고려하지 않은 코드
    // under print and over print
    Ipvm::Rect32s rtSrc = Ipvm::Rect32s(point.m_x, point.m_y, point.m_x + nxRef, point.m_y + nyRef);
    Ipvm::Rect32s rtRef = Ipvm::Rect32s(0, 0, nxRef, nyRef);

    Ipvm::Image8u lastSrc = Ipvm::Image8u(i_theshImage, rtSrc);
    Ipvm::Image8u mismatch = temp;
    Ipvm::ImageProcessing::BitwiseXor(lastSrc, refeference, Ipvm::Rect32s(refeference), mismatch);

    Ipvm::ImageProcessing::Fill(rtSrc, 0, underBuffer);
    Ipvm::ImageProcessing::Fill(rtSrc, 0, overBuffer);

    if (refBuffer.GetMem() != nullptr)
    {
        Ipvm::ImageProcessing::Copy(refImage, rtROIRef, rtSrc, refBuffer);
    }

    Ipvm::Image8u temp5_t;
    temp5_t.Create(nxRef, nyRef);

    // under print
    nUnderPrintNum = 0;
    Ipvm::ImageProcessing::BitwiseAnd(mismatch, refeference, Ipvm::Rect32s(refeference), temp5_t);
    for (int y = 0; y < nyRef; y++)
    {
        auto* temp5_y = temp5_t.GetMem(0, y);
        //auto* underBuffer_y = underBuffer.GetMem(rtSrc.m_left, rtSrc.m_top + y);

        for (int x = 0; x < nxRef; x++)
        {
            if (temp5_y[x])
            {
                nUnderPrintNum++;
            }
        }
    }

    if (underBuffer.GetMem() != nullptr)
    {
        for (int y = 0; y < nyRef; y++)
        {
            auto* temp5_y = temp5_t.GetMem(0, y);
            auto* underBuffer_y = underBuffer.GetMem(rtSrc.m_left, rtSrc.m_top + y);

            for (int x = 0; x < nxRef; x++)
            {
                if (temp5_y[x])
                {
                    underBuffer_y[x] = 255;
                }
            }
        }
    }

    // over print
    nOverPrintNum = 0;
    Ipvm::ImageProcessing::BitwiseAnd(mismatch, lastSrc, Ipvm::Rect32s(mismatch), temp5_t);

    for (int y = 0; y < nyRef; y++)
    {
        //auto* mismatch_y = mismatch.GetMem(0, y);
        auto* temp5_y = temp5_t.GetMem(0, y);

        for (int x = 0; x < nxRef; x++)
        {
            if (temp5_y[x])
            {
                nOverPrintNum++;
            }
        }
    }

    if (binBuffer.GetMem() != nullptr)
    {
        for (int y = 0; y < nyRef; y++)
        {
            auto* mismatch_y = mismatch.GetMem(0, y);
            auto* bin_y = binBuffer.GetMem(rtSrc.m_left, rtSrc.m_top + y);

            for (int x = 0; x < nxRef; x++)
            {
                bin_y[x] = mismatch_y[x];
            }
        }
    }

    if (overBuffer.GetMem() != nullptr)
    {
        for (int y = 0; y < nyRef; y++)
        {
            auto* temp5_y = temp5_t.GetMem(0, y);
            auto* over_y = overBuffer.GetMem(rtSrc.m_left, rtSrc.m_top + y);

            for (int x = 0; x < nxRef; x++)
            {
                if (temp5_y[x])
                    over_y[x] = 255;
            }
        }
    }

    return TRUE;
}

// 영훈 : Resize 없이 한번 검사해보자.
BOOL VisionInspectionMark::CharMatching_Blob_NormalSize(const Ipvm::Image8u& i_theshImage, const Ipvm::Rect32s& rtROI,
    const float fMarkAngle, int index, int& nUnderPrintNum, int& nOverPrintNum, int& nMatchingRate,
    Ipvm::Image8u& refBuffer, Ipvm::Image8u& binBuffer, Ipvm::Image8u& underBuffer, Ipvm::Image8u& overBuffer)
{
    const long imageSizeX = getImageLotInsp().GetImageSizeX();
    const long imageSizeY = getImageLotInsp().GetImageSizeY();

    Ipvm::Point32r2 imageCenter(imageSizeX * 0.5f, imageSizeY * 0.5f);

    if (!CheckROI(rtROI, imageSizeX, imageSizeY))
        return FALSE;

    if (m_VisionTempSpec->m_plTeachCharArea[index] < 1)
        return FALSE;

    Ipvm::Rect32s rtROIRef = m_VisionTempSpec->m_plTeachCharROI[index] + Ipvm::Conversion::ToPoint32s2(imageCenter);

    if (!CheckROI(rtROIRef, imageSizeX, imageSizeY))
        return FALSE;

    Ipvm::Image8u refImage = m_VisionTempSpec->m_specImage; // Teaching Image
    // Ref 영상 크기
    long nxROI = rtROI.Width();
    long nyROI = rtROI.Height();
    long nxRef = rtROIRef.Width();
    long nyRef = rtROIRef.Height();
    long nSearchRangeX = nxROI - nxRef;
    long nSearchRangeY = nyROI - nyRef;

    long lMatchingNum;
    long lMatchingNumMax = -9999;

    // 후보 영역 검사
    long nCount = 0;

    Ipvm::Point32s2 point(0, 0);
    Ipvm::Point32s2 ptSearchROICenter(0, 0);
    Ipvm::Rect32s rtSearchROI(0, 0, 0, 0);
    Ipvm::Rect32s rtFindROI(0, 0, 0, 0);
    Ipvm::Rect32s rtBlobUnionRect(0, 0, 0, 0);

    Ipvm::BlobInfo sBlobTemp;

    int32_t nBlobNum = BLOB_INFO_SMALL_SIZE;

    Ipvm::Image32s imageLabel;

    getReusableMemory().GetInspLongImage(imageLabel);
    m_pBlob->DoBlob(i_theshImage, rtROI, BLOB_INFO_SMALL_SIZE, imageLabel, m_psBlobInfo, nBlobNum,
        15); //BlobNum이 0개 일때가 있다.. 마지막 Image..
    m_pBlob->FilteringROI(imageLabel, rtROI, 5, m_psBlobInfo, nBlobNum);
    ptSearchROICenter = rtROI.CenterPoint();

    // 가장 큰 Blob 순서로 정렬
    for (long j = 0; j < nBlobNum - 1; j++)
    {
        for (long i = j + 1; i < nBlobNum; i++)
        {
            if (m_psBlobInfo[j].m_area < m_psBlobInfo[i].m_area)
            {
                sBlobTemp = m_psBlobInfo[j];
                m_psBlobInfo[j] = m_psBlobInfo[i];
                m_psBlobInfo[i] = sBlobTemp;
            }
        }
    }

    // Blob이 없다면 종료
    if (nBlobNum <= 0)
        return FALSE;

    // 영훈 20151126 : Mark Merge 대응으로 Center에서도 한번 비벼본다.
    rtBlobUnionRect = m_psBlobInfo[0].m_roi;

    float fMatchSocre(0.f), fMaxMatchScore(0.f);

    for (long nBlobID = 0; nBlobID < nBlobNum; nBlobID++)
    {
        if (nCount > 2)
            break;

        rtBlobUnionRect.UnionRect(rtBlobUnionRect, m_psBlobInfo[nBlobID].m_roi);

        rtSearchROI.m_left = m_psBlobInfo[nBlobID].m_roi.m_left;
        rtSearchROI.m_top = m_psBlobInfo[nBlobID].m_roi.m_top;
        rtSearchROI.m_right = rtSearchROI.m_left + nxRef;
        rtSearchROI.m_bottom = rtSearchROI.m_top + nyRef;

        // 매칭 점수 계산
        lMatchingNum = FindCharAndMatching(i_theshImage, refImage, rtROIRef, rtSearchROI, fMatchSocre);

        if (lMatchingNum > lMatchingNumMax)
        {
            lMatchingNumMax = lMatchingNum;
            rtFindROI = rtSearchROI;
            fMaxMatchScore = fMatchSocre;
        }

        nCount++;
    }

    // Merge 된 Mark 일 경우 Blob Center에서 다시 매칭 점수 계산
    rtSearchROI.m_left = rtBlobUnionRect.CenterPoint().m_x - (nxRef / 2);
    rtSearchROI.m_top = rtBlobUnionRect.CenterPoint().m_y - (nyRef / 2);
    rtSearchROI.m_right = rtSearchROI.m_left + nxRef;
    rtSearchROI.m_bottom = rtSearchROI.m_top + nyRef;

    lMatchingNum = FindCharAndMatching(i_theshImage, refImage, rtROIRef, rtSearchROI, fMatchSocre);
    if (lMatchingNum > lMatchingNumMax)
    {
        lMatchingNumMax = lMatchingNum;
        rtFindROI = rtSearchROI;
        fMaxMatchScore = fMatchSocre;
    }

    m_vecrtBlobROI[index] = rtFindROI;

    // 찾아진 Blob에서 좌우로 2 Pixel씩 늘려 Search ROI 생성
    rtFindROI.InflateRect(2, 2, 2, 2);

    nxROI = rtFindROI.Width();
    nyROI = rtFindROI.Height();
    nSearchRangeX = nxROI - nxRef;
    nSearchRangeY = nyROI - nyRef;

    lMatchingNumMax = -9999;

    // 생성된 Search ROI에서 Mark Matching률 계산
    for (long ny = 0; ny < nSearchRangeY; ny++)
    {
        for (long nx = 0; nx < nSearchRangeX; nx++)
        {
            rtSearchROI.m_left = rtFindROI.m_left;
            rtSearchROI.m_top = rtFindROI.m_top;
            rtSearchROI.m_right = rtSearchROI.m_left + nxRef;
            rtSearchROI.m_bottom = rtSearchROI.m_top + nyRef;
            rtSearchROI.OffsetRect(nx, ny);

            //참조변수로 하나 받아서 보상되지 않은값을 이용하자..
            lMatchingNum = FindCharAndMatching(i_theshImage, refImage, rtROIRef, rtSearchROI, fMatchSocre);

            if (lMatchingNum > lMatchingNumMax)
            {
                point.m_x = rtSearchROI.m_left;
                point.m_y = rtSearchROI.m_top;
                lMatchingNumMax = lMatchingNum;
                fMaxMatchScore = fMatchSocre;
            }
        }
    }

    // 계산 값이 없다면 종료
    if (lMatchingNumMax == -9999)
    {
        nUnderPrintNum = m_VisionTempSpec->m_plTeachCharArea[index];
        nOverPrintNum = m_VisionTempSpec->m_plTeachCharArea[index];

        return TRUE;
    }

    // matching rate
    BOOL bUseMarkMatchRateCompensate = SystemConfig::GetInstance().m_bUseMarkMatchRateAlgoritm2;
    float fArea = (float)m_VisionTempSpec->m_plTeachCharArea[index];
    if (!bUseMarkMatchRateCompensate)
    {
        lMatchingNumMax = long(fMaxMatchScore + .5f);
    }

    nMatchingRate = (int)(lMatchingNumMax / fArea * 100.f);
    if (nMatchingRate >= 100)
        nMatchingRate = 100;
    if (nMatchingRate < 0)
        nMatchingRate = 0;

    // 찾은 char ROI 생성
    if (point.m_x <= 0)
        point.m_x = 0;
    if (point.m_x >= imageSizeX)
        point.m_x = imageSizeX - 1;
    if (point.m_y <= 0)
        point.m_y = 0;
    if (point.m_y >= imageSizeY)
        point.m_y = imageSizeY - 1;

    rtSearchROI.m_left = point.m_x;
    rtSearchROI.m_top = point.m_y;
    rtSearchROI.m_right = rtSearchROI.m_left + nxRef;
    rtSearchROI.m_bottom = rtSearchROI.m_top + nyRef;

    SetMarkROI(index, rtSearchROI, fMarkAngle);

    // Matchgin이 된 ROI에서 Under Print 계산하기
    // Buffer image
    Ipvm::Image8u buffer0;
    Ipvm::Image8u buffer1;
    Ipvm::Image8u buffer2;
    Ipvm::Image8u buffer3;
    if (!getReusableMemory().GetInspByteImage(buffer0))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(buffer1))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(buffer2))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(buffer3))
        return FALSE;

    //Buffer Reset
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(buffer0), 0, buffer0);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(buffer1), 0, buffer1);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(buffer2), 0, buffer2);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(buffer3), 0, buffer3);

    Ipvm::ImageProcessing::Copy(i_theshImage, rtSearchROI, rtROIRef, buffer0);
    Ipvm::ImageProcessing::BitwiseXor(refImage, buffer0, rtROIRef, buffer2);
    Ipvm::Image8u refBufferROI(refBuffer, rtSearchROI);
    Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(refImage, rtROIRef), refBufferROI);

    // under print
    nUnderPrintNum = 0;
    Ipvm::ImageProcessing::Fill(rtROIRef, 0, buffer3);
    Ipvm::ImageProcessing::BitwiseAnd(refImage, buffer2, rtROIRef, buffer3);

    for (long y = 0; y < nyRef; y++)
    {
        BYTE* ref_y = buffer3.GetMem(rtROIRef.m_left, rtROIRef.m_top + y);
        BYTE* under_y = underBuffer.GetMem(rtSearchROI.m_left, rtSearchROI.m_top + y);

        for (long x = 0; x < nxRef; x++)
        {
            if (ref_y[x])
            {
                ++nUnderPrintNum;

                if (underBuffer.GetMem() != nullptr)
                {
                    under_y[x] = 255;
                }
            }
        }
    }

    // over print
    nOverPrintNum = 0;
    Ipvm::ImageProcessing::Fill(rtROIRef, 0, buffer3);
    Ipvm::ImageProcessing::BitwiseAnd(buffer2, buffer0, rtROIRef, buffer3);

    for (long y = 0; y < nyRef; y++)
    {
        BYTE* ref_y = buffer3.GetMem(rtROIRef.m_left, rtROIRef.m_top + y);
        BYTE* bin_y = binBuffer.GetMem(rtSearchROI.m_left, rtSearchROI.m_top + y);
        BYTE* over_y = overBuffer.GetMem(rtSearchROI.m_left, rtSearchROI.m_top + y);

        for (long x = 0; x < nxRef; x++)
        {
            if (ref_y[x])
            {
                nOverPrintNum++;

                if (overBuffer.GetMem() != nullptr)
                {
                    over_y[x] = 255;
                }
            }

            bin_y[x] = ref_y[x];
        }
    }

    return TRUE;
}

long VisionInspectionMark::FindCharAndMatching(const Ipvm::Image8u& i_theshImage, const Ipvm::Image8u& i_refImage,
    Ipvm::Rect32s rtROIRef, Ipvm::Rect32s rtSearchROI, float& o_fMatchSocre)
{
    // Buffer image
    Ipvm::Image8u buffer0;
    Ipvm::Image8u buffer1;
    Ipvm::Image8u buffer2;
    if (!getReusableMemory().GetInspByteImage(buffer0))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(buffer1))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(buffer2))
        return FALSE;

    Ipvm::ImageProcessing::Fill(rtROIRef, 0, buffer0);
    Ipvm::ImageProcessing::Fill(rtROIRef, 0, buffer1);
    Ipvm::ImageProcessing::Fill(rtROIRef, 0, buffer2);

    // source roi 영상으로부터 Ref 영상 다른 Ref 위치에 복사하기
    Ipvm::ImageProcessing::Copy(i_theshImage, rtSearchROI, rtROIRef, buffer0);
    //// source, reference 모두 255인 픽셀수
    double fScoreMatch = 0.0;
    Ipvm::ImageProcessing::BitwiseAnd(i_refImage, buffer0, rtROIRef, buffer1);
    Ipvm::ImageProcessing::GetSum(buffer1, rtROIRef, fScoreMatch);
    fScoreMatch /= 255.;
    o_fMatchSocre = (float)fScoreMatch;

    // source, reference 둘 중 하나만 255인 픽셀수
    double fScoreDiff = 0.0;
    Ipvm::ImageProcessing::BitwiseXor(i_refImage, buffer0, rtROIRef, buffer2);
    Ipvm::ImageProcessing::GetSum(buffer2, rtROIRef, fScoreDiff);
    fScoreDiff /= 255.;

    // Src, Ref 모두 255인 픽셀수. - Src, Ref가 서로 다른 픽셀수.
    return (long)(fScoreMatch - fScoreDiff);
}

BOOL VisionInspectionMark::CharContrastInsp(
    const Ipvm::Image8u& i_imageGray, const Ipvm::Image8u& i_imageThreshold, std::vector<Ipvm::Rect32s>& vecrtCharROI)
{
    VisionInspectionResult* pContrastResult
        = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_CONTRAST]);
    if (pContrastResult == nullptr)
        return FALSE;
    VisionInspectionSpec* pContrastSpec = GetSpecByName(pContrastResult->m_resultName);

    if (pContrastSpec == nullptr)
        return FALSE;

    if (!pContrastSpec->m_use)
        return TRUE;

    long nCharNum = (long)vecrtCharROI.size();
    CString sObjectID = NULL;
    for (long index = 0; index < nCharNum; index++)
    {
        int nContrast = 0;
        CalContrast(i_imageGray, i_imageThreshold, vecrtCharROI[index], nContrast,
            m_VisionPara->sMarkAlgoParam.nMarkRoiMargin, m_VisionPara->m_bWhiteBackGround);

        sObjectID.Format(_T("%d"), index);
        pContrastResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
            index, sObjectID, _T(""), _T(""), (float)nContrast, *pContrastSpec, 0.f, 0.f);
        pContrastResult->SetRect(index, m_result->m_vecrtCharPositionforImage[index]);
    }

    pContrastResult->SetTotalResult();

    sObjectID.Empty();
    return TRUE;
}

BOOL VisionInspectionMark::CharContrastInsp(
    const Ipvm::Image8u& i_imageGray, const Ipvm::Image8u& i_imageThreshold, Ipvm::Rect32s rtROI, int index)
{
    int nContrast = 0;

    VisionInspectionResult* pContrastResult
        = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_CONTRAST]);
    if (pContrastResult == nullptr)
        return FALSE;
    VisionInspectionSpec* pContrastSpec = GetSpecByName(pContrastResult->m_resultName);
    if (pContrastSpec == nullptr)
        return FALSE;

    if (!pContrastSpec->m_use)
        return TRUE;

    if (!CalContrast(i_imageGray, i_imageThreshold, rtROI, nContrast, m_VisionPara->sMarkAlgoParam.nMarkRoiMargin,
            m_VisionPara->m_bWhiteBackGround))
    {
        pContrastResult->SetResult(index, INVALID);
        return FALSE;
    }

    CString sObjectID = NULL;
    sObjectID.Format(_T("%d"), index);

    pContrastResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
        index, sObjectID, _T(""), _T(""), (float)nContrast, *pContrastSpec, 0.f, 0.f);
    pContrastResult->SetRect(index, m_result->m_vecrtCharPositionforImage[index]);

    sObjectID.Empty();
    return TRUE;
}

BOOL VisionInspectionMark::MarkAngleInsp(float fCurrentAngle, const Ipvm::Rect32s& inspROI)
{
    VisionInspectionResult* pMarkAngleResult
        = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_MARK_ANGLE]);
    if (pMarkAngleResult == NULL)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pMarkAngleResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    if (!pSpec->m_use)
        return TRUE;

    pMarkAngleResult->Clear();
    pMarkAngleResult->Resize(1);

    float fDiffAngle = fCurrentAngle; // - m_VisionTempSpec->m_fLocAngleForBody;
    float fAngleCur;
    float fAngleTest;
    if (m_VisionPara->m_nSelectAngleReference == Mark_Angle_Ref_Body_Align)
        fAngleCur = fCurrentAngle;
    else
    {
        fAngleCur = float(fCurrentAngle / ITP_PI * 180.f); // - m_VisionTempSpec->m_fLocAngleForBody;
        fAngleCur = (float)((long)(fAngleCur * 1000000) % 90000000) / 1000000.f;
        float fAngleSpec = (float)(m_VisionTempSpec->m_fLocAngleForBody / ITP_PI * 180.f);
        fAngleSpec = (float)((long)(fAngleSpec * 1000000) % 90000000) / 1000000.f;
        fDiffAngle = fAngleCur - fAngleSpec;
        long nCnt = 0;
        if (fDiffAngle > 10.f)
        {
            fAngleTest = fDiffAngle;
            do
            {
                fAngleTest -= 90.f;
                nCnt++;
            }
            while ((fAngleTest > 15.f || fAngleTest < -15.f) && nCnt < 8);
            if (nCnt < 7)
                fDiffAngle = fAngleTest;
        }
        else if (fDiffAngle < -10.f)
        {
            fAngleTest = fDiffAngle;
            do
            {
                fAngleTest += 90.f;
                nCnt++;
            }
            while ((fAngleTest > 15.f || fAngleTest < -15.f) && nCnt < 8);
            if (nCnt < 7)
                fDiffAngle = fAngleTest;
        }
    }

    // 세로열의 문자에 대한 Angle 관련 버그 수정, 김일남
    // 	if( abs(fDiffAngle) > ITP_HalfPI/2.0f )
    // 	{
    // 		if( fCurrentAngle > m_VisionTempSpec->m_fLocAngleForBody )
    // 			fDiffAngle -= (float)ITP_HalfPI;
    // 		else
    // 			fDiffAngle += (float)ITP_HalfPI;
    // 	}

    pMarkAngleResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
        0, _T("0"), _T(""), _T(""), fDiffAngle, *pSpec, 0.f, 0.f);

    Ipvm::Rect32s rtTotalROI;
    if (GetTotalROIFromSmallROIs(m_result->m_vecrtCharPositionforImage, rtTotalROI))
    {
        pMarkAngleResult->SetRect(0, rtTotalROI);
    }
    else
    {
        pMarkAngleResult->SetRect(0, inspROI);
    }
    return TRUE;
}

BOOL VisionInspectionMark::MarkBlobSizebyUnderOverImage()
{
    bool makeViewerImage = true;
    if (m_blobSizeUnderViewer->GetSizeX() <= 0 || m_blobSizeUnderViewer->GetSizeY() <= 0
        || m_blobSizeOverViewer->GetSizeX() <= 0 || m_blobSizeOverViewer->GetSizeY() <= 0)
    {
        makeViewerImage = false;
    }

    if (m_underViewer == NULL || m_overViewer == NULL)
        return FALSE;

    if (m_result->m_vecrtCharPositionforCalcImage.size() <= 0)
        return FALSE;

    Ipvm::Image8u minusBlob;
    Ipvm::Image8u plusBlob;

    if (!getReusableMemory().GetInspByteImage(minusBlob))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(plusBlob))
        return FALSE;

    //초기화
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(minusBlob), 0, minusBlob);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(plusBlob), 0, plusBlob);

    long nCharNum = m_VisionTempSpec->m_nCharNum;

    Ipvm::Rect32s rtMarkArea(m_result->m_vecrtCharPositionforCalcImage[0]);
    for (int i = 0; i < nCharNum; i++)
    {
        Ipvm::Rect32s rtTemp = m_result->m_vecrtCharPositionforCalcImage[i];
        rtMarkArea.UnionRect(rtMarkArea, rtTemp);
    }

    rtMarkArea.InflateRect(5, 5);

    Ipvm::Rect32s imageRoi(*m_underViewer);

    Ipvm::ImageProcessing::Copy(*m_overViewer, imageRoi, plusBlob);
    Ipvm::ImageProcessing::Copy(*m_underViewer, imageRoi, minusBlob);

    if (makeViewerImage)
    {
        Ipvm::ImageProcessing::Fill(imageRoi, 0, *m_blobSizeUnderViewer);
        Ipvm::ImageProcessing::Fill(imageRoi, 0, *m_blobSizeOverViewer);
    }

    if (m_VisionPara->sMarkAlgoParam.nThinningIntensity < 0)
    {
        m_VisionPara->sMarkAlgoParam.nThinningIntensity = 0;
    }
    else if (m_VisionPara->sMarkAlgoParam.nThinningIntensity > 0)
    {
        //// Under Print
        if (m_VisionPara->sMarkAlgoParam.nThinningIntensity > 0)
        {
            Ipvm::ImageProcessing::MorphErodeIter(
                rtMarkArea, 3, 3, m_VisionPara->sMarkAlgoParam.nThinningIntensity, minusBlob);
        }
        if (m_VisionPara->sMarkAlgoParam.nThinningIntensity > 0)
        {
            Ipvm::ImageProcessing::MorphDilateIter(
                rtMarkArea, 3, 3, m_VisionPara->sMarkAlgoParam.nThinningIntensity, minusBlob);
        }

        //// Over Print
        if (m_VisionPara->sMarkAlgoParam.nThinningIntensity > 0)
        {
            Ipvm::ImageProcessing::MorphErodeIter(
                rtMarkArea, 3, 3, m_VisionPara->sMarkAlgoParam.nThinningIntensity, plusBlob);
        }
        if (m_VisionPara->sMarkAlgoParam.nThinningIntensity > 0)
        {
            Ipvm::ImageProcessing::MorphDilateIter(
                rtMarkArea, 3, 3, m_VisionPara->sMarkAlgoParam.nThinningIntensity, plusBlob);
        }
    }

    if (makeViewerImage)
    {
        Ipvm::ImageProcessing::Copy(minusBlob, rtMarkArea, *m_blobSizeUnderViewer);
        Ipvm::ImageProcessing::Copy(plusBlob, rtMarkArea, *m_blobSizeOverViewer);
    }

    //// Over Print
    int32_t nOverBlobNum = BLOB_INFO_MEDIUM_SIZE;
    Ipvm::BlobInfo* pBlobInfo_Over = getReusableMemory().GetBlobInfo(0);

    Ipvm::Image32s imageLabel;
    getReusableMemory().GetInspLongImage(imageLabel);

    if (m_pBlob->DoBlob(plusBlob, rtMarkArea, BLOB_INFO_MEDIUM_SIZE, imageLabel, pBlobInfo_Over, nOverBlobNum, 3) != 0)
    {
        return FALSE;
    }

    if (!MergeMarkForTeach(pBlobInfo_Over, nOverBlobNum, imageLabel, m_VisionPara->sMarkAlgoParam.fBlobSizeMergeGap))
        return FALSE;

    // 영훈 20160716 : Blobinfo를 std::vector로 사용하면서 Size를 넘어가니까 Blob Size 검사항목에서 invalid가 발생 그 후 검사에서 모두 invalid가 발생한다.
    // 포인트로 변경하고 Blob size도 기존 100에서 500으로 늘린다.

    //// Under Print
    int32_t nUnderBlobNum = BLOB_INFO_MEDIUM_SIZE;
    Ipvm::BlobInfo* pBlobInfo_Under = getReusableMemory().GetBlobInfo(1);

    if (m_pBlob->DoBlob(minusBlob, rtMarkArea, BLOB_INFO_MEDIUM_SIZE, imageLabel, pBlobInfo_Under, nUnderBlobNum, 3)
        != 0)
    {
        return FALSE;
    }

    MergeMarkForTeach(pBlobInfo_Under, nUnderBlobNum, imageLabel, m_VisionPara->sMarkAlgoParam.fBlobSizeMergeGap);

    VisionInspectionResult* pBlobSizeResult
        = m_resultGroup.GetResultByName(g_szMarkInspectionName[MARK_INSPECTION_BLOB_SIZE]);
    if (pBlobSizeResult == nullptr)
        return FALSE;
    VisionInspectionSpec* pBlobSizeSpec = GetSpecByName(pBlobSizeResult->m_resultName);
    if (pBlobSizeSpec == nullptr)
        return FALSE;

    if (!pBlobSizeSpec->m_use)
        return TRUE;

    pBlobSizeResult->Clear();
    pBlobSizeResult->Resize(nCharNum);

    for (long i = 0; i < nCharNum; i++)
    {
        Ipvm::Rect32s rtChar = m_result->m_vecrtCharPositionforCalcImage[i];
        rtChar.InflateRect(3, 3);

        long nMaxOverArea = 0;
        long nMaxOverLabel = -1;
        Ipvm::Rect32s rtMaxOverArea = Ipvm::Rect32s(0, 0, 0, 0);

        for (long nIndex = 0; nIndex < nOverBlobNum; nIndex++)
        {
            const auto& overBlob = pBlobInfo_Over[nIndex];
            auto overBlobROI = overBlob.m_roi;

            if ((rtChar & overBlobROI) == overBlobROI)
            {
                if (nMaxOverArea < overBlob.m_area)
                {
                    nMaxOverArea = overBlob.m_area;
                    nMaxOverLabel = overBlob.m_label;
                    rtMaxOverArea = overBlobROI;
                }
            }
        }

        long nMaxUnderArea = 0;
        long nMaxUnderLabel = -1;
        Ipvm::Rect32s rtMaxUnderArea = Ipvm::Rect32s(0, 0, 0, 0);
        for (long nIndex = 0; nIndex < nUnderBlobNum; nIndex++)
        {
            const auto& underBlob = pBlobInfo_Under[nIndex];
            auto underBlobROI = underBlob.m_roi;

            if ((rtChar & underBlobROI) == underBlobROI)
            {
                if (nMaxUnderArea < underBlob.m_area)
                {
                    nMaxUnderArea = underBlob.m_area;
                    rtMaxUnderArea = underBlobROI;
                    nMaxUnderLabel = underBlob.m_label;
                }
            }
        }

        float fBlobSizeError = 0;
        Ipvm::Rect32s rtBlobSize = Ipvm::Rect32s(0, 0, 0, 0);
        if (nMaxUnderArea > nMaxOverArea)
        {
            fBlobSizeError = (float)nMaxUnderArea;
        }
        else
        {
            fBlobSizeError = (float)nMaxOverArea;
        }

        CString sObjectID = NULL;
        sObjectID.Format(_T("%d"), i);

        if (m_fixedInspectionSpecs[MARK_INSPECTION_BLOB_SIZE].m_use)
        {
            pBlobSizeResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                i, sObjectID, _T(""), _T(""), fBlobSizeError, *pBlobSizeSpec, 0.f, 0.f);
            pBlobSizeResult->SetRect(i, m_result->m_vecrtCharPositionforImage[i]);
        }

        sObjectID.Empty();
    }

    pBlobSizeResult->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionMark::CalContrast(const Ipvm::Image8u& i_imageGray, const Ipvm::Image8u& i_imageThreshold,
    Ipvm::Rect32s rtROI, int& nContrast, int nMargin, BOOL bBlackImage)
{
    long lSumFg, lNumFg;
    long lSumBg, lNumBg;

    rtROI.InflateRect(nMargin, nMargin, nMargin, nMargin);
    rtROI &= Ipvm::Rect32s(i_imageGray);

    lSumFg = lNumFg = lSumBg = lNumBg = 0;

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        auto* image_y = i_imageGray.GetMem(0, y);
        auto* threshold_y = i_imageThreshold.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            BYTE score = 0;
            if (bBlackImage)
                score = 255 - image_y[x];
            else
                score = image_y[x];

            if (threshold_y[x] > 0)
            {
                lSumFg += score;
                lNumFg++;
            }
            else
            {
                lSumBg += score;
                lNumBg++;
            }
        }
    }

    //{{ 김일남 2003.12.10
    if (lNumFg <= 0 || lNumBg <= 0)
    {
        nContrast = 255;
    }
    else
    {
        float fAvgFg = (float)lSumFg / lNumFg;
        float fAvgBg = (float)lSumBg / lNumBg;
        //if(fAvgFg==0. && fAvgBg==0.) return FALSE;
        //nContrast=(int)((fAvgFg-fAvgBg)/(fAvgFg+fAvgBg)*100);
        nContrast = (long)(fabs(fAvgFg - fAvgBg) + 0.5f);
    }
    //}}

    return TRUE;
}

// Update Result...
BOOL VisionInspectionMark::UpdateResult()
{
    BOOL bResult = TRUE;
    VisionInspectionSpec* pSpec = NULL;
    VisionInspectionResult* pResult = NULL;

    // 영훈 20150403_Mark_UpdateResult ; Result와 Spec의 개수가 맞지 않는 문제가 발생하여 이와 같이 최적화 하고 Spec도 NULL인지 확인하도록 한다.
    BOOL bInvalid = FALSE;

    for (long n = MARK_INSPECTION_START; n < MARK_INSPECTION_END; n++)
    {
        pResult = m_resultGroup.GetResultByName(g_szMarkInspectionName[n]);
        if (pResult == nullptr)
            continue;
        pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == nullptr)
            continue;

        if (pSpec->m_use)
        {
            pResult->UpdateTypicalValue(pSpec);
            pResult->SetTotalResult();

            if (pResult->m_totalResult != PASS)
            {
                if (pResult->m_totalResult == INVALID)
                {
                    // 더이상 계산이 불가능하다
                    bInvalid = TRUE;
                }

                bResult = FALSE;
            }
        }
    }

    if (bResult == FALSE)
        m_nTotalResult = REJECT;
    else
        m_nTotalResult = TRUE;

    return !bInvalid;
}

void VisionInspectionMark::DisplayResultOverlay(BOOL bTeach)
{
    // 영훈 20140722_Mark_BodyAlignResult : BodyAlignResult가 없어 Program이 죽는 문제 처리
    m_bodyAlignResult = GetAlignInfo();
    if (m_bodyAlignResult == nullptr)
    {
        return;
    }

    if (m_pVisionInspDlg != NULL)
    {
        if (bTeach)
        {
            COLORREF rgbCharColor = RGB(255, 255, 0);

            long nCharNum = (long)m_result->m_vecrtCharPositionforCalcImage.size();
            for (long i = 0; i < nCharNum; i++)
            {
                Ipvm::Rect32s rtROI(m_result->m_vecrtCharPositionforCalcImage[i]);

                Ipvm::Point32s2 pt(0, 0);
                pt.m_x = rtROI.m_left + ((rtROI.m_right - rtROI.m_left) / 2);
                pt.m_y = rtROI.m_bottom + 1;

                CString strIndex;
                strIndex.Format(_T("#%d"), i);

                m_pVisionInspDlg->m_imageLotView->Overlay_AddRectangle(rtROI, rgbCharColor);
                m_pVisionInspDlg->m_imageLotView->Overlay_AddText(pt, strIndex, rgbCharColor, 10);

                if ((i == m_VisionTempSpec->m_pnLocID[0]) || (i == m_VisionTempSpec->m_pnLocID[1]))
                {
                    Ipvm::Rect32s rtLocater = rtROI;
                    rtLocater.InflateRect(10, 10);
                    m_pVisionInspDlg->m_imageLotView->Overlay_AddRectangle(rtLocater, rgbCharColor);
                }

                strIndex.Empty();
            }

            auto bodyLT = m_bodyAlignResult->getBodyRect().TopLeft();
            Ipvm::Point32s2 ptLeftTop = Ipvm::Point32s2((long)bodyLT.m_x, (long)bodyLT.m_y - 5);
            m_pVisionInspDlg->m_imageLotView->Overlay_AddText(ptLeftTop, _T("Teaching Success"), RGB(0, 255, 0), 10);
        }
        else
        {
            auto bodyLT = m_bodyAlignResult->getBodyRect().TopLeft();
            Ipvm::Point32s2 ptLeftTop = Ipvm::Point32s2((long)bodyLT.m_x, (long)bodyLT.m_y - 5);
            m_pVisionInspDlg->m_imageLotView->Overlay_AddText(ptLeftTop, _T("Teaching Fail"), RGB(255, 0, 0), 10);
        }
        m_pVisionInspDlg->m_imageLotView->Overlay_Show(TRUE);
    }
}

BOOL VisionInspectionMark::SimilerStringCompare(BOOL bSimilerComp, CString strRef, CString strRes)
{
    CString strOrigin = strRes;

    if (strRef == strRes)
        return TRUE;
    else if (bSimilerComp == FALSE)
        return FALSE;

    //Ref의 영문 대문자 'O'를 모두 찾아서  Res의 대응대는 문자를 영문 대문자 'O'로 바꾸어 검사해본다
    long nOPos = 0;
    long nCnt = 0;

    do
    {
        ++nCnt;

        nOPos = strRef.Find(_T('O'));
        if (nOPos != -1)
        {
            strRes.SetAt(nOPos, _T('0'));
        }

        if (strRef == strRes)
            return TRUE;

        if (nCnt > 50)
            return FALSE;
    }
    while (nOPos != -1);

    //영문 대문자 'O'때문에 실패한 것이 아니므로 Res문자열을 원래것으로 원복
    strRes = strOrigin;

    //Ref의 숫자 '0'을 모두 찾아서
    long n0Pos = 0;
    nCnt = 0;

    do
    {
        ++nCnt;

        n0Pos = strRef.Find(_T('0'));
        if (nOPos != -1)
        {
            strRes.SetAt(nOPos, _T('0'));
        }

        if (strRef == strRes)
            return TRUE;

        if (nCnt > 50)
            return FALSE;
    }
    while (nOPos != -1);

    //영문 'O'나 숫자 '0'때문에 실패한 것이 아니다

    strOrigin.Empty();

    return FALSE;
}

CString VisionInspectionMark::RealignFailPartID(CString strPartID)
{
    CString strResult;
    strResult.Empty();

    int nStart = 0;

    if (true)
    {
        CString strIDBlock = strPartID.Mid(0, 7);
        if (strIDBlock == _T("SAMSUNG"))
        {
            if (strPartID.GetAt(7) <= _T('0') || strPartID.GetAt(7) >= _T('9'))
                nStart = 7;
            else
                nStart = 10;
            strResult = strPartID.Mid(nStart, 14);
        }
        strIDBlock.Empty();
    }

    if (strResult.IsEmpty())
    {
        CString strIDBlock = strPartID.Mid(0, 3);
        if (strIDBlock == _T("SEC"))
        {
            if (strPartID.GetAt(3) <= _T('0') || strPartID.GetAt(3) >= _T('9'))
                nStart = 7;
            else
                nStart = 10;
            strResult = strPartID.Mid(nStart, 10) + strPartID.Mid(nStart - 4, 4);
        }
        strIDBlock.Empty();
    }

    if (strResult.IsEmpty())
    {
        if (strPartID.GetAt(0) <= _T('0') || strPartID.GetAt(0) >= _T('9'))
            nStart = 0;
        else
            nStart = 3;
        strResult = strPartID.Mid(nStart, 14);
    }
    if (strResult.IsEmpty())
        strResult = strPartID;

    return strResult;
}

BOOL VisionInspectionMark::SimilerStringFind(BOOL bSimilerComp, CString strRef, CString strRes)
{
    long nRet = 0;
    long nOPos = 0;
    CString strOrigin = strRes;

    nRet = strRef.Find(strRes);

    if (nRet >= 0)
        return nRet;
    else if (bSimilerComp)
    {
        //1. Ref에서 유사 문자를 찾는다
        do
        {
            nOPos = strRef.Find('O', nOPos);
            if (nOPos != -1)
            {
                //2. Res에서 유사 문자와 매칭되는 문자를 바꾼다
                strRes.Delete(nOPos);
                strRes.Insert(nOPos, 'O');
            }
            else
                break;
            nOPos++;
        }
        while (nOPos != 0);
        //3. 찾는다
        nRet = strRef.Find(strRes);
        if (nRet >= 0)
            return nRet;

        //영문 대문자 'O'때문에 실패한 것이 아니므로 Res문자열을 원래것으로 원복
        strRes = strOrigin;

        //Ref의 숫자 '0'을 모두 찾아서
        long nNumPos = 0;
        do
        {
            nNumPos = strRef.Find('0', nNumPos);
            if (nNumPos != -1)
            {
                strRes.Delete(nNumPos);
                strRes.Insert(nNumPos, '0');
            }
            else
                break;
            nNumPos++;
        }
        while (nNumPos != 0);

        nRet = strRef.Find(strRes);
        if (nRet >= 0)
            return nRet;
    }

    strOrigin.Empty();

    return FALSE;
}

void VisionInspectionMark::CalcAutoTeachRoiOffset()
{
    if (m_VisionTempSpec == NULL)
        return;

    Ipvm::Point32s2 ptOffset;
    Ipvm::Point32s2 ptCalcOffset;
    Ipvm::Rect32s rtROI;

    //{{ Teach ROI
    ptOffset = m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter;
    if (ptOffset.m_x == 0 && ptOffset.m_y == 0)
    {
        rtROI = getScale().convert_umToPixel(m_VisionTempSpec->m_rtMarkTeachROI_BCU);
        ptCalcOffset.m_x = rtROI.m_left;
        ptCalcOffset.m_y = rtROI.m_top;
        m_VisionTempSpec->m_ptTeachRoiLtFromPackCenter = ptCalcOffset;
    }
    //}}

    //{{ Locator Target ROI
    if (m_VisionPara->m_bLocatorTargetROI)
    {
        for (long i = 0; i < NUM_OF_LOCATOR; i++)
        {
            ptOffset = m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i];
            if (ptOffset.m_x == 0 && ptOffset.m_y == 0)
            {
                rtROI = m_VisionTempSpec->m_rtLocatorTargetROI[i];
                ptCalcOffset.m_x = rtROI.m_left;
                ptCalcOffset.m_y = rtROI.m_top;
                m_VisionTempSpec->m_pptLocaterTargetRoiLtFromPackCenter[i] = ptCalcOffset;
            }
        }
    }
    //}}

    //{{ Merge ROI
    for (auto& info : m_VisionPara->m_teach_merge_infos)
    {
        auto& roiLtFromPackCenter = info.m_roiLtFromPackCenter;
        if (roiLtFromPackCenter.m_x == 0 && roiLtFromPackCenter.m_y == 0)
        {
            rtROI = info.m_roi;
            roiLtFromPackCenter.m_x = rtROI.m_left;
            roiLtFromPackCenter.m_y = rtROI.m_top;
        }
    }
    //}}

    //{{ Ignore ROI
    long nNumOfROI = (long)m_VisionPara->m_vecrtUserIgnore.size();
    if (nNumOfROI > 0)
    {
        BOOL bResize = FALSE;
        long nCurOffsetNum = (long)m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter.size();
        if (nNumOfROI != nCurOffsetNum)
        {
            bResize = TRUE;
            m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter.resize(nNumOfROI);
        }

        for (long i = 0; i < nNumOfROI; i++)
        {
            ptOffset = m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter[i];
            if (bResize || (ptOffset.m_x == 0 && ptOffset.m_y == 0))
            {
                rtROI = m_VisionPara->m_vecrtUserIgnore[i];
                ptCalcOffset.m_x = rtROI.m_left;
                ptCalcOffset.m_y = rtROI.m_top;
                m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter[i] = ptCalcOffset;
            }
        }
    }
    //}}
}

void VisionInspectionMark::ResetIgnoreBoxStatus() //kircheis_for_ITEK
{
    if (!m_VisionPara->m_bInitialIgnoreBox)
        return;

    m_VisionPara->m_nIgnoreROINum = 0;
    m_VisionPara->m_vecrtUserIgnore.clear();
    m_VisionPara->m_vecptUserIgnoreRoiLtFromPackCenter.clear();
}