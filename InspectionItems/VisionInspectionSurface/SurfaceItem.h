#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Para_PolygonRects.h"
#include "SurfaceAlgoPara.h"
#include "SurfaceCriteria.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
// 하나의 검사항목에 해당됨.
class __DPI_SURFACE_API__ CSurfaceItem
{
public:
    // Interfaces
    CSurfaceItem();
    CSurfaceItem(const CSurfaceItem& Src);

    CSurfaceItem& operator=(const CSurfaceItem& Src);

    void Init();

    BOOL IsNeed_Spec(VisionSurfaceCriteria_Column column) const;
    BOOL IsNeed_BrightObject() const;
    BOOL IsNeed_DarkObject() const;
    BOOL IsNeed_KeyContrastAverageContrast() const;
    BOOL IsNeed_WidthLengthAspectRatioBinaryDensity() const;
    BOOL IsNeed_ThicknessLocus() const;
    BOOL IsNeed_LengthXY() const;
    BOOL IsNeed_Local_Area() const;

    // Attributes
    CString strName;

    BOOL bInsp;

    BOOL b2ndInsp; // 딥러닝 사용 검사
    CString str2ndCode; // 딥러닝 모델 매칭 코드

    std::vector<CSurfaceCriteria> vecCriteria; // Mutti Inspection Criteria가 된다. 설정에 따라 제한없이 사용된다.

    CSurfaceAlgoPara AlgoPara;
    Para_PolygonRects m_decisionArea; // 여러개의 Decision Area를 가질 수 있다.

    long m_bSurfaceMaskMode;
    BOOL m_bSurfaceMask_GenerateAlways;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};