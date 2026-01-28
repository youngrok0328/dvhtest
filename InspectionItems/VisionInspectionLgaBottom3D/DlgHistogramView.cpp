//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgHistogramView.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionLgaBottom3D.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Widget/ProfileView.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgHistogramView, CDialog)

CDlgHistogramView::CDlgHistogramView(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgHistogramView::IDD, pParent)
    , m_profileView(nullptr)
{
}

CDlgHistogramView::~CDlgHistogramView()
{
    delete m_profileView;
}

void CDlgHistogramView::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgHistogramView, CDialog)
ON_BN_CLICKED(IDOK, &CDlgHistogramView::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgHistogramView::OnBnClickedCancel)
ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()

// CDlgHistogramView 메시지 처리기입니다.

void CDlgHistogramView::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    //	OnOK();
}

void CDlgHistogramView::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnCancel();
}

BOOL CDlgHistogramView::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    CRect Rect;
    GetDlgItem(IDC_CHART)->GetWindowRect(&Rect);
    ScreenToClient(&Rect);

    m_profileView = new Ipvm::ProfileView(GetSafeHwnd(), Ipvm::FromMFC(Rect), IDC_CHART);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgHistogramView::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
    if (bShow)
    {
    }
}

void CDlgHistogramView::DrawGraph(CString strTitle, std::vector<long> vecnHisto)
{
    SetWindowText(strTitle);

    std::vector<Ipvm::Point32r2> profileData;
    profileData.resize(vecnHisto.size());

    for (long n = 0; n < long(vecnHisto.size()); n++)
    {
        profileData[n].m_x = (float)n;
        profileData[n].m_y = (float)vecnHisto[n];
    }

    m_profileView->Clear();

    if (profileData.size())
    {
        m_profileView->SetData(
            0, &profileData[0], long(profileData.size()), Ipvm::k_noiseValue32r, RGB(0, 0, 255), RGB(0, 0, 0));
    }
}