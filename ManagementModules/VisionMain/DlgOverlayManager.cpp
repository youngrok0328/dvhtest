//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgOverlayManager.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridSlideBox.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgOverlayManager, CDialog)

DlgOverlayManager::DlgOverlayManager(VisionUnit& visionUnit, CWnd* pParent /*=NULL*/)
    : CDialog(DlgOverlayManager::IDD, pParent)
    , m_visionUnit(visionUnit)
{
    m_nSpecCount = (long)m_visionUnit.GetVisionProcessingCount();
    m_vecnOverlayMode.resize(m_nSpecCount);

    for (long n = 0; n < m_nSpecCount; n++)
    {
        m_vecnOverlayMode[n] = m_visionUnit.GetVisionProcessing(n)->m_nOverlayMode;
    }
}

DlgOverlayManager::~DlgOverlayManager()
{
}

void DlgOverlayManager::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_GRID, m_grid);
}

BEGIN_MESSAGE_MAP(DlgOverlayManager, CDialog)
ON_BN_CLICKED(IDOK, &DlgOverlayManager::OnBnClickedOk)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID, &DlgOverlayManager::OnGridSlideBoxClicked)
END_MESSAGE_MAP()

// DlgOverlayManager 메시지 처리기입니다.

BOOL DlgOverlayManager::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_grid.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_grid.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_grid.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_grid.SetDefCellMargin(0);
    m_grid.SetFixedColumnSelection(FALSE);
    m_grid.SetFixedRowSelection(FALSE);
    m_grid.SetSingleColSelection(FALSE);
    m_grid.SetSingleRowSelection(FALSE);
    m_grid.SetTrackFocusCell(FALSE);
    m_grid.SetRowResize(FALSE);
    m_grid.SetColumnResize(FALSE);

    // Items + Header
    m_grid.SetRowCount(m_nSpecCount + 2); // 메뉴랑 All Data + 2
    m_grid.SetFixedRowCount(1);
    m_grid.SetColumnCount(2);

    // 메뉴
    m_grid.SetItemText(0, 0, _T("Inspection Item Name"));
    m_grid.SetItemBkColour(0, 0, RGB(150, 150, 200));
    m_grid.SetItemState(0, 0, GVIS_READONLY);
    m_grid.SetColumnWidth(0, 200);

    m_grid.SetItemText(0, 1, _T("Use"));
    m_grid.SetItemBkColour(0, 1, RGB(150, 150, 200));
    m_grid.SetItemState(0, 1, GVIS_READONLY);
    m_grid.SetColumnWidth(1, 100);

    m_grid.SetItemText(1, 0, _T("All Data"));
    m_grid.SetItemBkColour(1, 0, RGB(200, 150, 200));
    m_grid.SetItemState(1, 0, GVIS_READONLY);

    m_grid.SetCellType(1, 1, RUNTIME_CLASS(CGridSlideBox));
    ((CGridSlideBox*)m_grid.GetCell(1, 1))->AddString(_T("SHOW"));
    ((CGridSlideBox*)m_grid.GetCell(1, 1))->AddString(_T("REJCT"));
    ((CGridSlideBox*)m_grid.GetCell(1, 1))->AddString(_T("HIDE"));
    ((CGridSlideBox*)m_grid.GetCell(1, 1))->SetCurSel(0);

    // 검사항목
    for (long nRow = 0; nRow < m_nSpecCount; nRow++)
    {
        CString strModuleName = m_visionUnit.GetVisionProcessing(nRow)->m_strModuleName;
        m_grid.SetItemText(nRow + 2, 0, strModuleName);
        m_grid.SetItemBkColour(nRow + 2, 0, RGB(200, 255, 255));
        m_grid.SetItemState(nRow + 2, 0, GVIS_READONLY);

        m_grid.SetCellType(nRow + 2, 1, RUNTIME_CLASS(CGridSlideBox));
        ((CGridSlideBox*)m_grid.GetCell(nRow + 2, 1))->AddString(_T("SHOW"));
        ((CGridSlideBox*)m_grid.GetCell(nRow + 2, 1))->AddString(_T("REJCT"));
        ((CGridSlideBox*)m_grid.GetCell(nRow + 2, 1))->AddString(_T("HIDE"));
        ((CGridSlideBox*)m_grid.GetCell(nRow + 2, 1))->SetCurSel(m_vecnOverlayMode[nRow]);

        strModuleName.Empty();
    }

    m_grid.Refresh();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgOverlayManager::OnBnClickedOk()
{
    for (long n = 0; n < m_nSpecCount; n++)
    {
        m_visionUnit.GetVisionProcessing(n)->m_nOverlayMode = m_vecnOverlayMode[n];
    }

    OnOK();
}

void DlgOverlayManager::OnGridSlideBoxClicked(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;
    long row = pNotify->iRow;

    if (row == 1) // AllData일 경우
    {
        long nCursel = ((CGridSlideBox*)m_grid.GetCell(row, 1))->GetCurSel();
        ((CGridSlideBox*)m_grid.GetCell(1, 1))->SetCurSel(nCursel); // all Data
        for (long n = 0; n < m_nSpecCount; n++)
        {
            m_vecnOverlayMode[n] = nCursel;
            ((CGridSlideBox*)m_grid.GetCell(n + 2, 1))->SetCurSel(nCursel);
        }
    }
    else
    {
        m_vecnOverlayMode[row - 2] = ((CGridSlideBox*)m_grid.GetCell(row, 1))->GetCurSel();
    }

    m_grid.Refresh();

    *result = 0;
}
