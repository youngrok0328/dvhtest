#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Enum.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API CustomPolygon
{
public:
    CString m_strCustomPolygonName;
    CustomPolygonCategory m_eCustomPolygonCategory;
    long m_nPointNum;
    float m_fSpecHeight_mm;
    BOOL m_bIgnore; //FALSE : 활성, TRUE : 비활성

    std::vector<Ipvm::Point32r2> m_vecfptPointInfo_mm;

    CustomPolygon();
    ~CustomPolygon();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    const CustomPolygonCategory GetCatrgory() const;
    const CString GetID() const;
};

} // namespace Package
