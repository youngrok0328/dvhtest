//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionBgaBottom2D.h"

//CPP_2_________________________________ This project's headers
#include "CDlgBall2DGroupParam.h"
#include "VisionInspectionBgaBottom2D.h"
#include "VisionInspectionBgaBottom2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID
{
    ITEM_ID_NORMAL_IMAGE_COMBINATION_MODE = 1,
    ITEM_ID_NORMAL_OBLIQUE_IMAGE,
    ITEM_ID_NORMAL_COAXIAL_IMAGE,
    ITEM_ID_QUALITY_IMAGE_COMBINATION_MODE,
    ITEM_ID_QUALITY_OBLIQUE_IMAGE,
    ITEM_ID_QUALITY_COAXIAL_IMAGE,
    ITEM_ID_GROUP_VIEWER,
    ITEM_ID_2NDINSPPARAM_USE_INSPECTION,
    ITEM_ID_2NDINSPPARAM_MATCH_CODE,
    ITEM_ID_END,
};

enum PropertyGridItemIDGroup
{
    ITEM_ID_GROUPIDX = ITEM_ID_END,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionBgaBottom2D, CDialog)

CDlgVisionInspectionBgaBottom2D::CDlgVisionInspectionBgaBottom2D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionBgaBottom2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionBgaBottom2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_pVisionPara)
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

CDlgVisionInspectionBgaBottom2D::~CDlgVisionInspectionBgaBottom2D()
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

void CDlgVisionInspectionBgaBottom2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionBgaBottom2D, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionBgaBottom2D::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionBgaBottom2D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionBgaBottom2D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionBgaBottom2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

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
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    SetPropertyCommonParam();
    SetPropertyGroupParam();

    if (m_pVisionInsp->m_visionUnit.m_systemConfig.m_bUseAiInspection)
    {
        SetPropertyDeepLearnParam(); // ksy deeplearning
    }
    UpdatePropertyState();

    m_propertyGrid->SetViewDivider(0.6); //kircheis_BallFlux
    m_propertyGrid->HighlightChangedItems(TRUE);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionInspectionBgaBottom2D::SetPropertyCommonParam()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Common Parameter")))
    {
        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Ball Group Viewer"), m_nSelectGroupID)))
        {
            long typeCount = long(m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes.size());

            for (int i = 0; i < typeCount; i++)
            {
                CString groupID = m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes[i].m_groupID;
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

    if (auto* category = m_propertyGrid->AddCategory(_T("Ball Inspection Common Parameter")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Image combination mode"),
                m_pVisionPara->m_nImageCombineMode, (int*)&m_pVisionPara->m_nImageCombineMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Default (Oblique - Coaxial)"),
                VisionInspectionBgaBottom2DPara::enumCombineMode::enumCombineMode_Default);
            item->GetConstraints()->AddConstraint(
                _T("Oblique"), VisionInspectionBgaBottom2DPara::enumCombineMode::enumCombineMode_Oblique);
            item->GetConstraints()->AddConstraint(
                _T("Self combination"), VisionInspectionBgaBottom2DPara::enumCombineMode::enumCombineMode_Self);
            item->GetConstraints()->AddConstraint(
                _T("Invert coaxial"), VisionInspectionBgaBottom2DPara::enumCombineMode::enumCombineMode_InvertCoaxial);
            item->GetConstraints()->AddConstraint(_T("Reformed convert coaxial"),
                VisionInspectionBgaBottom2DPara::enumCombineMode::enumCombineMode_ReformedInvertCoaxial);
            item->GetConstraints()->AddConstraint(_T("Reformed coaxial"),
                VisionInspectionBgaBottom2DPara::enumCombineMode::enumCombineMode_ReformedCoaxial);
            item->GetConstraints()->AddConstraint(_T("Improved default"),
                VisionInspectionBgaBottom2DPara::enumCombineMode::
                    enumCombineMode_ImprovedDefault); //kircheis_ImproveBall2D

            item->SetID(ITEM_ID_NORMAL_IMAGE_COMBINATION_MODE);
        }

        m_pVisionPara->m_obliqueImageIndex.makePropertyGridItem(
            category, _T("Oblique image"), ITEM_ID_NORMAL_OBLIQUE_IMAGE);
        m_pVisionPara->m_coaxialImageIndex.makePropertyGridItem(
            category, _T("Coaxial image"), ITEM_ID_NORMAL_COAXIAL_IMAGE);

        category->Expand();
    }
}

void CDlgVisionInspectionBgaBottom2D::SetPropertyGroupParam()
{
    if (auto* category = m_propertyGrid->AddCategory(_T("Ball quality Inspection Common parameters")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Image combination mode"),
                m_pVisionPara->m_qualityCombineMode, (int*)&m_pVisionPara->m_qualityCombineMode)))
        {
            item->GetConstraints()->AddConstraint(
                _T("Default (Oblique - Coaxial)"), VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Default);
            item->GetConstraints()->AddConstraint(
                _T("Oblique"), VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Oblique);
            item->GetConstraints()->AddConstraint(
                _T("Self combination"), VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Self);

            item->SetID(ITEM_ID_QUALITY_IMAGE_COMBINATION_MODE);
        }

        m_pVisionPara->m_qualityObliqueImageIndex.makePropertyGridItem(
            category, _T("Oblique image"), ITEM_ID_QUALITY_OBLIQUE_IMAGE);
        m_pVisionPara->m_qualityCoaxialImageIndex.makePropertyGridItem(
            category, _T("Coaxial image"), ITEM_ID_QUALITY_COAXIAL_IMAGE);

        category->Expand();
    }

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
                CString str;
                str.Format(_T("Diameter : %.2lf um / Height %.2lf um"), ballTypes[nidx].m_diameter_um,
                    ballTypes[nidx].m_height_um);
                CString strGroupidx("");
                strGroupidx.Format(_T("Group %d (%s, %s)"), nGroupStartID, LPCTSTR(groupID), LPCTSTR(str));

                item->SetCaption(strGroupidx);
                item->SetID(ITEM_ID_GROUPIDX + nidx);

                groupID.Empty();
                str.Empty();
                strGroupidx.Empty();
            }
        }

        category->Expand();
    }
}

void CDlgVisionInspectionBgaBottom2D::SetupGroupParam(long i_nSelectGroupID)
{
    if (i_nSelectGroupID < 0)
        return;

    long nRealGroupID = i_nSelectGroupID - ITEM_ID_GROUPIDX; //ID를 찾기위해 어쩔수없다..

    if (nRealGroupID < 0)
        return;

    auto& ballTypes = m_pVisionInsp->m_packageSpec.m_originalballMap->m_ballTypes[nRealGroupID];
    auto& ballparam = m_pVisionPara->m_parameters[nRealGroupID];

    CDlgBall2DGroupParam DlgGroupSetupParam(this, ballparam, ballTypes, nRealGroupID);
    if (DlgGroupSetupParam.DoModal() == IDOK)
        m_pVisionPara->m_parameters[nRealGroupID].Copy(DlgGroupSetupParam.GetGroupParam());
}

void CDlgVisionInspectionBgaBottom2D::SetPropertyDeepLearnParam()
{
    if (auto* DLParam = m_propertyGrid->AddCategory(_T("2nd Inspection Parameter (Deep Learning)")))
    {
        if (auto* Item = DLParam->AddChildItem(new CXTPPropertyGridItemBool(
                _T("Use Inspection"), m_pVisionPara->m_use2ndInspection, &m_pVisionPara->m_use2ndInspection)))
        {
            Item->SetID(ITEM_ID_2NDINSPPARAM_USE_INSPECTION);
        }

        if (auto* Item = DLParam->AddChildItem(new CXTPPropertyGridItem(
                _T("DL Model Match Code"), m_pVisionPara->m_str2ndInspCode, &m_pVisionPara->m_str2ndInspCode)))
        {
            Item->SetID(ITEM_ID_2NDINSPPARAM_MATCH_CODE);
        }

        DLParam->Expand();
    }
}

void CDlgVisionInspectionBgaBottom2D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionBgaBottom2D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionBgaBottom2D::OnBnClickedButtonInspect()
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

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionInspectionBgaBottom2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

LRESULT CDlgVisionInspectionBgaBottom2D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

LRESULT CDlgVisionInspectionBgaBottom2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    switch (item->GetID())
    {
        case ITEM_ID_GROUP_VIEWER:
            switch (wparam)
            {
                case XTP_PGN_ITEMVALUE_CHANGED:
                case XTP_PGN_SELECTION_CHANGED:
                    if (auto* data = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    {
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
            }
            break;
        case ITEM_ID_NORMAL_IMAGE_COMBINATION_MODE:
            switch (wparam)
            {
                case XTP_PGN_ITEMVALUE_CHANGED:
                    UpdatePropertyState();
                    ShowImage_Normal();
                    break;
                case XTP_PGN_SELECTION_CHANGED:
                    ShowImage_Normal();
                    break;
            }
            break;
        case ITEM_ID_NORMAL_OBLIQUE_IMAGE:
            switch (wparam)
            {
                case XTP_PGN_ITEMVALUE_CHANGED:
                case XTP_PGN_SELECTION_CHANGED:
                    m_imageLotView->ShowImage(m_pVisionPara->m_obliqueImageIndex.getFrameIndex());
                    break;
            }
            break;
        case ITEM_ID_NORMAL_COAXIAL_IMAGE:
            switch (wparam)
            {
                case XTP_PGN_ITEMVALUE_CHANGED:
                case XTP_PGN_SELECTION_CHANGED:
                    m_imageLotView->ShowImage(m_pVisionPara->m_coaxialImageIndex.getFrameIndex());
                    break;
            }
            break;
        case ITEM_ID_QUALITY_IMAGE_COMBINATION_MODE:
            switch (wparam)
            {
                case XTP_PGN_ITEMVALUE_CHANGED:
                    UpdatePropertyState();
                    ShowImage_Quality(FALSE);
                    break;
                case XTP_PGN_SELECTION_CHANGED:
                    ShowImage_Quality(FALSE);
                    break;
            }
            break;
        case ITEM_ID_QUALITY_OBLIQUE_IMAGE:
            switch (wparam)
            {
                case XTP_PGN_ITEMVALUE_CHANGED:
                case XTP_PGN_SELECTION_CHANGED:
                    m_imageLotView->ShowImage(m_pVisionPara->m_qualityObliqueImageIndex.getFrameIndex());
                    break;
            }
            break;
        case ITEM_ID_QUALITY_COAXIAL_IMAGE:
            switch (wparam)
            {
                case XTP_PGN_ITEMVALUE_CHANGED:
                case XTP_PGN_SELECTION_CHANGED:
                    m_imageLotView->ShowImage(m_pVisionPara->m_qualityCoaxialImageIndex.getFrameIndex());
                    break;
            }
            break;
    }

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        if (auto* value = dynamic_cast<CCustomItemButton*>(item))
        {
            if (item->GetID() >= ITEM_ID_GROUPIDX)
                SetupGroupParam(item->GetID());

            //switch (item->GetID()) //그외
            //{
            //}
        }
    }

    return 0;
}

void CDlgVisionInspectionBgaBottom2D::UpdatePropertyState()
{
    if (auto* item = m_propertyGrid->FindItem(ITEM_ID_NORMAL_OBLIQUE_IMAGE))
    {
        item->SetReadOnly(m_pVisionInsp->GetImageFrameCount() == 0);
    }

    if (auto* item = m_propertyGrid->FindItem(ITEM_ID_NORMAL_COAXIAL_IMAGE))
    {
        if ((m_pVisionPara->m_nImageCombineMode != VisionInspectionBgaBottom2DPara::enumCombineMode_Default
                && m_pVisionPara->m_nImageCombineMode
                    != VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault //kircheis_ImproveBall2D
                && m_pVisionPara->m_nImageCombineMode != VisionInspectionBgaBottom2DPara::enumCombineMode_InvertCoaxial
                && m_pVisionPara->m_nImageCombineMode
                    != VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedInvertCoaxial)
            || m_pVisionInsp->GetImageFrameCount() == 0)
        {
            item->SetReadOnly(TRUE);
        }
        else
        {
            item->SetReadOnly(FALSE);
        }
    }

    if (auto* item = m_propertyGrid->FindItem(ITEM_ID_QUALITY_OBLIQUE_IMAGE))
    {
        if ((m_pVisionPara->m_qualityCombineMode == VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Self)
            || m_pVisionInsp->GetImageFrameCount() == 0)
        {
            item->SetReadOnly(TRUE);
        }
        else
        {
            item->SetReadOnly(FALSE);
        }
    }

    if (auto* item = m_propertyGrid->FindItem(ITEM_ID_QUALITY_COAXIAL_IMAGE))
    {
        if ((m_pVisionPara->m_qualityCombineMode == VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Oblique)
            || m_pVisionInsp->GetImageFrameCount() == 0)
        {
            item->SetReadOnly(TRUE);
        }
        else
        {
            item->SetReadOnly(FALSE);
        }
    }
}

void CDlgVisionInspectionBgaBottom2D::ShowImage_Normal()
{
    const auto& sEdgeAlignResult = *m_pVisionInsp->m_sEdgeAlignResult;

    auto obliqueImage = m_pVisionPara->m_obliqueImageIndex.getImage(false);
    auto coaxialImage = m_pVisionPara->m_coaxialImageIndex.getImage(false);

    if (obliqueImage.GetMem() == nullptr)
    {
        SimpleMessage(_T("Error :: Select Oblique Frame"), MB_OK);
        return;
    }
    if (coaxialImage.GetMem() == nullptr)
    {
        SimpleMessage(_T("Error :: Select Coaxial Frame"), MB_OK);
        return;
    }

    Ipvm::Image8u combineImage;
    Ipvm::Image8u resultImage;

    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(resultImage))
        return;

    Ipvm::Rect32s rtROI(0, 0, 0, 0);
    rtROI.m_left = (long)(min(sEdgeAlignResult.fptLT.m_x, sEdgeAlignResult.fptLB.m_x) + 0.5f);
    rtROI.m_right = (long)(max(sEdgeAlignResult.fptRT.m_x, sEdgeAlignResult.fptRB.m_x) + 0.5f);
    rtROI.m_top = (long)(min(sEdgeAlignResult.fptLT.m_y, sEdgeAlignResult.fptRT.m_y) + 0.5f);
    rtROI.m_bottom = (long)(max(sEdgeAlignResult.fptLB.m_y, sEdgeAlignResult.fptRB.m_y) + 0.5f);
    rtROI.InflateRect(10, 10, 10, 10);

    Ipvm::ImageProcessing::Copy(obliqueImage, Ipvm::Rect32s(obliqueImage), resultImage);

    Ipvm::Rect32s modifyROI = rtROI;
    modifyROI.DeflateRect(2, 2, 2, 2);

    switch (m_pVisionPara->m_nImageCombineMode)
    {
        case VisionInspectionBgaBottom2DPara::enumCombineMode_Default:
        case VisionInspectionBgaBottom2DPara::enumCombineMode_ImprovedDefault: //kircheis_ImproveBall2D
            Ipvm::ImageProcessing::Subtract(obliqueImage, coaxialImage, Ipvm::Rect32s(obliqueImage), 0, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_Oblique:
            Ipvm::ImageProcessing::Copy(obliqueImage, modifyROI, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_Self:
            CippModules::EdgeDetect_Magnitude_Sobel(obliqueImage, modifyROI, combineImage);
            break;
        case VisionInspectionBgaBottom2DPara::enumCombineMode_InvertCoaxial: //kircheis_Shiny
        case VisionInspectionBgaBottom2DPara::enumCombineMode_ReformedInvertCoaxial: //kircheis_InvertCoax
            Ipvm::ImageProcessing::BitwiseNot(coaxialImage, modifyROI, combineImage);
            break;
    }

    CString FrameToString;
    FrameToString.Format(_T("Combined Image **"));

    m_imageLotView->SetImage(combineImage, FrameToString);
    FrameToString.Empty();
}

void CDlgVisionInspectionBgaBottom2D::ShowImage_Quality(BOOL bThreshImage, long nQualityThresholdValue)
{
    const auto& sEdgeAlignResult = *m_pVisionInsp->m_sEdgeAlignResult;

    auto obliqueImage = m_pVisionPara->m_qualityObliqueImageIndex.getImage(false);
    auto coaxialImage = m_pVisionPara->m_qualityCoaxialImageIndex.getImage(false);

    if (obliqueImage.GetMem() == nullptr)
    {
        SimpleMessage(_T("Error :: Select Oblique Frame"), MB_OK);
        return;
    }

    if (coaxialImage.GetMem() == nullptr)
    {
        SimpleMessage(_T("Error :: Select Coaxial Frame"), MB_OK);
        return;
    }

    Ipvm::Image8u combineImage;
    Ipvm::Image8u resultImage;

    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(resultImage))
        return;

    Ipvm::Rect32s rtROI(0, 0, 0, 0);
    rtROI.m_left = (long)(min(sEdgeAlignResult.fptLT.m_x, sEdgeAlignResult.fptLB.m_x) + 0.5f);
    rtROI.m_right = (long)(max(sEdgeAlignResult.fptRT.m_x, sEdgeAlignResult.fptRB.m_x) + 0.5f);
    rtROI.m_top = (long)(min(sEdgeAlignResult.fptLT.m_y, sEdgeAlignResult.fptRT.m_y) + 0.5f);
    rtROI.m_bottom = (long)(max(sEdgeAlignResult.fptLB.m_y, sEdgeAlignResult.fptRB.m_y) + 0.5f);
    rtROI.InflateRect(10, 10, 10, 10);

    Ipvm::ImageProcessing::Copy(obliqueImage, Ipvm::Rect32s(obliqueImage), resultImage);

    Ipvm::Rect32s modifyROI = rtROI;
    modifyROI.DeflateRect(2, 2, 2, 2);

    // Image Processing...
    if (bThreshImage) //kircheis_Shiny
    {
        switch (m_pVisionPara->m_qualityCombineMode)
        {
            case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Default:
                Ipvm::ImageProcessing::Subtract(
                    obliqueImage, coaxialImage, Ipvm::Rect32s(obliqueImage), 0, combineImage);
                break;
            case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Oblique:
                Ipvm::ImageProcessing::Copy(obliqueImage, modifyROI, combineImage);
                break;
            case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Self:
                CippModules::EdgeDetect_Magnitude_Sobel(obliqueImage, modifyROI, combineImage);
                break;
        }
    }
    else
    {
        switch (m_pVisionPara->m_qualityCombineMode)
        {
            case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Default:
                Ipvm::ImageProcessing::Subtract(
                    obliqueImage, coaxialImage, Ipvm::Rect32s(obliqueImage), 0, combineImage);
                break;
            case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Oblique:
                Ipvm::ImageProcessing::Copy(obliqueImage, modifyROI, combineImage);
                break;
            case VisionInspectionBgaBottom2DPara::enumQualityCombineMode_Self:
                CippModules::EdgeDetect_Magnitude_Sobel(obliqueImage, modifyROI, combineImage);
                break;
        }
    }

    CString FrameToString;
    FrameToString.Format(_T("Combined Image **"));

    if (bThreshImage)
    {
        BYTE threshold = (BYTE)min(255, max(0, nQualityThresholdValue));
        Ipvm::ImageProcessing::BinarizeGreaterEqual(combineImage, rtROI, threshold, resultImage);
        m_imageLotView->SetImage(resultImage, FrameToString);
    }
    else
    {
        m_imageLotView->SetImage(combineImage, FrameToString);
    }
    FrameToString.Empty();
}

LRESULT CDlgVisionInspectionBgaBottom2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_PAD_ALIGN_2D), false,
            m_pVisionInsp->GetCurVisionModule_Status());
    else if (m_pVisionInsp->m_visionUnit.IsTheUsingVisionProcessingByGuid(
                 m_pVisionInsp->m_visionUnit.GetVisionAlignProcessingModuleGUID()))
        m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}

BOOL CDlgVisionInspectionBgaBottom2D::TeachRingRadius(const bool detailSetupMode, long i_nBallGroupID)
{
    if (!m_pVisionInsp->DoTeach(detailSetupMode, i_nBallGroupID))
        return FALSE;

    return TRUE;
}

void CDlgVisionInspectionBgaBottom2D::ResetSpecAndResultDlg()
{
    m_procCommonSpecDlg->ResetInitGrid();
    m_procCommonResultDlg->Refresh();
}

BOOL CDlgVisionInspectionBgaBottom2D::MakeSpecROI(long i_nGroupID, std::vector<PI_RECT>& o_vecpirtSpecLand)
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
