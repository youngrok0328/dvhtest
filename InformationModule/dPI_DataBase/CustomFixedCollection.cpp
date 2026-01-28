//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CustomFixedCollection.h"

//CPP_2_________________________________ This project's headers
#include "CustomFixed.h"
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
long CustomFixedCollection::GetCount() const
{
    return (long)vecCustomFixedData.size();
}

BOOL CustomFixedCollection::DeleteData(long n_idx)
{
    if (n_idx < 0 || n_idx >= GetCount())
        return FALSE;

    vecCustomFixedData.erase(vecCustomFixedData.begin() + n_idx);

    return TRUE;
}

void CustomFixedCollection::ClearData()
{
    vecCustomFixedData.clear();
}

void CustomFixedCollection::SwapData(long n_idx1, long n_idx2)
{
    CustomFixed CustomFixedMapData = vecCustomFixedData[n_idx1];
    vecCustomFixedData[n_idx1] = vecCustomFixedData[n_idx2];
    vecCustomFixedData[n_idx2] = CustomFixedMapData;
}

void CustomFixedCollection::ResizeData(long nSize, BOOL bClear)
{
    if (bClear)
        ClearData();

    vecCustomFixedData.resize(nSize);
}

void CustomFixedCollection::FlipMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        if (vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Circle
            || vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Rectangle
            || vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_NotchHole)
        {
            vecCustomFixedData[n].m_fSpecCenterPos_mm.m_x = vecCustomFixedData[n].m_fSpecCenterPos_mm.m_x * (-1.f);
            vecCustomFixedData[n].m_fSpecCenterPos_mm.m_y = vecCustomFixedData[n].m_fSpecCenterPos_mm.m_y;
        }
    }
}

void CustomFixedCollection::MirrorMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        if (vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Circle
            || vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Rectangle
            || vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_NotchHole)
        {
            vecCustomFixedData[n].m_fSpecCenterPos_mm.m_x = vecCustomFixedData[n].m_fSpecCenterPos_mm.m_x;
            vecCustomFixedData[n].m_fSpecCenterPos_mm.m_y = vecCustomFixedData[n].m_fSpecCenterPos_mm.m_y * (-1.f);
        }
    }
}

void CustomFixedCollection::RotateMap(long nRotateIdx)
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

void CustomFixedCollection::RotateToIndex(long prevIndex, long curIndex)
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
            if (vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Circle
                || vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_Rectangle
                || vecCustomFixedData[n].m_eCustomFixedCategory == CustomFixedCategory::CustomFixedCategory_NotchHole)
            {
                float foffsetX = vecCustomFixedData[n].m_fSpecCenterPos_mm.m_y * -1.f;
                float foffsetY = vecCustomFixedData[n].m_fSpecCenterPos_mm.m_x;

                vecCustomFixedData[n].m_fSpecCenterPos_mm.m_x = foffsetX;
                vecCustomFixedData[n].m_fSpecCenterPos_mm.m_y = foffsetY;
            }
        }
    }

    m_nPrevRotateIdx = curIndex;
}
} // namespace Package