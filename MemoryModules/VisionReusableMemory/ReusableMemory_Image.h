#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ReusableMemory_Buffer
{
public:
    ReusableMemory_Buffer(__int64 size);
    ~ReusableMemory_Buffer();

    __int64 getAllocSize() const;
    __int64 getMaximumBlobSize_to_MemoryLeft() const;
    void* get(__int64 size);
    void release(void* buffer);

private:
    BYTE* m_buffer;
    __int64 m_bufferSize;

    std::map<__int64, __int64> m_usedPosToSize;
    std::map<__int64, __int64> m_unusedSPosToSize;
    std::map<__int64, __int64> m_unusedEPosToSize;
    std::map<__int64, std::map<__int64, BOOL>> m_unusedSizeToPos;

    void __remove_unusedPos(__int64 pos);
};

class ReusableMemory_Image
{
public:
    ReusableMemory_Image();
    ~ReusableMemory_Image();

    void Ready(size_t alloc_block_size, long imageSizeX, long imageSizeY, long minimumBlockCount);
    void Reset();

    void* alloc_byte_image_size_buffer();
    void free(void* mem);
    bool get(Ipvm::Image& object);
    bool get(Ipvm::Image& object, long sizeX, long sizeY);

private:
    long m_sizeX;
    long m_sizeY;
    size_t m_alloc_block_size;

    std::map<void*, ReusableMemory_Buffer*> m_alloc_table;
    std::map<size_t, std::map<ReusableMemory_Buffer*, BOOL>> m_buffers; // 남은 용량별 Buffer

private:
    static void* alloc(void* userData, size_t bytes);
    static void free(void* userData, void* mem);

    void* alloc(size_t bytes);
};
