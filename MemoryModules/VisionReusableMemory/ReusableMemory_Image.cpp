//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ReusableMemory_Image.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ReusableMemory_Buffer::ReusableMemory_Buffer(__int64 size)
    : m_buffer(new BYTE[size])
    , m_bufferSize(size)
{
    m_unusedSPosToSize[0] = size;
    m_unusedEPosToSize[size] = size;
    m_unusedSizeToPos[size][0] = TRUE;
}

ReusableMemory_Buffer::~ReusableMemory_Buffer()
{
    delete[] m_buffer;
}

__int64 ReusableMemory_Buffer::getAllocSize() const
{
    return m_bufferSize;
}

__int64 ReusableMemory_Buffer::getMaximumBlobSize_to_MemoryLeft() const
{
    if (m_unusedSizeToPos.size() == 0)
    {
        return 0;
    }

    return m_unusedSizeToPos.rbegin()->first;
}

void* ReusableMemory_Buffer::get(__int64 size)
{
    {
        // 사용을 하지 않는 동일한 크기의 메모리가 존재한다
        // 이것을 우선적으로 사용한다

        auto itUnused = m_unusedSizeToPos.find(size);

        if (itUnused != m_unusedSizeToPos.end())
        {
            auto unused_pos = itUnused->second.begin()->first;

            m_usedPosToSize[unused_pos] = size;

            __remove_unusedPos(unused_pos);

            return m_buffer + unused_pos;
        }
    }

    auto itUnused = m_unusedSizeToPos.rbegin();

    if (itUnused != m_unusedSizeToPos.rend() && itUnused->first > size)
    {
        // 사용을 하지 않는 동일한 크기의 메모리는 존재하지 않으나
        // 사용 가능한 더 큰 메모리가 존재한다
        // 이것을 사용한다.

        auto unused_size = itUnused->first;
        auto unused_pos = itUnused->second.begin()->first;

        m_usedPosToSize[unused_pos] = size;

        __remove_unusedPos(unused_pos);

        // 남는 메모리는 버퍼에 넣어두자
        m_unusedSPosToSize[unused_pos + size] = unused_size - size;
        m_unusedEPosToSize[unused_pos + unused_size] = unused_size - size;
        m_unusedSizeToPos[unused_size - size][unused_pos + size] = TRUE;

        return m_buffer + unused_pos;
    }

    // 사용 가능한 메모리 없음
    return nullptr;
}

void ReusableMemory_Buffer::release(void* buffer)
{
    __int64 removePos = ((BYTE*)buffer) - m_buffer;

    auto itUsedPos = m_usedPosToSize.find(removePos);
    if (itUsedPos == m_usedPosToSize.end())
        return;

    __int64 removeSize = itUsedPos->second;

    // Unuse Position 계산
    auto unused_start = removePos;
    auto unused_end = removePos + removeSize;

    // Used memory 목록에서 제거한다
    m_usedPosToSize.erase(itUsedPos);

    // 뒤에 Unused Memory가 있는가? 모두 지우며 크기를 확장한다
    while (1)
    {
        auto continusUnuseMemory = m_unusedSPosToSize.find(unused_end);

        if (continusUnuseMemory == m_unusedSPosToSize.end())
        {
            break;
        }

        auto incrementSize = continusUnuseMemory->second;
        __remove_unusedPos(unused_end);

        unused_end += incrementSize;
    }

    // 앞에 Unused Memory가 있는가? 모두 지우며 크기를 확장한다
    while (1)
    {
        auto continusUnuseMemory = m_unusedEPosToSize.find(unused_start);

        if (continusUnuseMemory == m_unusedEPosToSize.end())
        {
            break;
        }

        auto incrementSize = continusUnuseMemory->second;
        __remove_unusedPos(unused_start - incrementSize);

        unused_start -= incrementSize;
    }

    // Unused Memory로 집어 넣는다
    auto unused_size = unused_end - unused_start;
    m_unusedSizeToPos[unused_size][unused_start] = TRUE;
    m_unusedSPosToSize[unused_start] = unused_size;
    m_unusedEPosToSize[unused_end] = unused_size;
}

void ReusableMemory_Buffer::__remove_unusedPos(__int64 pos)
{
    auto unused_size = m_unusedSPosToSize[pos];

    m_unusedSPosToSize.erase(pos);
    m_unusedEPosToSize.erase(pos + unused_size);

    m_unusedSizeToPos[unused_size].erase(pos);

    if (m_unusedSizeToPos[unused_size].size() == 0)
    {
        m_unusedSizeToPos.erase(unused_size);
    }
}

//====================================================================================================================

ReusableMemory_Image::ReusableMemory_Image()
    : m_alloc_block_size(0)
    , m_sizeX(0)
    , m_sizeY(0)
{
}

ReusableMemory_Image::~ReusableMemory_Image()
{
    Reset();
}

void ReusableMemory_Image::Ready(size_t alloc_block_size, long imageSizeX, long imageSizeY, long minimumBlockCount)
{
    size_t totalAllocSize = 0;
    for (auto& size_buffer : m_buffers)
    {
        for (auto& buffer : size_buffer.second)
        {
            totalAllocSize += buffer.first->getAllocSize();
        }
    }

    if (m_sizeX == imageSizeX && m_sizeY == imageSizeY && m_alloc_block_size == alloc_block_size
        && totalAllocSize >= alloc_block_size * minimumBlockCount)
    {
        // 이미지 크기가 같고, 현재 할당된 버퍼가 할당하고자 하는 메모리보다 크거나 같으면 다시 할당하지 않는다
        return;
    }

    Reset();

    m_alloc_block_size = alloc_block_size;
    m_sizeX = imageSizeX;
    m_sizeY = imageSizeY;

    if (minimumBlockCount > 0)
    {
        size_t firstBlockSize = m_alloc_block_size * minimumBlockCount;
        auto& buf = m_buffers[firstBlockSize];

        buf[new ReusableMemory_Buffer(firstBlockSize)] = TRUE;
    }
}

void ReusableMemory_Image::Reset()
{
    for (auto& buff : m_buffers)
    {
        for (auto& memory : buff.second)
        {
            delete memory.first;
        }
    }

    m_alloc_table.clear();
    m_buffers.clear();
}

void* ReusableMemory_Image::alloc_byte_image_size_buffer()
{
    return alloc(m_sizeX * m_sizeY);
}

void ReusableMemory_Image::free(void* mem)
{
    auto itTable = m_alloc_table.find(mem);
    if (itTable == m_alloc_table.end())
    {
        return;
    }

    auto* memory = itTable->second;
    m_alloc_table.erase(itTable);

    auto oldSize = memory->getMaximumBlobSize_to_MemoryLeft();
    m_buffers[oldSize].erase(memory);
    if (m_buffers[oldSize].size() == 0)
    {
        m_buffers.erase(oldSize);
    }

    memory->release(mem);
    m_buffers[memory->getMaximumBlobSize_to_MemoryLeft()][memory] = TRUE;
}

bool ReusableMemory_Image::get(Ipvm::Image& object)
{
    if (object.Create(m_sizeX, m_sizeY, this, alloc, free) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool ReusableMemory_Image::get(Ipvm::Image& object, long sizeX, long sizeY)
{
    if (object.Create(sizeX, sizeY, this, alloc, free) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

void* ReusableMemory_Image::alloc(void* userData, size_t bytes)
{
    auto* class_object = (ReusableMemory_Image*)(userData);
    return class_object->alloc(bytes);
}

void ReusableMemory_Image::free(void* userData, void* mem)
{
    auto* class_object = (ReusableMemory_Image*)(userData);
    return class_object->free(mem);
}

void* ReusableMemory_Image::alloc(size_t bytes)
{
    if (m_alloc_block_size < bytes)
    {
        ASSERT(!_T("할당 할 수 없는 크기 요청함"));
        return nullptr;
    }

    if (m_buffers.size() == 0)
    {
        auto& buf = m_buffers[m_alloc_block_size];
        buf[new ReusableMemory_Buffer(m_alloc_block_size)] = TRUE;
    }

    auto itBuf = m_buffers.rbegin();
    if (itBuf->first < bytes)
    {
        auto& buf = m_buffers[m_alloc_block_size];
        buf[new ReusableMemory_Buffer(m_alloc_block_size)] = TRUE;
        itBuf = m_buffers.rbegin();
    }

    auto* memory = itBuf->second.begin()->first;
    auto* retValue = memory->get(bytes);

    // 할당후 남은 메모리에 맞는 위치로 이동
    itBuf->second.erase(memory);
    if (itBuf->second.size() == 0)
    {
        m_buffers.erase(itBuf->first);
    }
    m_alloc_table[retValue] = memory;
    m_buffers[memory->getMaximumBlobSize_to_MemoryLeft()][memory] = TRUE;

    return retValue;
}