//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionCommonSpecDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_NUM_X 11
#define IDC_GRID_SPEC 100

//CPP_7_________________________________ Implementation body
//
enum SpecDialogColumnID
{
    COL_PARAM_SPEC_NO = 0,
    COL_PARAM_SPEC_NAME,
    COL_PARAM_SPEC_USE,
    COL_PARAM_SPEC_MARGINAL_MIN_SPEC,
    COL_PARAM_SPEC_PASS_MIN_SPEC,
    COL_PARAM_SPEC_PASS_MAX_SPEC,
    COL_PARAM_SPEC_MARGINAL_MAX_SPEC,
    COL_PARAM_SPEC_UNIT,
    COL_PARAM_SPEC_USE_MIN,
    COL_PARAM_SPEC_USE_MAX,
    COL_PARAM_SPEC_USE_MARGINAL,
    COL_PARAM_SPEC_END,
};

static LPCTSTR g_strSpecDialogColumnName[] = {
    _T("No."),
    _T("Name"),
    _T("Use"),
    _T("Min."),
    _T(" "),
    _T("Max."),
    _T(" "),
    _T("Unit"),
    _T("Use"),
    _T(" "),
    _T(" "),
    _T(" "),
    _T(" "),
    _T(" "),
    _T("Margin"),
    _T("Pass"),
    _T("Pass"),
    _T("Margin"),
    _T(" "),
    _T("Min."),
    _T("Max."),
    _T("Margin"),
};

IMPLEMENT_DYNAMIC(VisionCommonSpecDlg, CDialog)

VisionCommonSpecDlg::VisionCommonSpecDlg(CWnd* pParent, const CRect& rtPositionOnParent, VisionUnit& visionUnit)
    : CDialog(IDD_DIALOG_SPEC, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_visionUnit(visionUnit)
    , m_pGridCtrl(new CGridCtrl)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_SPEC, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

VisionCommonSpecDlg::~VisionCommonSpecDlg()
{
    delete m_pGridCtrl;
}

void VisionCommonSpecDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(VisionCommonSpecDlg, CDialog)
END_MESSAGE_MAP()

// VisionCommonSpecDlg 메시지 처리기입니다.

BOOL VisionCommonSpecDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CRect rect;
    GetClientRect(rect);

    m_pGridCtrl->Create(rect, this, IDC_GRID_SPEC);
    m_pGridCtrl->SetFont(GetFont(), FALSE);
    m_pGridCtrl->SetEditable(FALSE);
    m_pGridCtrl->EnableDragAndDrop(FALSE);
    m_pGridCtrl->EnableTitleTips(FALSE);

    // Grid 정렬
    m_pGridCtrl->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    Refresh();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void VisionCommonSpecDlg::Refresh()
{
    auto visionInspectionSpecs = m_visionUnit.GetVisionInspectionSpecs();

    long nHeaderCnt = 2;

    //--------------------------------------------------------------------------------------
    // Report Category 별로 모듈이 몇개 있는지 카운팅한다
    //--------------------------------------------------------------------------------------

    std::map<long, std::map<CString, BOOL>> inspNameInCategory;
    std::map<long, std::map<CString, BOOL>>::iterator Test;
    std::map<CString, BOOL>::iterator FindCategoryIndex;
    std::map<std::pair<long, CString>, std::vector<long>> specListInCategoryAndInspName;
    CString strPreveInspName;
    for (long index = 0; index < long(visionInspectionSpecs.size()); index++)
    {
        if (index > 0)
            strPreveInspName = visionInspectionSpecs[index - 1]->m_inspName;

        auto& spec = visionInspectionSpecs[index];

        if (!spec->m_use)
            continue; // 사용하지 않는 것은 감춤

        long category = long(spec->m_hostReportCategory);

        if (strPreveInspName == spec->m_inspName) //mc_이전과 현재의 Index가 같은 Insecption Dll이라면
        {
            for (long n = 0; n < inspNameInCategory.size(); n++)
            {
                if (inspNameInCategory[n].find(strPreveInspName) != inspNameInCategory[n].end())
                    category = n;
            }
        }

        inspNameInCategory[category][spec->m_inspName] = TRUE;

        specListInCategoryAndInspName[std::pair<long, CString>(category, spec->m_inspName)].push_back(index);
    }

    //--------------------------------------------------------------------------------------

    m_pGridCtrl->SetRedraw(FALSE);
    m_pGridCtrl->DeleteAllItems();

    m_pGridCtrl->SetColumnCount(COL_PARAM_SPEC_END);
    m_pGridCtrl->SetRowCount(nHeaderCnt);
    m_pGridCtrl->SetFixedRowCount(nHeaderCnt);

    // 0 번째 Row 에 각각의 Title 을 정의
    for (long row = 0; row < 2; row++)
    {
        long nIdxR = COL_PARAM_SPEC_END * row;

        for (long column = 0; column < COL_PARAM_SPEC_END; column++)
        {
            long nIdxC = nIdxR + column;

            m_pGridCtrl->SetItemText(row, column, g_strSpecDialogColumnName[nIdxC]);
            m_pGridCtrl->GetCell(row, column)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            m_pGridCtrl->SetItemBkColour(row, column, RGB(200, 200, 250));
        }
    }

    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_NO, 30);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_NAME, 170);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_USE, 40);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_MARGINAL_MIN_SPEC, 50);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_PASS_MAX_SPEC, 50);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_PASS_MIN_SPEC, 50);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_MARGINAL_MAX_SPEC, 50);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_UNIT, 40);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_USE_MIN, 40);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_USE_MAX, 40);
    m_pGridCtrl->SetColumnWidth(COL_PARAM_SPEC_USE_MARGINAL, 50);

    m_pGridCtrl->MergeCells(0, COL_PARAM_SPEC_NO, 1, COL_PARAM_SPEC_NO);
    m_pGridCtrl->MergeCells(0, COL_PARAM_SPEC_NAME, 1, COL_PARAM_SPEC_NAME);
    m_pGridCtrl->MergeCells(0, COL_PARAM_SPEC_USE, 1, COL_PARAM_SPEC_USE);
    m_pGridCtrl->MergeCells(0, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, 0, COL_PARAM_SPEC_PASS_MIN_SPEC);
    m_pGridCtrl->MergeCells(0, COL_PARAM_SPEC_PASS_MAX_SPEC, 0, COL_PARAM_SPEC_MARGINAL_MAX_SPEC);
    m_pGridCtrl->MergeCells(0, COL_PARAM_SPEC_UNIT, 1, COL_PARAM_SPEC_UNIT);
    m_pGridCtrl->MergeCells(0, COL_PARAM_SPEC_USE_MIN, 0, COL_PARAM_SPEC_USE_MARGINAL);

    long index = 0;

    for (auto& itSpecList : specListInCategoryAndInspName)
    {
        CString inspName = itSpecList.first.second;
        long Inspcategory = itSpecList.first.first;
        COLORREF crItemBack = RGB(255, 255, 255);

        //if (inspNameInCategory[Inspcategory].size() > 1)
        if (inspNameInCategory[Inspcategory].size() > 0)
        {
            // 해당 카테고리를 쓰는 다른 항목이 존재한다.
            // 구분이 되지 않으므로 Spec을 구분하기 쉽게 모듈 이름을 적어준다

            long titleRow = m_pGridCtrl->InsertRow(_T(""));
            m_pGridCtrl->SetItemText(titleRow, 0, inspName);
            m_pGridCtrl->MergeCells(titleRow, 0, titleRow, COL_PARAM_SPEC_END - 1);
            m_pGridCtrl->SetItemBkColour(titleRow, COL_PARAM_SPEC_NO, RGB(120, 120, 220));
            m_pGridCtrl->SetItemFgColour(titleRow, 0, RGB(255, 255, 255));

            crItemBack = RGB(255, 255, 230);
        }

        for (auto& specIndex : itSpecList.second)
        {
            auto& spec = *visionInspectionSpecs[specIndex];
            long specRow = m_pGridCtrl->InsertRow(_T(""));

            for (long column = 0; column < m_pGridCtrl->GetColumnCount(); column++)
            {
                m_pGridCtrl->SetItemBkColour(specRow, column, crItemBack);
            }

            CString str;

            // No
            str.Format(_T("%d"), index + 1);
            m_pGridCtrl->SetItemText(specRow, COL_PARAM_SPEC_NO, str);
            m_pGridCtrl->SetItemBkColour(specRow, COL_PARAM_SPEC_NO, RGB(250, 250, 220));

            // Name
            m_pGridCtrl->SetItemBkColour(specRow, COL_PARAM_SPEC_NAME, RGB(255, 255, 255));
            m_pGridCtrl->SetItemText(specRow, COL_PARAM_SPEC_NAME, spec.m_specName);

            // Use
            m_pGridCtrl->SetCellType(specRow, COL_PARAM_SPEC_USE, RUNTIME_CLASS(CGridCellCheck2));
            ((CGridCellCheck2*)m_pGridCtrl->GetCell(specRow, COL_PARAM_SPEC_USE))->SetCheck(spec.m_use);

            // Marginal Min
            str.Format(_T("%.2f"), spec.m_marginalMin);
            m_pGridCtrl->SetItemText(specRow, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, str);

            // Pass Min
            str.Format(_T("%.2f"), spec.m_passMin);
            m_pGridCtrl->SetItemText(specRow, COL_PARAM_SPEC_PASS_MIN_SPEC, str);

            // Pass Max
            str.Format(_T("%.2f"), spec.m_passMax);
            m_pGridCtrl->SetItemText(specRow, COL_PARAM_SPEC_PASS_MAX_SPEC, str);

            // Marginal Max
            str.Format(_T("%.2f"), spec.m_marginalMax);
            m_pGridCtrl->SetItemText(specRow, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, str);

            // Unit
            m_pGridCtrl->SetItemText(specRow, COL_PARAM_SPEC_UNIT, spec.m_unit);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_UNIT, GVIS_READONLY);

            // Use Min
            m_pGridCtrl->SetCellType(specRow, COL_PARAM_SPEC_USE_MIN, RUNTIME_CLASS(CGridCellCheck2));
            ((CGridCellCheck2*)m_pGridCtrl->GetCell(specRow, COL_PARAM_SPEC_USE_MIN))->SetCheck(spec.m_useMin);

            // Use Max
            m_pGridCtrl->SetCellType(specRow, COL_PARAM_SPEC_USE_MAX, RUNTIME_CLASS(CGridCellCheck2));
            ((CGridCellCheck2*)m_pGridCtrl->GetCell(specRow, COL_PARAM_SPEC_USE_MAX))->SetCheck(spec.m_useMax);

            // Use Marginal
            m_pGridCtrl->SetCellType(specRow, COL_PARAM_SPEC_USE_MARGINAL, RUNTIME_CLASS(CGridCellCheck2));
            ((CGridCellCheck2*)m_pGridCtrl->GetCell(specRow, COL_PARAM_SPEC_USE_MARGINAL))
                ->SetCheck(spec.m_useMarginal);

            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_NO, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_NAME, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_USE, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_MARGINAL_MIN_SPEC, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_PASS_MIN_SPEC, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_PASS_MAX_SPEC, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_MARGINAL_MAX_SPEC, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_UNIT, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_USE_MIN, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_USE_MAX, GVIS_READONLY);
            m_pGridCtrl->SetItemState(specRow, COL_PARAM_SPEC_USE_MARGINAL, GVIS_READONLY);

            index++;
        }
    }

    m_pGridCtrl->SetRedraw(TRUE, TRUE);
}
