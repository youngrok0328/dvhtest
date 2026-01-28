//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PrivateProfileInMemoryImplementation.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <string_view>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Ipsc
{
PrivateProfileInMemoryImplementation::PrivateProfileInMemoryImplementation(LPCTSTR pathName)
{
    if (pathName)
    {
        Append(pathName);
    }
}

PrivateProfileInMemoryImplementation::~PrivateProfileInMemoryImplementation()
{
}

void PrivateProfileInMemoryImplementation::Clear()
{
    m_sections.clear();
}

bool PrivateProfileInMemoryImplementation::Append(LPCTSTR pathName)
{
    //=================================
    // 파일을 열어 텍스트 데이터를 가져온다.
    //=================================
    HANDLE fileHandle
        = ::CreateFile(pathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    LARGE_INTEGER fileSizeQuad{};

    if (::GetFileSizeEx(fileHandle, &fileSizeQuad) == FALSE)
    {
        ::CloseHandle(fileHandle);
        return false;
    }

    if (fileSizeQuad.QuadPart == 0)
    {
        ::CloseHandle(fileHandle);
        return true;
    }

    const size_t fileSize = fileSizeQuad.QuadPart;
    const size_t maxReadableSize = 0x00000000FFFFFFFFU;

    std::vector<char> buffer(fileSize);
    size_t totalReadBytes{};

    while (totalReadBytes < fileSize)
    {
        DWORD bytesToRead = static_cast<DWORD>(min(maxReadableSize, fileSize - totalReadBytes));
        DWORD readBytes{};

        if (::ReadFile(fileHandle, buffer.data() + totalReadBytes, bytesToRead, &readBytes, NULL) == FALSE
            || bytesToRead != readBytes)
        {
            break;
        }

        totalReadBytes += readBytes;
    }

    ::CloseHandle(fileHandle);

    if (fileSize != totalReadBytes)
    {
        return false;
    }

    std::string_view contentsView(buffer.data(), buffer.size());

    //=================================
    // 카테고리를 분류한다.
    //=================================
    size_t pos = 0;

    // 카테고리 시작 찾기
    auto spos = contentsView.find('[', pos);

    std::string sectionString;
    std::string keyString;

    while (spos != std::string::npos)
    {
        const auto epos = contentsView.find(']', pos);

        sectionString = contentsView.substr(spos + 1, epos - spos - 1);

        auto& section = m_sections[sectionString];

        pos = epos + 1;

        // 다음 spos 를 찾는다.
        spos = contentsView.find('[', pos);

        std::string_view sectionView;

        if (spos == std::string::npos)
        {
            sectionView = std::string_view(contentsView.begin() + epos + 3, contentsView.end());
        }
        else
        {
            sectionView = std::string_view(contentsView.begin() + epos + 3, contentsView.begin() + spos);
        }

        size_t keyPos = 0;

        while (1)
        {
            const auto equalPos = sectionView.find('=', keyPos);

            if (equalPos == std::string_view::npos)
            {
                break;
            }

            const auto valuePos = equalPos + sizeof('=');

            const auto carriageReturnPos = sectionView.find('\r', valuePos);

            if (carriageReturnPos != std::string_view::npos)
            {
                if (equalPos > keyPos && valuePos < carriageReturnPos)
                {
                    keyString = sectionView.substr(keyPos, equalPos - keyPos);

                    section[keyString]
                        = std::string(sectionView.begin() + valuePos, sectionView.begin() + carriageReturnPos);
                }
            }
            else
            {
                if (equalPos > keyPos && valuePos < sectionView.length())
                {
                    keyString = sectionView.substr(keyPos, equalPos - keyPos);

                    section[keyString] = std::string(sectionView.begin() + valuePos, sectionView.end());
                }

                break;
            }

            keyPos = carriageReturnPos + sizeof('\r') + sizeof('\n');
        }
    }

    return true;
}

bool PrivateProfileInMemoryImplementation::Save(LPCTSTR pathName)
{
    HANDLE fileHandle = ::CreateFile(pathName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        return false;
    }

    size_t memoryFileSize = 0;

    for (const auto& section : m_sections)
    {
        memoryFileSize += section.first.length();
        memoryFileSize += 4; // "[]\r\n"

        for (const auto& property : section.second)
        {
            memoryFileSize += property.first.length();
            memoryFileSize += property.second.length();
            memoryFileSize += 3; // "=\r\n"
        }
    }

    std::vector<char> memoryFile(memoryFileSize);
    auto* memoryFilePointer = memoryFile.data();

    for (const auto& section : m_sections)
    {
        *memoryFilePointer++ = '[';

        ::memcpy(memoryFilePointer, section.first.data(), section.first.length());
        memoryFilePointer += section.first.length();

        *memoryFilePointer++ = ']';
        *memoryFilePointer++ = '\r';
        *memoryFilePointer++ = '\n';

        for (const auto& property : section.second)
        {
            ::memcpy(memoryFilePointer, property.first.data(), property.first.length());
            memoryFilePointer += property.first.length();

            *memoryFilePointer++ = '=';

            ::memcpy(memoryFilePointer, property.second.data(), property.second.length());
            memoryFilePointer += property.second.length();

            *memoryFilePointer++ = '\r';
            *memoryFilePointer++ = '\n';
        }
    }

    const size_t maxWritableSize = 0x00000000FFFFFFFFU;
    size_t totalWrittenBytes{};

    while (totalWrittenBytes < memoryFileSize)
    {
        DWORD bytesToWrite = static_cast<DWORD>(min(maxWritableSize, memoryFileSize - totalWrittenBytes));
        DWORD writtenBytes{};

        if (::WriteFile(fileHandle, memoryFile.data() + totalWrittenBytes, bytesToWrite, &writtenBytes, NULL) == FALSE
            || bytesToWrite != writtenBytes)
        {
            break;
        }

        totalWrittenBytes += writtenBytes;
    }

    ::CloseHandle(fileHandle);

    if (memoryFileSize != totalWrittenBytes)
    {
        return false;
    }

    return true;
}

static std::string_view ToStdStringView(const CStringA& str)
{
    return std::string_view(str.GetString(), str.GetLength());
}

static CStringA ToMfcStringA(const std::wstring& str)
{
    return CStringA(str.data(), static_cast<int>(str.size()));
}

static std::string ToStdString(const CStringA& str)
{
    return std::string(str.GetString(), str.GetLength());
}

static CStringA ToMfcStringA(const CStringW& str)
{
    return CStringA(str.GetString(), str.GetLength());
}

void PrivateProfileInMemoryImplementation::Get(const CStringA& section, const CStringA& key, CStringA& value)
{
    if (const auto* valueHolder = FindValueHolder(ToStdStringView(section), ToStdStringView(key)); valueHolder)
    {
        value.SetString(valueHolder->c_str(), static_cast<int>(valueHolder->size()));
    }
    else
    {
        value.Empty();
    }
}

void PrivateProfileInMemoryImplementation::Get(const CStringW& section, const CStringW& key, CStringW& value)
{
    if (const auto* valueHolder
        = FindValueHolder(ToStdStringView(ToMfcStringA(section)), ToStdStringView(ToMfcStringA(key)));
        valueHolder)
    {
        value = CStringW(valueHolder->c_str(), static_cast<int>(valueHolder->size()));
    }
    else
    {
        value.Empty();
    }
}

void PrivateProfileInMemoryImplementation::Get(const std::string& section, const std::string& key, std::string& value)
{
    if (const auto* valueHolder = FindValueHolder(section, key); valueHolder)
    {
        value = *valueHolder;
    }
    else
    {
        value.clear();
    }
}

void PrivateProfileInMemoryImplementation::Get(
    const std::wstring& section, const std::wstring& key, std::wstring& value)
{
    if (const auto* valueHolder
        = FindValueHolder(ToStdStringView(ToMfcStringA(section)), ToStdStringView(ToMfcStringA(key)));
        valueHolder)
    {
        value = CStringW(valueHolder->c_str(), static_cast<int>(valueHolder->size()));
    }
    else
    {
        value.clear();
    }
}

void PrivateProfileInMemoryImplementation::Set(const CStringA& section, const CStringA& key, const CStringA& value)
{
    m_sections[ToStdString(section)][ToStdString(key)] = ToStdString(value);
}

void PrivateProfileInMemoryImplementation::Set(const CStringW& section, const CStringW& key, const CStringW& value)
{
    m_sections[ToStdString(ToMfcStringA(section))][ToStdString(ToMfcStringA(key))] = ToStdString(ToMfcStringA(value));
}

void PrivateProfileInMemoryImplementation::Set(
    const std::string& section, const std::string& key, const std::string& value)
{
    m_sections[section][key] = value;
}

void PrivateProfileInMemoryImplementation::Set(
    const std::wstring& section, const std::wstring& key, const std::wstring& value)
{
    m_sections[ToStdString(ToMfcStringA(section))][ToStdString(ToMfcStringA(key))] = ToStdString(ToMfcStringA(value));
}

std::string* PrivateProfileInMemoryImplementation::FindValueHolder(
    const std::string_view& section, const std::string_view& key)
{
    // 카테고리를 검색한다.
    auto itrCategory = m_sections.find(section);

    if (itrCategory == m_sections.end())
    {
        return {};
    }

    // 키값을 찾는다.
    auto& propertyMap = itrCategory->second;

    auto itrProperty = propertyMap.find(key);

    if (itrProperty == propertyMap.end())
    {
        return {};
    }

    return &itrProperty->second;
}
} // namespace Ipsc
