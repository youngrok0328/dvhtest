#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionScale;

//HDR_6_________________________________ Header body
//
class ParaEdgeSearchRoi
{
public:
    ParaEdgeSearchRoi();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    Ipvm::Rect32s ToRect(const VisionScale& scale, long direction, const Ipvm::Point32r2& center_px, float rectSizeX_px,
        float rectSizeY_px) const;

    float m_searchLength_um = 500.f; // Search Length
    float m_persent_start = 0.f; // 시작점
    float m_persent_end = 100.f; // 끝점
};
