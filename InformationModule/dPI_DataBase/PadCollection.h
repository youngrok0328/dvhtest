#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "Pad.h"

//HDR_3_________________________________ External library headers
#include <minwindef.h>
#include <vector>
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API PadCollection
{
public:
    long m_nPrevRotateIdx{};

    PadCollection() = default;
    ~PadCollection() = default;

    std::vector<Pad> vecPadData;

public:
    long GetType(long n_idx);
    long GetCount() const;
    Pad* GetMapDataPtr(long n_idx);
    Pad GetMapData(long n_idx);

    void AddData(Pad padData);
    BOOL DeleteData(long n_idx);
    void ClearData();
    void ModifyData(long n_idx, Pad padData);
    void SwapData(long n_idx1, long n_idx2);
    void ResizeData(long nSize, BOOL bClear = FALSE);

    void FlipMap();
    void MirrorMap();
    void RotateMap(long nRotateIdx);
    void RotateComp();
    void CalcChangeDataPos(Pad& io_padData);

private:
    void RotateToIndex(long prevIndex, long curIndex);
};

} // namespace Package