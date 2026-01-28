//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionBgaBottom3D.h"

//CPP_2_________________________________ This project's headers
#include "CDlgBall3DGroupParam.h"
#include "CDlgBucpParameter.h"
#include "VisionInspectionBgaBottom3D.h"
#include "VisionInspectionBgaBottom3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define Group_MaxSize 10
#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID
{
    ITEM_ID_LOGICAL_PIXEL_VISIBILITY_LOWER_START = 1,
    ITEM_ID_LOGICAL_PIXEL_VISIBILITY_LOWER_END,
    ITEM_ID_LOGICAL_PIXEL_VISIBILITY_UPPER_START,
    ITEM_ID_LOGICAL_PIXEL_VISIBILITY_UPPER_END,
    ITEM_ID_VISIBILITY_LOWER_BOUND,
    ITEM_ID_INVALIDATE_SINGLE_POINT = 7, //kircheis_loss3D
    ITEM_ID_SR_Z_ALGORITHM = 8,
    ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN,
    ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX,
    ITEM_ID_SR_IGNORE_OFFSET_X,
    ITEM_ID_SR_IGNORE_OFFSET_Y,
    ITEM_ID_GROUP_VIEWER,
    ITEM_ID_GROUP_BUCP_ADD_BUTTON,
    ITEM_ID_GROUP_ID = 100,
    ITEM_ID_BUCP_GROUP_ID = 1000,
    ITEM_ID_END,
};

enum PropertyGridItemIDGroup
{
    ITEM_ID_GROUPIDX = ITEM_ID_END,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionBgaBottom3D, CDialog)

CDlgVisionInspectionBgaBottom3D::CDlgVisionInspectionBgaBottom3D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionBgaBottom3D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionBgaBottom3D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_nSelectGroupID(0)
{
}

CDlgVisionInspectionBgaBottom3D::~CDlgVisionInspectionBgaBottom3D()
{
    m_pVisionInsp->SetVisionInspectionSpecs();

    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    delete m_imageLotView;
}

void CDlgVisionInspectionBgaBottom3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionBgaBottom3D, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionBgaBottom3D::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionBgaBottom3D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionBgaBottom3D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionBgaBottom3D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_pVisionPara->m_vecVisionInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    SetPropertyCommonParam();

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionInspectionBgaBottom3D::SetPropertyCommonParam()
{
    if (auto* category = m_propertyGrid->AddCategory(_T("Common Parameter")))
    {
        auto& para = *m_pVisionInsp->m_pVisionPara;

        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Ball Group Viewer"), m_nSelectGroupID)))
        {
            long typeCount = long(m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes.size());

            for (int i = 0; i < typeCount; i++)
            {
                CString groupInspectionName;
                CString groupID = m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes[i].m_groupID;
                long realidx = i + 1;

                groupInspectionName.Format(_T("Group %d (%s)"), realidx, LPCTSTR(groupID));

                item->GetConstraints()->AddConstraint(groupInspectionName, i);

                groupInspectionName.Empty();
                groupID.Empty();
            }

            item->SetID(ITEM_ID_GROUP_VIEWER);
        }

        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Logical Pixel Visibility Lower Start Bound (Ball)"),
            para.m_logicalPixelVisibilityLowerStartBound_ball, _T("%.2lf"),
            &para.m_logicalPixelVisibilityLowerStartBound_ball));
        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Logical Pixel Visibility Lower End Bound (Ball)"),
            para.m_logicalPixelVisibilityLowerEndBound_ball, _T("%.2lf"),
            &para.m_logicalPixelVisibilityLowerEndBound_ball));
        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Logical Pixel Visibility Upper Start Bound (Ball)"),
            para.m_logicalPixelVisibilityUpperStartBound_ball, _T("%.2lf"),
            &para.m_logicalPixelVisibilityUpperStartBound_ball));
        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Logical Pixel Visibility Upper End Bound (Ball)"),
            para.m_logicalPixelVisibilityUpperEndBound_ball, _T("%.2lf"),
            &para.m_logicalPixelVisibilityUpperEndBound_ball));
        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Visibility Lower Bound (SR)"),
            para.m_visibilityLowerBound_sr, _T("%.2lf"), &para.m_visibilityLowerBound_sr));

        //{{ //kircheis_loss3D
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Invalidate single 3D point data"),
                para.m_nInvalidateSinglePointData, &para.m_nInvalidateSinglePointData)))
        {
            item->GetConstraints()->AddConstraint(_T("Not use"), FALSE);
            item->GetConstraints()->AddConstraint(_T("Use"), TRUE);

            item->SetID(ITEM_ID_INVALIDATE_SINGLE_POINT);
        }
        //}}
        //{{//kircheis_Tan
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("SR Z Algorithm"), para.m_nSR_Algorithm, &para.m_nSR_Algorithm)))
        {
            item->GetConstraints()->AddConstraint(_T("Plane"), FALSE);
            item->GetConstraints()->AddConstraint(_T("Avr"), TRUE);

            item->SetID(ITEM_ID_SR_Z_ALGORITHM);
        }

        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(_T("SR Top Z-Value Average Percent Min(%)"),
                para.m_SR_topPercentAverage_Min, _T("%.2lf"), &para.m_SR_topPercentAverage_Min))
            ->SetID(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MIN);
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(_T("SR Top Z-Value Average Percent Max(%)"),
                para.m_SR_topPercentAverage_Max, _T("%.2lf"), &para.m_SR_topPercentAverage_Max))
            ->SetID(ITEM_ID_SR_TOP_Z_AVR_PERCENT_MAX);

        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("SR Ignore Offset X (um)"), para.m_SR_IgnoreOffsetXum, _T("%.0lf"), &para.m_SR_IgnoreOffsetXum))
            ->SetID(ITEM_ID_SR_IGNORE_OFFSET_X);
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("SR Ignore Offset Y (um)"), para.m_SR_IgnoreOffsetYum, _T("%.0lf"), &para.m_SR_IgnoreOffsetYum))
            ->SetID(ITEM_ID_SR_IGNORE_OFFSET_Y);
        //}}

        category->Expand();

        if (para.m_nSR_Algorithm == 0)
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

    SetPropertyGroupParam();

    auto& ballTypes = m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes;
    long GroupNum = (long)ballTypes.size();

    if (GroupNum > 1)
    {
        if (auto* category = m_propertyGrid->AddCategory(_T("Group Unit Copl Parameter")))
        {
            if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Inspection Add"), TRUE, FALSE)))
                Item->SetID(ITEM_ID_GROUP_BUCP_ADD_BUTTON);

            category->Expand();
        }

        m_BUCPCategory = m_propertyGrid->AddCategory(_T("Group Unit Copl List"));
        if (m_BUCPCategory != nullptr)
        {
            UpdateBUCPGroup();

            m_BUCPCategory->Expand();
        }
    }
}

void CDlgVisionInspectionBgaBottom3D::UpdateBUCPGroup()
{
    auto& para = *m_pVisionInsp->m_pVisionPara;

    long InspCount = para.m_nGroupUnitCoplCount;

    if (para.m_vecstrGroupUnitCoplName.size() != InspCount)
    {
        para.m_vecstrGroupUnitCoplName.clear();
        para.m_vecstrGroupUnitCoplName.resize(InspCount);

        for (int i = 0; i < InspCount; i++)
        {
            m_pVisionInsp->MakeInspectionName(i, para.m_vecstrGroupUnitCoplName[i]);
        }
    }

    m_BUCPCategory->GetChilds()->Clear();

    int ButtonWidth = int((m_procDlgInfo.m_rtParaArea.Width() * 0.2f) + 0.5f);

    for (int i = 0; i < InspCount; i++)
    {
        if (auto* Item = m_BUCPCategory->AddChildItem(new CXTPPropertyGridItem(para.m_vecstrGroupUnitCoplName[i])))
        {
            auto* button1 = new CXTPPropertyGridInplaceButton(ITEM_ID_BUCP_GROUP_ID + Group_MaxSize + i);
            auto* button2 = new CXTPPropertyGridInplaceButton(ITEM_ID_BUCP_GROUP_ID + (Group_MaxSize * 2) + i);
            Item->GetInplaceButtons()->AddButton(button1);
            Item->GetInplaceButtons()->AddButton(button2);
            button1->SetShowAlways(TRUE);
            button1->SetCaption(_T("Setup"));
            button1->SetWidth(ButtonWidth);
            button2->SetShowAlways(TRUE);
            button2->SetCaption(_T("Delete"));
            button2->SetWidth(ButtonWidth);
            Item->SetID(ITEM_ID_BUCP_GROUP_ID + i);
        }
    }
}

void CDlgVisionInspectionBgaBottom3D::SetPropertyGroupParam()
{
    auto& ballTypes = m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes;
    m_pVisionInsp->m_pVisionPara->m_parameters.resize(ballTypes.size());

    if (auto* category = m_propertyGrid->AddCategory(_T("Group Parameter")))
    {
        for (long nidx = 0; nidx < long(ballTypes.size()); nidx++)
        {
            if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Parameter"), FALSE, FALSE)))
            {
                CString groupID = ballTypes[nidx].m_groupID;

                long nGroupStartID = nidx + 1; //0번이 아닌 1번부터 시작을..
                CString strGroupidx("");

                CString str;
                str.Format(_T("Diameter : %.2lf um / Height %.2lf um"),
                    m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes[nidx].m_diameter_um,
                    m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes[nidx].m_height_um);

                strGroupidx.Format(_T("Group %d (%s, %s)"), nGroupStartID, LPCTSTR(groupID), LPCTSTR(str));

                item->SetCaption(strGroupidx);
                item->SetID(ITEM_ID_GROUPIDX + nidx); //kircheis_AIBC

                groupID.Empty();
                strGroupidx.Empty();
                str.Empty();
            }
        }

        category->Expand();
    }
}

void CDlgVisionInspectionBgaBottom3D::SetupGroupParam(long i_nSelectGroupID)
{
    if (i_nSelectGroupID < 0)
        return;

    long nRealGroupID = i_nSelectGroupID - ITEM_ID_GROUPIDX; //ID를 찾기위해 어쩔수없다..

    if (nRealGroupID < 0)
        return;

    auto& ballTypes = m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes[nRealGroupID];
    auto& ballparam = m_pVisionInsp->m_pVisionPara->m_parameters[nRealGroupID];

    CDlgBall3DGroupParam DlgGroupSetupParam(this, ballparam, ballTypes, nRealGroupID);
    if (DlgGroupSetupParam.DoModal() == IDOK)
        ballparam = DlgGroupSetupParam.GetGroupParam();
}

void CDlgVisionInspectionBgaBottom3D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionBgaBottom3D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionBgaBottom3D::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    m_pVisionInsp->SetVisionInspectionSpecs();

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
}

void CDlgVisionInspectionBgaBottom3D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

LRESULT CDlgVisionInspectionBgaBottom3D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

LRESULT CDlgVisionInspectionBgaBottom3D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam != XTP_PGN_ITEMVALUE_CHANGED && wparam != XTP_PGN_INPLACEBUTTONDOWN)
        return 0;
    auto& para = *m_pVisionInsp->m_pVisionPara;
    CString strTemp;
    switch (item->GetID())
    {
        case ITEM_ID_GROUP_VIEWER:
        {
            auto* data = dynamic_cast<CXTPPropertyGridItemEnum*>(item);

            m_nSelectGroupID = data->GetEnum();

            std::vector<PI_RECT> BallSpecROI;

            MakeSpecROI(m_nSelectGroupID, BallSpecROI);

            m_imageLotView->Overlay_RemoveAll();

            for (const auto& Ball : BallSpecROI)
            {
                m_imageLotView->Overlay_AddRectangle(Ball, RGB(0, 255, 0));
            }

            m_imageLotView->Overlay_Show(TRUE);
        }
        break;
        case ITEM_ID_SR_IGNORE_OFFSET_X:
        case ITEM_ID_SR_IGNORE_OFFSET_Y:
        {
            std::vector<Ipvm::Quadrangle32r> vecqrtBody(1);
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
                    para.m_SR_topPercentAverage_Min = value->GetDouble();
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
                    para.m_SR_topPercentAverage_Max = value->GetDouble();
            break;
    }
    if (auto* dValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_SR_Z_ALGORITHM:
            {
                if (m_pVisionInsp->m_pVisionPara->m_nSR_Algorithm == 0)
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
        }
    }

    if (auto* value = dynamic_cast<CCustomItemButton*>(item))
    {
        UINT ID = item->GetID();

        if (ID >= ITEM_ID_GROUPIDX)
            SetupGroupParam(item->GetID());

        else if (ID == ITEM_ID_GROUP_BUCP_ADD_BUTTON)
        {
            btnClickInspectionAdd();
        }
        //switch (item->GetID()) //그외
        //{
        //}
    }

    if (wparam == XTP_PGN_INPLACEBUTTONDOWN)
    {
        CXTPPropertyGridInplaceButton* itemGridButton = (CXTPPropertyGridInplaceButton*)lparam;

        UINT ID = itemGridButton->GetID();

        if (ID >= ITEM_ID_BUCP_GROUP_ID + Group_MaxSize && ID < ITEM_ID_BUCP_GROUP_ID + (Group_MaxSize * 2))
        {
            long TargetIndex = ID - (ITEM_ID_BUCP_GROUP_ID + Group_MaxSize);
            btnClickInspectionSetup(TargetIndex);
        }
        else if (ID >= ITEM_ID_BUCP_GROUP_ID + (Group_MaxSize * 2) && ID < ITEM_ID_BUCP_GROUP_ID + (Group_MaxSize * 3))
        {
            long TargetIndex = ID - (ITEM_ID_BUCP_GROUP_ID + (Group_MaxSize * 2));
            btnClickInspectionDel(TargetIndex);
        }
    }

    strTemp.Empty();

    return 0;
}

LRESULT CDlgVisionInspectionBgaBottom3D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}

void CDlgVisionInspectionBgaBottom3D::ViewBallSearchROI(long i_nBallGroupidx)
{
    std::vector<Ipvm::Rect32s> specROIs;
    std::vector<Ipvm::Rect32s> searchROIs;

    m_imageLotView->Overlay_RemoveAll();
    m_pVisionInsp->MakeSpecObjectROI_ByPKGSize(specROIs, searchROIs, i_nBallGroupidx);
    m_imageLotView->Overlay_AddRectangles(searchROIs, RGB(0, 255, 0));
    m_imageLotView->Overlay_Show(TRUE);
}

void CDlgVisionInspectionBgaBottom3D::ResetSpecAndResultDlg()
{
    m_procCommonSpecDlg->ResetInitGrid();
    m_procCommonResultDlg->Refresh();
}

void CDlgVisionInspectionBgaBottom3D::btnClickInspectionAdd()
{
    auto& para = *m_pVisionInsp->m_pVisionPara;

    long nOldNum = para.m_nGroupUnitCoplCount;
    long nNewNum = nOldNum + 1;
    CString strTemp;

    if (nNewNum > Group_MaxSize)
        return;

    para.m_vec2UnitCoplGroup.resize(nNewNum);
    para.m_vecstrGroupUnitCoplName.resize(nNewNum);

    m_pVisionInsp->MakeInspectionName(nNewNum - 1, para.m_vecstrGroupUnitCoplName[nNewNum - 1]);

    para.m_nGroupUnitCoplCount = nNewNum;
    m_pVisionInsp->UpdateSpec();

    UpdateBUCPGroup();

    strTemp.Empty();
}

void CDlgVisionInspectionBgaBottom3D::btnClickInspectionDel(long Targetindex)
{
    auto& para = *m_pVisionInsp->m_pVisionPara;

    long nOldNum = para.m_nGroupUnitCoplCount;
    long nNewNum = nOldNum - 1;

    if (nNewNum < 0)
        return;

    para.m_vec2UnitCoplGroup.erase(para.m_vec2UnitCoplGroup.begin() + Targetindex);
    para.m_vecstrGroupUnitCoplName.erase(para.m_vecstrGroupUnitCoplName.begin() + Targetindex);

    para.m_nGroupUnitCoplCount = nNewNum;
    m_pVisionInsp->DeleteSpec(Targetindex);

    UpdateBUCPGroup();
}

void CDlgVisionInspectionBgaBottom3D::btnClickInspectionSetup(long Targetindex)
{
    auto& para = *m_pVisionInsp->m_pVisionPara;

    std::vector<CString> group_id_list;
    for (auto& ballType : m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes)
    {
        group_id_list.push_back(ballType.m_groupID);
    }

    CDlgBucpParameter dlg(m_pVisionInsp->m_pVisionPara, group_id_list, Targetindex);

    if (dlg.DoModal() == IDOK)
    {
        m_pVisionInsp->MakeInspectionName(Targetindex, para.m_vecstrGroupUnitCoplName[Targetindex]);

        m_pVisionInsp->UpdateSpec();

        UpdateBUCPGroup();
    }

    for (int nIdx = 0; nIdx < group_id_list.size(); nIdx++)
    {
        group_id_list[nIdx].Empty();
    }
}

BOOL CDlgVisionInspectionBgaBottom3D::MakeSpecROI(long i_nGroupID, std::vector<PI_RECT>& o_vecpirtSpecLand)
{
    if (m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes.size() <= 0)
        return FALSE;

    Ipvm::Point32r2 fptBodyCenter = m_pVisionInsp->m_sEdgeAlignResult->m_center;

    for (auto BallMapData : m_pVisionInsp->m_packageSpec.m_ballMap->m_balls)
    {
        if (BallMapData.m_typeIndex != i_nGroupID)
            continue;

        float fX = CAST_FLOAT(BallMapData.m_posX_px);
        float fY = CAST_FLOAT(BallMapData.m_posY_px);

        FPI_RECT sfrtBall;
        Ipvm::Point32r2 fCenter(fX + fptBodyCenter.m_x, fY + fptBodyCenter.m_y);
        Ipvm::Rect32r frtROI;
        frtROI.m_left = CAST_FLOAT(fCenter.m_x - BallMapData.m_radiusX_px);
        frtROI.m_top = CAST_FLOAT(fCenter.m_y - BallMapData.m_radiusY_px);
        frtROI.m_right = CAST_FLOAT(fCenter.m_x + BallMapData.m_radiusX_px);
        frtROI.m_bottom = CAST_FLOAT(fCenter.m_y + BallMapData.m_radiusY_px);

        sfrtBall = FPI_RECT(Ipvm::Point32r2(frtROI.m_left, frtROI.m_top), Ipvm::Point32r2(frtROI.m_right, frtROI.m_top),
            Ipvm::Point32r2(frtROI.m_left, frtROI.m_bottom), Ipvm::Point32r2(frtROI.m_right, frtROI.m_bottom));

        o_vecpirtSpecLand.push_back(sfrtBall.GetSPI_RECT());
    }
    return TRUE;
}
