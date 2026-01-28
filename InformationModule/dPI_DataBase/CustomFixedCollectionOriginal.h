#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "CustomFixed.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API CustomFixedCollectionOriginal
{
public:
    float m_fUnit{1.f};
    BOOL m_bCustomMapFlipX{};
    BOOL m_bCustomMapFlipY{};
    long m_nRotateIdx{};

    CustomFixedCollectionOriginal() = default;

    std::vector<CustomFixed> vecOriginCustomFixedData;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db); //니가 저장되어야 한다.
    BOOL SaveMap(CString strFilePath);
    BOOL LoadMap(CString strFilePath, CString& strErrorMessage);
    //}}

private: //{{Function Help..
    float fGetValue_mm(const CStringA i_strfValue, const bool i_bisHeight = false);
    BOOL bGetVisble(const CStringA i_strVisble);
    CustomFixedCategory GetCustomFixedCategory(const CStringA i_strCustomCategory);

    BOOL Set_MapData(CString strpathName, CString& strErrorMessage);
    //}}
};
} // namespace Package