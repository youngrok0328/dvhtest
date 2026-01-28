#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "Land.h"

//HDR_3_________________________________ External library headers
#include <vector>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API LandCollectionOriginal
{
public:
    float m_fUnit{1.f};
    BOOL m_bLandMapFlipX{};
    BOOL m_bLandMapFlipY{};
    long m_nRotateIdx{};
    BOOL m_bTitleVisible{};
    long m_nRotateLandIdx{};
    //}}
    CString m_strAllGroupName{}; //kircheis_MED2.5

    LandCollectionOriginal() = default;

    std::vector<Package::Land> vecOriginLandData;

    BOOL SaveMap(CString strFilePath);
    BOOL LoadMap(CString strFilePath, CString& strErrorMessage);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db); //니가 저장되어야 한다.
    //}}

private: //{{Function Help..
    BOOL Set_MapData(CString strpathName, CString& strErrorMessage);
    BOOL Set_MapData_Shape(CString strpathName, CString& strErrorMessage);
    BOOL Set_MapData_IgnoreVisible(CString strpathName, CString& strErrorMessage);
    void Set_MapData(std::vector<CString>& vecstrToken, long nObjectNumber);
    //}}
};

} // namespace Package