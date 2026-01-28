#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class AlignPara
{
public:
    AlignPara(const AlignPara& object) = delete;
    AlignPara(VisionProcessing& parent);
    ~AlignPara();

    ImageProcPara m_imageProc;
    float m_edgeSearchOffset_um; // Edge Offset
    float m_edgeSearchLength_um; // Edge Search 범위
    long m_edgeSearchDirection; // Edge Search 빙향 : In->Out, Out->In
    long m_edgeType; // Edge : Both, Rising, Falling
    float m_edgeThreshold; // Edge Threshold
    BOOL m_findFirstEdge; // Edge First Edge
    long m_edgeSearchCount; // Edge Search Count
    long m_edgeSearchWidth; // Edge Search Width

    long m_blobThreshold;
    float m_blobSearchOffset_um; // Blob Search 범위

    void Init();

    BOOL LinkDataBase(BOOL save, CiDataBase& db);
    AlignPara& operator=(const AlignPara& object);
};
