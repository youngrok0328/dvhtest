//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionBgaBallBridge.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionBgaBallBridge.h"
#include "VisionInspectionBgaBallBridgePara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/ConversionEx.h"
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Quadrangle32r.h>

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
    ITEM_ID_IMAGE_COMBINE,
    ITEM_ID_BALL_LINK_RANGE_RATIO,
    ITEM_ID_BALL_BRIDGE_IGNORE_RATIO,
    ITEM_ID_BALL_BRIDGE_ROI_WIDTH_RATIO,
    ITEM_ID_THRESHOLD_VALUE,
    ITEM_ID_2NDINSPPARAM_USE_INSPECTION,
    ITEM_ID_2NDINSPPARAM_MATCH_CODE,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionBgaBallBridge, CDialog)

CDlgVisionInspectionBgaBallBridge::CDlgVisionInspectionBgaBallBridge(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionBgaBallBridge* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionBgaBallBridge::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_VisionPara)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
{
}

CDlgVisionInspectionBgaBallBridge::~CDlgVisionInspectionBgaBallBridge()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    delete m_imageLotView;
}

void CDlgVisionInspectionBgaBallBridge::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionBgaBallBridge, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionBgaBallBridge::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionBgaBallBridge::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionBgaBallBridge::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionBgaBallBridge::OnInitDialog()
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

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_fixedInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    UpdatePropertyGrid();

    if (m_pVisionInsp->m_visionUnit.m_systemConfig.m_bUseAiInspection)
    {
        SetPropertyDeepLearnParam(); // ksy deeplearning
    }

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionInspectionBgaBallBridge::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionBgaBallBridge::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionBgaBallBridge::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto frameIndex = m_pVisionInsp->GetImageFrameIndex(0);
    m_imageLotView->ShowImage(frameIndex);

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

void CDlgVisionInspectionBgaBallBridge::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

LRESULT CDlgVisionInspectionBgaBallBridge::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

LRESULT CDlgVisionInspectionBgaBallBridge::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case ITEM_ID_IMAGE_COMBINE:
                ClickedButtonImageCombine();
                break;
            case ITEM_ID_BALL_LINK_RANGE_RATIO:
                m_pVisionInsp->RefreshPackageSpecBallLink();
                m_pVisionInsp->MakeRefData(true);
                ShowThresholdImage(false);
                DrawBallLink();
                break;
            case ITEM_ID_BALL_BRIDGE_IGNORE_RATIO:
            case ITEM_ID_BALL_BRIDGE_ROI_WIDTH_RATIO:
                m_pVisionInsp->MakeRefData(true);
                ShowThresholdImage(false);
                DrawInspectionROI();
                break;
            case ITEM_ID_THRESHOLD_VALUE:
                ShowThresholdImage(true);
                break;
        }
    }
    else if (wparam == XTP_PGN_SELECTION_CHANGED && item->GetID() == ITEM_ID_THRESHOLD_VALUE)
    {
        if (auto* control = item->GetInplaceControls()->GetAt(0))
        {
            auto* slider = dynamic_cast<CXTPPropertyGridInplaceSlider*>(control);

            if (slider)
            {
                slider->GetSliderCtrl()->SetPageSize(8);
            }
        }
        ShowThresholdImage(true);
    }

    return 0;
}

void CDlgVisionInspectionBgaBallBridge::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Image")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))->SetID(ITEM_ID_IMAGE_COMBINE);
        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Inspection Parameter")))
    {
        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(
                _T("Link range ratio of the ball"), m_pVisionPara->m_fBallLinkRangeRatio,
                _T("%.0lf %% of Avr ball pitch."), m_pVisionPara->m_fBallLinkRangeRatio, 80, 250., 1.0)))
        {
            item->SetID(ITEM_ID_BALL_LINK_RANGE_RATIO);
        }
        if (auto* item = category->AddChildItem(
                new XTPPropertyGridItemCustomFloat<double>(_T("Ball ignore ratio"), m_pVisionPara->m_fBallIgnoreRatio,
                    _T("%.0lf %% of each ball width"), m_pVisionPara->m_fBallIgnoreRatio, 80, 180., 1.0)))
        {
            item->SetID(ITEM_ID_BALL_BRIDGE_IGNORE_RATIO);
        }
        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(
                _T("Inspection ROI width ratio"), m_pVisionPara->m_fROI_WidthRatio, _T("%.0lf %% of each ball width"),
                m_pVisionPara->m_fROI_WidthRatio, 50, 150., 1.0)))
        {
            item->SetID(ITEM_ID_BALL_BRIDGE_ROI_WIDTH_RATIO);
        }

        if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Threshold"), m_pVisionPara->m_nThresholdValue, &m_pVisionPara->m_nThresholdValue)))
        {
            if (auto* slider = Item->AddSliderControl())
            {
                slider->SetMin(0);
                slider->SetMax(256);
            }

            if (auto* spin = Item->AddSpinButton())
            {
                spin->SetMin(0);
                spin->SetMax(256);
            }

            Item->SetID(ITEM_ID_THRESHOLD_VALUE);
        }

        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.4);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

void CDlgVisionInspectionBgaBallBridge::SetPropertyDeepLearnParam()
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

void CDlgVisionInspectionBgaBallBridge::ShowThresholdImage(BOOL bThreshImage)
{
    const auto& sEdgeAlignResult = *m_pVisionInsp->m_sEdgeAlignResult;

    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    Ipvm::Image8u combineImage;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
        return;

    if (!CippModules::GrayImageProcessingManage(
            m_pVisionInsp->getReusableMemory(), &image, false, m_pVisionPara->m_ImageProcManagePara, combineImage))
    {
        return;
    }

    if (!bThreshImage)
    {
        m_imageLotView->SetImage(combineImage);
        return;
    }

    Ipvm::Image8u thresholdImage;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(thresholdImage))
        return;
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(thresholdImage), 0, thresholdImage);

    Ipvm::Image8u resultImage;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(resultImage))
        return;
    Ipvm::ImageProcessing::Copy(combineImage, Ipvm::Rect32s(combineImage), resultImage);

    Ipvm::Rect32s rtROI(0, 0, 0, 0);
    rtROI.m_left = (long)(min(sEdgeAlignResult.fptLT.m_x, sEdgeAlignResult.fptLB.m_x) + 0.5f);
    rtROI.m_right = (long)(max(sEdgeAlignResult.fptRT.m_x, sEdgeAlignResult.fptRB.m_x) + 0.5f);
    rtROI.m_top = (long)(min(sEdgeAlignResult.fptLT.m_y, sEdgeAlignResult.fptRT.m_y) + 0.5f);
    rtROI.m_bottom = (long)(max(sEdgeAlignResult.fptLB.m_y, sEdgeAlignResult.fptRB.m_y) + 0.5f);
    rtROI.InflateRect(10, 10, 10, 10);

    long threshold = (long)min(255, max(0, m_pVisionPara->m_nThresholdValue));
    CippModules::Binarize(combineImage, thresholdImage, rtROI, threshold, false);

    Ipvm::ImageProcessing::Subtract(combineImage, *(m_pVisionInsp->m_imageMaskInspROI), rtROI, 0, combineImage);
    Ipvm::ImageProcessing::BitwiseAnd(thresholdImage, *(m_pVisionInsp->m_imageMaskInspROI), rtROI, resultImage);
    Ipvm::ImageProcessing::Add(combineImage, resultImage, rtROI, 0, resultImage);

    m_imageLotView->SetImage(resultImage);
    DrawInspectionROI();
}

void CDlgVisionInspectionBgaBallBridge::DrawBallLink()
{
    m_imageLotView->Overlay_RemoveAll();

    std::vector<Ipvm::Point32r2> vecPtStart(0);
    std::vector<Ipvm::Point32r2> vecPtEnd(0);
    Ipvm::Point32r2 ptPoint;

    for (auto& ppInspData : m_pVisionInsp->m_vec2BallCalcData)
    {
        for (auto& pInspData : ppInspData)
        {
            auto& inspLine = pInspData.lineBallLink;
            ptPoint.Set(inspLine.m_sx, inspLine.m_sy);
            vecPtStart.push_back(ptPoint);
            ptPoint.Set(inspLine.m_ex, inspLine.m_ey);
            vecPtEnd.push_back(ptPoint);
        }
    }

    m_imageLotView->Overlay_AddLine(vecPtStart, vecPtEnd, RGB(0, 255, 0));
    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionBgaBallBridge::DrawInspectionROI()
{
    m_imageLotView->Overlay_RemoveAll();

    std::vector<Ipvm::Quadrangle32r> vecqRect(0);
    Ipvm::Quadrangle32r qRect;
    for (auto& ppInspData : m_pVisionInsp->m_vec2BallCalcData)
    {
        for (auto& pInspData : ppInspData)
        {
            auto& inspROI = pInspData.pirtBallBridgeShowROI;
            qRect = ConversionEx::ToQuadrangle32r(inspROI);
            vecqRect.push_back(qRect);
        }
    }

    m_imageLotView->Overlay_AddRectangles(vecqRect, RGB(0, 255, 0));

    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionBgaBallBridge::ClickedButtonImageCombine()
{
    auto* proc = m_pVisionInsp;

    if (::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), false, &proc->m_VisionPara->m_ImageProcManagePara)
        == IDOK)
    {
        // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
        Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
        if (image.GetMem() == nullptr)
            return;

        Ipvm::Image8u combineImage;
        if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
            return;

        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());
        if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_ImageProcManagePara, combineImage))
        {
            return;
        }

        CString FrameToString;

        if (m_pVisionInsp->m_VisionPara->m_ImageProcManagePara.isCombine())
        {
            FrameToString.Format(_T("Combined Image **"));
        }
        else
        {
            FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
        }

        m_imageLotView->SetImage(combineImage, FrameToString);
        FrameToString.Empty();
    }
}

LRESULT CDlgVisionInspectionBgaBallBridge::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D), false,
        m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}
