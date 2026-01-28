#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <string>
#include <unordered_map>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Ipsc
{
struct TransparentHash
{
    using is_transparent = void;

    std::size_t operator()(std::string_view sv) const noexcept
    {
        return std::hash<std::string_view>{}(sv);
    }

    std::size_t operator()(const std::string& s) const noexcept
    {
        return std::hash<std::string>{}(s);
    }
};

struct TransparentEqual
{
    using is_transparent = void;

    bool operator()(std::string_view lhs, std::string_view rhs) const noexcept
    {
        return lhs == rhs;
    }

    bool operator()(const std::string& lhs, std::string_view rhs) const noexcept
    {
        return lhs == rhs;
    }

    bool operator()(std::string_view lhs, const std::string& rhs) const noexcept
    {
        return lhs == rhs;
    }

    bool operator()(const std::string& lhs, const std::string& rhs) const noexcept
    {
        return lhs == rhs;
    }
};

class PrivateProfileInMemoryImplementation
{
public:
    PrivateProfileInMemoryImplementation(LPCTSTR pathName = nullptr);
    ~PrivateProfileInMemoryImplementation();

    void Clear();
    bool Append(LPCTSTR pathName);
    bool Save(LPCTSTR pathName);

    void Get(const CStringA& section, const CStringA& key, CStringA& value);
    void Get(const CStringW& section, const CStringW& key, CStringW& value);
    void Get(const std::string& section, const std::string& key, std::string& value);
    void Get(const std::wstring& section, const std::wstring& key, std::wstring& value);

    void Set(const CStringA& section, const CStringA& key, const CStringA& value);
    void Set(const CStringW& section, const CStringW& key, const CStringW& value);
    void Set(const std::string& section, const std::string& key, const std::string& value);
    void Set(const std::wstring& section, const std::wstring& key, const std::wstring& value);

private:
    std::string* FindValueHolder(const std::string_view& section, const std::string_view& key);

    CString m_pathName;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string, TransparentHash, TransparentEqual>,
        TransparentHash, TransparentEqual>
        m_sections;
};
} // namespace Ipsc
