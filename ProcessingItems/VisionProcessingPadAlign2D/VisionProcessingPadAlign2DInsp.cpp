//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingPadAlign2D.h"

//CPP_2_________________________________ This project's headers
#include "AlignPara.h"
#include "DlgVisionProcessingPadAlign2D.h"
#include "ParaDB.h"
#include "Result.h"
#include "Utility.h"
#include "VisionProcessingPadAlign2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/EdgeDetection.h>
#include <Ipvm/Algorithm/EdgeDetectionFilter.h>
#include <Ipvm/Algorithm/EdgeDetectionPara.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image32u.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Point32r3.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
#include <cmath>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingPadAlign2D::OnInspection()
{
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    return TRUE;
}

BOOL VisionProcessingPadAlign2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    Ipvm::TimeCheck time;
    m_errorLogText.Empty();

    ResetResult();
    checkJobParameter();

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    BOOL success = FALSE;
    if (align(detailSetupMode))
    {
        success = TRUE;
    }
    else
    {
        m_bInvalid = TRUE;
        success = FALSE;
    }

    if (!collect_debugResult(detailSetupMode))
    {
        success = FALSE;
    }

    m_fCalcTime = CAST_FLOAT(time.Elapsed_ms());

    return success;
}

bool VisionProcessingPadAlign2D::getBlobThresholdImage(
    LPCTSTR padName, Ipvm::Image8u& o_image, Ipvm::Rect32s& o_searchRoi)
{
    const auto& padInfos = m_packageSpec.m_PadMapManager;
    Ipvm::Point32r2 bodyCenter;

    if (!getBodyCenter(bodyCenter))
        return false;
    auto* para = m_VisionPara->GetAlignPAD_Para(padName);
    if (para == nullptr)
        return false;

    m_result->m_bodyCenterBefore = bodyCenter;
    make_pad_imageCoordinates(bodyCenter, *padInfos);

    // 검사에 사용될 Image 만듬
    Ipvm::Image8u imageCombine;

    if (!getReusableMemory().GetInspByteImage(imageCombine))
        return false;
    if (!getReusableMemory().GetInspByteImage(o_image))
        return false;

    Ipvm::Image8u image1 = GetInspectionFrameImage(FALSE, GetImageFrameIndex(0));

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image1, false, Ipvm::Rect32s(image1), para->m_imageProc, imageCombine))
    {
        return false;
    }

    Ipvm::ImageProcessing::Copy(imageCombine, Ipvm::Rect32s(imageCombine), o_image);

    long searchRangeX = max(0, (long)getScale().convert_umToPixelX(para->m_blobSearchOffset_um));
    long searchRangeY = max(0, (long)getScale().convert_umToPixelY(para->m_blobSearchOffset_um));

    for (long padIndex = 0; padIndex < long(m_result->m_pads.size()); padIndex++)
    {
        auto& padInfo = m_result->m_pads[padIndex];
        if (padInfo.m_name == padName)
        {
            auto search = padInfo.m_align_px.GetCRect();
            search.InflateRect(searchRangeX, searchRangeY);
            search &= Ipvm::Rect32s(image1);

            o_searchRoi = search;

            Ipvm::ImageProcessing::BinarizeGreater(
                imageCombine, search, (BYTE)min(255, max(0, para->m_blobThreshold)), o_image);

            return true;
        }
    }

    return false;
}

bool VisionProcessingPadAlign2D::align(const bool /*detailSetupMode*/)
{
    auto* layerMask = getSurfaceLayerMask();
    if (layerMask)
    {
        layerMask->Reset();
    }

    const auto& padInfos = m_packageSpec.m_PadMapManager;

    Ipvm::Point32r2 bodyCenter;

    if (!getBodyCenter(bodyCenter))
        return false;

    m_result->m_bodyCenterBefore = bodyCenter;
    make_pad_imageCoordinates(bodyCenter, *padInfos);

    m_result->m_applyAlignTobodyCenter = bodyCenter;

    Result_Pad mark1, mark2;
    Ipvm::Point32r2 fiducial_offset(0.f, 0.f);
    float fiducial_degree(0.f);

    if (!find_pad(bodyCenter, true))
        return false; // Fiducial 먼저 Align

    if (getSpec_fiducialMark2(m_result->m_pads, mark1, mark2))
    {
        // Fiducial로 설정되어 있는 Mark 가 두개 이상 있을 때만 Fiducial Align을 시도한다
        if (!calculation_offset_degree(bodyCenter, mark1, mark2, fiducial_offset, fiducial_degree))
            return false;
    }

    if (!apply_other_pad_from_fiducial(bodyCenter, fiducial_offset, fiducial_degree, m_result->m_pads))
    {
        return false;
    }

    if (!find_pad(bodyCenter, false))
        return false; // Fiducial 빼고 나머지 Align

    return true;
}

bool VisionProcessingPadAlign2D::collect_debugResult(const bool detailSetupMode)
{
    SetDebugInfoItem(detailSetupMode, _T("Body Center"), m_result->m_bodyCenterBefore);
    SetDebugInfoItem(detailSetupMode, _T("Spec PAD ROIs"), m_result->m_debugResult_spec_pad_rois);
    SetDebugInfoItem(detailSetupMode, _T("Edge Source ROIs"), m_result->m_debugResult_edgeSourceRois);
    SetDebugInfoItem(detailSetupMode, _T("Edge Detection Lines"), m_result->m_debugResult_edgeDetectionLines);
    SetDebugInfoItem(detailSetupMode, _T("Edge Points"), m_result->m_debugResult_edgePoints);
    SetDebugInfoItem(detailSetupMode, _T("Align PAD Blobs"), m_result->m_debugResult_align_Blobs, true);
    SetDebugInfoItem(detailSetupMode, _T("Align PAD Ellipses"), m_result->m_debugResult_align_Ellipses, true);
    SetDebugInfoItem(detailSetupMode, _T("Align PAD Polygons"), m_result->m_debugResult_align_Polygons, true);
    SetDebugInfoItem(detailSetupMode, _T("Align PAD Other"), m_result->m_debugResult_align_Other, true);
    SetDebugInfoItem(detailSetupMode, _T("Align PAD Center"), m_result->m_debugResult_align_Center, true);
    SetDebugInfoItem(detailSetupMode, _T("Apply PAD Align to Body Center"), m_result->m_applyAlignTobodyCenter, true);

    if (getSurfaceLayerMask())
    {
        SetDebugInfoItem(detailSetupMode, _T("Align PAD Image"), getSurfaceLayerMask()->GetMask());
    }

    return true;
}

bool VisionProcessingPadAlign2D::getBodyCenter(Ipvm::Point32r2& o_center)
{
    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_FIDUCIAL_ALIGN, _T("Apply PAD Align to Body Center"), nDataNum);

    if (pData != nullptr && nDataNum > 0)
    {
        Ipvm::Point32r2* BodyCenter;

        BodyCenter = (Ipvm::Point32r2*)pData;

        o_center = Ipvm::Point32r2(BodyCenter->m_x, BodyCenter->m_y);
    }
    else
    {
        Ipvm::Point32r2* BodyCenter;

        BodyCenter = new Ipvm::Point32r2(
            getReusableMemory().GetInspImageSizeX() * 0.5f, getReusableMemory().GetInspImageSizeY() * 0.5f);

        o_center = Ipvm::Point32r2(BodyCenter->m_x, BodyCenter->m_y);

        delete BodyCenter;
    }

    return true;
}

void VisionProcessingPadAlign2D::make_pad_imageCoordinates(
    const Ipvm::Point32r2& i_bodyCenter, const Package::PadCollection& i_padInfo)
{
    m_result->m_pads.resize(i_padInfo.vecPadData.size());
    m_result->m_debugResult_align_Center.resize(i_padInfo.vecPadData.size());

    for (long padIndex = 0; padIndex < long(i_padInfo.vecPadData.size()); padIndex++)
    {
        auto& padInfo = i_padInfo.vecPadData[padIndex];

        make_pad_imageCoordinate(i_bodyCenter, padInfo, m_result->m_pads[padIndex]);

        m_result->m_debugResult_spec_pad_rois.push_back(m_result->m_pads[padIndex].m_spec_px);
        m_result->m_debugResult_align_Center[padIndex] = m_result->m_pads[padIndex].m_spec_px.GetCenter();
    }
}

void VisionProcessingPadAlign2D::make_pad_imageCoordinate(
    const Ipvm::Point32r2& i_bodyCenter, const Package::Pad& i_padSpec, Result_Pad& o_padPos)
{
    const auto& mmToPixel = getScale().mmToPixel();

    FPI_RECT specROI;
    float halfSizeX = i_padSpec.fWidth * 0.5f;
    float halfSizeY = i_padSpec.fLength * 0.5f;

    if (i_padSpec.GetType() == PAD_TYPE::_typeCircle)
    {
        // Circle은 Width만 존재한다
        halfSizeY = halfSizeX;
    }

    auto& padCenter = i_padSpec.GetCenter();

    specROI.fptLT.m_x = padCenter.m_x - halfSizeX;
    specROI.fptLT.m_y = padCenter.m_y - halfSizeY;
    specROI.fptRT.m_x = padCenter.m_x + halfSizeX;
    specROI.fptRT.m_y = padCenter.m_y - halfSizeY;
    specROI.fptLB.m_x = padCenter.m_x - halfSizeX;
    specROI.fptLB.m_y = padCenter.m_y + halfSizeY;
    specROI.fptRB.m_x = padCenter.m_x + halfSizeX;
    specROI.fptRB.m_y = padCenter.m_y + halfSizeY;

    specROI.Rotate(i_padSpec.nAngle * DEF_DEG_TO_RAD, Ipvm::Point32r2(0.f, 0.f));

    o_padPos.m_enable = i_padSpec.bIgnore ? false : true;
    o_padPos.m_name = i_padSpec.strPadName;
    o_padPos.m_type = (PAD_TYPE)i_padSpec.GetType();
    o_padPos.m_spec_px = get_gerberRoi_to_imageRoi(mmToPixel, i_bodyCenter, specROI);
    o_padPos.m_align_px = o_padPos.m_spec_px;
}

bool VisionProcessingPadAlign2D::getSpec_fiducialMark2(
    const std::vector<Result_Pad>& i_padInfo, Result_Pad& o_mark1, Result_Pad& o_mark2)
{
    long findMarkCount = 0;

    for (auto& padInfo : i_padInfo)
    {
        if (m_VisionPara->GetAlignPAD_Category(padInfo.m_name) != enumPAD_Category::Fiducial)
            continue;
        if (!padInfo.m_enable)
            continue;

        switch (findMarkCount)
        {
            case 0:
                o_mark1 = padInfo;
                break;
            case 1:
                o_mark2 = padInfo;
                break;
        }

        findMarkCount++;
        if (findMarkCount == 2)
            break; // 더이상 Fiducial Mark를 찾을 필요 없다
    }

    return (findMarkCount == 2);
}

bool VisionProcessingPadAlign2D::find_pad(const Ipvm::Point32r2& i_bodyCenter, bool fiducialPad)
{
    bool success = true;

    for (long padIndex = 0; padIndex < long(m_result->m_pads.size()); padIndex++)
    {
        auto& padInfo = m_result->m_pads[padIndex];
        auto* para = m_VisionPara->GetAlignPAD_Para(padInfo.m_name);
        if (para == nullptr)
            continue;
        if (m_VisionPara->GetAlignPAD_Category(padInfo.m_name) == enumPAD_Category::Fiducial)
        {
            if (!fiducialPad)
                continue; // Fiducial 이 아닌것만 검사한다
        }
        else
        {
            if (fiducialPad)
                continue; // Fiducial 인것만 검사한다
        }

        if (!padInfo.m_enable)
            continue;

        if (!find_pad(padIndex, i_bodyCenter, padInfo))
        {
            success = false;
        }
    }
    return success;
}

bool VisionProcessingPadAlign2D::find_pad(const long padIndex, const Ipvm::Point32r2& i_bodyCenter, Result_Pad& io_mark)
{
    if (GetImageFrameCount() == 0)
        return false;

    auto* para = m_VisionPara->GetAlignPAD_Para(io_mark.m_name);
    if (para == nullptr)
    {
        ASSERT(!_T("알고리즘 파라메터를 찾을 수 없다"));
        m_errorLogText.AppendFormat(_T("  [%s] Algorithm parameter not found.\r\n"), (LPCTSTR)io_mark.m_name);
        return false;
    }

    switch (io_mark.m_type)
    {
        case PAD_TYPE::_typeCircle:
            return find_pad_circle(padIndex, *para, io_mark);

        case PAD_TYPE::_typePinIndex:
            if (m_VisionPara->GetAlignPAD_Category(io_mark.m_name) != enumPAD_Category::PinIndex)
            {
                ASSERT(!_T("PinIndex의 Algorithm 카테고리가 PinIndex가 아니라니 이 무슨"));
                return false;
            }
            return find_pad_pinmark(padIndex, i_bodyCenter, *para, io_mark);

        default:
            // 다른 타입은 Blob을 사용한다
            return find_pad_mask(padIndex, *para, io_mark);
    }
}

bool VisionProcessingPadAlign2D::find_pad_circle(const long padIndex, AlignPara& para, Result_Pad& io_mark)
{
    Ipvm::Image8u alignImage;
    Ipvm::Rect32s searchROI;
    if (!get_insp_image_and_searchROI(io_mark, para.m_edgeSearchLength_um, alignImage, searchROI))
        return false;

    Ipvm::EdgeDetectionFilter edgeFilter;
    Ipvm::EdgeDetectionPara edgePara;

    if (!get_edge_detection_para(para, edgePara))
        return false;

    float radius = searchROI.Width() * 0.5f - 2.f;

    float angle_step = 360.f / para.m_edgeSearchCount;

    Ipvm::Point32r2 searchCenter;
    searchCenter.m_x = (searchROI.m_left + searchROI.m_right) * 0.5f;
    searchCenter.m_y = (searchROI.m_top + searchROI.m_bottom) * 0.5f;

    std::vector<Ipvm::Point32s2> start_points;
    std::vector<Ipvm::Point32s2> end_points;

    for (long edgeIndex = 0; edgeIndex < para.m_edgeSearchCount; edgeIndex++)
    {
        float radian = CAST_FLOAT(angle_step * DEF_DEG_TO_RAD * edgeIndex);

        long x = long(cos(radian) * radius + searchCenter.m_x);
        long y = long(sin(radian) * radius + searchCenter.m_y);

        if (!addEdgeDirectionPoint(
                Ipvm::Point32s2(x, y), Ipvm::Conversion::ToPoint32s2(searchCenter), para, start_points, end_points))
        {
            return false;
        }
    }

    if (start_points.size() == 0)
    {
        return false;
    }

    std::vector<Ipvm::Point32r3> edgePoints(start_points.size());
    int32_t edgeNum = 0;
    if (m_edgeDetection->DetectEdges(alignImage, edgePara, edgeFilter, &start_points[0], &end_points[0],
            long(start_points.size()), para.m_edgeSearchWidth, &edgePoints[0], edgeNum)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (edgeNum == 0)
    {
        return false;
    }

    long startEdgeIndex = long(m_result->m_debugResult_edgePoints.size());

    for (long n = 0; n < edgeNum; n++)
    {
        m_result->m_debugResult_edgePoints.emplace_back(edgePoints[n].m_x, edgePoints[n].m_y);
    }

    Ipvm::EllipseEq32r ellipse;

    if (Ipvm::DataFitting::FitToEllipse(edgeNum, &m_result->m_debugResult_edgePoints[startEdgeIndex], ellipse)
        != Ipvm::Status::e_ok)
    {
        m_errorLogText.AppendFormat(_T("  [%s] Failed to calculate elliptical shape.\r\n"), (LPCTSTR)io_mark.m_name);
        return false;
    }

    // 라이브러리의 FitToEllipse가 성공 했다고 떠도 x_radius 와 m_yradius 가
    // nan으로 뜨는 경우가 있어서 이곳에서 예외처리하였다	.

    if (std::isnan(ellipse.m_xradius) || std::isnan(ellipse.m_yradius))
    {
        m_errorLogText.AppendFormat(_T("  [%s] Failed to calculate elliptical shape.\r\n"), (LPCTSTR)io_mark.m_name);
        return false;
    }

    auto* layerMask = getSurfaceLayerMask();
    if (layerMask)
    {
        layerMask->Add(ellipse, true);
    }

    m_result->m_debugResult_align_Ellipses.push_back(ellipse);
    m_result->m_debugResult_align_Center[padIndex] = Ipvm::Point32r2(ellipse.m_x, ellipse.m_y);

    io_mark.m_align_px = getEllipse_FPI_Rect(ellipse);

    return true;
}

bool VisionProcessingPadAlign2D::find_pad_mask(const long padIndex, AlignPara& para, Result_Pad& io_mark)
{
    Ipvm::Image8u alignImage;
    Ipvm::Rect32s searchROI;
    if (!get_insp_image_and_searchROI(io_mark, para.m_blobSearchOffset_um, alignImage, searchROI))
        return false;

    auto* layerMask = getSurfaceLayerMask();

    Ipvm::BlobInfo blobInfo;
    if (!get_largeBlob(alignImage, para.m_blobThreshold, searchROI, blobInfo, layerMask))
        return false;

    m_result->m_debugResult_align_Blobs.push_back(blobInfo.m_roi);

    io_mark.m_align_px.fptLT = Ipvm::Conversion::ToPoint32r2(blobInfo.m_roi.TopLeft());
    io_mark.m_align_px.fptRT = Ipvm::Conversion::ToPoint32r2(blobInfo.m_roi.TopRight());
    io_mark.m_align_px.fptLB = Ipvm::Conversion::ToPoint32r2(blobInfo.m_roi.BottomLeft());
    io_mark.m_align_px.fptRB = Ipvm::Conversion::ToPoint32r2(blobInfo.m_roi.BottomRight());

    m_result->m_debugResult_align_Center[padIndex] = io_mark.m_align_px.GetCenter();

    return true;
}

bool VisionProcessingPadAlign2D::find_pad_pinmark(
    const long padIndex, const Ipvm::Point32r2& i_bodyCenter, AlignPara& para, Result_Pad& io_mark)
{
    Ipvm::Image8u alignImage;
    Ipvm::Rect32s searchROI;
    if (!get_insp_image_and_searchROI(io_mark, para.m_blobSearchOffset_um, alignImage, searchROI))
        return false;

    Ipvm::ImageFile::SaveAsBmp(alignImage, _T("D:\\temp1.bmp"));
    Ipvm::ImageFile::SaveAsBmp(Ipvm::Image8u(alignImage, searchROI), _T("D:\\temp2.bmp"));

    Ipvm::BlobInfo blobInfo;
    if (!get_largeBlob(alignImage, para.m_blobThreshold, searchROI, blobInfo))
        return false;

    m_result->m_debugResult_align_Blobs.push_back(blobInfo.m_roi);

    auto blobRoi = blobInfo.m_roi;
    // Blob 후 BLOB 위치 기준으로 삼각형 폴리곤을 만든다 (Polygon_32f은 시계 방향으로 만들자)
    // 삼각형 모양은 Package에 위치한 모서리 위치 기준으로 하드코딩한다

    Ipvm::Polygon32r polygon;
    polygon.SetVertexNum(3);

    if (io_mark.m_spec_px.fptLT.m_x < i_bodyCenter.m_x)
    {
        // Left

        if (io_mark.m_spec_px.fptLT.m_y < i_bodyCenter.m_y)
        {
            // LT
            polygon.GetVertices()[0].m_x = (float)blobRoi.m_left;
            polygon.GetVertices()[0].m_y = (float)blobRoi.m_top;
            polygon.GetVertices()[1].m_x = (float)blobRoi.m_right;
            polygon.GetVertices()[1].m_y = (float)blobRoi.m_top;
            polygon.GetVertices()[2].m_x = (float)blobRoi.m_left;
            polygon.GetVertices()[2].m_y = (float)blobRoi.m_bottom;
        }
        else
        {
            // LB
            polygon.GetVertices()[0].m_x = (float)blobRoi.m_left;
            polygon.GetVertices()[0].m_y = (float)blobRoi.m_top;
            polygon.GetVertices()[1].m_x = (float)blobRoi.m_right;
            polygon.GetVertices()[1].m_y = (float)blobRoi.m_bottom;
            polygon.GetVertices()[2].m_x = (float)blobRoi.m_left;
            polygon.GetVertices()[2].m_y = (float)blobRoi.m_bottom;
        }
    }
    else
    {
        // Right
        if (io_mark.m_spec_px.fptLT.m_y < i_bodyCenter.m_y)
        {
            // RT
            polygon.GetVertices()[0].m_x = (float)blobRoi.m_left;
            polygon.GetVertices()[0].m_y = (float)blobRoi.m_top;
            polygon.GetVertices()[1].m_x = (float)blobRoi.m_right;
            polygon.GetVertices()[1].m_y = (float)blobRoi.m_top;
            polygon.GetVertices()[2].m_x = (float)blobRoi.m_right;
            polygon.GetVertices()[2].m_y = (float)blobRoi.m_bottom;
        }
        else
        {
            // RB
            polygon.GetVertices()[0].m_x = (float)blobRoi.m_right;
            polygon.GetVertices()[0].m_y = (float)blobRoi.m_top;
            polygon.GetVertices()[1].m_x = (float)blobRoi.m_right;
            polygon.GetVertices()[1].m_y = (float)blobRoi.m_bottom;
            polygon.GetVertices()[2].m_x = (float)blobRoi.m_left;
            polygon.GetVertices()[2].m_y = (float)blobRoi.m_bottom;
        }
    }

    io_mark.m_align_px.fptLT = Ipvm::Conversion::ToPoint32r2(blobRoi.TopLeft());
    io_mark.m_align_px.fptRT = Ipvm::Conversion::ToPoint32r2(blobRoi.TopRight());
    io_mark.m_align_px.fptLB = Ipvm::Conversion::ToPoint32r2(blobRoi.BottomLeft());
    io_mark.m_align_px.fptRB = Ipvm::Conversion::ToPoint32r2(blobRoi.BottomRight());

    m_result->m_debugResult_edgeSourceRois.emplace_back(
        io_mark.m_align_px.fptLT, io_mark.m_align_px.fptRT, io_mark.m_align_px.fptLB, io_mark.m_align_px.fptRB);

    return find_pad_polygon_edge_detect(padIndex, para, polygon, io_mark);
}

bool VisionProcessingPadAlign2D::find_pad_polygon_edge_detect(
    const long padIndex, AlignPara& para, const Ipvm::Polygon32r& i_polygon, Result_Pad& io_mark)
{
    Ipvm::Image8u alignImage;
    Ipvm::Rect32s searchROI;

    if (!get_insp_image_and_searchROI(io_mark, para.m_edgeSearchLength_um, alignImage, searchROI))
        return false;

    if (para.m_edgeSearchCount <= 0)
        return false;

    float searchOffset = max(0.f, (long)getScale().convert_umToPixelX(para.m_edgeSearchOffset_um));
    long searchLength = max(0, (long)getScale().convert_umToPixelY(para.m_edgeSearchLength_um));
    bool success = true;

    Ipvm::EdgeDetectionFilter edgeFilter;
    Ipvm::EdgeDetectionPara edgePara;

    if (!get_edge_detection_para(para, edgePara))
        return false;

    std::vector<Ipvm::LineEq32r> lines(i_polygon.GetVertexNum());

    for (long vertex = 0; vertex < i_polygon.GetVertexNum(); vertex++)
    {
        bool line_success = false;
        do
        {
            Ipvm::Point32r2 v_start(i_polygon.GetVertices()[vertex].m_x, i_polygon.GetVertices()[vertex].m_y);
            Ipvm::Point32r2 v_end(i_polygon.GetVertices()[(vertex + 1) % i_polygon.GetVertexNum()].m_x,
                i_polygon.GetVertices()[(vertex + 1) % i_polygon.GetVertexNum()].m_y);

            float lineDistance = CAST_FLOAT(
                max(0.f, sqrt(pow(v_end.m_x - v_start.m_x, 2) + pow(v_end.m_y - v_start.m_y, 2)) - searchOffset * 2.f));
            float step = lineDistance / (para.m_edgeSearchCount + 1);

            std::vector<Ipvm::Point32s2> start_points;
            std::vector<Ipvm::Point32s2> end_points;

            for (long edge = 0; edge < para.m_edgeSearchCount; edge++)
            {
                Ipvm::Point32r2 step_point, start, end;
                Ipvm::Geometry::GetOffsetPointAlongTheSegment(
                    v_end, v_start, v_start, (edge + 1.f) * step + searchOffset, step_point);
                Ipvm::Geometry::GetOffsetPointsAlongThePerpendicularSegment(
                    v_start, v_end, step_point, (float)searchLength, end, start);

                if (!addEdgeDirectionPoint(Ipvm::Conversion::ToPoint32s2(start), Ipvm::Conversion::ToPoint32s2(end),
                        para, start_points, end_points))
                {
                    return false;
                }
            }

            std::vector<Ipvm::Point32r3> edgePoints(start_points.size());
            int32_t edgeNum = 0;
            if (m_edgeDetection->DetectEdges(alignImage, edgePara, edgeFilter, &start_points[0], &end_points[0],
                    long(start_points.size()), para.m_edgeSearchWidth, &edgePoints[0], edgeNum)
                != Ipvm::Status::e_ok)
            {
                break;
            }

            if (edgeNum == 0)
            {
                break;
            }

            long startEdgeIndex = long(m_result->m_debugResult_edgePoints.size());

            for (long n = 0; n < edgeNum; n++)
            {
                m_result->m_debugResult_edgePoints.emplace_back(edgePoints[n].m_x, edgePoints[n].m_y);
            }

            std::vector<Ipvm::Point32r2> fitEdge(edgeNum);
            int32_t useEdgeNum = 0;

            if (Ipvm::DataFitting::FitToLineRansac(edgeNum, &m_result->m_debugResult_edgePoints[startEdgeIndex],
                    &fitEdge[0], useEdgeNum, lines[vertex], 3.f)
                != Ipvm::Status::e_ok)
            {
                break;
            }

            line_success = true;
        }
        while (0);

        if (!line_success)
        {
            success = false;
            m_errorLogText.AppendFormat(_T("  [%s] Polygon_32f line detection failed.\r\n"), (LPCTSTR)io_mark.m_name);
            break;
        }
    }

    if (success)
    {
        Ipvm::Polygon32r polygon;
        polygon.SetVertexNum(i_polygon.GetVertexNum());
        for (long vertex = 0; vertex < i_polygon.GetVertexNum(); vertex++)
        {
            auto& line1 = lines[vertex];
            auto& line2 = lines[(vertex + 1) % i_polygon.GetVertexNum()];

            Ipvm::Point32r2 crossPoint;
            if (Ipvm::Geometry::GetCrossPoint(line1, line2, crossPoint) != Ipvm::Status::e_ok)
            {
                m_errorLogText.AppendFormat(
                    _T("  [%s] Polygon_32f Cross point detection failed.\r\n"), (LPCTSTR)io_mark.m_name);
                success = false;
            }
            else
            {
                polygon.GetVertices()[vertex] = crossPoint;
            }
        }

        if (success)
        {
            auto* layerMask = getSurfaceLayerMask();
            if (layerMask)
                layerMask->Add(polygon, true);

            m_result->m_debugResult_align_Polygons.push_back(polygon);

            Ipvm::Rect32r rect = Utility::ConvertPolygonToRect_32f(polygon);
            io_mark.m_align_px.fptLT = Ipvm::Point32r2(rect.m_left, rect.m_top);
            io_mark.m_align_px.fptRT = Ipvm::Point32r2(rect.m_right, rect.m_top);
            io_mark.m_align_px.fptLB = Ipvm::Point32r2(rect.m_left, rect.m_bottom);
            io_mark.m_align_px.fptRB = Ipvm::Point32r2(rect.m_right, rect.m_bottom);

            m_result->m_debugResult_align_Center[padIndex] = io_mark.m_align_px.GetCenter();
        }
    }

    return success;
}

bool VisionProcessingPadAlign2D::calculation_offset_degree(const Ipvm::Point32r2& i_bodyCenter,
    const Result_Pad& i_mark1, const Result_Pad& i_mark2, Ipvm::Point32r2& o_offset, float& o_degree)
{
    const auto& mmToPixel = getScale().mmToPixel();

    Ipvm::Point32r2 fid_center_spec = getCenter_object2(i_mark1.m_spec_px.GetCenter(), i_mark2.m_spec_px.GetCenter());
    Ipvm::Point32r2 fid_center_result
        = getCenter_object2(i_mark1.m_align_px.GetCenter(), i_mark2.m_align_px.GetCenter());

    // Fiducial Offset 계산
    o_offset = fid_center_spec - fid_center_result;

    Ipvm::Point32r2 bodyCenter_t = i_bodyCenter + o_offset;

    // Fiducial Angle 계산

    Ipvm::Point32r2 center_mark1_s = get_imageRoi_to_gerberRoi(mmToPixel, i_bodyCenter, i_mark1.m_spec_px.GetCenter());
    Ipvm::Point32r2 center_mark2_s = get_imageRoi_to_gerberRoi(mmToPixel, i_bodyCenter, i_mark2.m_spec_px.GetCenter());
    Ipvm::Point32r2 center_mark1_t = get_imageRoi_to_gerberRoi(mmToPixel, bodyCenter_t, i_mark1.m_align_px.GetCenter());
    Ipvm::Point32r2 center_mark2_t = get_imageRoi_to_gerberRoi(mmToPixel, bodyCenter_t, i_mark2.m_align_px.GetCenter());

    float degree1 = 0.f;
    float degree2 = 0.f;

    if (Ipvm::Geometry::GetContainedAngleInDegrees(center_mark1_s, i_bodyCenter, center_mark1_t, degree1)
        != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::Geometry::GetContainedAngleInDegrees(center_mark2_s, bodyCenter_t, center_mark2_t, degree1)
        != Ipvm::Status::e_ok)
        return false;

    // Degree 가 시계 방향인지 반시계 방향인지 체크한다
    degree1 = fabs(degree1) * ((center_mark1_s.m_x > center_mark1_t.m_x) ? 1.f : -1.f);
    degree2 = fabs(degree2) * ((center_mark2_s.m_x > center_mark2_t.m_x) ? 1.f : -1.f);

    o_degree = (degree1 + degree2) * 0.5f;

    return true;
}

bool VisionProcessingPadAlign2D::apply_other_pad_from_fiducial(const Ipvm::Point32r2& i_bodyCenter,
    const Ipvm::Point32r2& i_offset, const float i_degree, std::vector<Result_Pad>& io_padInfo)
{
    m_result->m_applyAlignTobodyCenter = i_bodyCenter + i_offset;
    double radian = i_degree * DEF_DEG_TO_RAD;

    for (long padIndex = 0; padIndex < long(io_padInfo.size()); padIndex++)
    {
        auto& padInfo = io_padInfo[padIndex];

        bool isNoAlignPad = (m_VisionPara->FindAlignPAD_Index(padInfo.m_name) < 0);
        if (!padInfo.m_enable)
            isNoAlignPad = true; // 비활성화 되어 있는 건 상대좌표만 구하자

        auto category = m_VisionPara->GetAlignPAD_Category(padInfo.m_name);
        if (category == enumPAD_Category::Fiducial)
            continue;

        padInfo.m_align_px.Move(i_offset.m_x, i_offset.m_y); // Fiducial Offset 적용
        padInfo.m_align_px
            = padInfo.m_align_px.Rotate(radian, m_result->m_applyAlignTobodyCenter); // Fiducial Angle 적용

        if (isNoAlignPad)
        {
            // 따로 Align 하지 않는 PAD는 여기서 Mask 정보가 결정된다
            auto* layerMask = getSurfaceLayerMask();
            if (layerMask && padInfo.m_enable)
            {
                // 활성화 되어 있는 것만으로 Surface Mask를 구하자
                layerMask->Add(padInfo.m_align_px, true);
            }
            m_result->m_debugResult_align_Other.push_back(padInfo.m_align_px);
        }

        m_result->m_debugResult_align_Center[padIndex] = padInfo.m_align_px.GetCenter();
    }

    return true;
}

bool VisionProcessingPadAlign2D::get_edge_detection_para(const AlignPara& para, Ipvm::EdgeDetectionPara& o_edge_para)
{
    switch (para.m_edgeType)
    {
        case 0:
            o_edge_para.m_edgeType = Ipvm::EdgeType::e_both;
            break;
        case 1:
            o_edge_para.m_edgeType = Ipvm::EdgeType::e_rising;
            break;
        case 2:
            o_edge_para.m_edgeType = Ipvm::EdgeType::e_falling;
            break;
        default:
            return false;
    }

    o_edge_para.m_edgeThreshold = para.m_edgeThreshold;
    o_edge_para.m_findFirstEdge = para.m_findFirstEdge ? true : false;

    return true;
}

bool VisionProcessingPadAlign2D::get_insp_image_and_searchROI(
    const Result_Pad& i_targetPAD, float searhRange_um, Ipvm::Image8u& o_inspImage, Ipvm::Rect32s& o_searchROI)
{
    //--------------------------------------------------------------------------------------------------------------------------------
    // PAD 검사에 사용될 Image와 Search ROI 를 리턴한다
    //--------------------------------------------------------------------------------------------------------------------------------

    auto* para = m_VisionPara->GetAlignPAD_Para(i_targetPAD.m_name);
    if (para == nullptr)
        return false;

    if (!getReusableMemory().GetInspByteImage(o_inspImage))
        return false;

    Ipvm::Image8u image1 = GetInspectionFrameImage(FALSE, GetImageFrameIndex(0));

    long searhRangeX = max(0, (long)getScale().convert_umToPixelX(searhRange_um));
    long searhRangeY = max(0, (long)getScale().convert_umToPixelY(searhRange_um));

    o_searchROI = i_targetPAD.m_align_px.GetCRect();
    o_searchROI.InflateRect(searhRangeX, searhRangeY);
    o_searchROI &= Ipvm::Rect32s(o_inspImage);

    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image1, false, o_searchROI, para->m_imageProc, o_inspImage))
    {
        return false;
    }

    return true;
}

bool VisionProcessingPadAlign2D::get_largeBlob(const Ipvm::Image8u& i_image, long threshold,
    const Ipvm::Rect32s& searchROI, Ipvm::BlobInfo& o_largestBlob, SurfaceLayerMask* o_mask)
{
    //--------------------------------------------------------------------------------------------------------------------------------
    // 영역 내에서 Threshold 후 Blob하여 가장 큰 BlobInfo을 선택 후 해당 블럽의 Boundary을 얻는다
    //--------------------------------------------------------------------------------------------------------------------------------

    Ipvm::Image8u binaryImage;
    Ipvm::Image32s labelImage;

    if (!getReusableMemory().GetInspByteImage(binaryImage))
        return false;
    if (!getReusableMemory().GetInspLongImage(labelImage))
        return false;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(labelImage), 0, labelImage);

    Ipvm::Image32u histogram(256, 1);
    if (Ipvm::ImageProcessing::MakeHistogram(i_image, searchROI, histogram) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::BinarizeGreater(i_image, searchROI, (BYTE)min(255, max(0, threshold)), binaryImage)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    int32_t blobCount = 0;
    if (m_pBlob->DoBlob(binaryImage, searchROI, 100, labelImage, m_psBlobInfo, blobCount) < 0)
        return false;
    if (blobCount <= 0)
        return false;

    // 가장 큰 Blob 찾기
    long blobIndex = 0;
    for (long index = 0; index < blobCount; index++)
    {
        if (m_psBlobInfo[index].m_area > m_psBlobInfo[blobIndex].m_area)
            blobIndex = index;
    }

    o_largestBlob = m_psBlobInfo[blobIndex];
    if (o_mask)
    {
        o_mask->AddLabel(labelImage, o_largestBlob, true);
    }

    return true;
}

bool VisionProcessingPadAlign2D::boundaryPoints_makes_simple(
    const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, Ipvm::Polygon32r& o_boundary)
{
    if (i_boundary.GetVertexNum() < 2)
        return false;

    std::vector<Ipvm::Point32r2> boundaryPoints;

    long firstStartIndex = -1;
    long checkBase = 0;

    while (1)
    {
        long startIndex = 0;
        long endIndex = 0;

        if (firstStartIndex < 0)
        {
            getContinuoesPoints(i_boundary, pointDistanceForLine, checkBase, startIndex, endIndex);

            boundaryPoints.push_back(i_boundary.GetVertices()[startIndex]);
            boundaryPoints.push_back(i_boundary.GetVertices()[endIndex]);

            firstStartIndex = startIndex;
        }
        else
        {
            startIndex = checkBase;
            getContinuoesPoints_end(i_boundary, pointDistanceForLine, checkBase, checkBase, endIndex,
                getPolygon_Index(i_boundary, firstStartIndex + 1));

            if (endIndex == firstStartIndex)
            {
                break;
            }

            boundaryPoints.push_back(i_boundary.GetVertices()[endIndex]);
        }

        checkBase = endIndex;
    }

    o_boundary.SetVertexNum(long(boundaryPoints.size()));
    for (long index = 0; index < long(boundaryPoints.size()); index++)
    {
        o_boundary.GetVertices()[index] = boundaryPoints[index];
    }

    return true;
}

long VisionProcessingPadAlign2D::getPolygon_Index(const Ipvm::Polygon32r& i_boundary, long index)
{
    long size = static_cast<long>(i_boundary.GetVertexNum());

    while (index < 0)
        index += size;

    return index % size;
}

void VisionProcessingPadAlign2D::getContinuoesPoints(
    const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, long baseIndex, long& startIndex, long& endIndex)
{
    startIndex = baseIndex;
    endIndex = baseIndex;

    Ipvm::Point32r2 linePoints[2];
    linePoints[0] = i_boundary.GetVertices()[startIndex];
    linePoints[1] = i_boundary.GetVertices()[endIndex];

    for (long check = getPolygon_Index(i_boundary, baseIndex - 1); check != baseIndex;
        check = getPolygon_Index(i_boundary, check - 1))
    {
        linePoints[0] = i_boundary.GetVertices()[check];

        Ipvm::LineEq32r line;
        Ipvm::DataFitting::FitToLine(2, linePoints, line);

        if (!isValidLine(i_boundary, pointDistanceForLine, check, baseIndex, line))
        {
            break;
        }

        startIndex = check;
    }

    getContinuoesPoints_end(i_boundary, pointDistanceForLine, startIndex, baseIndex, endIndex);
}

void VisionProcessingPadAlign2D::getContinuoesPoints_end(const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine,
    long startIndex, long baseEndIndex, long& endIndex, long limitEndIndex)
{
    if (limitEndIndex < 0)
        limitEndIndex = startIndex;

    endIndex = baseEndIndex;

    Ipvm::Point32r2 linePoints[2];
    linePoints[0] = i_boundary.GetVertices()[startIndex];
    linePoints[1] = i_boundary.GetVertices()[endIndex];

    for (long check = getPolygon_Index(i_boundary, baseEndIndex + 1); check != limitEndIndex;
        check = getPolygon_Index(i_boundary, check + 1))
    {
        linePoints[1] = i_boundary.GetVertices()[check];

        Ipvm::LineEq32r line;
        Ipvm::DataFitting::FitToLine(2, linePoints, line);

        if (!isValidLine(i_boundary, pointDistanceForLine, startIndex, check, line))
        {
            break;
        }

        endIndex = check;
    }
}

bool VisionProcessingPadAlign2D::isValidLine(const Ipvm::Polygon32r& i_boundary, float i_pointDistanceForLine,
    long i_startIndex, long i_endIndex, const Ipvm::LineEq32r& i_line)
{
    long check = getPolygon_Index(i_boundary, i_startIndex);
    while (1)
    {
        float distance = 0.f;
        Ipvm::Geometry::GetDistance(i_line, i_boundary.GetVertices()[check], distance);
        if (distance > i_pointDistanceForLine)
        {
            return false;
        }

        check = getPolygon_Index(i_boundary, check + 1);
        if (check == i_endIndex)
            break;
    }

    return true;
}

Ipvm::Point32r2 VisionProcessingPadAlign2D::get_imageRoi_to_gerberRoi(
    const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const Ipvm::Point32r2& imagePos)
{
    float x = (imagePos.m_x - body_center_px.m_x);
    float y = (body_center_px.m_y - imagePos.m_y);

    Ipvm::Point32r2 retValue;
    retValue.m_x = x / pixelperMM.m_x;
    retValue.m_y = y / pixelperMM.m_y;

    return retValue;
}

Ipvm::Polygon32r VisionProcessingPadAlign2D::get_imageRoi_to_gerberRoi(
    const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const Ipvm::Polygon32r& imagePos)
{
    Ipvm::Polygon32r retValue;
    retValue.SetVertexNum(imagePos.GetVertexNum());

    for (long index = 0; index < imagePos.GetVertexNum(); index++)
    {
        retValue.GetVertices()[index]
            = get_imageRoi_to_gerberRoi(pixelperMM, body_center_px, imagePos.GetVertices()[index]);
    }

    return retValue;
}

Ipvm::Point32r2 VisionProcessingPadAlign2D::get_gerberRoi_to_imageRoi(
    const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const Ipvm::Point32r2& gerberPos)
{
    Ipvm::Point32r2 retValue;
    retValue.m_x = gerberPos.m_x * pixelperMM.m_x + body_center_px.m_x;
    retValue.m_y = body_center_px.m_y - gerberPos.m_y * pixelperMM.m_y;

    return retValue;
}

FPI_RECT VisionProcessingPadAlign2D::get_gerberRoi_to_imageRoi(
    const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const FPI_RECT& gerberPos)
{
    FPI_RECT retValue;
    retValue.fptLT = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, gerberPos.fptLT);
    retValue.fptRT = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, gerberPos.fptRT);
    retValue.fptLB = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, gerberPos.fptLB);
    retValue.fptRB = get_gerberRoi_to_imageRoi(pixelperMM, body_center_px, gerberPos.fptRB);

    return retValue;
}

Ipvm::Point32r2 VisionProcessingPadAlign2D::getCenter_object2(const Ipvm::Point32r2& obj1, const Ipvm::Point32r2& obj2)
{
    Ipvm::Point32r2 retValue;
    retValue.m_x = (obj1.m_x + obj2.m_x) * 0.5f;
    retValue.m_y = (obj1.m_y + obj2.m_y) * 0.5f;

    return retValue;
}

FPI_RECT VisionProcessingPadAlign2D::getRect32f_FPI_Rect(const Ipvm::Rect32r& object)
{
    FPI_RECT retValue;
    retValue.fptLT = Ipvm::Point32r2(object.m_left, object.m_top);
    retValue.fptRT = Ipvm::Point32r2(object.m_right, object.m_top);
    retValue.fptLB = Ipvm::Point32r2(object.m_left, object.m_bottom);
    retValue.fptRB = Ipvm::Point32r2(object.m_right, object.m_bottom);

    return retValue;
}

FPI_RECT VisionProcessingPadAlign2D::getEllipse_FPI_Rect(const Ipvm::EllipseEq32r& object)
{
    FPI_RECT retValue;
    retValue.fptLT = Ipvm::Point32r2(object.m_x - object.m_xradius, object.m_y - object.m_yradius);
    retValue.fptRT = Ipvm::Point32r2(object.m_x + object.m_xradius, object.m_y - object.m_yradius);
    retValue.fptLB = Ipvm::Point32r2(object.m_x - object.m_xradius, object.m_y + object.m_yradius);
    retValue.fptRB = Ipvm::Point32r2(object.m_x + object.m_xradius, object.m_y + object.m_yradius);

    return retValue;
}

SurfaceLayerMask* VisionProcessingPadAlign2D::getSurfaceLayerMask()
{
    return getReusableMemory().AddSurfaceLayerMaskClass(_T("PADAlign"));
}

bool VisionProcessingPadAlign2D::addEdgeDirectionPoint(const Ipvm::Point32s2& outPoint, const Ipvm::Point32s2& inPoint,
    const AlignPara& para, std::vector<Ipvm::Point32s2>& o_startList, std::vector<Ipvm::Point32s2>& o_endList)
{
    switch (para.m_edgeSearchDirection)
    {
        case 0:
            // In -> Out
            o_startList.push_back(inPoint);
            o_endList.push_back(outPoint);

            m_result->m_debugResult_edgeDetectionLines.emplace_back(
                float(inPoint.m_x), float(inPoint.m_y), float(outPoint.m_x), float(outPoint.m_y));
            break;
        case 1:
            // Out -> In
            o_startList.push_back(outPoint);
            o_endList.push_back(inPoint);

            m_result->m_debugResult_edgeDetectionLines.emplace_back(
                float(outPoint.m_x), float(outPoint.m_y), float(inPoint.m_x), float(inPoint.m_y));
            break;

        default:
            return false;
    }

    return true;
}

void VisionProcessingPadAlign2D::checkJobParameter()
{
    const auto& padInfos = m_packageSpec.m_PadMapManager;

    auto paraDB = new ParaDB(*this);
    paraDB->Load();

    for (long padIndex = 0; padIndex < padInfos->GetCount(); padIndex++)
    {
        auto& padInfo = padInfos->vecPadData[padIndex];

        bool isPinIndex = (padInfo.GetType() == _typePinIndex);
        if (m_VisionPara->FindAlignPAD_Index(padInfo.strPadName) < 0)
            continue;

        if (isPinIndex && m_VisionPara->GetAlignPAD_Category(padInfo.strPadName) == enumPAD_Category::PinIndex)
            continue;
        if (!isPinIndex && m_VisionPara->GetAlignPAD_Category(padInfo.strPadName) != enumPAD_Category::PinIndex)
            continue;

        // 서로 호환되지 않는 알고리즘 분류로 되어 있다
        // Job을 복사후 PackageSpec만 바꿨거나 했을 때 이런 현상이 일어 날 수 있다
        long algoIndex = -1;

        for (long check = 0; check < m_VisionPara->GetAlignPAD_Count(); check++)
        {
            if (isPinIndex && m_VisionPara->GetAlignPAD_Category(check) == enumPAD_Category::PinIndex)
            {
                algoIndex = check;
                break;
            }

            if (!isPinIndex && m_VisionPara->GetAlignPAD_Category(check) != enumPAD_Category::PinIndex)
            {
                algoIndex = check;
                break;
            }
        }

        if (algoIndex >= 0)
        {
            // 있는 것중 처음 찾은 알고리즘을 쓰게 하자
            m_VisionPara->SetAlignPAD(*paraDB, padInfo.strPadName, m_VisionPara->GetAlignPAD_Name(algoIndex),
                m_VisionPara->GetAlignPAD_Category(algoIndex));
        }
        else
        {
            CString nameBase = isPinIndex ? _T("PININDEX") : _T("NORMAL");
            CString newName = nameBase;
            long tail = 1;

            while (m_VisionPara->FindAlignPAD_Index(newName) >= 0)
            {
                newName.Format(_T("%s%d"), LPCTSTR(nameBase), tail);
                tail++;
            }

            m_VisionPara->SetAlignPAD(*paraDB, padInfo.strPadName, newName,
                isPinIndex ? enumPAD_Category::PinIndex : enumPAD_Category::Normal);

            nameBase.Empty();
            newName.Empty();
        }
    }

    delete paraDB;
}
