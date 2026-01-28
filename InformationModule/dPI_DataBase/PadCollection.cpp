//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PadCollection.h"

//CPP_2_________________________________ This project's headers
#include "Pad.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
long PadCollection::GetType(long n_idx)
{
    return (long)vecPadData[n_idx].GetType();
}

long PadCollection::GetCount() const
{
    return (long)vecPadData.size();
}

Pad* PadCollection::GetMapDataPtr(long n_idx)
{
    long vecDataCount = GetCount();
    if (n_idx < 0 || n_idx >= vecDataCount)
        return NULL;
    return &vecPadData[n_idx];
}

Pad PadCollection::GetMapData(long n_idx)
{
    long vecDataCount = GetCount();
    if (n_idx < 0 || n_idx >= vecDataCount)
    {
        Pad padMap;
        return padMap;
    }
    return vecPadData[n_idx];
}

void PadCollection::AddData(Pad padData)
{
    CalcChangeDataPos(padData);
    vecPadData.push_back(padData);
}

BOOL PadCollection::DeleteData(long n_idx)
{
    if (n_idx < 0 || n_idx >= GetCount())
        return FALSE;

    vecPadData.erase(vecPadData.begin() + n_idx);

    return TRUE;
}

void PadCollection::ClearData()
{
    vecPadData.clear();
}

void PadCollection::ModifyData(long n_idx, Pad padData)
{
    CalcChangeDataPos(padData);
    vecPadData[n_idx] = padData;
}

void PadCollection::CalcChangeDataPos(Pad& io_padData)
{
    auto& padCenter = io_padData.GetCenter();
    float fWidth(0.f), fLength(0.f);

    if (io_padData.nAngle == 0 || io_padData.nAngle == 180)
    {
        fWidth = io_padData.fLength * 0.5f;
        fLength = io_padData.fWidth * 0.5f;
    }
    else
    {
        fWidth = io_padData.fWidth * 0.5f;
        fLength = io_padData.fLength * 0.5f;
    }
    Ipvm::Rect32r frtCalcROI;
    frtCalcROI.m_left = (float)(padCenter.m_x - fWidth);
    frtCalcROI.m_right = (float)(padCenter.m_x + fWidth);
    frtCalcROI.m_top = (float)(padCenter.m_y - fLength);
    frtCalcROI.m_bottom = (float)(padCenter.m_y + fLength);

    FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)frtCalcROI.m_left, (float)frtCalcROI.m_top),
        Ipvm::Point32r2((float)frtCalcROI.m_right, (float)frtCalcROI.m_top),
        Ipvm::Point32r2((float)frtCalcROI.m_left, (float)frtCalcROI.m_bottom),
        Ipvm::Point32r2((float)frtCalcROI.m_right, (float)frtCalcROI.m_bottom));

    //io_padData.fsrtRealROI = frtChip;
}

void PadCollection::SwapData(long n_idx1, long n_idx2)
{
    Pad padMapData = vecPadData[n_idx1];
    vecPadData[n_idx1] = vecPadData[n_idx2];
    vecPadData[n_idx2] = padMapData;
}

void PadCollection::ResizeData(long nSize, BOOL bClear)
{
    if (bClear)
        ClearData();

    vecPadData.resize(nSize);
}

void PadCollection::FlipMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        vecPadData[n].SetOffsetX(vecPadData[n].GetOffset().m_x * -1.f);
    }
}

void PadCollection::MirrorMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        vecPadData[n].SetOffsetY(vecPadData[n].GetOffset().m_y * -1.f);
    }
}

void PadCollection::RotateMap(long nRotateIdx)
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

void PadCollection::RotateToIndex(long prevIndex, long curIndex)
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
            float offsetX = vecPadData[n].GetOffset().m_y * -1.f;
            float offsetY = vecPadData[n].GetOffset().m_x;

            vecPadData[n].SetOffsetX(offsetX);
            vecPadData[n].SetOffsetY(offsetY);

            if (vecPadData[n].nAngle == 90)
            {
                vecPadData[n].nAngle = 0;
            }
            else
            {
                vecPadData[n].nAngle += 90;
                vecPadData[n].nAngle -= (vecPadData[n].nAngle >= 360 ? 360 : 0);
            }
        }
    }

    m_nPrevRotateIdx = curIndex;
}

void PadCollection::RotateComp()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        if (vecPadData[n].nAngle == 90)
        {
            vecPadData[n].nAngle = 0;
        }
        else
        {
            vecPadData[n].nAngle += 90;
            vecPadData[n].nAngle -= (vecPadData[n].nAngle >= 360 ? 360 : 0);
        }
    }
}

} // namespace Package