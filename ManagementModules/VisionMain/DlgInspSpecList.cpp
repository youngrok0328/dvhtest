//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgInspSpecList.h"

//CPP_2_________________________________ This project's headers
#include "InspectionSpecConfig.h"
#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
static LPCTSTR g_SpecName[] = {
    _T("Use"),
    _T("Name"),
    _T("Min."),
    _T(" "),
    _T("Max."),
    _T(" "),
    _T("Use"),
    _T(" "),
    _T(" "),
    _T(" "),
    _T(" "),
    _T("Margin"),
    _T("Pass"),
    _T("Pass"),
    _T("Margin"),
    _T("Min."),
    _T("Max."),
    _T("Margin"),
};

enum SpecMenuList
{
    enum_Start = 0,
    enum_Use = enum_Start,
    enum_Name,
    enum_MarginalMin,
    enum_PassMin,
    enum_PassMax,
    enum_MarginalMax,
    enum_UseMin,
    enum_UseMax,
    enum_UseMarginal,
    enum_End,
};

IMPLEMENT_DYNAMIC(CDlgInspSpecList, CDialog)

CDlgInspSpecList::CDlgInspSpecList(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgInspSpecList::IDD, pParent)
{
    m_pInspectionSpecINI = new CInspectionSpecConfig;
}

CDlgInspSpecList::~CDlgInspSpecList()
{
    delete m_pInspectionSpecINI;
}

void CDlgInspSpecList::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_GRID_LIST_SPEC, m_gridSpecList);
}

BEGIN_MESSAGE_MAP(CDlgInspSpecList, CDialog)
ON_BN_CLICKED(IDOK, &CDlgInspSpecList::OnBnClickedOk)
ON_BN_CLICKED(IDC_BTN_LOAD_DEFAULT, &CDlgInspSpecList::OnBnClickedBtnLoadDefault)
ON_BN_CLICKED(IDC_BTN_SET_DEFAULT, &CDlgInspSpecList::OnBnClickedBtnSetDefault)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_LIST_SPEC, &CDlgInspSpecList::OnGridDataChanged)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_LIST_SPEC, &CDlgInspSpecList::OnGridDataChanged)
END_MESSAGE_MAP()

// CDlgInspSpecList 메시지 처리기입니다.

BOOL CDlgInspSpecList::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_gridSpecList.DeleteAllItems();

    /*영훈 20130807 : Grid 초기화*/
    m_gridSpecList.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridSpecList.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridSpecList.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridSpecList.SetDefCellMargin(0);
    m_gridSpecList.SetFixedColumnSelection(FALSE);
    m_gridSpecList.SetFixedRowSelection(FALSE);
    m_gridSpecList.SetSingleColSelection(FALSE);
    m_gridSpecList.SetSingleRowSelection(FALSE);
    m_gridSpecList.SetTrackFocusCell(FALSE);
    m_gridSpecList.SetRowResize(FALSE);
    m_gridSpecList.SetColumnResize(FALSE);

    CRect rtROI;
    GetClientRect(&rtROI);
    rtROI.left += 10;
    rtROI.top += 10;
    rtROI.right -= 10;
    rtROI.bottom -= 50;
    m_gridSpecList.MoveWindow(rtROI);

    SetGrid();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgInspSpecList::SetData(std::vector<VisionInspectionSpec> vecstSpecData)
{
    m_vecstSpecData = vecstSpecData;

    //	SetGrid();
}

std::vector<VisionInspectionSpec> CDlgInspSpecList::GetData()
{
    return m_vecstSpecData;
}

void CDlgInspSpecList::SetGrid()
{
    //m_gridSpecList.DeleteAllItems();

    long nHeaderCnt = 2;
    // Items + Header
    long nDataNum = (long)m_vecstSpecData.size();
    m_gridSpecList.SetRowCount(nDataNum + nHeaderCnt);
    m_gridSpecList.SetFixedRowCount(nHeaderCnt);
    m_gridSpecList.SetColumnCount(enum_End);
    m_gridSpecList.SetEditable(TRUE);

    m_gridSpecList.GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridSpecList.GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridSpecList.GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    long nCheck = 30;
    long nName = 130;
    long nValue = 49;

    m_gridSpecList.SetColumnWidth(enum_Use, nCheck);
    m_gridSpecList.SetColumnWidth(enum_Name, nName);
    m_gridSpecList.SetColumnWidth(enum_MarginalMin, nValue);
    m_gridSpecList.SetColumnWidth(enum_PassMin, nValue);
    m_gridSpecList.SetColumnWidth(enum_PassMax, nValue);
    m_gridSpecList.SetColumnWidth(enum_MarginalMax, nValue);
    m_gridSpecList.SetColumnWidth(enum_UseMin, nCheck);
    m_gridSpecList.SetColumnWidth(enum_UseMax, nCheck);
    m_gridSpecList.SetColumnWidth(enum_UseMarginal, nCheck + 15);

    for (long nRow = 0; nRow < nHeaderCnt; nRow++)
    {
        long nIdxB = enum_End * nRow;

        for (long nCol = enum_Start; nCol < enum_End; nCol++)
        {
            long nIdx = nIdxB + nCol;

            m_gridSpecList.SetItemBkColour(nRow, nCol, RGB(150, 150, 200));
            m_gridSpecList.SetItemState(nRow, nCol, GVIS_READONLY);
            m_gridSpecList.GetCell(nRow, nCol)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            m_gridSpecList.SetItemText(nRow, nCol, g_SpecName[nIdx]);
        }
    }

    for (long nRow = nHeaderCnt; nRow < nDataNum + nHeaderCnt; nRow++)
    {
        m_gridSpecList.SetCellType(nRow, enum_Use, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_Use))->SetCheck(m_vecstSpecData[nRow - nHeaderCnt].m_use);
        m_gridSpecList.GetCell(nRow, enum_Use)->SetFormat(DT_CENTER);

        m_gridSpecList.SetItemState(nRow, enum_Name, GVIS_READONLY);
        m_gridSpecList.SetItemText(nRow, enum_Name, m_vecstSpecData[nRow - nHeaderCnt].m_specName);
        m_gridSpecList.GetCell(nRow, enum_Name)->SetFormat(DT_CENTER);
        m_gridSpecList.SetItemTextFmt(
            nRow, enum_MarginalMin, _T("%.2f"), m_vecstSpecData[nRow - nHeaderCnt].m_marginalMin);
        m_gridSpecList.GetCell(nRow, enum_MarginalMin)->SetFormat(DT_CENTER);
        m_gridSpecList.SetItemTextFmt(nRow, enum_PassMin, _T("%.2f"), m_vecstSpecData[nRow - nHeaderCnt].m_passMin);
        m_gridSpecList.GetCell(nRow, enum_PassMin)->SetFormat(DT_CENTER);
        m_gridSpecList.SetItemTextFmt(nRow, enum_PassMax, _T("%.2f"), m_vecstSpecData[nRow - nHeaderCnt].m_passMax);
        m_gridSpecList.GetCell(nRow, enum_PassMax)->SetFormat(DT_CENTER);
        m_gridSpecList.SetItemTextFmt(
            nRow, enum_MarginalMax, _T("%.2f"), m_vecstSpecData[nRow - nHeaderCnt].m_marginalMax);
        m_gridSpecList.GetCell(nRow, enum_MarginalMax)->SetFormat(DT_CENTER);

        m_gridSpecList.SetCellType(nRow, enum_UseMin, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_UseMin))
            ->SetCheck(m_vecstSpecData[nRow - nHeaderCnt].m_useMin);
        m_gridSpecList.GetCell(nRow, enum_UseMin)->SetFormat(DT_CENTER);
        m_gridSpecList.SetCellType(nRow, enum_UseMax, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_UseMax))
            ->SetCheck(m_vecstSpecData[nRow - nHeaderCnt].m_useMax);
        m_gridSpecList.GetCell(nRow, enum_UseMax)->SetFormat(DT_CENTER);
        m_gridSpecList.SetCellType(nRow, enum_UseMarginal, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_UseMarginal))
            ->SetCheck(m_vecstSpecData[nRow - nHeaderCnt].m_useMarginal);
        m_gridSpecList.GetCell(nRow, enum_UseMarginal)->SetFormat(DT_CENTER);

        BOOL isUseSpec = m_vecstSpecData[nRow - nHeaderCnt].m_use;
        if (isUseSpec == TRUE)
        {
            m_gridSpecList.SetItemState(nRow, enum_MarginalMin, GVIS_MODIFIED);
            m_gridSpecList.SetItemBkColour(nRow, enum_MarginalMin, RGB(255, 255, 255));

            m_gridSpecList.SetItemState(nRow, enum_PassMin, GVIS_MODIFIED);
            m_gridSpecList.SetItemBkColour(nRow, enum_PassMin, RGB(255, 255, 255));

            m_gridSpecList.SetItemState(nRow, enum_PassMax, GVIS_MODIFIED);
            m_gridSpecList.SetItemBkColour(nRow, enum_PassMax, RGB(255, 255, 255));

            m_gridSpecList.SetItemState(nRow, enum_MarginalMax, GVIS_MODIFIED);
            m_gridSpecList.SetItemBkColour(nRow, enum_MarginalMax, RGB(255, 255, 255));

            m_gridSpecList.SetItemState(nRow, enum_UseMin, GVIS_MODIFIED);
            m_gridSpecList.SetItemBkColour(nRow, enum_UseMin, RGB(255, 255, 255));

            m_gridSpecList.SetItemState(nRow, enum_UseMax, GVIS_MODIFIED);
            m_gridSpecList.SetItemBkColour(nRow, enum_UseMax, RGB(255, 255, 255));

            m_gridSpecList.SetItemState(nRow, enum_UseMarginal, GVIS_MODIFIED);
            m_gridSpecList.SetItemBkColour(nRow, enum_UseMarginal, RGB(255, 255, 255));

            BOOL isUseMinSpec = m_vecstSpecData[nRow - nHeaderCnt].m_useMin;
            if (isUseMinSpec == FALSE)
            {
                m_gridSpecList.SetItemState(nRow, enum_PassMin, GVIS_READONLY);
                m_gridSpecList.SetItemBkColour(nRow, enum_PassMin, RGB(134, 135, 137));
            }

            BOOL isUseMaxSpec = m_vecstSpecData[nRow - nHeaderCnt].m_useMax;
            if (isUseMaxSpec == FALSE)
            {
                m_gridSpecList.SetItemState(nRow, enum_PassMax, GVIS_READONLY);
                m_gridSpecList.SetItemBkColour(nRow, enum_PassMax, RGB(134, 135, 137));
            }

            BOOL isUseMarginalSpec = m_vecstSpecData[nRow - nHeaderCnt].m_useMarginal;
            if (isUseMarginalSpec == FALSE)
            {
                m_gridSpecList.SetItemState(nRow, enum_MarginalMin, GVIS_READONLY);
                m_gridSpecList.SetItemBkColour(nRow, enum_MarginalMin, RGB(134, 135, 137));

                m_gridSpecList.SetItemState(nRow, enum_MarginalMax, GVIS_READONLY);
                m_gridSpecList.SetItemBkColour(nRow, enum_MarginalMax, RGB(134, 135, 137));
            }
        }
        else
        {
            m_gridSpecList.SetItemState(nRow, enum_MarginalMin, GVIS_READONLY);
            m_gridSpecList.SetItemBkColour(nRow, enum_MarginalMin, RGB(134, 135, 137));

            m_gridSpecList.SetItemState(nRow, enum_PassMin, GVIS_READONLY);
            m_gridSpecList.SetItemBkColour(nRow, enum_PassMin, RGB(134, 135, 137));

            m_gridSpecList.SetItemState(nRow, enum_PassMax, GVIS_READONLY);
            m_gridSpecList.SetItemBkColour(nRow, enum_PassMax, RGB(134, 135, 137));

            m_gridSpecList.SetItemState(nRow, enum_MarginalMax, GVIS_READONLY);
            m_gridSpecList.SetItemBkColour(nRow, enum_MarginalMax, RGB(134, 135, 137));

            m_gridSpecList.SetItemState(nRow, enum_UseMin, GVIS_READONLY);
            m_gridSpecList.SetItemBkColour(nRow, enum_UseMin, RGB(134, 135, 137));

            m_gridSpecList.SetItemState(nRow, enum_UseMax, GVIS_READONLY);
            m_gridSpecList.SetItemBkColour(nRow, enum_UseMax, RGB(134, 135, 137));

            m_gridSpecList.SetItemState(nRow, enum_UseMarginal, GVIS_READONLY);
            m_gridSpecList.SetItemBkColour(nRow, enum_UseMarginal, RGB(134, 135, 137));
        }
    }

    m_gridSpecList.MergeCells(0, enum_Use, 1, enum_Use);
    m_gridSpecList.MergeCells(0, enum_Name, 1, enum_Name);
    m_gridSpecList.MergeCells(0, enum_MarginalMin, 0, enum_PassMin);
    m_gridSpecList.MergeCells(0, enum_PassMax, 0, enum_MarginalMax);
    m_gridSpecList.MergeCells(0, enum_UseMin, 0, enum_UseMarginal);

    m_gridSpecList.Refresh();
}

void CDlgInspSpecList::OnBnClickedOk()
{
    GetGridData();

    OnOK();
}

void CDlgInspSpecList::GetGridData()
{
    long nDataNum = (long)m_vecstSpecData.size();

    for (long nRow = 2; nRow < nDataNum + 2; nRow++)
    {
        m_vecstSpecData[nRow - 2].m_use = ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_Use))->GetCheck();
        m_vecstSpecData[nRow - 2].m_specName = m_gridSpecList.GetItemText(nRow, enum_Name);
        m_vecstSpecData[nRow - 2].m_marginalMin = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_MarginalMin));
        m_vecstSpecData[nRow - 2].m_passMin = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_PassMin));
        m_vecstSpecData[nRow - 2].m_passMax = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_PassMax));
        m_vecstSpecData[nRow - 2].m_marginalMax = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_MarginalMax));
        m_vecstSpecData[nRow - 2].m_useMin = ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_UseMin))->GetCheck();
        m_vecstSpecData[nRow - 2].m_useMax = ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_UseMax))->GetCheck();
        m_vecstSpecData[nRow - 2].m_useMarginal
            = ((CGridCellCheck2*)m_gridSpecList.GetCell(nRow, enum_UseMarginal))->GetCheck();

        if (m_vecstSpecData[nRow - 2].m_marginalMin > m_vecstSpecData[nRow - 2].m_passMin
            || !m_vecstSpecData[nRow - 2].m_useMarginal)
            m_vecstSpecData[nRow - 2].m_marginalMin = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_PassMin));

        m_vecstSpecData[nRow - 2].m_marginalMin = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_MarginalMin));

        if (m_vecstSpecData[nRow - 2].m_marginalMax < m_vecstSpecData[nRow - 2].m_passMax
            || !m_vecstSpecData[nRow - 2].m_useMarginal)
            m_vecstSpecData[nRow - 2].m_marginalMax = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_PassMax));
        else
            m_vecstSpecData[nRow - 2].m_marginalMax = (float)_ttof(m_gridSpecList.GetItemText(nRow, enum_MarginalMax));
    }
}

void CDlgInspSpecList::OnBnClickedBtnLoadDefault()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (SimpleMessage(_T("Press the OK button to load the default parameters."), MB_OKCANCEL) == IDOK)
    {
        m_pInspectionSpecINI->LoadIni(m_vecstSpecData);

        SetGrid();
    }
}

void CDlgInspSpecList::OnBnClickedBtnSetDefault()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (SimpleMessage(_T("Press the OK button to save the default parameters."), MB_OKCANCEL) == IDOK)
    {
        GetGridData();

        m_pInspectionSpecINI->SaveIni(m_vecstSpecData);

        SimpleMessage(_T("Complete the save."), MB_OK);
    }
}

void CDlgInspSpecList::OnGridDataChanged(NMHDR* pNotifyStruct, LRESULT* result)
{
    UNREFERENCED_PARAMETER(pNotifyStruct);
    UNREFERENCED_PARAMETER(result);

    GetGridData();

    SetGrid();
}