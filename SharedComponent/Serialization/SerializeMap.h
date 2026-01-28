#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afx.h>

//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
typedef int int32_t;

//HDR_6_________________________________ Header body
//
template<typename KeyType, typename ValueType>
CArchive& operator<<(CArchive& ar, const std::map<KeyType, ValueType>& collection)
{
    ar << static_cast<int32_t>(collection.size());

    for (const auto& element : collection)
    {
        ar << element.first;
        ar << element.second;
    }

    return ar;
}

template<typename KeyType, typename ValueType>
CArchive& operator>>(CArchive& ar, std::map<KeyType, ValueType>& collection)
{
    int32_t size{};

    ar >> size;

    collection.clear();

    KeyType key;
    ValueType value;

    for (int32_t i = 0; i < size; i++)
    {
        ar >> key;
        ar >> value;

        collection[key] = value;
    }

    return ar;
}
