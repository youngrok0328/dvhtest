#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionSurfaceCriteriaDefinitions.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
LPCTSTR __DPI_SURFACE_API__ GetCriteriaColumnKey(VisionSurfaceCriteria_Column column);
LPCTSTR __DPI_SURFACE_API__ GetCriteriaColumnName(VisionSurfaceCriteria_Column column);
long __DPI_SURFACE_API__ GetCriteriaColumnSpecType(VisionSurfaceCriteria_Column column);
bool __DPI_SURFACE_API__ GetCriteriaColumnUse(VisionSurfaceCriteria_Column column, bool is3D);
long __DPI_SURFACE_API__ GetCriteriaColumnCount(bool is3D);
VisionSurfaceCriteria_Column __DPI_SURFACE_API__ GetCriteriaColumnID(bool is3D, long index);
