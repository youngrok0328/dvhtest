//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionOTITR.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionOTITR.h"
#include "VisionInspectionOTITRPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>

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
    ITEM_ID_OTI_START = 0,
    ITEM_ID_OTI_FILL_POCKET_TITLE = ITEM_ID_OTI_START,
    ITEM_ID_OTI_FILL_POCKET_STATUS,

    ITEM_ID_OTI_COVER_TAPE_ALIGN_TITLE,
    ITEM_ID_OTI_COVER_TAPE_ALIGN_FRAME_ID,
    ITEM_ID_OTI_COVER_TAPE_ALIGN_IMAGE_COMBINE,
    ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_MODE,
    ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_THRESHOLD,
    ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_SEARCH_LENGTH,
    ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_GAP,

    ITEM_ID_OTI_SEALING_ALIGN_TITLE,
    ITEM_ID_OTI_SEALING_ALIGN_FRAME_ID,
    ITEM_ID_OTI_SEALING_ALIGN_IMAGE_COMBINE,
    ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_MODE,
    ITEM_ID_OTI_SEALING_ALIGN_EDGE_THRESHOLD,
    ITEM_ID_OTI_SEALING_ALIGN_EDGE_SEARCH_LENGTH,
    ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_GAP,
    ITEM_ID_OTI_SEALING_ALIGN_SEALING_BOUNDARY_TYPE,

    ITEM_ID_OTI_END,
};

static LPCTSTR g_szParamName[] = {
    _T("Fill Pocket"), //ITEM_ID_OTI_FILL_POCKET_TITLE
    _T("Fill Pocket Status"), //ITEM_ID_OTI_FILL_POCKET_STATUS

    _T("Cover Tape Align paramters"), //ITEM_ID_OTI_COVER_TAPE_ALIGN_TITLE
    _T("Image Frame ID"), //ITEM_ID_OTI_COVER_TAPE_ALIGN_FRAME_ID
    _T("Image Combine"), //ITEM_ID_OTI_COVER_TAPE_ALIGN_IMAGE_COMBINE
    _T("Edge Detect Mode"), //ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_MODE
    _T("Edge Threshold"), //ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_THRESHOLD
    _T("Edge Search Length (um)"), //ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_SEARCH_LENGTH
    _T("Edge Detect Gap (um)"), //ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_GAP

    _T("Sealing Align paramters"), //ITEM_ID_OTI_SEALING_ALIGN_TITLE
    _T("Image Frame ID"), //ITEM_ID_OTI_SEALING_ALIGN_FRAME_ID
    _T("Image Combine"), //ITEM_ID_OTI_SEALING_ALIGN_IMAGE_COMBINE
    _T("Edge Detect Mode"), //ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_MODE
    _T("Edge Threshold"), //ITEM_ID_OTI_SEALING_ALIGN_EDGE_THRESHOLD
    _T("Edge Search Length (um)"), //ITEM_ID_OTI_SEALING_ALIGN_EDGE_SEARCH_LENGTH
    _T("Edge Detect Gap (um)"), //ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_GAP
    _T("Sealing Boundary Type"), //ITEM_ID_OTI_SEALING_ALIGN_SEALING_BOUNDARY_TYPE
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionOTITR, CDialog)

CDlgVisionInspectionOTITR::CDlgVisionInspectionOTITR(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionOTITR* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionOTITR::IDD, pParent)
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
    , m_receivedFillPocketStatus(FILL_POCKET_EMPTY) // 초기값은 Empty로 설정
    , m_nImageID(0)
{
}

CDlgVisionInspectionOTITR::~CDlgVisionInspectionOTITR()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    delete m_imageLotView;
}

void CDlgVisionInspectionOTITR::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionOTITR, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionOTITR::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionOTITR::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionOTITR::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionOTITR::OnInitDialog()
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

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionInspectionOTITR::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionOTITR::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionOTITR::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    if (m_pVisionInsp->GetImageFrameCount())
    {
        auto frameIndex = m_pVisionInsp->GetImageFrameIndex(0);
        m_imageLotView->ShowImage(frameIndex);
    }

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

void CDlgVisionInspectionOTITR::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

LRESULT CDlgVisionInspectionOTITR::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

LRESULT CDlgVisionInspectionOTITR::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(lparam);
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case ITEM_ID_OTI_FILL_POCKET_STATUS:
            {
                auto* enumItem = static_cast<CXTPPropertyGridItemEnum*>(item);
                if (enumItem != nullptr)
                {
                    m_pVisionInsp->SetReceivedFillPocketStatus(
                        m_receivedFillPocketStatus == FILL_POCKET_FILL ? true : false);
                    m_pVisionInsp->SetFillPocketInfoInImageLot(m_pVisionInsp->GetReceivedFillPocketStatus());
                }
            }
            break;
            case ITEM_ID_OTI_COVER_TAPE_ALIGN_FRAME_ID:
            {
                long nAlignFrameIdx = m_pVisionPara->m_coverTapeImageFrameIndex.getFrameIndex();
                m_nImageID = nAlignFrameIdx;
                ShowImage(nAlignFrameIdx);
            }
            break;
            case ITEM_ID_OTI_COVER_TAPE_ALIGN_IMAGE_COMBINE:
            {
                ClickedButtonCoverTapeAlignImageCombine();
            }
            break;
            case ITEM_ID_OTI_SEALING_ALIGN_FRAME_ID:
            {
                long nAlignFrameIdx = m_pVisionPara->m_sealingImageFrameIndex.getFrameIndex();
                m_nImageID = nAlignFrameIdx;
                ShowImage(nAlignFrameIdx);
            }
            break;
            case ITEM_ID_OTI_SEALING_ALIGN_IMAGE_COMBINE:
            {
                ClickedButtonSealingAlignImageCombine();
            }
            break;
            case ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_SEARCH_LENGTH:
                ShowCoverTapeSearchROI();
                break;
            case ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_GAP:
                ShowCoverTapeSearchLine();
                break;
            case ITEM_ID_OTI_SEALING_ALIGN_EDGE_SEARCH_LENGTH:
                ShowSealingSearchROI();
                break;
            case ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_GAP:
                ShowSealingSearchLine();
                break;
            default:
                break;
        }
    }

    return 0;
}

void CDlgVisionInspectionOTITR::ClickedButtonCoverTapeAlignImageCombine()
{
    auto* proc = m_pVisionInsp;

    const auto nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    if (::Combine_SetParameter(*proc, m_pVisionPara->m_coverTapeImageFrameIndex.getFrameIndex(), false,
            &proc->m_VisionPara->m_coverTapeImageProc)
        == IDOK)
    {
        ////////////////////////
        auto processor = m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_OTI_TR);
        auto& memory = processor->getReusableMemory();

        // Raw 이미지를 받아온다.
        auto frameIndex = m_pVisionPara->m_coverTapeImageFrameIndex.getFrameIndex();
        m_pVisionPara->m_coverTapeImageFrameIndex.setFrameIndex(frameIndex);
        const auto& image = m_pVisionPara->m_coverTapeImageFrameIndex.getImage(false);
        if (image.GetMem() == nullptr)
        {
            return;
        }

        Ipvm::Image8u combineImage;

        // 이미지 raw로 받아옴
        if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
            return;

        // Combine 이미지 생성 > 기존 값은 원본 이미지로 하였다.
        Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), combineImage);
        Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());

        // 이미지 combine 해서 결과 표시
        if (CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_coverTapeImageProc, combineImage)
            == false)
        {
            return;
        }

        // 이미지 표시 - 2024.05.29_JHB
       m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(frameIndex));
       m_imageLotView->SetImage(combineImage, _T("Cover Tape Align Combined Image **"));
       
    }
}

void CDlgVisionInspectionOTITR::ClickedButtonSealingAlignImageCombine()
{
    auto* proc = m_pVisionInsp;

    const auto nSideSection = m_pVisionInsp->GetCurVisionModule_Status();

    if (::Combine_SetParameter(*proc, m_pVisionPara->m_sealingImageFrameIndex.getFrameIndex(), false,
            &proc->m_VisionPara->m_sealingImageProc)
        == IDOK)
    {
        ////////////////////////
        auto processor = m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_OTI_TR);
        auto& memory = processor->getReusableMemory();

        // Raw 이미지를 받아온다.
        auto frameIndex = m_pVisionPara->m_sealingImageFrameIndex.getFrameIndex();
        m_pVisionPara->m_sealingImageFrameIndex.setFrameIndex(frameIndex);
        const auto& image = m_pVisionPara->m_sealingImageFrameIndex.getImage(false);
        if (image.GetMem() == nullptr)
        {
            return;
        }

        Ipvm::Image8u combineImage;

        // 이미지 raw로 받아옴
        if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
            return;

        // Combine 이미지 생성 > 기존 값은 원본 이미지로 하였다.
        Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), combineImage);
        Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());

        // 이미지 combine 해서 결과 표시
        if (CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_sealingImageProc, combineImage)
            == false)
        {
            return;
        }

        // 이미지 표시 - 2024.05.29_JHB
        m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(frameIndex));
        m_imageLotView->SetImage(combineImage, _T("Sealing Align Combined Image **"));
    }
}

void CDlgVisionInspectionOTITR::UpdatePropertyGrid()
{
    static const bool isHWExist = static_cast<bool>(SystemConfig::GetInstance().IsHardwareExist());

    m_propertyGrid->ResetContent();
    
    if (auto* category = m_propertyGrid->AddCategory(g_szParamName[ITEM_ID_OTI_FILL_POCKET_TITLE])) //Common Param
    {
        m_receivedFillPocketStatus
            = (m_pVisionInsp->GetReceivedFillPocketStatus() == true) ? FILL_POCKET_FILL : FILL_POCKET_EMPTY;

        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(g_szParamName[ITEM_ID_OTI_FILL_POCKET_STATUS],
                m_receivedFillPocketStatus, &m_receivedFillPocketStatus)))
        {
            item->GetConstraints()->AddConstraint(_T("Empty"), FILL_POCKET_EMPTY);
            item->GetConstraints()->AddConstraint(_T("Filled"), FILL_POCKET_FILL);

            item->SetID(ITEM_ID_OTI_FILL_POCKET_STATUS);

            if (isHWExist != false)
                item->SetReadOnly(TRUE);
        }
        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(g_szParamName[ITEM_ID_OTI_COVER_TAPE_ALIGN_TITLE])) //Cover Tape Align Parameters
    {
        m_pVisionPara->m_coverTapeImageFrameIndex.makePropertyGridItem(
            category, g_szParamName[ITEM_ID_OTI_COVER_TAPE_ALIGN_FRAME_ID], ITEM_ID_OTI_COVER_TAPE_ALIGN_FRAME_ID);

        category->AddChildItem(
            new CCustomItemButton(g_szParamName[ITEM_ID_OTI_COVER_TAPE_ALIGN_IMAGE_COMBINE], TRUE, FALSE))
            ->SetID(ITEM_ID_OTI_COVER_TAPE_ALIGN_IMAGE_COMBINE);

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szParamName[ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_MODE],
                    m_pVisionPara->m_coverTapeEdgeDetectMode,(int*)&m_pVisionPara->m_coverTapeEdgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), PI_ED_MAX_EDGE);
            item->GetConstraints()->AddConstraint(_T("First Edge"), PI_ED_FIRST_EDGE);
            item->SetID(ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_MODE);
        }

        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(g_szParamName[ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_THRESHOLD],
                    m_pVisionPara->m_coverTapeEdgeThreshold, _T("%.1lf"), &m_pVisionPara->m_coverTapeEdgeThreshold))
            ->SetID(ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_THRESHOLD);

        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(g_szParamName[ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_SEARCH_LENGTH],
                    m_pVisionPara->m_coverTapeAlignSearchLength_um, &m_pVisionPara->m_coverTapeAlignSearchLength_um))
            ->SetID(ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_SEARCH_LENGTH);

        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(g_szParamName[ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_GAP],
                m_pVisionPara->m_coverTapeDetectGap_um, &m_pVisionPara->m_coverTapeDetectGap_um))
            ->SetID(ITEM_ID_OTI_COVER_TAPE_ALIGN_EDGE_DETECT_GAP);

        category->Expand();
    }

    if (auto* category
        = m_propertyGrid->AddCategory(g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_TITLE])) //Sealing Align Parameters
    {
        m_pVisionPara->m_sealingImageFrameIndex.makePropertyGridItem(
            category, g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_FRAME_ID], ITEM_ID_OTI_SEALING_ALIGN_FRAME_ID);

        category
            ->AddChildItem(new CCustomItemButton(g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_IMAGE_COMBINE], TRUE, FALSE))
            ->SetID(ITEM_ID_OTI_SEALING_ALIGN_IMAGE_COMBINE);

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_MODE],
                    m_pVisionPara->m_sealingEdgeDetectMode, (int*)&m_pVisionPara->m_sealingEdgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), PI_ED_MAX_EDGE);
            item->GetConstraints()->AddConstraint(_T("First Edge"), PI_ED_FIRST_EDGE);
            item->SetID(ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_MODE);
        }

        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_EDGE_THRESHOLD],
                m_pVisionPara->m_sealingEdgeThreshold, _T("%.1lf"), &m_pVisionPara->m_sealingEdgeThreshold))
            ->SetID(ITEM_ID_OTI_SEALING_ALIGN_EDGE_THRESHOLD);

        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_EDGE_SEARCH_LENGTH],
                m_pVisionPara->m_sealingAlignSearchLength_um, &m_pVisionPara->m_sealingAlignSearchLength_um))
            ->SetID(ITEM_ID_OTI_SEALING_ALIGN_EDGE_SEARCH_LENGTH);

        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_GAP],
                m_pVisionPara->m_sealingDetectGap_um, &m_pVisionPara->m_sealingDetectGap_um))
            ->SetID(ITEM_ID_OTI_SEALING_ALIGN_EDGE_DETECT_GAP);

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(g_szParamName[ITEM_ID_OTI_SEALING_ALIGN_SEALING_BOUNDARY_TYPE],
                    m_pVisionPara->m_sealingBoundaryType, (int*)&m_pVisionPara->m_sealingBoundaryType)))
        {
            item->GetConstraints()->AddConstraint(_T("Inner"), enumSealingBoundaryType_Inner);
            item->GetConstraints()->AddConstraint(_T("Outer"), enumSealingBoundaryType_Outer);
            item->SetID(ITEM_ID_OTI_SEALING_ALIGN_SEALING_BOUNDARY_TYPE);
        }

        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.4);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspectionOTITR::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D), false,
        m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}

void CDlgVisionInspectionOTITR::ShowImage(long nFrameIdx)
{
    const long nVisionType = SystemConfig::GetInstance().GetVisionType();
    const long nCurSideSelection = m_pVisionInsp->GetCurVisionModule_Status();

    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage(false, nFrameIdx);
    if (image.GetMem() == nullptr)
        return;

    m_imageLotView->SetImage(image);
}

void CDlgVisionInspectionOTITR::ShowCoverTapeSearchROI()
{
    m_imageLotView->Overlay_RemoveAll();

    std::vector<Ipvm::Rect32s> searchROIs(2);

    m_pVisionInsp->GetCoverTapeSearchROI(searchROIs);

    m_imageLotView->Overlay_AddRectangles(searchROIs, RGB(255, 255, 0));
    
    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionOTITR::ShowSealingSearchROI()
{
    m_imageLotView->Overlay_RemoveAll();
    std::vector<Ipvm::Rect32s> searchROIs(4);
    m_pVisionInsp->GetSealingSearchROI(searchROIs);

    m_imageLotView->Overlay_AddRectangles(searchROIs, RGB(0, 255, 0));
    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionOTITR::ShowCoverTapeSearchLine()
{
    m_imageLotView->Overlay_RemoveAll();
    std::vector<Ipvm::LineSeg32r> searchLines(0);
    m_pVisionInsp->GetCoverTapeSearchLines(searchLines);
    std::vector<Ipvm::Point32r2> startPoint(0);
    std::vector<Ipvm::Point32r2> endPoint(0);

    const long nLineCount = static_cast<long>(searchLines.size());
    if (nLineCount <= 0)
        return;

    for (long idx = 0; idx < nLineCount; idx++)
    {
        startPoint.push_back(searchLines[idx].StartPoint());
        endPoint.push_back(searchLines[idx].EndPoint());
    }

    m_imageLotView->Overlay_AddLine(startPoint, endPoint, RGB(255, 255, 0));
    m_imageLotView->Overlay_Show(true);
}

void CDlgVisionInspectionOTITR::ShowSealingSearchLine()
{
    m_imageLotView->Overlay_RemoveAll();
    std::vector<Ipvm::LineSeg32r> searchLines(0);
    m_pVisionInsp->GetSealingSearchLines(searchLines);
    std::vector<Ipvm::Point32r2> startPoint(0);
    std::vector<Ipvm::Point32r2> endPoint(0);
    const long nLineCount = static_cast<long>(searchLines.size());
    if (nLineCount <= 0)
        return;

    for (long idx = 0; idx < nLineCount; idx++)
    {
        startPoint.push_back(searchLines[idx].StartPoint());
        endPoint.push_back(searchLines[idx].EndPoint());
    }
    m_imageLotView->Overlay_AddLine(startPoint, endPoint, RGB(0, 255, 0));
    m_imageLotView->Overlay_Show(true);
}