//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ProcCommonSpecDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_NUM_X 11
#define IDC_GRID_SPEC 100

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(ProcCommonSpecDlg, CDialog)

ProcCommonSpecDlg::ProcCommonSpecDlg(
    CWnd* pParent, const CRect& rtPositionOnParent, std::vector<VisionInspectionSpec>& inspectionSpecs)
    : CDialog(IDD_DIALOG_SPEC, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_inspectionSpecs(inspectionSpecs)
    , m_pGridCtrl(new CGridCtrl)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_SPEC, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

ProcCommonSpecDlg::~ProcCommonSpecDlg()
{
    delete m_pGridCtrl;
}

void ProcCommonSpecDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(ProcCommonSpecDlg, CDialog)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_SPEC, &ProcCommonSpecDlg::OnGridDataChanged)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_SPEC, &ProcCommonSpecDlg::OnGridDataChanged)
END_MESSAGE_MAP()

// ProcCommonSpecDlg 메시지 처리기입니다.

BOOL ProcCommonSpecDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CRect rect;
    GetClientRect(rect);

    m_pGridCtrl->Create(rect, this, IDC_GRID_SPEC);
    m_pGridCtrl->SetFont(GetFont(), FALSE);
    m_pGridCtrl->SetEditable(TRUE);
    m_pGridCtrl->EnableDragAndDrop(FALSE);

    // Grid 가로 세로 개수 설정
    long nColCount = GRID_NUM_X;

    // Grid 가로 세로 개수 설정
    long nHeaderCnt = 2;
    long nSpecNum = (long)(m_inspectionSpecs.size());
    m_pGridCtrl->SetColumnCount(nColCount);
    m_pGridCtrl->SetRowCount(nSpecNum + nHeaderCnt);
    m_pGridCtrl->SetFixedRowCount(nHeaderCnt);
    m_pGridCtrl->SetFixedColumnCount(2);
    m_pGridCtrl->SetEditable(TRUE);

    // Grid 정렬
    m_pGridCtrl->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SetInitGrid();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void ProcCommonSpecDlg::SetInitGrid()
{
    long nHeaderCnt = 2;
    long nSpecNum = (long)(m_inspectionSpecs.size());
    long i;
    CString str;
    long nTotalRowCount = nHeaderCnt;

    for (i = 0; i < nSpecNum; i++)
    {
        auto& spec = m_inspectionSpecs[i];

        m_pGridCtrl->SetItemBkColour(nTotalRowCount, 0, RGB(250, 250, 220));
        // ID
        str.Format(_T("%d"), i);
        m_pGridCtrl->SetItemText(nTotalRowCount, 0, str);
        // Name
        m_pGridCtrl->SetItemText(nTotalRowCount, 1, spec.m_specName);
        m_pGridCtrl->SetItemState(
            nTotalRowCount, 1, m_pGridCtrl->GetItemState(i + 1, 1) | GVIS_READONLY); // SpecName은 읽기전용
        // Use
        m_pGridCtrl->SetCellType(nTotalRowCount, 2, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 2))->SetCheck(spec.m_use);
        // Marginal Min
        str.Format(_T("%.2f"), spec.m_marginalMin);
        m_pGridCtrl->SetItemText(nTotalRowCount, 3, str);
        // Pass Min
        str.Format(_T("%.2f"), spec.m_passMin);
        m_pGridCtrl->SetItemText(nTotalRowCount, 4, str);
        // Pass Max
        str.Format(_T("%.2f"), spec.m_passMax);
        m_pGridCtrl->SetItemText(nTotalRowCount, 5, str);
        // Marginal Max
        str.Format(_T("%.2f"), spec.m_marginalMax);
        m_pGridCtrl->SetItemText(nTotalRowCount, 6, str);
        // Unit
        str = _T("[") + spec.m_unit + _T("]");
        m_pGridCtrl->SetItemText(nTotalRowCount, 7, str);
        m_pGridCtrl->SetItemState(nTotalRowCount, 7, GVIS_READONLY);
        // Use Min
        m_pGridCtrl->SetCellType(nTotalRowCount, 8, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 8))->SetCheck(spec.m_useMin);
        // Use Max
        m_pGridCtrl->SetCellType(nTotalRowCount, 9, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 9))->SetCheck(spec.m_useMax);
        // Use Marginal
        m_pGridCtrl->SetCellType(nTotalRowCount, 10, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 10))->SetCheck(spec.m_useMarginal);

        if (spec.m_marginalMin > spec.m_passMin || !spec.m_useMarginal)
        {
            spec.m_marginalMin = spec.m_passMin;
            str.Format(_T("%.2f"), spec.m_marginalMin);
            m_pGridCtrl->SetItemText(nTotalRowCount, 3, str);
        }

        if (spec.m_marginalMax < spec.m_passMax || !spec.m_useMarginal)
        {
            spec.m_marginalMax = spec.m_passMax;
            str.Format(_T("%.2f"), spec.m_marginalMax);
            m_pGridCtrl->SetItemText(nTotalRowCount, 6, str);
        }

        //k Spec 활성화 && 비활성화
        if (spec.m_use)
        {
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 7, RGB(255, 255, 255));
            m_pGridCtrl->SetItemState(nTotalRowCount, 8, GVIS_MODIFIED);
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 8, RGB(255, 255, 255));
            m_pGridCtrl->SetItemState(nTotalRowCount, 9, GVIS_MODIFIED);
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 9, RGB(255, 255, 255));
            m_pGridCtrl->SetItemState(nTotalRowCount, 10, GVIS_MODIFIED);
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 10, RGB(255, 255, 255));

            if (spec.m_useMin)
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 4, GVIS_MODIFIED);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 4, RGB(255, 255, 255));

                if (spec.m_useMarginal)
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 3, GVIS_MODIFIED);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 3, RGB(255, 255, 255));
                }
                else
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 3, GVIS_READONLY);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 3, RGB(134, 135, 137));
                }
            }
            else
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 3, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 3, RGB(134, 135, 137));
                m_pGridCtrl->SetItemState(nTotalRowCount, 4, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 4, RGB(134, 135, 137));
            }

            if (spec.m_useMax)
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 5, GVIS_MODIFIED);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 5, RGB(255, 255, 255));

                if (spec.m_useMarginal)
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 6, GVIS_MODIFIED);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 6, RGB(255, 255, 255));
                }
                else
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 6, GVIS_READONLY);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 6, RGB(134, 135, 137));
                }
            }
            else
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 5, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 5, RGB(134, 135, 137));
                m_pGridCtrl->SetItemState(nTotalRowCount, 6, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 6, RGB(134, 135, 137));
            }
        }
        else
        {
            for (int nCol = 3; nCol <= 10; nCol++)
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, nCol, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, nCol, RGB(134, 135, 137));
            }
        }

        nTotalRowCount++;
    }

    m_pGridCtrl->SetItemText(0, 0, _T("No."));
    m_pGridCtrl->SetItemText(0, 1, _T("Name"));
    m_pGridCtrl->SetItemText(0, 2, _T("Use"));
    m_pGridCtrl->SetItemText(0, 3, _T("Min."));
    m_pGridCtrl->SetItemText(0, 4, _T(" "));
    m_pGridCtrl->SetItemText(0, 5, _T("Max."));
    m_pGridCtrl->SetItemText(0, 6, _T(" "));
    m_pGridCtrl->SetItemText(0, 7, _T("Unit"));
    m_pGridCtrl->SetItemText(0, 8, _T("Use"));
    m_pGridCtrl->SetItemText(0, 9, _T(" "));
    m_pGridCtrl->SetItemText(0, 10, _T(" "));
    m_pGridCtrl->SetItemText(1, 0, _T(" "));
    m_pGridCtrl->SetItemText(1, 1, _T(" "));
    m_pGridCtrl->SetItemText(1, 2, _T(" "));
    m_pGridCtrl->SetItemText(1, 3, _T("Margin"));
    m_pGridCtrl->SetItemText(1, 4, _T("Pass"));
    m_pGridCtrl->SetItemText(1, 5, _T("Pass"));
    m_pGridCtrl->SetItemText(1, 6, _T("Margin"));
    m_pGridCtrl->SetItemText(1, 7, _T(" "));
    m_pGridCtrl->SetItemText(1, 8, _T("Min."));
    m_pGridCtrl->SetItemText(1, 9, _T("Max."));
    m_pGridCtrl->SetItemText(1, 10, _T("Margin"));

    for (long nRow = 0; nRow < 2; nRow++)
    {
        for (long nCol = 0; nCol <= 10; nCol++)
        {
            m_pGridCtrl->GetCell(nRow, nCol)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            m_pGridCtrl->SetItemBkColour(nRow, nCol, RGB(200, 200, 250));
        }
    }

    HDC hdc = ::GetDC(NULL);

    float designFactor = 1.f;
    if (hdc)
    {
        designFactor = GetDeviceCaps(hdc, LOGPIXELSY) / 96.f;
    }

    long no_width = static_cast<long>(40.f * designFactor);
    long check_width = static_cast<long>(40.f * designFactor);
    long value_width = static_cast<long>(55.f * designFactor);
    long name_width = (m_rtInitPosition.Width() - 40) - value_width * 4 - check_width * 5 - no_width;

    m_pGridCtrl->SetColumnWidth(0, no_width);
    m_pGridCtrl->SetColumnWidth(1, name_width);
    m_pGridCtrl->SetColumnWidth(2, check_width);
    m_pGridCtrl->SetColumnWidth(3, value_width);
    m_pGridCtrl->SetColumnWidth(4, value_width);
    m_pGridCtrl->SetColumnWidth(5, value_width);
    m_pGridCtrl->SetColumnWidth(6, value_width);
    m_pGridCtrl->SetColumnWidth(7, check_width);
    m_pGridCtrl->SetColumnWidth(8, check_width);
    m_pGridCtrl->SetColumnWidth(9, check_width);
    m_pGridCtrl->SetColumnWidth(10, check_width);

    m_pGridCtrl->MergeCells(0, 0, 1, 0);
    m_pGridCtrl->MergeCells(0, 1, 1, 1);
    m_pGridCtrl->MergeCells(0, 2, 1, 2);
    m_pGridCtrl->MergeCells(0, 3, 0, 4);
    m_pGridCtrl->MergeCells(0, 5, 0, 6);
    m_pGridCtrl->MergeCells(0, 7, 1, 7);
    m_pGridCtrl->MergeCells(0, 8, 0, 10);

    m_pGridCtrl->Refresh();
}

void ProcCommonSpecDlg::OnGridDataChanged(NMHDR* pNotifyStruct, LRESULT* result)
{
    UNREFERENCED_PARAMETER(pNotifyStruct);

    long nSpecNum = (long)(m_inspectionSpecs.size());
    long i;
    CString str;

    for (i = 0; i < nSpecNum; i++)
    {
        auto& spec = m_inspectionSpecs[i];

        // Use
        spec.m_use = ((CGridCellCheck2*)m_pGridCtrl->GetCell(i + 2, 2))->GetCheck();

        // Marginal Min
        str = m_pGridCtrl->GetItemText(i + 2, 3);
        spec.m_marginalMin = (float)_ttof(str);

        // Pass Min
        str = m_pGridCtrl->GetItemText(i + 2, 4);
        spec.m_passMin = (float)_ttof(str);

        // Pass Max
        str = m_pGridCtrl->GetItemText(i + 2, 5);
        spec.m_passMax = (float)_ttof(str);

        // Marginal Max
        str = m_pGridCtrl->GetItemText(i + 2, 6);
        spec.m_marginalMax = (float)_ttof(str);

        // Unit
        str = _T("[") + spec.m_unit + _T("]");
        m_pGridCtrl->SetItemText(i + 2, 7, str);

        // Use Min
        spec.m_useMin = ((CGridCellCheck2*)m_pGridCtrl->GetCell(i + 2, 8))->GetCheck();

        // Use Max
        spec.m_useMax = ((CGridCellCheck2*)m_pGridCtrl->GetCell(i + 2, 9))->GetCheck();

        // Use Margianl
        spec.m_useMarginal = ((CGridCellCheck2*)m_pGridCtrl->GetCell(i + 2, 10))->GetCheck();

        if (spec.m_marginalMin > spec.m_passMin || !spec.m_useMarginal)
            spec.m_marginalMin = spec.m_passMin;

        if (spec.m_marginalMax < spec.m_passMax || !spec.m_useMarginal)
            spec.m_marginalMax = spec.m_passMax;
    }

    Invalidate(FALSE);
    SetInitGrid();

    *result = 0;
}

// Spec에 정보 추가 했을 경우 이 함수를 사용 ( EX: Inspection Geometry )
void ProcCommonSpecDlg::ResetInitGrid()
{
    m_pGridCtrl->DeleteAllItems();
    // Grid 가로 세로 개수 설정
    long nColCount = GRID_NUM_X;

    // Grid 가로 세로 개수 설정
    long nHeaderCnt = 2;
    long nSpecNum = (long)(m_inspectionSpecs.size());
    long i;
    CString str;
    long nTotalRowCount = nHeaderCnt;

    m_pGridCtrl->SetColumnCount(nColCount);
    m_pGridCtrl->SetRowCount(nSpecNum + nHeaderCnt);
    m_pGridCtrl->SetFixedRowCount(nHeaderCnt);
    m_pGridCtrl->SetFixedColumnCount(2);

    for (i = 0; i < nSpecNum; i++)
    {
        auto& spec = m_inspectionSpecs[i];

        m_pGridCtrl->SetItemBkColour(nTotalRowCount, 0, RGB(250, 250, 220));
        // ID
        str.Format(_T("%d"), i);
        m_pGridCtrl->SetItemText(nTotalRowCount, 0, str);
        // Name
        m_pGridCtrl->SetItemText(nTotalRowCount, 1, spec.m_specName);
        m_pGridCtrl->SetItemState(
            nTotalRowCount, 1, m_pGridCtrl->GetItemState(i + 1, 1) | GVIS_READONLY); // SpecName은 읽기전용
        // Use
        m_pGridCtrl->SetCellType(nTotalRowCount, 2, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 2))->SetCheck(spec.m_use);
        // Marginal Min
        str.Format(_T("%.2f"), spec.m_marginalMin);
        m_pGridCtrl->SetItemText(nTotalRowCount, 3, str);
        // Pass Min
        str.Format(_T("%.2f"), spec.m_passMin);
        m_pGridCtrl->SetItemText(nTotalRowCount, 4, str);
        // Pass Max
        str.Format(_T("%.2f"), spec.m_passMax);
        m_pGridCtrl->SetItemText(nTotalRowCount, 5, str);
        // Marginal Max
        str.Format(_T("%.2f"), spec.m_marginalMax);
        m_pGridCtrl->SetItemText(nTotalRowCount, 6, str);
        // Unit
        str = _T("[") + spec.m_unit + _T("]");
        m_pGridCtrl->SetItemText(nTotalRowCount, 7, str);
        m_pGridCtrl->SetItemState(nTotalRowCount, 7, GVIS_READONLY);
        // Use Min
        m_pGridCtrl->SetCellType(nTotalRowCount, 8, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 8))->SetCheck(spec.m_useMin);
        // Use Max
        m_pGridCtrl->SetCellType(nTotalRowCount, 9, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 9))->SetCheck(spec.m_useMax);
        // Use Marginal
        m_pGridCtrl->SetCellType(nTotalRowCount, 10, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nTotalRowCount, 10))->SetCheck(spec.m_useMarginal);

        if (spec.m_marginalMin > spec.m_passMin || !spec.m_useMarginal)
        {
            spec.m_marginalMin = spec.m_passMin;
            str.Format(_T("%.2f"), spec.m_marginalMin);
            m_pGridCtrl->SetItemText(nTotalRowCount, 3, str);
        }

        if (spec.m_marginalMax < spec.m_passMax || !spec.m_useMarginal)
        {
            spec.m_marginalMax = spec.m_passMax;
            str.Format(_T("%.2f"), spec.m_marginalMax);
            m_pGridCtrl->SetItemText(nTotalRowCount, 6, str);
        }

        //k Spec 활성화 && 비활성화
        if (spec.m_use)
        {
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 7, RGB(255, 255, 255));
            m_pGridCtrl->SetItemState(nTotalRowCount, 8, GVIS_MODIFIED);
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 8, RGB(255, 255, 255));
            m_pGridCtrl->SetItemState(nTotalRowCount, 9, GVIS_MODIFIED);
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 9, RGB(255, 255, 255));
            m_pGridCtrl->SetItemState(nTotalRowCount, 10, GVIS_MODIFIED);
            m_pGridCtrl->SetItemBkColour(nTotalRowCount, 10, RGB(255, 255, 255));

            if (spec.m_useMin)
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 4, GVIS_MODIFIED);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 4, RGB(255, 255, 255));

                if (spec.m_useMarginal)
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 3, GVIS_MODIFIED);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 3, RGB(255, 255, 255));
                }
                else
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 3, GVIS_READONLY);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 3, RGB(134, 135, 137));
                }
            }
            else
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 3, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 3, RGB(134, 135, 137));
                m_pGridCtrl->SetItemState(nTotalRowCount, 4, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 4, RGB(134, 135, 137));
            }

            if (spec.m_useMax)
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 5, GVIS_MODIFIED);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 5, RGB(255, 255, 255));

                if (spec.m_useMarginal)
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 6, GVIS_MODIFIED);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 6, RGB(255, 255, 255));
                }
                else
                {
                    m_pGridCtrl->SetItemState(nTotalRowCount, 6, GVIS_READONLY);
                    m_pGridCtrl->SetItemBkColour(nTotalRowCount, 6, RGB(134, 135, 137));
                }
            }
            else
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, 5, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 5, RGB(134, 135, 137));
                m_pGridCtrl->SetItemState(nTotalRowCount, 6, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, 6, RGB(134, 135, 137));
            }
        }
        else
        {
            for (int nCol = 3; nCol <= 10; nCol++)
            {
                m_pGridCtrl->SetItemState(nTotalRowCount, nCol, GVIS_READONLY);
                m_pGridCtrl->SetItemBkColour(nTotalRowCount, nCol, RGB(134, 135, 137));
            }
        }

        nTotalRowCount++;
    }

    m_pGridCtrl->SetItemText(0, 0, _T("No."));
    m_pGridCtrl->SetItemText(0, 1, _T("Name"));
    m_pGridCtrl->SetItemText(0, 2, _T("Use"));
    m_pGridCtrl->SetItemText(0, 3, _T("Min."));
    m_pGridCtrl->SetItemText(0, 4, _T(" "));
    m_pGridCtrl->SetItemText(0, 5, _T("Max."));
    m_pGridCtrl->SetItemText(0, 6, _T(" "));
    m_pGridCtrl->SetItemText(0, 7, _T("Unit"));
    m_pGridCtrl->SetItemText(0, 8, _T("Use"));
    m_pGridCtrl->SetItemText(0, 9, _T(" "));
    m_pGridCtrl->SetItemText(0, 10, _T(" "));
    m_pGridCtrl->SetItemText(1, 0, _T(" "));
    m_pGridCtrl->SetItemText(1, 1, _T(" "));
    m_pGridCtrl->SetItemText(1, 2, _T(" "));
    m_pGridCtrl->SetItemText(1, 3, _T("Margin"));
    m_pGridCtrl->SetItemText(1, 4, _T("Pass"));
    m_pGridCtrl->SetItemText(1, 5, _T("Pass"));
    m_pGridCtrl->SetItemText(1, 6, _T("Margin"));
    m_pGridCtrl->SetItemText(1, 7, _T(" "));
    m_pGridCtrl->SetItemText(1, 8, _T("Min."));
    m_pGridCtrl->SetItemText(1, 9, _T("Max."));
    m_pGridCtrl->SetItemText(1, 10, _T("Margin"));

    for (long nRow = 0; nRow < 2; nRow++)
    {
        for (long nCol = 0; nCol <= 10; nCol++)
        {
            m_pGridCtrl->GetCell(nRow, nCol)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            m_pGridCtrl->SetItemBkColour(nRow, nCol, RGB(200, 200, 250));
        }
    }

    m_pGridCtrl->SetColumnWidth(0, 40);
    m_pGridCtrl->SetColumnWidth(1, 170);
    m_pGridCtrl->SetColumnWidth(2, 40);
    m_pGridCtrl->SetColumnWidth(3, 55);
    m_pGridCtrl->SetColumnWidth(4, 55);
    m_pGridCtrl->SetColumnWidth(5, 55);
    m_pGridCtrl->SetColumnWidth(6, 55);
    m_pGridCtrl->SetColumnWidth(7, 40);
    m_pGridCtrl->SetColumnWidth(8, 40);
    m_pGridCtrl->SetColumnWidth(9, 40);
    m_pGridCtrl->SetColumnWidth(10, 50);

    m_pGridCtrl->MergeCells(0, 0, 1, 0);
    m_pGridCtrl->MergeCells(0, 1, 1, 1);
    m_pGridCtrl->MergeCells(0, 2, 1, 2);
    m_pGridCtrl->MergeCells(0, 3, 0, 4);
    m_pGridCtrl->MergeCells(0, 5, 0, 6);
    m_pGridCtrl->MergeCells(0, 7, 1, 7);
    m_pGridCtrl->MergeCells(0, 8, 0, 10);

    m_pGridCtrl->Refresh();
}

void ProcCommonSpecDlg::SelectDisableCoulmn(long i_nIndex) //mc_해당 Index의 Spec을 비활성화 한다
{
    if (i_nIndex > m_inspectionSpecs.size())
        return;

    long nRowCount = 2 + i_nIndex;

    for (long nCol = 1; nCol < 11; nCol++)
    {
        m_pGridCtrl->SetItemState(nRowCount, nCol, GVIS_READONLY);
        m_pGridCtrl->SetItemBkColour(nRowCount, nCol, RGB(134, 135, 137));

        auto& spec = m_inspectionSpecs[i_nIndex];
        if (spec.m_use)
            spec.m_use = FALSE;

        ((CGridCellCheck2*)m_pGridCtrl->GetCell(nRowCount, 2))->SetCheck(spec.m_use);
    }

    m_pGridCtrl->Refresh();
}