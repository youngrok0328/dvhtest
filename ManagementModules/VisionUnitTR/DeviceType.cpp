//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DeviceType.h"

//CPP_2_________________________________ This project's headers
#include "DeviceType_Impl.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CDeviceType::CDeviceType()
    : m_nDeviceType(enDeviceType::PACKAGE_UNKNOWN)
    , m_nPackageType(enPackageType::UNKNOWN)
    , m_bDeadBug(FALSE)
{
}

CDeviceType::~CDeviceType()
{
}

BOOL CDeviceType::DoModal()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CDeviceType_Impl Dlg;
    if (Dlg.DoModal() == IDOK)
    {
        m_nDeviceType = Dlg.GetDeviceType();
        m_nPackageType = Dlg.GetPackageType();
        m_bDeadBug = Dlg.GetDeadBug();
        m_strJobName = Dlg.GetJobName();
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}
