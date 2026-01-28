//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "GlobalSerializerPool.h"

//CPP_2_________________________________ This project's headers
#include "Serializer.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <afxmt.h>

//CPP_5_________________________________ Standard library headers
#include <list>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Ipsc
{
static CCriticalSection s_lock;
static std::list<void*> s_list;

static void Return(Serializer* handle)
{
    CSingleLock lock(&s_lock, TRUE);

    s_list.push_back(handle);
}

std::unique_ptr<Serializer, GlobalSerializerPool::Deleter> GlobalSerializerPool::Pop(UINT bufferGrowSize)
{
    CSingleLock lock(&s_lock, TRUE);

    if (s_list.size())
    {
        auto serializer = s_list.back();

        s_list.pop_back();

        return std::unique_ptr<Serializer, Deleter>(static_cast<Serializer*>(serializer), Return);
    }

    return std::unique_ptr<Serializer, Deleter>(new Serializer(bufferGrowSize), Return);
}

size_t GlobalSerializerPool::GetReservedCount()
{
    CSingleLock lock(&s_lock, TRUE);

    return s_list.size();
}
} // namespace Ipsc
