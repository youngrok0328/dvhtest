//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Deserializer.h"

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
namespace Ipsc
{
Deserializer::Deserializer(const void* buffer, UINT bufferSize)
    : m_memFile(new CMemFile((BYTE*)buffer, bufferSize))
    , m_archiver(new CArchive(m_memFile, CArchive::load))
{
}

Deserializer::~Deserializer()
{
    delete m_archiver;
    delete m_memFile;
}

CArchive& Deserializer::Begin()
{
    return *m_archiver;
}
} // namespace Ipsc
