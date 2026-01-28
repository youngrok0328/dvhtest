//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionComponent3D.h"

//CPP_2_________________________________ This project's headers
#include "ComponentAlign.h"
#include "Dlg3DSpecDB.h"
#include "DlgChangeSpecName3D.h"
#include "DlgComponentAlign.h"
#include "VisionInspectionComponent3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Component.h"
#include "../../InformationModule/dPI_DataBase/DMSArrayChip.h"
#include "../../InformationModule/dPI_DataBase/DMSBall.h"
#include "../../InformationModule/dPI_DataBase/DMSChip.h"
#include "../../InformationModule/dPI_DataBase/DMSHeatsink.h"
#include "../../InformationModule/dPI_DataBase/DMSPassiveChip.h"
#include "../../InformationModule/dPI_DataBase/DMSPatch.h"
#include "../../ProcessingItems/VisionProcessingMapDataEditor/VisionMapDataEditorUI.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1013)

//CPP_7_________________________________ Implementation body
//
enum
{
    COLUMM_NO,
    COLUMN_CHIP,
    COLUMN_ANGLE,
    COLUMN_TYPES,
    COLUMN_SPEC,
    COLUMN_RESULT,
};

enum PropertyGridItemID
{
    ITEM_ID_SOURCE_IMAGE = 1,
    ITEM_ID_BUTTON_3D_VIEWER,
    ITEM_ID_BUTTON_ALGORITHM_DB,
    ITEM_ID_PARA_COMP_MISSING_HEIGHT_PER,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionComponent3D, CDialog)

CDlgVisionInspectionComponent3D::CDlgVisionInspectionComponent3D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionComponent3D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionComponent3D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_propertyGrid(nullptr)
    , m_pCurPassiveInfoDB(nullptr)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionPara = m_pVisionInsp->m_VisionPara;

    m_pDlg3DSpecDB = new CDlg3DSpecDB(pVisionInsp);
    m_pDlgCompAlign = new CDlgComponentAlign(pVisionInsp->m_pCompAlign);

    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
    m_nCurID = -1;

    m_nAddNum = 0;
    m_pSelChip = NULL;
    m_p3DViewer = NULL;

    m_nCmbSelImage = 0;
}

CDlgVisionInspectionComponent3D::~CDlgVisionInspectionComponent3D()
{
    m_pVisionInsp->SetVisionInspectionSpecs();

    delete m_imageLotView;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;

    if (m_pDlgCompAlign)
    {
        delete m_pDlgCompAlign;
        m_pDlgCompAlign = NULL;
    }

    m_pVisionInsp->m_pVisionInspDlg = NULL;

    if (m_pDlg3DSpecDB)
    {
        delete m_pDlg3DSpecDB;
        m_pDlg3DSpecDB = NULL;
    }

    delete m_propertyGrid;
}

void CDlgVisionInspectionComponent3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
    DDX_Control(pDX, IDC_CHIP_LIST, m_chipListControl);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionComponent3D, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionComponent3D::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionComponent3D::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionComponent3D::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspectionComponent3D::OnCbnSelchangeComboCurrpane)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &CDlgVisionInspectionComponent3D::OnRoiChanged)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionComponent3D::OnShowDetailResult)

ON_COMMAND(ID_MENU_DMS_INFO, &CDlgVisionInspectionComponent3D::OnMenuDmsInfo)
ON_COMMAND(ID_MENU_CHANGESPEC, &CDlgVisionInspectionComponent3D::OnMenuChangeSpec)
ON_COMMAND(ID_MENU_CHANGE_DMSINFO, &CDlgVisionInspectionComponent3D::OnMenuChangeDMSInfo)
ON_NOTIFY(NM_KEYDOWN, IDC_CHIP_LIST, OnReportKeyDown)
ON_NOTIFY(NM_CLICK, IDC_CHIP_LIST, OnReportItemClick)
ON_NOTIFY(NM_RCLICK, IDC_CHIP_LIST, OnReportRClick)
ON_NOTIFY(NM_DBLCLK, IDC_CHIP_LIST, OnReportItemDblClick)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionComponent3D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionComponent3D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    // Dialog Control 위치 설정
    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionPara->m_vecVisionInspectionSpecs);
    //m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_fixedInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, CAST_UINT(IDC_STATIC));
    m_propertyGrid->ShowHelp(FALSE);

    SetPropertyGrid();
    SetpropertyList();

    m_pVisionInsp->MakePreInspInfo(true);
    UpdateChipList();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionComponent3D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    m_pDlgCompAlign->Create(CDlgComponentAlign::IDD);
    m_pDlgCompAlign->ShowWindow(SW_HIDE);

    m_pDlg3DSpecDB->Create(CDlg3DSpecDB::IDD);
    m_pDlg3DSpecDB->ShowWindow(SW_HIDE);

    return 0;
}

void CDlgVisionInspectionComponent3D::OnDestroy()
{
    CDialog::OnDestroy();

    m_pDlg3DSpecDB->DestroyWindow();
    m_pDlgCompAlign->DestroyWindow();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
}

void CDlgVisionInspectionComponent3D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionComponent3D::SetPropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("3D Component Parameter")))
    {
        /*if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Source Image"), m_nCmbSelImage)))
		{
			item->GetConstraints()->AddConstraint(_T("Z_Map"), 0);
			item->GetConstraints()->AddConstraint(_T("I_Map"), 1);

			item->SetID(ITEM_ID_SOURCE_IMAGE);
		}*/

        category->AddChildItem(new CCustomItemButton(_T("Algorithm DB"), TRUE, FALSE))
            ->SetID(ITEM_ID_BUTTON_ALGORITHM_DB);
        //m_propertyGrid->FindItem(ITEM_ID_BUTTON_3D_VIEWER)->SetHeight(50);
        m_propertyGrid->FindItem(ITEM_ID_BUTTON_ALGORITHM_DB)->SetHeight(50);

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                _T("Comp Missing. Component height ratio"), m_pVisionPara->m_fCompMissing_Height_Ratio, _T("%.0f %%"),
                m_pVisionPara->m_fCompMissing_Height_Ratio, 10.f, 200.f, 1.0f)))
        {
            item->SetID(ITEM_ID_PARA_COMP_MISSING_HEIGHT_PER);
        }

        category->Expand();
    }
    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

void CDlgVisionInspectionComponent3D::SetpropertyList()
{
    CRect rtTemp;
    rtTemp = m_procDlgInfo.m_rtParaArea;
    //rtTemp.top = rtTemp.top + 65;

    // Algorithm Parameter UI Display 위치 고정을 위해 GridRect를 특정하려고 개별 Rect 지정 - 2023.06.14_JHB
    CRect rtGridRect;
    m_propertyGrid->GetWindowRect(rtGridRect);

    Ipvm::Rect32s rtNewArea;
    rtNewArea.m_left = rtTemp.left;
    rtNewArea.m_top = rtGridRect.top * 3 / 5;
    rtNewArea.m_right = rtTemp.right;
    rtNewArea.m_bottom = rtTemp.bottom;

    rtTemp.SetRect(rtNewArea.m_left, rtNewArea.m_top, rtNewArea.m_right, rtNewArea.m_bottom);
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    m_chipListControl.MoveWindow(rtTemp);

    int nLen = (rtTemp.Width() - 110 - GetSystemMetrics(SM_CXVSCROLL)) / 3;
    m_chipListControl.AddColumn(new CXTPGridColumn(COLUMM_NO, _T("No."), 50));
    m_chipListControl.AddColumn(new CXTPGridColumn(COLUMN_CHIP, _T("PassiveID"), 70));
    m_chipListControl.AddColumn(new CXTPGridColumn(COLUMN_ANGLE, _T("Angle"), 70));
    m_chipListControl.AddColumn(new CXTPGridColumn(COLUMN_TYPES, _T("Type"), nLen));
    m_chipListControl.AddColumn(new CXTPGridColumn(COLUMN_SPEC, _T("Spec"), nLen));

    m_chipListControl.Populate();
    m_chipListControl.SetMultipleSelection(TRUE);
    m_chipListControl.SetTheme(xtpGridThemeVisualStudio2012Light);
    m_chipListControl.SetGridStyle(0, xtpGridSmallDots);
    COLORREF rgbGridColor = RGB(0, 0, 0);
    m_chipListControl.SetGridColor(rgbGridColor);
}

LRESULT CDlgVisionInspectionComponent3D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CString strTemp;

    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* ComboValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = ComboValue->GetEnum();
        switch (item->GetID())
        {
            case ITEM_ID_SOURCE_IMAGE:
                auto& imageLotInsp = m_pVisionInsp->getImageLotInsp();

                switch (data)
                {
                    case 0: // ZMap
                        m_imageLotView->SetImage(
                            imageLotInsp.m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()][0], _T("Frame1"));
                        break;
                    case 1: // IMap
                        m_imageLotView->SetImage(
                            imageLotInsp.m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()][1], _T("Frame2"));
                        break;
                };

                break;
        }
    }
    else if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_BUTTON_ALGORITHM_DB:
                OnBnClickedButtonSpecdb();
                break;
        }

        value->SetBool(FALSE);
    }

    m_propertyGrid->Refresh();

    strTemp.Empty();

    return 0;
}

void CDlgVisionInspectionComponent3D::UpdateChipList()
{
    m_chipListControl.ResetContent(FALSE);

    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() < 0)
        return;

    for (long nItem = 0; nItem < m_pVisionInsp->m_vecsPassiveInfoDB.size(); nItem++)
    {
        auto PassiveDBInfo = m_pVisionInsp->m_vecsPassiveInfoDB[nItem];
        auto* repRecord = new CXTPGridRecord();
        // No.
        CString strTemp("");
        strTemp.Format(_T("%d"), nItem + 1);
        repRecord->AddItem(new CXTPGridRecordItemText(strTemp));

        // PassiveID
        repRecord->AddItem(new CXTPGridRecordItemText(PassiveDBInfo.strCompName));

        // Angle
        strTemp.Format(_T("%.2f"), PassiveDBInfo.fPassiveAngle);
        repRecord->AddItem(new CXTPGridRecordItemText(strTemp));

        // Type
        repRecord->AddItem(new CXTPGridRecordItemText(PassiveDBInfo.strCompType));

        // Spec
        repRecord->AddItem(new CXTPGridRecordItemText(PassiveDBInfo.strSpecName));

        m_chipListControl.AddRecord(repRecord);

        strTemp.Empty();
    }

    m_chipListControl.Populate();
}

void CDlgVisionInspectionComponent3D::OnReportItemClick(NMHDR* pNotifyStruct, LRESULT* result)
{
    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;

    if (!pItemNotify->pRow || !pItemNotify->pColumn)
        return;

    m_imageLotView->ROI_RemoveAll();
    m_imageLotView->Overlay_RemoveAll();

    long nID = pItemNotify->pRow->GetRecord()->GetIndex();
    long nMaxPassiveCount = (long)m_pVisionInsp->m_vecsPassiveInfoDB.size();
    if (nID < 0 || nID >= nMaxPassiveCount)
        return;

    std::vector<long> nvecID(0);
    long nSelCnt = m_chipListControl.GetSelectedRows()->GetCount();
    POSITION pos = m_chipListControl.GetSelectedRows()->GetFirstSelectedRowPosition();
    while (pos)
    {
        int nItem = m_chipListControl.GetSelectedRows()->GetNextSelectedRow(pos)->GetRecord()->GetIndex();
        nvecID.push_back(nItem);
    }
    VERIFY(nSelCnt == (long)nvecID.size());

    for (long i = 0; i < nvecID.size(); i++)
    {
        auto PassiveInfoDB = m_pVisionInsp->m_vecsPassiveInfoDB[nvecID[i]];
        m_imageLotView->Overlay_AddRectangle(PassiveInfoDB.sfrtSpecROI_px, RGB(0, 255, 0));
    }

    m_imageLotView->Overlay_Show(TRUE);

    *result = 0;
}

void CDlgVisionInspectionComponent3D::OnReportRClick(NMHDR* pNotifyStruct, LRESULT* pResult)
{
    VERIFY(NULL != pNotifyStruct);
    VERIFY(NULL != pResult);

    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;
    ASSERT(pItemNotify->pColumn);
    CPoint ptClick = pItemNotify->pt;

    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() < 0)
        return;

    if (pItemNotify->pRow)
    {
        long nID = pItemNotify->pRow->GetRecord()->GetIndex();
        auto pPassiveInfoDB = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];
        m_pCurPassiveInfoDB = pPassiveInfoDB;
    }

    CMenu MenuItem;
    if (!MenuItem.LoadMenu(IDR_MENU_SPEC))
        return;
    CMenu* pMenuSub = MenuItem.GetSubMenu(0);
    pMenuSub->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY, ptClick.x, ptClick.y, this);
}

void CDlgVisionInspectionComponent3D::OnReportItemDblClick(NMHDR* pNotifyStruct, LRESULT* /*result*/)
{
    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;

    if (!pItemNotify->pRow || !pItemNotify->pColumn)
        return;

    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() < 0)
        return;

    long nID = pItemNotify->pRow->GetRecord()->GetIndex();
    long nMaxPassiveCount = (long)m_pVisionInsp->m_vecsPassiveInfoDB.size();
    if (nID < 0 || nID >= nMaxPassiveCount)
        return;

    auto pPassiveInfoDB = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];
    m_pCurPassiveInfoDB = pPassiveInfoDB;
    if (pPassiveInfoDB->eComponentType == eComponentTypeDefine::tyPASSIVE)
    {
        switch (pPassiveInfoDB->ePassiveType)
        {
            case enumPassiveTypeDefine::enum_PassiveType_Capacitor:
            case enumPassiveTypeDefine::enum_PassiveType_Register:
            case enumPassiveTypeDefine::enum_PassiveType_MIA:
                if (m_pDlgCompAlign && m_pVisionInsp->m_pCompAlign)
                {
                    HideItemDialog();
                    SComponentAlignSpec* pSpec = m_pVisionInsp->m_pCompAlign->GetAlignSpec(pPassiveInfoDB->strSpecName);
                    if (pSpec)
                    {
                        m_pDlgCompAlign->ShowWindow(SW_SHOW);
                    }
                }
                break;
        }
    }
}

void CDlgVisionInspectionComponent3D::OnReportKeyDown(NMHDR* pNotifyStruct, LRESULT* /*result*/)
{
    LPNMKEY lpNMKey = (LPNMKEY)pNotifyStruct;

    if (!m_chipListControl.GetFocusedRow())
        return;

    if (lpNMKey->nVKey == VK_DOWN || lpNMKey->nVKey == VK_UP)
    {
        m_imageLotView->ROI_RemoveAll();
        m_imageLotView->Overlay_RemoveAll();

        long nID = m_chipListControl.GetFocusedRow()->GetRecord()->GetIndex();
        long nMaxPassiveCount = (long)m_pVisionInsp->m_vecsPassiveInfoDB.size();
        if (nID < 0 || nID >= nMaxPassiveCount)
            return;

        std::vector<long> nvecID(0);
        long nSelCnt = m_chipListControl.GetSelectedRows()->GetCount();
        POSITION pos = m_chipListControl.GetSelectedRows()->GetFirstSelectedRowPosition();
        while (pos)
        {
            int nItem = m_chipListControl.GetSelectedRows()->GetNextSelectedRow(pos)->GetRecord()->GetIndex();
            nvecID.push_back(nItem);
        }
        VERIFY(nSelCnt == (long)nvecID.size());

        for (long i = 0; i < nvecID.size(); i++)
        {
            m_imageLotView->Overlay_AddRectangle(
                m_pVisionInsp->m_vecsPassiveInfoDB[nvecID[i]].sfrtSpecROI_px, RGB(0, 255, 0));
        }

        m_imageLotView->Overlay_Show(TRUE);
    }
}

void CDlgVisionInspectionComponent3D::OnMenuDmsInfo()
{
    long nID = m_chipListControl.GetSelectedRows()->GetAt(0)->GetRecord()->GetIndex();
    if (nID < 0 || nID >= m_pVisionInsp->m_vecsPassiveInfoDB.size())
        return;

    auto* pPassiveInfo = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];

    if (pPassiveInfo == NULL)
        return;

    // Job 파일에 등록된 Dimension 정보 확인.
    CString strPassiveInfo;
    strPassiveInfo.AppendFormat(_T("Component Width=%2.3fmm\r\n"), pPassiveInfo->fPassiveSpecWidth_mm);
    strPassiveInfo.AppendFormat(_T("Component Length=%2.3fmm\r\n"), pPassiveInfo->fPassiveSpecLength_mm);
    strPassiveInfo.AppendFormat(_T("Component Angle=%2.3fdegree\r\n"), pPassiveInfo->fPassiveAngle);
    strPassiveInfo.AppendFormat(_T("Component Thickness=%2.3fmm\r\n"), pPassiveInfo->fPassiveHeight_mm);

    strPassiveInfo.AppendFormat(_T("Electrode Width=%2.3fmm\r\n"), pPassiveInfo->fPassiveElectrodeWidth_mm);
    strPassiveInfo.AppendFormat(_T("Electrode Thickness=%2.3fmm\r\n"), pPassiveInfo->fPassiveElectrodeHeight_mm);

    MessageBox(strPassiveInfo, pPassiveInfo->strCompType, MB_OK);

    strPassiveInfo.Empty();
}

void CDlgVisionInspectionComponent3D::OnMenuChangeSpec()
{
    if (m_pVisionInsp->m_vecsPassiveInfoDB.size() <= 0 || m_pCurPassiveInfoDB == nullptr)
        return;

    std::vector<long> vecnID(0);
    std::vector<CString> vecstrName(0);
    int nSelCnt = m_chipListControl.GetSelectedRows()->GetCount();
    POSITION pos = m_chipListControl.GetSelectedRows()->GetFirstSelectedRowPosition();

    while (pos)
    {
        int nItem = m_chipListControl.GetSelectedRows()->GetNextSelectedRow(pos)->GetRecord()->GetIndex();
        vecnID.push_back(nItem);
        CXTPGridRecord* prepRecord = m_chipListControl.GetRecords()->GetAt(nItem);
        CXTPGridRecordItem* pItem;
        pItem = DYNAMIC_DOWNCAST(CXTPGridRecordItem, prepRecord->GetItem(1));
        vecstrName.push_back(pItem->GetCaption());

        if (m_pCurPassiveInfoDB->eComponentType != m_pVisionInsp->m_vecsPassiveInfoDB[nItem].eComponentType)
        {
            MessageBox(_T("The type of selected object is different."));
            return;
        }
    }

    VERIFY(nSelCnt == (long)vecnID.size());

    CString strOriName = m_pCurPassiveInfoDB->strSpecName;
    CDlgChangeSpecName3D Dlg;
    if (Dlg.DoModal(m_pVisionPara, m_pCurPassiveInfoDB) != IDOK)
        return;
    if (Dlg.m_strReName.GetLength() == 0)
        return;

    bool updateChipInfo = false;
    auto& specLink = m_pVisionInsp->m_VisionPara->m_specLink;

    for (long i = 0; i < (long)vecnID.size(); i++)
    {
        long nID = vecnID[i];

        sPassive_InfoDB* pPassiveInfo = &m_pVisionInsp->m_vecsPassiveInfoDB[nID];
        if (pPassiveInfo == nullptr)
            continue;

        CString strName = vecstrName[i];
        specLink.GetSpecName(m_pCurPassiveInfoDB->eComponentType, strName) = Dlg.m_strReName;
        pPassiveInfo->strSpecName = Dlg.m_strReName;

        BOOL bNewAlgo = TRUE;
        long nAlgoNum = (long)m_pVisionPara->m_vecComp3DAlgoSpec.size();
        for (long nAlgo = 0; nAlgo < nAlgoNum; nAlgo++)
        {
            if (Dlg.m_strReName == m_pVisionPara->m_vecComp3DAlgoSpec[nAlgo].strSpecName)
            {
                bNewAlgo = FALSE;
            }
        }

        if (bNewAlgo) // 같은 이름의 Spec이 현재 job에 없다면
        {
            SComp3DAlgorithmSpec NewPassiveAlgoSpec;
            NewPassiveAlgoSpec.strSpecName = Dlg.m_strReName;
            m_pVisionPara->m_vecComp3DAlgoSpec.push_back(NewPassiveAlgoSpec);

            SaveAsAlgorithmPara(m_pCurPassiveInfoDB->eComponentType, strOriName);
        }

        updateChipInfo = true;
        strName.Empty();
    }

    if (updateChipInfo)
    {
        UpdateChipList();
    }

    for (int nIdx = 0; nIdx < vecstrName.size(); nIdx++)
    {
        vecstrName[nIdx].Empty();
    }

    strOriName.Empty();
}

void CDlgVisionInspectionComponent3D::OnMenuChangeDMSInfo()
{
    Chip::DMSChip DmsChipInfo;
    Chip::DMSPassiveChip DMSPassiveInfo;
    Chip::DMSArrayChip DmsArrayInfo;
    Chip::DMSHeatsink DmsHeatsink;
    Chip::DMSPatch DmsPatch;
    Chip::DMSBall DmsBallInfo;

    std::vector<Package::Component> test;

    VisionMapDataEditorUI VisionEditorDlg(GetSafeHwnd(), &m_pVisionInsp->m_packageSpec, &DmsChipInfo, &DMSPassiveInfo,
        &DmsArrayInfo, &DmsHeatsink, &DmsPatch, &DmsBallInfo, test);

    if (VisionEditorDlg.DoModal() == IDOK)
    {
        ::AfxMessageBox(_T("Test"));

        m_pVisionInsp->m_packageSpec.CreateDMSInfo();
        m_pVisionInsp->MakePreInspInfo(TRUE);
    }
    else
        return;
}

void CDlgVisionInspectionComponent3D::SaveAsAlgorithmPara(eComponentTypeDefine i_eComponentType, CString strOriName)
{
    long nCompAlgoNum = (long)m_pVisionPara->m_vecComp3DAlgoSpec.size();

    if (i_eComponentType == eComponentTypeDefine::tyPASSIVE)
    {
        for (long nAlgo = 0; nAlgo < nCompAlgoNum; nAlgo++)
        {
            if (strOriName == m_pVisionPara->m_vecComp3DAlgoSpec[nAlgo].strSpecName)
            {
                // 이전의 스펙값을 넣어준다.
                m_pVisionPara->m_vecComp3DAlgoSpec[nCompAlgoNum - 1].CompAlign
                    = m_pVisionPara->m_vecComp3DAlgoSpec[nAlgo].CompAlign;
                break;
            }
        }
    }
}

void CDlgVisionInspectionComponent3D::OnBnClickedButtonSpecdb()
{
    HideItemDialog();

    if (m_pDlg3DSpecDB)
        m_pDlg3DSpecDB->ShowWindow(SW_SHOW);
}

void CDlgVisionInspectionComponent3D::HideItemDialog()
{
    if (m_pDlgCompAlign)
        m_pDlgCompAlign->ShowWindow(SW_HIDE);
    if (m_pDlg3DSpecDB)
        m_pDlg3DSpecDB->ShowWindow(SW_HIDE);
}

void CDlgVisionInspectionComponent3D::SetInitButton()
{
    CRect rtDlg;
    GetClientRect(rtDlg);

    CRect rtButton;
    m_buttonClose.GetWindowRect(rtButton);
    long nButtonPitch = rtButton.Width() + 3;

    // Close 버튼 위치 지정
    m_buttonClose.MoveWindow(
        rtDlg.right - nButtonPitch, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());

    // Inspect 버튼 위치 지정
    m_buttonInspect.MoveWindow(
        rtDlg.right - nButtonPitch * 2, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());

    // Inspect 버튼 위치 지정
    m_buttonApply.MoveWindow(
        rtDlg.right - nButtonPitch * 3, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());
}

void CDlgVisionInspectionComponent3D::OnBnClickedButtonInspect()
{
    m_pVisionInsp->SetVisionInspectionSpecs();

    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();

    // 검사 결과 화면 갱신
    m_procCommonResultDlg->Refresh();

    // 리스트 업데이트
    UpdateChipList();

    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
    ShowImage();
}

void CDlgVisionInspectionComponent3D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonSpecDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonResultDlg->ShowWindow((nTab == 1) ? SW_SHOW : SW_HIDE);
    m_procCommonDetailResultDlg->ShowWindow((nTab == 2) ? SW_SHOW : SW_HIDE);
    m_procCommonDebugInfoDlg->ShowWindow(nTab == 3 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 4 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

void CDlgVisionInspectionComponent3D::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionComponent3D::ShowImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    CString FrameToString;
    FrameToString.Format(_T("Frame1"));

    m_imageLotView->SetImage(
        m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()][0], FrameToString);

    FrameToString.Empty();
}

void CDlgVisionInspectionComponent3D::OnCbnSelchangeComboCurrpane()
{
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    long nSel = m_ComboCurrPane.GetCurSel();
    m_pVisionInsp->SetCurrentPaneID(nSel);
    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());

    // 검사 리스트 및 Source 이미지 계산.
    m_pVisionInsp->MakePreInspInfo(true);

    ShowImage();

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);
    //m_imageLotView->SetFieldOfView(rtPane);
}

LRESULT CDlgVisionInspectionComponent3D::OnRoiChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return 0;
}

LRESULT CDlgVisionInspectionComponent3D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long itemIndex = long(wparam);

    if (itemIndex < 0)
    {
        m_TabResult.SetCurSel(1);
    }
    else
    {
        m_TabResult.SetCurSel(2);

        m_procCommonDetailResultDlg->Refresh(itemIndex);
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

LRESULT CDlgVisionInspectionComponent3D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}

void CDlgVisionInspectionComponent3D::ResetSpecAndResultDlg()
{
    m_procCommonSpecDlg->ResetInitGrid();
    m_procCommonResultDlg->Refresh();
}
