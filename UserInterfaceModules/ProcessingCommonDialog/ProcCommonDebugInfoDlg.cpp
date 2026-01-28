//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ProcCommonDebugInfoDlg.h"

//CPP_2_________________________________ This project's headers
#include "ProcCommonLogDlg.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
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
IMPLEMENT_DYNAMIC(ProcCommonDebugInfoDlg, CDialog)

ProcCommonDebugInfoDlg::ProcCommonDebugInfoDlg(CWnd* pParent, const CRect& rtPositionOnParent,
    VisionDebugInfoGroup& debugInfoGroup, ImageLotView& imageLotView, ProcCommonLogDlg& logDlg)
    : CDialog(IDD_DIALOG_DEBUG_INFO, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_debugInfoGroup(debugInfoGroup)
    , m_imageLotView(imageLotView)
    , m_logDlg(logDlg)
    , m_pGridCtrl(new CGridCtrl)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_DEBUG_INFO, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

ProcCommonDebugInfoDlg::~ProcCommonDebugInfoDlg()
{
    delete m_pGridCtrl;
}

void ProcCommonDebugInfoDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ProcCommonDebugInfoDlg, CDialog)
ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_DEBUG_INFO, OnGridSelChanged)
END_MESSAGE_MAP()

// ProcCommonDebugInfoDlg 메시지 처리기입니다.

BOOL ProcCommonDebugInfoDlg::OnInitDialog()
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

    Display(0);

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void ProcCommonDebugInfoDlg::OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    *pResult = 0;

    long itemIndex = (long)m_pGridCtrl->GetItemData(pGridView->iRow, 0);

    m_imageLotView.Overlay_RemoveAll();
    m_logDlg.SetLogBoxText(_T(""));

    if (itemIndex < 0 || itemIndex >= m_debugInfoGroup.m_vecDebugInfo.size())
    {
        m_imageLotView.Overlay_Show(TRUE);
        return;
    }

    CString overlayText;

    m_debugInfoGroup.m_vecDebugInfo[itemIndex].ToTextOverlay(overlayText, m_imageLotView.GetCoreView());

    // Text Log Show
    m_logDlg.SetLogBoxText(overlayText);

    // Overlay Show
    m_imageLotView.Overlay_Show(TRUE);
}

void ProcCommonDebugInfoDlg::Display(long key)
{
    const size_t debugInfoNum = m_debugInfoGroup.m_vecDebugInfo.size();

    long rowCount = 0;
    for (long i = 0; i < debugInfoNum; i++)
    {
        if (m_debugInfoGroup.m_vecDebugInfo[i].m_key == key)
        {
            rowCount++;
        }
    }

    m_pGridCtrl->SetRowCount(rowCount + 1);

    // Debug Info Grid 내용 채우기
    long rowIndex = 1;

    for (long i = 0; i < debugInfoNum; i++)
    {
        if (m_debugInfoGroup.m_vecDebugInfo[i].m_key != key)
        {
            continue;
        }

        m_pGridCtrl->SetItemText(rowIndex, 0, m_debugInfoGroup.m_vecDebugInfo[i].strDebugInfoName);
        m_pGridCtrl->SetItemText(rowIndex, 1, DebugInfoType2String(m_debugInfoGroup.m_vecDebugInfo[i].nDataType));
        m_pGridCtrl->SetItemData(rowIndex, 0, i);
        rowIndex++;
    }

    m_pGridCtrl->Refresh();

    Refresh();
}

void ProcCommonDebugInfoDlg::Refresh()
{
    const size_t debugInfoNum = m_debugInfoGroup.m_vecDebugInfo.size();

    for (long rowIndex = 1; rowIndex < m_pGridCtrl->GetRowCount(); rowIndex++)
    {
        long deubgIndex = (long)m_pGridCtrl->GetItemData(rowIndex, 0);

        if (deubgIndex < 0 || deubgIndex >= debugInfoNum || m_debugInfoGroup.m_vecDebugInfo[deubgIndex].nDataNum == 0)
        {
            for (long j = 0; j < GRID_NUM_X; j++)
            {
                m_pGridCtrl->SetItemBkColour(rowIndex, j, RGB(128, 128, 128));
            }
        }
        else
        {
            for (long j = 0; j < GRID_NUM_X; j++)
            {
                m_pGridCtrl->SetItemBkColour(rowIndex, j, RGB(255, 255, 255));
            }
        }
    }

    m_pGridCtrl->Refresh();
}
