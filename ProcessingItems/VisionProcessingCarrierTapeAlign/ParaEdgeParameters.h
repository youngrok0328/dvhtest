#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class ParaEdgeParameters
{
public:
    long m_edgeType = 0;
    long m_searchDirection = 0;
    long m_edgeDetectMode = EdgeDetectMode_BestEdge;
    float m_firstEdgeMinThreshold = 10.f;
    float m_edgeAlignGap_um = 40.f;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};
