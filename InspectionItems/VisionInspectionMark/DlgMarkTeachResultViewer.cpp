//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgMarkTeachResultViewer.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionMark.h"

//CPP_3_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgMarkTeachResultViewer, CDialog)

CDlgMarkTeachResultViewer::CDlgMarkTeachResultViewer(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgMarkTeachResultViewer::IDD, pParent)
    , m_imageView(nullptr)
{
    m_vecrtEachCharROI.resize(0);
}

CDlgMarkTeachResultViewer::~CDlgMarkTeachResultViewer()
{
    delete m_imageView;
}

void CDlgMarkTeachResultViewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgMarkTeachResultViewer, CDialog)
END_MESSAGE_MAP()

void CDlgMarkTeachResultViewer::SetImageData(
    const Ipvm::Image8u& image, Ipvm::Rect32s i_rtViewArea, long i_nMarkCharNum, Ipvm::Rect32s* i_prtEachCharROI)
{
    long teachImageSizeX = i_rtViewArea.Width();
    long teachImageSizeY = i_rtViewArea.Height();

    m_image.Create(teachImageSizeX, teachImageSizeY);

    Ipvm::Point32s2 ptViewAreaLT = i_rtViewArea.TopLeft();

    m_vecrtEachCharROI.resize(i_nMarkCharNum);

    Ipvm::ImageProcessing::Copy(Ipvm::Image8u(image, i_rtViewArea), Ipvm::Rect32s(m_image), m_image);

    for (long i = 0; i < i_nMarkCharNum; i++)
    {
        m_vecrtEachCharROI[i] = i_prtEachCharROI[i];
        m_vecrtEachCharROI[i] -= ptViewAreaLT;
        m_vecrtEachCharROI[i].InflateRect(1, 1, 1, 1);
    }
}

BOOL CDlgMarkTeachResultViewer::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect view;
    GetDlgItem(IDC_MARK_TEACH_RESULT_VIEW)->GetWindowRect(view);
    ScreenToClient(view);
    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(view), 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    UpdateImage();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgMarkTeachResultViewer::UpdateImage()
{
    if ((m_image.GetSizeX() * m_image.GetSizeY()) <= 0)
    {
        return;
    }

    static COLORREF rgb = RGB(255, 0, 255);

    m_imageView->ROIClear();
    m_imageView->ImageOverlayClear();
    m_imageView->SetImage(m_image);

    for (auto& roi : m_vecrtEachCharROI)
    {
        m_imageView->ImageOverlayAdd(roi, rgb);
    }
    m_imageView->ImageOverlayShow();
}