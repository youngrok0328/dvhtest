#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "Land.h"

//HDR_3_________________________________ External library headers
#include <vector>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API LandCollection
{
public:
    LandCollection() = default;
    ~LandCollection() = default;

    std::vector<Package::Land> vecLandData;

public:
    long m_nPrevRotateIdx{};
    long m_nPreRotateLandIdx{};

    long GetCount();
    void FlipMap();
    void MirrorMap();
    void RotateMap(long nRotateIdx);
    void RotateLand(long nRotateIdx);

private:
    void RotateToIndex(long prevIndex, long curIndex);
    void RotateToIndex_LandPos(long prevIndex, long curIndex);
};

} // namespace Package