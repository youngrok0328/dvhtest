//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgYieldViewer.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgYieldViewer, CDialog)

CDlgYieldViewer::CDlgYieldViewer(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgYieldViewer::IDD, pParent)
    , m_saveRNR(TRUE)
{
}

CDlgYieldViewer::~CDlgYieldViewer()
{
}

void CDlgYieldViewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Check(pDX, IDC_CHECK_SAVE_RNR, m_saveRNR);
    DDX_Control(pDX, IDC_GRID_LIST, m_GridList);
}

BEGIN_MESSAGE_MAP(CDlgYieldViewer, CDialog)
ON_BN_CLICKED(IDOK, &CDlgYieldViewer::OnBnClickedOk)
END_MESSAGE_MAP()

// CDlgYieldViewer 메시지 처리기입니다.
BOOL CDlgYieldViewer::OnInitDialog()
{
    CDialog::OnInitDialog();

    /*영훈 20130807 : Grid 초기화*/
    m_GridList.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_GridList.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_GridList.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_GridList.SetDefCellMargin(0);
    m_GridList.SetFixedColumnSelection(FALSE);
    m_GridList.SetFixedRowSelection(FALSE);
    m_GridList.SetSingleColSelection(FALSE);
    m_GridList.SetSingleRowSelection(FALSE);
    m_GridList.SetTrackFocusCell(FALSE);
    m_GridList.SetRowResize(FALSE);
    m_GridList.SetColumnResize(FALSE);

    OnGridSetup();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgYieldViewer::SetVisionInspResult(const std::vector<stBatchInspResult>& vecstBatchInspResult)
{
    m_vecstBatchInspResult = vecstBatchInspResult;
}

void CDlgYieldViewer::OnGridSetup()
{
    long nInspNum = (long)m_vecstBatchInspResult.size();
    long nRowCount = nInspNum + 1;
    long nColCount = 6; //kircheis_3DEmpty

    m_GridList.DeleteAllItems();

    m_GridList.SetRowCount(nRowCount);
    m_GridList.SetFixedRowCount(1);
    // Spec Name, Not Measured, Pass, Reject, Invalid, Empty, Double
    m_GridList.SetColumnCount(nColCount);
    m_GridList.SetColumnWidth(1, 70);

    for (long nCol = 0; nCol < nColCount; nCol++)
    {
        m_GridList.SetItemBkColour(0, nCol, RGB(200, 200, 255));
        m_GridList.SetItemState(0, nCol, GVIS_READONLY);
    }

    for (long nRow = 1; nRow < nRowCount; nRow++)
    {
        for (long nCol = 0; nCol < nColCount; nCol++)
        {
            m_GridList.SetItemBkColour(nRow, nCol, RGB(255, 255, 255));
            m_GridList.SetItemState(nRow, nCol, GVIS_READONLY);
        }
    }

    m_GridList.SetColumnWidth(0, 300);
    m_GridList.SetItemText(0, 0, _T("Item Name"));

    m_GridList.SetColumnWidth(1, 80);
    m_GridList.SetItemText(0, 1, _T("Pass"));

    m_GridList.SetColumnWidth(2, 80);
    m_GridList.SetItemText(0, 2, _T("Reject"));

    m_GridList.SetColumnWidth(3, 80);
    m_GridList.SetItemText(0, 3, _T("Invalid"));

    m_GridList.SetColumnWidth(4, 80);
    m_GridList.SetItemText(0, 4, _T("Empty"));

    m_GridList.SetColumnWidth(5, 80);
    m_GridList.SetItemText(0, 5, _T("Double"));

    for (long nRow = 1; nRow < nRowCount; nRow++)
    {
        m_GridList.SetItemFormat(nRow, 0, (m_GridList.GetItemFormat(nRow, 0) & ~DT_CENTER) | DT_LEFT);
        m_GridList.SetItemTextFmt(nRow, 0, _T("%s"), (LPCTSTR)m_vecstBatchInspResult[nRow - 1].strInspName);
        m_GridList.SetItemTextFmt(nRow, 1, _T("%d"), m_vecstBatchInspResult[nRow - 1].nPass);
        m_GridList.SetItemTextFmt(nRow, 2, _T("%d"), m_vecstBatchInspResult[nRow - 1].nReject);
        m_GridList.SetItemTextFmt(nRow, 3, _T("%d"), m_vecstBatchInspResult[nRow - 1].nInvalid);
        m_GridList.SetItemTextFmt(nRow, 4, _T("%d"), m_vecstBatchInspResult[nRow - 1].nEmpty);
        m_GridList.SetItemTextFmt(nRow, 5, _T("%d"), m_vecstBatchInspResult[nRow - 1].nDouble);
    }

    m_GridList.Refresh();
}

void CDlgYieldViewer::OnBnClickedOk()
{
    CDialog::OnOK();
}
