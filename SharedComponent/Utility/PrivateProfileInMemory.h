#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#ifdef IN_MEMORY_INI_STD_STRING_SUPPORT
#include <string>
#endif

//HDR_5_________________________________ Forward declarations
namespace Ipsc
{
class PrivateProfileInMemoryImplementation;
}

//HDR_6_________________________________ Header body
//
namespace Ipsc
{
class __INTEKPLUS_SHARED_UTILITY_API__ PrivateProfileInMemory
{
public:
    PrivateProfileInMemory(LPCTSTR pathName = nullptr);
    PrivateProfileInMemory(const PrivateProfileInMemory& rhs) = delete;
    PrivateProfileInMemory(PrivateProfileInMemory&& rhs) = delete;
    ~PrivateProfileInMemory();

    PrivateProfileInMemory& operator=(const PrivateProfileInMemory& rhs) = delete;
    PrivateProfileInMemory& operator=(PrivateProfileInMemory&& rhs) = delete;

    // 현재 가지고 있는 내용을 모두 제거한다.
    void Clear();

    // 현재 가지고 있는 내용에, 파일의 내용을 추가한다.
    // 파일을 여는데 실패하면 false 리턴
    bool Append(LPCTSTR pathName);

    // 현재 가지고 있는 내용을 파일에 저장한다.
    // 파일을 여는데 실패하면 false 리턴
    // 내용을 파일에 모두 저장하지 못하면 false 리턴
    bool Save(LPCTSTR pathName);

    void Get(const CStringA& section, const CStringA& key, CStringA& value);
    void Get(const CStringW& section, const CStringW& key, CStringW& value);

#ifdef IN_MEMORY_INI_STD_STRING_SUPPORT
    void Get(const std::string& section, const std::string& key, std::string& value);
    void Get(const std::wstring& section, const std::wstring& key, std::wstring& value);
#endif

    void Set(const CStringA& section, const CStringA& key, const CStringA& value);
    void Set(const CStringW& section, const CStringW& key, const CStringW& value);

#ifdef IN_MEMORY_INI_STD_STRING_SUPPORT
    void Set(const std::string& section, const std::string& key, const std::string& value);
    void Set(const std::wstring& section, const std::wstring& key, const std::wstring& value);
#endif

private:
    PrivateProfileInMemoryImplementation* m_impl;
};
} // namespace Ipsc
