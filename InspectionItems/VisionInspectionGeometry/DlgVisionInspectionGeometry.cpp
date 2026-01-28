//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionGeometry.h"

//CPP_2_________________________________ This project's headers
#include "DlgGeometryEditor.h"
#include "VisionInspectionGeometry.h"
#include "VisionInspectionGeometryPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum GridChildItemID
{
    ITEM_ID_IMAGE_COMBINE,
    ITEM_ID_INSPECTION_ADD,
    ITEM_ID_INSPECTION_TYPE,
};

enum PropertyGridItemID
{
    ITEM_ID_INSPECTION_NAME = 1,
    ITEM_ID_PARAM_SETUP,
};

enum enumSetupInfoTab
{
    TAB_INSPECTION_SPEC = 0,
    TAB_INSPECTION_RESULT,
    TAB_DETAIL_RESULT,
    TAB_DEBUG_INFO,
    TAB_TXT_LOG,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionGeometry, CDialog)

CDlgVisionInspectionGeometry::CDlgVisionInspectionGeometry(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionGeometry* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionGeometry::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_imageLotViewRearSide(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonResultDlgRear(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonDetailResultDlgRear(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonLogDlgRear(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_procCommonDebugInfoDlgRear(nullptr)
    , m_propertyGrid(nullptr)
    , m_nCurCommonTab(TAB_INSPECTION_SPEC)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionPara = m_pVisionInsp->m_VisionPara;
    m_pvecSpec = &m_pVisionInsp->m_variableInspectionSpecs;

    //m_pVisionInspParaDlg			= new CDlgVisionInspectionGeometryPara(this);
    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
}

CDlgVisionInspectionGeometry::~CDlgVisionInspectionGeometry()
{
    delete m_imageLotView;
    delete m_imageLotViewRearSide;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonDebugInfoDlgRear;
    delete m_procCommonLogDlg;
    delete m_procCommonLogDlgRear;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonDetailResultDlgRear;
    delete m_procCommonResultDlg;
    delete m_procCommonResultDlgRear;
    delete m_procCommonSpecDlg;
    //delete m_pVisionInspParaDlg;
    delete m_propertyGrid;
    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspectionGeometry::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
    DDX_Control(pDX, IDC_GEOMETRY_LIST, m_ParameterList);
    DDX_Control(pDX, IDC_STATIC_SIDE_FRONT_GEOMETRY, m_Label_Side_Front_Status);
    DDX_Control(pDX, IDC_STATIC_SIDE_REAR_GEOMETRY, m_Label_Side_Rear_Status);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionGeometry, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionGeometry::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionGeometry::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionGeometry::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspectionGeometry::OnCbnSelchangeComboCurrpane)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionGeometry::OnShowDetailResult)
ON_NOTIFY(XTP_NM_GRID_ITEMBUTTONCLICK, IDC_GEOMETRY_LIST, &CDlgVisionInspectionGeometry::OnItemButtonClick)
ON_NOTIFY(XTP_NM_GRID_VALUECHANGED, IDC_GEOMETRY_LIST, &CDlgVisionInspectionGeometry::OnReportValueChanged)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionGeometry::OnImageLotViewPaneSelChanged)
ON_STN_CLICKED(IDC_STATIC_SIDE_FRONT_GEOMETRY, &CDlgVisionInspectionGeometry::OnStnClickedStaticSideFrontGeometry)
ON_STN_CLICKED(IDC_STATIC_SIDE_REAR_GEOMETRY, &CDlgVisionInspectionGeometry::OnStnClickedStaticSideRearGeometry)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionGeometry::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Para Dlg
    CRect rtPropertyGrid;
    rtPropertyGrid = m_procDlgInfo.m_rtParaArea;
    rtPropertyGrid.bottom = rtPropertyGrid.bottom - 400; // Bottom Btn Size 추가

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtPropertyGrid, this, CAST_UINT(IDC_STATIC));
    m_propertyGrid->ShowHelp(FALSE);

    UpdatePropertyGrid();

    CRect rtParameterList;
    rtParameterList = m_procDlgInfo.m_rtParaArea;

    rtParameterList.top = rtPropertyGrid.bottom + 10; // Top Btn Size 추가
    rtParameterList.bottom = rtParameterList.bottom - 10; // Bottom Btn Size 추가
    rtParameterList.left = rtParameterList.left + 10;
    rtParameterList.right = rtParameterList.right - 10;

    m_ParameterList.MoveWindow(rtParameterList);

    m_ParameterList.ResetContent(FALSE);
    m_ParameterList.AddColumn(new CXTPGridColumn(0, _T("Inspection Name"), 250));
    m_ParameterList.AddColumn(new CXTPGridColumn(1, _T("Inspection Setup"), 250));

    m_ParameterList.Populate();
    m_ParameterList.SetTheme(xtpGridThemeVisualStudio2012Light);
    m_ParameterList.SetGridStyle(0, xtpGridSmallDots);
    COLORREF rgbGridColor = RGB(0, 0, 0);
    m_ParameterList.SetGridColor(rgbGridColor);
    m_ParameterList.FocusSubItems(TRUE);
    m_ParameterList.AllowEdit(TRUE);

    long nDataNum = 0;
    void* pData = m_pVisionInsp->m_visionUnit.GetVisionDebugInfo(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_pVisionInsp->m_sEdgeAlignResult = *(VisionAlignResult*)pData;

    ResetListControl();
    //------------

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_variableInspectionSpecs);

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        SetNonSideUI(rtTab);
    }
    else
    {
        SetSideUI(rtTab);
    }

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionInspectionGeometry::SetSideUI(const CRect& DataArea)
{
    CRect rtLotViewRegionF, rtLotViewRegionR;
    rtLotViewRegionF.left = m_procDlgInfo.m_rtImageArea.left;
    rtLotViewRegionF.top = m_procDlgInfo.m_rtImageArea.top;
    rtLotViewRegionF.right = m_procDlgInfo.m_rtImageArea.right;
    rtLotViewRegionF.bottom = (m_procDlgInfo.m_rtImageArea.bottom / 2) - 1;

    rtLotViewRegionR.left = m_procDlgInfo.m_rtImageArea.left;
    rtLotViewRegionR.top = (m_procDlgInfo.m_rtImageArea.bottom / 2) + 1;
    rtLotViewRegionR.right = m_procDlgInfo.m_rtImageArea.right;
    rtLotViewRegionR.bottom = m_procDlgInfo.m_rtImageArea.bottom;

    m_imageLotView = new ImageLotView(
        rtLotViewRegionF, *m_pVisionInsp, GetSafeHwnd(), enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    m_imageLotViewRearSide
        = new ImageLotView(rtLotViewRegionR, *m_pVisionInsp, GetSafeHwnd(), enSideVisionModule::SIDE_VISIONMODULE_REAR);

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_imageLotViewRearSide->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    ////// Side Status Selector 위치 설정
    CRect rtSIdeFrontStatus, rtSIdeRearStatus;

    GetDlgItem(IDC_STATIC_SIDE_FRONT_GEOMETRY)->GetWindowRect(rtSIdeFrontStatus);
    GetDlgItem(IDC_STATIC_SIDE_REAR_GEOMETRY)->GetWindowRect(rtSIdeRearStatus);

    SetDlgItemText(IDC_STATIC_SIDE_FRONT_GEOMETRY, _T("SIDE FRONT"));
    SetDlgItemText(IDC_STATIC_SIDE_REAR_GEOMETRY, _T("SIDE REAR"));

    m_Label_Side_Front_Status.MoveWindow(m_procDlgInfo.m_rtSideFrontStatusArea);

    m_Label_Side_Rear_Status.MoveWindow(m_procDlgInfo.m_rtSideRearStatusArea);

    SetInitialSideVisionSelector();
    ////////////////////////////////////////////////////////

    m_procCommonResultDlg = new ProcCommonResultDlg(this, DataArea, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, DataArea, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, DataArea);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, DataArea, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonResultDlgRear = new ProcCommonResultDlg(this, DataArea, m_pVisionInsp->m_resultGroup,
        *m_imageLotViewRearSide, m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlgRear
        = new ProcCommonDetailResultDlg(this, DataArea, m_pVisionInsp->m_resultGroup, *m_imageLotViewRearSide);
    m_procCommonLogDlgRear = new ProcCommonLogDlg(this, DataArea);
    m_procCommonDebugInfoDlgRear = new ProcCommonDebugInfoDlg(
        this, DataArea, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotViewRearSide, *m_procCommonLogDlg);

    UpdateCommonTabShow();

    SetChangeSideUI(m_pVisionPara->m_nGeometryInspType);
}

void CDlgVisionInspectionGeometry::SetNonSideUI(const CRect& DataArea)
{
    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_procCommonResultDlg = new ProcCommonResultDlg(this, DataArea, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, DataArea, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, DataArea);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, DataArea, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);
}

int CDlgVisionInspectionGeometry::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionInspectionGeometry::OnDestroy()
{
    CDialog::OnDestroy();

    // Normal & Front
    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();

    if (SystemConfig::GetInstance().m_nVisionType == VISIONTYPE_SIDE_INSP)
    {
        // Rear
        m_procCommonDebugInfoDlgRear->DestroyWindow();
        m_procCommonLogDlgRear->DestroyWindow();
        m_procCommonResultDlgRear->DestroyWindow();
        m_procCommonDetailResultDlgRear->DestroyWindow();
    }
    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionGeometry::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionGeometry::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Geometry Measured Parameter")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))->SetID(ITEM_ID_IMAGE_COMBINE);
        category->AddChildItem(new CCustomItemButton(_T("Add Inspection"), TRUE, FALSE))->SetID(ITEM_ID_INSPECTION_ADD);

        if (SystemConfig::GetInstance().m_nVisionType == VISIONTYPE_SIDE_INSP)
        {
            if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Side Vision Insepction Type"),
                    m_pVisionPara->m_nGeometryInspType, (int*)&m_pVisionPara->m_nGeometryInspType)))
            {
                item->GetConstraints()->AddConstraint(_T("Both"), eGeometryInspType::GeometryInspType_Both);
                item->GetConstraints()->AddConstraint(
                    _T("Only Front Vision"), eGeometryInspType::GeometryInspType_Front_Vision);
                item->GetConstraints()->AddConstraint(
                    _T("Only Rear Vision"), eGeometryInspType::GeometryInspType_Rear_Vision);
                item->SetID(ITEM_ID_INSPECTION_TYPE);
            }
        }

        m_propertyGrid->FindItem(ITEM_ID_IMAGE_COMBINE)->SetHeight(50);
        m_propertyGrid->FindItem(ITEM_ID_INSPECTION_ADD)->SetHeight(50);
        category->Expand();
    }
    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

void CDlgVisionInspectionGeometry::ResetListControl()
{
    m_ParameterList.ResetContent(FALSE);
    CString strTemp;
    for (long nInspeID = 0; nInspeID < m_pVisionPara->m_nInspectionNumber; nInspeID++)
    {
        CXTPGridRecord* repRecord = new CXTPGridRecord();

        // Inspection Name
        strTemp.Format(_T("%s"), (LPCTSTR)m_pVisionPara->m_vecstrInspectionName[nInspeID]);
        repRecord->AddItem(new CXTPGridRecordItemText(strTemp));
        // Inspection Setup
        strTemp.Format(_T(""));
        repRecord->AddItem(new CXTPGridRecordItemText(strTemp));
        // Add Record to List Control
        m_ParameterList.AddRecord(repRecord);

        // Add Setup Button
        CXTPGridRecordItem* pItem = repRecord->GetItem(1);
        pItem->SetEditable(0);
        auto* pButton = pItem->GetItemControls()->AddControl(xtpItemControlTypeButton);
        if (!pButton)
            continue;
        pButton->SetIconIndex(PBS_NORMAL, 0);
        pButton->SetIconIndex(PBS_PRESSED, 1);
        pButton->SetCaption(_T("Setup"));
        pButton->SetSize(CSize(181, 0));

        // Add Delete Button
        pButton = pItem->GetItemControls()->AddControl(xtpItemControlTypeButton);
        if (!pButton)
            continue;
        pButton->SetIconIndex(PBS_NORMAL, 0);
        pButton->SetIconIndex(PBS_PRESSED, 1);
        pButton->SetCaption(_T("Delete"));
        pButton->SetSize(CSize(60, 0));
    }

    m_ParameterList.Populate();

    strTemp.Empty();
}

void CDlgVisionInspectionGeometry::OnItemButtonClick(NMHDR* pNotifyStruct, LRESULT*)
{
    XTP_NM_GRIDITEMCONTROL* pItemNotify = (XTP_NM_GRIDITEMCONTROL*)pNotifyStruct;
    if (!(pItemNotify->pRow && pItemNotify->pItem && pItemNotify->pItemControl))
        return;

    if (pItemNotify->pItemControl->GetCaption() == _T("Setup"))
    {
        long nSelectInspectionID = pItemNotify->pRow->GetRecord()->GetIndex();
        CString strTemp;

        CheckData(nSelectInspectionID);
        CDlgGeometryEditor Dlg(m_pVisionInsp, nSelectInspectionID, this);
        if (Dlg.DoModal() == IDOK)
        {
            Dlg.GetParameter(m_pVisionInsp->m_VisionPara, nSelectInspectionID);

            m_pVisionInsp->UpdateSpec();
        }
        strTemp.Empty();
    }
    else if (pItemNotify->pItemControl->GetCaption() == _T("Delete"))
    {
        if (m_pVisionPara->m_nInspectionNumber <= 1)
            return;

        long nSelectInspectionID = pItemNotify->pRow->GetRecord()->GetIndex();

        CString strTemp;
        strTemp.Format(_T("Press the OK button to delete %d ROI Parameter."), nSelectInspectionID);
        if (::SimpleMessage(strTemp, MB_OKCANCEL) == IDCANCEL)
            return;

        long nOldNum = m_pVisionPara->m_nInspectionNumber;
        long nNewNum = nOldNum - 1;

        m_pVisionPara->m_vecstrInspectionName.erase(
            m_pVisionPara->m_vecstrInspectionName.begin() + nSelectInspectionID);
        m_pVisionPara->m_vecnInspectionType.erase(m_pVisionPara->m_vecnInspectionType.begin() + nSelectInspectionID);
        m_pVisionPara->m_vecnInspectionDistanceResult.erase(
            m_pVisionPara->m_vecnInspectionDistanceResult.begin() + nSelectInspectionID);
        m_pVisionPara->m_vecnInspectionCircleResult.erase(
            m_pVisionPara->m_vecnInspectionCircleResult.begin() + nSelectInspectionID);
        m_pVisionPara->m_vecrtInspectionROI_Ref_BCU.erase(
            m_pVisionPara->m_vecrtInspectionROI_Ref_BCU.begin() + nSelectInspectionID);
        m_pVisionPara->m_vecrtInspectionROI_Tar_BCU.erase(
            m_pVisionPara->m_vecrtInspectionROI_Tar_BCU.begin() + nSelectInspectionID);
        m_pVisionPara->m_vecInfoParameter_0_Ref.erase(
            m_pVisionPara->m_vecInfoParameter_0_Ref.begin() + nSelectInspectionID);
        m_pVisionPara->m_vecInfoParameter_0_Tar.erase(
            m_pVisionPara->m_vecInfoParameter_0_Tar.begin() + nSelectInspectionID);

        // 	m_pVisionPara->m_vecInfoParameter_1_Ref.erase(m_pVisionPara->m_vecInfoParameter_1_Ref.begin() + nSelectInspectionID);
        // 	m_pVisionPara->m_vecInfoParameter_1_Tar.erase(m_pVisionPara->m_vecInfoParameter_1_Ref.begin() + nSelectInspectionID);
        // 	m_pVisionPara->m_vecInfoParameter_2_Ref.erase(m_pVisionPara->m_vecInfoParameter_2_Ref.begin() + nSelectInspectionID);
        // 	m_pVisionPara->m_vecInfoParameter_2_Tar.erase(m_pVisionPara->m_vecInfoParameter_2_Ref.begin() + nSelectInspectionID);

        m_pVisionPara->m_nInspectionNumber = nNewNum;

        m_pVisionInsp->DeleteSpec(nSelectInspectionID);

        if (nSelectInspectionID > m_pVisionPara->m_nInspectionNumber)
            nSelectInspectionID = m_pVisionPara->m_nInspectionNumber - 1;

        if (nSelectInspectionID < 0)
            nSelectInspectionID = 0;

        ResetListControl();

        strTemp.Empty();
    }
}

BOOL CDlgVisionInspectionGeometry::CheckData(long nDataNum)
{
    long nRealSize = nDataNum + 1;

    if ((long)m_pVisionPara->m_vecstrInspectionName.size() < nRealSize)
        m_pVisionPara->m_vecstrInspectionName.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecnInspectionType.size() < nRealSize)
        m_pVisionPara->m_vecnInspectionType.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecnInspectionDistanceResult.size() < nRealSize)
        m_pVisionPara->m_vecnInspectionDistanceResult.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecnInspectionCircleResult.size() < nRealSize)
        m_pVisionPara->m_vecnInspectionCircleResult.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecfReferenceSpec.size() < nRealSize)
        m_pVisionPara->m_vecfReferenceSpec.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecInfoParameter_0_Ref.size() < nRealSize)
        m_pVisionPara->m_vecInfoParameter_0_Ref.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecInfoParameter_0_Tar.size() < nRealSize)
        m_pVisionPara->m_vecInfoParameter_0_Tar.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecstrinfoName_Ref.size() < nRealSize)
        m_pVisionPara->m_vecstrinfoName_Ref.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecstrinfoName_Tar.size() < nRealSize)
        m_pVisionPara->m_vecstrinfoName_Tar.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecrtInspectionROI_Ref_BCU.size() < nRealSize)
        m_pVisionPara->m_vecrtInspectionROI_Ref_BCU.resize(nRealSize);

    if ((long)m_pVisionPara->m_vecrtInspectionROI_Tar_BCU.size() < nRealSize)
        m_pVisionPara->m_vecrtInspectionROI_Tar_BCU.resize(nRealSize);

    return TRUE;
}

void CDlgVisionInspectionGeometry::OnReportValueChanged(NMHDR* pNotifyStruct, LRESULT* /*result*/)
{
    XTP_NM_GRIDRECORDITEM* pItemNotify = (XTP_NM_GRIDRECORDITEM*)pNotifyStruct;
    ASSERT(pItemNotify != NULL);

    long nSelectInspectionID = pItemNotify->pRow->GetRecord()->GetIndex();

    CString strSpecNameTemp;
    CString strSpecNameTemp0;
    CString strSpecNameTemp1;
    BOOL bSameName = FALSE;

    srand((unsigned int)time(NULL));
    long nRandomID = rand() % 100;

    strSpecNameTemp = pItemNotify->pItem->GetCaption();
    ;
    strSpecNameTemp0 = strSpecNameTemp;

    strSpecNameTemp0.MakeUpper();

    // 영훈 : 뒤에 띄어쓰기가 포함되어 있으면 같은 문자열인데 다르게 인식 되므로 뒤에 띄어쓰기는 다 없애버리고 시작한다.
    long nCount = 0;
    while (1)
    {
        if (nCount > 100)
            break;

        ++nCount;

        if (strSpecNameTemp0.Right(1) == 32) // 아스키 코드 '띄어쓰기'
        {
            strSpecNameTemp0 = strSpecNameTemp0.Left(strSpecNameTemp0.GetLength() - 1);
            continue;
        }
        if (strSpecNameTemp0.Left(1) == 32) // 아스키 코드 '띄어쓰기'
        {
            strSpecNameTemp0 = strSpecNameTemp0.Right(strSpecNameTemp0.GetLength() - 1);
            continue;
        }

        //strSpecNameTemp = strSpecNameTemp0;
        break;
    }

    for (long nID = 0; nID < (long)m_pVisionPara->m_vecstrInspectionName.size(); nID++)
    {
        if (nSelectInspectionID == nID) // 자기 끼린 비교하지 않는다.
            continue;

        strSpecNameTemp1 = m_pVisionPara->m_vecstrInspectionName[nID];
        strSpecNameTemp1.MakeUpper();

        if (strSpecNameTemp0 == strSpecNameTemp1)
        {
            bSameName = TRUE;
            break;
        }
    }

    if (bSameName == TRUE)
    {
        ::SimpleMessage(_T("It already has the same name.\nPlease check again."), MB_OK);
        m_pVisionPara->m_vecstrInspectionName[nSelectInspectionID].Format(
            _T("%s_%d"), (LPCTSTR)strSpecNameTemp, nRandomID);
    }
    else
    {
        m_pVisionPara->m_vecstrInspectionName[nSelectInspectionID] = strSpecNameTemp;
    }

    (*m_pvecSpec)[nSelectInspectionID].m_specName
        = m_pVisionPara->m_vecstrInspectionName
              [nSelectInspectionID]; // 영훈 20150429 : Geometry 검사는 사용자가 검사항목 이름을 바꿀 수 있도록 한다.
    m_pVisionInsp->UpdateSpec();

    ResetListControl();

    strSpecNameTemp.Empty();
    strSpecNameTemp0.Empty();
    strSpecNameTemp1.Empty();
}

LRESULT CDlgVisionInspectionGeometry::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_IMAGE_COMBINE:
                btnClickImageCombine();
                break;
            case ITEM_ID_INSPECTION_ADD:
                btnClickInspectionAdd();
                break;
        }

        value->SetBool(FALSE);
    }
    else if (auto* EnumValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_INSPECTION_TYPE:
                SetChangeSideUI(m_pVisionPara->m_nGeometryInspType);
                break;
        }
    }

    return 0;
}

void CDlgVisionInspectionGeometry::btnClickImageCombine()
{
    auto* proc = m_pVisionInsp;

    if (::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), false, &proc->m_VisionPara->m_ImageProcMangePara)
        == IDOK)
    {
        ShowImage();
    }
}

void CDlgVisionInspectionGeometry::btnClickInspectionAdd()
{
    long nOldNum = m_pVisionPara->m_nInspectionNumber;
    long nNewNum = nOldNum + 1;

    m_pVisionPara->m_vecstrInspectionName.resize(nNewNum);
    m_pVisionPara->m_vecnInspectionType.resize(nNewNum);
    m_pVisionPara->m_vecnInspectionDistanceResult.resize(nNewNum);
    m_pVisionPara->m_vecnInspectionCircleResult.resize(nNewNum);
    m_pVisionPara->m_vecfReferenceSpec.resize(nNewNum);

    m_pVisionPara->m_vecInfoParameter_0_Ref.resize(nNewNum);
    m_pVisionPara->m_vecInfoParameter_0_Tar.resize(nNewNum);

    m_pVisionPara->m_vecstrinfoName_Ref.resize(nNewNum);
    m_pVisionPara->m_vecstrinfoName_Tar.resize(nNewNum);

    m_pVisionPara->m_vecrtInspectionROI_Ref_BCU.resize(nNewNum);
    m_pVisionPara->m_vecrtInspectionROI_Tar_BCU.resize(nNewNum);

    CString strTemp;
    for (long nCount = nOldNum; nCount < nNewNum; nCount++)
    {
        strTemp.Format(_T("Untitle Name_%d"), nCount);
        m_pVisionPara->m_vecstrInspectionName[nCount] = strTemp;
        m_pVisionPara->m_vecnInspectionType[nCount] = Insp_Type_Shape_Distance;
        m_pVisionPara->m_vecnInspectionDistanceResult[nCount] = Insp_Param_Dist_XY;
        m_pVisionPara->m_vecnInspectionCircleResult[nCount] = Insp_Param_Circle_Radius;
        m_pVisionPara->m_vecfReferenceSpec[nCount] = 0.f;

        m_pVisionPara->m_vecInfoParameter_0_Ref[nCount].clear();
        m_pVisionPara->m_vecInfoParameter_0_Tar[nCount].clear();
        m_pVisionPara->m_vecstrinfoName_Ref[nCount].clear();
        m_pVisionPara->m_vecstrinfoName_Tar[nCount].clear();
        m_pVisionPara->m_vecrtInspectionROI_Ref_BCU[nCount].clear();
        m_pVisionPara->m_vecrtInspectionROI_Tar_BCU[nCount].clear();
    }

    m_pVisionPara->m_nInspectionNumber = nNewNum;
    m_pVisionInsp->UpdateSpec();

    ResetListControl();

    long nSelectInspectionID = m_pVisionPara->m_nInspectionNumber - 1;

    if (nSelectInspectionID < 0)
        nSelectInspectionID = 0;

    strTemp.Empty();
}

void CDlgVisionInspectionGeometry::SetInitButton()
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

void CDlgVisionInspectionGeometry::OnBnClickedButtonInspect()
{
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

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);

    ShowImage(TRUE);
}

void CDlgVisionInspectionGeometry::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDlgVisionInspectionGeometry::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionGeometry::ShowImage(BOOL bChange)
{
    long nVisionType = SystemConfig::GetInstance().GetVisionType();
    bool bIsSideVision = SystemConfig::GetInstance().IsVisionTypeSide();
    bool bIsSideFront
        = m_pVisionInsp->GetCurVisionModule_Status() == enSideVisionModule::SIDE_VISIONMODULE_FRONT ? true : false;

    if (m_pVisionInsp->GetImageFrameCount() == 0)
    {
        return;
    }

    if (!bChange)
    {
        m_nImageID = max(0, m_pVisionInsp->GetImageFrameIndex(0));
    }

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() <= 0)
        return;

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size()
        <= m_nImageID)
        m_nImageID
            = (long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() - 1;

    Ipvm::Image8u& image
        = m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()][m_nImageID];

    Ipvm::Image8u dst(image.GetSizeX(), image.GetSizeY());
    dst.FillZero();

    if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, Ipvm::Rect32s(image),
            m_pVisionInsp->m_VisionPara->m_ImageProcMangePara, dst))
    {
        return;
    }

    CString FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_nImageID);

    if ((bIsSideVision == false && nVisionType != VISIONTYPE_3D_INSP) || (bIsSideVision && bIsSideFront))
    {
        m_imageLotView->Overlay_RemoveAll();
        m_imageLotView->ROI_RemoveAll();

        m_imageLotView->SetImage(dst, FrameToString);
        m_imageLotView->ZoomImageFit();
    }
    else if (bIsSideVision && bIsSideFront == false)
    {
        m_imageLotViewRearSide->Overlay_RemoveAll();
        m_imageLotViewRearSide->ROI_RemoveAll();

        m_imageLotViewRearSide->SetImage(dst, FrameToString);
        m_imageLotViewRearSide->ZoomImageFit();
    }

    FrameToString.Empty();
}

void CDlgVisionInspectionGeometry::OnCbnSelchangeComboCurrpane()
{
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    long nSel = m_ComboCurrPane.GetCurSel();
    m_pVisionInsp->SetCurrentPaneID(nSel);
    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());

    ShowImage();

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);
}

void CDlgVisionInspectionGeometry::ResetSpecAndResultDlg()
{
    // 새로고침 기능 필요
    m_procCommonSpecDlg->ResetInitGrid();
    m_procCommonResultDlg->Refresh();
    //m_procCommonDetailResultDlg->Refresh();
    //m_procCommonDebugInfoDlg->Refresh();
}

LRESULT CDlgVisionInspectionGeometry::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

LRESULT CDlgVisionInspectionGeometry::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}

void CDlgVisionInspectionGeometry::UpdateCommonTabShow()
{
    const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    m_procCommonSpecDlg->ShowWindow(SW_HIDE);

    m_procCommonResultDlg->ShowWindow(SW_HIDE);
    m_procCommonDetailResultDlg->ShowWindow(SW_HIDE);
    m_procCommonDebugInfoDlg->ShowWindow(SW_HIDE);
    m_procCommonLogDlg->ShowWindow(SW_HIDE);

    m_procCommonResultDlgRear->ShowWindow(SW_HIDE);
    m_procCommonDetailResultDlgRear->ShowWindow(SW_HIDE);
    m_procCommonDebugInfoDlgRear->ShowWindow(SW_HIDE);
    m_procCommonLogDlgRear->ShowWindow(SW_HIDE);

    if (m_nCurCommonTab == TAB_INSPECTION_SPEC)
    {
        m_procCommonSpecDlg->ShowWindow(SW_SHOW);
        return;
    }

    if (nSideSection == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        switch (m_nCurCommonTab)
        {
            case TAB_INSPECTION_RESULT:
                m_procCommonResultDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_DETAIL_RESULT:
                m_procCommonDetailResultDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_DEBUG_INFO:
                m_procCommonDebugInfoDlgRear->ShowWindow(SW_SHOW);
                break;
            case TAB_TXT_LOG:
                m_procCommonLogDlgRear->ShowWindow(SW_SHOW);
                break;
            default:
                m_procCommonSpecDlg->ShowWindow(SW_SHOW);
                break;
        }
    }
    else
    {
        switch (m_nCurCommonTab)
        {
            case TAB_INSPECTION_RESULT:
                m_procCommonResultDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_DETAIL_RESULT:
                m_procCommonDetailResultDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_DEBUG_INFO:
                m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);
                break;
            case TAB_TXT_LOG:
                m_procCommonLogDlg->ShowWindow(SW_SHOW);
                break;
            default:
                m_procCommonSpecDlg->ShowWindow(SW_SHOW);
                break;
        }
    }
}

void CDlgVisionInspectionGeometry::SetInitialSideVisionSelector()
{
    // Default Color 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    const long nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    // 혹시 모를 쓰레기 값을 방지하기 위해 FRONT/REAR 값 이외의 값이 들어오면 무조건 FRONT로 변경시킨다.
    switch (nSideSection)
    {
        case enSideVisionModule::SIDE_VISIONMODULE_REAR:
            m_Label_Side_Front_Status.SetBkColor(defColor);
            m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
            break;

        case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
        default:
            m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
            m_Label_Side_Rear_Status.SetBkColor(defColor);
            break;
    }
}

void CDlgVisionInspectionGeometry::OnStnClickedStaticSideFrontGeometry()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_pVisionInsp->GetCurVisionModule_Status() != enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
        m_Label_Side_Rear_Status.SetBkColor(defColor);

        m_pVisionInsp->m_visionUnit.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        m_pVisionInsp->SetCurVisionModule_Status(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }
    else
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
    }

    if (m_pVisionPara->m_nGeometryInspType == eGeometryInspType::GeometryInspType_Front_Vision)
        m_Label_Side_Rear_Status.SetBkColor(RGB(128, 128, 128));

    m_pVisionInsp->ResetDebugInfo();

    if (m_pVisionInsp->m_visionUnit.RunInspectionInTeachMode(
            m_pVisionInsp, true, m_pVisionInsp->GetCurVisionModule_Status()))
    {
        ShowImage();
    }

    //m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    //m_imageLotView->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionInspectionGeometry::OnStnClickedStaticSideRearGeometry()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_pVisionInsp->GetCurVisionModule_Status() != enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_Label_Side_Front_Status.SetBkColor(defColor);
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));

        m_pVisionInsp->m_visionUnit.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_REAR);
        m_pVisionInsp->SetCurVisionModule_Status(enSideVisionModule::SIDE_VISIONMODULE_REAR);
    }
    else
    {
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
    }

    if (m_pVisionPara->m_nGeometryInspType == eGeometryInspType::GeometryInspType_Rear_Vision)
        m_Label_Side_Front_Status.SetBkColor(RGB(128, 128, 128));

    m_pVisionInsp->ResetDebugInfo();

    if (m_pVisionInsp->m_visionUnit.RunInspectionInTeachMode(
            m_pVisionInsp, true, m_pVisionInsp->GetCurVisionModule_Status()))
    {
        ShowImage();
    }

    //m_imageLotViewRearSide->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    //m_imageLotViewRearSide->ZoomImageFit();

    UpdateCommonTabShow();
}

void CDlgVisionInspectionGeometry::SetChangeSideUI(const long& SelectOnlySIdeType)
{
    switch (SelectOnlySIdeType)
    {
        case eGeometryInspType::GeometryInspType_Both:
            m_Label_Side_Front_Status.SetDisabled(FALSE);
            m_Label_Side_Rear_Status.SetDisabled(FALSE);
            OnStnClickedStaticSideFrontGeometry();
            break;
        case eGeometryInspType::GeometryInspType_Front_Vision:
            m_Label_Side_Front_Status.SetDisabled(FALSE);
            m_Label_Side_Rear_Status.SetDisabled(TRUE);
            OnStnClickedStaticSideFrontGeometry();
            break;
        case eGeometryInspType::GeometryInspType_Rear_Vision:
            m_Label_Side_Front_Status.SetDisabled(TRUE);
            m_Label_Side_Rear_Status.SetDisabled(FALSE);
            OnStnClickedStaticSideRearGeometry();
            break;

        default:
            break;
    }
}
