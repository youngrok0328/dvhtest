#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum enumInstruction
{
    INSTRUCTION_COMMAND,
    INSTRUCTION_CHANNEL0_INTENSITY, //	0 ~ 254
    INSTRUCTION_CHANNEL1_INTENSITY, //	0 ~ 254
    INSTRUCTION_CHANNEL2_INTENSITY, //	0 ~ 254
    INSTRUCTION_CHANNEL3_INTENSITY, //	0 ~ 254
    INSTRUCTION_TYPE_END,
};

class DPI_SYNCCONTROLLERDEF_API SlitIlluminator
{
public:
    SlitIlluminator(void);
    ~SlitIlluminator(void);

    static SlitIlluminator& GetInstance();

public:
    BOOL Download(BYTE byAddress, BYTE byData);

private:
    BOOL ConnectController();
    BOOL OpenPort(LPCTSTR szPortName, UINT baud, UINT parity, UINT databits, UINT stopbits);

    BYTE UpperPacket(BYTE byPacket);
    BYTE LowerPacket(BYTE byPacket);

    BOOL SafeSendString(BYTE* pbySend, int nSendNum, std::vector<BYTE>& vecChecker, DWORD dwTimeOut);
    BOOL WriteToPortByteStream(BYTE* pbyWrite, int nCharNum, DWORD dwTimeOut);
    BOOL ReadFromPortOneByte(BYTE* pbyRead, DWORD dwTimeOut);
    BOOL ReadFromPortByteStream(std::vector<BYTE>& vecReceive, DWORD dwTimeOut);

private:
    HANDLE m_hDevice;
};
