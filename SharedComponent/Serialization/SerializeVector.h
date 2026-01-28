#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afx.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
typedef int int32_t;

//HDR_6_________________________________ Header body
//
template<typename ValueType>
CArchive& operator<<(CArchive& ar, const std::vector<ValueType>& collection)
{
    ar << static_cast<int32_t>(collection.size());

    for (const auto& element : collection)
    {
        ar << element;
    }

    return ar;
}

template<typename ValueType>
CArchive& operator>>(CArchive& ar, std::vector<ValueType>& collection)
{
    int32_t size{};

    ar >> size;

    collection.resize(size);

    for (auto& element : collection)
    {
        ar >> element;
    }

    return ar;
}
