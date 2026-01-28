#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "Component.h"

//HDR_3_________________________________ External library headers
#include <vector>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API ComponentCollection
{
public:
    ComponentCollection() = default;
    ~ComponentCollection() = default;

    std::vector<Component> vecCompData;

    long GetType(long n_idx);
    long GetCount();
    Component* GetMapDataPtr(long n_idx);
    Component GetMapData(long n_idx);

    void AddData(Component compData);
    BOOL DeleteData(long n_idx);
    void ClearData();
    void ModifyData(long n_idx, Component compData);
    void SwapData(long n_idx1, long n_idx2);
    void ResizeData(long nSize, BOOL bClear = FALSE);

    void FlipMap();
    void MirrorMap();
    void RotateMap(long nRotateIdx);
    void RotateComp(long nRotateIdx);
    void CalcChangeDataPos(Component& io_compData);

    long m_nPrevRotateIdx{};
    long m_nPreRotateCompIdx{};

private:
    void RotateToIndex(long prevIndex, long curIndex);
    void RotateToIndex_CompPos(long prevIndex, long curIndex);
};

} // namespace Package