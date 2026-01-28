#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Predefine.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class SpecRoi;

//HDR_6_________________________________ Header body
//
struct ResultUserROI
{
    OperationType m_operation;
    UserRoiType m_type;
    Ipvm::Rect32s m_rect;

    CString m_preparedObjectName;
    float m_preparedMaskDilateInUm;
    float m_preparedROIExpandX_um;
    float m_preparedROIExpandY_um;

    std::vector<Ipvm::Point32s2> m_polygon;

    bool operator==(const ResultUserROI& object) const;
    bool operator!=(const ResultUserROI& object) const;
};

class ResultLayerPre
{
public:
    ResultLayerPre();
    ~ResultLayerPre();

    void Add(const Ipvm::Point32r2& px2um, const Ipvm::Point32r2& imageCenter, const SpecRoi& roi);

    bool operator==(const ResultLayerPre& object) const;
    bool operator!=(const ResultLayerPre& object) const;

    // User ROI일 경우 데이터

    std::vector<ResultUserROI> m_userROIs;
};
