//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ComponentCollection.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "Component.h"
#include "Enum.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>
#include <minwindef.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
long ComponentCollection::GetType(long n_idx)
{
    return (long)vecCompData[n_idx].nCompType;
}

long ComponentCollection::GetCount()
{
    return (long)vecCompData.size();
}

Component* ComponentCollection::GetMapDataPtr(long n_idx)
{
    long vecDataCount = GetCount();
    if (n_idx < 0 || n_idx >= vecDataCount)
        return NULL;
    return &vecCompData[n_idx];
}

Component ComponentCollection::GetMapData(long n_idx)
{
    long vecDataCount = GetCount();
    if (n_idx < 0 || n_idx >= vecDataCount)
    {
        Component compMap;
        return compMap;
    }
    return vecCompData[n_idx];
}

void ComponentCollection::AddData(Component compData)
{
    CalcChangeDataPos(compData);
    vecCompData.push_back(compData);
}

BOOL ComponentCollection::DeleteData(long n_idx)
{
    if (n_idx < 0 || n_idx >= GetCount())
        return FALSE;

    vecCompData.erase(vecCompData.begin() + n_idx);

    return TRUE;
}

void ComponentCollection::ClearData()
{
    vecCompData.clear();
}

void ComponentCollection::ModifyData(long n_idx, Component compData)
{
    CalcChangeDataPos(compData);
    vecCompData[n_idx] = compData;
}

void ComponentCollection::CalcChangeDataPos(Component& io_compData)
{
    float fOffsetX = io_compData.fOffsetX;
    float fOffsetY = io_compData.fOffsetY;
    float fWidth(0.f), fLength(0.f);

    if (io_compData.nAngle == 0 || io_compData.nAngle == 180)
    {
        fWidth = io_compData.fLength * 0.5f;
        fLength = io_compData.fWidth * 0.5f;
    }
    else
    {
        fWidth = io_compData.fWidth * 0.5f;
        fLength = io_compData.fLength * 0.5f;
    }
    Ipvm::Rect32r frtCalcROI;
    frtCalcROI.m_left = (float)(fOffsetX - fWidth);
    frtCalcROI.m_right = (float)(fOffsetX + fWidth);
    frtCalcROI.m_top = (float)(fOffsetY - fLength);
    frtCalcROI.m_bottom = (float)(fOffsetY + fLength);

    FPI_RECT frtChip = FPI_RECT(Ipvm::Point32r2((float)frtCalcROI.m_left, (float)frtCalcROI.m_top),
        Ipvm::Point32r2((float)frtCalcROI.m_right, (float)frtCalcROI.m_top),
        Ipvm::Point32r2((float)frtCalcROI.m_left, (float)frtCalcROI.m_bottom),
        Ipvm::Point32r2((float)frtCalcROI.m_right, (float)frtCalcROI.m_bottom));

    io_compData.fsrtRealROI = frtChip;
}

void ComponentCollection::SwapData(long n_idx1, long n_idx2)
{
    Component compMapData = vecCompData[n_idx1];
    vecCompData[n_idx1] = vecCompData[n_idx2];
    vecCompData[n_idx2] = compMapData;
}

void ComponentCollection::ResizeData(long nSize, BOOL bClear)
{
    if (bClear)
        ClearData();

    vecCompData.resize(nSize);
}

void ComponentCollection::FlipMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        vecCompData[n].fOffsetX = vecCompData[n].fOffsetX * (-1);
        vecCompData[n].fOffsetY = vecCompData[n].fOffsetY;
    }
}

void ComponentCollection::MirrorMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        vecCompData[n].fOffsetX = vecCompData[n].fOffsetX;
        vecCompData[n].fOffsetY = vecCompData[n].fOffsetY * (-1);
    }
}

void ComponentCollection::RotateMap(long nRotateIdx)
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    //if (m_nPrevRotateIdx == nRotateIdx)
    //	return;

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

void ComponentCollection::RotateToIndex(long prevIndex, long curIndex)
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

    float fOffsetX(0.f), fOffsetY(0.f);

    for (long rotate = 0; rotate < rotateCount; rotate++)
    {
        for (long n = 0; n < GetCount(); n++)
        {
            fOffsetX = vecCompData[n].fOffsetY * (-1);
            fOffsetY = vecCompData[n].fOffsetX;

            vecCompData[n].fOffsetX = fOffsetX;
            vecCompData[n].fOffsetY = fOffsetY;

            if (vecCompData[n].nAngle == 90)
            {
                vecCompData[n].nAngle = 0;
            }
            else
            {
                vecCompData[n].nAngle += 90;
                vecCompData[n].nAngle -= (vecCompData[n].nAngle >= 360 ? 360 : 0);
            }
        }
    }

    m_nPrevRotateIdx = curIndex;
}

void ComponentCollection::RotateComp(long nRotateIdx)
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    if (m_nPreRotateCompIdx == nRotateIdx)
        return;

    if (m_nPreRotateCompIdx == Rotate_0)
    {
        RotateToIndex_CompPos(m_nPreRotateCompIdx, nRotateIdx);
    }
    else if (m_nPreRotateCompIdx == Rotate_90)
    {
        RotateToIndex_CompPos(m_nPreRotateCompIdx, nRotateIdx);
    }
    else if (m_nPreRotateCompIdx == Rotate_180)
    {
        RotateToIndex_CompPos(m_nPreRotateCompIdx, nRotateIdx);
    }
    else if (m_nPreRotateCompIdx == Rotate_270)
    {
        RotateToIndex_CompPos(m_nPreRotateCompIdx, nRotateIdx);
    }
}

void ComponentCollection::RotateToIndex_CompPos(long prevIndex, long curIndex)
{
    long rotateCount(0);

    if (prevIndex > curIndex)
        rotateCount = 4 - abs(curIndex - prevIndex);
    else
        rotateCount = curIndex - prevIndex;

    for (long n = 0; n < rotateCount; n++)
    {
        for (auto& CompData : vecCompData)
        {
            if (CompData.nAngle == 90)
                CompData.nAngle = 0;
            else
            {
                CompData.nAngle += 90;
                CompData.nAngle -= (CompData.nAngle >= 360 ? 360 : 0);
            }
        }
    }

    m_nPreRotateCompIdx = curIndex;
}
} // namespace Package
