#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "CustomPolygon.h"
#include "Enum.h"
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <vector>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API CustomPolygonCollectionOriginal
{
public:
    float m_fUnit{1.f};
    BOOL m_bCustomMapFlipX{};
    BOOL m_bCustomMapFlipY{};
    long m_nRotateIdx{};

    CustomPolygonCollectionOriginal() = default;

    std::vector<CustomPolygon> vecOriginCustomPolygonData;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db); //니가 저장되어야 한다.
    BOOL SaveMap(CString strFilePath);
    BOOL LoadMap(CString strFilePath, CString& strErrorMessage);
    //}}

private
    : //{{Function Help..
      //CCustomMap GetCurrentMapInfo(const std::vector<CStringA> i_vecstrDataList, const long i_fixColumn_Link[fixColumn_CustomMap::FX_END]);
    float fGetValue_mm(const CStringA i_strfValue, const bool i_bisHeight = false, const bool i_bisPointValue = false);
    BOOL bGetVisble(const CStringA i_strVisble);
    CustomPolygonCategory GetCustomPolygonCategory(const CStringA i_strCustomCategory);
    std::vector<Ipvm::Point32r2> GetPointInfo(const std::vector<CStringA> i_vecstrDataList, const long i_nPointNum);
    CString GetTotalPointInfo(const std::vector<Ipvm::Point32r2> i_vecfptTotalPointInfo, const long i_nMax_PointNum);

    BOOL Set_MapData(CString strpathName, CString& strErrorMessage);
    //}}
};

} // namespace Package