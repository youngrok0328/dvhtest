#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CArchive;
class CMemFile;
typedef unsigned int UINT;

//HDR_6_________________________________ Header body
//
namespace Ipsc
{
class __INTEKPLUS_SHARED_SERIALIZATION_API__ Deserializer
{
public:
    Deserializer() = delete;
    Deserializer(const Deserializer& rhs) = delete;
    Deserializer(const void* buffer, UINT bufferSize);
    ~Deserializer();

    Deserializer& operator=(const Deserializer& rhs) = delete;

public:
    // 디시리얼라이즈를 시작할 때 호출.
    CArchive& Begin();

private:
    CMemFile* m_memFile;
    CArchive* m_archiver;
};
} // namespace Ipsc
