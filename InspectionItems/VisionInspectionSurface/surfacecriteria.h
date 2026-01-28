#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "InspBase.h"
#include "SurfaceCirteriaColumn.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
struct SDetectedSurfaceObject;

//HDR_6_________________________________ Header body
//
class __DPI_SURFACE_API__ CSurfaceCriteria
{
public:
    // Interfaces
    CSurfaceCriteria();
    CSurfaceCriteria(const CSurfaceCriteria& Src);
    ~CSurfaceCriteria();

    CSurfaceCriteria& operator=(const CSurfaceCriteria& Src);

    long Judge(bool is3D, SDetectedSurfaceObject& DSOValue) const;

    // Attributes
    void Init();

    BOOL bInsp; // Criteria Usability

    CString strName; // Criteria Name
    std::vector<BOOL> vecbUseDecisionArea;

    SItemSpec& GetSpecMin(VisionSurfaceCriteria_Column column);
    SItemSpec& GetSpecMax(VisionSurfaceCriteria_Column column);
    const SItemSpec& GetSpecMin(VisionSurfaceCriteria_Column column) const;
    const SItemSpec& GetSpecMax(VisionSurfaceCriteria_Column column) const;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    long m_color; // 0 : DARK, 1 : BRIGHT, 2 : ANY
    long m_type; // 0 : INGORE CRITERIA, 1 : VISION_REJECT CRITERIA

private:
    SItemSpec* m_specTable_min[long(VisionSurfaceCriteria_Column::Surface_Criteria_END)];
    SItemSpec* m_specTable_max[long(VisionSurfaceCriteria_Column::Surface_Criteria_END)];
};
