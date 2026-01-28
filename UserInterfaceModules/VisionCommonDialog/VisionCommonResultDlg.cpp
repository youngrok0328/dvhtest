//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionCommonResultDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define GRID_NUM_X 8
#define IDC_GRID_RESULT 100

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(VisionCommonResultDlg, CDialog)

VisionCommonResultDlg::VisionCommonResultDlg(CWnd* pParent, const CRect& rtPositionOnParent, VisionUnit& visionUnit,
    ImageLotView& imageLotView, UINT uiDetailButtonClickedMsg)
    : CDialog(IDD_DIALOG_RESULT, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_visionUnit(&visionUnit)
    , m_imageLotView(imageLotView)
    , m_uiDetailButtonClickedMsg(uiDetailButtonClickedMsg)
    , m_pGridCtrl(new CGridCtrl)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_RESULT, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

VisionCommonResultDlg::~VisionCommonResultDlg()
{
    delete m_pGridCtrl;
}

void VisionCommonResultDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(VisionCommonResultDlg, CDialog)
ON_NOTIFY(GVN_SELCHANGED, IDC_GRID_RESULT, OnGridSelChanged)
ON_NOTIFY(NM_DBLCLK, IDC_GRID_RESULT, OnDblclkGridResult)
END_MESSAGE_MAP()

// VisionCommonResultDlg 메시지 처리기입니다.

BOOL VisionCommonResultDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CRect rect;
    GetClientRect(rect);

    m_pGridCtrl->Create(rect, this, IDC_GRID_RESULT);
    m_pGridCtrl->SetFont(GetFont(), FALSE);
    m_pGridCtrl->SetEditable(TRUE);
    m_pGridCtrl->EnableDragAndDrop(FALSE);

    // Grid 가로 세로 개수 설정
    m_pGridCtrl->SetColumnCount(GRID_NUM_X);
    m_pGridCtrl->SetRowCount(1);
    m_pGridCtrl->SetFixedRowCount(1);
    m_pGridCtrl->SetFixedColumnCount(1);
    m_pGridCtrl->SetEditable(FALSE);
    m_pGridCtrl->EnableTitleTips(FALSE);

    // Grid 정렬
    m_pGridCtrl->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_pGridCtrl->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    // 0 번째 Row 에 각각의 Title 을 정의
    m_pGridCtrl->SetItemText(0, 0, _T("No."));
    m_pGridCtrl->SetItemText(0, 1, _T("Item"));
    m_pGridCtrl->SetItemText(0, 2, _T("Pass Spec"));
    m_pGridCtrl->SetItemText(0, 3, _T("Marginal Spec"));
    m_pGridCtrl->SetItemText(0, 4, _T("Result"));
    m_pGridCtrl->SetItemText(0, 5, _T("Value"));
    m_pGridCtrl->SetItemText(0, 6, _T("Unit"));
    m_pGridCtrl->SetItemText(0, 7, _T("Detail"));

    m_pGridCtrl->SetItemBkColour(0, 0, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 1, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 2, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 3, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 4, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 5, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 6, RGB(200, 200, 250));
    m_pGridCtrl->SetItemBkColour(0, 7, RGB(200, 200, 250));

    m_pGridCtrl->SetColumnWidth(0, 30);
    m_pGridCtrl->SetColumnWidth(1, 180);
    m_pGridCtrl->SetColumnWidth(4, 90);
    m_pGridCtrl->SetColumnWidth(5, 45);
    m_pGridCtrl->SetColumnWidth(6, 45);
    m_pGridCtrl->SetColumnWidth(7, 45);

    Refresh();

    return TRUE; // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void VisionCommonResultDlg::OnDblclkGridResult(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;
    if (pGridView == NULL)
        return;

    CCellID CellID = m_pGridCtrl->GetFocusCell();

    if (CellID.col == 7 && m_uiDetailButtonClickedMsg >= WM_USER)
    {
        long rowIndex = CellID.row;
        long itemIndex = CAST_LONG(m_pGridCtrl->GetItemData(rowIndex, 0));

        GetParent()->PostMessage(m_uiDetailButtonClickedMsg, WPARAM(itemIndex));
    }

    *pResult = 0;
}

void VisionCommonResultDlg::OnGridSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    NM_GRIDVIEW* pGridView = (NM_GRIDVIEW*)pNMHDR;

    *pResult = 0;

    if (pGridView->iColumn == 5)
    {
        return;
    }

    long rowIndex = pGridView->iRow;
    long itemIndex = CAST_LONG(m_pGridCtrl->GetItemData(rowIndex, 0));

    const long nInspResultNum = (long)m_vecpVisionInspectionResult.size();
    const bool bIsUseResultInVisionUnit = (nInspResultNum <= 0);

    auto inspectionResults
        = bIsUseResultInVisionUnit ? m_visionUnit->GetVisionInspectionResults() : m_vecpVisionInspectionResult;

    if (itemIndex < 0 || (itemIndex >= inspectionResults.size()))
    {
        m_imageLotView.Overlay_RemoveAll();
        m_imageLotView.Overlay_Show(TRUE);
        return;
    }

    m_imageLotView.Overlay_RemoveAll();

    const auto& result = inspectionResults[itemIndex];

    long nRejectSize = (long)(result->vecrtRejectROI.size());
    long nMarginalSize = (long)(result->vecrtMarginalROI.size());

    for (long i = 0; i < nRejectSize; i++)
    {
        m_imageLotView.Overlay_AddRectangle(result->vecrtRejectROI[i], RGB(255, 0, 0));
    }

    for (long i = 0; i < nMarginalSize; i++)
    {
        m_imageLotView.Overlay_AddRectangle(result->vecrtMarginalROI[i], RGB(243, 157, 58));
    }

    m_imageLotView.Overlay_Show(TRUE);
}

void VisionCommonResultDlg::Refresh(VisionUnit& visionUnit)
{
    m_visionUnit = &visionUnit;
    Refresh();
}

void VisionCommonResultDlg::Refresh()
{
    m_pGridCtrl->SetRedraw(FALSE);

    const long nInspResultNum = (long)m_vecpVisionInspectionResult.size();
    const bool bIsUseResultInVisionUnit = (nInspResultNum <= 0);

    auto visionInspectionSpecs = m_visionUnit->GetVisionInspectionSpecs();
    auto visionInspectionResults
        = bIsUseResultInVisionUnit ? m_visionUnit->GetVisionInspectionResults() : m_vecpVisionInspectionResult;

    m_pGridCtrl->SetRowCount(1);

    //--------------------------------------------------------------------------------------
    // Report Category 별로 모듈이 몇개 있는지 카운팅한다
    //--------------------------------------------------------------------------------------

    std::map<long, std::map<CString, BOOL>> inspNameInCategory;
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

    // 값 채우기
    long index = 0;

    for (auto& itSpecList : specListInCategoryAndInspName)
    {
        CString inspName = itSpecList.first.second;
        long category = itSpecList.first.first;
        COLORREF crItemBack = RGB(255, 255, 255);

        if (inspNameInCategory[category].size() > 0)
        {
            // 해당 카테고리를 쓰는 다른 항목이 존재한다.
            // 구분이 되지 않으므로 Spec을 구분하기 쉽게 모듈 이름을 적어준다

            long titleRow = m_pGridCtrl->InsertRow(_T(""));
            m_pGridCtrl->SetItemText(titleRow, 0, inspName);
            m_pGridCtrl->MergeCells(titleRow, 0, titleRow, m_pGridCtrl->GetColumnCount() - 1);
            m_pGridCtrl->SetItemBkColour(titleRow, 0, RGB(120, 120, 220));
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

            // ID
            CString str;
            str.Format(_T("%d"), index + 1);
            m_pGridCtrl->SetItemText(specRow, 0, str);
            m_pGridCtrl->SetItemBkColour(specRow, 0, RGB(250, 250, 220));

            long findResultIndex = -1;
            for (long n = 0; n < long(visionInspectionResults.size()); n++)
            {
                if (visionInspectionResults[n]->m_inspName != inspName)
                    continue;
                if (visionInspectionResults[n]->m_resultName == spec.m_specName)
                {
                    findResultIndex = n;
                    break;
                }
            }

            m_pGridCtrl->SetItemData(specRow, 0, findResultIndex);

            if (findResultIndex >= 0)
            {
                auto& result = *visionInspectionResults[findResultIndex];
                CString strPassSpec;
                CString strMarginalSpec;

                strPassSpec.Format(_T("%.2f ~ %.2f"), spec.m_passMin, spec.m_passMax);
                strMarginalSpec.Format(_T("%.2f ~ %.2f"), spec.m_marginalMin, spec.m_marginalMax);

                m_pGridCtrl->SetItemText(specRow, 1, spec.m_specName);
                m_pGridCtrl->SetItemText(specRow, 2, strPassSpec);

                if (spec.m_useMarginal)
                    m_pGridCtrl->SetItemText(specRow, 3, strMarginalSpec);
                else
                    m_pGridCtrl->SetItemText(specRow, 3, _T(" "));

                // Result
                m_pGridCtrl->SetItemText(specRow, 4, Result2String(result.m_totalResult));
                m_pGridCtrl->SetItemBkColour(specRow, 4, Result2Color(result.m_totalResult));

                // Value
                if (result.getObjectWorstErrorValue() == Ipvm::k_noiseValue32r)
                {
                    str = _T("INV");
                }
                else
                {
                    str.Format(_T("%.2f"), result.getObjectWorstErrorValue());
                }
                m_pGridCtrl->SetItemText(specRow, 5, str);

                m_pGridCtrl->SetItemText(specRow, 6, result.m_itemUnit);

                // Detail
                m_pGridCtrl->SetItemText(specRow, 7, _T(">>"));
            }
            else
            {
                m_pGridCtrl->SetItemText(specRow, 1, spec.m_specName);
                m_pGridCtrl->SetItemText(specRow, 2, _T("Name Error"));
                m_pGridCtrl->SetItemText(specRow, 3, _T("Name Error"));
            }

            index++;
        }
    }

    // Total Result [항상 Grid 맨 마지막에 표시된다]

    long nResult = m_visionUnit->GetInspTotalResult();

    long totalRowIndex = m_pGridCtrl->InsertRow(_T(""));

    CString lastNo;
    lastNo.Format(_T("%d"), index + 1);
    m_pGridCtrl->SetItemBkColour(totalRowIndex, 0, RGB(250, 250, 220));
    m_pGridCtrl->SetItemText(totalRowIndex, 0, lastNo);
    m_pGridCtrl->SetItemText(totalRowIndex, 1, _T("Total Result"));
    m_pGridCtrl->SetItemText(totalRowIndex, 4, Result2String(nResult));
    m_pGridCtrl->SetItemBkColour(totalRowIndex, 4, Result2Color(nResult));
    m_pGridCtrl->SetItemText(totalRowIndex, 5, _T(""));
    m_pGridCtrl->SetItemText(totalRowIndex, 6, _T(""));
    m_pGridCtrl->SetItemText(totalRowIndex, 7, _T(""));
    m_pGridCtrl->SetItemData(totalRowIndex, 0, -1);

    m_pGridCtrl->SetRedraw(TRUE, TRUE);
}

bool VisionCommonResultDlg::CollectVisionInspectionResult()
{
    ClearVisionInspectionResult();
    auto inspectionResults = m_visionUnit->GetVisionInspectionResults();
    const long nInspResultNum = (long)inspectionResults.size();

    if (nInspResultNum <= 0)
        return false;

    m_vecpVisionInspectionResult.resize(nInspResultNum);
    for (long nIdx = 0; nIdx < nInspResultNum; nIdx++)
    {
        m_vecpVisionInspectionResult[nIdx] = new VisionInspectionResult(*inspectionResults[nIdx]);
    }

    return true;
}

void VisionCommonResultDlg::ClearVisionInspectionResult()
{
    const long nInspResultNum = (long)m_vecpVisionInspectionResult.size();
    if (nInspResultNum < 0)
        return;

    for (long nIdx = 0; nIdx < nInspResultNum; nIdx++)
        delete m_vecpVisionInspectionResult[nIdx];

    m_vecpVisionInspectionResult.clear();
}