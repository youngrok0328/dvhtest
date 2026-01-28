#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
class ParaSprocketHoleAlign
{
public:
    long m_edgeType = 0;
    EdgeDetectMode m_edgeDetectMode = EdgeDetectMode_FirstEdge;
    float m_firstEdgeMinThreshold = 10.f;

    long m_edgeCount = 36;
    float m_persent_start = 0.0f; // 시작점
    float m_persent_end = 150.0f; // 끝점

    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};
