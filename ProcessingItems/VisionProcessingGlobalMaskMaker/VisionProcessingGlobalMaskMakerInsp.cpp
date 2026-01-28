//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingGlobalMaskMaker.h"

//CPP_2_________________________________ This project's headers
#include "DlgSetupUI.h"
#include "ResultData.h"
#include "VisionProcessingGlobalMaskMakerPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingGlobalMaskMaker::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionProcessingGlobalMaskMaker::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    if (!CreateLayersBeforeIndex(detailSetupMode, m_VisionPara->getLayerCount()))
    {
        return FALSE;
    }

    return TRUE;
}

bool VisionProcessingGlobalMaskMaker::GenerateLayers(
    const bool /*detailSetupMode*/, long limitLayerIndex, Ipvm::Image8u* o_totalImage)
{
    const auto& px2um = getScale().pixelToUm();

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    Ipvm::Rect32s operationROI = GetOperationRegion();

    for (long layer = 0; layer < min(limitLayerIndex, m_VisionPara->getLayerCount()); layer++)
    {
        auto& layerSpec = m_VisionPara->getLayer(layer);

        m_result->m_layers.emplace_back(layerSpec, px2um, imageCenter);
        auto& layerResult = m_result->m_layers[long(m_result->m_layers.size()) - 1];

        if (!CreateSingleLayer(layer, layerResult))
        {
            return false;
        }

        if (o_totalImage)
        {
            auto* layerMask = getReusableMemory().AddSurfaceLayerMaskClass(layerResult.m_surfaceMaskName);
            if (layerMask)
            {
                auto& maskImage = layerMask->GetMask();
                Ipvm::ImageProcessing::BitwiseOr(maskImage, operationROI, *o_totalImage);
            }
        }
    }

    return true;
}

bool VisionProcessingGlobalMaskMaker::CreateSetupImage(long imageIndex, Ipvm::Image8u& o_image)
{
    Ipvm::Image8u currentImage;
    if (imageIndex < 0)
    {
        currentImage = GetInspectionFrameImage();
    }
    else if (imageIndex >= 0 && imageIndex < long(getImageLotInsp().m_vecImages[GetCurVisionModule_Status()].size()))
    {
        currentImage = getImageLotInsp().m_vecImages[GetCurVisionModule_Status()][imageIndex];
    }

    if (currentImage.GetMem() == nullptr)
        return false;

    if (!getReusableMemory().GetInspByteImage(o_image))
        return false;

    Ipvm::ImageProcessing::Copy(currentImage, Ipvm::Rect32s(currentImage), o_image);

    return true;
}

bool VisionProcessingGlobalMaskMaker::CreateLayersBeforeIndex(const bool detailSetupMode, long limitLayerIndex)
{
    m_inspErrorMessage = _T("");

    Ipvm::TimeCheck fTime;

    ResetResult();

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    // Init...
    GetEdgeAlignResult();

    Ipvm::Image8u totalMaskImage;

    if (!getReusableMemory().GetInspByteImage(totalMaskImage))
    {
        return FALSE;
    }

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(totalMaskImage), 0, totalMaskImage);

    m_result->m_layers.clear();

    bool success = GenerateLayers(detailSetupMode, limitLayerIndex, &totalMaskImage);

    if (!success)
    {
        m_bInvalid = TRUE;
    }

    SetDebugInfoItem(detailSetupMode, _T("Total Mask"), totalMaskImage);

    m_fCalcTime = CAST_FLOAT(fTime.Elapsed_ms());

    return success;
}

bool VisionProcessingGlobalMaskMaker::CreateSingleLayer(long currentLayerIndex, ResultLayer& layer)
{
    Ipvm::Image8u maskImage;
    Ipvm::Rect32s operationROI = GetOperationRegion();

    if (!getReusableMemory().GetInspByteImage(maskImage))
        return false;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(maskImage), 0, maskImage);

    for (auto& roi : layer.m_pre.m_userROIs)
    {
        if (roi.m_type == UserRoiType::PreparedMask)
        {
            if (!CreatePreparedMask(roi, currentLayerIndex, layer, maskImage))
            {
                return false;
            }

            continue;
        }

        BYTE fillValue = (roi.m_operation != OperationType::Minus) ? 255 : 0;

        bool needPostProcessing = false;
        Ipvm::Image8u preMask;
        if (!getReusableMemory().GetInspByteImage(preMask))
            return false;

        if (roi.m_operation == OperationType::Plus || roi.m_operation == OperationType::Minus)
        {
            // Plus, Minus는 대상 마스크에 바로 처리해 버리자
            preMask = maskImage;
        }
        else
        {
            Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(preMask), 0, preMask);
            needPostProcessing = true;
        }

        switch (roi.m_type)
        {
            case UserRoiType::Rect:
                Ipvm::ImageProcessing::Fill(roi.m_rect, fillValue, preMask);
                break;

            case UserRoiType::Ellipse_32f:
                if (1)
                {
                    Ipvm::EllipseEq32r ellipse;
                    ellipse.m_x = (roi.m_rect.m_left + roi.m_rect.m_right) * 0.5f;
                    ellipse.m_y = (roi.m_rect.m_top + roi.m_rect.m_bottom) * 0.5f;
                    ellipse.m_xradius = roi.m_rect.Width() * 0.5f;
                    ellipse.m_yradius = roi.m_rect.Height() * 0.5f;
                    Ipvm::ImageProcessing::Fill(ellipse, fillValue, preMask);
                }
                break;

            case UserRoiType::Polygon_32f:
                if (1)
                {
                    Ipvm::Polygon32r polygon;
                    polygon.SetVertexNum(long(roi.m_polygon.size()));

                    for (long n = 0; n < long(roi.m_polygon.size()); n++)
                    {
                        polygon.GetVertices()[n] = Ipvm::Conversion::ToPoint32r2(roi.m_polygon[n]);
                    }
                    Ipvm::ImageProcessing::Fill(polygon, fillValue, preMask);
                }
                break;
        }

        if (needPostProcessing)
        {
            switch (roi.m_operation)
            {
                case OperationType::And:
                    Ipvm::ImageProcessing::BitwiseAnd(preMask, Ipvm::Rect32s(preMask), maskImage);
                    break;
            }
        }
    }

    Ipvm::Point32r2 imageCenter(
        getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

    layer.m_surfaceMaskName = GetSurfaceMaskNameFromLayerName(layer.m_name);
    auto* layerMask = getReusableMemory().AddSurfaceLayerMaskClass(layer.m_surfaceMaskName);
    if (layerMask)
    {
        layerMask->Reset();
        layerMask->Add(maskImage);
    }

    return true;
}

bool VisionProcessingGlobalMaskMaker::isLayerValid(long layerIndex)
{
    std::map<CString, BOOL> layerListBefore;

    auto& layerInfo = m_VisionPara->getLayer(layerIndex);

    for (long index = 0; index < layerIndex; index++)
    {
        layerListBefore[m_VisionPara->getLayer(index).m_name] = index;
    }

    std::map<CString, BOOL> checkList;

    for (long item = 0; item < layerInfo.m_single.count(); item++)
    {
        auto& roiInfo = layerInfo.m_single[item];
        if (roiInfo.getType() != UserRoiType::PreparedMask)
            continue;

        // 내부 내용중
        auto itList = layerListBefore.find(roiInfo.m_preparedObjectName);
        if (itList != layerListBefore.end())
        {
            if (!isLayerValid(itList->second))
            {
                return false;
            }
        }
        else
        {
            checkList[roiInfo.m_preparedObjectName] = TRUE;
        }
    }

    //----------------------------------------------------------------------------
    // Global Mask Maker 가 만들어 낸 Layer를 빼고 사용가능한 Mask 이름을 수집한다
    //----------------------------------------------------------------------------

    CString maskHeader = GetSurfaceMaskNameHeader();

    for (long maskIndex = 0; maskIndex < getReusableMemory().GetSurfaceLayerMaskCount(); maskIndex++)
    {
        CString name = getReusableMemory().GetSurfaceLayerMaskName(maskIndex);

        if (name.Left(maskHeader.GetLength()) != maskHeader)
        {
            checkList.erase(name);
        }

        name.Empty();
    }

    for (long roiIndex = 0; roiIndex < getReusableMemory().GetSurfaceLayerRoiCount(); roiIndex++)
    {
        CString name = getReusableMemory().GetSurfaceLayerRoiName(roiIndex);

        if (name.Left(maskHeader.GetLength()) != maskHeader)
        {
            checkList.erase(name);
        }

        name.Empty();
    }

    maskHeader.Empty();

    // 체크가 필요한 항목이 없다면 필요한 것이 모두 있는 것이다
    return checkList.size() == 0;
}

bool VisionProcessingGlobalMaskMaker::CreatePreparedMask(
    const ResultUserROI& userRoi, long currentLayerIndex, ResultLayer& layer, Ipvm::Image8u& io_maskImage)
{
    float um2px_x = getScale().umToPixel().m_x;
    float um2px_y = getScale().umToPixel().m_y;
    float um2px = (um2px_x + um2px_y) * 0.5f;
    long layerIndex = m_VisionPara->getLayerIndex(userRoi.m_preparedObjectName);
    const Ipvm::Image8u* preMask = nullptr;

    if (layerIndex >= 0)
    {
        if (layerIndex >= currentLayerIndex)
        {
            // 순서상 선택할 수 없는 Layer가 선택되었다.
            return false;
        }

        preMask
            = getReusableMemory().GetSurfaceLayerMask(GetSurfaceMaskNameFromLayerName(userRoi.m_preparedObjectName));
    }
    else
    {
        preMask = getReusableMemory().GetSurfaceLayerMask(userRoi.m_preparedObjectName);
    }

    if (preMask)
    {
        Ipvm::Image8u procMask;
        if (!getReusableMemory().GetInspByteImage(procMask))
            return false;

        Ipvm::ImageProcessing::Copy(*preMask, Ipvm::Rect32s(*preMask), procMask);

        auto dilate = long(userRoi.m_preparedMaskDilateInUm * um2px + 0.5f);

        if (dilate < 0)
        {
            Ipvm::ImageProcessing::MorphErodeIter(Ipvm::Rect32s(procMask), 3, 3, min(500, abs(dilate)), procMask);
        }
        else if (dilate > 0)
        {
            Ipvm::ImageProcessing::MorphDilateIter(Ipvm::Rect32s(procMask), 3, 3, min(500, abs(dilate)), procMask);
        }

        switch (userRoi.m_operation)
        {
            case OperationType::Plus:
                Ipvm::ImageProcessing::Add(procMask, Ipvm::Rect32s(procMask), 0, io_maskImage);
                break;

            case OperationType::Minus:
                Ipvm::ImageProcessing::Subtract(procMask, Ipvm::Rect32s(procMask), 0, io_maskImage);
                break;

            case OperationType::And:
                Ipvm::ImageProcessing::BitwiseAnd(procMask, Ipvm::Rect32s(procMask), io_maskImage);
                break;
        }

        return true;
    }

    if (auto* layerRoi = getReusableMemory().GetSurfaceLayerRoiClass(userRoi.m_preparedObjectName))
    {
        long expandX_px = long(userRoi.m_preparedROIExpandX_um * um2px_x + 0.5f);
        long expandY_px = long(userRoi.m_preparedROIExpandY_um * um2px_y + 0.5f);

        switch (userRoi.m_operation)
        {
            case OperationType::Plus:
                layerRoi->AddToMask(expandX_px, expandY_px, io_maskImage);
                break;

            case OperationType::Minus:
                layerRoi->SubtractToMask(expandX_px, expandY_px, io_maskImage);
                break;

            case OperationType::And:
                if (1)
                {
                    Ipvm::Image8u procMask;
                    if (!getReusableMemory().GetInspByteImage(procMask))
                        return false;

                    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(procMask), 0, procMask);
                    layerRoi->AddToMask(expandX_px, expandY_px, procMask);

                    Ipvm::ImageProcessing::BitwiseAnd(procMask, Ipvm::Rect32s(procMask), io_maskImage);
                }
                break;
        }

        return true;
    }

    m_inspErrorMessage.AppendFormat(_T("%s Single layer : Prepared object not found - %s\n"), (LPCTSTR)layer.m_name,
        (LPCTSTR)userRoi.m_preparedObjectName);

    return false;
}

void VisionProcessingGlobalMaskMaker::GetEdgeAlignResult()
{
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_sEdgeAlignResult = *(VisionAlignResult*)pData;
}

Ipvm::Rect32s VisionProcessingGlobalMaskMaker::GetOperationRegion()
{
    return Ipvm::Rect32s(0, 0, getReusableMemory().GetInspImageSizeX(), getReusableMemory().GetInspImageSizeY());
}
