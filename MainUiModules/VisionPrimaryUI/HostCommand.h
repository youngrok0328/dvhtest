#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CHostCommand
{
public:
    CHostCommand();
    CHostCommand(long nCommandID, BYTE* pBuffer, long nBufferSize);
    CHostCommand(const CHostCommand& Src);
    ~CHostCommand();

    CHostCommand& operator=(const CHostCommand& Src);

    long m_nCommandID;
    long m_nBufferSize;
    BYTE* m_pBuffer;
};
