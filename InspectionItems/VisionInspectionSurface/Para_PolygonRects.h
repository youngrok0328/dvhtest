#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionScale;

//HDR_6_________________________________ Header body
//
class __DPI_SURFACE_API__ Para_PolygonRects
{
public:
    struct SPolygonRectNamePair
    {
        CString strName;
        CString strROIKey;

        short nPolygonType;
        // 0 : Rectangle_32f
        // 1 : Ellpise
        // 2 : Detect Rect //kircheis_Hy
        // 3 ~ : Arbitrary Polygon_32f

        // For Polygon_32f
        std::vector<Ipvm::Point32r2> vecptPolygon_BCU;

        BOOL IsRectIntersectWithPolygon(
            const VisionScale& scale, const Ipvm::Point32r2& imageCenter, const Ipvm::Rect32s& rtObj) const;
    };

    Para_PolygonRects();
    Para_PolygonRects(const Para_PolygonRects& object);
    ~Para_PolygonRects();

    void Init();
    bool IsValid(LPCTSTR maskName) const;
    long FindIndex(LPCTSTR maskName) const;
    BOOL LinkDataBase(BOOL save, CiDataBase& db);

    void Add(const SPolygonRectNamePair& maskInfo);
    bool Del(long orderIndex);
    const SPolygonRectNamePair& GetAt(long orderIndex) const;
    SPolygonRectNamePair& GetAt(long orderIndex);
    long GetCount() const;

    Para_PolygonRects& operator=(const Para_PolygonRects& object);

private:
    std::vector<SPolygonRectNamePair> m_items;
};
