//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLgaBottom3D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionLgaBottom3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/ConversionEx.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedComponent/IntelSpecific/ShapeAlgorithm.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/Coplanarity_Algorithm.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image64r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/PlaneEq64r.h>
#include <Ipvm/Base/Point32r3.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/Size32s2.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define _PI_ 3.1415926535897932384626433832795
#define _2PI_ (2 * _PI_)
#define PI_ED_DIR_OUTER 0 // 안에서 밖으로
#define PI_ED_DIR_INNER 1 // 밖에서 안으로

//CPP_7_________________________________ Implementation body
//
BOOL VisionInspectionLgaBottom3D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck TimeInspectionTotal;
    //{{ //kircheis_3DEmpty
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    Ipvm::TimeCheck Time;

    BOOL bPass = FALSE;

    m_result.Clear();

    ResetResult();
    m_vecrtReject.clear();

    Ipvm::TimeCheck TimeAlign;
    if (DoAlign(detailSetupMode))
    {
        AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("AlignTime"), (float)TimeAlign.Elapsed_ms());

        Ipvm::TimeCheck TimeInspection;
        bPass = DoInsp(detailSetupMode);
        AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("InspTime"), (float)TimeInspection.Elapsed_ms());
    }
    else
    {
        m_bInvalid = TRUE;
    }

    m_fCalcTime = (float)Time.Elapsed_ms();

    AddProcessingFunctionExcuteTimeLog(m_strModuleName, _T("InspTotalTime"), (float)TimeInspectionTotal.Elapsed_ms());

    return bPass;
}

BOOL VisionInspectionLgaBottom3D::GetBodyAlignInfo(
    float& o_fAngle, Ipvm::Rect32r& o_frtBody, Ipvm::Point32r2& o_fptCenter)
{
    auto visionDebugInfos = m_visionUnit.GetVisionDebugInfos();

    FPI_RECT* psfrtBody = NULL;

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body ROI"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        psfrtBody = (FPI_RECT*)pData;

    {
        nDataNum = 0;
        pData = m_visionUnit.GetVisionDebugInfo(
            m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
        if (pData != nullptr && nDataNum > 0)
            *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        else
            return FALSE;
    }

    FPI_RECT sfrtBody = (psfrtBody != NULL) ? *psfrtBody
                                            : FPI_RECT(m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT,
                                                  m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB);

    o_frtBody = sfrtBody.GetFRect();
    o_fptCenter = o_frtBody.CenterPoint();

    const float angleLower
        = (float)atan2(sfrtBody.fptRB.m_y - sfrtBody.fptLB.m_y, sfrtBody.fptRB.m_x - sfrtBody.fptLB.m_x);
    const float angleUpper
        = (float)atan2(sfrtBody.fptRT.m_y - sfrtBody.fptLT.m_y, sfrtBody.fptRT.m_x - sfrtBody.fptLT.m_x);
    o_fAngle = (angleLower + angleUpper) / 2.f;

    m_result.sfrtPackageBody = sfrtBody;
    m_result.m_packageCenter = m_sEdgeAlignResult->m_center;
    m_result.m_packageAngle_rad = m_sEdgeAlignResult->m_angle_rad;

    m_result.sfrtRotatePackageBody = sfrtBody.Rotate(o_fAngle);

    return TRUE;
}

void VisionInspectionLgaBottom3D::ImageCopyinMask(const Ipvm::Image8u i_mask, const Ipvm::Rect32s i_rtDstROI,
    const Ipvm::Rect32s i_rtMaskROI, Ipvm::Image8u& o_dstImage)
{
    long nLength = i_rtMaskROI.m_bottom - i_rtMaskROI.m_top;
    long nWidth = i_rtMaskROI.m_right - i_rtMaskROI.m_left;

    for (long y = 0; y < nLength; y++)
    {
        long masky = i_rtMaskROI.m_top + y;
        long dsty = i_rtDstROI.m_top + y;

        auto mask = i_mask.GetMem(0, masky);
        auto dst = o_dstImage.GetMem(0, dsty);

        for (long x = 0; x < nWidth; x++)
        {
            long maskx = i_rtMaskROI.m_left + x;
            long dstx = i_rtDstROI.m_left + x;

            if (mask[maskx] == 255)
            {
                dst[dstx] = mask[maskx];
            }
        }
    }
}

bool VisionInspectionLgaBottom3D::CollectBodyAlignResult()
{
    bool available = false;
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
    {
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
        available = true;
    }

    return available;
}

void VisionInspectionLgaBottom3D::GetSrIgnoreQRT(Ipvm::Quadrangle32r& o_qrtValidSR)
{
    Ipvm::Quadrangle32r qrtBody;
    qrtBody = ConversionEx::ToQuadrangle32r(
        m_sEdgeAlignResult->fptLT, m_sEdgeAlignResult->fptRT, m_sEdgeAlignResult->fptLB, m_sEdgeAlignResult->fptRB);

    float fOffsetLeftUM = m_VisionPara.m_SR_IgnoreOffsetXum;
    float fOffsetTopUM = m_VisionPara.m_SR_IgnoreOffsetYum;
    float fOffsetRightUM = m_VisionPara.m_SR_IgnoreOffsetXum;
    float fOffsetBottomUM = m_VisionPara.m_SR_IgnoreOffsetYum;

    const auto& um2px = getScale().umToPixel();

    const float fOffsetLeftPXL = fOffsetLeftUM * um2px.m_x;
    const float fOffsetTopPXL = fOffsetTopUM * um2px.m_y;
    const float fOffsetRightPXL = fOffsetRightUM * um2px.m_x;
    const float fOffsetBottomPXL = fOffsetBottomUM * um2px.m_y;

    qrtBody.m_ltX += fOffsetLeftPXL;
    qrtBody.m_ltY += fOffsetTopPXL;
    qrtBody.m_rtX -= fOffsetRightPXL;
    qrtBody.m_rtY += fOffsetTopPXL;
    qrtBody.m_lbX += fOffsetLeftPXL;
    qrtBody.m_lbY -= fOffsetBottomPXL;
    qrtBody.m_rbX -= fOffsetRightPXL;
    qrtBody.m_rbY -= fOffsetBottomPXL;

    Ipvm::Rect32s roiImage(getImageLotInsp().GetImageRect());

    qrtBody.m_ltX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_ltX));
    qrtBody.m_ltY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_ltY));
    qrtBody.m_rtX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_rtX));
    qrtBody.m_rtY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_rtY));
    qrtBody.m_lbX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_lbX));
    qrtBody.m_lbY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_lbY));
    qrtBody.m_rbX = (float)max(0.f, (float)min((float)roiImage.m_right - 1.f, qrtBody.m_rbX));
    qrtBody.m_rbY = (float)max(0.f, (float)min((float)roiImage.m_bottom - 1.f, qrtBody.m_rbY));

    o_qrtValidSR = qrtBody;
}

BOOL VisionInspectionLgaBottom3D::MakeLandMask(
    const Ipvm::Image8u i_binImage, const Ipvm::Image8u i_grayImage, const bool detailSetupMode)
{
    long nGroupSize = (long)m_group_id_list.size();
    Ipvm::Rect32s rtPackage = m_result.sfrtPackageBody.GetRoughRect();

    //FindLand
    Ipvm::Image8u tempImage;
    Ipvm::Image8u maskImage;
    Ipvm::Image8u SRmaskImage_Ignore;
    Ipvm::Image8u SRmaskImage_Dilate;

    if (!getReusableMemory().GetInspByteImage(tempImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(maskImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(SRmaskImage_Ignore))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(SRmaskImage_Dilate))
        return FALSE;

    //초기화
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(tempImage), 0, tempImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(maskImage), 0, maskImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(SRmaskImage_Ignore), 0, SRmaskImage_Ignore);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(SRmaskImage_Dilate), 0, SRmaskImage_Dilate);

    std::vector<Ipvm::Image8u> vecLandMasks;
    std::vector<Ipvm::Image8u> vecGrayLandMasks;
    vecLandMasks.clear();
    vecGrayLandMasks.clear();

    m_result.vecrtBlobObj.clear();
    m_result.vecrtBlobObj.resize(nGroupSize);

    std::vector<Ipvm::Image8u> LandMasks;

    LandMasks.resize(m_group_id_list.size());

    float fSRSearchRatio = m_VisionPara.m_fSRSearchExtRatio / 100.f + 1.f;

    for (long GroupID = 0; GroupID < m_group_id_list.size(); GroupID++)
    {
        std::vector<Ipvm::Image8u> vecCalcImage;
        long nOLandSizeX = 0;
        long nOLandSizeY = 0;

        for (const auto& LandROI : m_result.vecsfrtBlobLand[GroupID])
        {
            if (nOLandSizeX < LandROI.GetRoughRect().Width())
                nOLandSizeX = LandROI.GetRoughRect().Width();
            if (nOLandSizeY < LandROI.GetRoughRect().Height())
                nOLandSizeY = LandROI.GetRoughRect().Height();
        }

        //ROI Size 110% 확장

        float fumtopx = getScale().umToPixelXY();
        long nSR_DilateSize = 0;
        long nSR_IgnoreSize = 0;

        if (m_VisionPara.m_vecLandParameter[GroupID].m_SR_landDilateSize > 0)
        {
            nSR_DilateSize
                = (long)(m_VisionPara.m_vecLandParameter[GroupID].m_SR_landDilateSize * fumtopx * 2.f + 0.5f);
        }

        if (m_VisionPara.m_vecLandParameter[GroupID].m_SR_landIgnoreSize > 0)
        {
            nSR_IgnoreSize
                = (long)(m_VisionPara.m_vecLandParameter[GroupID].m_SR_landIgnoreSize * fumtopx * 2.f + 0.5f);
        }

        long nLandSizeX = (long)max(nOLandSizeX * 1.5f, nOLandSizeX + nSR_DilateSize + 2);
        long nLandSizeY = (long)max(nOLandSizeY * 1.5f, nOLandSizeY + nSR_DilateSize + 2);

        long nSR_SizeX = (long)max(nLandSizeX, nLandSizeX * fSRSearchRatio);
        long nSR_SizeY = (long)max(nLandSizeY, nLandSizeY * fSRSearchRatio);

        long nValidIdx = 0;

        for (auto LandCenter : m_result.vecptObjectLandCenter[GroupID])
        {
            if (!m_result.vecptObjectValidLand[GroupID][nValidIdx++])
            {
                continue;
            }

            Ipvm::Rect32s LandROI;
            LandROI.m_left = int32_t(LandCenter.m_x - (nLandSizeX / 2));
            LandROI.m_top = int32_t(LandCenter.m_y - (nLandSizeY / 2));
            LandROI.m_right = int32_t(LandROI.m_left + nLandSizeX);
            LandROI.m_bottom = int32_t(LandROI.m_top + nLandSizeY);

            Ipvm::Image8u imageCalcAverage(i_grayImage, LandROI);

            vecCalcImage.push_back(imageCalcAverage);
        }

        Ipvm::Image8u TempMask(nLandSizeX, nLandSizeY);
        Ipvm::Image8u Temp(nLandSizeX, nLandSizeY);
        Ipvm::Image8u LandMask(nLandSizeX, nLandSizeY);

        Ipvm::Image8u SRMask_Ignore(nSR_SizeX, nSR_SizeY);
        Ipvm::Image8u SRMask_Dilate(nSR_SizeX, nSR_SizeY);

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(TempMask), 0, TempMask);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(Temp), 0, Temp);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(LandMask), 0, LandMask);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(SRMask_Ignore), 0, SRMask_Ignore);
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(SRMask_Dilate), 0, SRMask_Dilate);

        //평균 Image를 획득한다
        if (!GetAverageImageImage(vecCalcImage, TempMask))
            return FALSE;

        //Threshold 후 Blob하여 검은색 부분을 채운다.

        Ipvm::Image8u ThresholdImage(nLandSizeX, nLandSizeY);
        ThresholdImage.FillZero();

        unsigned char LowMean = 0;
        unsigned char HighMean = 0;
        unsigned char ThresholdValue = 0;

        Ipvm::ImageProcessing::GetThresholdOtsu(TempMask, Ipvm::Rect32s(TempMask), LowMean, HighMean, ThresholdValue);
        ThresholdValue = (unsigned char)(ThresholdValue / 1.5);
        CippModules::Binarize(TempMask, ThresholdImage, Ipvm::Rect32s(TempMask), ThresholdValue, FALSE);

        const long nMaxBlobNum = 100;
        std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);
        int32_t nCurBlobNum = nMaxBlobNum;

        Ipvm::Image32s imageLabel(nLandSizeX, nLandSizeY);

        if (m_pBlob->DoBlob(
                ThresholdImage, Ipvm::Rect32s(ThresholdImage), nMaxBlobNum, imageLabel, &vecObjBlob[0], nCurBlobNum)
                < 0
            || nCurBlobNum <= 0)
        {
            return FALSE;
        }

        long maxBlobIndex = 0;
        long maxBlobArea = 0;
        for (long i = 0; i < nCurBlobNum; i++)
        {
            if (vecObjBlob[i].m_area > maxBlobArea)
            {
                maxBlobArea = vecObjBlob[i].m_area;
                maxBlobIndex = i;
            }
        }

        for (long y = 0; y < nLandSizeY; y++)
        {
            auto* mask_y = ThresholdImage.GetMem(0, y);
            auto* label_y = imageLabel.GetMem(0, y);

            for (long x = 0; x < nLandSizeX; x++)
            {
                if (vecObjBlob[maxBlobIndex].m_label != label_y[x])
                {
                    mask_y[x] = 0;
                }
            }
        }

        Ipvm::Rect32s rtObject = vecObjBlob[maxBlobIndex].m_roi;
        //const long label = vecObjBlob[maxBlobIndex].m_label;

        vecGrayLandMasks.push_back(TempMask);
        vecLandMasks.push_back(ThresholdImage);

        long nErodeSize = 0;

        if (m_VisionPara.m_vecLandParameter[GroupID].m_fLandMaskErodeSize > 0)
        {
            nErodeSize = (long)(m_VisionPara.m_vecLandParameter[GroupID].m_fLandMaskErodeSize * fumtopx + 0.5f);

            Ipvm::ImageProcessing::MorphErode(
                ThresholdImage, Ipvm::Rect32s(ThresholdImage), nErodeSize, nErodeSize, true, LandMask);
        }
        else
        {
            Ipvm::ImageProcessing::Copy(ThresholdImage, Ipvm::Rect32s(ThresholdImage), LandMask);
        }

        Ipvm::Rect32s rtSRSearchROI(SRMask_Dilate);
        Ipvm::Rect32s rtThresholdImage(ThresholdImage);
        long m_CenterX = rtSRSearchROI.CenterPoint().m_x;
        long m_CenterY = rtSRSearchROI.CenterPoint().m_y;

        if (nLandSizeX > nSR_SizeX || nLandSizeY > nSR_SizeY)
            return false;

        //Ipvm::Rect32s rtSRarea(m_CenterX - (rtThresholdImage.Width() / 2.f) + 1, m_CenterY - (rtThresholdImage.Height() / 2.f) + 1, m_CenterX + (rtThresholdImage.Width() / 2.f), m_CenterY + (rtThresholdImage.Height() / 2.f));//kircheis_재확인필요
        Ipvm::Rect32s rtSRarea(
            rtThresholdImage); //kircheis_Bug20200918//원인은 모르겠는데-수정된 이력이 없음-  위의 Code에서 Rect의 크기가 달라진다. 이를 막기 위해 아래의 Code로 확장 시킴
        if (nLandSizeX != nSR_SizeX)
        {
            long nOffset = m_CenterX - (long)((float)nLandSizeX * .5f);
            rtSRarea.MoveLeftTo(nOffset);
        }
        if (nLandSizeY != nSR_SizeY)
        {
            long nOffset = m_CenterY - (long)((float)nLandSizeY * .5f);
            rtSRarea.MoveTopTo(nOffset);
        }

        Ipvm::Image8u ThresholdImageSR(nSR_SizeX, nSR_SizeY);

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(ThresholdImageSR), 0, ThresholdImageSR);

        Ipvm::ImageProcessing::Copy(ThresholdImage, Ipvm::Rect32s(ThresholdImage), rtSRarea, ThresholdImageSR);

        if (nSR_DilateSize > 0)
        {
            Ipvm::ImageProcessing::MorphDilate(
                ThresholdImageSR, Ipvm::Rect32s(ThresholdImageSR), nSR_DilateSize, nSR_DilateSize, true, SRMask_Dilate);
        }
        else
        {
            Ipvm::ImageProcessing::Copy(ThresholdImageSR, Ipvm::Rect32s(ThresholdImageSR), SRMask_Dilate);
        }

        if (nSR_IgnoreSize > 0)
        {
            Ipvm::ImageProcessing::MorphDilate(
                ThresholdImageSR, Ipvm::Rect32s(ThresholdImageSR), nSR_IgnoreSize, nSR_IgnoreSize, true, SRMask_Ignore);
        }
        else
        {
            Ipvm::ImageProcessing::Copy(ThresholdImageSR, Ipvm::Rect32s(ThresholdImageSR), SRMask_Ignore);
        }

        LandMasks[GroupID] = LandMask;

        //만들었다면 이제 붙여야지
        for (long idx = 0; idx < m_result.vecptObjectValidLandCenter[GroupID].size(); idx++)
        {
            Ipvm::Rect32s rtCopyDstROI;
            rtCopyDstROI.m_left = int32_t(
                m_result.vecptObjectValidLandCenter[GroupID][idx].m_x - (vecObjBlob[maxBlobIndex].m_roi.Width() / 2));
            rtCopyDstROI.m_top = int32_t(
                m_result.vecptObjectValidLandCenter[GroupID][idx].m_y - (vecObjBlob[maxBlobIndex].m_roi.Height() / 2));
            rtCopyDstROI.m_right = int32_t(rtCopyDstROI.m_left + vecObjBlob[maxBlobIndex].m_roi.Width());
            rtCopyDstROI.m_bottom = int32_t(rtCopyDstROI.m_top + vecObjBlob[maxBlobIndex].m_roi.Height());

            ImageCopyinMask(LandMask, rtCopyDstROI, vecObjBlob[maxBlobIndex].m_roi, maskImage);

            m_result.vecrtBlobObj[GroupID].push_back(rtCopyDstROI);

            Ipvm::Rect32s LandMaskROI(SRMask_Dilate);
            rtCopyDstROI.m_left
                = int32_t(m_result.vecptObjectValidLandCenter[GroupID][idx].m_x - (LandMaskROI.Width() / 2));
            rtCopyDstROI.m_top
                = int32_t(m_result.vecptObjectValidLandCenter[GroupID][idx].m_y - (LandMaskROI.Height() / 2));
            rtCopyDstROI.m_right = int32_t(rtCopyDstROI.m_left + LandMaskROI.Width());
            rtCopyDstROI.m_bottom = int32_t(rtCopyDstROI.m_top + LandMaskROI.Height());

            ImageCopyinMask(SRMask_Ignore, rtCopyDstROI, LandMaskROI, SRmaskImage_Ignore);
            ImageCopyinMask(SRMask_Dilate, rtCopyDstROI, LandMaskROI, SRmaskImage_Dilate);
        }
    }

    std::vector<Ipvm::Rect32s> vecDebugMaskROI;
    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
        vecDebugMaskROI.insert(
            vecDebugMaskROI.end(), m_result.vecrtBlobObj[GroupID].begin(), m_result.vecrtBlobObj[GroupID].end());
    SetDebugInfoItem(detailSetupMode, _T("Mask Blob ROI"), vecDebugMaskROI);

    long nDebugFullSizeX = 0;
    long nDebugFullSizeY = 0;

    for (long idx = 0; idx < vecLandMasks.size(); idx++)
    {
        Ipvm::Rect32s rtTemp = Ipvm::Rect32s(vecLandMasks[idx]);

        nDebugFullSizeX += rtTemp.m_right;
        nDebugFullSizeY = max(nDebugFullSizeY, rtTemp.m_bottom);
    }

    Ipvm::Image8u GrayMaskImage(nDebugFullSizeX, nDebugFullSizeY);
    Ipvm::Image8u ThresholdMaskImage(nDebugFullSizeX, nDebugFullSizeY);

    long PositionX = 0;

    for (long idx = 0; idx < vecLandMasks.size(); idx++)
    {
        Ipvm::Rect32s rtTemp = Ipvm::Rect32s(vecGrayLandMasks[idx]);
        rtTemp.m_left = PositionX;
        PositionX += rtTemp.m_right;
        rtTemp.m_right = PositionX;

        Ipvm::ImageProcessing::Copy(vecGrayLandMasks[idx], Ipvm::Rect32s(vecGrayLandMasks[idx]), rtTemp, GrayMaskImage);
        Ipvm::ImageProcessing::Copy(vecLandMasks[idx], Ipvm::Rect32s(vecLandMasks[idx]), rtTemp, ThresholdMaskImage);
    }

    SetDebugInfoItem(detailSetupMode, _T("Object Mask Image"), maskImage, true);
    SetDebugInfoItem(detailSetupMode, _T("Average Gray Mask Image"), GrayMaskImage);
    SetDebugInfoItem(detailSetupMode, _T("Average Mask Image"), ThresholdMaskImage);

    auto* alignMask = getReusableMemory().AddSurfaceLayerMaskClass(_T("Measured - Land"));
    if (alignMask)
    {
        alignMask->Reset();
        alignMask->Add(maskImage);
    }

    for (long groupIndex = 0; groupIndex < nGroupSize; groupIndex++)
    {
        Ipvm::Image8u layerMask;
        if (!getReusableMemory().GetInspByteImage(layerMask))
            return FALSE;

        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(tempImage), 0, tempImage);

        long nLandNum = (long)m_result.vecsfrtBlobLand[groupIndex].size();
        for (long nLand = 0; nLand < nLandNum; nLand++)
        {
            auto roi = m_result.vecsfrtBlobLand[groupIndex][nLand].GetSPI_RECT();
            roi.Offset(1, 1, FALSE);
            FillArbitraryRect(roi, 255, tempImage);
        }

        if (Ipvm::ImageProcessing::BitwiseAnd(maskImage, tempImage, Ipvm::Rect32s(maskImage), layerMask)
            != Ipvm::Status::e_ok)
        {
            return FALSE;
        }

        CString layerName;
        CString strGroupID = m_group_id_list[groupIndex];
        if (strGroupID.GetAt(0) == _T('G'))
            strGroupID.Delete(0);
        layerName.Format(_T("Measured - Land Group%s"), (LPCTSTR)strGroupID);
        auto* surfaceLayerMask = getReusableMemory().AddSurfaceLayerMaskClass(layerName);
        surfaceLayerMask->Reset();
        surfaceLayerMask->Add(layerMask);

        layerName.Empty();
        strGroupID.Empty();
    }

    //{{ SR Mask 생성.
    //float fSizeOffsetWidthRatioforSR = 0.2f; // 10%
    //float fSizeOffsetLengthRatioforSR = 0.2f; // 10%
    //std::vector<std::vector<PI_RECT>> vec2srtMaskLandforSR;
    //std::vector<std::vector<FPI_RECT>> vec2sfrtMaskLandforSR;
    //MakeLandMaskROI(m_result.vecptObjectValidLandCenter, vec2srtMaskLandforSR, vec2sfrtMaskLandforSR, nGroupSize, fSizeOffsetWidthRatioforSR, fSizeOffsetLengthRatioforSR);
    // }}

    float fSRSearchWidthRatio = m_VisionPara.m_fSRSearchExtRatio / 100.f;
    float fSRSearchLengthRatio = m_VisionPara.m_fSRSearchExtRatio / 100.f;
    std::vector<std::vector<PI_RECT>> vecsrtMeasureSR;
    RotateLandROI(m_result.vecptObjectValidLandCenter, vecsrtMeasureSR, m_result.vecsfrtSR, fSRSearchWidthRatio,
        fSRSearchLengthRatio);

    std::vector<FPI_RECT> DebugvecsrtMeasureSR;
    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
    {
        DebugvecsrtMeasureSR.insert(
            DebugvecsrtMeasureSR.end(), m_result.vecsfrtSR[GroupID].begin(), m_result.vecsfrtSR[GroupID].end());
    }
    SetDebugInfoItem(detailSetupMode, _T("SR Measure ROI"), DebugvecsrtMeasureSR);

    Ipvm::Image8u SRmaskImage;
    if (!getReusableMemory().GetInspByteImage(SRmaskImage))
        return FALSE;

    Ipvm::ImageProcessing::Subtract(SRmaskImage_Dilate, SRmaskImage_Ignore, Ipvm::Rect32s(rtPackage), 0, SRmaskImage);

    //{{//kircheis_IgnoreEdge
    Ipvm::Rect32s rtMaskFull;
    Ipvm::Image8u SrEdgeMaskImage;
    if (!getReusableMemory().GetInspByteImage(SrEdgeMaskImage))
        return FALSE;
    rtMaskFull = Ipvm::Rect32s(SrEdgeMaskImage);
    Ipvm::ImageProcessing::Fill(rtMaskFull, 0, SrEdgeMaskImage);
    Ipvm::Quadrangle32r qrtBody;
    GetSrIgnoreQRT(qrtBody);

    Ipvm::ImageProcessing::Fill(qrtBody, 255, SrEdgeMaskImage);
    Ipvm::ImageProcessing::BitwiseAnd(SrEdgeMaskImage, rtMaskFull, SRmaskImage);
    //}}

    SetDebugInfoItem(detailSetupMode, _T("SR Mask Image"), SRmaskImage, true);

    CalcZPos(maskImage, SRmaskImage);

    SetDebugInfoItem(detailSetupMode, _T("SR Selected Image"), SRmaskImage, true);

    return TRUE;
}

void VisionInspectionLgaBottom3D::CalcZPos(Ipvm::Image8u i_maskImage, Ipvm::Image8u i_SRmaskImage)
{
    long nGroupSize = (long)m_vec2LandData.size();
    // SR 및 Land 높이 계산.
    std::vector<Ipvm::Rect32s> vecrtSerch;

    const auto& zmap = getImageLotInsp().m_zmapImage;
    const auto& vmap = getImageLotInsp().m_vmapImage;

    m_result.vec3DSR.resize(m_vecLandData.size());
    m_result.vec3DObj.resize(m_vecLandData.size());

    //------------------------------------------------------------------------------
    // Mask 이미지에서 ZMap값이 없는 부분도 미리 제거해 놓자.
    // 뒤에서 Mask 체크하면 괜히 ZMap 유효한지 두번씩 체크하지 말자.
    //------------------------------------------------------------------------------
    //mc_Test중이라 잠깐 묶어놓는다 시간이 너무 많이 걸려 ..

    float boundScale = 65535 / 255.f;
    USHORT lowerBound = (USHORT)min(USHRT_MAX, max(0, m_VisionPara.m_SR_visibilityLowerBound * boundScale));

    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
    {
        for (auto& laneInfo : m_result.vecsfrtSR[GroupID])
        {
            Ipvm::Rect32s roi = laneInfo.GetExtCRect();
            remove_noise_point(zmap, roi, i_SRmaskImage);
            remove_lowVisibility_point(zmap, vmap, roi, lowerBound, i_SRmaskImage);
        }

        for (auto& roi_original : m_result.vecrtBlobObj[GroupID])
        {
            Ipvm::Rect32s roi = roi_original;
            remove_noise_point(zmap, roi, i_maskImage);
        }

        //------------------------------------------------------------------------------

        for (long nLandSR = 0; nLandSR < (long)m_result.vecsfrtSR[GroupID].size(); nLandSR++)
        {
            float fSRZPos = 0.f;
            Ipvm::Rect32s rtSearchSR = m_result.vecsfrtSR[GroupID][nLandSR].GetExtCRect();

            CalcZPosSr(zmap, i_SRmaskImage, rtSearchSR, fSRZPos);
            m_result.vec3DSR[m_vec2LandData[GroupID][nLandSR].nIndex]
                = Ipvm::Point32r3(m_result.vecptObjectValidLandCenter[GroupID][nLandSR].m_x,
                    m_result.vecptObjectValidLandCenter[GroupID][nLandSR].m_y, fSRZPos);
        }

        for (long nLand = 0; nLand < (long)m_result.vecrtBlobObj[GroupID].size(); nLand++)
        {
            float fLandZPos(0.f);
            Ipvm::Rect32s rtSearchLand = m_result.vecrtBlobObj[GroupID][nLand];

            CalcZPosLand(zmap, i_maskImage, rtSearchLand, fLandZPos);
            m_result.vec3DObj[m_vec2LandData[GroupID][nLand].nIndex]
                = Ipvm::Point32r3(m_result.vecptObjectValidLandCenter[GroupID][nLand].m_x,
                    m_result.vecptObjectValidLandCenter[GroupID][nLand].m_y, fLandZPos);
        }
    }
}

BOOL VisionInspectionLgaBottom3D::DoAlign(const bool detailSetupMode)
{
    if (m_pVisionInspDlg != NULL && detailSetupMode)
    {
        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    if (!MakePreInspInfo(detailSetupMode))
        return FALSE;

    long nGroupSize = (long)m_group_id_list.size();

    // Object Spec Point
    Ipvm::Point32r2 fptSpecGravityCenter = Ipvm::Point32r2(0.f, 0.f);
    if (!MakeSpecObject_ByPKGSize(m_result.sfrtPackageBody, m_result.vecfptSpecLand, fptSpecGravityCenter))
        return FALSE;

    std::vector<Ipvm::Point32r2> vecDebugfptSpecLand;
    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
        vecDebugfptSpecLand.insert(vecDebugfptSpecLand.end(), m_result.vecfptSpecLand[GroupID].begin(),
            m_result.vecfptSpecLand[GroupID].end());
    SetDebugInfoItem(detailSetupMode, _T("Object Spec Point"), vecDebugfptSpecLand);

    // Object Spec ROI
    std::vector<std::vector<PI_RECT>> vecsrtSpecLandROI;
    if (!RotateLandROI(m_result.vecfptSpecLand, vecsrtSpecLandROI, m_result.vecsfrtSpecLand))
        return FALSE;

    std::vector<PI_RECT> vecDebugsrtSpecLandROI;
    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
    {
        vecDebugsrtSpecLandROI.insert(
            vecDebugsrtSpecLandROI.end(), vecsrtSpecLandROI[GroupID].begin(), vecsrtSpecLandROI[GroupID].end());
    }
    SetDebugInfoItem(detailSetupMode, _T("Object Spec ROI"), vecDebugsrtSpecLandROI);

    // Object Land Search ROI
    std::vector<std::vector<Ipvm::Rect32s>> vecrtSearchLandROI;
    MakeLandSearchROI(m_result.vecsfrtSpecLand, vecrtSearchLandROI);

    std::vector<Ipvm::Rect32s> vecDebugrtSearchLandROI;
    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
        vecDebugrtSearchLandROI.insert(
            vecDebugrtSearchLandROI.end(), vecrtSearchLandROI[GroupID].begin(), vecrtSearchLandROI[GroupID].end());
    SetDebugInfoItem(detailSetupMode, _T("Object Search ROI"), vecDebugrtSearchLandROI);

    //FindLand
    Ipvm::Image8u binImage;
    Ipvm::Image8u grayImage;

    if (!getReusableMemory().GetInspByteImage(binImage))
        return FALSE;
    if (!getReusableMemory().GetInspByteImage(grayImage))
        return FALSE;

    //초기화
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(binImage), 0, binImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(grayImage), 0, grayImage);

    Ipvm::Point32r2 fptGravityCenter = Ipvm::Point32r2(0.f, 0.f);
    std::vector<std::vector<Ipvm::Point32r2>> vecfptObjectBlob;
    std::vector<std::vector<Ipvm::Rect32s>> vecrtBlobObj;

    if (!FindObjectBlob2(binImage, grayImage, m_result.vecsfrtSpecLand, vecrtSearchLandROI, vecrtBlobObj,
            vecfptObjectBlob, fptGravityCenter))
        return FALSE;

    SetDebugInfoItem(detailSetupMode, _T("Gray Image"), grayImage);
    SetDebugInfoItem(detailSetupMode, _T("Binary Image"), binImage);

    std::vector<Ipvm::Point32r2> vecDebugrtBlobLandPoint;
    std::vector<Ipvm::Rect32s> vecDebugrtBlobLandROI;
    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
    {
        vecDebugrtBlobLandPoint.insert(
            vecDebugrtBlobLandPoint.end(), vecfptObjectBlob[GroupID].begin(), vecfptObjectBlob[GroupID].end());
        vecDebugrtBlobLandROI.insert(
            vecDebugrtBlobLandROI.end(), vecrtBlobObj[GroupID].begin(), vecrtBlobObj[GroupID].end());
    }
    SetDebugInfoItem(detailSetupMode, _T("Object Blob Point"), vecDebugrtBlobLandPoint);
    SetDebugInfoItem(detailSetupMode, _T("Object Blob ROI"), vecDebugrtBlobLandROI);

    std::vector<std::vector<Ipvm::Point32r2>> specLandPositions_px;

    specLandPositions_px.clear();
    specLandPositions_px.resize(nGroupSize);

    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
    {
        for (const auto& Land : m_vec2LandData[GroupID])
        {
            Ipvm::Point32r2 fptSpecLandPos((float)Land.fOffsetX, (float)Land.fOffsetY);
            specLandPositions_px[GroupID].emplace_back(fptSpecLandPos);
        }
    }

    std::vector<std::vector<Ipvm::Point32r2>> vecptObjectLandCenter;
    std::vector<std::vector<Ipvm::Point32r2>> vecptObjectValidLandCenter;
    std::vector<std::vector<Ipvm::Rect32s>> vecrtObjectLandROI;
    if (!AlignLand(specLandPositions_px, vecfptObjectBlob, vecptObjectLandCenter, vecptObjectValidLandCenter,
            vecrtObjectLandROI))
        return FALSE;

    vecrtObjectLandROI.clear();
    //1. 경향성 비교 (Rough Bolb Center vs Spec Center)를 통한 Center 보정
    if (!AlignLand(specLandPositions_px, vecptObjectValidLandCenter, m_result.vecptObjectLandCenter,
            m_result.vecptObjectValidLandCenter, vecrtObjectLandROI))
        return FALSE;

    std::vector<std::vector<Ipvm::Point32r2>> vecfptLandCenter = m_result.vecptObjectValidLandCenter;
    std::vector<std::vector<PI_RECT>> vecsrtLand;
    RotateLandROI(vecfptLandCenter, vecsrtLand, m_result.vecsfrtBlobLand);
    std::vector<Ipvm::Point32r2> vecDebugfptLandCenter;
    std::vector<PI_RECT> vecDebugsrtLand;
    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
    {
        vecDebugfptLandCenter.insert(
            vecDebugfptLandCenter.end(), vecfptLandCenter[GroupID].begin(), vecfptLandCenter[GroupID].end());
        vecDebugsrtLand.insert(vecDebugsrtLand.end(), vecsrtLand[GroupID].begin(), vecsrtLand[GroupID].end());
    }

    SetDebugInfoItem(detailSetupMode, _T("Object Point"), vecDebugfptLandCenter);
    SetDebugInfoItem(detailSetupMode, _T("Object ROI"), vecDebugsrtLand);

    if (!MakeLandMask(binImage, grayImage, detailSetupMode))
        return FALSE;

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::DoInsp(const bool detailSetupMode)
{
    BOOL bResult = TRUE;

    long nGroupSize = (long)m_group_id_list.size();

    m_result.vecrtLand.resize(m_vecLandData.size());
    m_result.vecrtSR.resize(m_vecLandData.size());

    for (long GroupID = 0; GroupID < nGroupSize; GroupID++)
    {
        for (long nidx = 0; nidx < m_result.vecsfrtBlobLand[GroupID].size(); nidx++)
            m_result.vecrtLand[m_vec2LandData[GroupID][nidx].nIndex]
                = m_result.vecsfrtBlobLand[GroupID][nidx].GetRoughRect();

        for (long nidx = 0; nidx < m_result.vecsfrtSR[GroupID].size(); nidx++)
            m_result.vecrtSR[m_vec2LandData[GroupID][nidx].nIndex] = m_result.vecsfrtSR[GroupID][nidx].GetRoughRect();
    }

    //{{//kircheis_MED2LW
    const bool bUseLandCopl = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_COPL].m_use;
    const bool bUseLandUnitCopl = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_UNIT_COPL].m_use;
    const bool bUseLandHeight = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_HEIGHT].m_use;
    const bool bUseLandWarpage = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_WARPAGE].m_use;
    const bool bUseLandUnitWarpage = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_UNIT_WARPAGE].m_use;
    const bool bUseLandUnitWarpageAX = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AX].m_use;
    const bool bUseLandUnitWarpageAV = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AV].m_use;
    const bool bUseLandUnitWarpageAS = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AS].m_use;
    const bool bUseLandUnitWarpageAO = m_fixedInspectionSpecs[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AO].m_use;
    //}}
    bool buseLandCoplItem = false;

    for (auto spec : m_variableInspectionSpecs)
    {
        buseLandCoplItem |= (bool)spec.m_use;
    }

    if (bUseLandCopl || bUseLandUnitCopl || buseLandCoplItem)
        bResult &= InspLandCoplanarity(m_result.vec3DObj, m_result.vecrtLand);

    if (bUseLandHeight)
        bResult &= InspLandHeight(m_result.vecrtLand, m_result.vec3DObj, m_result.vec3DSR);

    if (bUseLandWarpage || bUseLandUnitWarpage || bUseLandUnitWarpageAX || bUseLandUnitWarpageAV
        || bUseLandUnitWarpageAS || bUseLandUnitWarpageAO)
        bResult &= InspWarpage(m_result.vec3DSR, m_result.vecrtSR, detailSetupMode);

    return bResult;
}

BOOL VisionInspectionLgaBottom3D::MakePreInspInfo(const bool detailSetupMode)
{
    float fAngle = 0.f;
    Ipvm::Rect32r frtBody;
    Ipvm::Point32r2 fptBodyCenter;
    if (!GetBodyAlignInfo(fAngle, frtBody, fptBodyCenter))
        return FALSE;

    SetDebugInfoItem(detailSetupMode, _T("PACKAGE BODY ROI"), m_result.sfrtPackageBody);

    m_fAngle = fAngle * ITP_RAD_TO_DEG;

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::MakeSpecObject_ByPKGSize(FPI_RECT i_sfrtPackageBody,
    std::vector<std::vector<Ipvm::Point32r2>>& o_vecfptObjectSpec, Ipvm::Point32r2& o_fptGravityCenter)
{
    /////============[Gerber와 Image상의 Coefficient 구하기]===============///////
    /// Xi = a0 + a1*xr + a2*yr + a3*xr*yr
    /// Yi = b0 + b1*xr + b2*yr + a3*xr*yr

    /// Image 좌표계에서 찾아진 4개의 꼭지점
    Ipvm::Point32r2 pfptImgBody[4] = {
        Ipvm::Point32r2(0.f, 0.f),
    };
    pfptImgBody[0] = i_sfrtPackageBody.fptLT;
    pfptImgBody[1] = i_sfrtPackageBody.fptRT;
    pfptImgBody[2] = i_sfrtPackageBody.fptRB;
    pfptImgBody[3] = i_sfrtPackageBody.fptLB;

    /// 실제 좌표계에서 Packge Center를 기준으로 하는 4개의 꼭지점.
    /// 실제 좌표계에서 Packge Center를 기준으로 하는 4개의 꼭지점. 원래는 Body Size 정보를 사용하였으나, 문제가 있어 Align된 Size를 사용한다.
    PI_RECT srtPackageBody(
        i_sfrtPackageBody.fptLT, i_sfrtPackageBody.fptRT, i_sfrtPackageBody.fptLB, i_sfrtPackageBody.fptRB);
    float sizeMinX(0.f), sizeMaxX(0.f), sizeAvrX(0.f), sizeMinY(0.f), sizeMaxY(0.f), sizeAvrY(0.f);
    CPI_Geometry::GetRectSizeX(srtPackageBody, true, sizeMinX, sizeMaxX, sizeAvrX);
    CPI_Geometry::GetRectSizeY(srtPackageBody, true, sizeMinY, sizeMaxY, sizeAvrY);

    double dszX = (double)sizeMaxX;
    double dszY = (double)sizeMaxY;
    Ipvm::Point32r2 pfptRealBody[4] = {
        Ipvm::Point32r2(0.f, 0.f),
    };
    pfptRealBody[0] = Ipvm::Point32r2(CAST_FLOAT(-dszX / 2.0f), CAST_FLOAT(-dszY / 2.0f));
    pfptRealBody[1] = Ipvm::Point32r2(CAST_FLOAT(+dszX / 2.0f), CAST_FLOAT(-dszY / 2.0f));
    pfptRealBody[2] = Ipvm::Point32r2(CAST_FLOAT(+dszX / 2.0f), CAST_FLOAT(+dszY / 2.0f));
    pfptRealBody[3] = Ipvm::Point32r2(CAST_FLOAT(-dszX / 2.0f), CAST_FLOAT(+dszY / 2.0f));

    /// 실좌표와 Image좌표간의 방정식 계수를 구한다. (Polynomial warpping 또는 Bilinear transform을 참고하라)
    double bilinearTransform[2][4] = {
        0.,
    };

    if (Ipvm::Status::e_ok != Ipvm::Geometry::GetBilinearTransform(pfptRealBody, pfptImgBody, 4, bilinearTransform))
        return FALSE;

    const auto& mmToPixel = getScale().mmToPixel();

    o_vecfptObjectSpec.clear();
    o_vecfptObjectSpec.resize(m_group_id_list.size());

    for (int GroupID = 0; GroupID < m_group_id_list.size(); GroupID++)
    {
        long nDataCount(0);

        for (auto LandMapData : m_vec2LandData[GroupID])
        {
            float fX = LandMapData.fOffsetX * mmToPixel.m_x;
            float fY = (LandMapData.fOffsetY * mmToPixel.m_y) * -1.f;

            o_vecfptObjectSpec[GroupID].push_back(
                Ipvm::Geometry::TransformBilinear(Ipvm::Point32r2(fX, fY), bilinearTransform));

            o_fptGravityCenter.m_x += o_vecfptObjectSpec[GroupID][nDataCount].m_x;
            o_fptGravityCenter.m_y += o_vecfptObjectSpec[GroupID][nDataCount].m_y;

            nDataCount++;
        }
    }

    long nPointNum = (long)m_vecLandData.size();

    o_fptGravityCenter.m_x /= (float)nPointNum;
    o_fptGravityCenter.m_y /= (float)nPointNum;

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::RotateLandROI(const std::vector<std::vector<Ipvm::Point32r2>>& i_vecfptSpecLand,
    std::vector<std::vector<PI_RECT>>& o_vecpirtSpecLand, std::vector<std::vector<FPI_RECT>>& o_vecfpirtSpecLand,
    float i_fSizeOffsetWidthRatio, float i_fSizeOffsetLengthRatio)
{
    if (m_vec2LandData.size() <= 0)
        return FALSE;

    float mmToPixel = (getScale().mmToPixel().m_x + getScale().mmToPixel().m_y) / 2.f;

    o_vecfpirtSpecLand.clear();
    o_vecfpirtSpecLand.resize(m_group_id_list.size());
    o_vecpirtSpecLand.clear();
    o_vecpirtSpecLand.resize(m_group_id_list.size());

    for (long GroupID = 0; GroupID < m_group_id_list.size(); GroupID++)
    {
        long nDataCount(0);
        for (auto LandMapData : m_vec2LandData[GroupID])
        {
            float fSizeOffsetWidth = LandMapData.fWidth * i_fSizeOffsetWidthRatio;
            float fSizeOffsetLength = LandMapData.fLength * i_fSizeOffsetLengthRatio;
            float fLandHalfWidth = (LandMapData.fWidth + fSizeOffsetWidth) * mmToPixel / 2.f;
            float fLandHalfLength = (LandMapData.fLength + fSizeOffsetLength) * mmToPixel / 2.f;
            float fLandAngle = (float)LandMapData.nAngle;

            FPI_RECT sfrtLand;
            Ipvm::Point32r2 fCenter = i_vecfptSpecLand[GroupID][nDataCount];
            Ipvm::Rect32r frtROI;
            frtROI.m_left = (fCenter.m_x - fLandHalfWidth);
            frtROI.m_top = (fCenter.m_y - fLandHalfLength);
            frtROI.m_right = (fCenter.m_x + fLandHalfWidth);
            frtROI.m_bottom = (fCenter.m_y + fLandHalfLength);

            sfrtLand
                = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
                    Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

            if (fLandAngle != 0)
            {
                float fAngle = fLandAngle * ITP_DEG_TO_RAD;

                // 시계 반대 방향이 +
                sfrtLand = sfrtLand.Rotate(fAngle);
            }

            o_vecpirtSpecLand[GroupID].push_back(sfrtLand.GetSPI_RECT());
            o_vecfpirtSpecLand[GroupID].push_back(sfrtLand);

            nDataCount++;
        }
    }
    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::MakeLandSearchROI(
    std::vector<std::vector<FPI_RECT>> i_vecfpirtSpecLand, std::vector<std::vector<Ipvm::Rect32s>>& o_vecrtSearchLand)
{
    if (i_vecfpirtSpecLand.size() <= 0)
        return FALSE;

    const auto& mmToPixel = getScale().mmToPixel();

    o_vecrtSearchLand.clear();
    o_vecrtSearchLand.resize(m_group_id_list.size());

    for (long GroupID = 0; GroupID < long(m_group_id_list.size()); GroupID++)
    {
        long nObjSearchX
            = static_cast<long>(m_VisionPara.m_vecLandParameter[GroupID].m_fObjSearchX / 2.f * mmToPixel.m_x + .5f);
        long nObjSearchY
            = static_cast<long>(m_VisionPara.m_vecLandParameter[GroupID].m_fObjSearchY / 2.f * mmToPixel.m_y + .5f);

        for (const auto& LandSearchROI : i_vecfpirtSpecLand[GroupID])
        {
            Ipvm::Rect32s rtSearchLand = LandSearchROI.GetRoughRect();
            rtSearchLand.InflateRect(nObjSearchX, nObjSearchY);
            o_vecrtSearchLand[GroupID].push_back(rtSearchLand);
        }
    }
    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::FindObjectBlob2(Ipvm::Image8u& binImage, Ipvm::Image8u& grayImage,
    std::vector<std::vector<FPI_RECT>> i_vecfrtSpecLandROI, std::vector<std::vector<Ipvm::Rect32s>>& i_vecrtObjSearch,
    std::vector<std::vector<Ipvm::Rect32s>>& o_vecrtObj, std::vector<std::vector<Ipvm::Point32r2>>& o_vecfptObjCenter,
    Ipvm::Point32r2& o_fptGravityCenter)
{
    if (binImage.GetMem() == NULL || grayImage.GetMem() == NULL)
        return FALSE;

    if (i_vecfrtSpecLandROI.size() <= 0) //SpecLand가 없다면 진행하면 안되지
        return FALSE;

    float maxHeightFactor = 0.4f; // 0.8f

    const auto& zmap = getImageLotInsp().m_zmapImage;
    Ipvm::Rect32s rtPackage = m_result.sfrtPackageBody.GetExtCRect();

    Ipvm::Image64r integralImage;
    Ipvm::Image32s sumCountImage;

    if (!getReusableMemory().GetInspDoubleImage(integralImage))
        return FALSE;
    if (!getReusableMemory().GetInspLongImage(sumCountImage))
        return FALSE;

    o_vecrtObj.clear();
    o_vecrtObj.resize(m_group_id_list.size());
    o_vecfptObjCenter.clear();
    o_vecfptObjCenter.resize(m_group_id_list.size());
    m_result.vecptObjectValidLand.clear();
    m_result.vecptObjectValidLand.resize(m_group_id_list.size());

    float maxLandLengthX_px(0.f);
    float maxLandLengthY_px(0.f);
    float maxHeight_um(-FLT_MAX);
    float minHeight_um(FLT_MAX);
    float maxLandWidthX_px(0.f);
    float maxLandWidthY_px(0.f);

    // Resize을 안한다고 가정.
    const auto& mmToPixel = getScale().mmToPixel();

    for (auto LandData : m_vecLandData)
    {
        //const float LandLengthX_px = LandData.fWidth * mmToPixel.m_x;
        // AzureWave Package에서는 문제가 없었지만, Intel 자재에서는 직사각 형태의 Land가 있기 때문에 BlobMax Area Spec이 타이트해지는 문제가 발생하여 원복함 - JHB_2023.06.27
        const float LandLengthX_px = LandData.fLength * mmToPixel.m_x;
        if (LandLengthX_px > maxLandLengthX_px)
            maxLandLengthX_px = LandLengthX_px;

        const float LandLengthY_px = LandData.fLength * mmToPixel.m_y;
        if (LandLengthY_px > maxLandLengthY_px)
            maxLandLengthY_px = LandLengthY_px;

        const float Height_um = CAST_FLOAT(maxHeightFactor * fabs(LandData.fHeight * 1000.f));
        if (Height_um > maxHeight_um)
            maxHeight_um = Height_um;
        if (Height_um < minHeight_um)
            minHeight_um = Height_um;

        const float LandWidthX_px = LandData.fWidth * mmToPixel.m_x;
        if (LandWidthX_px > maxLandWidthX_px)
            maxLandWidthX_px = LandWidthX_px;

        const float LandWidthY_px = LandData.fWidth * mmToPixel.m_y;
        if (LandWidthY_px > maxLandWidthY_px)
            maxLandWidthY_px = LandWidthY_px;
    }

    Ipvm::Size32s2 windowSize(
        long(1.5f * (float)maxLandLengthX_px + 0.5f), long(1.5f * (float)maxLandLengthY_px + 0.5f));

    Ipvm::Image32r FirstbackZmap;
    if (!getReusableMemory().GetInspFloatImage(FirstbackZmap))
        return FALSE;

    if (Ipvm::ImageProcessing::IntegralInclusiveWithNoise(
            zmap, rtPackage, Ipvm::k_noiseValue32r, integralImage, sumCountImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (Ipvm::ImageProcessing::MakeBackgroundImageWithNoise(
            integralImage, sumCountImage, rtPackage, windowSize, Ipvm::k_noiseValue32r, FirstbackZmap)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    Ipvm::Image8u temp_back;
    if (!getReusableMemory().GetInspByteImage(temp_back))
        return FALSE;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(temp_back), 0, temp_back);

    for (long y = rtPackage.m_top; y < rtPackage.m_bottom; y++)
    {
        auto* zmap_y = zmap.GetMem(0, y);
        auto* back_y = FirstbackZmap.GetMem(0, y);
        BYTE* mask_y = temp_back.GetMem(0, y);

        for (long x = rtPackage.m_left; x < rtPackage.m_right; x++)
        {
            if (zmap_y[x] != Ipvm::k_noiseValue32r && back_y[x] != Ipvm::k_noiseValue32r)
            {
                if (zmap_y[x] >= back_y[x])
                {
                    mask_y[x] = 255;
                }
                else
                {
                    mask_y[x] = 0;
                }
            }
            else
            {
                mask_y[x] = 0;
            }
        }
    }

    integralImage.FillZero();
    sumCountImage.FillZero();
    if (!getReusableMemory().GetInspDoubleImage(integralImage))
        return FALSE;
    if (!getReusableMemory().GetInspLongImage(sumCountImage))
        return FALSE;

    Ipvm::Image32r LastbackZmap;
    if (!getReusableMemory().GetInspFloatImage(LastbackZmap))
        return FALSE;

    if (Ipvm::ImageProcessing::IntegralInclusive(zmap, temp_back, rtPackage, integralImage, sumCountImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (Ipvm::ImageProcessing::MakeBackgroundImageWithNoise(
            integralImage, sumCountImage, rtPackage, windowSize, Ipvm::k_noiseValue32r, LastbackZmap)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    for (long y = rtPackage.m_top; y < rtPackage.m_bottom; y++)
    {
        auto* zmap_y = zmap.GetMem(0, y);
        auto* back_y = LastbackZmap.GetMem(0, y);
        BYTE* gray_y = grayImage.GetMem(0, y);

        for (long x = rtPackage.m_left; x < rtPackage.m_right; x++)
        {
            if (zmap_y[x] != Ipvm::k_noiseValue32r && back_y[x] != Ipvm::k_noiseValue32r)
            {
                const float diff = back_y[x] - zmap_y[x];

                if (diff >= maxHeight_um)
                {
                    gray_y[x] = 255;
                }
                else if (diff <= 0.f)
                {
                    gray_y[x] = 0;
                }
                else
                {
                    gray_y[x] = (BYTE)(diff / maxHeight_um * 255.f);
                }
            }
            else
            {
                gray_y[x] = 0;
            }
        }
    }

    //Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(temp_back), 0, temp_back);

    //Ipvm::ImageProcessing::Scale(LastbackZmap, Ipvm::Rect32s(LastbackZmap), Ipvm::k_noiseValue32r, minval, maxval, temp_back);

    //Ipvm::Image8u(temp_back, i_vecrtObjSearch[0][1800]).SaveBmp(_T("D:\\LastBack.bmp"));

    //Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(temp_back), 0, temp_back);

    //Ipvm::ImageProcessing::Scale(zmap, Ipvm::Rect32s(zmap), Ipvm::k_noiseValue32r, z_minval, z_maxval, temp_back);

    //Ipvm::Image8u(temp_back, i_vecrtObjSearch[0][1800]).SaveBmp(_T("D:\\zmap.bmp"));

    //Ipvm::ImageProcessing::Scale(getImageLot().m_vmapImage, Ipvm::Rect32s(getImageLot().m_vmapImage), backgroundImage);

    //Ipvm::ImageProcessing::Dilate(getImageLot().m_vecImages[1], Ipvm::Rect32s(getImageLot().m_vecImages[1]), 1, 15, true, true, grayImage);
    //Ipvm::ImageProcessing::Erode(grayImage, Ipvm::Rect32s(grayImage), 1, 15, true, true, backgroundImage);
    ////Ipvm::ImageProcessing::Dilate(backgroundImage, Ipvm::Rect32s(backgroundImage), 15, 1, true, true, grayImage);
    ////Ipvm::ImageProcessing::Erode(grayImage, Ipvm::Rect32s(grayImage), 15, 1, true, true, backgroundImage);
    //Ipvm::ImageProcessing::MedianFilter(backgroundImage, Ipvm::Rect32s(backgroundImage), 3, 3, true, true, grayImage);

    //Ipvm::ImageProcessing::MedianFilter(backgroundImage, Ipvm::Rect32s(backgroundImage), 5, 5, true, true, grayImage);
    //Ipvm::ImageProcessing::Copy(getImageLot().m_vecImages[1], Ipvm::Rect32s(getImageLot().m_vecImages[1]), grayImage);

    Ipvm::Image8u BlobSearchImage;
    getReusableMemory().GetInspByteImage(BlobSearchImage);
    Ipvm::Image8u tempImage;
    getReusableMemory().GetInspByteImage(tempImage);
    Ipvm::Image8u temp_binImage;
    getReusableMemory().GetInspByteImage(temp_binImage);

    Ipvm::Point32r2 fPackageBodyCenter = m_result.sfrtPackageBody.GetCenter();
    const long nMaxBlobNum = 100;
    long nPointNum = 0;

    Ipvm::ImageProcessing::Fill(rtPackage, 255, binImage);

    if (m_VisionPara.m_nBlobSearchImage == 0)
    {
        Ipvm::ImageProcessing::Copy(grayImage, Ipvm::Rect32s(grayImage), BlobSearchImage);
    }
    else
    {
        Ipvm::ImageProcessing::Copy(getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][1],
            Ipvm::Rect32s(getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][1]), BlobSearchImage);
    }

    for (long GroupID = 0; GroupID < long(m_group_id_list.size()); GroupID++)
    {
        for (auto rtLandSearchROI : i_vecrtObjSearch[GroupID])
        {
            rtLandSearchROI.InflateRect(5, 5);

            BYTE nLowMean(0);
            BYTE nHighMean(0);
            BYTE nThreshold(0);
            Ipvm::ImageProcessing::GetThresholdOtsu(BlobSearchImage, rtLandSearchROI, nLowMean, nHighMean, nThreshold);

            CippModules::Binarize(
                BlobSearchImage, temp_binImage, rtLandSearchROI, nThreshold, m_VisionPara.isReverseThreshold);

            Ipvm::ImageProcessing::MorphErode(temp_binImage, rtLandSearchROI, 5, 5, true, tempImage);
            Ipvm::ImageProcessing::MorphDilate(tempImage, rtLandSearchROI, 5, 5, true, temp_binImage);

            rtLandSearchROI.DeflateRect(5, 5);

            Ipvm::ImageProcessing::BitwiseAnd(temp_binImage, rtLandSearchROI, binImage);
        }
    }

    for (long GroupID = 0; GroupID < long(m_group_id_list.size()); GroupID++)
    {
        long nLandidx(0);
        nPointNum += (long)i_vecrtObjSearch[GroupID].size();
        for (auto rtLandSearchROI : i_vecrtObjSearch[GroupID])
        {
            //BYTE nLowMean(0);
            //BYTE nHighMean(0);
            //BYTE nThreshold(0);
            //Ipvm::ImageProcessing::GetThresholdOtsu(grayImage, rtLandSearchROI, nLowMean, nHighMean, nThreshold);

            //CippModules::IppThreshold(grayImage, backgroundImage, rtLandSearchROI, nThreshold, TRUE);

            //Ipvm::ImageProcessing::Erode(backgroundImage, rtLandSearchROI, 5, 5, true, true, tempImage);
            //Ipvm::ImageProcessing::Dilate(tempImage, rtLandSearchROI, 5, 5, true, true, backgroundImage);

            std::vector<Ipvm::BlobInfo> vecObjBlob(nMaxBlobNum);
            int32_t nCurBlobNum = nMaxBlobNum;

            Ipvm::Image32s imageLabel;

            getReusableMemory().GetInspLongImage(imageLabel);

            if (m_pBlob->DoBlob(binImage, rtLandSearchROI, nMaxBlobNum, imageLabel, &vecObjBlob[0], nCurBlobNum) < 0
                || nCurBlobNum <= 0)
            {
                auto saveROI
                    = Ipvm::Rect32s(getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][1]) & rtLandSearchROI;
                if (!saveROI.IsRectEmpty())
                {
                    Ipvm::ImageFile::SaveAsBmp(
                        Ipvm::Image8u(getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][1], saveROI),
                        _T("D:\\Temp\\GraySearchImage.bmp"));

                    Ipvm::ImageFile::SaveAsBmp(Ipvm::Image8u(binImage, saveROI), _T("D:\\Temp\\BinObjImage.bmp"));
                }

                o_vecfptObjCenter[GroupID].push_back(
                    Ipvm::Point32r2(0.5f * (rtLandSearchROI.m_left + rtLandSearchROI.m_right - 1),
                        0.5f * (rtLandSearchROI.m_top + rtLandSearchROI.m_bottom - 1)));
                m_result.vecptObjectValidLand[GroupID].push_back(false);
                continue;
            }

            float fCurLandWidth_X_px = m_vec2LandData[GroupID][nLandidx].fWidth * mmToPixel.m_x;
            float fCurLandWidth_Y_px = m_vec2LandData[GroupID][nLandidx].fWidth * mmToPixel.m_y;
            float fCurLandLength_X_px = m_vec2LandData[GroupID][nLandidx].fLength * mmToPixel.m_x;
            float fCurLandLength_Y_px = m_vec2LandData[GroupID][nLandidx].fLength * mmToPixel.m_y;

            long maxBlobIndex = -1;
            //long maxBlobArea = (long)(((maxLandLengthX_px + maxLandLengthY_px) / 2) * ((maxLandWidthX_px + maxLandWidthY_px) / 2) / 2);
            long maxBlobArea = (long)(((fCurLandLength_X_px + fCurLandLength_Y_px) / 2)
                * ((fCurLandWidth_X_px + fCurLandWidth_Y_px) / 2) / 2);
            long maximumArea = (long)(maxLandLengthX_px
                * maxLandWidthY_px); //(수정고려 및 필요)mc_23.05.03 Maximum AreaSize에 Margin 10%정도는 주어야 할거 같다 너무 타이트 하다

            long DebugBlobIndex = -1;
            long DebugBlobArea = 0;
            for (long i = 0; i < nCurBlobNum; i++)
            {
                if (vecObjBlob[i].m_roi.PtInRect(rtLandSearchROI.CenterPoint()) && vecObjBlob[i].m_area > maxBlobArea
                    && vecObjBlob[i].m_area < maximumArea)
                {
                    maxBlobArea = vecObjBlob[i].m_area;
                    maxBlobIndex = i;
                }
                else if (vecObjBlob[i].m_area > DebugBlobArea)
                {
                    DebugBlobArea = vecObjBlob[i].m_area;
                    DebugBlobIndex = i;
                }
            }

            Ipvm::Rect32s rtObject;
            long label;

            if (maxBlobIndex == -1)
            {
                o_vecrtObj[GroupID].push_back(vecObjBlob[DebugBlobIndex].m_roi);
                o_vecfptObjCenter[GroupID].push_back(
                    Ipvm::Point32r2(0.5f * (rtLandSearchROI.m_left + rtLandSearchROI.m_right - 1),
                        0.5f * (rtLandSearchROI.m_top + rtLandSearchROI.m_bottom - 1)));
                m_result.vecptObjectValidLand[GroupID].push_back(false);
                continue;
            }
            else
            {
                rtObject = vecObjBlob[maxBlobIndex].m_roi;
                label = vecObjBlob[maxBlobIndex].m_label;
            }

            Ipvm::Point32r2 pt = {0, 0};
            if (!MakeGravityCenter(imageLabel, rtObject, label, pt))
            {
                o_vecfptObjCenter[GroupID].push_back(
                    Ipvm::Point32r2(0.5f * (rtLandSearchROI.m_left + rtLandSearchROI.m_right - 1),
                        0.5f * (rtLandSearchROI.m_top + rtLandSearchROI.m_bottom - 1)));
                m_result.vecptObjectValidLand[GroupID].push_back(false);
                continue;
            }

            o_vecrtObj[GroupID].push_back(rtObject);
            o_vecfptObjCenter[GroupID].push_back(pt);
            m_result.vecptObjectValidLand[GroupID].push_back(true);

            o_fptGravityCenter.m_x += (float)pt.m_x;
            o_fptGravityCenter.m_y += (float)pt.m_y;

            nLandidx++;
        }
    }

    o_fptGravityCenter.m_x /= (float)nPointNum;
    o_fptGravityCenter.m_y /= (float)nPointNum;

    return TRUE;
}

//가중치 구하기
BOOL VisionInspectionLgaBottom3D::MakeGravityCenter(const Ipvm::Image32s i_imageLabel,
    const Ipvm::Rect32s i_rtObjectROI, const long i_nlable, Ipvm::Point32r2& o_GravityCenter)
{
    o_GravityCenter.m_x = 0.f;
    o_GravityCenter.m_y = 0.f;
    float weightSum = 0;

    for (long y = i_rtObjectROI.m_top; y < i_rtObjectROI.m_bottom; y++)
    {
        const auto* labels = i_imageLabel.GetMem(0, y);

        for (long x = i_rtObjectROI.m_left; x < i_rtObjectROI.m_right; x++)
        {
            if (labels[x] == i_nlable)
            {
                o_GravityCenter.m_x += x;
                o_GravityCenter.m_y += y;
                weightSum++;
            }
        }
    }

    if (weightSum > 0.f)
    {
        o_GravityCenter.m_x /= weightSum;
        o_GravityCenter.m_y /= weightSum;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL VisionInspectionLgaBottom3D::MakeLandMaskROI(std::vector<std::vector<Ipvm::Point32r2>> i_vecfptSpecLand,
    std::vector<std::vector<PI_RECT>>& o_vec2pirtSpecLand, std::vector<std::vector<FPI_RECT>>& o_vec2fpirtSpecLand,
    long& o_nLayerNum, float i_fSizeOffsetWidthRatio, float i_fSizeOffsetLengthRatio)
{
    if (i_vecfptSpecLand.size() <= 0 || m_vec2LandData.size() <= 0)
        return FALSE;

    float mmToPixel = (getScale().mmToPixel().m_x + getScale().mmToPixel().m_y) / 2.f;

    o_vec2pirtSpecLand.resize(o_nLayerNum);
    o_vec2fpirtSpecLand.resize(o_nLayerNum);

    for (long GroupID = 0; GroupID < o_nLayerNum; GroupID++)
    {
        long nDataCount(0);

        for (auto LandMapData : m_vec2LandData[GroupID])
        {
            float fSizeOffsetWidth = LandMapData.fWidth * i_fSizeOffsetWidthRatio;
            float fSizeOffsetLength = LandMapData.fLength * i_fSizeOffsetLengthRatio;
            float fLandHalfWidth = (LandMapData.fWidth + fSizeOffsetWidth) * mmToPixel / 2.f;
            float fLandHalfLength = (LandMapData.fLength + fSizeOffsetLength) * mmToPixel / 2.f;
            float fLandAngle = (float)LandMapData.nAngle;

            FPI_RECT sfrtLand;
            Ipvm::Point32r2 fCenter = i_vecfptSpecLand[GroupID][nDataCount];
            Ipvm::Rect32r frtROI;
            frtROI.m_left = (fCenter.m_x - fLandHalfWidth);
            frtROI.m_top = (fCenter.m_y - fLandHalfLength);
            frtROI.m_right = (fCenter.m_x + fLandHalfWidth);
            frtROI.m_bottom = (fCenter.m_y + fLandHalfLength);

            sfrtLand
                = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
                    Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

            if (fLandAngle != 0)
            {
                float fAngle = fLandAngle * ITP_DEG_TO_RAD;

                // 시계 반대 방향이 +
                sfrtLand = sfrtLand.Rotate(fAngle);
            }

            o_vec2pirtSpecLand[GroupID].push_back(sfrtLand.GetSPI_RECT());
            o_vec2fpirtSpecLand[GroupID].push_back(sfrtLand);

            nDataCount++;
        }
    }

    return TRUE;
}

float VisionInspectionLgaBottom3D::GetAverageValueByImage(std::vector<Ipvm::Image8u> i_vecCalcImage)
{
    if (i_vecCalcImage.size() <= 0)
        return 0;

    float fImageSumValue(0.f);
    for (auto ImageCalcAverage : i_vecCalcImage)
    {
        double dImageValue(0.);
        Ipvm::ImageProcessing::GetMean(ImageCalcAverage, Ipvm::Rect32s(ImageCalcAverage), dImageValue);
        fImageSumValue += (float)dImageValue;
    }
    long nCalcImageCount = (long)i_vecCalcImage.size();

    return (float)(fImageSumValue / nCalcImageCount); //ThresholdValue로 사용할 녀석임
}

BOOL VisionInspectionLgaBottom3D::GetAverageImageImage(
    std::vector<Ipvm::Image8u> i_vecCalcImage, Ipvm::Image8u& o_AverageImage)
{
    if (i_vecCalcImage.size() <= 0) //Threshold Value가 음수값일 수는 없다
        return FALSE;

    long maxChildImageSizeX = 0;
    long maxChildImageSizeY = 0;

    for (const auto& image : i_vecCalcImage)
    {
        if (image.GetSizeX() > maxChildImageSizeX)
        {
            maxChildImageSizeX = image.GetSizeX();
        }

        if (image.GetSizeY() > maxChildImageSizeY)
        {
            maxChildImageSizeY = image.GetSizeY();
        }
    }

    Ipvm::Image32s sumImage(maxChildImageSizeX, maxChildImageSizeY);
    sumImage.FillZero();

    for (const auto& image : i_vecCalcImage)
    {
        const long imageSizeX = image.GetSizeX();
        const long imageSizeY = image.GetSizeY();

        for (long y = 0; y < imageSizeY; y++)
        {
            auto* src_y = image.GetMem(0, y);
            auto* dst_y = sumImage.GetMem(0, y);

            for (long x = 0; x < imageSizeX; x++)
            {
                dst_y[x] += src_y[x];
            }
        }
    }

    o_AverageImage.Create(maxChildImageSizeX, maxChildImageSizeY);
    o_AverageImage.FillZero();

    //평균 Image

    const long imageCount = (long)i_vecCalcImage.size();

    if (imageCount <= 0)
        return FALSE;

    for (long y = 0; y < maxChildImageSizeY; y++)
    {
        auto* src_y = sumImage.GetMem(0, y);
        auto* dst_y = o_AverageImage.GetMem(0, y);

        for (long x = 0; x < maxChildImageSizeX; x++)
        {
            dst_y[x] = CAST_BYTE(src_y[x] / imageCount);
        }
    }

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::FillArbitraryRect(const PI_RECT& srtFillRect, BYTE byVal, Ipvm::Image8u& io_image)
{
    Ipvm::Quadrangle32r quadRangle;
    quadRangle.m_ltX = float(srtFillRect.ltX);
    quadRangle.m_ltY = float(srtFillRect.ltY);
    quadRangle.m_rtX = float(srtFillRect.rtX);
    quadRangle.m_rtY = float(srtFillRect.rtY);
    quadRangle.m_lbX = float(srtFillRect.blX);
    quadRangle.m_lbY = float(srtFillRect.blY);
    quadRangle.m_rbX = float(srtFillRect.brX);
    quadRangle.m_rbY = float(srtFillRect.brY);

    if (Ipvm::ImageProcessing::Fill(quadRangle, byVal, io_image) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

void VisionInspectionLgaBottom3D::CalcZPosSr(
    const Ipvm::Image32r& zmap, const Ipvm::Image8u& mask, const Ipvm::Rect32s& rtROI, float& fZPos)
{
    if (m_VisionPara.m_nSR_Algorithm == 0)
    {
        CalcZPosSrPlane(zmap, mask, rtROI, fZPos);
        return;
    }
    /////////Test
    std::vector<float> vecfZMapData;
    vecfZMapData.reserve(rtROI.Width() * rtROI.Height());

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        auto* mask_y = mask.GetMem(0, y);
        auto* zmap_y = zmap.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            if (mask_y[x] == 255)
            {
                vecfZMapData.push_back(zmap_y[x]);
            }
        }
    }

    if (vecfZMapData.size() <= 0)
    {
        // Z값을 계산할 수 없다
        fZPos = Ipvm::k_noiseValue32r;
        return;
    }

    // TODO: std::sort() 함수는 느리므로, std::nth_element() 함수 조합으로 변경하는 것을 고려해야 함.
    std::sort(vecfZMapData.begin(), vecfZMapData.end(), std::less<float>());

    // 평균 계산
    const long calcCutoffMin
        = (long)max(0, ((float)vecfZMapData.size() * (m_VisionPara.m_SR_topPercentAverage_Min / 100.f))); //kircheis_Tan
    const long calcCutoffMax = (long)min(vecfZMapData.size(),
        ((float)vecfZMapData.size() * (m_VisionPara.m_SR_topPercentAverage_Max / 100.f))); //kircheis_Tan
    long calcCount = 0;

    double sum = 0.;
    const float* data = &vecfZMapData[0];

    for (long idx = calcCutoffMin; idx < calcCutoffMax; idx++)
    {
        sum += data[idx];
        calcCount++;
    }

    if (calcCount <= 5)
    {
        // Z값을 계산할 수 없다
        fZPos = Ipvm::k_noiseValue32r;
        return;
    }

    fZPos = float(sum / calcCount);
}

void VisionInspectionLgaBottom3D::CalcZPosSrPlane(
    const Ipvm::Image32r& zmap, const Ipvm::Image8u& mask, const Ipvm::Rect32s& rtROI, float& fZPos)
{
    if (m_VisionPara.m_nSR_Algorithm == 1)
    {
        CalcZPosSr(zmap, mask, rtROI, fZPos);
        return;
    }
    /////////Test
    std::vector<Ipvm::Point32r3> vecPoints(0);
    const float px2um_x = getScale().pixelToUm().m_x;
    const float px2um_y = getScale().pixelToUm().m_y;

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        auto* mask_y = mask.GetMem(0, y);
        auto* zmap_y = zmap.GetMem(0, y);
        const float y_um = (float)y * px2um_y;

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            if (mask_y[x] == 255)
            {
                vecPoints.push_back(Ipvm::Point32r3((float)x * px2um_x, y_um, zmap_y[x]));
            }
        }
    }

    if (vecPoints.size() <= 0)
    {
        // Z값을 계산할 수 없다
        fZPos = Ipvm::k_noiseValue32r;
        return;
    }

    Ipvm::PlaneEq64r planeEq = {0., 0., 0., 0.};

    if (Ipvm::Status::e_ok != Ipvm::DataFitting::FitToPlane((long)vecPoints.size(), &vecPoints[0], planeEq))
    {
        vecPoints.clear();
        fZPos = Ipvm::k_noiseValue32r;
        return;
    }

    const float centerx_um = 0.5f * (rtROI.m_right - 1 + rtROI.m_left) * px2um_x;
    const float centery_um = 0.5f * (rtROI.m_bottom - 1 + rtROI.m_top) * px2um_y;

    fZPos = CAST_FLOAT(planeEq.GetPositionZ((double)centerx_um, (double)centery_um));
}

///const BOOL ReadDataType : FALSE - GroupUnitCopl Type, TRUE - Other
///const std::vector<CString> &vecObjectID : 검색한 Object ID
///const std::vector<Ipvm::Point32r2> &vecObjectSpecUM : 검색한 Object Spec (um)
BOOL VisionInspectionLgaBottom3D::InspLandCoplanarity(const BOOL ReadDataType,
    const std::vector<Ipvm::Point32r3>& vecObjectPos, const std::vector<Ipvm::Rect32s>& vecrtObject,
    const std::vector<CString>& vecObjectID, const std::vector<CString>& vecObjectGroupID,
    const std::vector<Ipvm::Point32r2>& vecObjectSpecUM, const LPCTSTR strCoplInspName,
    const LPCTSTR strUnitCoplInspName, const CString strCompName) //kircheis_MED2.5
{
    if (vecObjectPos.size() != vecObjectID.size()) //Count가 안맞으면 말이 안되지..
        return FALSE;

    const long objectCount = (long)vecObjectPos.size();

    std::vector<float> vecfCopl_LSF(objectCount);

    const BOOL bDeadBug = m_packageSpec.m_deadBug;

    if (!Coplanarity_Algorithm::CalcReferencePlaneDeviation(vecObjectPos, getScale().pixelToUm().m_x,
            getScale().pixelToUm().m_y, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, vecfCopl_LSF, bDeadBug))
    {
        return FALSE;
    }

    if (ReadDataType)
    {
        VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strCoplInspName);
        if (pResult == NULL)
            return FALSE;
        VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
        if (pSpec == NULL)
            return FALSE;
        pResult->Clear();

        VisionInspectionResult* pUnitResult = m_resultGroup.GetResultByName(strUnitCoplInspName);
        if (pUnitResult == NULL)
            return FALSE;
        VisionInspectionSpec* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
        if (pUnitSpec == NULL)
            return FALSE;
        pUnitResult->Clear();

        BOOL bUseCopl = pSpec->m_use;
        BOOL bUseUnitCopl = pUnitSpec->m_use;

        if (bUseCopl)
            pResult->Resize(objectCount);
        if (bUseUnitCopl)
            pUnitResult->Resize(1);

        //여기 값을 기존 Copl의 Result에서도 뽑을수는 있는데 기존 Copl을 안쓰고 Unit Copl만 사용할 수도 있으니까 별도로 계산한다.
        float fMin = FLT_MAX;
        float fMax = FLT_MIN;
        float fWorst = FLT_MIN;
        float fABS = 0.f;
        Ipvm::Rect32s rtWorstPos{};
        CString strWorstID;

        for (long nObj = 0; nObj < objectCount; nObj++)
        {
            float fCopl = vecfCopl_LSF[nObj];

            if (bUseCopl)
            {
                pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nObj, vecObjectID[nObj],
                    vecObjectGroupID[nObj], strCompName, fCopl, *pSpec, vecObjectSpecUM[nObj].m_x,
                    vecObjectSpecUM[nObj].m_y); //kircheis_MED2.5
                pResult->SetRect(nObj, vecrtObject[nObj]);
            }
            if (bUseUnitCopl)
            {
                fMin = (float)min(fMin, fCopl);
                fMax = (float)max(fMax, fCopl);
                fABS = (float)fabs(fCopl);
                if (fABS > fWorst)
                {
                    fWorst = fABS;
                    rtWorstPos = vecrtObject[nObj];
                    strWorstID = vecObjectID[nObj];
                }
            }
        }

        if (bUseCopl)
        {
            pResult->UpdateTypicalValue(pSpec);
            pResult->SetTotalResult();
        }
        if (bUseUnitCopl)
        {
            if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(0, strWorstID, strCompName, strCompName,
                    Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
            }
            else
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
            }

            pUnitResult->SetRect(0, rtWorstPos);
            pUnitResult->UpdateTypicalValue(pUnitSpec);
            pUnitResult->SetTotalResult();
        }
        strWorstID.Empty();
    }
    else
    {
        VisionInspectionResult* pUnitResult = m_resultGroup.GetResultByName(strUnitCoplInspName);
        if (pUnitResult == NULL)
            return FALSE;
        VisionInspectionSpec* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
        if (pUnitSpec == NULL)
            return FALSE;
        pUnitResult->Clear();

        BOOL bUseUnitCopl = pUnitSpec->m_use;

        if (bUseUnitCopl)
            pUnitResult->Resize(1);

        //여기 값을 기존 Copl의 Result에서도 뽑을수는 있는데 기존 Copl을 안쓰고 Unit Copl만 사용할 수도 있으니까 별도로 계산한다.
        float fMin = FLT_MAX;
        float fMax = FLT_MIN;
        float fWorst = FLT_MIN;
        float fABS = 0.f;
        Ipvm::Rect32s rtWorstPos{};
        CString strWorstID;

        for (long nObj = 0; nObj < objectCount; nObj++)
        {
            float fCopl = vecfCopl_LSF[nObj];

            if (bUseUnitCopl)
            {
                fMin = (float)min(fMin, fCopl);
                fMax = (float)max(fMax, fCopl);
                fABS = (float)fabs(fCopl);
                if (fABS > fWorst)
                {
                    fWorst = fABS;
                    rtWorstPos = vecrtObject[nObj];
                    strWorstID = vecObjectID[nObj];
                }
            }
        }

        if (bUseUnitCopl)
        {
            if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(0, strWorstID, strCompName, strCompName,
                    Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
            }
            else
            {
                pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
            }

            pUnitResult->SetRect(0, rtWorstPos);
            pUnitResult->UpdateTypicalValue(pUnitSpec);
            pUnitResult->SetTotalResult();
        }
        strWorstID.Empty();
    }

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::InspLandCoplanarity(
    const std::vector<Ipvm::Point32r3>& vecObjectPos, const std::vector<Ipvm::Rect32s>& vecrtObject)
{
    //All Object 구간
    CString strCompID = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5
    if (!InspLandCoplanarity(TRUE, vecObjectPos, vecrtObject, m_result.vecstrID, m_result.vecstrGroupID,
            m_result.vecfptSpecLandUM, g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_COPL],
            g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_COPL], strCompID)) //kircheis_MED2.5
    {
        return FALSE;
    }

    //Group별 Data 분리 구간
    long ObjectCount = long(m_vecLandData.size());

    std::vector<std::vector<Ipvm::Point32r3>> vecObjectPosition;
    std::vector<std::vector<Ipvm::Rect32s>> vecObject;
    std::vector<std::vector<CString>> vecObjectID;
    std::vector<std::vector<CString>> vecObjectGroupID; //kircheis_MED2.5
    std::vector<std::vector<Ipvm::Point32r2>> vecLandSpec_um;

    vecObjectPosition.resize(m_group_id_list.size());
    vecObject.resize(m_group_id_list.size());
    vecObjectID.resize(m_group_id_list.size());
    vecObjectGroupID.resize(m_group_id_list.size()); //kircheis_MED2.5
    vecLandSpec_um.resize(m_group_id_list.size());

    for (long groupIndex = 0; groupIndex < long(m_group_id_list.size()); groupIndex++)
    {
        CString group_id = m_group_id_list[groupIndex];
        for (int idx = 0; idx < ObjectCount; idx++)
        {
            if (m_vecLandData[idx].m_groupID != group_id)
                continue;

            vecObjectPosition[groupIndex].push_back(vecObjectPos[idx]);
            vecObject[groupIndex].push_back(vecrtObject[idx]);
            vecObjectID[groupIndex].push_back(m_result.vecstrID[idx]);
            vecObjectGroupID[groupIndex].push_back(m_result.vecstrGroupID[idx]); //kircheis_MED2.5
            vecLandSpec_um[groupIndex].push_back(m_result.vecfptSpecLandUM[idx]);
        }
        group_id.Empty();
    }

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara.m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        strCompID = m_group_id_list[idx]; //kircheis_MED2.5
        if (!InspLandCoplanarity(TRUE, vecObjectPosition[idx], vecObject[idx], vecObjectID[idx], vecObjectGroupID[idx],
                vecLandSpec_um[idx], m_VisionPara.m_vecstrGroupInspName[idx].LandCoplName,
                m_VisionPara.m_vecstrGroupInspName[idx].LandUnitCoplName, strCompID)) //kircheis_MED2.5
        {
            return FALSE;
        }
    }

    //Group Copl 구간
    long SpecCount = long(m_VisionPara.m_vec2UnitCoplGroup.size());

    for (int idx = 0; idx < SpecCount; idx++)
    {
        strCompID.Empty(); //kircheis_MED2.5
        std::vector<Ipvm::Point32r3> ObjectPos;
        std::vector<Ipvm::Rect32s> ObjectRoi;
        std::vector<CString> ObjectID;
        std::vector<CString> groupID; //kircheis_MED2.5
        std::vector<Ipvm::Point32r2> LandSpec_um;

        long GroupCount = long(m_VisionPara.m_vec2UnitCoplGroup[idx].size());

        for (int i = 0; i < GroupCount; i++)
        {
            long selectedidx = m_VisionPara.m_vec2UnitCoplGroup[idx][i];

            //만약 저장된 값보다 Layer 갯수가 작을경우 Invalid 처리한다. (Index = 5, LayerNum = 3 일경우)
            if (long(m_group_id_list.size()) <= selectedidx)
            {
                m_bInvalid = TRUE;
                return FALSE;
            }

            ObjectPos.insert(
                ObjectPos.end(), vecObjectPosition[selectedidx].begin(), vecObjectPosition[selectedidx].end());
            ObjectRoi.insert(ObjectRoi.end(), vecObject[selectedidx].begin(), vecObject[selectedidx].end());
            ObjectID.insert(ObjectID.end(), vecObjectID[selectedidx].begin(), vecObjectID[selectedidx].end());
            groupID.insert(groupID.end(), vecObjectGroupID[selectedidx].begin(),
                vecObjectGroupID[selectedidx].end()); //kircheis_MED2.5
            LandSpec_um.insert(
                LandSpec_um.end(), vecLandSpec_um[selectedidx].begin(), vecLandSpec_um[selectedidx].end());
            strCompID += m_group_id_list[selectedidx]; //kircheis_MED2.5
        }

        if (!InspLandCoplanarity(FALSE, ObjectPos, ObjectRoi, ObjectID, groupID, LandSpec_um, _T(""),
                m_VisionPara.m_vecstrGroupUnitCoplName[idx], strCompID)) //kircheis_MED2.5
        {
            return FALSE;
        }

        for (int nIdx = 0; nIdx < ObjectID.size(); nIdx++)
        {
            ObjectID[nIdx].Empty();
        }

        for (int nIdx = 0; nIdx < groupID.size(); nIdx++)
        {
            groupID[nIdx].Empty();
        }
    }

    for (int nIdx = 0; nIdx < vecObjectID.size(); nIdx++)
    {
        for (int nIdx2 = 0; nIdx2 < vecObjectID[nIdx].size(); nIdx2++)
        {
            vecObjectID[nIdx][nIdx2].Empty();
        }
    }

    for (int nIdx = 0; nIdx < vecObjectGroupID.size(); nIdx++)
    {
        for (int nIdx2 = 0; nIdx2 < vecObjectGroupID[nIdx].size(); nIdx2++)
        {
            vecObjectGroupID[nIdx][nIdx2].Empty();
        }
    }
    strCompID.Empty();

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::InspLandHeight(const std::vector<Ipvm::Rect32s>& vecrtObjectROI,
    const std::vector<Ipvm::Point32r3>& vec3DObjectData, const std::vector<Ipvm::Point32r3>& vec3DSRData,
    const std::vector<CString>& vecObjectID, const std::vector<CString>& vecObjectGroupID,
    const std::vector<Ipvm::Point32r2>& vecObjectSpecUM, const CString strHeightInspeName, const CString strCompName)
{
    if (vecrtObjectROI.size() <= 0 || vec3DObjectData.size() <= 0 || vec3DSRData.size() <= 0)
        return FALSE;

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strHeightInspeName);
    if (pResult == NULL)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;

    pResult->Clear();

    BOOL bUse = pSpec->m_use;

    if (!bUse)
        return TRUE;

    const long nDataNum = (long)vec3DObjectData.size();
    if (nDataNum <= 0)
        return FALSE;
    if (nDataNum != (long)vec3DSRData.size())
        return FALSE;

    pResult->Resize(nDataNum);

    long idx(0);
    long nLandNum = (long)m_vecLandData.size();

    for (long nLand = 0; nLand < nLandNum; nLand++)
    {
        if (idx >= vecObjectID.size())
            break;

        if (m_vecLandData[nLand].strLandID.Compare(vecObjectID[idx]) != 0)
        {
            continue;
        }

        const float fSpecLandHeight_um = m_vecLandData[nLand].fHeight * 1000.f;

        float fLandHeight_um = (vec3DObjectData[idx].m_z - vec3DSRData[idx].m_z);
        float fLandHeightDiff_um = fLandHeight_um - fSpecLandHeight_um;

        if (vec3DObjectData[idx].m_z == Ipvm::k_noiseValue32r || vec3DSRData[idx].m_z == Ipvm::k_noiseValue32r)
        {
            fLandHeightDiff_um = Ipvm::k_noiseValue32r;
        }

        pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(idx, vecObjectID[idx], vecObjectGroupID[idx],
            strCompName, fLandHeightDiff_um, *pSpec, vecObjectSpecUM[idx].m_x, vecObjectSpecUM[idx].m_y,
            fSpecLandHeight_um); //kircheis_MED2.5
        pResult->SetRect(idx, vecrtObjectROI[idx]);

        idx++;
    }

    pResult->UpdateTypicalValue(pSpec);
    pResult->SetTotalResult();

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::InspLandHeight(const std::vector<Ipvm::Rect32s>& vecrtObjectROI,
    const std::vector<Ipvm::Point32r3>& vec3DObjectData, const std::vector<Ipvm::Point32r3>& vec3DSRData)
{
    //All Object 구간
    CString strCompName = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5
    if (!InspLandHeight(vecrtObjectROI, vec3DObjectData, vec3DSRData, m_result.vecstrID, m_result.vecstrGroupID,
            m_result.vecfptSpecLandUM, g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_HEIGHT],
            strCompName)) //kircheis_MED2.5
    {
        return FALSE;
    }

    //Group별 Data 분리 구간
    long ObjectCount = long(m_vecLandData.size());

    std::vector<std::vector<Ipvm::Point32r3>> vecObjectPosition;
    std::vector<std::vector<Ipvm::Point32r3>> vecSRPosition;
    std::vector<std::vector<Ipvm::Rect32s>> vecObject;
    std::vector<std::vector<CString>> vecObjectID;
    std::vector<std::vector<CString>> vecObjectGroupID; //kircheis_MED2.5
    std::vector<std::vector<Ipvm::Point32r2>> vecLandSpec_um;

    vecObjectPosition.resize(m_group_id_list.size());
    vecSRPosition.resize(m_group_id_list.size());
    vecObject.resize(m_group_id_list.size());
    vecObjectID.resize(m_group_id_list.size());
    vecObjectGroupID.resize(m_group_id_list.size()); //kircheis_MED2.5
    vecLandSpec_um.resize(m_group_id_list.size());

    for (long groupIndex = 0; groupIndex < long(m_group_id_list.size()); groupIndex++)
    {
        CString group_id = m_group_id_list[groupIndex];
        for (int idx = 0; idx < ObjectCount; idx++)
        {
            if (m_vecLandData[idx].m_groupID != group_id)
                continue;

            vecObjectPosition[groupIndex].push_back(vec3DObjectData[idx]);
            vecSRPosition[groupIndex].push_back(vec3DSRData[idx]);
            vecObject[groupIndex].push_back(vecrtObjectROI[idx]);
            vecObjectID[groupIndex].push_back(m_result.vecstrID[idx]);
            vecObjectGroupID[groupIndex].push_back(m_result.vecstrGroupID[idx]); //kircheis_MED2.5
            vecLandSpec_um[groupIndex].push_back(m_result.vecfptSpecLandUM[idx]);
        }
        group_id.Empty();
    }

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara.m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!InspLandHeight(vecObject[idx], vecObjectPosition[idx], vecSRPosition[idx], vecObjectID[idx],
                vecObjectGroupID[idx], vecLandSpec_um[idx], m_VisionPara.m_vecstrGroupInspName[idx].LandHeightName,
                m_group_id_list[idx])) //kircheis_MED2.5
        {
            return FALSE;
        }
    }

    for (int nIdx = 0; nIdx < vecObjectID.size(); nIdx++)
    {
        for (int nIdx2 = 0; nIdx2 < vecObjectID[nIdx].size(); nIdx2++)
        {
            vecObjectID[nIdx][nIdx2].Empty();
        }
    }

    for (int nIdx = 0; nIdx < vecObjectGroupID.size(); nIdx++)
    {
        for (int nIdx2 = 0; nIdx2 < vecObjectGroupID[nIdx].size(); nIdx2++)
        {
            vecObjectGroupID[nIdx][nIdx2].Empty();
        }
    }
    strCompName.Empty();
    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::InspWarpage(const std::vector<Ipvm::Point32r3>& vec3DSrPos,
    const std::vector<Ipvm::Rect32s>& vecrtSr, const std::vector<CString>& vecObjectID,
    const std::vector<CString>& vecObjectGroupID, const std::vector<Ipvm::Point32r2>& vecObjectSpecUM,
    const LPCTSTR strWarpageInspName, const LPCTSTR strUnitWarpageInspName, const CString strCompName) //kircheis_MED2.5
{
    const long objectCount = (long)vec3DSrPos.size();
    std::vector<float> vecfWarpage_LSF(objectCount);

    const BOOL bDeadBug = m_packageSpec.m_deadBug;
    //const float fDeadBugGain = bDeadBug ? -1.f : 1.f;

    if (!Coplanarity_Algorithm::CalcReferencePlaneDeviation(vec3DSrPos, getScale().pixelToUm().m_x,
            getScale().pixelToUm().m_y, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, vecfWarpage_LSF, bDeadBug))
    {
        return FALSE;
    }

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strWarpageInspName);
    if (pResult == NULL)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;
    pResult->Clear();

    VisionInspectionResult* pUnitResult = m_resultGroup.GetResultByName(strUnitWarpageInspName);
    if (pUnitResult == NULL)
        return FALSE;
    VisionInspectionSpec* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
    if (pUnitSpec == NULL)
        return FALSE;
    pUnitResult->Clear();

    BOOL bUseWarpage = pSpec->m_use;
    BOOL bUseUnitWarpage = pUnitSpec->m_use;

    if (bUseWarpage)
        pResult->Resize(objectCount);
    if (bUseUnitWarpage)
        pUnitResult->Resize(1);

    //// Warpage shape
    //{
    //	Shape_Algorithm shapeAlgorithm;

    //	std::vector<Ipvm::Point64r3> dataPoints;

    //	long nIndex = 0;
    //	for (const auto &SrPos : vec3DSrPos)
    //	{
    //		float fWarpage = vecfWarpage_LSF[nIndex];

    //		if (fWarpage > -10000.f)
    //		{
    //			dataPoints.emplace_back(SrPos.m_x * fDeadBugGain, SrPos.m_y, fWarpage);
    //		}
    //		nIndex++;
    //	}

    //	CString strSign;
    //	CString strShape;
    //	double R;
    //	double B4_um;
    //	double B5_um;
    //	double angle_deg;

    //	if (shapeAlgorithm.GetShape(dataPoints, strSign, strShape, R, B4_um, B5_um, angle_deg))
    //	{
    //		m_visionWarpageShapeResult.SetResult(strSign, strShape, R, B4_um, B5_um, angle_deg);

    //		auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Warpage Shape Info"));
    //		if (psDebugInfo != NULL)
    //		{
    //			if (psDebugInfo->pData != NULL)
    //			{
    //				psDebugInfo->Reset();
    //			}
    //			psDebugInfo->nDataNum = 1;
    //			VisionWarpageShapeResult* pVisionWarpageShapeResult = new VisionWarpageShapeResult[psDebugInfo->nDataNum];
    //			for (long i = 0; i < psDebugInfo->nDataNum; i++)
    //			{
    //				pVisionWarpageShapeResult[i].SetResult(m_visionWarpageShapeResult);
    //			}
    //			psDebugInfo->pData = pVisionWarpageShapeResult;
    //		}
    //	}
    //	else
    //	{
    //		m_visionWarpageShapeResult.Init();
    //	}
    //}

    float fMin = 999999.999999f;
    float fMax = -999999.999999f;
    float fWorst = -999999.999999f;
    float fABS = 0.f;
    Ipvm::Rect32s rtWorstPos{};
    CString strWorstID;

    for (long nObj = 0; nObj < objectCount; nObj++)
    {
        float fWarpage = vecfWarpage_LSF[nObj];

        if (bUseWarpage)
        {
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nObj, vecObjectID[nObj], vecObjectGroupID[nObj],
                strCompName, fWarpage, *pSpec, vecObjectSpecUM[nObj].m_x, vecObjectSpecUM[nObj].m_y); //kircheis_MED2.5
            pResult->SetRect(nObj, vecrtSr[nObj]);
        }
        if (bUseUnitWarpage)
        {
            fMin = (float)min(fMin, fWarpage);
            fMax = (float)max(fMax, fWarpage);
            fABS = (float)fabs(fWarpage);
            if (fABS > fWorst)
            {
                fWorst = fABS;
                rtWorstPos = vecrtSr[nObj];
                strWorstID = vecObjectID[nObj];
            }
        }
    }

    if (bUseWarpage)
    {
        pResult->UpdateTypicalValue(pSpec);
        pResult->SetTotalResult();
    }

    if (bUseUnitWarpage)
    {
        if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }
        else
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }
        pUnitResult->SetRect(0, rtWorstPos);
        pUnitResult->UpdateTypicalValue(pUnitSpec);
        pUnitResult->SetTotalResult();
    }

    strWorstID.Empty();

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::InspWarpage(const std::vector<Ipvm::Point32r3>& vec3DSrPos,
    const std::vector<Ipvm::Rect32s>& vecrtSr, const std::vector<CString>& vecObjectID,
    const std::vector<CString>& vecObjectGroupID, const std::vector<Ipvm::Point32r2>& vecObjectSpecUM,
    const LPCTSTR strWarpageInspName, const LPCTSTR strUnitWarpageInspName, const bool detailSetupMode,
    const CString strCompName) //kircheis_MED2.5
{
    const long objectCount = (long)vec3DSrPos.size();
    std::vector<float> vecfWarpage_LSF(objectCount);

    const BOOL bDeadBug = m_packageSpec.m_deadBug;
    const float fDeadBugGain = bDeadBug ? -1.f : 1.f;

    if (!Coplanarity_Algorithm::CalcReferencePlaneDeviation(vec3DSrPos, getScale().pixelToUm().m_x,
            getScale().pixelToUm().m_y, Ipvm::k_noiseValue32r, Ipvm::k_noiseValue32r, vecfWarpage_LSF, bDeadBug))
    {
        return FALSE;
    }

    VisionInspectionResult* pResult = m_resultGroup.GetResultByName(strWarpageInspName);
    if (pResult == NULL)
        return FALSE;
    VisionInspectionSpec* pSpec = GetSpecByName(pResult->m_resultName);
    if (pSpec == NULL)
        return FALSE;
    pResult->Clear();

    VisionInspectionResult* pUnitResult = m_resultGroup.GetResultByName(strUnitWarpageInspName);
    if (pUnitResult == NULL)
        return FALSE;
    VisionInspectionSpec* pUnitSpec = GetSpecByName(pUnitResult->m_resultName);
    if (pUnitSpec == NULL)
        return FALSE;
    pUnitResult->Clear();

    //{{
    auto* resultUnitAX
        = m_resultGroup.GetResultByName(g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AX]);
    if (resultUnitAX == NULL)
        return FALSE;
    auto* specUnitAX = GetSpecByName(resultUnitAX->m_resultName);
    if (specUnitAX == NULL)
        return FALSE;
    resultUnitAX->Clear();

    auto* resultUnitAV
        = m_resultGroup.GetResultByName(g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AV]);
    if (resultUnitAV == NULL)
        return FALSE;
    auto* specUnitAV = GetSpecByName(resultUnitAV->m_resultName);
    if (specUnitAV == NULL)
        return FALSE;
    resultUnitAV->Clear();

    auto* resultUnitAS
        = m_resultGroup.GetResultByName(g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AS]);
    if (resultUnitAS == NULL)
        return FALSE;
    auto* specUnitAS = GetSpecByName(resultUnitAS->m_resultName);
    if (specUnitAS == NULL)
        return FALSE;
    resultUnitAS->Clear();

    auto* resultUnitAO
        = m_resultGroup.GetResultByName(g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE_AO]);
    if (resultUnitAO == NULL)
        return FALSE;
    auto* specUnitAO = GetSpecByName(resultUnitAO->m_resultName);
    if (specUnitAO == NULL)
        return FALSE;
    resultUnitAO->Clear();
    //}}
    BOOL bUseWarpage = pSpec->m_use;
    BOOL bUseUnitWarpage = pUnitSpec->m_use;
    BOOL bUseUnitWarpageAX = specUnitAX->m_use;
    BOOL bUseUnitWarpageAV = specUnitAV->m_use;
    BOOL bUseUnitWarpageAS = specUnitAS->m_use;
    BOOL bUseUnitWarpageAO = specUnitAO->m_use;

    BOOL bUseUnitAs
        = (bUseUnitWarpageAX && bUseUnitWarpageAV && bUseUnitWarpageAS && bUseUnitWarpageAO); //kircheis_MED2LW //
    BOOL bUseOrUnitAs
        = (bUseUnitWarpageAX || bUseUnitWarpageAV || bUseUnitWarpageAS || bUseUnitWarpageAO); //kircheis_MED2LW

    if (detailSetupMode && (!bUseUnitAs && bUseOrUnitAs))
    {
        ::SimpleMessage(_T("All items from [Unit Warpage AX] to [Unit Warpage AO] must be on."));
    }

    if (bUseWarpage)
        pResult->Resize(objectCount);
    if (bUseUnitWarpage)
        pUnitResult->Resize(1);
    if (bUseUnitWarpageAX)
        resultUnitAX->Resize(1);
    if (bUseUnitWarpageAV)
        resultUnitAV->Resize(1);
    if (bUseUnitWarpageAS)
        resultUnitAS->Resize(1);
    if (bUseUnitWarpageAO)
        resultUnitAO->Resize(1);

    // Warpage shape
    {
        ShapeAlgorithm shapeAlgorithm;

        std::vector<Ipvm::Point64r3> dataPoints;

        long nIndex = 0;
        for (const auto& SrPos : vec3DSrPos)
        {
            float fWarpage = vecfWarpage_LSF[nIndex];

            if (fWarpage > -10000.f)
            {
                dataPoints.emplace_back(SrPos.m_x * fDeadBugGain, SrPos.m_y, fWarpage);
            }
            nIndex++;
        }

        CString strSign;
        CString strShape;
        double R;
        double B4_um;
        double B5_um;
        double angle_deg;

        if (shapeAlgorithm.GetShape(dataPoints, strSign, strShape, R, B4_um, B5_um, angle_deg))
        {
            m_visionWarpageShapeResult.SetResult(strSign, strShape, R, B4_um, B5_um, angle_deg);

            auto* psDebugInfo = m_DebugInfoGroup.GetDebugInfo(_T("Warpage Shape Info"));
            if (psDebugInfo != NULL)
            {
                if (psDebugInfo->pData != NULL)
                {
                    psDebugInfo->Reset();
                }
                psDebugInfo->nDataNum = 1;
                VisionWarpageShapeResult* pVisionWarpageShapeResult
                    = new VisionWarpageShapeResult[psDebugInfo->nDataNum];
                for (long i = 0; i < psDebugInfo->nDataNum; i++)
                {
                    pVisionWarpageShapeResult[i].SetResult(m_visionWarpageShapeResult);
                }
                psDebugInfo->pData = pVisionWarpageShapeResult;
            }
        }
        else
        {
            m_visionWarpageShapeResult.Init();
        }

        strSign.Empty();
        strShape.Empty();
    }

    float fMin = 999999.999999f;
    float fMax = -999999.999999f;
    float fWorst = -999999.999999f;
    float fABS = 0.f;
    Ipvm::Rect32s rtWorstPos{};
    CString strWorstID;

    for (long nObj = 0; nObj < objectCount; nObj++)
    {
        float fWarpage = vecfWarpage_LSF[nObj];

        if (bUseWarpage)
        {
            pResult->SetValue_EachResult_AndTypicalValueBySingleSpec(nObj, vecObjectID[nObj], vecObjectGroupID[nObj],
                strCompName, fWarpage, *pSpec, vecObjectSpecUM[nObj].m_x, vecObjectSpecUM[nObj].m_y); //kircheis_MED2.5
            pResult->SetRect(nObj, vecrtSr[nObj]);
        }
        if (bUseUnitWarpage || bUseUnitAs)
        {
            fMin = (float)min(fMin, fWarpage);
            fMax = (float)max(fMax, fWarpage);
            fABS = (float)fabs(fWarpage);
            if (fABS > fWorst)
            {
                fWorst = fABS;
                rtWorstPos = vecrtSr[nObj];
                strWorstID = vecObjectID[nObj];
            }
        }
    }

    if (bUseWarpage)
    {
        pResult->UpdateTypicalValue(pSpec);
        pResult->SetTotalResult();
    }

    //{{//kircheis_MED2.5
    CString strWarpageShape;
    BOOL bIsSignPlus = (m_visionWarpageShapeResult.m_sign == _T("+"));
    float fValue = (fMax - fMin) * (bIsSignPlus ? 1.f : -1.f);
    if (bIsSignPlus
        && (m_visionWarpageShapeResult.m_shape == _T("Cylindrical")
            || m_visionWarpageShapeResult.m_shape == _T("Spherical"))) //AX
        strWarpageShape.Format(_T("AX"));
    else if (m_visionWarpageShapeResult.m_shape == _T("Cylindrical")
        || m_visionWarpageShapeResult.m_shape == _T("Spherical")) //AV
        strWarpageShape.Format(_T("AV"));
    else if (m_visionWarpageShapeResult.m_shape == _T("Saddle")) //AS
        strWarpageShape.Format(_T("AS"));
    else //AO
        strWarpageShape.Format(_T("AO"));
    //}}

    if (bUseUnitWarpage)
    {
        if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(0, strWorstID, strCompName, strWarpageShape,
                Ipvm::k_noiseValue32r, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }
        else
        {
            pUnitResult->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strWarpageShape, fMax - fMin, *pUnitSpec, 0.f, 0.f); //kircheis_MED2.5
        }
        pUnitResult->SetRect(0, rtWorstPos);
        pUnitResult->UpdateTypicalValue(pUnitSpec);
        pUnitResult->SetTotalResult();
    }

    if (bUseUnitAs) //kircheis_MED2.5
    {
        if (fMax == Ipvm::k_noiseValue32r || fMin == Ipvm::k_noiseValue32r)
        {
            resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, Ipvm::k_noiseValue32r, *specUnitAX, 0.f, 0.f);
            resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, Ipvm::k_noiseValue32r, *specUnitAV, 0.f, 0.f);
            resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, Ipvm::k_noiseValue32r, *specUnitAS, 0.f, 0.f);
            resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                0, strWorstID, strCompName, strCompName, Ipvm::k_noiseValue32r, *specUnitAO, 0.f, 0.f);
        }
        else
        {
            if (strWarpageShape == _T("AX")) //AX
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fValue, *specUnitAX, 0.f, 0.f);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAV, 0.f, 0.f, 0.f, PASS);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAS, 0.f, 0.f, 0.f, PASS);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAO, 0.f, 0.f, 0.f, PASS);
            }
            else if (strWarpageShape == _T("AV")) //AV
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAX, 0.f, 0.f, 0.f, PASS);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fValue, *specUnitAV, 0.f, 0.f);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAS, 0.f, 0.f, 0.f, PASS);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAO, 0.f, 0.f, 0.f, PASS);
            }
            else if (strWarpageShape == _T("AS")) //AS
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAX, 0.f, 0.f, 0.f, PASS);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAV, 0.f, 0.f, 0.f, PASS);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fValue, *specUnitAS, 0.f, 0.f);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAO, 0.f, 0.f, 0.f, PASS);
            }
            else //AO
            {
                resultUnitAX->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAX, 0.f, 0.f, 0.f, PASS);
                resultUnitAV->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAV, 0.f, 0.f, 0.f, PASS);
                resultUnitAS->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, 0.f, *specUnitAS, 0.f, 0.f, 0.f, PASS);
                resultUnitAO->SetValue_EachResult_AndTypicalValueBySingleSpec(
                    0, strWorstID, strCompName, strCompName, fValue, *specUnitAO, 0.f, 0.f);
            }
        }
        resultUnitAX->SetRect(0, rtWorstPos);
        resultUnitAV->SetRect(0, rtWorstPos);
        resultUnitAS->SetRect(0, rtWorstPos);
        resultUnitAO->SetRect(0, rtWorstPos);

        resultUnitAX->UpdateTypicalValue(specUnitAX);
        resultUnitAV->UpdateTypicalValue(specUnitAV);
        resultUnitAS->UpdateTypicalValue(specUnitAS);
        resultUnitAO->UpdateTypicalValue(specUnitAO);

        resultUnitAX->SetTotalResult();
        resultUnitAV->SetTotalResult();
        resultUnitAS->SetTotalResult();
        resultUnitAO->SetTotalResult();
    }

    strWorstID.Empty();
    strWarpageShape.Empty();

    return TRUE;
}

BOOL VisionInspectionLgaBottom3D::InspWarpage(const std::vector<Ipvm::Point32r3>& vec3DSrPos,
    const std::vector<Ipvm::Rect32s>& vecrtSr, const bool detailSetupMode)
{
    //All Object 구간
    CString strCompID = m_packageSpec.m_OriginLandData->m_strAllGroupName; //kircheis_MED2.5
    if (!InspWarpage(vec3DSrPos, vecrtSr, m_result.vecstrID, m_result.vecstrGroupID, m_result.vecfptSpecLandUM,
            g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_WARPAGE],
            g_szLand3DInspectionName[LAND3D_INSPECTION_LAND_UNIT_WARPAGE], detailSetupMode,
            strCompID)) //kircheis_MED2.5
    {
        return FALSE;
    }

    //Group별 Data 분리 구간
    long ObjectCount = long(m_vecLandData.size());

    std::vector<std::vector<Ipvm::Point32r3>> vecSRPosition;
    std::vector<std::vector<Ipvm::Rect32s>> vecObject;
    std::vector<std::vector<CString>> vecObjectID;
    std::vector<std::vector<CString>> vecObjectGroupID; //kircheis_MED2.5
    std::vector<std::vector<Ipvm::Point32r2>> vecLandSpec_um;

    vecSRPosition.resize(m_group_id_list.size());
    vecObject.resize(m_group_id_list.size());
    vecObjectID.resize(m_group_id_list.size());
    vecObjectGroupID.resize(m_group_id_list.size()); //kircheis_MED2.5
    vecLandSpec_um.resize(m_group_id_list.size());

    for (long groupIndex = 0; groupIndex < long(m_group_id_list.size()); groupIndex++)
    {
        CString group_id = m_group_id_list[groupIndex];
        for (int idx = 0; idx < ObjectCount; idx++)
        {
            if (m_vecLandData[idx].m_groupID != group_id)
                continue;

            vecSRPosition[groupIndex].push_back(vec3DSrPos[idx]);
            vecObject[groupIndex].push_back(vecrtSr[idx]);
            vecObjectID[groupIndex].push_back(m_result.vecstrID[idx]);
            vecObjectGroupID[groupIndex].push_back(m_result.vecstrGroupID[idx]); //kircheis_MED2.5
            vecLandSpec_um[groupIndex].push_back(m_result.vecfptSpecLandUM[idx]);
        }
        group_id.Empty();
    }

    //Group Object 구간
    long GroupSpecCount = long(m_VisionPara.m_vecstrGroupInspName.size());

    if (GroupSpecCount != 0 && GroupSpecCount != long(m_group_id_list.size()))
        return TRUE;

    for (int idx = 0; idx < GroupSpecCount; idx++)
    {
        if (!InspWarpage(vecSRPosition[idx], vecObject[idx], vecObjectID[idx], vecObjectGroupID[idx],
                vecLandSpec_um[idx], m_VisionPara.m_vecstrGroupInspName[idx].LandWarpageName,
                m_VisionPara.m_vecstrGroupInspName[idx].LandUnitWarpageName, m_group_id_list[idx])) //kircheis_MED2.5
        {
            return FALSE;
        }
    }

    for (int nIdx = 0; nIdx < vecObjectID.size(); nIdx++)
    {
        for (int nIdx2 = 0; nIdx2 < vecObjectID[nIdx].size(); nIdx2++)
        {
            vecObjectID[nIdx][nIdx2].Empty();
        }
    }

    for (int nIdx = 0; nIdx < vecObjectGroupID.size(); nIdx++)
    {
        for (int nIdx2 = 0; nIdx2 < vecObjectGroupID[nIdx].size(); nIdx2++)
        {
            vecObjectGroupID[nIdx][nIdx2].Empty();
        }
    }
    strCompID.Empty();
    return TRUE;
}

void VisionInspectionLgaBottom3D::CalcZPosLand(
    const Ipvm::Image32r& zmap, const Ipvm::Image8u& mask, Ipvm::Rect32s rtArea, float& o_fZPos)
{
    float fSumZPos(0.f);

    long nValidDataNum(0);

    for (long y = rtArea.m_top; y < rtArea.m_bottom; y++)
    {
        auto* zmap_y = zmap.GetMem(0, y);
        auto* mask_y = mask.GetMem(0, y);

        for (long x = rtArea.m_left; x < rtArea.m_right; x++)
        {
            if (mask_y[x] == 255 && zmap_y[x] != Ipvm::k_noiseValue32r)
            {
                nValidDataNum++;

                fSumZPos += zmap_y[x];
            }
        }
    }

    if (nValidDataNum <= 0)
    {
        // Z값을 계산 불가능하다
        o_fZPos = Ipvm::k_noiseValue32r;
        return;
    }

    o_fZPos = fSumZPos / (float)nValidDataNum;
}

void VisionInspectionLgaBottom3D::remove_noise_point(
    const Ipvm::Image32r& zmap, const Ipvm::Rect32s& roi, Ipvm::Image8u& io_mask)
{
    for (long y = roi.m_top; y < roi.m_bottom; y++)
    {
        auto* mask_y = io_mask.GetMem(0, y);
        auto* zmap_y = zmap.GetMem(0, y);

        for (long x = roi.m_left; x < roi.m_right; x++)
        {
            if (zmap_y[x] == Ipvm::k_noiseValue32r)
            {
                mask_y[x] = 0;
            }
        }
    }
}

void VisionInspectionLgaBottom3D::remove_lowVisibility_point(const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap,
    const Ipvm::Rect32s& roi, USHORT visibilityLowerBound, Ipvm::Image8u& io_mask)
{
    UNREFERENCED_PARAMETER(zmap);

    for (long y = roi.m_top; y < roi.m_bottom; y++)
    {
        auto* mask_y = io_mask.GetMem(0, y);
        auto* vmap_y = vmap.GetMem(0, y);

        for (long x = roi.m_left; x < roi.m_right; x++)
        {
            if (vmap_y[x] < visibilityLowerBound)
            {
                mask_y[x] = 0;
            }
        }
    }
}

BOOL VisionInspectionLgaBottom3D::AlignLand(const std::vector<std::vector<Ipvm::Point32r2>>& specLandPositions_px,
    const std::vector<std::vector<Ipvm::Point32r2>>& allBlobPositions,
    std::vector<std::vector<Ipvm::Point32r2>>& allLandPositions,
    std::vector<std::vector<Ipvm::Point32r2>>& validLandPositions,
    std::vector<std::vector<Ipvm::Rect32s>>& validLandROIs)
{
    // 처음에 찾은 Land 들을 가지고 Land Align 을 수행한다.
    // 수행한 결과는 스펙의 배치와 똑같은 배치를 가지는 볼 위치이다.

    // Spec Land 위치에서 찾은 Blob 위치로의 변환을 구한다.

    // 먼저 Spec Land 위치를 수집

    float mm2px = getScale().mmToPixelXY();

    allLandPositions.clear();
    allLandPositions.resize(m_group_id_list.size());
    validLandPositions.clear();
    validLandPositions.resize(m_group_id_list.size());
    validLandROIs.clear();
    validLandROIs.resize(m_group_id_list.size());

    for (long GroupID = 0; GroupID < long(m_group_id_list.size()); GroupID++)
    {
        if (specLandPositions_px[GroupID].size() < 4)
        {
            return FALSE;
        }

        if (allBlobPositions[GroupID].size() != specLandPositions_px[GroupID].size())
        {
            return FALSE;
        }

        double affineTransformSpecToReal[2][3] = {0., 0., 0., 0., 0., 0.};

        if (Ipvm::Status::e_ok
            != Ipvm::Geometry::GetAffineTransform(&specLandPositions_px[GroupID][0], &allBlobPositions[GroupID][0],
                (long)allBlobPositions[GroupID].size(), affineTransformSpecToReal))
        {
            return FALSE;
        }

        std::vector<float> errorScores(specLandPositions_px[GroupID].size());

        for (long idx = (long)specLandPositions_px.size() - 1; idx >= 0; idx--)
        {
            const float x = (float)(specLandPositions_px[GroupID][idx].m_x * affineTransformSpecToReal[0][0]
                + specLandPositions_px[GroupID][idx].m_y * affineTransformSpecToReal[0][1]
                + affineTransformSpecToReal[0][2]);
            const float y = (float)(specLandPositions_px[GroupID][idx].m_x * affineTransformSpecToReal[1][0]
                + specLandPositions_px[GroupID][idx].m_y * affineTransformSpecToReal[1][1]
                + affineTransformSpecToReal[1][2]);

            const float dx = allBlobPositions[GroupID][idx].m_x - x;
            const float dy = allBlobPositions[GroupID][idx].m_y - y;

            errorScores[idx] = dx * dx + dy * dy;
        }

        Ipvm::Geometry::GetOptimalAffineTransform(&specLandPositions_px[GroupID][0], &allBlobPositions[GroupID][0],
            &errorScores[0], 0.5f, (long)specLandPositions_px[GroupID].size(), affineTransformSpecToReal);

        // 구한 변환으로 정확한 Land 위치를 정한다.
        allLandPositions[GroupID].clear();
        allLandPositions[GroupID].reserve(specLandPositions_px[GroupID].size());

        validLandPositions[GroupID].clear();
        validLandPositions[GroupID].reserve(specLandPositions_px[GroupID].size());

        validLandROIs[GroupID].clear();
        validLandROIs[GroupID].reserve(specLandPositions_px[GroupID].size());

        const long totalLandCount = (long)specLandPositions_px[GroupID].size();

        for (long idx = 0; idx < totalLandCount; idx++)
        {
            const float t_x = (float)(specLandPositions_px[GroupID][idx].m_x * affineTransformSpecToReal[0][0]
                + specLandPositions_px[GroupID][idx].m_y * affineTransformSpecToReal[0][1]
                + affineTransformSpecToReal[0][2]);
            const float t_y = (float)(specLandPositions_px[GroupID][idx].m_x * affineTransformSpecToReal[1][0]
                + specLandPositions_px[GroupID][idx].m_y * affineTransformSpecToReal[1][1]
                + affineTransformSpecToReal[1][2]);

            Ipvm::Point32r2 pt = {t_x, t_y};

            allLandPositions[GroupID].push_back(pt);

            if (m_vec2LandData[GroupID][idx].bIgnore)
            {
                continue;
            }

            validLandPositions[GroupID].push_back(pt);

            const float HalfWidth = m_vec2LandData[GroupID][idx].fWidth * mm2px / 2;
            const float HalfLength = m_vec2LandData[GroupID][idx].fLength * mm2px / 2;

            validLandROIs[GroupID].emplace_back(Ipvm::Rect32s(int32_t(t_x - HalfWidth + 0.5f),
                int32_t(t_y - HalfLength + 0.5f), int32_t(t_x + HalfWidth + 1.5f), int32_t(t_y + HalfLength + 1.5f)));
        }
    }

    return TRUE;
}

void VisionInspectionLgaBottom3D::getGroupInfo(
    std::vector<Package::Land> i_vecLandMap, std::vector<CString>& o_group_id_list)
{
    o_group_id_list.clear();

    for (auto LandInfoGroupID : i_vecLandMap)
    {
        BOOL bCheckValue = std::find(o_group_id_list.begin(), o_group_id_list.end(), LandInfoGroupID.m_groupID)
            != o_group_id_list.end();

        if (!bCheckValue)
            o_group_id_list.push_back(LandInfoGroupID.m_groupID);
    }

    std::sort(o_group_id_list.begin(), o_group_id_list.end());
}
