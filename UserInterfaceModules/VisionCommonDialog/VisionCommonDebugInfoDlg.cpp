//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionCommonDebugInfoDlg.h"

//CPP_2_________________________________ This project's headers
#include "VisionCommonTextResultDlg.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionDebugInfo.h"
#include "../ImageLotView/ImageLotView.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_NUM_X 2
#define IDC_GRID_DEBUG_INFO 100

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(VisionCommonDebugInfoDlg, CDialog)

VisionCommonDebugInfoDlg::VisionCommonDebugInfoDlg(CWnd* pParent, const CRect& rtPositionOnParent,
    VisionUnit& visionUnit, ImageLotView& imageLotView, VisionCommonTextResultDlg& textResultDlg)
    : CDialog(IDD_DIALOG_DEBUG_INFO, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_visionUnit(&visionUnit)
    , m_imageLotView(imageLotView)
    , m_textResultDlg(textResultDlg)
    , m_pGridCtrl(new CGridCtrl)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_DEBUG_INFO, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

VisionCommonDebugInfoDlg::~VisionCommonDebugInfoDlg()
{
    delete m_pGridCtrl;
}

void VisionCommonDebugInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(VisionCommonDebugInfoDlg, CDialog)
ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_DEBUG_INFO, OnGridSelChanged)
END_MESSAGE_MAP()

// VisionCommonDebugInfoDlg 메시지 처리기입니다.

BOOL VisionCommonDebugInfoDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CRect rect;
    GetClientRect(rect);

    m_pGridCtrl->Create(rect, this, IDC_GRID_DEBUG_INFO);
    m_pGridCtrl->SetFont(GetFont(), FALSE);
    m_pGridCtrl->SetEditable(TRUE);
    m_pGridCtrl->EnableDragAndDrop(FALSE);

    HDC hdc = ::GetDC(NULL);

    float designFactor = 1.f;
    if (hdc)
    {
        designFactor = GetDeviceCaps(hdc, LOGPIXELSY) / 96.f;
    }

    // Grid 가로 세로 개수 설정
    m_pGridCtrl->SetColumnCount(GRID_NUM_X);
    m_pGridCtrl->SetRowCount(1);
    m_pGridCtrl->SetFixedRowCount(1);
    m_pGridCtrl->SetEditable(FALSE);

    long type_width = CAST_LONG(designFactor * 80.f);
    m_pGridCtrl->SetColumnWidth(0, rect.Width() - type_width - 40);
    m_pGridCtrl->SetColumnWidth(1, type_width);

    // 0 번째 Row 에 각각의 Title 을 정의
    m_pGridCtrl->SetItemText(0, 0, _T("Name"));
    m_pGridCtrl->SetItemText(0, 1, _T("Type"));
    m_pGridCtrl->SetItemBkColour(0, 0, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 1, RGB(200, 200, 250));

    Refresh();

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void VisionCommonDebugInfoDlg::OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    *pResult = 0;

    const long itemIndex = pGridView->iRow - 1;

    m_imageLotView.Overlay_RemoveAll();
    m_textResultDlg.SetLogBoxText(_T(""));

    const auto debugInfos = m_visionUnit->GetVisionDebugInfos();

    if (itemIndex < 0 || itemIndex >= debugInfos.size())
    {
        m_imageLotView.Overlay_Show(TRUE);
        return;
    }

    CString overlayText;

    debugInfos[itemIndex]->ToTextOverlay(overlayText, m_imageLotView.GetCoreView());

    // Text Log Show
    m_textResultDlg.SetLogBoxText(overlayText);

    // Overlay Show
    m_imageLotView.Overlay_Show(TRUE);
}

void VisionCommonDebugInfoDlg::Refresh(VisionUnit& visionUnit)
{
    m_visionUnit = &visionUnit;
    Refresh();
}

void VisionCommonDebugInfoDlg::Refresh()
{
    m_pGridCtrl->SetRedraw(FALSE);

    const auto debugInfos = m_visionUnit->GetVisionDebugInfos();
    const long debugInfoCount = (long)debugInfos.size();

    m_pGridCtrl->SetRowCount(1 + debugInfoCount);

    // Debug Info Grid 내용 채우기
    long i;
    for (i = 0; i < debugInfoCount; i++)
    {
        m_pGridCtrl->SetItemText(i + 1, 0, debugInfos[i]->strDebugInfoName);
        m_pGridCtrl->SetItemText(i + 1, 1, DebugInfoType2String(debugInfos[i]->nDataType));

        if (debugInfos[i]->nDataNum == 0)
        {
            for (long j = 0; j < GRID_NUM_X; j++)
            {
                m_pGridCtrl->SetItemBkColour(i + 1, j, RGB(128, 128, 128));
            }
        }
        else
        {
            for (long j = 0; j < GRID_NUM_X; j++)
            {
                m_pGridCtrl->SetItemBkColour(i + 1, j, RGB(255, 255, 255));
            }
        }
    }

    m_pGridCtrl->SetRedraw(TRUE, TRUE);
}
