//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionLgaBottom3D.h"

//CPP_2_________________________________ This project's headers
#include "Dlg3DView.h"
#include "DlgLandParameter.h"
#include "DlgLucpParameter.h"
#include "VisionInspectionLgaBottom3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
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
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID
{
    ITEM_ID_BLOBSEARCH_IMAGE = 1,
    ITEM_ID_REVERSE_THRESHOLD,
    ITEM_ID_GROUP_VIEWER,
    ITEM_ID_SEARCH_SR_EXTENSION_RATIO,
    ITEM_ID_SR_IGNORE_OFFSET_X,
    ITEM_ID_SR_IGNORE_OFFSET_Y,
    ITEM_ID_SR_REF_POS,
    ITEM_ID_SR_ALGORITHM = 9,
    ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN,
    ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX,
    ITEM_ID_GROUP_LUCP_ADD_BUTTON,
    ITEM_ID_GROUP_ID = 100,
    ITEM_ID_LUCP_GROUP_ID = 1000,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionLgaBottom3D, CDialog)

CDlgVisionInspectionLgaBottom3D::CDlgVisionInspectionLgaBottom3D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionLgaBottom3D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionLgaBottom3D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_propertyGrid(nullptr)
{
    m_pVisionInsp = pVisionInsp;

    m_pVisionPara = &m_pVisionInsp->m_VisionPara;
    m_p3DViewer = NULL;
    m_nCmbSelImage = 0;

    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
    m_nSelectGroupID = 0;
}

CDlgVisionInspectionLgaBottom3D::~CDlgVisionInspectionLgaBottom3D()
{
    m_pVisionInsp->SetVisionInspectionSpecs();

    delete m_imageLotView;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;

    m_pVisionInsp->m_pVisionInspDlg = NULL;

    delete m_propertyGrid;
}

void CDlgVisionInspectionLgaBottom3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionLgaBottom3D, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionLgaBottom3D::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionLgaBottom3D::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionLgaBottom3D::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspectionLgaBottom3D::OnCbnSelchangeComboCurrpane)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionLgaBottom3D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgVisionInspectionLgaBottom3D::OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionLgaBottom3D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionLgaBottom3D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtPropertyGrid;
    rtPropertyGrid = m_procDlgInfo.m_rtParaArea;
    rtPropertyGrid.top = rtPropertyGrid.top + 5;

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtPropertyGrid, this, CAST_UINT(IDC_STATIC));

    UpdatePropertyGrid();

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(
        this, rtTab, m_pVisionPara->m_vecVisionInspectionSpecs); //m_pVisionInsp->m_fixedInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionLgaBottom3D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionInspectionLgaBottom3D::OnDestroy()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionLgaBottom3D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionLgaBottom3D::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Land 3D Align Parameter")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Land Center Align Image"),
                m_pVisionPara->m_nBlobSearchImage, (int*)&m_pVisionPara->m_nBlobSearchImage)))
        {
            item->GetConstraints()->AddConstraint(_T("GrayImage"), 0);
            item->GetConstraints()->AddConstraint(_T("V-Map"), 1);

            item->SetID(ITEM_ID_BLOBSEARCH_IMAGE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Use Reverse Threshold"), m_pVisionPara->isReverseThreshold, &m_pVisionPara->isReverseThreshold)))
        {
            item->GetConstraints()->AddConstraint(_T("Not Apply"), FALSE);
            item->GetConstraints()->AddConstraint(_T("Apply"), TRUE);

            item->SetID(ITEM_ID_REVERSE_THRESHOLD);
        }

        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Land Group Viewer"), m_nSelectGroupID)))
        {
            for (int i = 0; i < long(m_pVisionInsp->m_group_id_list.size()); i++)
            {
                CString groupID = m_pVisionInsp->m_group_id_list[i];

                CString str;
                long realidx = i + 1;

                str.Format(_T("Group %d (%s)"), realidx, LPCTSTR(groupID));

                item->GetConstraints()->AddConstraint(str, i);

                groupID.Empty();
                str.Empty();
            }

            item->SetID(ITEM_ID_GROUP_VIEWER);
        }

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Land 3D SR Parameter")))
    {
        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("SR Visibility Lower Bound (GV)"),
            m_pVisionPara->m_SR_visibilityLowerBound, _T("%.2lf"), &m_pVisionPara->m_SR_visibilityLowerBound));

        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("SR Search Extension Area (%)"), m_pVisionPara->m_fSRSearchExtRatio, _T("%.2f")))
            ->SetID(ITEM_ID_SEARCH_SR_EXTENSION_RATIO);

        category
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("SR Ignore Offset X (um)"),
                m_pVisionPara->m_SR_IgnoreOffsetXum, _T("%.0f"), m_pVisionPara->m_SR_IgnoreOffsetXum, 0.f, 10000.f,
                1.f))
            ->SetID(ITEM_ID_SR_IGNORE_OFFSET_X); //kircheis_Tan
        category
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("SR Ignore Offset Y (um)"),
                m_pVisionPara->m_SR_IgnoreOffsetYum, _T("%.0f"), m_pVisionPara->m_SR_IgnoreOffsetYum, 0.f, 10000.f,
                1.f))
            ->SetID(ITEM_ID_SR_IGNORE_OFFSET_Y); //kircheis_Tan

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("SR Z Algorithm"), m_pVisionPara->m_nSR_Algorithm, &m_pVisionPara->m_nSR_Algorithm)))
        {
            item->GetConstraints()->AddConstraint(_T("Plane"), FALSE);
            item->GetConstraints()->AddConstraint(_T("Avr"), TRUE);
            item->SetID(ITEM_ID_SR_ALGORITHM);
        }
        category
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("SR Top Z-Value Average Percent Min(%)"),
                m_pVisionPara->m_SR_topPercentAverage_Min, _T("%.0f"), m_pVisionPara->m_SR_topPercentAverage_Min, 0.f,
                100.f, 0.1f))
            ->SetID(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN); //kircheis_Tan
        category
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("SR Top Z-Value Average Percent Max(%)"),
                m_pVisionPara->m_SR_topPercentAverage_Max, _T("%.0f"), m_pVisionPara->m_SR_topPercentAverage_Max, 0.f,
                100.f, 0.1f))
            ->SetID(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX); //kircheis_Tan

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Group Parameter")))
    {
        for (int i = 0; i < long(m_pVisionInsp->m_group_id_list.size()); i++)
        {
            CString groupID = m_pVisionInsp->m_group_id_list[i];

            CString str;
            str.Format(_T("Group %d (%s)"), i + 1, LPCTSTR(groupID));

            if (auto* Item = category->AddChildItem(new CCustomItemButton(str, FALSE, FALSE)))
                Item->SetID(ITEM_ID_GROUP_ID + i);

            groupID.Empty();
            str.Empty();
        }

        category->Expand();
    }
    if (m_pVisionInsp->m_group_id_list.size() > 1)
    {
        if (auto* category = m_propertyGrid->AddCategory(_T("Group Unit Copl Parameter")))
        {
            if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Inspection Add"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_GROUP_LUCP_ADD_BUTTON);

            category->Expand();
        }
        m_LUCPCategory = m_propertyGrid->AddCategory(_T("Group Unit Copl List"));
        if (m_LUCPCategory != nullptr)
        {
            UpdateLUCPGroup();

            m_LUCPCategory->Expand();
        }
    }

    if (m_pVisionPara->m_nSR_Algorithm == 0)
    {
        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX)->SetReadOnly(TRUE);
    }
    else
    {
        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN)->SetReadOnly(FALSE);
        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX)->SetReadOnly(FALSE);
    }

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

void CDlgVisionInspectionLgaBottom3D::UpdateLUCPGroup()
{
    long InspCount = m_pVisionPara->m_nGroupUnitCoplCount;

    if (m_pVisionPara->m_vecstrGroupUnitCoplName.size() != InspCount)
    {
        m_pVisionPara->m_vecstrGroupUnitCoplName.clear();
        m_pVisionPara->m_vecstrGroupUnitCoplName.resize(InspCount);

        for (int i = 0; i < InspCount; i++)
        {
            m_pVisionInsp->MakeInspectionName(i, m_pVisionPara->m_vecstrGroupUnitCoplName[i]);
        }
    }

    m_LUCPCategory->GetChilds()->Clear();

#define Group_MaxSize 10

    if (InspCount > Group_MaxSize)
    {
        return;
    }

    int ButtonWidth = int((m_procDlgInfo.m_rtParaArea.Width() * 0.2f) + 0.5f);

    for (int i = 0; i < InspCount; i++)
    {
        if (auto* Item
            = m_LUCPCategory->AddChildItem(new CXTPPropertyGridItem(m_pVisionPara->m_vecstrGroupUnitCoplName[i])))
        {
            auto* button1 = new CXTPPropertyGridInplaceButton(ITEM_ID_LUCP_GROUP_ID + Group_MaxSize + i);
            auto* button2 = new CXTPPropertyGridInplaceButton(ITEM_ID_LUCP_GROUP_ID + (Group_MaxSize * 2) + i);
            Item->GetInplaceButtons()->AddButton(button1);
            Item->GetInplaceButtons()->AddButton(button2);
            button1->SetShowAlways(TRUE);
            button1->SetCaption(_T("Setup"));
            button1->SetWidth(ButtonWidth);
            button2->SetShowAlways(TRUE);
            button2->SetCaption(_T("Delete"));
            button2->SetWidth(ButtonWidth);
            Item->SetID(ITEM_ID_LUCP_GROUP_ID + i);
        }
    }
}

LRESULT CDlgVisionInspectionLgaBottom3D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CString strTemp;

    if (wparam != XTP_PGN_ITEMVALUE_CHANGED && wparam != XTP_PGN_INPLACEBUTTONDOWN)
        return 0;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

        if (auto* dValue = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
        {
            double data = dValue->GetDouble();
            switch (item->GetID())
            {
                case ITEM_ID_SEARCH_SR_EXTENSION_RATIO:
                    m_pVisionPara->m_fSRSearchExtRatio = (float)data;
                    break;
            }
        }
        auto& para = m_pVisionInsp->m_VisionPara;
        switch (item->GetID())
        {
            case ITEM_ID_SR_IGNORE_OFFSET_X:
            case ITEM_ID_SR_IGNORE_OFFSET_Y:
            {
                std::vector<Ipvm::Quadrangle32r> vecqrtBody(1);
                m_pVisionInsp->CollectBodyAlignResult();
                m_pVisionInsp->GetSrIgnoreQRT(vecqrtBody[0]);
                m_imageLotView->Overlay_RemoveAll();
                m_imageLotView->Overlay_AddRectangles(vecqrtBody, RGB(0, 255, 0));
                m_imageLotView->Overlay_Show(TRUE);
            }
            break;
            case ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    if (value->GetDouble() < 0 || value->GetDouble() > 99)
                    {
                        para.m_SR_topPercentAverage_Min = 0;
                        strTemp.Format(_T("%.0lf"), para.m_SR_topPercentAverage_Min);
                        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN)->SetValue(strTemp);
                    }
                    else
                        para.m_SR_topPercentAverage_Min = CAST_FLOAT(value->GetDouble());
                break;
            case ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    if (value->GetDouble() < 1 || value->GetDouble() > 100)
                    {
                        para.m_SR_topPercentAverage_Max = 100;
                        strTemp.Format(_T("%.0lf"), para.m_SR_topPercentAverage_Max);
                        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX)->SetValue(strTemp);
                    }
                    else
                        para.m_SR_topPercentAverage_Max = CAST_FLOAT(value->GetDouble());
                break;
        }

        if (auto* dValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
        {
            double data = dValue->GetEnum();
            switch (item->GetID())
            {
                case ITEM_ID_BLOBSEARCH_IMAGE:
                    m_pVisionPara->m_nBlobSearchImage = (long)data;
                    break;
                case ITEM_ID_REVERSE_THRESHOLD:
                    m_pVisionPara->isReverseThreshold = (BOOL)data;
                    break;
                case ITEM_ID_SR_ALGORITHM:
                {
                    if (m_pVisionPara->m_nSR_Algorithm == 0)
                    {
                        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN)->SetReadOnly(TRUE);
                        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX)->SetReadOnly(TRUE);
                    }
                    else
                    {
                        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN)->SetReadOnly(FALSE);
                        m_propertyGrid->GetItem(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX)->SetReadOnly(FALSE);
                    }
                }
                break;
                case ITEM_ID_GROUP_VIEWER:
                {
                    m_nSelectGroupID = (long)data;

                    std::vector<PI_RECT> LandSpecROI;

                    MakeSpecROI(m_nSelectGroupID, LandSpecROI);

                    m_imageLotView->Overlay_RemoveAll();

                    for (auto Land : LandSpecROI)
                    {
                        m_imageLotView->Overlay_AddRectangle(Land, RGB(0, 255, 0));
                    }

                    m_imageLotView->Overlay_Show(TRUE);
                }
                break;
            }
        }

        if (auto* value = dynamic_cast<CCustomItemButton*>(item))
        {
            UINT ID = item->GetID();

            if (ID == ITEM_ID_GROUP_LUCP_ADD_BUTTON)
            {
                btnClickInspectionAdd();
            }
            else if (ID >= ITEM_ID_GROUP_ID && ID < ITEM_ID_LUCP_GROUP_ID)
            {
                if (ID < 0)
                    return -1;

                long nRealGroupID = item->GetID() - ITEM_ID_GROUP_ID; //ID를 찾기위해 어쩔수없다..

                if (nRealGroupID < 0)
                    return -1;

                DlgLandParameter DlgGroupSetupParam(m_pVisionPara, nRealGroupID);

                DlgGroupSetupParam.DoModal();
            }
        }
    }
    else if (wparam == XTP_PGN_INPLACEBUTTONDOWN)
    {
        CXTPPropertyGridInplaceButton* item = (CXTPPropertyGridInplaceButton*)lparam;

        UINT ID = item->GetID();

        if (ID >= ITEM_ID_LUCP_GROUP_ID + Group_MaxSize && ID < ITEM_ID_LUCP_GROUP_ID + (Group_MaxSize * 2))
        {
            long TargetIndex = ID - (ITEM_ID_LUCP_GROUP_ID + Group_MaxSize);
            btnClickInspectionSetup(TargetIndex);
        }
        else if (ID >= ITEM_ID_LUCP_GROUP_ID + (Group_MaxSize * 2) && ID < ITEM_ID_LUCP_GROUP_ID + (Group_MaxSize * 3))
        {
            long TargetIndex = ID - (ITEM_ID_LUCP_GROUP_ID + (Group_MaxSize * 2));
            btnClickInspectionDel(TargetIndex);
        }
    }

    m_propertyGrid->Refresh();
    strTemp.Empty();

    return 0;
}

void CDlgVisionInspectionLgaBottom3D::OnBnClickedBtnOpen3dViewer()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    m_imageLotView->ROI_RemoveAll();

    Ipvm::Rect32s rtROI = Ipvm::Rect32s(100, 100, 300, 300);
    //if(m_pSelChip != NULL)
    //{
    //	rtROI = m_pSelChip->sfrtChip.GetCRect();
    //	rtROI.InflateRect(50, 50);
    //}

    m_imageLotView->ROI_Add(_T(""), _T("ROI"), rtROI, RGB(255, 0, 255), TRUE, TRUE);
    m_imageLotView->ROI_Show(TRUE);

    if (m_p3DViewer == NULL)
    {
        m_p3DViewer = new CDlg3DView(*m_imageLotView, m_pVisionInsp->getImageLotInsp());
        m_p3DViewer->Create(CDlg3DView::IDD);
    }

    m_p3DViewer->ShowWindow(SW_SHOW);
}

void CDlgVisionInspectionLgaBottom3D::SetInitButton()
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

void CDlgVisionInspectionLgaBottom3D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDlgVisionInspectionLgaBottom3D::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    m_pVisionInsp->SetVisionInspectionSpecs();

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());

    // 검사 결과 화면 갱신
    m_procCommonResultDlg->Refresh();
    m_procCommonDebugInfoDlg->Refresh();
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
    ShowImage();
}

void CDlgVisionInspectionLgaBottom3D::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionLgaBottom3D::ShowImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    m_imageLotView->SetImage(
        m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()][0], _T("Frame1"));
}

void CDlgVisionInspectionLgaBottom3D::OnCbnSelchangeComboCurrpane()
{
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    long nSel = m_ComboCurrPane.GetCurSel();
    m_pVisionInsp->SetCurrentPaneID(nSel);
    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());

    // 검사 중간 결과 화면 갱신
    //m_pVisionInspDebugInfoDlg->UpdateDebugInfoColor();

    // 검사 결과 화면 갱신
    //m_pVisionInspResultDlg->UpdateResultData();

    // 검사 리스트 및 Source 이미지 계산.
    m_pVisionInsp->MakePreInspInfo(true);

    ShowImage();

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);
    //m_imageLotView->SetFieldOfView(rtPane);
}

LRESULT CDlgVisionInspectionLgaBottom3D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

LRESULT CDlgVisionInspectionLgaBottom3D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}

void CDlgVisionInspectionLgaBottom3D::ResetSpecAndResultDlg()
{
    m_procCommonSpecDlg->ResetInitGrid();
    m_procCommonResultDlg->Refresh();
}

void CDlgVisionInspectionLgaBottom3D::btnClickInspectionAdd()
{
    long nOldNum = m_pVisionPara->m_nGroupUnitCoplCount;
    long nNewNum = nOldNum + 1;
    CString strTemp;

    if (nNewNum > Group_MaxSize)
        return;

    m_pVisionPara->m_vec2UnitCoplGroup.resize(nNewNum);
    m_pVisionPara->m_vecstrGroupUnitCoplName.resize(nNewNum);

    m_pVisionInsp->MakeInspectionName(nNewNum - 1, m_pVisionPara->m_vecstrGroupUnitCoplName[nNewNum - 1]);

    m_pVisionPara->m_nGroupUnitCoplCount = nNewNum;
    m_pVisionInsp->UpdateSpec(m_pVisionPara->m_vecstrGroupUnitCoplName[nNewNum - 1]);

    UpdateLUCPGroup();
    strTemp.Empty();
}

void CDlgVisionInspectionLgaBottom3D::btnClickInspectionDel(long Targetindex)
{
    long nOldNum = m_pVisionPara->m_nGroupUnitCoplCount;
    long nNewNum = nOldNum - 1;

    if (nNewNum < 0)
        return;

    m_pVisionPara->m_vec2UnitCoplGroup.erase(m_pVisionPara->m_vec2UnitCoplGroup.begin() + Targetindex);
    m_pVisionPara->m_vecstrGroupUnitCoplName.erase(m_pVisionPara->m_vecstrGroupUnitCoplName.begin() + Targetindex);

    m_pVisionPara->m_nGroupUnitCoplCount = nNewNum;
    m_pVisionInsp->DeleteSpec(Targetindex);

    UpdateLUCPGroup();
}

void CDlgVisionInspectionLgaBottom3D::btnClickInspectionSetup(long Targetindex)
{
    DlgLucpParameter dlg(m_pVisionPara, m_pVisionInsp->m_group_id_list, Targetindex);

    if (dlg.DoModal() == IDOK)
    {
        m_pVisionInsp->MakeInspectionName(Targetindex, m_pVisionPara->m_vecstrGroupUnitCoplName[Targetindex]);

        m_pVisionInsp->UpdateSpec(m_pVisionPara->m_vecstrGroupUnitCoplName[Targetindex]);

        UpdateLUCPGroup();
    }
}

BOOL CDlgVisionInspectionLgaBottom3D::MakeSpecROI(long i_nGroupID, std::vector<PI_RECT>& o_vecpirtSpecLand)
{
    if (m_pVisionInsp->m_vec2LandData.size() <= 0)
        return FALSE;

    const auto& mm2px = m_pVisionInsp->getScale().mmToPixel();
    float fPixelperMM = (mm2px.m_x + mm2px.m_y) / 2.f;

    float fBodyAngle = 0.f;
    Ipvm::Rect32r frtBody;
    Ipvm::Point32r2 fptBodyCenter;
    if (!m_pVisionInsp->GetBodyAlignInfo(fBodyAngle, frtBody, fptBodyCenter))
        return FALSE;

    for (auto LandMapData : m_pVisionInsp->m_vec2LandData[i_nGroupID])
    {
        float fLandHalfWidth = LandMapData.fWidth * fPixelperMM / 2.f;
        float fLandHalfLength = LandMapData.fLength * fPixelperMM / 2.f;
        float fLandAngle = (float)LandMapData.nAngle;

        float fX = LandMapData.fOffsetX * mm2px.m_x;
        float fY = (LandMapData.fOffsetY * mm2px.m_y) * -1.f;

        FPI_RECT sfrtLand;
        Ipvm::Point32r2 fCenter(fX + fptBodyCenter.m_x, fY + fptBodyCenter.m_y);
        Ipvm::Rect32r frtROI;
        frtROI.m_left = (fCenter.m_x - fLandHalfWidth);
        frtROI.m_top = (fCenter.m_y - fLandHalfLength);
        frtROI.m_right = (fCenter.m_x + fLandHalfWidth);
        frtROI.m_bottom = (fCenter.m_y + fLandHalfLength);

        sfrtLand = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
            Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

        if (fLandAngle != 0)
        {
            float fAngle = fLandAngle * ITP_DEG_TO_RAD;

            // 시계 반대 방향이 +
            sfrtLand = sfrtLand.Rotate(fAngle);
        }

        o_vecpirtSpecLand.push_back(sfrtLand.GetSPI_RECT());
    }
    return TRUE;
}
