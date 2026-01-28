#pragma once

// 이 클래스는 비전마당에서 임시로 가져온 것임
// 추후 비전 마당 라이브러리 것을 사용하도록 변경할 예정. 이현민.

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "SharedMemoryResource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <type_traits>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Ipvm
{
// 이 구현 내부에서는 아무런 메모리 초기화를 수행하지 않는다.
// 메모리를 어떻게 초기화할지는 사용하는 쪽에서 시나리오에 따라 결정한다.
// readOnly 템플릿 인자는 읽기 전용으로 공유 메모리를 열지 여부를 결정한다.
// 읽기 전용으로 열 경우, 메모리에 쓰기 작업을 수행하면 Access Violation 예외가 발생한다.
template<typename ObjectType, bool readOnly = false>
class SharedMemory
{
public:
    SharedMemory() = delete;
    SharedMemory(const SharedMemory& rhs) = delete;
    SharedMemory(SharedMemory&& rhs) = delete;

    SharedMemory(const wchar_t* sharedMemoryName, const size_t maxCount = 1)
        : m_maxCount(maxCount)
        , m_maxBytes(sizeof(ObjectType) * maxCount)
        , m_memoryAddress(static_cast<ObjectType*>(SharedMemoryResource::New(sharedMemoryName, m_maxBytes, readOnly)))
    {
        static_assert(std::is_standard_layout_v<ObjectType>, "ObjectType is not standard layout");
        static_assert(std::is_trivial_v<ObjectType>, "ObjectType is not trivial");
    }

    SharedMemory(const char* sharedMemoryName, const size_t maxCount = 1)
        : m_maxCount(maxCount)
        , m_maxBytes(sizeof(ObjectType) * maxCount)
        , m_memoryAddress(static_cast<ObjectType*>(SharedMemoryResource::New(sharedMemoryName, m_maxBytes, readOnly)))
    {
        static_assert(std::is_standard_layout_v<ObjectType>, "ObjectType is not standard layout");
        static_assert(std::is_trivial_v<ObjectType>, "ObjectType is not trivial");
    }

    ~SharedMemory()
    {
        SharedMemoryResource::Delete(m_memoryAddress);
    }

    SharedMemory& operator=(const SharedMemory& rhs) = delete;
    SharedMemory& operator=(SharedMemory&& rhs) = delete;

    const ObjectType* operator->() const
    {
        return m_memoryAddress;
    }

    ObjectType* operator->()
    {
        static_assert(readOnly == false, "Cannot modify read-only shared memory");
        return m_memoryAddress;
    }

    const ObjectType& operator*() const
    {
        return *m_memoryAddress;
    }

    ObjectType& operator*()
    {
        static_assert(readOnly == false, "Cannot modify read-only shared memory");
        return *m_memoryAddress;
    }

    const ObjectType& operator[](const size_t index) const
    {
        return m_memoryAddress[index];
    }

    ObjectType& operator[](const size_t index)
    {
        static_assert(readOnly == false, "Cannot modify read-only shared memory");
        return m_memoryAddress[index];
    }

    const ObjectType* GetMemoryAddress() const
    {
        return m_memoryAddress;
    }

    ObjectType* GetMemoryAddress()
    {
        static_assert(readOnly == false, "Cannot modify read-only shared memory");
        return m_memoryAddress;
    }

    void Reset()
    {
        static_assert(readOnly == false, "Cannot modify read-only shared memory");
        ::memset(m_memoryAddress, 0, m_maxBytes);
    }

    size_t GetMaxCount() const
    {
        return m_maxCount;
    }

    size_t GetMaxBytes() const
    {
        return m_maxBytes;
    }

private:
    const size_t m_maxCount;
    const size_t m_maxBytes;

    ObjectType* const m_memoryAddress;
};
} // namespace Ipvm
