#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ParaEdgeParameters.h"
#include "ParaEdgeSearchRoi.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <array>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class ParaPocketAlign
{
public:
    ParaEdgeParameters m_edgeParams;
    std::array<ParaEdgeSearchRoi, 4> m_edgeSearchRois;

    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};
