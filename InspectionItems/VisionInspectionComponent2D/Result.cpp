//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Result.h"

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
Result::Result()
{
}

Result::~Result()
{
}

void Result::clear()
{
    m_passiveItems.clear();
}

ResultPassiveItem* Result::findPassiveItem(long chipIndex)
{
    for (auto& item : m_passiveItems)
    {
        if (item.m_sPassive_Index == chipIndex)
        {
            return &item;
        }
    }

    return nullptr;
}
