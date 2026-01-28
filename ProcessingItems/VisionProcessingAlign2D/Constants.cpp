//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Constants.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingAlign2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Constants::Constants(VisionProcessingAlign2D* processor)
    : m_processor(processor)
{
}

Constants::~Constants()
{
}

bool Constants::Make()
{
    const auto& image = m_processor->getImageLot().GetImageFrame(0, m_processor->GetCurVisionModule_Status());
    if (image.GetMem() == nullptr)
        return false;

    const auto& scale = m_processor->getScale();

    m_rawImageSizeX = image.GetSizeX();
    m_rawImageSizeY = image.GetSizeY();
    m_rawImageCenter.m_x = m_rawImageSizeX * 0.5f;
    m_rawImageCenter.m_y = m_rawImageSizeY * 0.5f;

    m_processor->GetPackageSize(true, m_packageSizeX_px, m_packageSizeY_px);

    /*m_packageSizeX_px = scale.convert_umToPixelX(m_processor->m_packageSpec.m_bodyInfoMaster->GetBodySizeX());
	m_packageSizeY_px = scale.convert_umToPixelY(m_processor->m_packageSpec.m_bodyInfoMaster->GetBodySizeY());*/

    m_stitchCountX = (long)m_processor->getInspectionAreaInfo().m_stichCountX;
    m_stitchCountY = (long)m_processor->getInspectionAreaInfo().m_stichCountY;

    m_sensorSizeX = m_processor->getImageLot().GetSensorSizeX();
    m_sensorSizeY = m_processor->getImageLot().GetSensorSizeY();

    float pixel2UmX = scale.pixelToUm().m_x; // 10.f;
    float pixel2UmY = scale.pixelToUm().m_y; // 10.f;

    float um2Pixel = scale.umToPixelXY();

    float moveUmX = m_processor->GetMoveDistanceXbetweenFOVs_mm() * 1000.f; // 28250.f;	mm-> um
    float moveUmY = m_processor->GetMoveDistanceYbetweenFOVs_mm() * 1000.f; // 38250.f;	mm-> um

    // SDY Stitch count가 1일 경우에는 겹치는 영역이 없으므로 overlap을 0으로 출력하도록 수정
    m_foverlapUmX = (m_stitchCountX == 1) ? 0 : m_sensorSizeX * pixel2UmX - moveUmX;
    m_foverlapUmY = (m_stitchCountY == 1) ? 0 : m_sensorSizeX * pixel2UmY - moveUmY;

    //m_foverlapUmX = m_sensorSizeX * pixel2UmX - moveUmX;
    //m_foverlapUmY = m_sensorSizeY * pixel2UmY - moveUmY;

    m_floatOverlapSizeX_px = scale.convert_umToPixelX(m_foverlapUmX);
    m_floatOverlapSizeY_px = scale.convert_umToPixelY(m_foverlapUmY);
    m_overlapPixelX = (long)(m_floatOverlapSizeX_px + .5f);
    m_overlapPixelY = (long)(m_floatOverlapSizeY_px + .5f);

    // Interpolation 길이는 overlap 영역을 전부 사용할 필요는 없다.
    // Overlap 영역이 5 mm 보다 길면 5 mm 만큼만 사용하자.
    float enoughOverlapLengthUm = 5000.f;
    long enoughOverlapPixel = (long)(enoughOverlapLengthUm * um2Pixel + .5f);
    m_InterpolationLengthX = min(m_overlapPixelX, enoughOverlapPixel);
    m_InterpolationLengthY = min(m_overlapPixelY, enoughOverlapPixel);

    // Interpolation 길이에 맞추어, 시작위치가 이동된다.
    m_InterpolationOffsetX = (m_overlapPixelX - m_InterpolationLengthX) / 2;
    m_InterpolationOffsetY = (m_overlapPixelY - m_InterpolationLengthY) / 2;

    m_frameIndexForStitchInfo = m_processor->m_VisionPara.m_calcFrameIndex.getFrameIndex();

    if (m_stitchCountX > 1 || m_stitchCountY > 1)
    {
        if (m_frameIndexForStitchInfo < 0)
        {
            return false;
        }
    }

    m_prepareStitchInfo.resize(m_stitchCountX * m_stitchCountY);

    for (long y = 0; y < m_stitchCountY; y++)
    {
        for (long x = 0; x < m_stitchCountX; x++)
        {
            long stitchIndex = y * m_stitchCountX + x;
            MakePrepareStitchInfo(x, y, m_prepareStitchInfo[stitchIndex]);
        }
    }

    return true;
}

const Ipvm::Rect32s& Constants::GetHorPreOverlapROI(long stitchIndex) const
{
    return m_prepareStitchInfo[stitchIndex].m_rtHorPre;
}

const Ipvm::Rect32s& Constants::GetHorCurOverlapROI(long stitchIndex) const
{
    return m_prepareStitchInfo[stitchIndex].m_rtHorCur;
}

const Ipvm::Rect32s& Constants::GetVerPreOverlapROI(long stitchIndex) const
{
    return m_prepareStitchInfo[stitchIndex].m_rtVerPre;
}

const Ipvm::Rect32s& Constants::GetVerCurOverlapROI(long stitchIndex) const
{
    return m_prepareStitchInfo[stitchIndex].m_rtVerCur;
}

const Ipvm::Rect32s& Constants::GetSimpleCopyRoi(long stitchIndex) const
{
    return m_prepareStitchInfo[stitchIndex].m_rtSimpleCopy;
}

void Constants::MakePrepareStitchInfo(long x, long y, PreapreStitchInfo& info)
{
    // x=0 또는 y=0 에 위치한 FOV들에 대한 초기화
    info.m_rtVerPre.SetRectEmpty();
    info.m_rtVerCur.SetRectEmpty();
    info.m_rtHorPre.SetRectEmpty();
    info.m_rtHorCur.SetRectEmpty();

    if (y != 0) // 위 아래로 붙는 경우
    {
        // Vertical Overlap ROI Set
        // verPreROIOverlap, imageOrigin 기준
        info.m_rtVerPre.m_left = x * m_sensorSizeX;
        info.m_rtVerPre.m_right = info.m_rtVerPre.m_left + m_sensorSizeX;
        info.m_rtVerPre.m_top = y * m_sensorSizeY - m_overlapPixelY;
        info.m_rtVerPre.m_bottom = info.m_rtVerPre.m_top + m_overlapPixelY;

        // verCurROIOverlap, imageOrigin 기준
        info.m_rtVerCur.m_left = x * m_sensorSizeX;
        info.m_rtVerCur.m_right = info.m_rtVerCur.m_left + m_sensorSizeX;
        info.m_rtVerCur.m_top = y * m_sensorSizeY;
        info.m_rtVerCur.m_bottom = y * m_sensorSizeY + m_overlapPixelY;
    }

    if (x != 0) //좌우로 붙는 경우
    {
        // Horizontal Overlap ROI Set
        // horPreROIOverlap, imageStitch 기준
        info.m_rtHorPre.m_left = x * m_sensorSizeX - m_overlapPixelX;
        info.m_rtHorPre.m_right = info.m_rtHorPre.m_left + m_overlapPixelX;
        info.m_rtHorPre.m_top = y * m_sensorSizeY;
        info.m_rtHorPre.m_bottom = info.m_rtHorPre.m_top + m_sensorSizeY;

        // horCurROIOverlap, imageOrigin 기준
        info.m_rtHorCur.m_left = x * m_sensorSizeX;
        info.m_rtHorCur.m_right = info.m_rtHorCur.m_left + m_overlapPixelX;
        info.m_rtHorCur.m_top = y * m_sensorSizeY;
        info.m_rtHorCur.m_bottom = info.m_rtHorCur.m_top + m_sensorSizeY;
    }

    BOOL imageStitchingInterpolation = (m_processor->m_VisionPara.m_boundaryInterpolation == 1); // TRUE;

    long nCopyOffsetX1, nCopyOffsetY1;
    long nCopyOffsetX2, nCopyOffsetY2;

    if (imageStitchingInterpolation)
    {
        nCopyOffsetX1 = m_InterpolationOffsetX + m_InterpolationLengthX;
        nCopyOffsetY1 = m_InterpolationOffsetY + m_InterpolationLengthY;
        nCopyOffsetX2 = m_InterpolationOffsetX;
        nCopyOffsetY2 = m_InterpolationOffsetY;
    }
    else
    {
        nCopyOffsetX1 = m_overlapPixelX / 2;
        nCopyOffsetY1 = m_overlapPixelY / 2;
        nCopyOffsetX2 = nCopyOffsetX1;
        nCopyOffsetY2 = nCopyOffsetY1;
    }

    auto& CurSimpleCopy = info.m_rtSimpleCopy;

    if (x == 0)
        CurSimpleCopy.m_left = 0;
    else
        CurSimpleCopy.m_left = nCopyOffsetX1;

    if (y == 0)
        CurSimpleCopy.m_top = 0;
    else
        CurSimpleCopy.m_top = nCopyOffsetY1;

    if (x == m_stitchCountX - 1)
        CurSimpleCopy.m_right = m_sensorSizeX;
    else
        CurSimpleCopy.m_right = m_sensorSizeX - m_overlapPixelX
            + nCopyOffsetX1; // 오른쪽, 아래의 interpolation 영역은 여기서 같이 복사된다.

    if (y == m_stitchCountY - 1)
        CurSimpleCopy.m_bottom = m_sensorSizeY;
    else
        CurSimpleCopy.m_bottom = m_sensorSizeY - m_overlapPixelY
            + nCopyOffsetY1; // 오른쪽, 아래의 interpolation 영역은 여기서 같이 복사된다.
}
