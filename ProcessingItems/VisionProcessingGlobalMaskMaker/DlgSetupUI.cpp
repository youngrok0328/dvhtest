//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSetupUI.h"

//CPP_2_________________________________ This project's headers
#include "DlgLayer.h"
#include "DlgLayerName.h"
#include "DlgPara.h"
#include "ResultLayer.h"
#include "VisionProcessingGlobalMaskMaker.h"
#include "VisionProcessingGlobalMaskMakerPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../MemoryModules/VisionReusableMemory/SurfaceLayerMask.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Point32r2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_LIST 100
#define UM_REFRESH_UI (WM_USER + 1099)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)
#define UM_IMAGE_LOT_VIEW_IMAGE_CHANGED (WM_USER + 1013)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgSetupUI, CDialog)

CDlgSetupUI::CDlgSetupUI(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingGlobalMaskMaker* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgSetupUI::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_VisionPara)
    , m_currentLayer(-1)
    , m_imageLotView(nullptr)
    , m_layerDlg(nullptr)
    , m_paraDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
{
}

CDlgSetupUI::~CDlgSetupUI()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_paraDlg;
    delete m_layerDlg;
    delete m_imageLotView;

    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgSetupUI::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(CDlgSetupUI, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgSetupUI::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_REFRESH_UI, OnRefreshUI)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgSetupUI::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &CDlgSetupUI::OnImageLotViewRoiChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_IMAGE_CHANGED, &CDlgSetupUI::OnImageLotViewImageChanged)
END_MESSAGE_MAP()

// CDlgSetupUI 메시지 처리기입니다.
BOOL CDlgSetupUI::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);
    m_imageLotView->SetMessage_SelChangedImage(UM_IMAGE_LOT_VIEW_IMAGE_CHANGED);

    // ImageView을 중심으로 이동시켜 놓자
    Ipvm::Rect32s imageRoi(0, 0, m_pVisionInsp->getReusableMemory().GetInspImageSizeX(),
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY());

    m_imageLotView->NavigateTo(imageRoi);

    // Dialog Control 위치 설정

    // 강제로 Log 위치를 줄어고 파라메터 창을 늘렸다.
    // 나중에 이런것들을 고려해서 UI을 배치할 수 있는 법을 생각해 봤으면 좋겠다.

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    rtTab.right = m_procDlgInfo.m_rtParaArea.left;
    m_TabResult.MoveWindow(rtTab);

    CRect rtPara = m_procDlgInfo.m_rtParaArea;
    rtPara.bottom = rtTab.bottom;

    m_TabResult.InsertItem(0, _T("Layer"));
    m_TabResult.InsertItem(1, _T("Debug"));
    m_TabResult.InsertItem(2, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_layerDlg = new DlgLayer(this);
    m_layerDlg->Create(IDD_DIALOG_LAYER, this);
    m_layerDlg->MoveWindow(rtTab);
    m_layerDlg->ShowWindow(SW_SHOW);

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_paraDlg = new DlgPara(rtPara, m_pVisionInsp, this);
    m_paraDlg->Create(IDD_DIALOG_PARA, this);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgSetupUI::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();

    m_paraDlg->DestroyWindow();
    m_layerDlg->DestroyWindow();
}

void CDlgSetupUI::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgSetupUI::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp);
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());
    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());

    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();
}

void CDlgSetupUI::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_layerDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonDebugInfoDlg->ShowWindow(nTab == 1 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 2 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT CDlgSetupUI::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    // 현재 Pane 선택때마다 ImageLotView가 Pane ROI을 그리고 있기에
    // Pane ROI 을 지우기 위해 호출했다.
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp->m_visionUnit.GetPreviousVisionProcessing(m_pVisionInsp));

    // 보여주는 화면을 사용자가 바꾸었다 그것에 맞추어 배경을 다시 계산해서 보여주자
    m_sourceImage.Free();

    SetROI();
    SetDisplayImage();

    m_imageLotView->NavigateTo(Ipvm::Rect32s(m_sourceImage));

    return 0L;
}

LRESULT CDlgSetupUI::OnImageLotViewRoiChanged(WPARAM, LPARAM)
{
    GetROI();
    SetDisplayImage();

    return 0;
}

LRESULT CDlgSetupUI::OnImageLotViewImageChanged(WPARAM imageIndex, LPARAM /*lparam*/)
{
    // 보여주는 화면을 사용자가 바꾸었다 그것에 맞추어 배경을 다시 계산해서 보여주자
    m_sourceImage.Free();

    SetROI();
    SetDisplayImage(long(imageIndex));

    return 0L;
}

bool CDlgSetupUI::GetROI()
{
    m_pVisionInsp->GetEdgeAlignResult();

    const auto& scale = m_pVisionInsp->getScale();
    const auto& px2um = scale.pixelToUm();

    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    bool change = false;

    long layerIndex = m_currentLayer;
    if (layerIndex >= 0 && layerIndex < m_pVisionPara->getLayerCount())
    {
        auto& specSingleLayer = m_pVisionPara->getLayer(layerIndex).m_single;

        if (specSingleLayer.count() == long(m_resultLayer.m_pre.m_userROIs.size()))
        {
            for (long roiIndex = 0; roiIndex < specSingleLayer.count(); roiIndex++)
            {
                auto& spec = specSingleLayer[roiIndex];
                auto result = m_resultLayer.m_pre.m_userROIs[roiIndex];

                bool isROI = true;

                CString key;
                key.Format(_T("%d"), roiIndex);

                switch (spec.getType())
                {
                    case UserRoiType::Rect:
                    case UserRoiType::Ellipse_32f:
                        m_imageLotView->ROI_Get(key, result.m_rect);
                        break;

                    case UserRoiType::Polygon_32f:
                        m_imageLotView->ROI_Get(key, result.m_polygon);
                        break;

                    default:
                        isROI = false;
                        break;
                }

                if (!isROI)
                    continue;

                if (result != m_resultLayer.m_pre.m_userROIs[roiIndex])
                {
                    //-------------------------------------------------------------
                    // 위치가 변경된 Roi 만 갱신한다
                    // 실제 um 단위인 Roi가 Image 좌표계로 변환되면서
                    // 실수에서 정수로 변화하며 버림이 발생 할 수 있는데
                    // 무조건 다시 덮어쓰다보면 Job이동시 Resolution이 바뀌면서
                    // 버림에 의한 오차가 누적될까봐 이런 작업을 미리 해두었다.
                    //-------------------------------------------------------------

                    m_resultLayer.m_pre.m_userROIs[roiIndex] = result;

                    spec.Set(px2um, imageCenter, m_resultLayer.m_pre, roiIndex);
                    change = true;
                }

                key.Empty();
            }
        }
    }

    return change;
}

void CDlgSetupUI::SetROI()
{
    m_imageLotView->ROI_RemoveAll();
    m_pVisionInsp->GetEdgeAlignResult(); //kircheis_USI_OA

    Ipvm::Rect32s rtROITemp = Ipvm::Rect32s(0, 0, 100, 100);

    m_pVisionInsp->GetEdgeAlignResult();

    const auto& scale = m_pVisionInsp->getScale();
    const auto& px2um = scale.pixelToUm();

    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    m_resultLayer = ResultLayer();

    long layerIndex = m_currentLayer;

    if (layerIndex >= 0 && layerIndex < m_pVisionPara->getLayerCount())
    {
        auto& specLayer = m_pVisionPara->getLayer(layerIndex);

        m_resultLayer = ResultLayer(specLayer, px2um, imageCenter);

        for (long roiIndex = 0; roiIndex < long(m_resultLayer.m_pre.m_userROIs.size()); roiIndex++)
        {
            CString key;
            key.Format(_T("%d"), roiIndex);

            auto& resultRoi = m_resultLayer.m_pre.m_userROIs[roiIndex];
            switch (resultRoi.m_type)
            {
                case UserRoiType::Rect:
                case UserRoiType::Ellipse_32f:
                    m_imageLotView->ROI_Add(key, _T(""), resultRoi.m_rect, RGB(0, 255, 0), TRUE, TRUE);
                    break;

                case UserRoiType::Polygon_32f:
                    m_imageLotView->ROI_Add(key, _T(""), resultRoi.m_polygon, RGB(0, 255, 0), TRUE, TRUE);
                    break;
            }

            key.Empty();
        }
    }

    m_imageLotView->ROI_Show(TRUE);
}

void CDlgSetupUI::SetDisplayImage(long imageIndex)
{
    CreateBackImage(imageIndex);

    Ipvm::Image8u imageMask;

    if (m_sourceImage.GetMem() == nullptr)
        return;

    m_imageLotView->SetImage(m_sourceImage);

    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(imageMask))
    {
        return;
    }

    if (m_otherMaskImage.GetMem() != nullptr)
    {
        Ipvm::ImageProcessing::Copy(m_otherMaskImage, Ipvm::Rect32s(m_otherMaskImage), imageMask);
    }
    else
    {
        Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(imageMask), BACKGROUNDGV, imageMask);
    }

    if (m_currentLayer < 0 || m_currentLayer >= m_pVisionPara->getLayerCount())
    {
        return;
    }

    if (!m_pVisionInsp->CreateSingleLayer(m_currentLayer, m_resultLayer))
    {
        return;
    }

    auto* layerMask = m_pVisionInsp->getReusableMemory().GetSurfaceLayerMask(m_resultLayer.m_surfaceMaskName);
    if (layerMask == nullptr)
        return;

    BYTE backLevel[256];
    for (long intensity = 0; intensity < 256; intensity++)
    {
        backLevel[intensity] = CAST_BYTE(252 * intensity / 255 + 1);
    }

    for (long y = 0; y < m_sourceImage.GetSizeY(); y++)
    {
        auto* src_y = m_sourceImage.GetMem(0, y);
        auto* mask_y = imageMask.GetMem(0, y);
        auto* layer_y = layerMask->GetMem(0, y);
        for (long x = 0; x < m_sourceImage.GetSizeX(); x++)
        {
            if (layer_y[x] == 0)
            {
                if (mask_y[x] == BACKGROUNDGV)
                {
                    mask_y[x] = backLevel[src_y[x]];
                }
            }
            else
            {
                mask_y[x] = BACKGROUNDGV;
            }
        }
    }

    PatternImage pattern;
    if (pattern.setThresholdImage(imageMask))
    {
        m_imageLotView->SetImage(pattern);
    }
    else
    {
        MessageBox(_T("Not enough memory."));
    }
}

long CDlgSetupUI::GetCurrentLayer() const
{
    return m_currentLayer;
}

void CDlgSetupUI::SetCurrentLayer(long layerIndex)
{
    m_currentLayer = layerIndex;
    m_pVisionInsp->CreateLayersBeforeIndex(true, m_currentLayer);

    FreeOtherMaskImage();
    SetROI();

    PostMessage(UM_REFRESH_UI);
}

bool CDlgSetupUI::isLayerValid(long layerIndex)
{
    return m_pVisionInsp->isLayerValid(layerIndex);
}

CVisionProcessingGlobalMaskMakerPara& CDlgSetupUI::GetPara()
{
    return *m_pVisionPara;
}

void CDlgSetupUI::CreateBackImage(long imageIndex)
{
    if (m_sourceImage.GetMem() == nullptr)
    {
        m_pVisionInsp->GetEdgeAlignResult();
        m_pVisionInsp->CreateSetupImage(imageIndex, m_sourceImage);
    }

    bool otherLayerDisplay = m_paraDlg->m_displayOtherLayerMask;

    if (!otherLayerDisplay)
    {
        m_otherMaskImage.Free();
        return;
    }

    if (m_otherMaskImage.GetMem() != nullptr)
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(m_otherMaskImage))
        return;

    const auto& scale = m_pVisionInsp->getScale();
    const auto& px2um = scale.pixelToUm();

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(m_otherMaskImage), BACKGROUNDGV, m_otherMaskImage);

    Ipvm::Point32r2 imageCenter(m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    for (long layer = 0; layer < m_pVisionPara->getLayerCount(); layer++)
    {
        if (layer == m_currentLayer)
            continue;

        auto& specLayer = m_pVisionPara->getLayer(layer);

        ResultLayer resultLayer(specLayer, px2um, imageCenter);

        if (!m_pVisionInsp->CreateSingleLayer(layer, resultLayer))
            break;

        auto* layerMask = m_pVisionInsp->getReusableMemory().AddSurfaceLayerMaskClass(resultLayer.m_surfaceMaskName);
        if (layerMask == nullptr)
            break;

        auto& mask = layerMask->GetMask();
        for (long y = 0; y < mask.GetSizeY(); y++)
        {
            auto* mask_y = mask.GetMem(0, y);
            auto* dst_y = m_otherMaskImage.GetMem(0, y);
            for (long x = 0; x < mask.GetSizeX(); x++)
            {
                if (mask_y[x])
                {
                    dst_y[x] = DARKOBJECTPIXELGV;
                }
            }
        }
    }
}

void CDlgSetupUI::FreeOtherMaskImage()
{
    m_otherMaskImage.Free();
}

LRESULT CDlgSetupUI::OnRefreshUI(WPARAM, LPARAM)
{
    m_otherMaskImage.Free();
    m_layerDlg->UpdateLayerList();
    m_paraDlg->UpdateLayerMaskList();
    SetDisplayImage();

    return 0;
}
