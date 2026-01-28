//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CustomPolygonCollection.h"

//CPP_2_________________________________ This project's headers
#include "Enum.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
long CustomPolygonCollection::GetCount() const
{
    return (long)vecCustomPolygonData.size();
}

BOOL CustomPolygonCollection::DeleteData(long n_idx)
{
    if (n_idx < 0 || n_idx >= GetCount())
        return FALSE;

    vecCustomPolygonData.erase(vecCustomPolygonData.begin() + n_idx);

    return TRUE;
}

void CustomPolygonCollection::ClearData()
{
    vecCustomPolygonData.clear();
}

void CustomPolygonCollection::SwapData(long n_idx1, long n_idx2)
{
    CustomPolygon CustomPolygonMapData = vecCustomPolygonData[n_idx1];
    vecCustomPolygonData[n_idx1] = vecCustomPolygonData[n_idx2];
    vecCustomPolygonData[n_idx2] = CustomPolygonMapData;
}

void CustomPolygonCollection::ResizeData(long nSize, BOOL bClear)
{
    if (bClear)
        ClearData();

    vecCustomPolygonData.resize(nSize);
}

void CustomPolygonCollection::FlipMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        if (vecCustomPolygonData[n].m_eCustomPolygonCategory == CustomPolygonCategory::CustomPolygonCategory_Polygon)
        {
            for (auto& vecfptPolygon : vecCustomPolygonData[n].m_vecfptPointInfo_mm)
            {
                vecfptPolygon.m_x = vecfptPolygon.m_x * (-1.f);
                vecfptPolygon.m_y = vecfptPolygon.m_y;
            }
        }
    }
}

void CustomPolygonCollection::MirrorMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        if (vecCustomPolygonData[n].m_eCustomPolygonCategory == CustomPolygonCategory::CustomPolygonCategory_Polygon)
        {
            for (auto& vecfptPolygon : vecCustomPolygonData[n].m_vecfptPointInfo_mm)
            {
                vecfptPolygon.m_x = vecfptPolygon.m_x;
                vecfptPolygon.m_y = vecfptPolygon.m_y * (-1.f);
            }
        }
    }
}

void CustomPolygonCollection::RotateMap(long nRotateIdx)
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    if (m_nPrevRotateIdx == nRotateIdx)
        return;

    if (m_nPrevRotateIdx == Rotate_0)
    {
        RotateToIndex(m_nPrevRotateIdx, nRotateIdx);
    }
    else if (m_nPrevRotateIdx == Rotate_90)
    {
        RotateToIndex(m_nPrevRotateIdx, nRotateIdx);
    }
    else if (m_nPrevRotateIdx == Rotate_180)
    {
        RotateToIndex(m_nPrevRotateIdx, nRotateIdx);
    }
    else if (m_nPrevRotateIdx == Rotate_270)
    {
        RotateToIndex(m_nPrevRotateIdx, nRotateIdx);
    }
}

void CustomPolygonCollection::RotateToIndex(long prevIndex, long curIndex)
{
    long rotateCount;
    if (prevIndex > curIndex)
    {
        rotateCount = 4 - abs(curIndex - prevIndex);
    }
    else
    {
        rotateCount = curIndex - prevIndex;
    }

    for (long rotate = 0; rotate < rotateCount; rotate++)
    {
        for (long n = 0; n < GetCount(); n++)
        {
            if (vecCustomPolygonData[n].m_eCustomPolygonCategory
                == CustomPolygonCategory::CustomPolygonCategory_Polygon)
            {
                for (auto& vecfptPolygon : vecCustomPolygonData[n].m_vecfptPointInfo_mm)
                {
                    float foffsetX = vecfptPolygon.m_y * -1.f;
                    float foffsetY = vecfptPolygon.m_x;

                    vecfptPolygon.m_x = foffsetX;
                    vecfptPolygon.m_y = foffsetY;
                }
            }
        }
    }

    m_nPrevRotateIdx = curIndex;
}

} // namespace Package