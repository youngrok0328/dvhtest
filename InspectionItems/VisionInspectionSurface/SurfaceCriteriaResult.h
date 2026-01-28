#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Para_PolygonRects.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CSurfaceCriteria;
class VisionScale;
struct SDetectedSurfaceObject;
struct SPolygonRectNamePair;

//HDR_6_________________________________ Header body
//
class CSurfaceCriteriaResult
{
public:
    CSurfaceCriteriaResult();
    CSurfaceCriteriaResult(const CSurfaceCriteriaResult& Src);

    CSurfaceCriteriaResult& operator=(const CSurfaceCriteriaResult& Src);

    long nResult;

#if !defined(USE_BY_HANDLER)
    void InitnResults(long nrInit);

    void UpdateEachResult(const VisionScale& scale, bool is3D, const CSurfaceCriteria& Spec, long nID,
        const Ipvm::Point32r2& imageCenter, SDetectedSurfaceObject& DSOValue, const Para_PolygonRects& decisionArea);

    void UpdateTotalResult();

    std::vector<long> vecnResult; // DSO 개수 만큼 존재함.
#endif
};
