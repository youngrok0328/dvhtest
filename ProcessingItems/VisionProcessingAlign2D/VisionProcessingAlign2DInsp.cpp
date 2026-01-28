//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingAlign2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingAlign2D.h"
#include "EngineBasedOnBodySize.h"
#include "EngineBasedOnMatching.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Inspection.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Para.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Result_2DEmpty.h"
#include "../../VisionNeedLibrary/VisionEdgeAlign/Result_EdgeAlign.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Size32s2.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CString _debugImageFolder = _T("C:\\IntekPlus\\iPIS-500I\\Vision\\Debug\\");

BOOL VisionProcessingAlign2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

bool VisionProcessingAlign2D::CheckGrabFailure() //Grab Success는 true, Fail은 false 리턴
{
    // 실제 카메라 Image Size
    static const long nCameraImageSizeX = m_constants.m_sensorSizeX;
    static const long nCameraImageSizeY = m_constants.m_sensorSizeY;

    // 전체 Image Size
    const long nImageSizeX = getImageLot().GetImageSizeX();
    const long nImageSizeY = getImageLot().GetImageSizeY();

    // Image Count
    const long nImageCount = (long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size();

    // Stitch Image Count : 0보다 크면 Stitching 개수로 선정하고 0이면 Stitching을 안하기 때문에 1의 값을 가져간다.
    long nStitchSizeX = m_constants.m_stitchCountX > 0 ? m_constants.m_stitchCountX : 1;
    long nStitchSizeY = m_constants.m_stitchCountY > 0 ? m_constants.m_stitchCountY : 1;

    // Stitching 검증
    if ((nImageSizeY / nCameraImageSizeY) != nStitchSizeY)
    {
        nStitchSizeY = nImageSizeY / nCameraImageSizeY;
    }

    if ((nImageSizeX / nCameraImageSizeX) != nStitchSizeX)
    {
        nStitchSizeX = nImageSizeX / nCameraImageSizeX;
    }

    // SearchROI 설정
    // 1. MinMax SearchROI의 X, Y 반경 설정
    const long nROIHalfX = (nCameraImageSizeX / 1000) * 50;
    const long nROIHalfY = (nCameraImageSizeY / 1000) * 50;

    // 2. Image의 1/4지점의 X, Y 좌표 확인
    long nQuarterLengthX = nCameraImageSizeX / 4;
    long nQuarterLengthY = nCameraImageSizeY / 4;

    // 3. ROI 입력 - 총 9개 ROI 생성
    std::vector<Ipvm::Rect32s> vecrtSearchROI(0); //얘는 원본 좌표다.
    Ipvm::Rect32s rtCalcROI;
    for (long nYaxis = nQuarterLengthY; nYaxis < nCameraImageSizeY; nYaxis += nQuarterLengthY)
    {
        for (long nXaxis = nQuarterLengthX; nXaxis < nCameraImageSizeX; nXaxis += nQuarterLengthX)
        {
            int32_t nLeft = nXaxis - nROIHalfX;
            int32_t nTop = nYaxis - nROIHalfY;
            int32_t nRight = nXaxis + nROIHalfX;
            int32_t nBottom = nYaxis + nROIHalfY;
            vecrtSearchROI.push_back(Ipvm::Rect32s(nLeft, nTop, nRight, nBottom));
        }
    }
    const long nSearchROINum = (long)vecrtSearchROI.size();
    /////////////////

    //Loop 1 Image Cnt
    for (long nImage = 0; nImage < nImageCount; nImage++)
    {
        auto& image = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][nImage];

        //Loop 2 Stitch Cout
        for (long nStitchY = 0; nStitchY < nStitchSizeY; nStitchY++)
        {
            long nROI_Top = nCameraImageSizeY * nStitchY;
            for (long nStitchX = 0; nStitchX < nStitchSizeX; nStitchX++)
            {
                // Stitching 영역에 대한 ROI를 부여해 줌
                long nROI_Left = nCameraImageSizeX * nStitchX;

                //Make Detail ROI
                uint8_t uiMinimum = 255;
                uint8_t uiMaximum = 0;
                uint8_t uiMin = 0;
                uint8_t uiMax = 0;

                //Loop 3 Detail ROI Cnt
                for (long nROI = 0; nROI < nSearchROINum; nROI++)
                {
                    rtCalcROI = vecrtSearchROI[nROI];
                    rtCalcROI.OffsetRect(nROI_Left, nROI_Top);

                    uiMin = 0;
                    uiMax = 0;

                    Ipvm::ImageProcessing::GetMinMax(image, rtCalcROI, uiMin, uiMax);

                    uiMinimum = (uint8_t)min(uiMinimum, uiMin);
                    uiMaximum = (uint8_t)max(uiMaximum, uiMax);
                }

                if (uiMinimum == HALF_BYTE && uiMaximum == HALF_BYTE)
                {
                    return false;
                }
            }
        }
    }

    return true;
}

BOOL VisionProcessingAlign2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck TimeCheck;

    if (!GetConstants())
    {
        return FALSE;
    }

    ResetResult();
    m_processingState.SetNum(3);

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    m_fCalcTime = 0.f;

    BOOL success = TRUE;

    if (getImageLot().GetImageFrameCount() == 0)
    {
        return FALSE;
    }

    Ipvm::Rect32s alignImageRoi(getImageLot().GetImageFrame(0, GetCurVisionModule_Status()));

    // 여기서 Grab Fail시의 INVALID 판정을 실시한다 :: Empty Check가 활성화 되어 있을 경우에만 실시 함
    if (m_edgeAlign_para->m_emptyInsp == TRUE)
    {
        if (CheckGrabFailure() == false)
        {
            m_bInvalid = TRUE;
            return FALSE;
        }
    }
    //////////////////////////////////////////////////

    if (m_constants.m_stitchCountX > 1 || m_constants.m_stitchCountY > 1)
    {
        long nLotImageCnt = (long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size();
        if (nLotImageCnt < 1)
            return FALSE;

        // Stitch영상을 초기화 한다. 꼭 필요하지는 않지만. 디버깅할때, 혹시 남을지 모르는 이전 계산의 찌꺼기때문에 헤맬까봐 한다.
        for (long i = 0; i < getImageLot().GetImageFrameCount(); i++)
        {
            if (i >= (long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size())
                continue;

            auto& imageStitch = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].at(i);
            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageStitch), 0, imageStitch);
        }

        if (!FindAllStitchInfo_InRaw(detailSetupMode))
        {
            success = FALSE;
        }

        int alignImageSizeX = getImageLot().GetImageSizeX();
        int alignImageSizeY = getImageLot().GetImageSizeY();

        Ipvm::Rect32s alignImageRegion(0, 0, alignImageSizeX, alignImageSizeY);

        Ipvm::Point32r2 paneCenter(alignImageSizeX * 0.5f, alignImageSizeY * 0.5f);

        alignImageRoi.m_left = int(paneCenter.m_x - getReusableMemory().GetInspImageSizeX() / 2);
        alignImageRoi.m_top = int(paneCenter.m_y - getReusableMemory().GetInspImageSizeY() / 2);
        alignImageRoi.m_right = alignImageRoi.m_left + getReusableMemory().GetInspImageSizeX();
        alignImageRoi.m_bottom = alignImageRoi.m_top + getReusableMemory().GetInspImageSizeY();

        if (!m_VisionPara.m_skipEdgeAlign)
        {
            success &= Do2DStitching_AlignFrame(detailSetupMode, alignImageRoi);
            success &= Do2DStitching_EmptyCheckFrame(detailSetupMode);
        }
    }
    else
    {
        if (!m_VisionPara.m_skipEdgeAlign)
        {
            Ipvm::Image8u alignImage;
            Ipvm::Image8u emptyImage;

            alignImageRoi = m_edgeAlign_inspection->getImageSourceRoiForNoStitch();

            if (!m_edgeAlign_inspection->getImageForAlign(*m_edgeAlign_para, false, alignImage)
                || !m_edgeAlign_inspection->getImageFor2DEmpty(*m_edgeAlign_para, false, emptyImage))
            {
                success = FALSE;
            }
            else
            {
                auto imageCenter = Ipvm::Point32r2(alignImage.GetSizeX() * 0.5f, alignImage.GetSizeY() * 0.5f);

                m_edgeAlign_inspection->getResult_EdgeAlign()->SetImage(alignImage, imageCenter);
                m_edgeAlign_inspection->getResult_2DEmpty()->SetImage(emptyImage, emptyImage);
            }
        }
    }

    m_processingState.SetState(0,
        success ? VisionProcessingState::enumState::Active_Success : VisionProcessingState::enumState::Active_Invalid);

    if (success)
    {
        if (!m_VisionPara.m_skipEdgeAlign)
        {
            if (!m_edgeAlign_inspection->run(*m_edgeAlign_para, detailSetupMode))
            {
                success = FALSE;
            }

            if (m_edgeAlign_inspection->getResult_2DEmpty()->m_inspectioned)
            {
                if (m_edgeAlign_inspection->getResult_2DEmpty()->m_success)
                {
                    m_processingState.SetState(1, VisionProcessingState::enumState::Active_Success);
                }
                else
                {
                    m_processingState.SetState(1, VisionProcessingState::enumState::Active_Invalid);
                }
            }

            if (m_edgeAlign_inspection->getResult_EdgeAlign()->m_inspectioned)
            {
                if (m_edgeAlign_inspection->getResult_EdgeAlign()->m_success)
                {
                    m_processingState.SetState(2, VisionProcessingState::enumState::Active_Success);
                }
                else
                {
                    m_processingState.SetState(2, VisionProcessingState::enumState::Active_Invalid);
                }
            }
        }
        else
        {
            // Edge Align 결과로 보정되는 것은 전혀없는 것으로 만들어 주자
            auto* alignResult = m_edgeAlign_inspection->getResult_EdgeAlign();

            Ipvm::Point32r2 center(getImageLot().GetImageSizeX() * 0.5f, getImageLot().GetImageSizeY() * 0.5f);
            alignResult->Init(center);
        }
    }

    if (success)
    {
        if (m_constants.m_stitchCountX > 1 || m_constants.m_stitchCountY > 1)
        {
            success = Do2DAlignedStitchAll(alignImageRoi, detailSetupMode);
            if (success == TRUE && detailSetupMode == true)
            {
                //auto &dstImage = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][frameIndex];
            }
        }
        else // Stitching 하지 않는 영상을 회전시켜준다.
        {
            success = Do2DAlignAllImages_NoStitch(alignImageRoi, detailSetupMode);
        }

        if (success)
        {
            getImageLotInsp().m_paneIndexForCalculationImage = GetCurrentPaneID();
        }

        auto* alignResult = m_edgeAlign_inspection->getResult_EdgeAlign();

        Ipvm::Point32r2 shift(getReusableMemory().GetInspImageSizeX() * 0.5f - alignResult->m_center.m_x,
            getReusableMemory().GetInspImageSizeY() * 0.5f - alignResult->m_center.m_y);

        m_edgeAlign_inspection->setGlobalAlignResult_ZeroAngle(shift);
    }
    else
    {
        //------------------------------------------------------------------------------------
        // Align 실패시나 Empty 일때 뒤에 검사설정탭에서 이전이미지가 보이지 않게
        // 기존 검사 이미지를 지워준다
        //------------------------------------------------------------------------------------

        long inspImageSizeX = getReusableMemory().GetInspImageSizeX();
        long inspImageSizeY = getReusableMemory().GetInspImageSizeY();

        //mc_Image를 Open하지 않고 아래의 작업을 수행하려고 했을 때 Crash 방지
        for (auto& dstImage : getImageLotInsp().m_vecImages[GetCurVisionModule_Status()])
        {
            dstImage.Create(inspImageSizeX, inspImageSizeY);
            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(dstImage), 0, dstImage);
        }

        //------------------------------------------------------------------------------------

        m_edgeAlign_inspection->setGlobalAlignResult();
    }

    m_fCalcTime = (float)TimeCheck.Elapsed_ms();

    if (success == TRUE) //mc_CustomLayer Mask
    {
        const Ipvm::Point32r2 fptAlignCenter = getAlignCenter(enumCenterDefine::eCenterDefine_Body_Align);

        if (fptAlignCenter.m_x > 0 || fptAlignCenter.m_y > 0)
            SetCustomLayerMask(fptAlignCenter);
    }

    return success;
}

void VisionProcessingAlign2D::SetCustomLayerMask(const Ipvm::Point32r2 i_fptAlignCenter)
{
    long nCustomFixedDataCount = m_packageSpec.m_CustomFixedMapManager->GetCount();
    long nCustomPolygonDataCount = m_packageSpec.m_CustomPolygonMapManager->GetCount();

    std::vector<Ipvm::EllipseEq32r> vecEllipse_Circle;
    std::vector<Ipvm::EllipseEq32r> vecEllipse_NotchHole;
    std::vector<FPI_RECT> vecfRect_Rectangle;
    std::vector<Ipvm::Polygon32r> vecPolygon_Polygon;
    //std::vector<Ipvm::Polygon32r> vecPolygon_InkBarrier;

    std::vector<CustomFixed> vecCustomFixedMap_px = GetCustomFixedMapInfo_px(i_fptAlignCenter);
    for (auto CustomFixedMapInfo_px : vecCustomFixedMap_px)
    {
        if (CustomFixedMapInfo_px.m_bIgnore == TRUE)
            continue;

        CString strMaskName("");
        strMaskName.Format(_T("CustomFixedMask_%s"), (LPCTSTR)(CustomFixedMapInfo_px.GetID()));

        auto* CustomFixedMask = getReusableMemory().AddSurfaceLayerMaskClass(strMaskName);
        if (CustomFixedMask == nullptr)
            return;
        CustomFixedMask->Reset();

        switch ((long)CustomFixedMapInfo_px.m_eCustomFixedCategory)
        {
            case CustomFixedCategory::CustomFixedCategory_Circle:
            {
                CustomFixedMask->Add(CustomFixedMapInfo_px.m_fSpec_ellipse_mm, true);
                vecEllipse_Circle.push_back(CustomFixedMapInfo_px.m_fSpec_ellipse_mm);
            }
            break;
            case CustomFixedCategory::CustomFixedCategory_NotchHole:
            {
                CustomFixedMask->Add(CustomFixedMapInfo_px.m_fSpec_ellipse_mm, true);
                vecEllipse_NotchHole.push_back(CustomFixedMapInfo_px.m_fSpec_ellipse_mm);
            }
            break;
            case CustomFixedCategory::CustomFixedCategory_Rectangle:
            {
                CustomFixedMask->Add(CustomFixedMapInfo_px.m_fsrtSpecROI, true);
                vecfRect_Rectangle.push_back(CustomFixedMapInfo_px.m_fsrtSpecROI);
            }
            break;
            //case CustomCategory::CustomCategory_Polygon:	vecPolygon_Polygon.push_back(GetPolygon(CustomMapInfo_px.vecfptPointInfo_mm));		break;
            //case CustomCategory::CustomCategory_InkBarrier:	vecPolygon_InkBarrier.push_back(GetPolygon(CustomMapInfo_px.vecfptPointInfo_mm));	break;
            default:
                break;
        }
    }

    std::vector<Package::CustomPolygon> vecCustomPolygonMap_px = GetCustomPolygonMapInfo_px(i_fptAlignCenter);
    for (auto CustomPolygonMapInfo_px : vecCustomPolygonMap_px)
    {
        if (CustomPolygonMapInfo_px.m_bIgnore == TRUE)
            continue;

        CString strMaskName("");
        strMaskName.Format(_T("CustomPolygonMask_%s"), (LPCTSTR)(CustomPolygonMapInfo_px.GetID()));

        auto* CustomPolygonMask = getReusableMemory().AddSurfaceLayerMaskClass(strMaskName);
        if (CustomPolygonMask == nullptr)
            return;
        CustomPolygonMask->Reset();

        switch ((long)CustomPolygonMapInfo_px.m_eCustomPolygonCategory)
        {
            case CustomPolygonCategory::CustomPolygonCategory_Polygon:
            {
                CustomPolygonMask->Add(GetPolygon(CustomPolygonMapInfo_px.m_vecfptPointInfo_mm), true);
                vecPolygon_Polygon.push_back(GetPolygon(CustomPolygonMapInfo_px.m_vecfptPointInfo_mm));
            }
            break;
            //case CustomCategory::CustomCategory_InkBarrier:	vecPolygon_InkBarrier.push_back(GetPolygon(CustomMapInfo_px.vecfptPointInfo_mm));	break;
            default:
                break;
        }
    }

    if (nCustomFixedDataCount > 0)
    {
        if ((long)vecEllipse_Circle.size() > 0)
        {
            auto* CustomMask_Circle = getReusableMemory().AddSurfaceLayerMaskClass(_T("Entire_CustomFixedMask Circle"));
            if (CustomMask_Circle == nullptr)
                return;
            CustomMask_Circle->Reset();

            for (auto Custom_Circle : vecEllipse_Circle)
            {
                CustomMask_Circle->Add(Custom_Circle, true);
            }
        }

        if ((long)vecEllipse_NotchHole.size() > 0)
        {
            auto* CustomMask_NotchHole
                = getReusableMemory().AddSurfaceLayerMaskClass(_T("Entire_CustomFixedMask NotchHole"));
            if (CustomMask_NotchHole == nullptr)
                return;
            CustomMask_NotchHole->Reset();

            for (auto Custom_NotchHole : vecEllipse_NotchHole)
            {
                CustomMask_NotchHole->Add(Custom_NotchHole, true);
            }
        }

        if ((long)vecfRect_Rectangle.size() > 0)
        {
            auto* CustomMask_Rectangle
                = getReusableMemory().AddSurfaceLayerMaskClass(_T("Entire_CustomFixedMask Rectangle"));
            if (CustomMask_Rectangle == nullptr)
                return;
            CustomMask_Rectangle->Reset();

            for (const auto& Custom_Rectangle : vecfRect_Rectangle)
            {
                CustomMask_Rectangle->Add(Custom_Rectangle, true);
            }
        }
    }

    if (nCustomPolygonDataCount > 0)
    {
        if ((long)vecPolygon_Polygon.size() > 0)
        {
            auto* CustomMask_Polygon
                = getReusableMemory().AddSurfaceLayerMaskClass(_T("Entire_CustomPolygonMask Polygon"));
            if (CustomMask_Polygon == nullptr)
                return;
            CustomMask_Polygon->Reset();

            for (auto Custom_Polygon : vecPolygon_Polygon)
            {
                CustomMask_Polygon->Add(Custom_Polygon, true);
            }
        }

        //auto *CustomMask_InkBarrier = getReusableMemory().AddSurfaceLayerMaskClass(_T("CustomMask_Spec_InkBarrier"));
        //if (CustomMask_InkBarrier == nullptr) return;
        //CustomMask_InkBarrier->Reset();

        //for (auto Custom_InkBarrier : vecPolygon_InkBarrier)
        //{
        //	CustomMask_InkBarrier->Add(Custom_InkBarrier, true);
        //}
    }
}

std::vector<CustomFixed> VisionProcessingAlign2D::GetCustomFixedMapInfo_px(const Ipvm::Point32r2 i_fptAlignCenter)
{
    std::vector<CustomFixed> vecCustomFixedMapInfo_px;

    if (m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData.size() <= 0)
        return vecCustomFixedMapInfo_px;

    const auto& mm2px = getScale().mmToPixel();

    for (auto CustomFixedInfo_mm : m_packageSpec.m_CustomFixedMapManager->vecCustomFixedData)
    {
        if (CustomFixedInfo_mm.m_bIgnore == TRUE)
            continue;

        CustomFixed cCurrentCustomFixedMapInfo_px = CustomFixedInfo_mm;

        cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_x
            = i_fptAlignCenter.m_x + (cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_x * mm2px.m_x);
        cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_y
            = i_fptAlignCenter.m_y - (cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_y * mm2px.m_y);
        cCurrentCustomFixedMapInfo_px.m_fSpecWidth_mm = cCurrentCustomFixedMapInfo_px.m_fSpecWidth_mm * mm2px.m_x;
        cCurrentCustomFixedMapInfo_px.m_fSpecLength_mm = cCurrentCustomFixedMapInfo_px.m_fSpecLength_mm * mm2px.m_y;

        if (cCurrentCustomFixedMapInfo_px.m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Circle
            || cCurrentCustomFixedMapInfo_px.m_eCustomFixedCategory
                == CustomFixedCategory::CustomFixedCategory_NotchHole)
        {
            Ipvm::EllipseEq32r Ellipse(cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_x,
                cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_y,
                (cCurrentCustomFixedMapInfo_px.m_fSpecWidth_mm) * .5f,
                (cCurrentCustomFixedMapInfo_px.m_fSpecLength_mm) * .5f);
            cCurrentCustomFixedMapInfo_px.m_fSpec_ellipse_mm = Ellipse;
        }
        else if (cCurrentCustomFixedMapInfo_px.m_eCustomFixedCategory
            == CustomFixedCategory::CustomFixedCategory_Rectangle)
        {
            FPI_RECT sfrtSpecROI_px = FPI_RECT(Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f),
                Ipvm::Point32r2(0.f, 0.f), Ipvm::Point32r2(0.f, 0.f));
            float fHalfWidth_px = cCurrentCustomFixedMapInfo_px.m_fSpecWidth_mm * .5f;
            float fHalfLength_px = cCurrentCustomFixedMapInfo_px.m_fSpecLength_mm * .5f;

            sfrtSpecROI_px.fptLT.m_x = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_x - fHalfWidth_px;
            sfrtSpecROI_px.fptLT.m_y = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_y - fHalfLength_px;
            sfrtSpecROI_px.fptRT.m_x = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_x + fHalfWidth_px;
            sfrtSpecROI_px.fptRT.m_y = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_y - fHalfLength_px;
            sfrtSpecROI_px.fptLB.m_x = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_x - fHalfWidth_px;
            sfrtSpecROI_px.fptLB.m_y = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_y + fHalfLength_px;
            sfrtSpecROI_px.fptRB.m_x = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_x + fHalfWidth_px;
            sfrtSpecROI_px.fptRB.m_y = cCurrentCustomFixedMapInfo_px.m_fSpecCenterPos_mm.m_y + fHalfLength_px;

            cCurrentCustomFixedMapInfo_px.m_fsrtSpecROI = sfrtSpecROI_px;
        }

        vecCustomFixedMapInfo_px.push_back(cCurrentCustomFixedMapInfo_px);
    }

    return vecCustomFixedMapInfo_px;
}

std::vector<Package::CustomPolygon> VisionProcessingAlign2D::GetCustomPolygonMapInfo_px(
    const Ipvm::Point32r2 i_fptAlignCenter)
{
    std::vector<Package::CustomPolygon> vecCustomPolygonMapInfo_px;

    if (m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData.size() <= 0)
        return vecCustomPolygonMapInfo_px;

    const auto& mm2px = getScale().mmToPixel();

    for (auto CustomPolygonInfo_mm : m_packageSpec.m_CustomPolygonMapManager->vecCustomPolygonData)
    {
        if (CustomPolygonInfo_mm.m_bIgnore == TRUE)
            continue;

        Package::CustomPolygon cCurrentCustomPolygonMapInfo_px = CustomPolygonInfo_mm;

        if (cCurrentCustomPolygonMapInfo_px.m_eCustomPolygonCategory
            == CustomPolygonCategory::CustomPolygonCategory_Polygon)
        {
            for (auto& PolygonPos : cCurrentCustomPolygonMapInfo_px.m_vecfptPointInfo_mm)
            {
                PolygonPos.m_x = i_fptAlignCenter.m_x + (PolygonPos.m_x * mm2px.m_x);
                PolygonPos.m_y = i_fptAlignCenter.m_y - (PolygonPos.m_y * mm2px.m_y);
            }
        }

        vecCustomPolygonMapInfo_px.push_back(cCurrentCustomPolygonMapInfo_px);
    }

    return vecCustomPolygonMapInfo_px;
}

Ipvm::Polygon32r VisionProcessingAlign2D::GetPolygon(const std::vector<Ipvm::Point32r2> i_vecfptPolygonPos)
{
    return Ipvm::Polygon32r(&i_vecfptPolygonPos[0], (int32_t)i_vecfptPolygonPos.size());
}

float VisionProcessingAlign2D::GetMoveDistanceXbetweenFOVs_mm()
{
    if (getInspectionAreaInfo().m_stichCountX < 2)
    {
        return 0.f;
    }

    int fov_id1 = getInspectionAreaInfo().m_fovList[0];
    int fov_id2 = getInspectionAreaInfo().m_fovList[1];

    return getTrayScanSpec().GetFovCenter(fov_id2).m_x - getTrayScanSpec().GetFovCenter(fov_id1).m_x;
}

float VisionProcessingAlign2D::GetMoveDistanceYbetweenFOVs_mm()
{
    if (getInspectionAreaInfo().m_stichCountX < 1 || getInspectionAreaInfo().m_stichCountY < 2)
    {
        return 0.f;
    }

    int fov_id1 = getInspectionAreaInfo().m_fovList[0];
    int fov_id2 = getInspectionAreaInfo().m_fovList[getInspectionAreaInfo().m_stichCountX];

    return getTrayScanSpec().GetFovCenter(fov_id2).m_y - getTrayScanSpec().GetFovCenter(fov_id1).m_y;
}

bool VisionProcessingAlign2D::Do2DStitching_GetStitchInfo(const bool detailSetupMode)
{
    Ipvm::TimeCheck TimeCheck;

    if (!GetConstants())
        return false;

    bool success = true;

    if (m_constants.m_stitchCountX > 1 || m_constants.m_stitchCountY > 1)
    {
        success = FindAllStitchInfo_InRaw(detailSetupMode);

        if (success)
        {
            getImageLotInsp().m_paneIndexForCalculationImage = GetCurrentPaneID();
        }
    }

    m_fCalcTime = (float)TimeCheck.Elapsed_ms();

    return success;
}

bool VisionProcessingAlign2D::GetConstants()
{
    static const bool isSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    static const float fMargin = ((isSideVision == true)
            ? 5000.f
            : 10000.f); //Side Vision 일때 여유 구간은 많이 가져갈 필요가 없다. Side Vision일 때는 절반 수준으로 낮춤

    if (!m_constants.Make())
        return false;
    if (!m_basedOnMatching->GetConstants())
        return false;
    if (!m_basedOnBodySize->GetConstants())
        return false;

    float inspMargeX = getScale().convert_umToPixelX(fMargin);
    float inspMargeY = getScale().convert_umToPixelY(fMargin);
    long inspSizeX
        = (long(m_constants.m_packageSizeX_px + inspMargeX) / 4) * 4; // 양쪽을 균일하게 만들기 편하게 4의 배수로 만들자
    long inspSizeY
        = (long(m_constants.m_packageSizeY_px + inspMargeY) / 4) * 4; // 양쪽을 균일하게 만들기 편하게 4의 배수로 만들자

    getReusableMemory().SetInspImageSize(inspSizeX, inspSizeY);

    long eStitchedSizeX = m_constants.m_rawImageSizeX - m_constants.m_overlapPixelX * (m_constants.m_stitchCountX - 1);
    long eStitchedSizeY = m_constants.m_rawImageSizeY - m_constants.m_overlapPixelY * (m_constants.m_stitchCountY - 1);

    m_StichedOffset.Set(
        (m_constants.m_rawImageSizeX - eStitchedSizeX) / 2, (m_constants.m_rawImageSizeY - eStitchedSizeY) / 2);

    return true;
}

BOOL VisionProcessingAlign2D::Do2DStitching_EmptyCheckFrame(const bool detailSetupMode)
{
    if (!m_edgeAlign_para->m_emptyInsp)
        return true;

    long emptyCheckFrame = m_edgeAlign_para->m_emptyFrameIndex.getFrameIndex();
    if (emptyCheckFrame < 0)
    {
        return false;
    }

    // commit 하기 위해 일단 기존 stitching 영상을 이용한다.
    long emptySampling = 4;

    long stitchSizeX = getImageLot().GetImageSizeX();
    long stitchSizeY = getImageLot().GetImageSizeY();
    long stitchSizeXForSampling = stitchSizeX / emptySampling;
    long stitchSizeYForSampling = stitchSizeY / emptySampling;

    Ipvm::Image8u curImageStitch_Normal;
    Ipvm::Image8u curImageStitch_Sampling;

    if (!getReusableMemory().GetByteImage(curImageStitch_Sampling, stitchSizeXForSampling, stitchSizeYForSampling))
    {
        return FALSE;
    }

    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    if (detailSetupMode)
    {
        if (!getReusableMemory().GetByteImage(curImageStitch_Normal, rawImageSizeX, rawImageSizeY))
        {
            return FALSE;
        }

        curImageStitch_Normal.FillZero();
        curImageStitch_Sampling.FillZero();
    }

    auto& rawImage = getImageLot().GetImageFrame(emptyCheckFrame, GetCurVisionModule_Status());

    for (long runIndex = 0; runIndex < 2; runIndex++)
    {
        auto curRawImage = rawImage;
        auto& dstImage = (runIndex == 0) ? curImageStitch_Sampling : curImageStitch_Normal;
        int sampling = (runIndex == 0) ? emptySampling : 1;

        int fovSizeX = m_constants.m_sensorSizeX / sampling;
        int fovSizeY = m_constants.m_sensorSizeY / sampling;
        int imageSizeX = (runIndex == 0) ? stitchSizeXForSampling : stitchSizeX;
        int imageSizeY = (runIndex == 0) ? stitchSizeYForSampling : stitchSizeY;

        if (!detailSetupMode && runIndex == 1)
        {
            // Detail Setup이 아닌경우 Sampling 영상만 계산한다
            break;
        }

        if (runIndex == 0)
        {
            if (!getReusableMemory().GetByteImage(curRawImage, imageSizeX, imageSizeY))
            {
                return FALSE;
            }

            Ipvm::ImageProcessing::ResizeLinearInterpolation(rawImage, curRawImage);
        }

        // resized 된 raw 영상을 stitch 한다.
        float fRotateAngle;
        Ipvm::Point32r2 fptCenter;
        Ipvm::Point32r2 fptShift;
        Ipvm::Point32r2 fptShift_Tot;
        for (int32_t y = 0; y < m_constants.m_stitchCountY; y++)
        {
            for (int32_t x = 0; x < m_constants.m_stitchCountX; x++)
            {
                long stitchIndex = y * m_constants.m_stitchCountX + x;
                auto& curStitchInfo = m_result.m_stitchRois[stitchIndex];

                if (x == 0 && y == 0)
                {
                    Ipvm::Rect32s ROI = Ipvm::Rect32s(0, 0, fovSizeX, fovSizeY);
                    Ipvm::ImageProcessing::Copy(curRawImage, ROI, ROI + (m_StichedOffset / sampling), dstImage);
                }
                else
                {
                    fRotateAngle = (float)(-(curStitchInfo.GetAngle()));
                    fptCenter = curStitchInfo.GetOrigin() / (float)sampling;
                    fptShift = curStitchInfo.GetShift() / (float)sampling;
                    fptShift_Tot = Ipvm::Conversion::ToPoint32r2(m_StichedOffset); // / (float)sampling;

                    //doStitch_OneFrame(x, y, m_nAlignFrame, fptCenter, fRotateAngle, fptShift, fptShift_Tot);
                    BOOL bImageSave = m_VisionPara.m_saveTemplateDebugImage; //FALSE;

                    //// Raw Image의 현재 FOV 영역만 Rotate & Shift 시키기  -> imageRotateSmall 에 복사됨.
                    Ipvm::Image8u imageRotateSmall;
                    if (!getReusableMemory().GetByteImage(imageRotateSmall, fovSizeX, fovSizeY))
                        return FALSE;

                    // RotateShiftFOVImage(x, y, nFrameID, fptCenter, fRotateAngle, fptShift_FOV, imageRotateSmall);
                    // Raw Image 에서 현재 FOV 영역의 Image pointer 가져오기
                    Ipvm::Rect32s rtCurFOVImage((int32_t)(x * fovSizeX), (int32_t)(y * fovSizeY),
                        (int32_t)((x + 1) * fovSizeX), (int32_t)((y + 1) * fovSizeY));
                    const Ipvm::Image8u curRawFOVImage(curRawImage, rtCurFOVImage);

                    Ipvm::Rect32s rectImage = Ipvm::Rect32s(0, 0, curRawFOVImage.GetSizeX(), curRawFOVImage.GetSizeY());
                    auto error = Ipvm::ImageProcessing::RotateLinearInterpolation(
                        curRawFOVImage, rectImage, fptCenter, -fRotateAngle, fptShift, imageRotateSmall);
                    VERIFY(error == Ipvm::Status::e_ok);

                    if (bImageSave)
                    {
                        Ipvm::ImageFile::SaveAsBmp(dstImage, _debugImageFolder + _T("stitch_old.bmp"));
                        Ipvm::ImageFile::SaveAsBmp(imageRotateSmall, _debugImageFolder + _T("SrcRotated.bmp"));
                    }

                    // Stitching을 함으로써 preOverlapROI가 이동되는 영역을 계산한다.
                    const auto& verPreROIOverlap = m_constants.GetVerPreOverlapROI(stitchIndex);
                    const auto& horPreROIOverlap = m_constants.GetHorPreOverlapROI(stitchIndex);
                    Ipvm::Rect32s shiftedVerPreROI, shiftedHorPreROI;

                    UpdatePreOverlapROI(
                        x, y, verPreROIOverlap, horPreROIOverlap, fptShift_Tot, shiftedVerPreROI, shiftedHorPreROI);

                    //// 단순 복사할 영역 복사하기
                    // 단순히 복사해와야할 영역구하기
                    Ipvm::Point32s2 ptSrcCopyStart;
                    Ipvm::Point32s2 ptDstCopyStart;
                    Ipvm::Size32s2 roiCopySize;
                    GetCopyRegion(
                        x, y, shiftedVerPreROI, shiftedHorPreROI, ptSrcCopyStart, ptDstCopyStart, roiCopySize);

                    ptSrcCopyStart = (ptSrcCopyStart / sampling);
                    ptDstCopyStart = (ptDstCopyStart / sampling);
                    Ipvm::Size32s2 resizeroiCopySize(roiCopySize.m_w / sampling, roiCopySize.m_h / sampling);

                    CippModules::CopyValid(
                        imageRotateSmall, ptSrcCopyStart, ptDstCopyStart, resizeroiCopySize, dstImage);

                    if (bImageSave)
                    {
                        Ipvm::ImageFile::SaveAsBmp(dstImage, _debugImageFolder + _T("stitch_SimpleCopy.bmp"));

                        Ipvm::Image8u srcCopyImage(fovSizeX, fovSizeY,
                            imageRotateSmall.GetMem() + ptSrcCopyStart.m_x + (ptSrcCopyStart.m_y * fovSizeX), fovSizeX);

                        Ipvm::ImageFile::SaveAsBmp(srcCopyImage, _debugImageFolder + _T("SrcCopyImage.bmp"));
                    }
                }
            }
        }
    }

    Ipvm::Rect32s roi;
    roi.m_left = (getImageLot().GetImageSizeX() - getReusableMemory().GetInspImageSizeX()) / 2;
    roi.m_top = (getImageLot().GetImageSizeY() - getReusableMemory().GetInspImageSizeY()) / 2;
    roi.m_right = roi.m_left + getReusableMemory().GetInspImageSizeX();
    roi.m_bottom = roi.m_top + getReusableMemory().GetInspImageSizeY();

    // curImageStitch_Normal은 Inline중에는 만들지 않는다.
    //roi &= Ipvm::Rect32s(curImageStitch_Normal);
    roi &= Ipvm::Rect32s(0, 0, getImageLot().GetImageSizeX(), getImageLot().GetImageSizeY());

    float samplingX = float(stitchSizeX) / stitchSizeXForSampling;
    float samplingY = float(stitchSizeY) / stitchSizeYForSampling;

    Ipvm::Rect32s roi_Sampling(static_cast<int>(roi.m_left / samplingX + 0.5f),
        static_cast<int>(roi.m_top / samplingY + 0.5f), static_cast<int>(roi.m_right / samplingX + 1.5f),
        static_cast<int>(roi.m_bottom / samplingY + 1.5f));
    roi_Sampling &= Ipvm::Rect32s(curImageStitch_Sampling);

    curImageStitch_Normal = Ipvm::Image8u(curImageStitch_Normal, roi);
    curImageStitch_Sampling = Ipvm::Image8u(curImageStitch_Sampling, roi_Sampling);

    if (detailSetupMode)
    {
        m_edgeAlign_inspection->getResult_2DEmpty()->SetImage(curImageStitch_Normal, curImageStitch_Sampling);
    }
    else
    {
        m_edgeAlign_inspection->getResult_2DEmpty()->SetImage(curImageStitch_Sampling, samplingX, samplingY);
    }

    return TRUE;
}

// 스티치할 위치를 먼저 찾고, Align을 위해 한 장만 붙인다.
// 스티치할 위치는 StichingFrame 에서 찾고, 실제 이어 붙이는것은  Align frame 이다.
BOOL VisionProcessingAlign2D::Do2DStitching_AlignFrame(const bool detailSetupMode, const Ipvm::Rect32s& alignImageRoi)
{
    float fCalcTimeTotal_Copy = 0;

    Ipvm::TimeCheck TimeCheck;

    float fRotateAngle;
    Ipvm::Point32r2 fptCenter;
    Ipvm::Point32r2 fptShift;
    Ipvm::Point32r2 fptShift_Tot;

    Ipvm::Image8u curRawImage;

    if (!m_edgeAlign_inspection->getImageForAlign(*m_edgeAlign_para, true, curRawImage))
    {
        return FALSE;
    }

    Ipvm::Image8u curImageStitch;

    int32_t rawImageSizeX = getImageLot().GetImageSizeX();
    int32_t rawImageSizeY = getImageLot().GetImageSizeY();

    if (!getReusableMemory().GetByteImage(curImageStitch, rawImageSizeX, rawImageSizeY))
    {
        return FALSE;
    }

    if (detailSetupMode)
        curImageStitch.FillZero();

    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;
            auto& curStitchInfo = m_result.m_stitchRois[stitchIndex];

            if (x == 0 && y == 0)
            {
                //Ipvm::Rect32s ROI = Ipvm::Rect32s(0, 0, m_sensorSizeX, m_sensorSizeY);
                const auto& CurSimpleCopy = m_constants.GetSimpleCopyRoi(stitchIndex);
                Ipvm::ImageProcessing::Copy(
                    curRawImage, CurSimpleCopy, CurSimpleCopy + m_StichedOffset, curImageStitch);
            }
            else
            {
                fRotateAngle = (float)(-(curStitchInfo.GetAngle()));
                fptCenter = curStitchInfo.GetOrigin();
                fptShift = curStitchInfo.GetShift();
                fptShift_Tot = Ipvm::Conversion::ToPoint32r2(m_StichedOffset);

                doStitch_OneFrame(x, y, curRawImage, fptCenter, fRotateAngle, fptShift, fptShift_Tot, curImageStitch);
            }
        }
    }

    auto validImageCenter = Ipvm::Point32r2(alignImageRoi.Width() * 0.5f, alignImageRoi.Height() * 0.5f);

    Ipvm::Image8u image;
    if (!getReusableMemory().GetInspByteImage(image))
    {
        return FALSE;
    }

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(image), 0, image);
    CippModules::CopyValid(curImageStitch, alignImageRoi.TopLeft(), Ipvm::Point32s2(0, 0),
        Ipvm::Conversion::ToSize32s2(alignImageRoi), image);

    // 이미지 중심에서 Roi를 만들었으므로
    // 이미지 영역을 벗어나는 녀석들을 잘라도 이미지 중심이 Pane Center이다

    m_edgeAlign_inspection->getResult_EdgeAlign()->SetImage(image, validImageCenter);

    float fCalcTime = (float)TimeCheck.Elapsed_ms();
    fCalcTimeTotal_Copy += fCalcTime;

    return TRUE;
}

bool VisionProcessingAlign2D::FindAllStitchInfo_InRaw(const bool detailSetupMode)
{
    Ipvm::TimeCheck TimeCheck;

    if (!GetConstants())
    {
        return false;
    }

    bool success = false;
    switch (m_VisionPara.m_type)
    {
        case enumAlgorithmType::Matching:
            success = m_basedOnMatching->FindAllStitchInfo_InRaw(detailSetupMode);
            break;

        case enumAlgorithmType::BodySize:
            success = m_basedOnBodySize->FindAllStitchInfo_InRaw(detailSetupMode);
            break;
    }

    float fCalcTimeTotal_Matching = (float)TimeCheck.Elapsed_ms();

    CString strTemp;
    strTemp.Format(_T("Find StitchInfo Time : %.2f"), fCalcTimeTotal_Matching);
    //	AfxMessageBox(strTemp);

    return success;
}

bool VisionProcessingAlign2D::SetROIsInRaw()
{
    switch (m_VisionPara.m_type)
    {
        case enumAlgorithmType::Matching:
            return m_basedOnMatching->SetROIsInRaw();
        case enumAlgorithmType::BodySize:
            return m_basedOnBodySize->SetROIsInRaw();
    }
    return false;
}

bool VisionProcessingAlign2D::MakeDefaultTemplateROI_BaseOnMatching()
{
    if (!GetConstants())
    {
        return false;
    }

    return m_basedOnMatching->MakeDefaultTemplateROI();
}

// 단순히 복사해와야할 영역구하기
bool VisionProcessingAlign2D::GetCopyRegion(const long x, const long y, const Ipvm::Rect32s& verPreROIOverlap,
    const Ipvm::Rect32s& horPreROIOverlap, Ipvm::Point32s2& ptSrcCopyStart, Ipvm::Point32s2& ptDstCopyStart,
    Ipvm::Size32s2& roiCopySize)
{
    BOOL imageStitchingInterpolation = (m_VisionPara.m_boundaryInterpolation == 1); // TRUE;

    long stitchIndex = y * m_constants.m_stitchCountX + x;

    const auto& CurSimpleCopy = m_constants.GetSimpleCopyRoi(stitchIndex);

    const long nCopyOffsetX = m_constants.m_InterpolationOffsetX + m_constants.m_InterpolationLengthX;
    const long nCopyOffsetY = m_constants.m_InterpolationOffsetY + m_constants.m_InterpolationLengthY;

    if (imageStitchingInterpolation)
    {
        if (x == 0)
            ptDstCopyStart.m_x = verPreROIOverlap.m_left;
        else
            ptDstCopyStart.m_x = horPreROIOverlap.m_left + nCopyOffsetX;

        if (y == 0)
            ptDstCopyStart.m_y = horPreROIOverlap.m_top;
        else
            ptDstCopyStart.m_y = verPreROIOverlap.m_top + nCopyOffsetY;
    }
    else // StitchingInterpolation 을 안하는 경우.
    {
        if (x == 0)
            ptDstCopyStart.m_x = verPreROIOverlap.m_left;
        else
            ptDstCopyStart.m_x = horPreROIOverlap.CenterPoint().m_x;

        if (y == 0)
            ptDstCopyStart.m_y = horPreROIOverlap.m_top;
        ptDstCopyStart.m_y = verPreROIOverlap.CenterPoint().m_y;
    }

    ptSrcCopyStart.m_x = CurSimpleCopy.m_left;
    ptSrcCopyStart.m_y = CurSimpleCopy.m_top;

    roiCopySize.m_w = CurSimpleCopy.Width();
    roiCopySize.m_h = CurSimpleCopy.Height();

    return true;
}

bool VisionProcessingAlign2D::RotateShiftFOVImage(const long x, const long y, const long nFrameID,
    const Ipvm::Point32r2& ptCenter, const float fAngle, const Ipvm::Point32r2& pfShift, Ipvm::Image8u& dstImage)
{
    auto& curRawImage = getImageLot().GetImageFrame(nFrameID, GetCurVisionModule_Status());

    return RotateShiftFOVImage(x, y, curRawImage, ptCenter, fAngle, pfShift, dstImage);
}

bool VisionProcessingAlign2D::RotateShiftFOVImage(const long x, const long y, const Ipvm::Image8u& srcImageRaw,
    const Ipvm::Point32r2& ptCenter, const float fAngle, const Ipvm::Point32r2& pfShift, Ipvm::Image8u& dstImage)
{
    // Raw Image 에서 현재 FOV 영역의 Image pointer 가져오기
    Ipvm::Rect32s rtCurFOVImage((int32_t)(x * m_constants.m_sensorSizeX), (int32_t)(y * m_constants.m_sensorSizeY),
        (int32_t)((x + 1) * m_constants.m_sensorSizeX), (int32_t)((y + 1) * m_constants.m_sensorSizeY));
    const Ipvm::Image8u curRawFOVImage(srcImageRaw, rtCurFOVImage);

    Ipvm::Rect32s rectImage = Ipvm::Rect32s(0, 0, curRawFOVImage.GetSizeX(), curRawFOVImage.GetSizeY());
    auto error = Ipvm::ImageProcessing::RotateLinearInterpolation(
        curRawFOVImage, rectImage, ptCenter, -fAngle, pfShift, dstImage);
    VERIFY(error == Ipvm::Status::e_ok);

    return true;
}

bool VisionProcessingAlign2D::doStitch_OneFrame(long x, long y, const Ipvm::Image8u& srcImageRaw,
    const Ipvm::Point32r2& fptCenter, const float& fRotateAngle, const Ipvm::Point32r2& fptShift_FOV,
    const Ipvm::Point32r2& fptShift_Total, Ipvm::Image8u& dstImageStitch)
{
    BOOL imageStitchingInterpolation = (m_VisionPara.m_boundaryInterpolation == 1); // TRUE;
    BOOL bImageSave = m_VisionPara.m_saveTemplateDebugImage; //FALSE;

    long stitchIndex = y * m_constants.m_stitchCountX + x;

    //// Raw Image의 현재 FOV 영역만 Rotate & Shift 시키기  -> imageRotateSmall 에 복사됨.
    Ipvm::Image8u imageRotateSmall;
    if (!getReusableMemory().GetByteImage(imageRotateSmall, m_constants.m_sensorSizeX, m_constants.m_sensorSizeY))
        return FALSE;
    RotateShiftFOVImage(x, y, srcImageRaw, fptCenter, fRotateAngle, fptShift_FOV, imageRotateSmall);

    if (bImageSave)
    {
        Ipvm::ImageFile::SaveAsBmp(dstImageStitch, _debugImageFolder + _T("01.stitch_old.bmp"));
        Ipvm::ImageFile::SaveAsBmp(imageRotateSmall, _debugImageFolder + _T("02.srcRotated.bmp"));
    }

    // Stitching을 함으로써 preOverlapROI가 이동되는 영역을 계산한다.
    const auto& verPreROIOverlap = m_constants.GetVerPreOverlapROI(stitchIndex);
    const auto& horPreROIOverlap = m_constants.GetHorPreOverlapROI(stitchIndex);
    Ipvm::Rect32s shiftedVerPreROI, shiftedHorPreROI;

    UpdatePreOverlapROI(x, y, verPreROIOverlap, horPreROIOverlap, fptShift_Total, shiftedVerPreROI, shiftedHorPreROI);

    //// 단순 복사할 영역 복사하기
    // 단순히 복사해와야할 영역구하기
    Ipvm::Point32s2 ptSrcCopyStart;
    Ipvm::Point32s2 ptDstCopyStart;
    Ipvm::Size32s2 roiCopySize;
    GetCopyRegion(x, y, shiftedVerPreROI, shiftedHorPreROI, ptSrcCopyStart, ptDstCopyStart, roiCopySize);

    CippModules::CopyValid(imageRotateSmall, ptSrcCopyStart, ptDstCopyStart, roiCopySize, dstImageStitch);

    if (bImageSave)
    {
        Ipvm::ImageFile::SaveAsBmp(dstImageStitch, _debugImageFolder + _T("03.stitch_SimpleCopy.bmp"));

        Ipvm::Image8u srcCopyImage(m_constants.m_sensorSizeX, m_constants.m_sensorSizeY,
            imageRotateSmall.GetMem() + ptSrcCopyStart.m_x + (ptSrcCopyStart.m_y * m_constants.m_sensorSizeX),
            m_constants.m_sensorSizeX);

        Ipvm::ImageFile::SaveAsBmp(srcCopyImage, _debugImageFolder + _T("04.SrcCopyImage.bmp"));
    }

    if (x == 0 && y == 0)
        return true;

    // Stitching Interpolation 처리
    if (imageStitchingInterpolation)
    {
        if (x != 0)
        {
            InterpolationStitchX(x, y, shiftedVerPreROI, shiftedHorPreROI, imageRotateSmall, dstImageStitch);
            if (bImageSave)
            {
                Ipvm::ImageFile::SaveAsBmp(dstImageStitch, _debugImageFolder + _T("05-1.stitch_IP_X.bmp"));
            }
        }

        if (y != 0)
        {
            InterpolationStitchY(x, y, shiftedVerPreROI, shiftedHorPreROI, imageRotateSmall, dstImageStitch);
            if (bImageSave)
            {
                Ipvm::ImageFile::SaveAsBmp(dstImageStitch, _debugImageFolder + _T("05-2.stitch_IP_Y.bmp"));
            }
        }

        if (x != 0 && y != 0)
        {
            InterpolationStitchXY(shiftedVerPreROI, shiftedHorPreROI, imageRotateSmall, dstImageStitch);
        }
    }

    if (bImageSave)
    {
        Ipvm::ImageFile::SaveAsBmp(dstImageStitch, _debugImageFolder + _T("05-F.stitch_new.bmp"));
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
BOOL VisionProcessingAlign2D::Do2DAlignedStitchAll(const Ipvm::Rect32s& alignImageRoi, const bool /*detailSetupMode*/)
{
    float calculationTime_ms = 0;
    Ipvm::TimeCheck TimeCheck;

    // Align Angle 과 Device 중심위치를 가져온다.
    auto* alignResult = m_edgeAlign_inspection->getResult_EdgeAlign();
    float align_angle_rad = -alignResult->m_angle_rad;
    // align_angle = 5 *3.141592 / 180; (테스트코드)

    const double RadToDeg = 180 / ITP_PI;
    float align_angle_deg = (float)(align_angle_rad * RadToDeg);
    const float fCos = (float)cos(align_angle_rad);
    const float fSin = (float)sin(align_angle_rad);

    Ipvm::Point32r2 oneStitchCenterInAlignImage = m_edgeAlign_inspection->getResult_EdgeAlign()->getPaneCenter()
        + Ipvm::Conversion::ToPoint32r2(alignImageRoi.TopLeft());
    Ipvm::Point32r2 alignCenterInAlignImage
        = alignResult->m_center + Ipvm::Conversion::ToPoint32r2(alignImageRoi.TopLeft());

    // Device Center를 화면 정 중앙으로 이동시킨다.
    long inspImageSizeX = getReusableMemory().GetInspImageSizeX();
    long inspImageSizeY = getReusableMemory().GetInspImageSizeY();

    Ipvm::Point32r2 alignedShiftInAlignImage = oneStitchCenterInAlignImage - alignCenterInAlignImage;

    for (long frameIndex = 0; frameIndex < getImageLot().GetImageFrameCount(); frameIndex++)
    {
        if (frameIndex > (long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size())
            continue;

        const auto& rawImage = getImageLot().GetImageFrame(frameIndex, GetCurVisionModule_Status());
        auto& dstImage = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][frameIndex];

        dstImage.Create(inspImageSizeX, inspImageSizeY);

        for (long y = 0; y < m_constants.m_stitchCountY; y++)
        {
            for (long x = 0; x < m_constants.m_stitchCountX; x++)
            {
                long stitchIndex = y * m_constants.m_stitchCountX + x;
                auto& curStitchInfo = m_result.m_stitchRois[stitchIndex];

                float fRotateAngle = -(align_angle_deg + (float)curStitchInfo.GetAngle());

                // rotateOriginInFOV 는 영상회전시 사용. FOV 내에서의 회전 Origin 좌표
                Ipvm::Point32r2 rotateOriginInFOV;

                if (x == 0 && y == 0)
                {
                    rotateOriginInFOV
                        = Ipvm::Point32r2((float)m_constants.m_sensorSizeX / 2, (float)m_constants.m_sensorSizeY / 2);
                }
                else
                {
                    rotateOriginInFOV = curStitchInfo.GetOrigin();
                }

                // rotateOriginInAlignImage 는 Edge Align 용 이미지에서의 회전 Origin 좌표
                Ipvm::Point32r2 rotateOriginInAlignImage;
                rotateOriginInAlignImage.m_x
                    = rotateOriginInFOV.m_x + x * (m_constants.m_sensorSizeX - m_constants.m_overlapPixelX);
                rotateOriginInAlignImage.m_y
                    = rotateOriginInFOV.m_y + y * (m_constants.m_sensorSizeY - m_constants.m_overlapPixelY);
                rotateOriginInAlignImage.m_x += m_StichedOffset.m_x;
                rotateOriginInAlignImage.m_y += m_StichedOffset.m_y;
                rotateOriginInAlignImage.m_x += curStitchInfo.GetShift().m_x;
                rotateOriginInAlignImage.m_y += curStitchInfo.GetShift().m_y;

                // device aglign각을 보상하기 위해 FOV의 Origin을 기준으로 회전시킬때,
                // 전체 디바이스 중심기준으로 FOV 회전중심이 이동한 양

                // shiftByRotation 는 FOV 회전중심이 Device Alging 보상에 의해 이동한 양
                // shiftInFOV 는 영상회전시 사용. FOV 내에서의 이동해야 할 양
                Ipvm::Point32r2 shiftByRotation
                    = GetShift_FOVRotateCenter(rotateOriginInAlignImage, alignCenterInAlignImage, fCos, fSin);
                Ipvm::Point32r2 shiftInFOV = curStitchInfo.GetShift() + shiftByRotation;

                // shift_Total 는 Stitch 결과 영상이 이동된 양
                Ipvm::Point32r2 shift_Total = Ipvm::Conversion::ToPoint32r2(m_StichedOffset) + alignedShiftInAlignImage;
                shift_Total.m_x -= alignImageRoi.m_left;
                shift_Total.m_y -= alignImageRoi.m_top;

                doStitch_OneFrame(x, y, rawImage, rotateOriginInFOV, fRotateAngle, shiftInFOV, shift_Total, dstImage);
            }
        }
    }

    float fCalcTime = (float)TimeCheck.Elapsed_ms();
    calculationTime_ms += fCalcTime;

    return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
BOOL VisionProcessingAlign2D::Do2DAlignAllImages_NoStitch(
    const Ipvm::Rect32s& alignImageRoi, const bool /*detailSetupMode*/)
{
    float fCalcTimeTotal_Copy = 0;
    Ipvm::TimeCheck TimeCheck;

    // Align Angle 과 Device 중심위치를 가져온다.
    auto* alignResult = m_edgeAlign_inspection->getResult_EdgeAlign();

    const double RadToDeg = 180 / ITP_PI;
    float alignedAngle_deg = (float)(alignResult->m_angle_rad * RadToDeg);
    Ipvm::Point32r2 fptAlignedCenter = alignResult->m_center;

    // Align은 Raw Pane 영역을 Cutting해서 진행했으므로 Pane영역을 더해주자
    fptAlignedCenter.m_x += alignImageRoi.m_left;
    fptAlignedCenter.m_y += alignImageRoi.m_top;

    // Device Center를 화면 정 중앙으로 이동시킨다.
    int inspImageSizeX = getReusableMemory().GetInspImageSizeX();
    int inspImageSizeY = getReusableMemory().GetInspImageSizeY();
    Ipvm::Point32r2 rawImageCenter(getImageLot().GetImageSizeX() * 0.5f, getImageLot().GetImageSizeY() * 0.5f);
    Ipvm::Point32r2 inspImageCenter(inspImageSizeX * 0.5f, inspImageSizeY * 0.5f);
    Ipvm::Point32r2 fptAlignedShift = inspImageCenter - fptAlignedCenter;

    long paneID = GetCurrentPaneID();
    auto& scale = getImageLot().GetScale();

    Ipvm::Rect32s rawPaneRoi(0, 0, 0, 0);
    const auto& inspectionAreaInfo = getInspectionAreaInfo();

    // Pane 영역 얻기
    for (auto& fovIndex : inspectionAreaInfo.m_fovList)
    {
        if (paneID >= 0 && paneID < (long)inspectionAreaInfo.m_unitIndexList.size())
        {
            long unitID = inspectionAreaInfo.m_unitIndexList[paneID];
            auto pocketRegion
                = scale.convert_mmToPixel(inspectionAreaInfo.m_parent->GetUnitPocketRegionInFOV(fovIndex, unitID))
                + Ipvm::Conversion::ToPoint32s2(rawImageCenter);
            rawPaneRoi |= pocketRegion;
        }
    }

    Ipvm::Rect32s inspRoi = Ipvm::Rect32s(0, 0, inspImageSizeX, inspImageSizeY);

    for (long i = 0; i < getImageLot().GetImageFrameCount(); i++)
    {
        if (i > (long)getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size())
            continue;

        Ipvm::Image8u curImageStitch;
        auto& curRawImage = getImageLot().GetImageFrame(i, GetCurVisionModule_Status());

        curImageStitch.Create(inspImageSizeX, inspImageSizeY);
        curImageStitch.FillZero();

        auto error = Ipvm::ImageProcessing::RotateLinearInterpolation(
            curRawImage, rawPaneRoi, fptAlignedCenter, -alignedAngle_deg, fptAlignedShift, curImageStitch);

        VERIFY(error == Ipvm::Status::e_ok);

        // 검사 영역만 실제 검사영상으로 남기자
        getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][i] = Ipvm::Image8u(curImageStitch, inspRoi);
    }

    float fCalcTime = (float)TimeCheck.Elapsed_ms();
    fCalcTimeTotal_Copy += fCalcTime;

    return TRUE;
}

bool VisionProcessingAlign2D::UpdatePreOverlapROI(const long x, const long y, const Ipvm::Rect32s& verPreROIOverlap,
    const Ipvm::Rect32s& horPreROIOverlap, const Ipvm::Point32r2& fptShift_Total, Ipvm::Rect32s& shiftedVerPreROI,
    Ipvm::Rect32s& shiftedHorPreROI)
{
    Ipvm::Point32s2 AccumulateOffset(x * m_constants.m_overlapPixelX, y * m_constants.m_overlapPixelY);

    shiftedVerPreROI = verPreROIOverlap + Ipvm::Conversion::ToPoint32s2(fptShift_Total);
    shiftedHorPreROI = horPreROIOverlap + Ipvm::Conversion::ToPoint32s2(fptShift_Total);

    // Stiching 을 수행함에 따라서 누적되는 이동량을 계산한다.
    if (y == 0)
    {
        if (x > 1)
            shiftedHorPreROI -= Ipvm::Point32s2((x - 1) * m_constants.m_overlapPixelX, 0);
    }
    else if (x == 0)
    {
        if (y > 1)
            shiftedVerPreROI -= Ipvm::Point32s2(0, (y - 1) * m_constants.m_overlapPixelY);
    }
    else
    {
        shiftedHorPreROI -= Ipvm::Point32s2((x - 1) * m_constants.m_overlapPixelX, y * m_constants.m_overlapPixelY);
        shiftedVerPreROI -= Ipvm::Point32s2(x * m_constants.m_overlapPixelX, (y - 1) * m_constants.m_overlapPixelY);
    }

    return true;
}

// Stitch할 영상의 회전중심이 Align회전각에 의해 이동하는 값 구하기
Ipvm::Point32r2 VisionProcessingAlign2D::GetShift_FOVRotateCenter(
    const Ipvm::Point32r2& fptRotateCenter, const Ipvm::Point32r2& fptAlignedCenter, const float fCos, const float fSin)
{
    float fX0 = fptRotateCenter.m_x;
    float fY0 = fptRotateCenter.m_y;
    float fXc = fptAlignedCenter.m_x;
    float fYc = fptAlignedCenter.m_y;

    float fX1 = fXc + fCos * (fX0 - fXc) - fSin * (fY0 - fYc);
    float fY1 = fYc + fSin * (fX0 - fXc) + fCos * (fY0 - fYc);

    return Ipvm::Point32r2(fX1 - fX0, fY1 - fY0);
}

bool VisionProcessingAlign2D::InterpolationStitchX(long x, long y, const Ipvm::Rect32s& verPreROIOverlap,
    const Ipvm::Rect32s& horPreROIOverlap, Ipvm::Image8u& srcImageSmall, Ipvm::Image8u& dstImageTotal)
{
    UNREFERENCED_PARAMETER(verPreROIOverlap);

    long stitchIndex = y * m_constants.m_stitchCountX + x;
    const auto& CurSimpleCopy = m_constants.GetSimpleCopyRoi(stitchIndex);

    long offsetY = 0; // srcImageSmall image에서 Interpolation 시작점
    if (y != 0)
        offsetY = m_constants.m_InterpolationOffsetY
            + m_constants.m_InterpolationLengthY; // InterpolationXY 영역은 제외한다.

    long preStartX = horPreROIOverlap.m_left + m_constants.m_InterpolationOffsetX;
    long preStartY = horPreROIOverlap.m_top + offsetY;

    long curStartX = m_constants.m_InterpolationOffsetX;
    long curStartY = offsetY;
    long lengthX = m_constants.m_InterpolationLengthX + 1;
    long lengthY = CurSimpleCopy.Height(); // Interpolation 할 길이

    BOOL bInterpolationTest = FALSE;
    if (bInterpolationTest)
    {
        Ipvm::ImageFile::SaveAsBmp(dstImageTotal, _debugImageFolder + _T("Interpolation_dstTotal_0.bmp"));
        Ipvm::ImageFile::SaveAsBmp(srcImageSmall, _debugImageFolder + _T("Interpolation_srcSmall_0.bmp"));
    }

    // Interpolation 계산할 영역이 dstImageTotal 이미지의 영역을 벗어나지 않게 하자
    long xmin = max(0, -preStartX);
    long ymin = max(0, -preStartY);
    long xmax = min(lengthX, dstImageTotal.GetSizeX() - preStartX);
    long ymax = min(lengthY, dstImageTotal.GetSizeY() - preStartY);
    lengthX = xmax - xmin;
    lengthY = ymax - ymin;

    for (long iy = ymin; iy < ymax; iy++)
    {
        auto* image_y = dstImageTotal.GetMem(preStartX, iy + preStartY);
        auto* rotate_y = srcImageSmall.GetMem(curStartX, iy + curStartY);

        for (long ix = xmin; ix < xmax; ix++)
        {
            long value = (image_y[ix] * (lengthX - ix) + rotate_y[ix] * ix) / lengthX;
            image_y[ix] = (BYTE)value;
        }
    }

    if (bInterpolationTest)
    {
        Ipvm::ImageFile::SaveAsBmp(dstImageTotal, _debugImageFolder + _T("Interpolation_dstTotal_1.bmp"));
    }

    return true;
}

bool VisionProcessingAlign2D::InterpolationStitchY(long x, long y, const Ipvm::Rect32s& verPreROIOverlap,
    const Ipvm::Rect32s& horPreROIOverlap, Ipvm::Image8u& srcImageSmall, Ipvm::Image8u& dstImageTotal)
{
    UNREFERENCED_PARAMETER(horPreROIOverlap);

    long stitchIndex = y * m_constants.m_stitchCountX + x;
    const auto& CurSimpleCopy = m_constants.GetSimpleCopyRoi(stitchIndex);

    long offsetX = 0; // srcImageSmall image에서 Interpolation 시작점
    if (x != 0)
        offsetX = m_constants.m_InterpolationOffsetX
            + m_constants.m_InterpolationLengthX; // InterpolationXY 영역은 제외한다.

    long preStartX = verPreROIOverlap.m_left + offsetX;
    long preStartY = verPreROIOverlap.m_top + m_constants.m_InterpolationOffsetY;

    long curStartX = offsetX;
    long curStartY = m_constants.m_InterpolationOffsetY;
    long lengthX = CurSimpleCopy.Width(); // x==0 일때 Interpolation 할 길이
    long lengthY = m_constants.m_InterpolationLengthY + 1;

    BOOL bInterpolationTest = FALSE;
    if (bInterpolationTest)
    {
        Ipvm::ImageFile::SaveAsBmp(dstImageTotal, _debugImageFolder + _T("Interpolation_dstTotal_0.bmp"));
        Ipvm::ImageFile::SaveAsBmp(srcImageSmall, _debugImageFolder + _T("Interpolation_srcSmall_0.bmp"));
    }

    // Interpolation 계산할 영역이 dstImageTotal 이미지의 영역을 벗어나지 않게 하자
    long xmin = max(0, -preStartX);
    long ymin = max(0, -preStartY);
    long xmax = min(lengthX, dstImageTotal.GetSizeX() - preStartX);
    long ymax = min(lengthY, dstImageTotal.GetSizeY() - preStartY);
    lengthX = xmax - xmin;
    lengthY = ymax - ymin;

    for (long iy = ymin; iy < ymax; iy++)
    {
        auto* image_y = dstImageTotal.GetMem(preStartX, iy + preStartY);
        auto* rotate_y = srcImageSmall.GetMem(curStartX, iy + curStartY);

        for (long ix = xmin; ix < xmax; ix++)
        {
            long value = (image_y[ix] * (lengthY - iy) + rotate_y[ix] * iy) / lengthY;
            image_y[ix] = (BYTE)value;
        }
    }

    if (bInterpolationTest)
    {
        Ipvm::ImageFile::SaveAsBmp(dstImageTotal, _debugImageFolder + _T("Interpolation_dstTotal_1.bmp"));
    }

    return true;
}

bool VisionProcessingAlign2D::InterpolationStitchXY(const Ipvm::Rect32s& verPreROIOverlap,
    const Ipvm::Rect32s& horPreROIOverlap, Ipvm::Image8u& srcImageSmall, Ipvm::Image8u& dstImageTotal)
{
    long preStartX = horPreROIOverlap.m_left + m_constants.m_InterpolationOffsetX;
    long preStartY = verPreROIOverlap.m_top + m_constants.m_InterpolationOffsetY;

    long curStartX = m_constants.m_InterpolationOffsetX;
    long curStartY = m_constants.m_InterpolationOffsetY;

    long lengthX = m_constants.m_InterpolationLengthX + 1;
    long lengthY = m_constants.m_InterpolationLengthY + 1;

    // Left - Top
    // Interpolation 계산할 영역이 dstImageTotal 이미지의 영역을 벗어나지 않게 하자
    long xmin = max(0, -preStartX);
    long ymin = max(0, -preStartY);
    long xmax = min(lengthX, dstImageTotal.GetSizeX() - preStartX);
    long ymax = min(lengthY, dstImageTotal.GetSizeY() - preStartY);
    lengthX = xmax - xmin;
    lengthY = ymax - ymin;

    long xylength = lengthX * lengthY;
    for (long iy = ymin; iy < ymax; iy++)
    {
        auto* image_y = dstImageTotal.GetMem(preStartX, iy + preStartY);
        auto* rotate_y = srcImageSmall.GetMem(curStartX, iy + curStartY);

        for (long ix = xmin; ix < xmax; ix++)
        {
            long value = image_y[ix] + (rotate_y[ix] - image_y[ix]) * ix * iy / (xylength);
            image_y[ix] = (BYTE)value;
        }
    }

    return true;
}
