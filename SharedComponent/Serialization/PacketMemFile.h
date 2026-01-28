#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afx.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class PacketMemFile : public CMemFile
{
    DECLARE_DYNAMIC(PacketMemFile)

public:
    explicit PacketMemFile(UINT growBytes = 1024);
    PacketMemFile(BYTE* lpBuffer, UINT bufferSize, UINT growBytes = 0);
    ~PacketMemFile() override;

    SIZE_T GetBufferSize() const;
};
