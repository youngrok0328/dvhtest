//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "LandCollection.h"

//CPP_2_________________________________ This project's headers
#include "Enum.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <cstdlib>

//CPP_6_________________________________ Preprocessor macros
//CPP_7_________________________________ Implementation body
//
namespace Package
{
long LandCollection::GetCount()
{
    return (long)vecLandData.size();
}

void LandCollection::RotateMap(long nRotateIdx)
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

void LandCollection::RotateToIndex(long prevIndex, long curIndex)
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

    for (long rotateIndex = 0; rotateIndex < rotateCount; rotateIndex++)
    {
        for (long n = 0; n < GetCount(); n++)
        {
            fOffsetX = vecLandData[n].fOffsetY * (-1);
            fOffsetY = vecLandData[n].fOffsetX;

            vecLandData[n].fOffsetX = fOffsetX;
            vecLandData[n].fOffsetY = fOffsetY;

            if (vecLandData[n].nAngle == 90)
            {
                vecLandData[n].nAngle = 0;
            }
            else
            {
                vecLandData[n].nAngle += 90;
                vecLandData[n].nAngle -= (vecLandData[n].nAngle >= 360 ? 360 : 0);
            }
        }
    }

    m_nPrevRotateIdx = curIndex;
}

void LandCollection::RotateLand(long nRotateIdx)
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    if (m_nPreRotateLandIdx == nRotateIdx)
        return;

    if (m_nPreRotateLandIdx == Rotate_0)
    {
        RotateToIndex_LandPos(m_nPreRotateLandIdx, nRotateIdx);
    }
    else if (m_nPreRotateLandIdx == Rotate_90)
    {
        RotateToIndex_LandPos(m_nPreRotateLandIdx, nRotateIdx);
    }
    else if (m_nPreRotateLandIdx == Rotate_180)
    {
        RotateToIndex_LandPos(m_nPreRotateLandIdx, nRotateIdx);
    }
    else if (m_nPreRotateLandIdx == Rotate_270)
    {
        RotateToIndex_LandPos(m_nPreRotateLandIdx, nRotateIdx);
    }
}

void LandCollection::RotateToIndex_LandPos(long prevIndex, long curIndex)
{
    long rotateCount(0);

    if (prevIndex > curIndex)
        rotateCount = 4 - abs(curIndex - prevIndex);
    else
        rotateCount = curIndex - prevIndex;

    for (long n = 0; n < rotateCount; n++)
    {
        for (auto& LandData : vecLandData)
        {
            if (LandData.nAngle == 90)
                LandData.nAngle = 0;
            else
            {
                LandData.nAngle += 90;
                LandData.nAngle -= (LandData.nAngle >= 360 ? 360 : 0);
            }
        }
    }

    m_nPreRotateLandIdx = curIndex;
}

void LandCollection::FlipMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        vecLandData[n].fOffsetX = vecLandData[n].fOffsetX * (-1);
        vecLandData[n].fOffsetY = vecLandData[n].fOffsetY;
    }
}

void LandCollection::MirrorMap()
{
    long nDataCount = GetCount();

    if (nDataCount <= 0)
        return;

    for (long n = 0; n < nDataCount; n++)
    {
        vecLandData[n].fOffsetX = vecLandData[n].fOffsetX;
        vecLandData[n].fOffsetY = vecLandData[n].fOffsetY * (-1);
    }
}

} // namespace Package
