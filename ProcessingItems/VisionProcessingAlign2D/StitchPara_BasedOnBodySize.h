#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class Constants;
class VisionScale;

//HDR_6_________________________________ Header body
//
class EdgeSearchRoiResult
{
public:
    Ipvm::Rect32s m_roi;
    long m_searchDirection;

    std::vector<Ipvm::Point32r2> m_edges;
    Ipvm::LineEq32r m_lineEq;
};

class EdgeSearchRoiPara
{
public:
    EdgeSearchRoiPara();

    void Initialize();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    void MakeResult(const VisionScale& scale, const Constants& constants, long searchDirection, long stitchIndex);

    float m_searchLength_um[2]; // Search Length
    float m_persent_start[2]; // 시작점
    float m_persent_end[2]; // 끝점

    float m_resultAngle;
    Ipvm::Point32r2 m_resultRefCornerPoint;
    Ipvm::Point32r2 m_resultCornerPoint;
    std::vector<EdgeSearchRoiResult> m_resultRois;
};

class StitchPara_BasedOnBodySize
{
public:
    StitchPara_BasedOnBodySize();
    ~StitchPara_BasedOnBodySize();

    BOOL LinkDataBase(BOOL bSave, long version, CiDataBase& db);
    void Init();

    long m_edgeDirection;
    long m_searchDirection;
    long m_edgeDetectMode;
    long m_edgeNum;
    float m_firstEdgeMinThreshold;

    std::vector<EdgeSearchRoiPara> m_stitchPara;
};
