//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgMarkTeachTotalResultViewer.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32s2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgMarkTeachTotalResultViewer, CDialog)

CDlgMarkTeachTotalResultViewer::CDlgMarkTeachTotalResultViewer(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgMarkTeachTotalResultViewer::IDD, pParent)
    , m_rtViewArea(Ipvm::Rect32s(0, 0, 0, 0))
    , m_imageView(nullptr)
    , m_nMarkCharNum(0)
{
    m_vecrtEachCharROI.resize(0);
}

CDlgMarkTeachTotalResultViewer::~CDlgMarkTeachTotalResultViewer()
{
    delete m_imageView;

    m_vecrtEachCharROI.clear();
}

void CDlgMarkTeachTotalResultViewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgMarkTeachTotalResultViewer, CDialog)
END_MESSAGE_MAP()

void CDlgMarkTeachTotalResultViewer::SetImageData(
    const Ipvm::Image8u& image, Ipvm::Rect32s i_rtViewArea, long i_nMarkCharNum, Ipvm::Rect32s* i_prtEachCharROI)
{
    m_imageTeach.Create(i_rtViewArea.Width(), i_rtViewArea.Height());
    m_rtViewArea = i_rtViewArea;
    m_nMarkCharNum = i_nMarkCharNum;

    Ipvm::Point32s2 ptViewAreaLT = m_rtViewArea.TopLeft();

    m_vecrtEachCharROI.clear();
    m_vecrtEachCharROI.resize(m_nMarkCharNum);

    if ((Ipvm::Rect32s(image) & i_rtViewArea) == i_rtViewArea)
    {
        Ipvm::ImageProcessing::Copy(Ipvm::Image8u(image, i_rtViewArea), Ipvm::Rect32s(m_imageTeach), m_imageTeach);
    }

    for (long i = 0; i < m_nMarkCharNum; i++)
    {
        m_vecrtEachCharROI[i] = i_prtEachCharROI[i];
        m_vecrtEachCharROI[i] -= ptViewAreaLT;
        m_vecrtEachCharROI[i].InflateRect(1, 1, 1, 1);
    }
    UpdateImage();
}

BOOL CDlgMarkTeachTotalResultViewer::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect viewRegion;
    GetDlgItem(IDC_MARK_TEACH_RESULT_VIEW)->GetWindowRect(viewRegion);
    ScreenToClient(viewRegion);

    m_imageView
        = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(viewRegion), 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgMarkTeachTotalResultViewer::UpdateImage()
{
    if ((m_imageTeach.GetSizeX() * m_imageTeach.GetSizeY()) <= 0)
        return;

    static COLORREF rgb = RGB(255, 0, 255);

    m_imageView->ROIClear();
    m_imageView->ImageOverlayClear();
    m_imageView->SetImage(m_imageTeach);

    for (auto& object : m_vecrtEachCharROI)
    {
        m_imageView->ImageOverlayAdd(object, rgb);
    }

    m_imageView->ImageOverlayShow();
}