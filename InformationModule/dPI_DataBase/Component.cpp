//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Component.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "Enum.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <minwindef.h>
#include <regex>
#include <string>
#include <winnt.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
Component::Component(LPCTSTR i_strCompName, float i_fOffsetX, float i_fOffsetY, LPCTSTR i_strCompCategory,
    LPCTSTR i_strCompType, float i_fWidth, float i_fLength, float i_fHeight, long i_nAngle, float i_fPassiveElectLenght,
    float i_fPassiveElectHeight, float i_fPassivePadGap, float i_fPassivePadWidth, float i_fPassivePadLenght,
    BOOL i_bIgnore)
    : strCompName(i_strCompName)
    , fOffsetX(i_fOffsetX)
    , fOffsetY(i_fOffsetY)
    , strCompCategory(i_strCompCategory)
    , strCompType(i_strCompType)
    , fWidth(i_fWidth)
    , fLength(i_fLength)
    , fHeight(i_fHeight)
    , nAngle(i_nAngle)
    , fPassiveElectLenght(i_fPassiveElectLenght)
    , fPassiveElectHeight(i_fPassiveElectHeight)
    , fPassivePadGap(i_fPassivePadGap)
    , fPassivePadWidth(i_fPassivePadWidth)
    , fPassivePadLenght(i_fPassivePadLenght)
    , bIgnore(i_bIgnore)
{
}

long Component::GetCompType()
{
    //Component에 입력된 Str문자를 비교하여 판단한다.
    long type = -100; //못찾는다면 음수값으로 고정하자

    CString strFindCompType;
    strFindCompType = strCompCategory;
    strFindCompType = strFindCompType.MakeUpper();

    std::regex pattern("(^[a-zA-Z]{1,5})[0-9_]*[0-9a-zA-Z_]*[0-9.]*");
    //std::string compType = CT2A(strFindCompType);
    CT2A asciiStr(strFindCompType);
    std::string compType(asciiStr);

    CString strNewCompType = _T("");
    std::smatch m;
    if (std::regex_match(compType, m, pattern))
    {
        std::string strMatch = m[1].str();

        if (strMatch == "DIE" || strMatch == "D")
            type = _typeChip;
        else if (strMatch == "C" || strMatch == "CAP" || strMatch == "CAPACITOR" || strMatch == "R" || strMatch == "RES"
            || strMatch == "RESITOR" || strMatch == "M" || strMatch == "MIA")
            type = _typePassive;
        else if (strMatch == "A" || strMatch == "ARRAY")
            type = _typeArray;
        else if (strMatch == "H" || strMatch == "HEATSYNC" || strMatch == "LID")
            type = _typeHeatSync;
        else if (strMatch == "P" || strMatch == "Patch" || strMatch == "PATCH") //kircheis_POI
            type = _typePatch;
    }

    return type;
}
} // namespace Package
