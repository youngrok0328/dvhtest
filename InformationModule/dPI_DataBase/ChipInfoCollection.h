#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ChipInfo.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <afxstr.h>
#include <minwindef.h>
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Chip
{
class ChipInfoCollection
{
    //private:

public:
    std::vector<ChipInfo> vecChips;

public:
    ChipInfoCollection() = default;

    void Clear()
    {
        vecChips.clear();
    }

    ChipInfo* GetChipInfo(long nIndex)
    {
        long nChipNum = GetChipCount();
        if (nIndex < 0 || nIndex >= nChipNum)
            return NULL;
        return &vecChips[nIndex];
    }

    long GetChipCount()
    {
        return (long)vecChips.size();
    }

    BOOL Delete(long nIndex)
    {
        if (nIndex < 0 || nIndex >= GetChipCount())
            return FALSE;

        vecChips.erase(vecChips.begin() + nIndex);

        return TRUE;
    }

    BOOL Delete(CString strName)
    {
        for (long i = 0; i < GetChipCount(); i++)
        {
            if (strName.Compare(vecChips[i].strCompType) == 0)
                return Delete(i);
        }

        return FALSE;
    }

    void Add(ChipInfo Chip)
    {
        vecChips.push_back(Chip);
    }

    void Add(CString strName)
    {
        ChipInfo Chip;
        Chip.strCompType = strName;

        vecChips.push_back(Chip);
    }
};

} // namespace Chip