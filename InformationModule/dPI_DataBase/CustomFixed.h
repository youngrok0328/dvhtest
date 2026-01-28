#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Enum.h"
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Point32r2.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//

class DPI_DATABASE_API CustomFixed
{
public:
    CString m_strCustomFixedName;
    CustomFixedCategory m_eCustomFixedCategory;
    float m_fSpecWidth_mm;
    float m_fSpecLength_mm;
    Ipvm::Point32r2 m_fSpecCenterPos_mm;
    float m_fSpecHeight_mm;
    BOOL m_bIgnore; //FALSE : 활성, TRUE : 비활성

    Ipvm::EllipseEq32r m_fSpec_ellipse_mm; //SpecEllipse
    FPI_RECT m_fsrtSpecROI; // SpecROI

    CustomFixed();
    ~CustomFixed();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    const CustomFixedCategory GetCatrgory() const;
    const CString GetID() const;
};
