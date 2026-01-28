//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "HostCommand.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CHostCommand::CHostCommand()
    : m_nCommandID(-1)
    , m_nBufferSize(0)
    , m_pBuffer(NULL) {};

CHostCommand::CHostCommand(long nCommandID, BYTE* pBuffer, long nBufferSize)
    : m_nCommandID(nCommandID)
    , m_nBufferSize(nBufferSize)
    , m_pBuffer(new BYTE[nBufferSize])
{
    memcpy(m_pBuffer, pBuffer, nBufferSize);
}

CHostCommand::CHostCommand(const CHostCommand& Src)
    : m_nCommandID(Src.m_nCommandID)
    , m_nBufferSize(Src.m_nBufferSize)
    , m_pBuffer(new BYTE[m_nBufferSize])
{
    memcpy(m_pBuffer, Src.m_pBuffer, m_nBufferSize);
}

CHostCommand::~CHostCommand()
{
    delete[] m_pBuffer;
}

CHostCommand& CHostCommand::operator=(const CHostCommand& Src)
{
    m_nCommandID = Src.m_nCommandID;

    delete[] m_pBuffer;

    m_nBufferSize = Src.m_nBufferSize;
    m_pBuffer = new BYTE[m_nBufferSize];

    memcpy(m_pBuffer, Src.m_pBuffer, m_nBufferSize);

    return *this;
}
