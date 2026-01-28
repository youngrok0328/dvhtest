//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Serializer.h"

//CPP_2_________________________________ This project's headers
#include "PacketMemFile.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Ipsc
{
Serializer::Serializer(UINT bufferGrowSize)
    : m_archiveBufferSize(bufferGrowSize)
    , m_archiveSize(0)
    , m_archiveBuffer((BYTE*)::malloc(bufferGrowSize)) // MFC 클래스 내부에서 malloc/free 를 사용하므로, 이를 따른다.
    , m_memFile(nullptr)
    , m_archiver(nullptr)
{
}

Serializer::~Serializer()
{
    // MFC 클래스 내부에서 malloc/free 를 사용하므로, 이를 따른다.
    ::free(m_archiveBuffer);
}

CArchive& Serializer::Begin()
{
    ASSERT(m_memFile == nullptr);
    ASSERT(m_archiver == nullptr);

    delete m_archiver;
    delete m_memFile;

    m_memFile = new PacketMemFile(m_archiveBuffer, m_archiveBufferSize, m_archiveBufferSize);
    m_archiver = new CArchive(m_memFile, CArchive::store);
    m_archiveSize = 0;

    return *m_archiver;
}

void Serializer::End()
{
    if (m_archiver == nullptr || m_memFile == nullptr)
    {
        return;
    }

    m_archiver->Close();

    m_archiveSize = static_cast<UINT>(m_memFile->GetLength());
    m_archiveBufferSize = static_cast<UINT>(m_memFile->GetBufferSize());
    m_archiveBuffer = m_memFile->Detach();

    delete m_memFile;
    delete m_archiver;

    m_memFile = nullptr;
    m_archiver = nullptr;
}

UINT Serializer::GetSize()
{
    End();

    return m_archiveSize;
}

BYTE* Serializer::GetBuffer()
{
    End();

    return m_archiveBuffer;
}
} // namespace Ipsc
