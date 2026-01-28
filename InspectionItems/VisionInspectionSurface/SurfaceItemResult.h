#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SurfaceCirteriaColumn.h"
#include "SurfaceCriteriaResult.h"
#include "SurfaceSupportMask.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CSurfaceItem;
class VisionScale;

//HDR_6_________________________________ Header body
//
struct SDetectedSurfaceObject
{
    long m_color;

    // 영훈 : 검사 결과 값
    float m_inspValue[long(VisionSurfaceCriteria_Column::Surface_Criteria_END)];

    // 검사 결과에 필요한 변수
    Ipvm::Rect32s rtROI;
    float fCenX;
    float fCenY;
    float fCenX0;
    float fCenY0;
    float fObjectRealCenterX;
    float fObjectRealCenterY;
    PI_RECT arROI;

    long nLabel;
    long nArea;

    void Clear()
    {
        m_color = 0;
        for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
        {
            m_inspValue[item] = 0.f;
        }

        fCenX = 0.f;
        fCenY = 0.f;
        fCenX0 = 0.f;
        fCenY0 = 0.f;
        rtROI = Ipvm::Rect32s(0, 0, 0, 0);
        arROI = rtROI;
        nLabel = 0;
        nArea = 0;
    }
};

class __DPI_SURFACE_API__ CSurfaceItemResult
{
public:
    // Interfaces
    CSurfaceItemResult();
    CSurfaceItemResult(const CSurfaceItemResult& Src);
    ~CSurfaceItemResult();

    CSurfaceItemResult& operator=(const CSurfaceItemResult& Src);
    CSurfaceItemResult& operator+=(const CSurfaceItemResult& Src);

    // Attributes
    long nResult;

    std::vector<CSurfaceCriteriaResult> vecCriteriaResult; // Muti Criteria수 만큼 존재함.

    void InitnResults(const CSurfaceItem& SurfaceItem);
    void UpdateResults(
        const VisionScale& scale, bool is3D, const Ipvm::Point32r2& imageCenter, const CSurfaceItem& SurfItemSpec);

    // Each DSO Total Result
    std::vector<long> vecnResult; //DSO의 개수 만큼 존재함.

    std::vector<SDetectedSurfaceObject> vecDSOValue; // 발견된 Surface Object 들
    SurfaceSupportMask m_supportMask;

private:
    void UpdateCurrentResult(
        BOOL& o_bNotMeasured, BOOL& o_bInvalid, BOOL& o_bReject, BOOL& o_bMarginal, const int i_nResult);
    void UpdateCriteriaResults(
        const VisionScale& scale, bool is3D, const Ipvm::Point32r2& imageCenter, const CSurfaceItem& SurfItemSpec);
    void UpdateTotalResult();
};