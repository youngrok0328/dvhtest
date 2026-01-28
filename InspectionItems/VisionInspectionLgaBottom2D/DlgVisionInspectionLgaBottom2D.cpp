//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionLgaBottom2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgLandParameter.h"
#include "VisionInspectionLgaBottom2D.h"
#include "VisionInspectionLgaBottom2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
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
    ITEM_ID_SEARCH_BACKGROUND = 1,
    ITEM_ID_GROUP_VIEWER,
    ITEM_ID_GROUP_ID = 100,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionLgaBottom2D, CDialog)

CDlgVisionInspectionLgaBottom2D::CDlgVisionInspectionLgaBottom2D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionLgaBottom2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionLgaBottom2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_propertyGrid(nullptr)
    , m_nSelectGroupID(0)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionInsp->ResetResult();

    m_pVisionPara = m_pVisionInsp->m_VisionPara;
    //m_pVisionInspParaDlg			= new CDlgVisionInspectionLgaBottom2DPara(pVisionInsp->m_packageSpec, this);
    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
}

CDlgVisionInspectionLgaBottom2D::~CDlgVisionInspectionLgaBottom2D()
{
    m_pVisionInsp->SetVisionInspectionSpecs();

    delete m_imageLotView;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;

    //delete m_pVisionInspParaDlg;
    delete m_propertyGrid;
    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspectionLgaBottom2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionLgaBottom2D, CDialog)
ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionLgaBottom2D::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspectionLgaBottom2D::OnCbnSelchangeComboCurrpane)
ON_BN_CLICKED(IDC_BUTTON_PREIMAGE, &CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonPreimage)
ON_BN_CLICKED(IDC_BUTTON_NEXTIMAGE, &CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonNextimage)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionLgaBottom2D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgVisionInspectionLgaBottom2D::OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionLgaBottom2D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionLgaBottom2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    long nDataNum = 0;
    void* pData = m_pVisionInsp->m_visionUnit.GetVisionDebugInfo(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        *m_pVisionInsp->m_sEdgeAlignResult = *(VisionAlignResult*)pData;

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, CAST_UINT(IDC_STATIC));

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

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionPara->m_vecVisionInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    // Dialog Control 위치 설정
    //SetInitDialog();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionLgaBottom2D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    //m_pVisionInspParaDlg		->Create(IDD_DIALOG_PARAMETER,		this);

    return 0;
}

void CDlgVisionInspectionLgaBottom2D::OnDestroy()
{
    CDialog::OnDestroy();

    //m_pVisionInspParaDlg		->DestroyWindow();
}

void CDlgVisionInspectionLgaBottom2D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionLgaBottom2D::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Land Specification Parameter")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Intensity of Land"), m_pVisionPara->m_nIntensityofLand, (int*)&m_pVisionPara->m_nIntensityofLand)))
        {
            item->GetConstraints()->AddConstraint(_T("Bright Land"), 0);
            item->GetConstraints()->AddConstraint(_T("Dark Land"), 1);

            item->SetID(ITEM_ID_SEARCH_BACKGROUND);
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

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspectionLgaBottom2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_SELECTION_CHANGED || wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        if (auto* dValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
        {
            if (item->GetID() == ITEM_ID_GROUP_VIEWER)
            {
                m_nSelectGroupID = (long)dValue->GetEnum();

                std::vector<PI_RECT> LandSpecROI;

                MakeSpecROI(m_nSelectGroupID, LandSpecROI);

                m_imageLotView->Overlay_RemoveAll();

                for (const auto& Land : LandSpecROI)
                {
                    m_imageLotView->Overlay_AddRectangle(Land, RGB(0, 255, 0));
                }

                m_imageLotView->Overlay_Show(TRUE);
            }

            return 0;
        }
    }

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        if (auto* value = dynamic_cast<CCustomItemButton*>(item))
        {
            if (item->GetID() >= ITEM_ID_GROUP_ID)
            {
                if (item->GetID() < 0)
                    return -1;

                long nRealGroupID = item->GetID() - ITEM_ID_GROUP_ID; //ID를 찾기위해 어쩔수없다..

                if (nRealGroupID < 0)
                    return -1;

                DlgLandParameter DlgGroupSetupParam(m_pVisionPara, nRealGroupID);

                DlgGroupSetupParam.DoModal();
            }
        }
    }

    return 0;
}

void CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonInspect()
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
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
    ShowImage(TRUE);
}

void CDlgVisionInspectionLgaBottom2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionLgaBottom2D::ShowPrevImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID <= 0)
        return;

    m_nImageID--;

    ShowImage(TRUE);
}

void CDlgVisionInspectionLgaBottom2D::ShowNextImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID
        >= (long)(m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size()) - 1)
        return;

    m_nImageID++;

    ShowImage(TRUE);
}

void CDlgVisionInspectionLgaBottom2D::ShowImage(BOOL bChange)
{
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
    CString FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_nImageID);
    m_imageLotView->SetImage(image, FrameToString);

    FrameToString.Empty();
}

void CDlgVisionInspectionLgaBottom2D::OnCbnSelchangeComboCurrpane()
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

void CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonPreimage()
{
    ShowPrevImage();
}

void CDlgVisionInspectionLgaBottom2D::OnBnClickedButtonNextimage()
{
    ShowNextImage();
}

LRESULT CDlgVisionInspectionLgaBottom2D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

LRESULT CDlgVisionInspectionLgaBottom2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
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

void CDlgVisionInspectionLgaBottom2D::ResetSpecAndResultDlg()
{
    m_procCommonSpecDlg->ResetInitGrid();
    m_procCommonResultDlg->Refresh();
}

BOOL CDlgVisionInspectionLgaBottom2D::MakeSpecROI(long i_nGroupID, std::vector<PI_RECT>& o_vecpirtSpecLand)
{
    if (m_pVisionInsp->m_vec2LandInfoPerLayer.size() <= 0)
        return FALSE;

    const auto& mm2px = m_pVisionInsp->getScale().mmToPixel();
    float fPixelperMM = (mm2px.m_x + mm2px.m_y) / 2.f;

    Ipvm::Point32r2 fptBodyCenter = m_pVisionInsp->m_sEdgeAlignResult->m_center;

    for (auto LandMapData : m_pVisionInsp->m_vec2LandInfoPerLayer[i_nGroupID])
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
