#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "CustomFixed.h"
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <vector>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API CustomFixedCollection
{
public:
    long m_nPrevRotateIdx{};

    CustomFixedCollection() = default;
    ~CustomFixedCollection() = default;

    std::vector<CustomFixed> vecCustomFixedData;

    long GetCount() const;
    BOOL DeleteData(long n_idx);
    void ClearData();
    void SwapData(long n_idx1, long n_idx2);
    void ResizeData(long nSize, BOOL bClear = FALSE);
    void FlipMap();
    void MirrorMap();
    void RotateMap(long nRotateIdx);

private:
    void RotateToIndex(long prevIndex, long curIndex);
};
} // namespace Package