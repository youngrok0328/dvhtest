//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PacketMemFile.h"

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
IMPLEMENT_DYNAMIC(PacketMemFile, CMemFile);

PacketMemFile::PacketMemFile(UINT growBytes)
    : CMemFile(growBytes)
{
}

PacketMemFile::PacketMemFile(BYTE* lpBuffer, UINT bufferSize, UINT growBytes)
    : CMemFile(lpBuffer, bufferSize, growBytes)
{
}

PacketMemFile::~PacketMemFile()
{
}

SIZE_T PacketMemFile::GetBufferSize() const
{
    return m_nBufferSize;
}
