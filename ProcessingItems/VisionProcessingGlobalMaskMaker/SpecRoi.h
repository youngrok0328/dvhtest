#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Predefine.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class ResultLayerPre;

//HDR_6_________________________________ Header body
//
class SpecRoi
{
public:
    SpecRoi();
    ~SpecRoi();

    void Init();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Set(const Ipvm::Point32r2& px2um, const Ipvm::Point32r2& imageCenter, const ResultLayerPre& layerPre,
        long userRoiIndex);

    UserRoiType getType() const;
    void setType(UserRoiType type);
    long getPolygonPointCount() const;
    void setPolygonPointCount(long count);

    OperationType m_operation;

    CString m_preparedObjectName;
    float m_preparedMaskDilateInUm;
    float m_preparedROIExpandX_um;
    float m_preparedROIExpandY_um;

    Ipvm::Rect32r m_rect;
    Ipvm::Polygon32r m_polygon;

private:
    UserRoiType m_type;
    long m_polygonPointCount;
};
