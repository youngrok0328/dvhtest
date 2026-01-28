//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DeviceType_Impl.h"

//CPP_2_________________________________ This project's headers
#include "Resource.h"

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
IMPLEMENT_DYNAMIC(CDeviceType_Impl, CDialog)

CDeviceType_Impl::CDeviceType_Impl(CWnd* pParent /*=NULL*/)
    : CDialog(CDeviceType_Impl::IDD, pParent)
    , m_nDeviceType(enDeviceType::PACKAGE_UNKNOWN)
    , m_nPackageType(enPackageType::UNKNOWN)
    , m_bDeadBug(FALSE)
    , m_nRadioPackageType(0)
    , m_strJobName("Untitle")
{
}

CDeviceType_Impl::~CDeviceType_Impl()
{
}

void CDeviceType_Impl::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHK_DEADBUG, m_bDeadBug);
    DDX_Radio(pDX, IDC_RADIO_DEVICE_TYPE, m_nRadioPackageType);
}

BEGIN_MESSAGE_MAP(CDeviceType_Impl, CDialog)
ON_BN_CLICKED(IDOK, &CDeviceType_Impl::OnBnClickedOk)
END_MESSAGE_MAP()

// CDeviceType_Impl 메시지 처리기입니다.
BOOL CDeviceType_Impl::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetDlgItemText(IDC_EDIT_JOB_NAME, m_strJobName);

    return TRUE;
}

void CDeviceType_Impl::OnBnClickedOk()
{
    UpdateData(TRUE);

    switch (m_nRadioPackageType)
    {
        case 0:
            m_nPackageType = enPackageType::TSOP_1;
            break;
        case 1:
            m_nPackageType = enPackageType::TSOP_2;
            break;
        case 2:
            m_nPackageType = enPackageType::QFP;
            break;
        case 3:
            m_nPackageType = enPackageType::BGA;
            break;
        case 4:
            m_nPackageType = enPackageType::CIS;
            break;
        case 5:
            m_nPackageType = enPackageType::QFN;
            break;
        case 6:
            m_nPackageType = enPackageType::CARD;
            break;
        case 7:
            m_nPackageType = enPackageType::FC_BGA;
            break;
        case 8:
            m_nPackageType = enPackageType::LGA;
            break; //mc_2017. 09.11
        default:
            ASSERT(!"No implementation for that package type.");
    }

    if (m_nPackageType <= enPackageType::QFP)
    {
        m_nDeviceType = enDeviceType::PACKAGE_GULLWING;
    }
    else if (m_nPackageType == enPackageType::BGA || m_nPackageType == enPackageType::CIS)
    {
        m_nDeviceType = enDeviceType::PACKAGE_BALL;
    }
    else if (m_nPackageType == enPackageType::QFN || m_nPackageType == enPackageType::CARD)
    {
        m_nDeviceType = enDeviceType::PACKAGE_LEADLESS;
    }
    else if (m_nPackageType == enPackageType::FC_BGA)
    {
        m_nDeviceType = enDeviceType::PACKAGE_CHIP;
    }
    else if (m_nPackageType == enPackageType::LGA)
    {
        m_nDeviceType = enDeviceType::PACKAGE_LAND;
    }

    ((CEdit*)GetDlgItem(IDC_EDIT_JOB_NAME))->GetWindowText(m_strJobName);

    OnOK();
}

enDeviceType CDeviceType_Impl::GetDeviceType()
{
    return m_nDeviceType;
}

enPackageType CDeviceType_Impl::GetPackageType()
{
    return m_nPackageType;
}

BOOL CDeviceType_Impl::GetDeadBug()
{
    return m_bDeadBug;
}

CString CDeviceType_Impl::GetJobName()
{
    return m_strJobName;
}
