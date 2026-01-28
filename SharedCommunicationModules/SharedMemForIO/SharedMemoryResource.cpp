//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SharedMemoryResource.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <unordered_map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Ipvm
{
class SharedMemoryResourceHandle
{
public:
    SharedMemoryResourceHandle(const wchar_t* sharedMemoryName, const size_t bytes, const bool readOnly)
    {
        LARGE_INTEGER chunkBytesLargeInt{};
        chunkBytesLargeInt.QuadPart = static_cast<LONGLONG>(bytes);

        m_fileMappingHandle = ::CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            chunkBytesLargeInt.HighPart, chunkBytesLargeInt.LowPart, sharedMemoryName);

        if (m_fileMappingHandle)
        {
            m_memoryPointer
                = ::MapViewOfFile(m_fileMappingHandle, readOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, 0);
        }
    }

    SharedMemoryResourceHandle(const char* sharedMemoryName, const size_t bytes, const bool readOnly)
    {
        LARGE_INTEGER chunkBytesLargeInt{};
        chunkBytesLargeInt.QuadPart = static_cast<LONGLONG>(bytes);

        m_fileMappingHandle = ::CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
            chunkBytesLargeInt.HighPart, chunkBytesLargeInt.LowPart, sharedMemoryName);

        if (m_fileMappingHandle)
        {
            m_memoryPointer
                = ::MapViewOfFile(m_fileMappingHandle, readOnly ? FILE_MAP_READ : FILE_MAP_ALL_ACCESS, 0, 0, 0);
        }
    }

    ~SharedMemoryResourceHandle()
    {
        if (m_memoryPointer)
        {
            ::UnmapViewOfFile(m_memoryPointer);
        }

        if (m_fileMappingHandle)
        {
            ::CloseHandle(m_fileMappingHandle);
        }
    }

    void* GetMemoryPointer() const
    {
        return m_memoryPointer;
    }

private:
    HANDLE m_fileMappingHandle{};
    void* m_memoryPointer{};
};

class SharedMemoryResourceHandleMap
{
public:
    SharedMemoryResourceHandleMap()
    {
        ::InitializeCriticalSection(&m_criticalSection);
    }

    ~SharedMemoryResourceHandleMap()
    {
        ::DeleteCriticalSection(&m_criticalSection);
    }

    void* New(const wchar_t* sharedMemoryName, const size_t bytes, const bool readOnly)
    {
        auto* newHandle = new SharedMemoryResourceHandle(sharedMemoryName, bytes, readOnly);
        {
            ::EnterCriticalSection(&m_criticalSection);

            m_handleMap[newHandle->GetMemoryPointer()] = newHandle;

            ::LeaveCriticalSection(&m_criticalSection);
        }

        return newHandle->GetMemoryPointer();
    }

    void* New(const char* sharedMemoryName, const size_t bytes, const bool readOnly)
    {
        auto* newHandle = new SharedMemoryResourceHandle(sharedMemoryName, bytes, readOnly);
        {
            ::EnterCriticalSection(&m_criticalSection);

            m_handleMap[newHandle->GetMemoryPointer()] = newHandle;

            ::LeaveCriticalSection(&m_criticalSection);
        }

        return newHandle->GetMemoryPointer();
    }

    void Delete(void* memoryAddress)
    {
        SharedMemoryResourceHandle* handle{};
        {
            ::EnterCriticalSection(&m_criticalSection);

            auto it = m_handleMap.find(memoryAddress);
            if (it != m_handleMap.end())
            {
                handle = it->second;
                m_handleMap.erase(it);
            }

            ::LeaveCriticalSection(&m_criticalSection);
        }

        delete handle;
    }

private:
    std::unordered_map<void*, SharedMemoryResourceHandle*> m_handleMap;
    CRITICAL_SECTION m_criticalSection;
};

static SharedMemoryResourceHandleMap* s_sharedMemoryResourceHandleMap{};

void InitializeStaticResourceSharedMemoryResourceHandleMap()
{
    if (s_sharedMemoryResourceHandleMap == nullptr)
    {
        s_sharedMemoryResourceHandleMap = new SharedMemoryResourceHandleMap;
    }
}

void TerminateStaticResourceSharedMemoryResourceHandleMap()
{
    delete s_sharedMemoryResourceHandleMap;
    s_sharedMemoryResourceHandleMap = nullptr;
}

void* SharedMemoryResource::New(const wchar_t* sharedMemoryName, const size_t bytes, const bool readOnly)
{
    return s_sharedMemoryResourceHandleMap->New(sharedMemoryName, bytes, readOnly);
}

void* SharedMemoryResource::New(const char* sharedMemoryName, const size_t bytes, const bool readOnly)
{
    return s_sharedMemoryResourceHandleMap->New(sharedMemoryName, bytes, readOnly);
}

void SharedMemoryResource::Delete(void* memoryAddress)
{
    s_sharedMemoryResourceHandleMap->Delete(memoryAddress);
}
} // namespace Ipvm
