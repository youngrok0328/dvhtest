//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSubThresholdNormal.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgSubThresholdNormal, DlgSubThresholdBase)

DlgSubThresholdNormal::DlgSubThresholdNormal(
    int nUse3DImageType, CWnd* pParent /*=nullptr*/) //SDY_3D Surface VMap UI 오류 수정
    : DlgSubThresholdBase(IDD_SUB_THRESHOLD_NORMAL, pParent)
    , m_sbarInitialize(false)
    , m_low(nullptr)
    , m_high(nullptr)
{
    m_nUse3DImageType = nUse3DImageType; //SDY_3D Surface VMap UI 오류 수정
}

DlgSubThresholdNormal::~DlgSubThresholdNormal()
{
}

void DlgSubThresholdNormal::DoDataExchange(CDataExchange* pDX)
{
    DlgSubThresholdBase::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SBAR_DARK, m_sbarDark);
    DDX_Control(pDX, IDC_SBAR_BRIGHT, m_sbarBright);
}

BEGIN_MESSAGE_MAP(DlgSubThresholdNormal, DlgSubThresholdBase)
ON_EN_CHANGE(IDC_MinDarkContrast, &DlgSubThresholdNormal::OnEnChangeThresholdDark)
ON_EN_CHANGE(IDC_MinBrightContrast, &DlgSubThresholdNormal::OnEnChangeThresholdBright)
END_MESSAGE_MAP()

// DlgSubThresholdNormal message handlers

BOOL DlgSubThresholdNormal::OnInitDialog()
{
    DlgSubThresholdBase::OnInitDialog();

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && m_nUse3DImageType == SURFACE_3D_USE_ZMAP) //SDY_3D Surface VMap UI 오류 수정
    {
        // 3D 에서는 Threshold 값을 um단위로 입력하며, ScrollBar를 사용하지 않는다
        GetDlgItem(IDC_STC_ThresholdDarkName)->SetWindowText(_T("Limit Low Height :"));
        GetDlgItem(IDC_STC_ThresholdBrightName)->SetWindowText(_T("Limit High Height :"));
        GetDlgItem(IDC_STC_ThresholdDarkUnit)->SetWindowText(_T("um"));
        GetDlgItem(IDC_STC_ThresholdBrightUnit)->SetWindowText(_T("um"));

        m_sbarDark.SetScrollRange(0, 1000);
        m_sbarBright.SetScrollRange(0, 1000);
    }
    else
    {
        m_sbarDark.SetScrollRange(0, 255);
        m_sbarBright.SetScrollRange(0, 255);
    }

    m_sbarInitialize = true;

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgSubThresholdNormal::Set(long& low, long& high)
{
    m_low = &low;
    m_high = &high;

    m_sbarInitialize = false;

    m_sbarDark.SetScrollPos(low);
    m_sbarBright.SetScrollPos(high);

    SetDlgItemInt(IDC_MinDarkContrast, low);
    SetDlgItemInt(IDC_MinBrightContrast, high);

    m_sbarInitialize = true;
}

void DlgSubThresholdNormal::OnEnChangeThreshold(long target)
{
    if (!m_sbarInitialize)
        return;
    if (m_low == nullptr)
        return;
    if (m_high == nullptr)
        return;

    CString minBright;
    CString minDark;

    GetDlgItemText(IDC_MinBrightContrast, minBright);
    GetDlgItemText(IDC_MinDarkContrast, minDark);

    *m_low = _ttoi(minDark);
    *m_high = _ttoi(minBright);

    m_sbarDark.SetScrollPos(*m_low);
    m_sbarBright.SetScrollPos(*m_high);

    sendMessage_ThresholdChanged(target);

    minBright.Empty();
    minDark.Empty();
}

void DlgSubThresholdNormal::OnEnChangeThresholdDark()
{
    OnEnChangeThreshold(0);
}

void DlgSubThresholdNormal::OnEnChangeThresholdBright()
{
    OnEnChangeThreshold(1);
}

long DlgSubThresholdNormal::OnChangedScroll(CScrollBar* pScrollBar)
{
    if (m_low == nullptr)
        return 0;
    if (m_high == nullptr)
        return 0;

    int nID = pScrollBar->GetDlgCtrlID();
    auto scrollValue = pScrollBar->GetScrollPos();
    long retValue = 0;

    switch (nID)
    {
        case IDC_SBAR_DARK:
            retValue = 0;
            *m_low = max(0, scrollValue);
            break;

        case IDC_SBAR_BRIGHT:
            retValue = 1;
            *m_high = max(0, scrollValue);
            break;
    }

    m_sbarInitialize = false;
    SetDlgItemInt(IDC_MinDarkContrast, m_sbarDark.GetScrollPos());
    SetDlgItemInt(IDC_MinBrightContrast, m_sbarBright.GetScrollPos());
    m_sbarInitialize = true;

    return retValue;
}
