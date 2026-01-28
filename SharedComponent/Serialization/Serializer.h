#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CArchive;
class PacketMemFile;
typedef unsigned char BYTE;

//HDR_6_________________________________ Header body
//
namespace Ipsc
{
class __INTEKPLUS_SHARED_SERIALIZATION_API__ Serializer
{
public:
    Serializer() = delete;
    Serializer(const Serializer& rhs) = delete;
    Serializer(UINT bufferGrowSize);
    ~Serializer();

    Serializer& operator=(const Serializer& rhs) = delete;

public:
    // 시리얼라이즈를 시작할 때 호출.
    CArchive& Begin();

    // 아래 두 함수 중 하나라도 호출하면, 시리얼라이즈를 끝내도록 되어 있음.
    // 따라서 End() 함수를 별도로 호출할 수 없도록 구현함.
    UINT GetSize();
    BYTE* GetBuffer();

private:
    void End();

private:
    UINT m_archiveBufferSize;
    UINT m_archiveSize;
    BYTE* m_archiveBuffer;

    PacketMemFile* m_memFile;
    CArchive* m_archiver;
};
} // namespace Ipsc
