#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __DPI_EDGEDETECT_API__ CPI_EdgeDetect
{
public:
    CPI_EdgeDetect();
    ~CPI_EdgeDetect();

    float SetMininumThreshold(float i_fthreshold);
    float GetMininumThreshold();

    //정리
    BOOL PI_ED_CalcAvgEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, int nSearchDir, int i_nSearchLength,
        int i_nSearchWidth, const Ipvm::Image8u& image, Ipvm::Point32r3& o_edgeXYV, BOOL bIsFirst = FALSE);

    BOOL PI_ED_CalcAvgEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, int nSearchDir, int i_nSearchLength,
        int i_nSearchWidth, const Ipvm::Image8u& image, Ipvm::Point32r2& o_edgeXY, BOOL bIsFirst = FALSE);

    BOOL PI_ED_CalcEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, Ipvm::Point32s2 i_ptEnd,
        const Ipvm::Image8u& image, Ipvm::Point32r3& o_edgeXYV, BOOL bIsFirst = FALSE);

    BOOL PI_ED_CalcEdgePointFromImage(int edgeType, Ipvm::Point32s2 i_ptStart, Ipvm::Point32s2 i_ptEnd,
        const Ipvm::Image8u& image, Ipvm::Point32r2& o_edgeXY, BOOL bIsFirst = FALSE);

    BOOL PI_ED_CalcEdgePointFromImage(const Ipvm::Image8u& image, int edgeType, int direction, const Ipvm::Rect32s& roi,
        long findEdgeNum, long searchWidth, std::vector<Ipvm::Point32r2>& o_edges, BOOL bIsFirst = FALSE);

    BOOL PI_ED_CalcEdgePointFromLine(int i_eddir, int i_ndnum, BYTE* i_byLineI, float& o_fedge, float& o_fV,
        BOOL& o_bSubPixelingDone, BOOL bIsFirst = FALSE);

private:
    Ipvm::EdgeDetection* m_edgeDetection;
    Ipvm::EdgeDetectionPara* m_edgeDetectionPara;
    Ipvm::EdgeDetectionFilter* m_edgeDetectionFilter;
};
