#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "Component.h"
//HDR_3_________________________________ External library headers
#include <afxstr.h>
#include <vector>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API ComponentCollectionOriginal
{
public:
    float m_fUnit{1.f};
    BOOL m_bCompMapFlipX{};
    BOOL m_bCompMapFlipY{};
    long m_nRotateIdx{};
    long m_nRotateCompIdx{};

    ComponentCollectionOriginal() = default;

    std::vector<Component> vecOriginCompData;

    BOOL CheckIntegrityOfCompMapData(long& o_nErrorNum, long& o_nTotalNum); //kircheis_Comp3DHeightBug
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db); //니가 저장되어야 한다.
    BOOL SaveMap(CString strFilePath);
    BOOL LoadMap(CString strFilePath, CString& strErrorMessage);

private: //{{Function Help..
    BOOL Set_MapData(CString strpathName, CString& strErrorMessage);
    BOOL VerifyFile(CString csvTitle); //CString strpathName, CString &strErrorMessage);
    void Set_MapData(std::vector<CString>& vecstrToken, long nObjectNumber);
    BOOL CompTypeConv();
    CString CompTypeFloattoString(float fValue);
    //}}
};

} // namespace Package