//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
#include "ProcCommonDetailResultDlg.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionResultGroup.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_NUM_X 6
#define IDC_GRID_RESULT_DETAIL 100

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(ProcCommonDetailResultDlg, CDialog)

ProcCommonDetailResultDlg::ProcCommonDetailResultDlg(CWnd* pParent, const CRect& rtPositionOnParent,
    VisionInspectionResultGroup& inspectionResultGroup, ImageLotView& imageLotView)
    : CDialog(IDD_DIALOG_RESULT_DETAIL, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_inspectionResultGroup(inspectionResultGroup)
    , m_imageLotView(imageLotView)
    , m_pGridCtrl(new CGridCtrl)
    , m_nCurGrid(0)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_RESULT_DETAIL, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

ProcCommonDetailResultDlg::~ProcCommonDetailResultDlg()
{
    delete m_pGridCtrl;
}

void ProcCommonDetailResultDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ProcCommonDetailResultDlg, CDialog)
ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_RESULT_DETAIL, OnGridSelChanged)
END_MESSAGE_MAP()

// ProcCommonDetailResultDlg 메시지 처리기입니다.

BOOL ProcCommonDetailResultDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CRect rect;
    GetClientRect(rect);

    m_pGridCtrl->Create(rect, this, IDC_GRID_RESULT_DETAIL);
    m_pGridCtrl->SetFont(GetFont(), FALSE);
    m_pGridCtrl->SetEditable(FALSE);
    m_pGridCtrl->EnableDragAndDrop(FALSE);

    long nResultNum = 0;

    // Grid 가로 세로 개수 설정
    m_pGridCtrl->SetColumnCount(GRID_NUM_X);
    m_pGridCtrl->SetRowCount(nResultNum + 1);
    m_pGridCtrl->SetFixedRowCount(1);

    // 0 번째 Row 에 각각의 Title 을 정의
    m_pGridCtrl->SetItemText(0, 0, _T("NO"));
    m_pGridCtrl->SetItemText(0, 1, _T("ID"));
    m_pGridCtrl->SetItemText(0, 2, _T("X"));
    m_pGridCtrl->SetItemText(0, 3, _T("Y"));
    m_pGridCtrl->SetItemText(0, 4, _T("Value"));
    m_pGridCtrl->SetItemText(0, 5, _T("Result"));
    m_pGridCtrl->SetColumnWidth(0, 50);
    m_pGridCtrl->SetColumnWidth(1, 50);

    m_pGridCtrl->SetHeaderSort();
    m_pGridCtrl->SetCompareFunction(compareGridSort);

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void ProcCommonDetailResultDlg::Refresh(long itemIndex)
{
    if (itemIndex >= 0 && itemIndex < m_inspectionResultGroup.m_vecResult.size())
    {
        m_nCurGrid = itemIndex;
    }

    long nResultNum = 0;
    if (m_inspectionResultGroup.m_vecResult.size() > 0)
    {
        if (m_nCurGrid >= 0 && m_nCurGrid < (long)(m_inspectionResultGroup.m_vecResult.size()))
        {
            nResultNum = (long)(m_inspectionResultGroup.m_vecResult[m_nCurGrid].m_objectResults.size());
        }
    }

    m_pGridCtrl->SetRedraw(FALSE);

    // Grid 가로 세로 개수 설정
    m_pGridCtrl->SetRowCount(nResultNum + 1);
    m_pGridCtrl->SetFixedRowCount(1);

    if (nResultNum <= 0)
    {
        m_pGridCtrl->SetRedraw(TRUE, TRUE);
        return;
    }

    const auto& result = m_inspectionResultGroup.m_vecResult[m_nCurGrid];

    // 값 채우기
    long i;
    CString str;
    CString strObjectName;
    for (i = 0; i < nResultNum; i++)
    {
        // NO
        m_pGridCtrl->SetItemTextFmt(i + 1, 0, _T("%d"), i + 1);

        // ID
        strObjectName = result.m_objectNames[i];
        if (strObjectName.IsEmpty())
            m_pGridCtrl->SetItemTextFmt(i + 1, 1, _T("%d"), i + 1);
        else
            m_pGridCtrl->SetItemText(i + 1, 1, strObjectName);

        // X
        m_pGridCtrl->SetItemTextFmt(i + 1, 2, _T("%d"), result.m_objectRects[i].CenterPoint().m_x);

        // Y
        m_pGridCtrl->SetItemTextFmt(i + 1, 3, _T("%d"), result.m_objectRects[i].CenterPoint().m_y);

        // Value
        if (result.m_objectErrorValues[i] == Ipvm::k_noiseValue32r)
        {
            m_pGridCtrl->SetItemTextFmt(i + 1, 4, _T("INV"));
        }
        else
        {
            m_pGridCtrl->SetItemTextFmt(i + 1, 4, _T("%.2f"), result.m_objectErrorValues[i]);
        }

        // Result
        m_pGridCtrl->SetItemText(i + 1, 5, Result2String(result.m_objectResults[i]));
        m_pGridCtrl->SetItemBkColour(i + 1, 5, Result2Color(result.m_objectResults[i]));
    }

    m_pGridCtrl->SetRedraw(TRUE, TRUE);
}

void ProcCommonDetailResultDlg::OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    *pResult = 0;

    const long index = ::_ttoi(m_pGridCtrl->GetItemText(pGridView->iRow, 0)) - 1;

    if (index < 0 || index >= m_inspectionResultGroup.m_vecResult[m_nCurGrid].m_objectNames.size())
    {
        return;
    }

    const auto& rect = m_inspectionResultGroup.m_vecResult[m_nCurGrid].m_objectRects[index];

    Ipvm::Point32s2 ptCenter = rect.CenterPoint();

    m_imageLotView.Overlay_RemoveAll();

    Ipvm::Point32s2 ptX_Begin, ptX_End, ptY_Begin, ptY_End;
    ptX_Begin.Set(ptCenter.m_x - 40, ptCenter.m_y);
    ptX_End.Set(ptCenter.m_x + 40, ptCenter.m_y);

    ptY_Begin.Set(ptCenter.m_x, ptCenter.m_y - 40);
    ptY_End.Set(ptCenter.m_x, ptCenter.m_y + 40);

    m_imageLotView.Overlay_AddLine(ptX_Begin, ptX_End, RGB(0, 255, 0));
    m_imageLotView.Overlay_AddLine(ptY_Begin, ptY_End, RGB(0, 255, 0));

    m_imageLotView.Overlay_Show(TRUE);
}

int ProcCommonDetailResultDlg::compareGridSort(LPARAM source1, LPARAM source2, LPARAM data)
{
    auto* cell1 = (CGridCellBase*)source1;
    auto* cell2 = (CGridCellBase*)source2;

    long columnIndex = cell1->GetGrid()->GetSortColumn();

    CString text1 = cell1->GetText();
    CString text2 = cell2->GetText();

    if (columnIndex == 1 || columnIndex == 5)
    {
        // ID or RESULT
        if (!data)
        {
            return text1.Compare(text2);
        }

        return text1.Compare(text2) * -1;
    }

    float value1 = 0.f;
    float value2 = 0.f;

    if (text1 == _T("INV"))
    {
        value1 = Ipvm::k_noiseValue32r;
    }
    else
    {
        value1 = (float)_ttof(text1);
    }

    if (text2 == _T("INV"))
    {
        value2 = Ipvm::k_noiseValue32r;
    }
    else
    {
        value2 = (float)_ttof(text2);
    }

    long retValue = value1 > value2 ? 1 : value1 < value2 ? -1 : 0;

    if (data)
    {
        retValue *= -1;
    }

    return retValue;
}
