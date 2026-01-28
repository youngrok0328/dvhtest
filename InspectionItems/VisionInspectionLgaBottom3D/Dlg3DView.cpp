//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Dlg3DView.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionLgaBottom3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Widget/SurfaceView.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlg3DView, CDialog)

CDlg3DView::CDlg3DView(ImageLotView& view, VisionImageLotInsp& imageLotInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlg3DView::IDD, pParent)
    , m_p3DViewerAgent(nullptr)
    , m_imageLotView(view)
    , m_imageLotInsp(imageLotInsp)
    , m_bMedianFilter(FALSE)
    , m_nMedianRepeat(1)
    , m_f3DViewScale(1.f)
    , m_nMaskSize(5)
{
}

CDlg3DView::~CDlg3DView()
{
    delete m_p3DViewerAgent;
}

void CDlg3DView::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_MEDIAN_FILTER, m_bMedianFilter);
    DDX_Text(pDX, IDC_EDIT_MEDIAN_REPEAT, m_nMedianRepeat);
    DDX_Text(pDX, IDC_EDIT_VIEW_SCALE, m_f3DViewScale);
    DDX_Text(pDX, IDC_EDIT_MASK_SIZE, m_nMaskSize);
}

BEGIN_MESSAGE_MAP(CDlg3DView, CDialog)
ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CDlg3DView::OnBnClickedButtonRefresh)
ON_BN_CLICKED(IDC_BUTTON_EXIT, &CDlg3DView::OnBnClickedButtonExit)
ON_BN_CLICKED(IDC_CHECK_MEDIAN_FILTER, &CDlg3DView::OnBnClickedCheckMedianFilter)
ON_EN_CHANGE(IDC_EDIT_MEDIAN_REPEAT, &CDlg3DView::OnEnChangeEditMedianRepeat)
ON_EN_CHANGE(IDC_EDIT_VIEW_SCALE, &CDlg3DView::OnEnChangeEditViewScale)
END_MESSAGE_MAP()

// CDlg3DView 메시지 처리기입니다.

BOOL CDlg3DView::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rt;
    GetDlgItem(IDC_STATIC_VIEW)->GetWindowRect(rt);
    ScreenToClient(rt);

    m_p3DViewerAgent = new Ipvm::SurfaceView(GetSafeHwnd(), Ipvm::FromMFC(rt));

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlg3DView::OnBnClickedButtonRefresh()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);

    Ipvm::Rect32s rtROI;
    m_imageLotView.ROI_Get(_T(""), rtROI);

    rtROI &= m_imageLotInsp.GetImageRect();

    m_p3DViewerAgent->Load(m_imageLotInsp.m_zmapImage, rtROI, 10.f, 10.f, 0.f, Ipvm::ColorMapIndex::e_rainbow);
}

void CDlg3DView::OnBnClickedButtonExit()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    m_imageLotView.ROI_RemoveAll();
    m_imageLotView.ROI_Show(TRUE);

    OnCancel();
}

void CDlg3DView::OnBnClickedCheckMedianFilter()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);
}

void CDlg3DView::OnEnChangeEditMedianRepeat()
{
    UpdateData(TRUE);
}

void CDlg3DView::OnEnChangeEditViewScale()
{
    UpdateData(TRUE);
}
