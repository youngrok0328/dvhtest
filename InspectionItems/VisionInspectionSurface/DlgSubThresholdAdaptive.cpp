//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSubThresholdAdaptive.h"

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
IMPLEMENT_DYNAMIC(DlgSubThresholdAdaptive, DlgSubThresholdBase)

DlgSubThresholdAdaptive::DlgSubThresholdAdaptive(
    int nUse3DImageType, CWnd* pParent /*=nullptr*/) //SDY_3D Surface VMap UI 오류 수정
    : DlgSubThresholdBase(IDD_SUB_THRESHOLD_ADAPTIVE, pParent)
    , m_sbarInitialize(false)
    , m_scrollDarkMax(255)
    , m_darkThreshold(nullptr)
    , m_brightThreshold(nullptr)
{
    m_nUse3DImageType = nUse3DImageType; //SDY_3D Surface VMap UI 오류 수정
}

DlgSubThresholdAdaptive::~DlgSubThresholdAdaptive()
{
}

void DlgSubThresholdAdaptive::DoDataExchange(CDataExchange* pDX)
{
    DlgSubThresholdBase::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SBAR_DARK, m_sbarDark);
    DDX_Control(pDX, IDC_SBAR_BRIGHT, m_sbarBright);
}

BEGIN_MESSAGE_MAP(DlgSubThresholdAdaptive, DlgSubThresholdBase)
ON_EN_CHANGE(IDC_MinDarkContrast, &DlgSubThresholdAdaptive::OnEnChangeThresholdDark)
ON_EN_CHANGE(IDC_MinBrightContrast, &DlgSubThresholdAdaptive::OnEnChangeThresholdBright)
END_MESSAGE_MAP()

// DlgSubThresholdAdaptive message handlers

BOOL DlgSubThresholdAdaptive::OnInitDialog()
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

        m_scrollDarkMax = 1000;
        m_sbarDark.SetScrollRange(0, 1000);
        m_sbarBright.SetScrollRange(0, 1000);
    }
    else
    {
        m_scrollDarkMax = 255;
        m_sbarDark.SetScrollRange(0, 255);
        m_sbarBright.SetScrollRange(0, 255);
    }

    m_sbarInitialize = true;

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgSubThresholdAdaptive::Set(float& darkThreshold, float& brightThreshold)
{
    m_darkThreshold = &darkThreshold;
    m_brightThreshold = &brightThreshold;

    auto intThresholdDark = (long)(darkThreshold + 0.5f);
    auto intThresholdBright = (long)(brightThreshold + 0.5f);

    m_sbarInitialize = false;

    m_sbarDark.SetScrollPos(m_scrollDarkMax - intThresholdDark);
    m_sbarBright.SetScrollPos(intThresholdBright);

    SetDlgItemInt(IDC_MinDarkContrast, intThresholdDark);
    SetDlgItemInt(IDC_MinBrightContrast, intThresholdBright);

    m_sbarInitialize = true;
}

void DlgSubThresholdAdaptive::OnEnChangeThreshold(long target)
{
    if (!m_sbarInitialize)
        return;
    if (m_darkThreshold == nullptr)
        return;
    if (m_brightThreshold == nullptr)
        return;

    CString minBright;
    CString minDark;

    GetDlgItemText(IDC_MinBrightContrast, minBright);
    GetDlgItemText(IDC_MinDarkContrast, minDark);

    *m_darkThreshold = (float)_ttoi(minDark);
    *m_brightThreshold = (float)_ttoi(minBright);

    m_sbarDark.SetScrollPos(m_scrollDarkMax - long(*m_darkThreshold + 0.5f));
    m_sbarBright.SetScrollPos(long(*m_brightThreshold + 0.5f));

    sendMessage_ThresholdChanged(target);

    minBright.Empty();
    minDark.Empty();
}

void DlgSubThresholdAdaptive::OnEnChangeThresholdDark()
{
    OnEnChangeThreshold(0);
}

void DlgSubThresholdAdaptive::OnEnChangeThresholdBright()
{
    OnEnChangeThreshold(1);
}

long DlgSubThresholdAdaptive::OnChangedScroll(CScrollBar* pScrollBar)
{
    if (m_darkThreshold == nullptr)
        return 0;
    if (m_brightThreshold == nullptr)
        return 0;

    int nID = pScrollBar->GetDlgCtrlID();
    auto scrollValue = pScrollBar->GetScrollPos();
    long retValue = 0;

    switch (nID)
    {
        case IDC_SBAR_DARK:
            retValue = 0;
            *m_darkThreshold = m_scrollDarkMax - max(0.f, (float)(scrollValue));
            break;

        case IDC_SBAR_BRIGHT:
            retValue = 1;
            *m_brightThreshold = max(0.f, (float)scrollValue);
            break;
    }

    m_sbarInitialize = false;
    SetDlgItemInt(IDC_MinDarkContrast, CAST_UINT(*m_darkThreshold));
    SetDlgItemInt(IDC_MinBrightContrast, CAST_UINT(*m_brightThreshold));
    m_sbarInitialize = true;

    return retValue;
}
