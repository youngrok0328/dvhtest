//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "EngineBasedOnBodySize.h"

//CPP_2_________________________________ This project's headers
#include "Constants.h"
#include "StitchResult.h"
#include "VisionProcessingAlign2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
EngineBasedOnBodySize::EngineBasedOnBodySize(VisionProcessingAlign2D* processor, const Constants& constants)
    : m_processor(processor)
    , m_constants(constants)
    , m_packageSpec(processor->m_packageSpec)
    , m_parentPara(processor->m_VisionPara)
    , m_para(processor->m_VisionPara.m_paraBasedOnBodySize)
    , m_result(processor->m_result)
{
}

EngineBasedOnBodySize::~EngineBasedOnBodySize()
{
}

bool EngineBasedOnBodySize::GetConstants()
{
    long stitchCount = m_constants.m_stitchCountX * m_constants.m_stitchCountY;
    m_para.m_stitchPara.resize(stitchCount);

    return true;
}

bool EngineBasedOnBodySize::SetROIsInRaw()
{
    if (m_constants.m_stitchCountX > 2 || m_constants.m_stitchCountX > 2)
    {
        m_result.m_additionalCriticalLog = _T("Can't use Algorithm 'based on BodySize'.\r\n");
        m_result.m_additionalCriticalLog += _T("Terms: Stitch X Num <= 2 and Stitch Y Num <= 2");
        return false;
    }

    const auto& scale = m_processor->getScale();

    bool success = true;
    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;

            m_para.m_stitchPara[stitchIndex].MakeResult(scale, m_constants, m_para.m_searchDirection, stitchIndex);

            if (stitchIndex >= m_result.m_stitchRois.size())
            {
                success = false;
                continue;
            }
        }
    }

    return success;
}

bool EngineBasedOnBodySize::FindAllStitchInfo_InRaw(const bool detailSetupMode)
{
    if (!SetROIsInRaw())
        return false;
    if (!EdgeDetectInROIs())
        return false;
    if (!FittingLineUsingEdgePoints())
        return true;
    if (!MakeAllStitchInformation())
        return true;

    UNREFERENCED_PARAMETER(detailSetupMode); //위의 함수에서 detailSetupMode를 사용할 수도 있다.

    return true;
}

bool EngineBasedOnBodySize::EdgeDetectInROIs()
{
    if (m_para.m_edgeNum < 1 || m_para.m_edgeNum > 100000)
        return false;

    auto& imageRaw = m_processor->getImageLot().GetImageFrame(
        m_constants.m_frameIndexForStitchInfo, m_processor->GetCurVisionModule_Status());
    auto* edgeDetect = m_processor->getReusableMemory().GetEdgeDetect();
    bool success = true;

    // Stitch Section Image Combine
    Ipvm::Image8u combineImage;
    if (!m_processor->getReusableMemory().GetByteImage(combineImage, imageRaw.GetSizeX(), imageRaw.GetSizeY()))
    {
        return false;
    }
    //  Image Combine 적용
    if (!CippModules::GrayImageProcessingManage(m_processor->getReusableMemory(), &imageRaw, true,
            m_processor->m_VisionPara.m_stitchImageProcManagePara, combineImage))
    {
        return false;
    }

    if (m_para.m_edgeDetectMode == EdgeDetectMode_FirstEdge)
    {
        edgeDetect->SetMininumThreshold(m_para.m_firstEdgeMinThreshold);
    }
    else
    {
        // Best Edge
        edgeDetect->SetMininumThreshold(1.f);
    }

    for (int stitchIndex = 0; stitchIndex < long(m_para.m_stitchPara.size()); stitchIndex++)
    {
        auto& rois = m_para.m_stitchPara[stitchIndex].m_resultRois;

        for (auto& roi : rois)
        {
            if (!edgeDetect->PI_ED_CalcEdgePointFromImage(combineImage, m_para.m_edgeDirection, roi.m_searchDirection,
                    roi.m_roi, m_para.m_edgeNum, 3, roi.m_edges, (m_para.m_edgeDetectMode == EdgeDetectMode_FirstEdge)))
            {
                success = false;
            }
        }
    }

    return success;
}

bool EngineBasedOnBodySize::FittingLineUsingEdgePoints()
{
    if (m_para.m_edgeNum < 1 || m_para.m_edgeNum > 100000)
        return false;
    if (m_para.m_stitchPara.size() != m_constants.m_stitchCountX * m_constants.m_stitchCountY)
        return false;

    std::vector<Ipvm::Point32r2> usedPoints;
    usedPoints.resize(m_para.m_edgeNum);

    bool success = true;
    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;

            auto& stitchPara = m_para.m_stitchPara[stitchIndex];
            auto& rois = stitchPara.m_resultRois;

            if (rois.size() < 2)
            {
                // X 방향, Y 방향 Line 을 찾아야 하니 두개가 되야 하는데 이상하다
                success = false;
                continue;
            }

            bool lineFittingSuccess = true;

            for (auto& roi : rois)
            {
                long edgeCount = (long)roi.m_edges.size();
                if (edgeCount < 2)
                {
                    success = false;
                    continue;
                }

                int32_t useEdgeNum = 0;
                if (Ipvm::DataFitting::FitToLineRansac(
                        edgeCount, &roi.m_edges[0], &usedPoints[0], useEdgeNum, roi.m_lineEq, 3.f)
                    != Ipvm::Status::e_ok)
                {
                    lineFittingSuccess = false;
                }
            }

            if (lineFittingSuccess)
            {
                // Angle 계산
                double RadToDeg = 180 / ITP_PI;

                auto& hori = rois[1];
                auto& vert = rois[0];

                float angle1 = (float)(atan(-hori.m_lineEq.m_a / hori.m_lineEq.m_b) * RadToDeg);
                stitchPara.m_resultAngle = angle1; // angle2;// (angle1 + angle2) * 0.5f;

                /*
				float angle2 = (float)(atan(-vert.m_lineEq.m_a / vert.m_lineEq.m_b) * RadToDeg + 90.f);
				angle2 = fabs(angle2 - 180.f) < fabs(angle2) ? angle2 - 180.f : angle2;
				stitchPara.m_resultAngle = angle2;// (angle1 + angle2) * 0.5f;
				*/

                if (Ipvm::Geometry::GetCrossPoint(hori.m_lineEq, vert.m_lineEq, stitchPara.m_resultCornerPoint)
                    != Ipvm::Status::e_ok)
                {
                    success = false;
                }

                if (stitchIndex == 0)
                {
                    stitchPara.m_resultRefCornerPoint = stitchPara.m_resultCornerPoint;
                }
                else
                {
                    if (x == 1 && y == 0)
                    {
                        Ipvm::Geometry::GetOffsetPointAlongTheLine(m_para.m_stitchPara[0].m_resultRois[1].m_lineEq,
                            m_para.m_stitchPara[stitchIndex - 1].m_resultRefCornerPoint, m_constants.m_packageSizeX_px,
                            false, stitchPara.m_resultRefCornerPoint);

                        stitchPara.m_resultRefCornerPoint.m_x += m_constants.m_floatOverlapSizeX_px;
                    }
                    else if (x == 0 && y == 1)
                    {
                        Ipvm::Geometry::GetOffsetPointAlongTheLine(m_para.m_stitchPara[0].m_resultRois[0].m_lineEq,
                            m_para.m_stitchPara[stitchIndex - m_constants.m_stitchCountX].m_resultRefCornerPoint,
                            m_constants.m_packageSizeY_px, true, stitchPara.m_resultRefCornerPoint);

                        stitchPara.m_resultRefCornerPoint.m_y += m_constants.m_floatOverlapSizeY_px;
                    }
                    else
                    {
                        Ipvm::Point32r2 offset = m_para.m_stitchPara[1].m_resultRefCornerPoint
                            - m_para.m_stitchPara[0].m_resultRefCornerPoint;
                        m_para.m_stitchPara[3].m_resultRefCornerPoint
                            = m_para.m_stitchPara[2].m_resultRefCornerPoint + offset;
                    }
                }
            }
            else
            {
                success = false;
            }
        }
    }

    return success;
}

bool EngineBasedOnBodySize::MakeAllStitchInformation()
{
    double firstStitchAngle = 0.f;

    for (long y = 0; y < m_constants.m_stitchCountY; y++)
    {
        for (long x = 0; x < m_constants.m_stitchCountX; x++)
        {
            long stitchIndex = y * m_constants.m_stitchCountX + x;

            auto& stitchPara = m_para.m_stitchPara[stitchIndex];
            auto& result_curStitchROI = m_result.m_stitchRois[stitchIndex];

            MakeStitchInformation(x, y, stitchPara, result_curStitchROI.m_AccMatchingInfo);

            if (stitchIndex == 0)
            {
                firstStitchAngle = result_curStitchROI.m_AccMatchingInfo.m_angle_diff;
                result_curStitchROI.m_AccMatchingInfo.m_angle_diff = 0.f;
                result_curStitchROI.m_AccMatchingInfo.m_shiftX = 0.f;
                result_curStitchROI.m_AccMatchingInfo.m_shiftY = 0.f;
                continue; // x==y==0 이면 계산하지 않는다.
            }

            result_curStitchROI.m_AccMatchingInfo.m_angle_diff -= firstStitchAngle;
        }
    }

    return true;
}

void EngineBasedOnBodySize::MakeStitchInformation(
    long x, long y, EdgeSearchRoiPara& object, StitchMatchingInfo& o_MatchingInfo)
{
    o_MatchingInfo.m_angle_diff = -object.m_resultAngle;
    o_MatchingInfo.m_shiftX = object.m_resultRefCornerPoint.m_x - object.m_resultCornerPoint.m_x;
    o_MatchingInfo.m_shiftY = object.m_resultRefCornerPoint.m_y - object.m_resultCornerPoint.m_y;

    // Rotate Origin은 Sensor Size 크기 이미지 기준이다
    o_MatchingInfo.m_rotate_origin_x = object.m_resultCornerPoint.m_x - x * m_constants.m_sensorSizeX;
    o_MatchingInfo.m_rotate_origin_y = object.m_resultCornerPoint.m_y - y * m_constants.m_sensorSizeY;
}
