//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgRnRHelper.h"

//CPP_2_________________________________ This project's headers
#include "DlgRnRReport.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_SPEC 100
#define IDC_GRID_RNR 101

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgRnRHelper, CDialog)

CDlgRnRHelper::CDlgRnRHelper(VisionUnit& visionUnit, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgRnRHelper::IDD, pParent)
    , m_visionUnit(visionUnit)
    , m_pgridSpec(new CGridCtrl)
    , m_Grid_RnR(new CGridCtrl)
{
    m_pgridSpec = NULL;
    m_nCurrentPackageCnt = 0;
    m_nCurrentResultCnt = 0;
    m_nSelectResult = 0;
}

CDlgRnRHelper::~CDlgRnRHelper()
{
    delete m_pgridSpec;
    delete m_Grid_RnR;
}

void CDlgRnRHelper::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_CUSTOM_GRID_RNR, *m_Grid_RnR);
}

BEGIN_MESSAGE_MAP(CDlgRnRHelper, CDialog)
ON_BN_CLICKED(IDC_BUTTON_START, &CDlgRnRHelper::OnBnClickedButtonStart)
ON_BN_CLICKED(IDC_BUTTON_END, &CDlgRnRHelper::OnBnClickedButtonEnd)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_SPEC, OnGridEditEnd)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_SPEC, OnGridEditEnd)
ON_BN_CLICKED(IDC_BUTTON_REPORT, &CDlgRnRHelper::OnBnClickedButtonReport)
END_MESSAGE_MAP()

// CDlgRnRHelper 메시지 처리기입니다.
void CDlgRnRHelper::OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;
    long col = pNotify->iColumn;

    if (col == 5)
    {
        for (long n = 0; n < m_vecbUserRnRData.size(); n++)
        {
            m_vecbUserRnRData[n] = FALSE;
        }

        m_vecbUserRnRData[row] = TRUE;
        m_nSelectResult = row;

        auto visionInspectionSpecs = m_visionUnit.GetVisionInspectionSpecs();
        ;

        long nSpecNum = (long)(visionInspectionSpecs.size());
        long i;
        CString str;

        for (i = 0; i < nSpecNum; i++)
        {
            m_pgridSpec->SetCellType(i + 1, 5, RUNTIME_CLASS(CGridCellCheck2));
            ((CGridCellCheck2*)m_pgridSpec->GetCell(i + 1, 5))->SetCheck(m_vecbUserRnRData[i]);
        }
        m_pgridSpec->Refresh();
        UpdateSpecData();
        RefreshRnRGrid();
    }

    *result = 0;
}

BOOL CDlgRnRHelper::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_nRepeatCount = 9;
    m_nPackageCount = 10;
    m_pvecvecvecResult.resize(m_nPackageCount);
    for (long n = 0; n < m_nPackageCount; n++)
    {
        m_pvecvecvecResult[n].resize(m_nRepeatCount);
    }
    SetDlgItemInt(IDC_EDIT_REPEATCNT, m_nRepeatCount);
    SetDlgItemInt(IDC_EDIT_PACKAGECNT, m_nPackageCount);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgRnRHelper::SetInitDialog()
{
    CRect rect;
    this->GetClientRect(rect);
    rect.top = rect.top + 150;
    rect.bottom = rect.top + 200;

    // 영훈 : 이걸 사용하는 모든 부분에서 Create 시 이미 생성됐는지 확인하는 예외처리를 추가한다. 메모리 누수 및 UI 버그를 유발한다.
    if (m_pgridSpec->GetSafeHwnd() == NULL)
        m_pgridSpec->Create(rect, this, IDC_GRID_SPEC);

    SetInitGrid();

    this->GetClientRect(rect);
    rect.top = rect.top + 150 + 200 + 3;
    GetDlgItem(IDC_CUSTOM_GRID_RNR)->MoveWindow(rect);
    SetInitRnRGrid();
}

void CDlgRnRHelper::SetInitRnRGrid()
{
    m_Grid_RnR->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_Grid_RnR->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_Grid_RnR->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_Grid_RnR->SetColumnCount(m_nRepeatCount + 1);
    m_Grid_RnR->SetRowCount(m_nPackageCount + 1);
    m_Grid_RnR->SetFixedRowCount(1);
    m_Grid_RnR->SetFixedColumnCount(1);

    m_Grid_RnR->SetGridLineColor(RGB(0, 0, 0));
    m_Grid_RnR->SetSingleColSelection(FALSE);
    m_Grid_RnR->SetSingleRowSelection(FALSE);
    m_Grid_RnR->SetRowResize(FALSE);
    m_Grid_RnR->SetColumnResize(FALSE);

    m_Grid_RnR->SetDefCellMargin(0);

    for (long nCol = 0; nCol < m_nRepeatCount + 1; nCol++)
        m_Grid_RnR->SetColumnWidth(nCol, 50);

    CString str;
    for (long nRow = 1; nRow < m_nPackageCount + 1; nRow++)
    {
        str.Format(_T("#%3d"), nRow);
        m_Grid_RnR->SetItemText(nRow, 0, str);
    }
}

void CDlgRnRHelper::RefreshRnRGrid()
{
    CString str;
    for (long nPackageNum = 0; nPackageNum < m_nPackageCount; nPackageNum++)
    {
        for (long nRepeatNum = 0; nRepeatNum < m_nRepeatCount; nRepeatNum++)
        {
            if (m_pvecvecvecResult[nPackageNum][nRepeatNum].size())
            {
                if (m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult].m_objectErrorValues.size())
                {
                    float fWorst(0.f);
                    long nWorstID(-1);
                    CString strText;

                    long nSize = (long)(m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                            .m_objectErrorValues.size());
                    if (nSize > 0)
                    {
                        for (long i = 0; i < nSize; i++)
                        {
                            if (fabs(fWorst) <= fabs(m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                                        .m_objectErrorValues[i]))
                            {
                                fWorst = m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                                             .m_objectErrorValues[i];
                                nWorstID = i;
                            }
                        }
                    }

                    float fVal(0.f);
                    if (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_2D_INSP
                        || m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_TR)
                        fVal = m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult].m_objectErrorValues[0];
                    else if (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_3D_INSP)
                        fVal = m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                                   .m_objectErrorValues[nWorstID];
                    str.Format(_T("%0.2f"), fVal);

                    m_Grid_RnR->SetItemText(nPackageNum + 1, nRepeatNum + 1, str);
                }
            }
        }
    }

    m_Grid_RnR->Refresh();
    m_Grid_RnR->SetFocus();
}

void CDlgRnRHelper::SetInitGrid()
{
    m_pgridSpec->SetEditable(TRUE);
    m_pgridSpec->EnableDragAndDrop(FALSE);

    // Grid 가로 세로 개수 설정
    auto visionInspectionSpecs = m_visionUnit.GetVisionInspectionSpecs();
    ;

    long nSpecNum = (long)(visionInspectionSpecs.size());
    m_pgridSpec->SetColumnCount(6);
    m_pgridSpec->SetRowCount(nSpecNum + 1);
    m_pgridSpec->SetFixedRowCount(1);
    m_pgridSpec->SetFixedColumnCount(1);
    m_pgridSpec->SetEditable(TRUE);
    m_vecbUserRnRData.resize(nSpecNum);

    if (nSpecNum > 0)
    {
        m_vecbUserRnRData[0] = TRUE;
    }

    long i;
    CString str;
    for (i = 0; i < nSpecNum; i++)
    {
        m_pgridSpec->SetItemBkColour(i + 1, 0, RGB(250, 250, 220));
        // ID
        str.Format(_T("%d"), i);
        m_pgridSpec->SetItemText(i + 1, 0, str);

        // Name
        str = _T("[") + visionInspectionSpecs[i]->m_inspName + _T("]  ") + visionInspectionSpecs[i]->m_specName;
        m_pgridSpec->SetItemText(i + 1, 1, str /*(*(*m_pvecSpec)[i]).m_specName*/);
        m_pgridSpec->SetItemState(i + 1, 1, m_pgridSpec->GetItemState(i + 1, 1) | GVIS_READONLY); // SpecName은 읽기전용

        // Use
        if (visionInspectionSpecs[i]->m_use)
        {
            m_pgridSpec->SetRowHeight(i + 1, 20);
        }
        else
        {
            m_pgridSpec->SetRowHeight(i + 1, 0);
        }

        // Min
        str.Format(_T("%.2f"), visionInspectionSpecs[i]->m_passMin);
        m_pgridSpec->SetItemText(i + 1, 2, str);

        // Max
        str.Format(_T("%.2f"), visionInspectionSpecs[i]->m_passMax);
        m_pgridSpec->SetItemText(i + 1, 3, str);

        // Unit
        str = _T("[") + visionInspectionSpecs[i]->m_unit + _T("]");
        m_pgridSpec->SetItemText(i + 1, 4, str);

        // Use Min
        m_pgridSpec->SetCellType(i + 1, 5, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_pgridSpec->GetCell(i + 1, 5))->SetCheck(m_vecbUserRnRData[i]);

        // Use Max
        m_pgridSpec->SetItemState(i + 1, 2, m_pgridSpec->GetItemState(i + 1, 2) | GVIS_READONLY); // SpecName은 읽기전용
        m_pgridSpec->SetItemState(i + 1, 3, m_pgridSpec->GetItemState(i + 1, 3) | GVIS_READONLY); // SpecName은 읽기전용
        m_pgridSpec->SetItemState(i + 1, 4, m_pgridSpec->GetItemState(i + 1, 4) | GVIS_READONLY); // SpecName은 읽기전용
    }

    // 0 번째 Row 에 각각의 Title 을 정의
    m_pgridSpec->SetItemText(0, 0, _T("No"));
    m_pgridSpec->SetItemText(0, 1, _T("Name"));
    m_pgridSpec->SetItemText(0, 2, _T("Min"));
    m_pgridSpec->SetItemText(0, 3, _T("Max"));
    m_pgridSpec->SetItemText(0, 4, _T("Unit"));
    m_pgridSpec->SetItemText(0, 5, _T("Use RnR"));

    m_pgridSpec->SetItemBkColour(0, 0, RGB(200, 200, 250));
    m_pgridSpec->SetItemBkColour(0, 1, RGB(200, 200, 250));
    m_pgridSpec->SetItemBkColour(0, 2, RGB(200, 200, 250));
    m_pgridSpec->SetItemBkColour(0, 3, RGB(200, 200, 250));
    m_pgridSpec->SetItemBkColour(0, 4, RGB(200, 200, 250));
    m_pgridSpec->SetItemBkColour(0, 5, RGB(200, 200, 250));

    m_pgridSpec->SetColumnWidth(0, 30);
    m_pgridSpec->SetColumnWidth(1, 200);
    m_pgridSpec->SetColumnWidth(2, 70);
    m_pgridSpec->SetColumnWidth(3, 70);
    m_pgridSpec->SetColumnWidth(4, 40);
    m_pgridSpec->SetColumnWidth(5, 60);

    m_pgridSpec->Refresh();
}

void CDlgRnRHelper::UpdateSpecData()
{
    auto visionInspectionSpecs = m_visionUnit.GetVisionInspectionSpecs();

    long nSpecNum = (long)(visionInspectionSpecs.size());
    long i;
    CString str;

    for (i = 0; i < nSpecNum; i++)
    {
        m_vecbUserRnRData[i] = ((CGridCellCheck2*)m_pgridSpec->GetCell(i + 1, 5))->GetCheck();
    }
}

void CDlgRnRHelper::OnBnClickedButtonStart()
{
    UpdateSpecData();

    m_pvecvecvecResult.clear();

    CString str;
    GetDlgItemText(IDC_EDIT_REPEATCNT, str);
    m_nRepeatCount = _ttoi(str);

    GetDlgItemText(IDC_EDIT_PACKAGECNT, str);
    m_nPackageCount = _ttoi(str);

    m_pvecvecvecResult.resize(m_nPackageCount);
    for (long n = 0; n < m_nPackageCount; n++)
    {
        m_pvecvecvecResult[n].resize(m_nRepeatCount);
    }

    m_nCurrentResultCnt = 0;
    m_nCurrentPackageCnt = 0;
    SetInitRnRGrid();
}

void CDlgRnRHelper::SetResultData(long nRepeatNum, long nPackageNum, std::vector<VisionInspectionResult*>* pResult)
{
    if (nRepeatNum == -1 || nPackageNum == -1)
    {
        nPackageNum = m_nCurrentPackageCnt;
        nRepeatNum = m_nCurrentResultCnt;
        m_nCurrentPackageCnt++;
        if (m_nCurrentPackageCnt >= m_nPackageCount)
        {
            m_nCurrentResultCnt++;
            m_nCurrentPackageCnt = 0;
        }
    }
    else if (nRepeatNum == -1)
    {
        if (nPackageNum == m_nPackageCount - 1)
        {
            m_nCurrentResultCnt++;
        }
    }
    if (nRepeatNum < m_nRepeatCount && nPackageNum < m_nPackageCount)
    {
        m_pvecvecvecResult[nPackageNum][nRepeatNum].resize((long)pResult->size(),
            VisionInspectionResult(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                II_RESULT_TYPE::MEASURED, HostReportCategory::MEASURE));
        for (long n = 0; n < (long)pResult->size(); n++)
        {
            m_pvecvecvecResult[nPackageNum][nRepeatNum][n] = *(*pResult)[n];
        }
    }

    RefreshRnRGrid();
}

void CDlgRnRHelper::OnBnClickedButtonEnd()
{
    RefreshRnRGrid();
}

void CDlgRnRHelper::OnBnClickedButtonReport()
{
    CDlgRnRReport dlg;

    dlg.m_gridRnR.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    dlg.m_gridRnR.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    dlg.m_gridRnR.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    dlg.m_gridRnR.SetColumnCount(m_nRepeatCount + 1);
    dlg.m_gridRnR.SetRowCount(m_nPackageCount + 1);
    dlg.m_gridRnR.SetFixedRowCount(1);
    dlg.m_gridRnR.SetFixedColumnCount(1);
    dlg.m_gridRnR.SetGridLineColor(RGB(0, 0, 0));
    dlg.m_gridRnR.SetSingleColSelection(FALSE);
    dlg.m_gridRnR.SetSingleRowSelection(FALSE);
    dlg.m_gridRnR.SetRowResize(FALSE);
    dlg.m_gridRnR.SetColumnResize(FALSE);
    dlg.m_gridRnR.SetDefCellMargin(0);

    for (long nCol = 0; nCol < m_nRepeatCount + 1; nCol++)
        dlg.m_gridRnR.SetColumnWidth(nCol, 50);

    CString str;
    for (long nRow = 1; nRow < m_nPackageCount + 1; nRow++)
    {
        str.Format(_T("#%3d"), nRow);
        dlg.m_gridRnR.SetItemText(nRow, 0, str);
    }

    for (long nPackageNum = 0; nPackageNum < m_nPackageCount; nPackageNum++)
    {
        for (long nRepeatNum = 0; nRepeatNum < m_nRepeatCount; nRepeatNum++)
        {
            if (m_pvecvecvecResult[nPackageNum][nRepeatNum].size())
            {
                if (m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult].m_objectErrorValues.size())
                {
                    float fWorst(0.f);
                    long nWorstID(-1);
                    CString strText;

                    long nSize = (long)(m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                            .m_objectErrorValues.size());
                    if (nSize > 0)
                    {
                        for (long i = 0; i < nSize; i++)
                        {
                            if (fabs(fWorst) <= fabs(m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                                        .m_objectErrorValues[i]))
                            {
                                fWorst = m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                                             .m_objectErrorValues[i];
                                nWorstID = i;
                            }
                        }
                    }

                    float fVal(0.f);
                    if (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_2D_INSP
                        || m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_TR)
                        fVal = m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult].m_objectErrorValues[0];
                    else if (m_visionUnit.m_systemConfig.GetVisionType() == VISIONTYPE_3D_INSP)
                        fVal = m_pvecvecvecResult[nPackageNum][nRepeatNum][m_nSelectResult]
                                   .m_objectErrorValues[nWorstID];
                    str.Format(_T("%0.2f"), fVal);

                    dlg.m_gridRnR.SetItemText(nPackageNum + 1, nRepeatNum + 1, str);
                }
            }
        }
    }

    dlg.m_gridRnR.Refresh();
    dlg.DoModal();
}
