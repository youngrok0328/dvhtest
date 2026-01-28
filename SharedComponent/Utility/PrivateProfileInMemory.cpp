//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PrivateProfileInMemory.h"

//CPP_2_________________________________ This project's headers
#include "PrivateProfileInMemoryImplementation.h"

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
PrivateProfileInMemory::PrivateProfileInMemory(LPCTSTR pathName)
    : m_impl(new PrivateProfileInMemoryImplementation(pathName))
{
}

PrivateProfileInMemory::~PrivateProfileInMemory()
{
    delete m_impl;
}

void PrivateProfileInMemory::Clear()
{
    return m_impl->Clear();
}

bool PrivateProfileInMemory::Append(LPCTSTR pathName)
{
    return m_impl->Append(pathName);
}

bool PrivateProfileInMemory::Save(LPCTSTR pathName)
{
    return m_impl->Save(pathName);
}

void PrivateProfileInMemory::Get(const CStringA& section, const CStringA& key, CStringA& value)
{
    m_impl->Get(section, key, value);
}

void PrivateProfileInMemory::Get(const CStringW& section, const CStringW& key, CStringW& value)
{
    m_impl->Get(section, key, value);
}

#ifdef IN_MEMORY_INI_STD_STRING_SUPPORT
void PrivateProfileInMemory::Get(const std::string& section, const std::string& key, std::string& value)
{
    m_impl->Get(section, key, value);
}

void PrivateProfileInMemory::Get(const std::wstring& section, const std::wstring& key, std::wstring& value)
{
    m_impl->Get(section, key, value);
}
#endif

void PrivateProfileInMemory::Set(const CStringA& section, const CStringA& key, const CStringA& value)
{
    m_impl->Set(section, key, value);
}

void PrivateProfileInMemory::Set(const CStringW& section, const CStringW& key, const CStringW& value)
{
    m_impl->Set(section, key, value);
}

#ifdef IN_MEMORY_INI_STD_STRING_SUPPORT
void PrivateProfileInMemory::Set(const std::string& section, const std::string& key, const std::string& value)
{
    m_impl->Set(section, key, value);
}

void PrivateProfileInMemory::Set(const std::wstring& section, const std::wstring& key, const std::wstring& value)
{
    m_impl->Set(section, key, value);
}
#endif
} // namespace Ipsc
