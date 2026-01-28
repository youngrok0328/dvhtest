//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SlitIlluminator.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SlitIlluminator::SlitIlluminator(void)
    : m_hDevice(INVALID_HANDLE_VALUE)
{
    ConnectController();
}

BOOL SlitIlluminator::ConnectController()
{
    auto& systemConfig = SystemConfig::GetInstance();

    if (systemConfig.IsHardwareExist() && systemConfig.GetVisionType() == VISIONTYPE_3D_INSP)
    {
        long nBaudRate = 57600;
        long nParity = 0; //0~4 ==> no ,odd, even, mark, space
        long nDataBit = 8;
        long nStopBit = 0; //0~2 ==> 1, 1.5, 2

        CString strComNum;
        strComNum.Format(_T("COM%d"), systemConfig.m_nSyncPort);

        if (OpenPort(strComNum, nBaudRate, nParity, nDataBit, nStopBit) == FALSE)
        {
            CString str;
            str.Format(_T("Cannot open COM%d port for Slitbeam light source"), SystemConfig::GetInstance().m_nSyncPort);

            ::AfxMessageBox(str, MB_ICONERROR | MB_OK);
            ::exit(0);

            return FALSE;
        }
    }

    return TRUE;
}

SlitIlluminator::~SlitIlluminator(void)
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        return;
    }

    CloseHandle(m_hDevice);
    m_hDevice = INVALID_HANDLE_VALUE;
}

SlitIlluminator& SlitIlluminator::GetInstance()
{
    static SlitIlluminator singleton;

    return singleton;
}

BOOL SlitIlluminator::OpenPort(LPCTSTR szPortName, UINT baud, UINT parity, UINT databits, UINT stopbits)
{
    CString str;
    DCB dcb;
    DWORD inSize, outSize, dwError;
    inSize = 4096;
    outSize = 4096;

    m_hDevice
        = ::CreateFile(szPortName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);

    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        dwError = GetLastError();
        return FALSE;
    }

    SetCommMask(m_hDevice, EV_CTS | EV_RXCHAR | EV_TXEMPTY | EV_DSR);
    SetupComm(m_hDevice, inSize, outSize);

    GetCommState(m_hDevice, &dcb);

    dcb.BaudRate = baud;
    dcb.ByteSize = (BYTE)databits;

    switch (parity)
    {
        case 0:
            dcb.Parity = NOPARITY;
            break;
        case 1:
            dcb.Parity = ODDPARITY;
            break;
        case 2:
            dcb.Parity = EVENPARITY;
            break;
    }

    dcb.StopBits = (unsigned char)stopbits;

    if (SetCommState(m_hDevice, &dcb) != TRUE)
        return FALSE;

    return TRUE;
}

BOOL SlitIlluminator::Download(BYTE byAddress, BYTE byData)
{
    if (m_hDevice == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    if (byAddress == INSTRUCTION_CHANNEL0_INTENSITY) //kircheis_3DLensIssue
    {
        float fInten = (float)byData * SystemConfig::GetInstance().m_f3DIllumGain;
        if (fInten > 0.f && fInten < 1.f)
            fInten = 1;
        else if (fInten > 255.f)
            fInten = 255.f;
        else if (fInten < 0.f)
            fInten = 0.f;
        byData = (BYTE)(fInten + .5f);
    }

    static BYTE pChecker[] = {'$', 'G', 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A, 0x00};
    pChecker[2] = UpperPacket(byAddress);
    pChecker[3] = LowerPacket(byAddress);
    pChecker[4] = UpperPacket(byData);
    pChecker[5] = LowerPacket(byData);
    std::vector<BYTE> vecChecker(pChecker, pChecker + 8);

    static BYTE pSendBuf[] = {'$', 'G', 0x00, 0x00, 0x00, 0x00, 0x0D, 0x0A};
    pSendBuf[2] = UpperPacket(byAddress);
    pSendBuf[3] = LowerPacket(byAddress);
    pSendBuf[4] = UpperPacket(byData);
    pSendBuf[5] = LowerPacket(byData);

    BOOL ret = FALSE;

    const long nRetryCount = 2;
    long nRetry = 0;

    for (; nRetry < nRetryCount; nRetry++)
    {
        ret = SafeSendString(pSendBuf, 8, vecChecker, 1000);
        if (ret)
        {
            break;
        }
    }

    if (nRetry == nRetryCount)
    {
        if (!ConnectController())
        {
            AfxMessageBox(_T("Failed to connected with slitbeam illuminator!"), MB_OK | MB_TOPMOST);

            return ret;
        }

        nRetry = 0;

        for (; nRetry < nRetryCount; nRetry++)
        {
            ret = SafeSendString(pSendBuf, 8, vecChecker, 1000);
            if (ret)
            {
                break;
            }
        }

        if (!ret)
        {
            AfxMessageBox(_T("Failed to communicate with slitbeam illuminator!"), MB_OK | MB_TOPMOST);
            //		::ToConsole("[SYNC][ERROR] ", "Response Check Failed!");
        }
    }

    return ret;
}

BYTE SlitIlluminator::UpperPacket(BYTE byPacket)
{
    BYTE byUpperPacket = byPacket >> 4;

    if (byUpperPacket <= 0x09)
        byUpperPacket = (byUpperPacket - 0x00) + '0';
    else
        byUpperPacket = (byUpperPacket - 0x0a) + 'A';

    return byUpperPacket;
}

BYTE SlitIlluminator::LowerPacket(BYTE byPacket)
{
    BYTE byLowerPacket = byPacket & 0x0f;

    if (byLowerPacket <= 0x09)
        byLowerPacket = (byLowerPacket - 0x00) + '0';
    else
        byLowerPacket = (byLowerPacket - 0x0a) + 'A';

    return byLowerPacket;
}

BOOL SlitIlluminator::SafeSendString(BYTE* pbySend, int nSendNum, std::vector<BYTE>& vecChecker, DWORD dwTimeOut)
{
    //	if(!OpenPort())
    //	{
    //		return FALSE;
    //	}

    BOOL bRet = FALSE;

    //	::ToConsoleHexAndChars("[SYNC][COMMAND] ", (void *)pbySend, nSendNum);

    if (WriteToPortByteStream(pbySend, nSendNum, dwTimeOut))
    {
        //		::ToConsole("[SYNC][DESC] ", "Write to port - OK!");
    }
    else
    {
        //		::ToConsole("[SYNC][ERROR] ", "Write to port - Failed!");
        bRet = FALSE;
    }

    std::vector<BYTE> vecReceive;

    if (ReadFromPortByteStream(vecReceive, dwTimeOut))
    {
        //		::ToConsoleHexAndChars("[SYNC][RESPONSE] ", (void *)&vecReceive[0],  (long)(vecReceive.size()));
        //		::ToConsole("[SYNC][DESC] ", "Read from port - OK!");

        static BYTE pSyncError[] = {'E', 'R', 'R', 'O', 'R', 0x00};
        static std::vector<BYTE> vecSyncError(pSyncError, pSyncError + 5);

        if (vecReceive == vecChecker)
        {
            //			::ToConsole("[SYNC][DESC] ", "Response Check Success!");
            bRet = TRUE;
        }
        else if (vecReceive.end()
            != std::search(vecReceive.begin(), vecReceive.end(), vecSyncError.begin(), vecSyncError.end()))
        {
            //			::ToConsole("[SYNC][ERROR] ", "Sync Error Returned!");
        }
        else
        {
            //			::ToConsole("[SYNC][ERROR] ", "Unknown Response!");
        }
    }
    else
    {
        bRet = FALSE;
        //		::ToConsoleHexAndChars("[SYNC][RESPONSE] ", (void *)&vecReceive[0],  (long)(vecReceive.size()));
        //		::ToConsole("[SYNC][ERROR] ", "Read from port - Failed!");
    }

    //	ClosePort();

    return bRet;
}

BOOL SlitIlluminator::WriteToPortByteStream(BYTE* pbyWrite, int nCharNum, DWORD dwTimeOut)
{
    OVERLAPPED osWrite = {0};
    DWORD dwWritten;
    DWORD dwRes;
    BOOL fRes;

    osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (osWrite.hEvent == NULL)
        return FALSE;

    if (!WriteFile(m_hDevice, pbyWrite, nCharNum, &dwWritten, &osWrite))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            fRes = FALSE;
        }
        else
        {
            dwRes = WaitForSingleObject(osWrite.hEvent, dwTimeOut);
            switch (dwRes)
            {
                case WAIT_OBJECT_0:
                    if (GetOverlappedResult(m_hDevice, &osWrite, &dwWritten, FALSE))
                    {
                        fRes = TRUE;
                    }
                    else
                    {
                        fRes = FALSE;
                    }
                    break;
                case WAIT_TIMEOUT:
                    //			::ToConsole("[SYNC][ERROR] ", "Timeout while writing!");
                default:
                    fRes = FALSE;
                    break;
            }
        }
    }
    else
    {
        fRes = TRUE;
    }

    CloseHandle(osWrite.hEvent);

    return fRes;
}

BOOL SlitIlluminator::ReadFromPortOneByte(BYTE* pbyRead, DWORD dwTimeOut)
{
    OVERLAPPED osRead = {0};
    DWORD dwRead;
    DWORD dwRes;
    BOOL bRes;

    osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    if (osRead.hEvent == NULL)
        return FALSE;

    if (!ReadFile(m_hDevice, pbyRead, 1, &dwRead, &osRead))
    {
        if (GetLastError() != ERROR_IO_PENDING)
        {
            bRes = FALSE;
        }
        else
        {
            dwRes = WaitForSingleObject(osRead.hEvent, dwTimeOut);
            switch (dwRes)
            {
                case WAIT_OBJECT_0:
                    if (GetOverlappedResult(m_hDevice, &osRead, &dwRead, FALSE))
                    {
                        bRes = TRUE;
                    }
                    else
                    {
                        bRes = FALSE;
                    }
                    break;
                case WAIT_TIMEOUT:
                    //				::ToConsole("[SYNC][ERROR] ", "Timeout while reading one byte.");
                default:
                    bRes = FALSE;
                    break;
            }
        }
    }
    else
    {
        bRes = TRUE;
    }

    CloseHandle(osRead.hEvent);

    return bRes;
}

BOOL SlitIlluminator::ReadFromPortByteStream(std::vector<BYTE>& vecReceive, DWORD dwTimeOut)
{
    while (1)
    {
        BYTE byReceive = 0;

        if (!ReadFromPortOneByte(&byReceive, dwTimeOut))
        {
            if ((long)(vecReceive.size()) > 2)
            {
                return FALSE;
            }
        }

        vecReceive.push_back(byReceive);

        if ((long)(vecReceive.size()) > 2)
        {
            std::vector<BYTE>::reverse_iterator itr = vecReceive.rbegin();

            if (vecReceive[(long)(vecReceive.size()) - 1] == BYTE(0x0a)
                && vecReceive[(long)(vecReceive.size()) - 2] == BYTE(0x0d))
            {
                return TRUE;
            }
        }

        //if(TimeChecker.EndTime() > dwTimeOut)
        //{
        //	return FALSE;
        //}
    }

    return FALSE;
}
