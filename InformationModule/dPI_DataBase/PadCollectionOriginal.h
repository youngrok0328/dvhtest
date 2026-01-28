#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "Pad.h"

//HDR_3_________________________________ External library headers
#include <minwindef.h>
#include <vector>
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API PadCollectionOriginal
{
public:
    float m_fUnit{1.f};
    BOOL m_bPadMapFlipX{};
    BOOL m_bPadMapFlipY{};
    long m_nRotateIdx{};

    PadCollectionOriginal() = default;

    std::vector<Pad> vecOriginPadData;

public:
    BOOL SaveMap(CString strFilePath);
    BOOL LoadMap(CString strFilePath, CString& strErrorMessage);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db); //니가 저장되어야 한다.
    //}}

private:
    BOOL Set_MapData(CString strpathName, CString& strErrorMessage);
    BOOL VerifyFile(CString csvTitle);
    void Set_MapData(std::vector<CString>& vecstrToken, long nObjectNumber);
};

} // namespace Package