//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspSpec.h"

//CPP_2_________________________________ This project's headers
#include "DlgInspSpecList.h"
#include "MainDefine.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../InspectionItems/VisionInspectionGeometry/VisionInspectionGeometry.h"
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMark.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurface.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurfacePara.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCellCheck2.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_SM 100
#define IDC_GRID_SI 101
#define IDC_GRID_MARK 102
#define IDC_GRID_GEOMETRY 103
#define IDC_GRID_INSPITEM 104

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgVisionInspSpec, CDialog)

CDlgVisionInspSpec::CDlgVisionInspSpec(VisionUnit* visionUnit, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspSpec::IDD, pParent)
    , m_gridVisionSM(nullptr)
    , m_gridVisionSI(nullptr)
    , m_gridVisionMark(nullptr)
    , m_gridVisionGeometry(nullptr)
    , m_gridVisionFixed(nullptr)
{
    m_visionUnit = visionUnit;

    m_visionUnit->GetAvailableFixedItemList(m_availableFixedItems);

    //--------------------------------------------------------------------------
    // 설정전 Job을 백업한다
    //--------------------------------------------------------------------------

    ArchiveAllType ar(&m_dbBackup, ArchiveAllType::store);
    CiDataBase db;

    m_visionUnit->LinkDataBase(LAST_JOB_VERSION, TRUE, db);
    db.Serialize(ar);
}

CDlgVisionInspSpec::~CDlgVisionInspSpec()
{
    delete m_gridVisionSM;
    delete m_gridVisionSI;
    delete m_gridVisionMark;
    delete m_gridVisionGeometry;
    delete m_gridVisionFixed;
}

void CDlgVisionInspSpec::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspSpec, CDialog)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_SM, OnGridEditEnd_SM)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_SM, OnGridEditEnd_SM)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_SI, OnGridEditEnd_SI)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_SI, OnGridEditEnd_SI)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_MARK, OnGridEditEnd_Mark)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_MARK, OnGridEditEnd_Mark)
ON_NOTIFY(NM_DBLCLK, IDC_GRID_MARK, OnGridDbClick_Mark)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_INSPITEM, OnGridEditEnd_InspItem)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_INSPITEM, OnGridEditEnd_InspItem)
ON_NOTIFY(GVN_ENDLABELEDIT, IDC_GRID_GEOMETRY, OnGridEditEnd_Geometry)
ON_NOTIFY(GVN_STATECHANGED, IDC_GRID_GEOMETRY, OnGridEditEnd_Geometry)
ON_NOTIFY(NM_DBLCLK, IDC_GRID_INSPITEM, OnGridDbClick_InspItem)
ON_BN_CLICKED(IDC_BUTTON_SM_ADD, &CDlgVisionInspSpec::OnBnClickedButtonSmAdd)
ON_BN_CLICKED(IDC_BUTTON_SM_DELETE, &CDlgVisionInspSpec::OnBnClickedButtonSmDelete)
ON_BN_CLICKED(IDC_BUTTON_SM_EXPORT, &CDlgVisionInspSpec::OnBnClickedButtonSmExport)
ON_BN_CLICKED(IDC_BUTTON_SM_IMPORT, &CDlgVisionInspSpec::OnBnClickedButtonSmImport)

ON_BN_CLICKED(IDC_BUTTON_SI_ADD, &CDlgVisionInspSpec::OnBnClickedButtonSiAdd)
ON_BN_CLICKED(IDC_BUTTON_SI_DELETE, &CDlgVisionInspSpec::OnBnClickedButtonSiDelete)
ON_BN_CLICKED(IDC_BUTTON_SI_EXPORT, &CDlgVisionInspSpec::OnBnClickedButtonSiExport)
ON_BN_CLICKED(IDC_BUTTON_SI_IMPORT, &CDlgVisionInspSpec::OnBnClickedButtonSiImport)

ON_BN_CLICKED(IDC_BUTTON_MARK_ADD, &CDlgVisionInspSpec::OnBnClickedButtonMarkAdd)
ON_BN_CLICKED(IDC_BUTTON_MARK_DELETE, &CDlgVisionInspSpec::OnBnClickedButtonMarkDelete)
ON_BN_CLICKED(IDC_BTN_GEO_DEL, &CDlgVisionInspSpec::OnBnClickedBtnGeoDel)
ON_BN_CLICKED(IDC_BTN_GEO_ADD, &CDlgVisionInspSpec::OnBnClickedBtnGeoAdd)
ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDlgVisionInspSpec::OnBnClickedButtonApply)
ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgVisionInspSpec::OnBnClickedButtonCancel)
END_MESSAGE_MAP()

// CDlgVisionInspSpec 메시지 처리기입니다.
BOOL CDlgVisionInspSpec::OnInitDialog()
{
    CDialog::OnInitDialog();

    static const BOOL bIsNotSideVision = SystemConfig::GetInstance().IsVisionTypeSide() == FALSE;
    static const bool bIsVision2D = SystemConfig::GetInstance().IsVisionType2D() == TRUE;
    static const bool bIsVisionSWIR = SystemConfig::GetInstance().IsVisionTypeSWIR() == TRUE; //kircheis_SWIR
    static const bool isMarkInspectionEnabled = (bIsNotSideVision == TRUE && bIsVisionSWIR == false); //kircheis_SWIR

    m_gridVisionSM = new CGridCtrl;
    m_gridVisionSI = new CGridCtrl;
    m_gridVisionMark = new CGridCtrl;
    m_gridVisionGeometry = new CGridCtrl;
    m_gridVisionFixed = new CGridCtrl;

    CRect rect;
    GetClientRect(rect);

    CString strFontName = _T("Arial");
    CFont fontGrid;
    fontGrid.CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, strFontName);

    CRect rtSM;
    CRect rtSI;
    CRect rtMark;
    CRect rtGeometry;
    CRect rtVisionItem;

    GetDlgItem(IDC_REGION_SM)->GetWindowRect(rtSM);
    GetDlgItem(IDC_REGION_SI)->GetWindowRect(rtSI);
    GetDlgItem(IDC_REGION_MARK)->GetWindowRect(rtMark);
    GetDlgItem(IDC_REGION_GEOMETRY)->GetWindowRect(rtGeometry);
    GetDlgItem(IDC_REGION_VISIONITEM)->GetWindowRect(rtVisionItem);
    ScreenToClient(rtSM);
    ScreenToClient(rtSI);
    ScreenToClient(rtMark);
    ScreenToClient(rtGeometry);
    ScreenToClient(rtVisionItem);

    m_gridVisionSM->Create(rtSM, this, IDC_GRID_SM);
    m_gridVisionSM->SetEditable(TRUE);
    m_gridVisionSM->EnableDragAndDrop(FALSE);

    m_gridVisionSI->Create(rtSI, this, IDC_GRID_SI);
    m_gridVisionSI->SetEditable(TRUE);
    m_gridVisionSI->EnableDragAndDrop(FALSE);

    m_gridVisionMark->Create(rtMark, this, IDC_GRID_MARK);
    m_gridVisionMark->SetEditable(isMarkInspectionEnabled); //kircheis_SWIR
    m_gridVisionMark->EnableDragAndDrop(FALSE);

    m_gridVisionGeometry->Create(rtGeometry, this, IDC_GRID_GEOMETRY);
    m_gridVisionGeometry->SetEditable(bIsNotSideVision);
    m_gridVisionGeometry->EnableDragAndDrop(FALSE);

    m_gridVisionFixed->Create(rtVisionItem, this, IDC_GRID_INSPITEM);
    m_gridVisionFixed->SetEditable(TRUE);
    m_gridVisionFixed->EnableDragAndDrop(FALSE);

    m_gridVisionSM->SetFont(&fontGrid);
    m_gridVisionSI->SetFont(&fontGrid);
    m_gridVisionMark->SetFont(&fontGrid);
    m_gridVisionGeometry->SetFont(&fontGrid);
    m_gridVisionFixed->SetFont(&fontGrid);

    SetGrid();

    strFontName.Empty();

    ((CEdit*)(GetDlgItem(IDC_BUTTON_MARK_ADD)))->EnableWindow(isMarkInspectionEnabled); //kircheis_SWIR
    ((CEdit*)(GetDlgItem(IDC_BUTTON_MARK_DELETE)))->EnableWindow(isMarkInspectionEnabled); //kircheis_SWIR
    ((CEdit*)(GetDlgItem(IDC_BTN_GEO_ADD)))->EnableWindow(TRUE);
    ((CEdit*)(GetDlgItem(IDC_BTN_GEO_DEL)))->EnableWindow(TRUE);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionInspSpec::SetGrid()
{
    SetGridSurface(m_gridVisionSM);
    SetGridSurface(m_gridVisionSI);
    SetGridMark(m_gridVisionMark);
    SetGridGeometry(m_gridVisionGeometry);

    long nSpecNum = (long)(m_availableFixedItems.size());
    m_gridVisionFixed->SetColumnCount(3);
    m_gridVisionFixed->SetRowCount(nSpecNum + 1);
    m_gridVisionFixed->SetFixedRowCount(1);
    m_gridVisionFixed->SetFixedColumnCount(1);
    m_gridVisionFixed->SetEditable(TRUE);

    CString str;

    for (long i = 0; i < nSpecNum; i++)
    {
        // ID
        m_gridVisionFixed->SetItemBkColour(i + 1, 0, RGB(250, 250, 200));
        str.Format(_T("%d"), i);
        m_gridVisionFixed->SetItemText(i + 1, 0, str);

        // Name
        m_gridVisionFixed->SetItemText(i + 1, 1, m_availableFixedItems[i]->m_strModuleName);
        m_gridVisionFixed->GetCell(i + 1, 0)->SetFormat(DT_LEFT);
        m_gridVisionFixed->GetCell(i + 1, 1)->SetFormat(DT_LEFT);
        m_gridVisionFixed->SetCellType(i + 1, 2, RUNTIME_CLASS(CGridCellCheck2));
        m_gridVisionFixed->SetItemText(i + 1, 2, _T(""));
        ((CGridCellCheck2*)m_gridVisionFixed->GetCell(i + 1, 2))->SetCheck(m_availableFixedItems[i]->IsEnabled());
        m_gridVisionFixed->GetCell(i + 1, 2)->SetFormat(DT_LEFT);
        m_gridVisionFixed->SetItemState(i + 1, 1, GVIS_READONLY);
    }

    // 0 번째 Row 에 각각의 Title 을 정의
    m_gridVisionFixed->SetItemText(0, 0, _T("No"));
    m_gridVisionFixed->GetCell(0, 0)->SetFormat(DT_CENTER);
    m_gridVisionFixed->SetItemText(0, 1, _T("Name"));
    m_gridVisionFixed->GetCell(0, 1)->SetFormat(DT_CENTER);

    m_gridVisionFixed->SetItemText(0, 2, _T("Use"));
    m_gridVisionFixed->GetCell(0, 2)->SetFormat(DT_CENTER);

    m_gridVisionFixed->SetItemBkColour(0, 0, RGB(200, 200, 250));
    m_gridVisionFixed->SetItemBkColour(0, 1, RGB(200, 200, 250));
    m_gridVisionFixed->SetItemBkColour(0, 2, RGB(200, 200, 250));

    m_gridVisionFixed->SetColumnWidth(0, 45);
    m_gridVisionFixed->SetColumnWidth(1, 260);
    m_gridVisionFixed->SetColumnWidth(2, 45);

    // Grid 정렬
    m_gridVisionFixed->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridVisionFixed->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    m_gridVisionFixed->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    m_gridVisionFixed->Refresh();

    str.Empty();
}

void CDlgVisionInspSpec::SetGridSurface(CGridCtrl* grid)
{
    long index_s = GetGridSurface_StartIndex(grid);
    long index_e = GetGridSurface_EndIndex(grid);

    grid->SetColumnCount(3);
    grid->SetRowCount(GetGridSurface_ItemCount(grid) + 1);
    grid->SetFixedRowCount(1);
    grid->SetEditable(TRUE);

    CString str;
    long rowIndex = 1;
    for (long index = index_s; index <= index_e; index++, rowIndex++)
    {
        // ID
        grid->SetItemBkColour(rowIndex, 0, RGB(250, 250, 200));
        str.Format(_T("%d"), rowIndex);
        grid->SetItemText(rowIndex, 0, str);

        auto* itemInfo = m_visionUnit->m_visionInspectionSurfaces[index];

        // Name
        str = itemInfo->m_strModuleName;
        grid->SetItemText(rowIndex, 1, str);
        grid->GetCell(rowIndex, 0)->SetFormat(DT_LEFT);
        grid->GetCell(rowIndex, 1)->SetFormat(DT_LEFT);

        // Use
        grid->SetCellType(rowIndex, 2, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)grid->GetCell(rowIndex, 2))->SetCheck(itemInfo->IsEnabled());
        grid->GetCell(rowIndex, 2)->SetFormat(DT_LEFT);
    }

    // 0 번째 Row 에 각각의 Title 을 정의
    grid->SetItemText(0, 0, _T("No"));
    grid->SetItemText(0, 1, _T("Surface Name"));
    grid->SetItemText(0, 2, _T("Use"));

    for (long col = 0; col < grid->GetColumnCount(); col++)
    {
        grid->GetCell(0, col)->SetFormat(DT_CENTER);
        grid->SetItemBkColour(0, col, RGB(200, 200, 250));
    }

    grid->SetColumnWidth(0, 45);
    grid->SetColumnWidth(1, 195);
    grid->SetColumnWidth(2, 55);

    // Grid 정렬
    grid->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    grid->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    grid->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    grid->Refresh();

    str.Empty();
}

void CDlgVisionInspSpec::SetGridMark(CGridCtrl* grid)
{
    // Grid 가로 세로 개수 설정
    long nSpecNum = (long)(m_visionUnit->m_visionInspectionMarks.size());
    CString str;

    grid->SetColumnCount(3);
    grid->SetRowCount(nSpecNum + 1);
    grid->SetFixedRowCount(1);
    grid->SetEditable(TRUE);

    long rowIndex = 1;
    for (long i = 0; i < nSpecNum; i++, rowIndex++)
    {
        auto* itemInfo = m_visionUnit->m_visionInspectionMarks[i];

        // ID
        grid->SetItemBkColour(rowIndex, 0, RGB(250, 250, 200));
        str.Format(_T("%d"), i);
        grid->SetItemText(rowIndex, 0, str);
        grid->SetItemState(rowIndex, 0, GVIS_READONLY);

        // Name
        grid->SetItemText(rowIndex, 1, itemInfo->m_strModuleName);
        grid->GetCell(rowIndex, 0)->SetFormat(DT_LEFT);
        grid->GetCell(rowIndex, 1)->SetFormat(DT_LEFT);

        // Use
        grid->SetCellType(rowIndex, 2, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)grid->GetCell(rowIndex, 2))->SetCheck(itemInfo->IsEnabled());
        grid->GetCell(rowIndex, 2)->SetFormat(DT_LEFT);
    }

    // 0 번째 Row 에 각각의 Title 을 정의
    grid->SetItemText(0, 0, _T("No"));
    grid->SetItemText(0, 1, _T("Mark Name"));
    grid->SetItemText(0, 2, _T("Use"));

    for (long col = 0; col < grid->GetColumnCount(); col++)
    {
        grid->GetCell(0, col)->SetFormat(DT_CENTER);
        grid->SetItemBkColour(0, col, RGB(200, 200, 250));
    }

    grid->SetColumnWidth(0, 45);
    grid->SetColumnWidth(1, 220);
    grid->SetColumnWidth(2, 55);

    // Grid 정렬
    grid->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    grid->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    grid->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    grid->Refresh();

    str.Empty();
}

void CDlgVisionInspSpec::SetGridGeometry(CGridCtrl* grid)
{
    // Grid 가로 세로 개수 설정
    long nSpecNum = (long)m_visionUnit->m_visionInspectionGeometries.size();
    grid->SetColumnCount(3);
    grid->SetRowCount(nSpecNum + 1);
    grid->SetFixedRowCount(1);
    grid->SetEditable(TRUE);

    CString str;

    long rowIndex = 1;
    for (long i = 0; i < nSpecNum; i++, rowIndex++)
    {
        auto* itemInfo = m_visionUnit->m_visionInspectionGeometries[i];

        // ID
        grid->SetItemBkColour(i + 1, 0, RGB(250, 250, 200));
        str.Format(_T("%d"), i);
        grid->SetItemText(i + 1, 0, str);

        // Name
        grid->SetItemText(i + 1, 1, itemInfo->m_strModuleName);
        grid->GetCell(i + 1, 0)->SetFormat(DT_LEFT);
        grid->GetCell(i + 1, 1)->SetFormat(DT_LEFT);
        grid->SetItemState(i + 1, 0, GVIS_READONLY);

        // Use
        grid->SetCellType(rowIndex, 2, RUNTIME_CLASS(CGridCellCheck2));
        ((CGridCellCheck2*)grid->GetCell(rowIndex, 2))->SetCheck(itemInfo->IsEnabled());
        grid->GetCell(rowIndex, 2)->SetFormat(DT_LEFT);
    }

    // 0 번째 Row 에 각각의 Title 을 정의
    grid->SetItemText(0, 0, _T("No"));
    grid->SetItemText(0, 1, _T("Geometry Name"));
    grid->SetItemText(0, 2, _T("Use"));

    for (long col = 0; col < grid->GetColumnCount(); col++)
    {
        grid->GetCell(0, col)->SetFormat(DT_CENTER);
        grid->SetItemBkColour(0, col, RGB(200, 200, 250));
    }

    grid->SetColumnWidth(0, 45);
    grid->SetColumnWidth(1, 220);
    grid->SetColumnWidth(2, 55);

    // Grid 정렬
    grid->GetDefaultCell(FALSE, TRUE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    grid->GetDefaultCell(TRUE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    grid->GetDefaultCell(FALSE, FALSE)->SetFormat(DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    grid->Refresh();

    str.Empty();
}

void CDlgVisionInspSpec::OnGridEditEnd_SM(NMHDR* pNotifyStruct, LRESULT* result)
{
    EventGridSurfaceEditEnd(m_gridVisionSM, pNotifyStruct, result);
}

void CDlgVisionInspSpec::OnGridEditEnd_SI(NMHDR* pNotifyStruct, LRESULT* result)
{
    EventGridSurfaceEditEnd(m_gridVisionSI, pNotifyStruct, result);
}

void CDlgVisionInspSpec::OnGridEditEnd_Mark(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;
    long columnIndex = pNotify->iColumn;

    if (row < m_visionUnit->m_visionInspectionMarks.size())
    {
        auto* itemInfo = m_visionUnit->m_visionInspectionMarks[row];
        long gridRow = row + 1;
        CString strCurrentModuleName = m_gridVisionMark->GetItemText(gridRow, 1);

        switch (columnIndex)
        {
            case 1:
                if (strCurrentModuleName.Find(_T("Mark")) < 0)
                {
                    strCurrentModuleName = _T("Mark_") + strCurrentModuleName;
                }

                itemInfo->m_strModuleName = strCurrentModuleName;

                for (auto& inspSpec : itemInfo->m_fixedInspectionSpecs)
                {
                    inspSpec.m_inspName = strCurrentModuleName;
                }

                m_gridVisionMark->SetItemText(gridRow, 1, strCurrentModuleName);
                break;

            case 2:
                // Use Check
                if (1)
                {
                    BOOL bCheck = ((CGridCellCheck2*)m_gridVisionMark->GetCell(gridRow, columnIndex))->GetCheck();
                    itemInfo->SetEnabled(bCheck);
                }
                break;
        }

        strCurrentModuleName.Empty();
    }

    m_gridVisionMark->Refresh();

    *result = 0;
}

void CDlgVisionInspSpec::OnGridEditEnd_Geometry(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;
    long columnIndex = pNotify->iColumn;

    if (row < m_visionUnit->m_visionInspectionGeometries.size())
    {
        auto* itemInfo = m_visionUnit->m_visionInspectionGeometries[row];
        long gridRow = row + 1;

        CString strCurrentModuleName = m_gridVisionGeometry->GetItemText(gridRow, 1);

        switch (columnIndex)
        {
            case 1:
                if (strCurrentModuleName.Find(_T("Geo_")) < 0)
                {
                    strCurrentModuleName = _T("Geo_") + strCurrentModuleName;
                }

                itemInfo->m_strModuleName = strCurrentModuleName;

                for (auto& inspSpec : itemInfo->m_fixedInspectionSpecs)
                {
                    inspSpec.m_inspName = strCurrentModuleName;
                }

                for (auto& inspSpec : itemInfo->m_variableInspectionSpecs)
                {
                    inspSpec.m_inspName = strCurrentModuleName;
                }

                m_gridVisionGeometry->SetItemText(gridRow, 1, strCurrentModuleName);
                break;

            case 2:
                // Use Check
                if (1)
                {
                    BOOL bCheck = ((CGridCellCheck2*)m_gridVisionGeometry->GetCell(gridRow, columnIndex))->GetCheck();
                    itemInfo->SetEnabled(bCheck);
                }
                break;
        }

        strCurrentModuleName.Empty();
    }

    m_gridVisionGeometry->Refresh();

    *result = 0;
}

void CDlgVisionInspSpec::OnGridEditEnd_InspItem(NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;

    if (row > -1)
    {
        BOOL bUse = ((CGridCellCheck2*)m_gridVisionFixed->GetCell(row + 1, 2))->GetCheck();
        m_availableFixedItems[row]->SetEnabled(bUse);
    }

    *result = 0;
}

void CDlgVisionInspSpec::OnBnClickedButtonSmAdd()
{
    CString str;
    str.Format(_T("MSK_Surface_%d"), (long)(m_visionUnit->m_visionInspectionSurfaces.size()) + 1);

    long insertIndex = max(0, GetGridSurface_EndIndex(m_gridVisionSM) + 1);
    VisionInspectionSurface* VisionSurface
        = new VisionInspectionSurface(::CreateGUID(), str, *m_visionUnit, *m_visionUnit->m_pPackageSpec);
    VisionSurface->SetEnabled(TRUE);
    VisionSurface->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode = TRUE;

    m_visionUnit->m_visionInspectionSurfaces.insert(
        m_visionUnit->m_visionInspectionSurfaces.begin() + insertIndex, VisionSurface);

    SetGrid();

    str.Empty();
}

void CDlgVisionInspSpec::OnBnClickedButtonSmDelete()
{
    long nRow = GetSelectedSurfaceIndex(m_gridVisionSM);

    if (nRow >= 0 && nRow < (long)(m_visionUnit->m_visionInspectionSurfaces.size()))
    {
        m_visionUnit->m_visionInspectionSurfaces.erase(m_visionUnit->m_visionInspectionSurfaces.begin() + nRow);
    }

    SetGrid();
}

void CDlgVisionInspSpec::OnBnClickedButtonSmExport()
{
    ExportSurfaceItem(GetSelectedSurfaceIndex(m_gridVisionSM));
}

void CDlgVisionInspSpec::OnBnClickedButtonSmImport()
{
    ImportSurfaceItem(GetSelectedSurfaceIndex(m_gridVisionSM));
}

void CDlgVisionInspSpec::OnBnClickedButtonSiAdd()
{
    CString str;
    str.Format(_T("Surface_%d"), (long)(m_visionUnit->m_visionInspectionSurfaces.size()) + 1);

    VisionInspectionSurface* VisionSurface
        = new VisionInspectionSurface(::CreateGUID(), str, *m_visionUnit, *m_visionUnit->m_pPackageSpec);
    VisionSurface->SetEnabled(TRUE);

    m_visionUnit->m_visionInspectionSurfaces.push_back(VisionSurface);

    SetGrid();

    str.Empty();
}

void CDlgVisionInspSpec::OnBnClickedButtonSiDelete()
{
    long nRow = GetSelectedSurfaceIndex(m_gridVisionSI);

    if (nRow >= 0 && nRow < (long)(m_visionUnit->m_visionInspectionSurfaces.size()))
    {
        m_visionUnit->m_visionInspectionSurfaces.erase(m_visionUnit->m_visionInspectionSurfaces.begin() + nRow);
    }

    SetGrid();
}

void CDlgVisionInspSpec::OnBnClickedButtonSiExport()
{
    ExportSurfaceItem(GetSelectedSurfaceIndex(m_gridVisionSI));
}

void CDlgVisionInspSpec::OnBnClickedButtonSiImport()
{
    ImportSurfaceItem(GetSelectedSurfaceIndex(m_gridVisionSI));
}

void CDlgVisionInspSpec::OnBnClickedButtonMarkAdd()
{
    CString str;
    str.Format(_T("Mark_%d"), (long)(m_visionUnit->m_visionInspectionMarks.size()));
    VisionInspectionMark* VisionMark
        = new VisionInspectionMark(::CreateGUID(), str, *m_visionUnit, *m_visionUnit->m_pPackageSpec);

    long nTemp = 0;
    VisionMark->InitMemory(TRUE, nTemp);
    VisionMark->SetEnabled(TRUE);

    m_visionUnit->m_visionInspectionMarks.push_back(VisionMark);

    SetGrid();

    str.Empty();
}

void CDlgVisionInspSpec::OnBnClickedButtonMarkDelete()
{
    long nRow = GetSelectedGridCellRow(m_gridVisionMark);

    if (nRow >= 0 && nRow < (long)(m_visionUnit->m_visionInspectionMarks.size()))
    {
        m_visionUnit->m_visionInspectionMarks.erase(m_visionUnit->m_visionInspectionMarks.begin() + nRow);
    }

    SetGrid();
}

long CDlgVisionInspSpec::GetSelectedGridCellRow(CGridCtrl* grid)
{
    CCellRange CellRange = grid->GetSelectedCellRange();

    if (!CellRange.IsValid())
        return -1;

    if (CellRange.GetMinRow() != CellRange.GetMaxRow())
        return -1;

    long nSelItem = CellRange.GetTopLeft().row - 1;

    return nSelItem;
}

void CDlgVisionInspSpec::OnGridDbClick_InspItem(NMHDR* pNotifyStruct, LRESULT* result)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;

    if (row > -1)
    {
        CString strModuleName = m_gridVisionFixed->GetItemText(row + 1, 1);

        std::vector<VisionInspectionSpec> vecVisionItemSpec;

        m_visionUnit->GetSpecData(strModuleName, vecVisionItemSpec);

        long nSpecSize = (long)vecVisionItemSpec.size();
        if (nSpecSize == 0) // 한국_ edge align 같은 경우 spec 값이 없음-> 열리지 않아야함
            return;

        CDlgInspSpecList dlgSpecList;
        dlgSpecList.SetData(vecVisionItemSpec);

        if (dlgSpecList.DoModal() == IDOK)
        {
            m_visionUnit->SetSpecData(strModuleName, dlgSpecList.GetData());
        }

        strModuleName.Empty();
    }

    *result = 0;
}

void CDlgVisionInspSpec::OnGridDbClick_Mark(NMHDR* pNotifyStruct, LRESULT* result)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long row = pNotify->iRow - 1;
    long col = pNotify->iColumn;

    if (col == 0 && row > -1)
    {
        std::vector<VisionInspectionSpec> vecVisionItemSpec;
        m_visionUnit->GetSpecData(_T(""), vecVisionItemSpec, row);

        CDlgInspSpecList dlgSpecList;
        dlgSpecList.SetData(vecVisionItemSpec);

        if (dlgSpecList.DoModal() == IDOK)
        {
            m_visionUnit->SetSpecData(_T(""), dlgSpecList.GetData(), row);
        }
    }

    *result = 0;
}

void CDlgVisionInspSpec::OnBnClickedBtnGeoDel()
{
    long nRow = GetSelectedGridCellRow(m_gridVisionGeometry);

    if (nRow >= 0 && nRow < (long)(m_visionUnit->m_visionInspectionGeometries.size()))
    {
        m_visionUnit->m_visionInspectionGeometries.erase(m_visionUnit->m_visionInspectionGeometries.begin() + nRow);
    }

    SetGrid();
}

void CDlgVisionInspSpec::OnBnClickedBtnGeoAdd()
{
    CString str;
    str.Format(_T("Geo_Insp%d"), (long)m_visionUnit->m_visionInspectionGeometries.size() + 1);

    auto* inspItem = new VisionInspectionGeometry(::CreateGUID(), str, *m_visionUnit, *m_visionUnit->m_pPackageSpec);
    inspItem->SetEnabled(TRUE);
    m_visionUnit->m_visionInspectionGeometries.push_back(inspItem);

    SetGrid();

    str.Empty();
}

void CDlgVisionInspSpec::OnBnClickedButtonApply()
{
    long nSpecNum = (long)m_visionUnit->m_visionInspectionSurfaces.size();

    CString str;
    std::vector<CString> surfaceNameList;
    std::map<CString, BOOL> nameSameCheck;
    surfaceNameList.resize(nSpecNum);

    for (long surfaceType = 0; surfaceType < 2; surfaceType++)
    {
        CGridCtrl* grid = surfaceType == 0 ? m_gridVisionSM : m_gridVisionSI;

        // Name
        long rowIndex = 1;
        for (long index = GetGridSurface_StartIndex(grid); index <= GetGridSurface_EndIndex(grid); index++)
        {
            surfaceNameList[index] = grid->GetItemText(rowIndex, 1);
            if (nameSameCheck.find(surfaceNameList[index]) != nameSameCheck.end())
            {
                AfxMessageBox(_T("Change the same inspection item Name"));
                return;
            }

            nameSameCheck[surfaceNameList[index]] = TRUE;
            rowIndex++;
        }
    }

    long nSpecNum2 = (long)m_visionUnit->m_visionInspectionMarks.size();
    std::vector<CString> vecStr2;
    vecStr2.resize(nSpecNum2);
    for (long i = 0; i < nSpecNum2; i++)
    {
        // Name
        vecStr2[i] = m_gridVisionMark->GetItemText(i + 1, 1);
        if (nameSameCheck.find(vecStr2[i]) != nameSameCheck.end())
        {
            AfxMessageBox(_T("Change the same inspection item Name"));
            return;
        }

        nameSameCheck[vecStr2[i]] = TRUE;
    }

    long nSpecNum3 = (long)m_visionUnit->m_visionInspectionGeometries.size();
    std::vector<CString> vecStr3;
    vecStr3.resize(nSpecNum3);

    for (long i = 0; i < nSpecNum3; i++)
    {
        // Name
        vecStr3[i] = m_gridVisionGeometry->GetItemText(i + 1, 1);
        if (nameSameCheck.find(vecStr3[i]) != nameSameCheck.end())
        {
            AfxMessageBox(_T("Change the same inspection item Name"));
            return;
        }

        nameSameCheck[vecStr3[i]] = TRUE;
    }

    CDialog::OnOK();
}

void CDlgVisionInspSpec::OnBnClickedButtonCancel()
{
    CMemFile dbCurrent;
    if (1)
    {
        ArchiveAllType ar(&dbCurrent, ArchiveAllType::store);
        CiDataBase db;

        m_visionUnit->LinkDataBase(LAST_JOB_VERSION, TRUE, db);
        db.Serialize(ar);
    }

    //--------------------------------------------------------------------------
    // Data가 바뀐 것인 있는지 JobFile을 MemFile로 만들어서 비교한다
    //--------------------------------------------------------------------------

    bool change = false;
    if (dbCurrent.GetLength() != m_dbBackup.GetLength())
    {
        change = true;
    }
    else
    {
        long dataLength = long(dbCurrent.GetLength());

        m_dbBackup.SeekToBegin();
        dbCurrent.SeekToBegin();

        BYTE* buffer1 = new BYTE[dataLength];
        BYTE* buffer2 = new BYTE[dataLength];

        m_dbBackup.Read(buffer1, dataLength);
        dbCurrent.Read(buffer2, dataLength);

        if (memcmp(buffer1, buffer2, dataLength) != 0)
        {
            change = true;
        }

        delete[] buffer2;
        delete[] buffer1;
    }

    if (change)
    {
        if (MessageBox(_T("Do you want to restore your previous setting?"), _T("Message"), MB_YESNO) != IDYES)
        {
            return;
        }
    }

    //--------------------------------------------------------------------------
    // Backup 되어있던 MemFile로 복원한다
    //--------------------------------------------------------------------------

    m_dbBackup.SeekToBegin();

    CiDataBase db;
    ArchiveAllType ar(&m_dbBackup, ArchiveAllType::load);
    db.Serialize(ar);

    m_visionUnit->LinkDataBase(LAST_JOB_VERSION, FALSE, db);

    CDialog::OnCancel();
}

long CDlgVisionInspSpec::GetGridSurface_StartIndex(CGridCtrl* grid)
{
    if (grid == m_gridVisionSM)
    {
        long startIndex = 0;

        for (long index = 0; index < (long)(m_visionUnit->m_visionInspectionSurfaces.size()); index++)
        {
            if (m_visionUnit->m_visionInspectionSurfaces[index]->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
            {
                startIndex = index;
                break;
            }
        }

        return startIndex;
    }

    long startIndex = long(m_visionUnit->m_visionInspectionSurfaces.size());

    for (long index = 0; index < (long)(m_visionUnit->m_visionInspectionSurfaces.size()); index++)
    {
        if (!m_visionUnit->m_visionInspectionSurfaces[index]->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
        {
            startIndex = index;
            break;
        }
    }

    return startIndex;
}

long CDlgVisionInspSpec::GetGridSurface_EndIndex(CGridCtrl* grid)
{
    long endIndex = -1;

    if (grid == m_gridVisionSM)
    {
        for (long index = 0; index < (long)(m_visionUnit->m_visionInspectionSurfaces.size()); index++)
        {
            if (m_visionUnit->m_visionInspectionSurfaces[index]->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
            {
                endIndex = index;
            }
        }

        return endIndex;
    }

    for (long index = 0; index < (long)(m_visionUnit->m_visionInspectionSurfaces.size()); index++)
    {
        if (!m_visionUnit->m_visionInspectionSurfaces[index]->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode)
        {
            endIndex = index;
        }
    }

    return endIndex;
}

long CDlgVisionInspSpec::GetGridSurface_ItemCount(CGridCtrl* grid)
{
    long index_s = GetGridSurface_StartIndex(grid);
    long index_e = GetGridSurface_EndIndex(grid);

    return max(0, index_e - index_s + 1);
}

long CDlgVisionInspSpec::GetSelectedSurfaceIndex(CGridCtrl* grid)
{
    long index = GetSelectedGridCellRow(grid);
    if (index >= 0)
    {
        index += GetGridSurface_StartIndex(grid);
    }

    return index;
}

void CDlgVisionInspSpec::ExportSurfaceItem(long surfaceItemIndex)
{
    if (surfaceItemIndex >= 0 && surfaceItemIndex < (long)(m_visionUnit->m_visionInspectionSurfaces.size()))
    {
        CString strModuleName = m_visionUnit->m_visionInspectionSurfaces[surfaceItemIndex]->m_strModuleName;

        CFileDialog Dlg(
            FALSE, _T("sitem"), strModuleName, OFN_HIDEREADONLY, _T("Surface Item File Format (*.sitem)|*.sitem||"));

        if (Dlg.DoModal() != IDOK)
            return;

        CString strFilePath = Dlg.GetPathName();

        m_visionUnit->SurfaceJobManager(TRUE, surfaceItemIndex, strFilePath);

        strModuleName.Empty();
        strFilePath.Empty();
    }
}

void CDlgVisionInspSpec::ImportSurfaceItem(long surfaceItemIndex)
{
    if (surfaceItemIndex >= 0 && surfaceItemIndex < (long)(m_visionUnit->m_visionInspectionSurfaces.size()))
    {
        CFileDialog Dlg(TRUE, _T("sitem"), NULL, OFN_HIDEREADONLY, _T("Surface Item File Format (*.sitem)|*.sitem||"));

        if (Dlg.DoModal() != IDOK)
            return;

        CString strFilePath = Dlg.GetPathName();

        m_visionUnit->SurfaceJobManager(FALSE, surfaceItemIndex, strFilePath);
        SetGrid();

        strFilePath.Empty();
    }
}

void CDlgVisionInspSpec::EventGridSurfaceEditEnd(CGridCtrl* grid, NMHDR* pNotifyStruct, LRESULT* result)
{
    NM_GRIDVIEW* pNotify = (NM_GRIDVIEW*)pNotifyStruct;

    long gridRow = pNotify->iRow;
    long columnIndex = pNotify->iColumn;

    if (gridRow <= 0)
    {
        *result = 0;
        return;
    }

    long itemIndex = GetGridSurface_StartIndex(grid) + gridRow - 1;

    if (itemIndex < (long)(m_visionUnit->m_visionInspectionSurfaces.size()))
    {
        auto* itemInfo = m_visionUnit->m_visionInspectionSurfaces[itemIndex];

        CString strPrevModuleName = itemInfo->m_strModuleName; // 이거가지고 찾아서 바꿔야함

        CString strSurfaceInspName = grid->GetItemText(gridRow, 1);

        BOOL maskMode = itemInfo->m_surfacePara->m_SurfaceItem.m_bSurfaceMaskMode;
        CString nameHead = strSurfaceInspName.Left(4);

        switch (columnIndex)
        {
            case 0:
                // No
                if (1)
                {
                    long newNoIndex = _ttoi(grid->GetItemText(gridRow, columnIndex));
                    if (newNoIndex >= 1 && newNoIndex < GetGridSurface_ItemCount(grid) + 1)
                    {
                        long insertIndex = GetGridSurface_StartIndex(grid) + newNoIndex - 1;
                        m_visionUnit->m_visionInspectionSurfaces.erase(
                            m_visionUnit->m_visionInspectionSurfaces.begin() + itemIndex);
                        m_visionUnit->m_visionInspectionSurfaces.insert(
                            m_visionUnit->m_visionInspectionSurfaces.begin() + insertIndex, itemInfo);
                    }
                }
                break;
            case 1:
                // Name
                if (maskMode)
                {
                    if (nameHead != _T("MSK_"))
                    {
                        strSurfaceInspName = _T("MSK_") + strSurfaceInspName;
                        grid->SetItemText(gridRow, 1, strSurfaceInspName);
                    }
                }
                else
                {
                    if (nameHead == _T("MSK_"))
                    {
                        strSurfaceInspName.Delete(0, 4);
                        grid->SetItemText(gridRow, 1, strSurfaceInspName);
                    }
                }

                itemInfo->m_strModuleName = strSurfaceInspName;
                for (auto& inspSpec : itemInfo->m_fixedInspectionSpecs)
                {
                    inspSpec.m_inspName = strSurfaceInspName;
                    inspSpec.m_specName = strSurfaceInspName;
                }
                break;

            case 2:
                // Use Check
                if (1)
                {
                    BOOL bCheck = ((CGridCellCheck2*)grid->GetCell(gridRow, columnIndex))->GetCheck();
                    itemInfo->SetEnabled(bCheck);
                }
                break;
        }

        strPrevModuleName.Empty();
        strSurfaceInspName.Empty();
        nameHead.Empty();
    }

    SetGrid();

    *result = 0;
}
