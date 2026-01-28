//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dPI_EdgeDetectLib.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/EdgeDetection.h>
#include <Ipvm/Algorithm/EdgeDetectionFilter.h>
#include <Ipvm/Algorithm/EdgeDetectionPara.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32r3.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CPI_EdgeDetect::CPI_EdgeDetect()
    : m_edgeDetection(new Ipvm::EdgeDetection)
    , m_edgeDetectionPara(new Ipvm::EdgeDetectionPara)
    , m_edgeDetectionFilter(new Ipvm::EdgeDetectionFilter)
{
}

CPI_EdgeDetect::~CPI_EdgeDetect()
{
    delete m_edgeDetectionFilter;
    delete m_edgeDetectionPara;
    delete m_edgeDetection;
}

float CPI_EdgeDetect::SetMininumThreshold(float i_fthreshold)
{
    float fOldValue = m_edgeDetectionPara->m_edgeThreshold;

    m_edgeDetectionPara->m_edgeThreshold = i_fthreshold;

    return fOldValue;
}

float CPI_EdgeDetect::GetMininumThreshold()
{
    return m_edgeDetectionPara->m_edgeThreshold;
}

BOOL CPI_EdgeDetect::PI_ED_CalcEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, Ipvm::Point32s2 i_ptEnd,
    const Ipvm::Image8u& image, Ipvm::Point32r3& o_edgeXYV, BOOL bIsFirst)
{
    if (!Ipvm::Rect32s(image).PtInRect(i_ptStart))
        return FALSE;
    if (!Ipvm::Rect32s(image).PtInRect(i_ptEnd))
        return FALSE;

    switch (edgeType)
    {
        case PI_ED_DIR_BOTH:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_both;
            break;
        case PI_ED_DIR_RISING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_rising;
            break;
        case PI_ED_DIR_FALLING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_falling;
            break;
        default:
            ASSERT(!"XX");
            return FALSE;
    }

    m_edgeDetectionPara->m_findFirstEdge = bIsFirst ? true : false;

    return Ipvm::Status::e_ok
            == m_edgeDetection->DetectEdge(
                image, *m_edgeDetectionPara, *m_edgeDetectionFilter, i_ptStart, i_ptEnd, 1, o_edgeXYV)
        ? TRUE
        : FALSE;
}

BOOL CPI_EdgeDetect::PI_ED_CalcEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, Ipvm::Point32s2 i_ptEnd,
    const Ipvm::Image8u& image, Ipvm::Point32r2& o_edgeXY, BOOL bIsFirst)
{
    Ipvm::Point32r3 edgeXYV;
    if (!PI_ED_CalcEdgePointFromImage(edgeType, i_ptStart, i_ptEnd, image, edgeXYV, bIsFirst))
    {
        return FALSE;
    }

    o_edgeXY.m_x = edgeXYV.m_x;
    o_edgeXY.m_y = edgeXYV.m_y;

    return TRUE;
}

BOOL CPI_EdgeDetect::PI_ED_CalcEdgePointFromImage(const Ipvm::Image8u& image, int edgeType, int direction,
    const Ipvm::Rect32s& roi, long findEdgeNum, long searchWidth, std::vector<Ipvm::Point32r2>& o_edges, BOOL bIsFirst)
{
    if (findEdgeNum <= 0)
    {
        o_edges.clear();
        return FALSE;
    }

    switch (edgeType)
    {
        case PI_ED_DIR_BOTH:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_both;
            break;
        case PI_ED_DIR_RISING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_rising;
            break;
        case PI_ED_DIR_FALLING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_falling;
            break;
        default:
            ASSERT(!"XX");
            o_edges.clear();
            return FALSE;
    }

    switch (direction)
    {
        case LEFT:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toLeft;
            break;
        case RIGHT:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toRight;
            break;
        case UP:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toUp;
            break;
        case DOWN:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toDown;
            break;
        default:
            ASSERT(!"XX");
            o_edges.clear();
            return FALSE;
    }

    std::vector<Ipvm::Point32r3> edges;
    edges.resize(findEdgeNum);
    m_edgeDetectionPara->m_findFirstEdge = bIsFirst ? true : false;

    int32_t foundEdgeNum = 0;
    if (m_edgeDetection->DetectEdges(
            image, *m_edgeDetectionPara, *m_edgeDetectionFilter, roi, findEdgeNum, searchWidth, &edges[0], foundEdgeNum)
        != Ipvm::Status::e_ok)
    {
        o_edges.clear();
        return false;
    }

    o_edges.resize(foundEdgeNum);
    for (long index = 0; index < foundEdgeNum; index++)
    {
        o_edges[index].m_x = edges[index].m_x;
        o_edges[index].m_y = edges[index].m_y;
    }
    return TRUE;
}

BOOL CPI_EdgeDetect::PI_ED_CalcAvgEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, int nSearchDir,
    int i_nSearchLength, int i_nSearchWidth, const Ipvm::Image8u& image, Ipvm::Point32r3& o_edgeXYV, BOOL bIsFirst)
{
    if (!Ipvm::Rect32s(image).PtInRect(i_ptStart))
        return FALSE;

    switch (edgeType)
    {
        case PI_ED_DIR_BOTH:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_both;
            break;
        case PI_ED_DIR_RISING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_rising;
            break;
        case PI_ED_DIR_FALLING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_falling;
            break;
        default:
            ASSERT(!"XX");
            return FALSE;
    }

    switch (nSearchDir)
    {
        case LEFT:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toLeft;
            if (!Ipvm::Rect32s(image).PtInRect(i_ptStart + Ipvm::Point32s2(-i_nSearchLength, -i_nSearchWidth / 2)))
                return FALSE;
            if (!Ipvm::Rect32s(image).PtInRect(i_ptStart + Ipvm::Point32s2(-i_nSearchLength, +i_nSearchWidth / 2 - 1)))
                return FALSE;
            break;
        case UP:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toUp;
            if (!Ipvm::Rect32s(image).PtInRect(i_ptStart + Ipvm::Point32s2(-i_nSearchWidth / 2, -i_nSearchLength)))
                return FALSE;
            if (!Ipvm::Rect32s(image).PtInRect(i_ptStart + Ipvm::Point32s2(+i_nSearchWidth / 2 - 1, -i_nSearchLength)))
                return FALSE;
            break;
        case RIGHT:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toRight;
            if (!Ipvm::Rect32s(image).PtInRect(i_ptStart + Ipvm::Point32s2(+i_nSearchLength - 1, -i_nSearchWidth / 2)))
                return FALSE;
            if (!Ipvm::Rect32s(image).PtInRect(
                    i_ptStart + Ipvm::Point32s2(+i_nSearchLength - 1, +i_nSearchWidth / 2 - 1)))
                return FALSE;
            break;
        case DOWN:
            m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toDown;
            if (!Ipvm::Rect32s(image).PtInRect(i_ptStart + Ipvm::Point32s2(-i_nSearchWidth / 2, +i_nSearchLength - 1)))
                return FALSE;
            if (!Ipvm::Rect32s(image).PtInRect(
                    i_ptStart + Ipvm::Point32s2(+i_nSearchWidth / 2 - 1, +i_nSearchLength - 1)))
                return FALSE;
            break;
        default:
            ASSERT(!"XX");
            return FALSE;
    }

    m_edgeDetectionPara->m_findFirstEdge = bIsFirst ? true : false;

    return Ipvm::Status::e_ok
            == m_edgeDetection->DetectEdge(image, *m_edgeDetectionPara, *m_edgeDetectionFilter, i_ptStart, 0,
                i_nSearchLength, i_nSearchWidth, o_edgeXYV)
        ? TRUE
        : FALSE;
}

BOOL CPI_EdgeDetect::PI_ED_CalcAvgEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, int nSearchDir,
    int i_nSearchLength, int i_nSearchWidth, const Ipvm::Image8u& image, Ipvm::Point32r2& o_edgeXY, BOOL bIsFirst)
{
    Ipvm::Point32r3 edgeXYV;
    if (!PI_ED_CalcAvgEdgePointFromImage(
            edgeType, i_ptStart, nSearchDir, i_nSearchLength, i_nSearchWidth, image, edgeXYV, bIsFirst))
    {
        return FALSE;
    }

    o_edgeXY.m_x = edgeXYV.m_x;
    o_edgeXY.m_y = edgeXYV.m_y;

    return TRUE;
}

BOOL CPI_EdgeDetect::PI_ED_CalcEdgePointFromLine(
    int i_eddir, int i_ndnum, BYTE* i_byLineI, float& o_fedge, float& o_fV, BOOL& o_bSubPixelingDone, BOOL bIsFirst)
{
    switch (i_eddir)
    {
        case PI_ED_DIR_BOTH:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_both;
            break;
        case PI_ED_DIR_RISING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_rising;
            break;
        case PI_ED_DIR_FALLING:
            m_edgeDetectionPara->m_edgeType = Ipvm::EdgeType::e_falling;
            break;
        default:
            ASSERT(!"XX");
            return FALSE;
    }

    m_edgeDetectionPara->m_findFirstEdge = bIsFirst ? true : false;
    m_edgeDetectionPara->m_searchDirection = Ipvm::SearchDirection::e_toRight;

    Ipvm::Image8u srcImage(i_ndnum, 1, i_byLineI, i_ndnum);

    Ipvm::Point32r3 found = {0.f, 0.f, 0.f};

    BOOL ret = Ipvm::Status::e_ok
            == m_edgeDetection->DetectEdge(
                srcImage, *m_edgeDetectionPara, *m_edgeDetectionFilter, Ipvm::Point32s2(0, 0), 0, i_ndnum, 1, found)
        ? TRUE
        : FALSE;

    o_bSubPixelingDone = ret;

    o_fedge = found.m_x;
    o_fV = found.m_z;

    return ret;
}
