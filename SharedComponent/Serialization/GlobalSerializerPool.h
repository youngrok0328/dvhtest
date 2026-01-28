#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
namespace Ipsc
{
class Serializer;
}

typedef unsigned int UINT;

//HDR_6_________________________________ Header body
//
namespace Ipsc
{
struct __INTEKPLUS_SHARED_SERIALIZATION_API__ GlobalSerializerPool
{
    using Deleter = void (*)(Serializer*);

    static std::unique_ptr<Serializer, Deleter> Pop(UINT bufferGrowSize);

    static size_t GetReservedCount();
};
} // namespace Ipsc
