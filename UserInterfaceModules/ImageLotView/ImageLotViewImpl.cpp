//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageLotViewImpl.h"

//CPP_2_________________________________ This project's headers
#include "Dialog3D.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../ImageViewEx/ImageViewEx.h"

//CPP_4_________________________________ External library headers
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_5_________________________________ Standard library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/Image8u.h> //kircheis_WB
#include <Ipvm/Base/Image8u3.h> //kircheis_WB
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Size32s2.h>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define LAYER_CROSSLINE (1)
#define LAYER_PANE_ROI (2)

//CPP_7_________________________________ Implementation body
//
enum enumTimer
{
    enumTimer_BackPane,
    enumTimer_NextPane,
    enumTimer_UpdateCurrentPaneZoom,
    enumTimer_ChangeOverlayMode,
};

ImageLotViewImpl::ImageLotViewImpl(const CRect& roi, const IllumInfo2D& illumInfo2D,
    const InspectionAreaInfo& inspectionAreaInfo, VisionTrayScanSpec& visionTrayScanSpec,
    const VisionImageLot& imageLot, const VisionImageLotInsp& imageLotInsp, bool showInitialPaneArea, CWnd* pParent,
    const enSideVisionModule i_eSideVisionModule)
    : CDialog(ImageLotViewImpl::IDD, pParent)
    , m_coreView(nullptr)
    , m_mode(ImageLotView::Mode::LotRawAndInspImage)
    , m_overlay_mode(0)
    , m_enabled_paneWindow(true)
    , m_startPosition(roi)
    , m_illumInfo2D(illumInfo2D)
    , m_inspectionAreaInfo(inspectionAreaInfo)
    , m_visionTrayScanSpec(visionTrayScanSpec)
    , m_scale(imageLot.GetScale())
    , m_imageLot(&imageLot)
    , m_imageLotInsp(&imageLotInsp)
    , m_paneSelChangedMsg(0)
    , m_roiChangedMsg(0)
    , m_currentImageIndex(0)
    , m_showInitialPaneArea(showInitialPaneArea)
    , m_selChangedRawMsg(0)
    , m_selChangedImageMsg(0)
    , m_display_raw(false)
    , m_eSideVisionModule(i_eSideVisionModule)
{
}

ImageLotViewImpl::ImageLotViewImpl(const CRect& roi, const IllumInfo2D& illumInfo2D,
    const InspectionAreaInfo& inspectionAreaInfo, VisionTrayScanSpec& visionTrayScanSpec,
    const VisionImageLot& imageLot, bool showInitialPaneArea, CWnd* pParent,
    const enSideVisionModule i_eSideVisionModule)
    : CDialog(ImageLotViewImpl::IDD, pParent)
    , m_coreView(nullptr)
    , m_mode(ImageLotView::Mode::LotRawImage)
    , m_overlay_mode(0)
    , m_startPosition(roi)
    , m_illumInfo2D(illumInfo2D)
    , m_inspectionAreaInfo(inspectionAreaInfo)
    , m_visionTrayScanSpec(visionTrayScanSpec)
    , m_scale(imageLot.GetScale())
    , m_imageLot(&imageLot)
    , m_imageLotInsp(nullptr)
    , m_paneSelChangedMsg(0)
    , m_roiChangedMsg(0)
    , m_currentImageIndex(0)
    , m_showInitialPaneArea(showInitialPaneArea)
    , m_selChangedRawMsg(0)
    , m_selChangedImageMsg(0)
    , m_display_raw(false)
    , m_eSideVisionModule(i_eSideVisionModule)
{
}

ImageLotViewImpl::ImageLotViewImpl(const CRect& roi, const IllumInfo2D& illumInfo2D,
    const InspectionAreaInfo& inspectionAreaInfo, VisionTrayScanSpec& visionTrayScanSpec, const VisionScale& scale,
    const VisionImageLotInsp& imageLotInsp, CWnd* pParent, const enSideVisionModule i_eSideVisionModule)
    : CDialog(ImageLotViewImpl::IDD, pParent)
    , m_coreView(nullptr)
    , m_mode(ImageLotView::Mode::LotInspImage)
    , m_overlay_mode(0)
    , m_startPosition(roi)
    , m_illumInfo2D(illumInfo2D)
    , m_inspectionAreaInfo(inspectionAreaInfo)
    , m_visionTrayScanSpec(visionTrayScanSpec)
    , m_scale(scale)
    , m_imageLot(nullptr)
    , m_imageLotInsp(&imageLotInsp)
    , m_paneSelChangedMsg(0)
    , m_roiChangedMsg(0)
    , m_currentImageIndex(0)
    , m_showInitialPaneArea(FALSE)
    , m_selChangedRawMsg(0)
    , m_selChangedImageMsg(0)
    , m_display_raw(false)
    , m_eSideVisionModule(i_eSideVisionModule)
{
}

ImageLotViewImpl::~ImageLotViewImpl()
{
    delete m_coreView;
}

void ImageLotViewImpl::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_3DVIEW, m_button3DView);
    DDX_Control(pDX, IDC_BUTTON_2D_COLOR_VIEW, m_button2DColor); //kircheis_WB
    DDX_Control(pDX, IDC_BUTTON_PREV, m_buttonPrev);
    DDX_Control(pDX, IDC_BUTTON_NEXT, m_buttonNext);
    DDX_Control(pDX, IDC_COMBO_PANE, m_comboPane);
    DDX_Control(pDX, IDC_COMBO_PANE_X, m_comboPaneX);
    DDX_Control(pDX, IDC_COMBO_PANE_Y, m_comboPaneY);
    DDX_Control(pDX, IDC_CHECK_RAW_IMAGE, m_checkRawImage);
}

BEGIN_MESSAGE_MAP(ImageLotViewImpl, CDialog)
ON_WM_SIZE()
ON_BN_CLICKED(IDC_BUTTON_PREV, &ImageLotViewImpl::OnBnClickedButtonPrev)
ON_BN_CLICKED(IDC_BUTTON_NEXT, &ImageLotViewImpl::OnBnClickedButtonNext)
ON_CBN_SELCHANGE(IDC_COMBO_PANE, &ImageLotViewImpl::OnCbnSelchangeComboPane)
ON_CBN_SELCHANGE(IDC_COMBO_PANE_X, &ImageLotViewImpl::OnCbnSelchangeComboPaneXY)
ON_CBN_SELCHANGE(IDC_COMBO_PANE_Y, &ImageLotViewImpl::OnCbnSelchangeComboPaneXY)
ON_BN_CLICKED(IDC_CHECK_RAW_IMAGE, &ImageLotViewImpl::OnBnClickedCheckRawImage)
ON_BN_CLICKED(IDC_BUTTON_3DVIEW, &ImageLotViewImpl::OnBnClickedButton3dview)
ON_BN_CLICKED(IDC_BUTTON_2D_COLOR_VIEW, &ImageLotViewImpl::OnBnClickedButton2dColorview) //kircheis_WB
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BUTTON_COLOR_UNIFROM, &ImageLotViewImpl::OnBnClickedButtonColorUnifrom)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// ImageLotViewImpl message handlers
// MB_YESNOCANCEL, MB_OKCANCEL, MB_OK 3가지를 구분하여 사용한다.
// m_nMBType의 기본값은 MB_OK이다.

BOOL ImageLotViewImpl::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_startPosition);

    m_stateView.Create(_T(""), WS_CHILD | WS_VISIBLE | WS_BORDER, CRect(0, 0, 0, 0), this);
    m_coreView = new ImageViewEx(GetSafeHwnd(), Ipvm::Rect32s(0, 0, 0, 0), 0, m_eSideVisionModule);

    m_coreView->RegisterCallback_ROIChangeEnd(GetSafeHwnd(), this, callback_roiChanged);

    arraymentUI();
    UpdateState();
    OnJobChanged(true, m_showInitialPaneArea);

    GetDlgItem(IDC_BUTTON_COLOR_UNIFROM)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_BUTTON_2D_COLOR_VIEW)->ShowWindow(SW_HIDE);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void ImageLotViewImpl::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
    if (m_stateView.GetSafeHwnd() == NULL)
    {
        return;
    }

    arraymentUI();
}

void ImageLotViewImpl::arraymentUI()
{
    CRect window;
    GetClientRect(window);
    auto cx = window.Width();
    auto cy = window.Height();

    m_stateView.MoveWindow(0, cy - 30, cx, 30);

    CRect rtBtnPrev;
    m_buttonPrev.GetWindowRect(rtBtnPrev);
    ScreenToClient(rtBtnPrev);

    m_buttonPrev.MoveWindow(0, rtBtnPrev.top, cx / 2, rtBtnPrev.Height());
    m_buttonNext.MoveWindow(cx - cx / 2, rtBtnPrev.top, cx / 2, rtBtnPrev.Height());

    ::MoveWindow(m_coreView->GetSafeHwnd(), 0, rtBtnPrev.bottom + 3, cx, cy - rtBtnPrev.bottom - 33, TRUE);
}

void ImageLotViewImpl::OnCbnSelchangeComboPane()
{
    const long horPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayX.size();
    //const long verPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayY.size();
    const long newPaneID = m_comboPane.GetCurSel();

    m_comboPaneX.SetCurSel(newPaneID % horPaneCount);
    m_comboPaneY.SetCurSel(newPaneID / horPaneCount);

    if (m_visionTrayScanSpec.SetCurrentPaneID(newPaneID))
    {
        bool display_raw = m_display_raw;
        CheckRawButtonCheckForCurrentPane(display_raw);
        OnBnClickedButtonCurr();

        ZoomPaneFit(true);

        if (m_paneSelChangedMsg >= WM_USER)
        {
            GetParent()->PostMessage(m_paneSelChangedMsg, WPARAM(newPaneID));
        }
    }
}

void ImageLotViewImpl::OnCbnSelchangeComboPaneXY()
{
    long x = m_comboPaneX.GetCurSel();
    long y = m_comboPaneY.GetCurSel();

    const long horPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayX.size();

    m_comboPane.SetCurSel(y * horPaneCount + x);
    OnCbnSelchangeComboPane();
}

void ImageLotViewImpl::SystemEvent_BackPane()
{
    SetTimer(enumTimer_BackPane, 50, nullptr);
}

void ImageLotViewImpl::SystemEvent_NextPane()
{
    SetTimer(enumTimer_NextPane, 50, nullptr);
}

void ImageLotViewImpl::SystemEvent_UpdateCurrentPaneZoom()
{
    SetTimer(enumTimer_UpdateCurrentPaneZoom, 50, nullptr);
}

void ImageLotViewImpl::SystemEvent_ChangeOverlayMode()
{
    m_overlay_mode++;
    if (m_overlay_mode > 1)
        m_overlay_mode = 0;

    SetTimer(enumTimer_ChangeOverlayMode, 50, nullptr);
}

void ImageLotViewImpl::SetMode(ImageLotView::Mode mode)
{
    m_mode = mode;
    UpdateUIState();
}

void ImageLotViewImpl::SetPaneWindowEnabled(bool enabled)
{
    m_enabled_paneWindow = enabled;
    UpdateUIState();
}

void ImageLotViewImpl::SetInspectionAreaInfo(const InspectionAreaInfo& inspectionAreaInfo)
{
    m_inspectionAreaInfo = inspectionAreaInfo;
    OnJobChanged(true, m_showInitialPaneArea);
}

void ImageLotViewImpl::SetMessage_RoiChanged(UINT message)
{
    m_roiChangedMsg = message;
}

void ImageLotViewImpl::SetMessage_SelChangedPane(UINT message)
{
    m_paneSelChangedMsg = message;
}

void ImageLotViewImpl::SetMessage_SelChangedRaw(UINT message)
{
    m_selChangedRawMsg = message;
}

void ImageLotViewImpl::SetMessage_SelChangedImage(UINT message)
{
    m_selChangedImageMsg = message;
}

void ImageLotViewImpl::SetTarget(VisionImageLot& imageLot, VisionImageLotInsp& imageLotInsp)
{
    m_imageLot = &imageLot;
    m_imageLotInsp = &imageLotInsp;
}

void ImageLotViewImpl::ShowImage(const long frameIndex)
{
    long imageCount = getImageFrameCount();

    if (imageCount == 0)
    {
        ::AfxMessageBox(_T("No image"), MB_ICONWARNING | MB_OK);
        return;
    }

    m_currentImageIndex = min(max(0, frameIndex), imageCount - 1);
    OnBnClickedButtonCurr();
}

void ImageLotViewImpl::ShowPane(const long paneIndex, bool showRaw)
{
    if (paneIndex < 0 || paneIndex >= m_comboPane.GetCount())
    {
        return;
    }

    m_display_raw = showRaw;
    CheckRawButtonCheckForCurrentPane(showRaw);

    const long horPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayX.size();

    m_comboPane.SetCurSel(paneIndex);
    m_comboPaneX.SetCurSel(paneIndex % horPaneCount);
    m_comboPaneY.SetCurSel(paneIndex / horPaneCount);

    OnCbnSelchangeComboPane();
}

void ImageLotViewImpl::ShowImage(const long frameIndex, bool showRaw, bool bIsSrcRaw)
{
    m_display_raw = showRaw;
    CheckRawButtonCheckForCurrentPane(showRaw, bIsSrcRaw);
    ShowImage(frameIndex);
}

void ImageLotViewImpl::OnJobChanged(const bool updateImage, const bool showPaneArea)
{
    m_comboPane.ResetContent();
    m_comboPaneX.ResetContent();
    m_comboPaneY.ResetContent();

    const long horPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayX.size();
    const long verPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayY.size();
    const long paneCount = horPaneCount * verPaneCount;
    const long currentPaneID = m_visionTrayScanSpec.GetCurrentPaneID(paneCount);

    if (paneCount > 0)
    {
        for (long index = 0; index < horPaneCount; index++)
        {
            CString textIndex;
            textIndex.Format(_T("%d"), index + 1);
            m_comboPaneX.AddString(textIndex);
        }

        for (long index = 0; index < verPaneCount; index++)
        {
            CString textIndex;
            textIndex.Format(_T("%d"), index + 1);
            m_comboPaneY.AddString(textIndex);
        }

        for (long pane = 0; pane < paneCount; pane++)
        {
            CString str;
            str.Format(_T("Pane %d"), pane + 1);

            m_comboPane.AddString(str);
        }

        m_comboPane.SetCurSel(currentPaneID);
        m_comboPaneX.SetCurSel(currentPaneID % horPaneCount);
        m_comboPaneY.SetCurSel(currentPaneID / horPaneCount);
    }

    UpdateUIState();

    if (getImageFrameCount() > 0)
    {
        if (updateImage)
        {
            m_currentImageIndex = 0;
            OnBnClickedButtonCurr();
        }
    }
    else
    {
        m_button3DView.ShowWindow(SW_HIDE);
        m_button2DColor.ShowWindow(SW_HIDE); //kircheis_WB
        m_checkRawImage.EnableWindow(FALSE);
    }

    if (paneCount > 0 && getImageFrameCount() > 0 && showPaneArea)
    {
        ZoomPaneFit(true);
    }
}

bool ImageLotViewImpl::IsShowRaw()
{
    return getCurrentRawMode();
}

long ImageLotViewImpl::GetCurrentFrame()
{
    return m_currentImageIndex;
}

long ImageLotViewImpl::GetCurrentPane()
{
    return m_comboPane.GetCurSel();
}

void ImageLotViewImpl::OnBnClickedCheckRawImage()
{
    m_display_raw = getCurrentRawMode();

    if (m_selChangedRawMsg >= WM_USER)
    {
        GetParent()->PostMessage(m_selChangedRawMsg);
    }

    OnBnClickedButtonCurr();
}

void ImageLotViewImpl::OnBnClickedButton2dColorview() //kircheis_WB
{
    auto& systemConfig = SystemConfig::GetInstance();
    static const bool bIs2DVision = (systemConfig.GetVisionType() == VISIONTYPE_2D_INSP);
    static const bool bIsNgrvVision = (systemConfig.GetVisionType() == VISIONTYPE_NGRV_INSP);
    static const bool bIsSideVision = (systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);
    static const bool bIsSWIRVision = (systemConfig.GetVisionType() == VISIONTYPE_SWIR_INSP); //kircheis_SWIR

    if (bIsSWIRVision == true) //kircheis_SWIR
    {
        // SWIR Vision은 Color View가 없다
        return;
    }

    if (bIsNgrvVision == true)
    {
        //{{ Intensity Uniformity Test
        CalcGrayTargetSubAreaIntensities(getCurrentColorImageFrame());
        //}}
    }
    else
    {
        BOOL bUseColorFrame = m_illumInfo2D.m_2D_colorFrame;

        if (bUseColorFrame == FALSE || (bIs2DVision == false && bIsSideVision == false))
            return;

        auto image_R = getImageFrame(m_illumInfo2D.m_2D_colorFrameIndex_red, m_eSideVisionModule);
        auto image_G = getImageFrame(m_illumInfo2D.m_2D_colorFrameIndex_green, m_eSideVisionModule);
        auto image_B = getImageFrame(m_illumInfo2D.m_2D_colorFrameIndex_blue, m_eSideVisionModule);

        if (image_R.GetSizeX() != image_G.GetSizeX() || image_R.GetSizeY() != image_G.GetSizeY())
            return;
        if (image_R.GetSizeX() != image_B.GetSizeX() || image_R.GetSizeY() != image_B.GetSizeY())
            return;

        Ipvm::Image8u3 imageColor(image_R.GetSizeX(), image_R.GetSizeY());

        Ipvm::ImageProcessing::CombineRGB(image_R, image_G, image_B, Ipvm::Rect32s(imageColor), imageColor);
        m_coreView->SetImage(imageColor);
    }
}

void ImageLotViewImpl::OnBnClickedButton3dview()
{
    auto gray_z = getImageFrame(0);
    auto gray_v = getImageFrame(1);
    auto zmap = getZmapFrame();
    auto vmap = getVmapFrame();

    if (gray_z.GetSizeX() == 0 || gray_v.GetSizeX() == 0 || zmap.GetSizeX() == 0 || vmap.GetSizeX() == 0)
    {
        MessageBox(_T("ImageLot empty."));
        return;
    }

    Dialog3D dlg(gray_z, gray_v, zmap, vmap, m_scale);
    dlg.DoModal();
}

void ImageLotViewImpl::OnBnClickedButtonPrev()
{
    m_currentImageIndex = max(0, m_currentImageIndex - 1);

    if (m_selChangedImageMsg >= WM_USER)
    {
        GetParent()->PostMessage(m_selChangedImageMsg, m_currentImageIndex);
    }

    OnBnClickedButtonCurr();
}

void ImageLotViewImpl::OnBnClickedButtonCurr()
{
    m_frameText = m_illumInfo2D.getIllumFullName(m_currentImageIndex);

    UpdateState();

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        m_coreView->SetImage(getCurrentColorImageFrame());
    }
    else
    {
        m_coreView->SetImage(getCurrentImageFrame());
    }

    const bool useRawImage = m_checkRawImage.GetCheck() == BST_CHECKED;

    if (useRawImage)
    {
        m_coreView->ImageOverlayShow(LAYER_PANE_ROI);
    }
    else
    {
        // 검사 이미지는 Pane Roi가 의미없다
        m_coreView->ImageOverlayHide(LAYER_PANE_ROI);
    }

    // 3D Button 활성화 타이밍이 마땅치 않아서 일단 여기다 넣음
    bool valid3D = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP);

    if (valid3D)
    {
        if (getZmapFrame().GetSizeX() == 0)
            valid3D = false;

        // Tugu 20190313
        auto zmap = getZmapFrame();
        m_coreView->SetRawImage(zmap);
    }

    if (m_imageLot)
    {
        const auto& imageInfo = m_imageLot->GetInfo();
        CString scanIndex;
        scanIndex.Format(_T("T%d S%d"), imageInfo.m_trayIndex, imageInfo.m_scanAreaIndex);
        GetDlgItem(IDC_SCANINDEX)->SetWindowText(scanIndex);
    }

    m_button3DView.ShowWindow(valid3D ? SW_SHOW : SW_HIDE);

    //{{//kircheis_WB
    auto& systemConfig = SystemConfig::GetInstance();
    static const bool bIs2DVision = (systemConfig.GetVisionType() == VISIONTYPE_2D_INSP);
    static const bool bIsSideVision = (systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIs2DVision == true || bIsSideVision == true)
    {
        m_button2DColor.ShowWindow(SW_SHOW);
        m_button2DColor.EnableWindow(m_illumInfo2D.m_2D_colorFrame);
    }
    else
        m_button2DColor.ShowWindow(SW_HIDE);
    //}}

    UpdateUIState();
}

void ImageLotViewImpl::OnBnClickedButtonNext()
{
    //const bool useRawImage = m_checkRawImage.GetCheck() == BST_CHECKED;

    m_currentImageIndex = min(getImageFrameCount(m_eSideVisionModule) - 1, m_currentImageIndex + 1);

    if (m_selChangedImageMsg >= WM_USER)
    {
        GetParent()->PostMessage(m_selChangedImageMsg, m_currentImageIndex);
    }

    OnBnClickedButtonCurr();
}

void ImageLotViewImpl::callback_roiChanged(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(keyLength);

    auto* impl = (ImageLotViewImpl*)userData;
    if (impl->m_roiChangedMsg >= WM_USER)
    {
        impl->GetParent()->PostMessage(impl->m_roiChangedMsg, (WPARAM)key);
    }
}

void ImageLotViewImpl::OnTimer(UINT_PTR nIDEvent)
{
    KillTimer(nIDEvent);

    Ipvm::Size32s2 imageSize;
    m_coreView->GetImageSize(imageSize);

    switch (nIDEvent)
    {
        case enumTimer_BackPane:
            if (m_comboPane.GetCurSel() > 0)
            {
                ShowPane(m_comboPane.GetCurSel() - 1, m_display_raw);
            }
            break;

        case enumTimer_NextPane:
            if (m_comboPane.GetCurSel() < m_comboPane.GetCount() - 1)
            {
                ShowPane(m_comboPane.GetCurSel() + 1, m_display_raw);
            }
            break;

        case enumTimer_UpdateCurrentPaneZoom:
            if (1)
            {
                ZoomPaneFit(false);
            }
            break;
        case enumTimer_ChangeOverlayMode:
            m_coreView->ImageOverlayClear(LAYER_CROSSLINE);

            switch (m_overlay_mode)
            {
                case 0:
                    m_coreView->ImageOverlayShow(LAYER_CROSSLINE);
                    break;

                case 1:
                    m_coreView->ImageOverlayAdd(LAYER_CROSSLINE,
                        Ipvm::LineSeg32r(0.f, CAST_FLOAT(imageSize.m_h * 0.5f), CAST_FLOAT(imageSize.m_w),
                            CAST_FLOAT(imageSize.m_h * 0.5f)),
                        RGB(0, 255, 0));
                    m_coreView->ImageOverlayAdd(LAYER_CROSSLINE,
                        Ipvm::LineSeg32r(CAST_FLOAT(imageSize.m_w * 0.5f), 0.f, CAST_FLOAT(imageSize.m_w * 0.5f),
                            CAST_FLOAT(imageSize.m_h)),
                        RGB(0, 255, 0));
                    m_coreView->ImageOverlayShow(LAYER_CROSSLINE);
                    break;
            }
            break;
    }

    CDialog::OnTimer(nIDEvent);
}

void ImageLotViewImpl::UpdateState()
{
    CString text;
    text.Format(_T("Scale X : %.2f um, Scale Y : %.2f um, %s"), m_scale.pixelToUm().m_x, m_scale.pixelToUm().m_y,
        (LPCTSTR)m_frameText);
    m_stateView.SetWindowText(text);
}

void ImageLotViewImpl::ZoomImageFit()
{
    Ipvm::Size32s2 imageSize;
    m_coreView->GetImageSize(imageSize);
    Ipvm::Rect32s imageRoi(0, 0, imageSize.m_w, imageSize.m_h);

    m_coreView->NavigateTo(imageRoi);
}

void ImageLotViewImpl::ZoomPaneFit(bool displayPaneROI)
{
    Ipvm::Size32s2 imageSize;
    m_coreView->GetImageSize(imageSize);
    Ipvm::Rect32s rtPane(0, 0, imageSize.m_w, imageSize.m_h);
    Ipvm::Point32r2 ptImageCenter(imageSize.m_w * 0.5f, imageSize.m_h * 0.5f);

    const long paneID = m_comboPane.GetCurSel();

    if (getCurrentRawMode())
    {
        rtPane = Ipvm::Rect32s(0, 0, 0, 0);
        for (auto& fovIndex : m_inspectionAreaInfo.m_fovList)
        {
            if (paneID >= 0 && paneID < (long)m_inspectionAreaInfo.m_unitIndexList.size())
            {
                long unitID = m_inspectionAreaInfo.m_unitIndexList[paneID];
                auto pocketRegion = m_scale.convert_mmToPixel(
                                        m_inspectionAreaInfo.m_parent->GetUnitPocketRegionInFOV(fovIndex, unitID))
                    + Ipvm::Conversion::ToPoint32s2(ptImageCenter);
                rtPane |= pocketRegion;
            }
        }
    }

    m_coreView->ImageOverlayClear(LAYER_PANE_ROI);

    if (getCurrentRawMode() && displayPaneROI)
    {
        const long horPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayX.size();

        CString str;
        str.Format(_T("Pane %d : X[%d] Y[%d]"), paneID + 1, paneID % horPaneCount + 1, paneID / horPaneCount + 1);

        m_coreView->ImageOverlayAdd(LAYER_PANE_ROI, rtPane, RGB(255, 0, 255));
        m_coreView->ImageOverlayAdd(
            LAYER_PANE_ROI, Ipvm::Point32s2(rtPane.m_left, rtPane.m_bottom + 10), str, RGB(255, 0, 255), 10);
    }

    m_coreView->ImageOverlayShow(LAYER_PANE_ROI);
    m_coreView->NavigateTo(rtPane);
}

void ImageLotViewImpl::SetScanIndexField(LPCTSTR text)
{
    GetDlgItem(IDC_SCANINDEX)->SetWindowText(text);
}

bool ImageLotViewImpl::CheckRawButtonCheckForCurrentPane(bool& showRaw, bool bIsSrcRaw)
{
    if (m_imageLotInsp == nullptr || m_imageLotInsp->m_paneIndexForCalculationImage != m_comboPane.GetCurSel())
    {
        m_checkRawImage.EnableWindow(FALSE);

        bool oldCheck = m_checkRawImage.GetCheck() ? true : false;
        if (!bIsSrcRaw) //Stitching을 해야할 경우
            m_checkRawImage.SetCheck(showRaw ? BST_CHECKED : BST_UNCHECKED);
        else //Stitching을 하지않아도 될경우
            m_checkRawImage.SetCheck(FALSE);

        if (!showRaw)
        {
            // 검사영상을 보고 싶어도 영상이 없어서 Raw로 보여줄 수 밖에 없다..
            m_checkRawImage.SetCheck(TRUE);

            if (oldCheck)
            {
                if (m_selChangedRawMsg >= WM_USER)
                {
                    GetParent()->PostMessage(m_selChangedRawMsg);
                }

                return false;
            }
        }
    }
    else
    {
        m_checkRawImage.EnableWindow(TRUE);
        if ((m_checkRawImage.GetCheck() ? true : false) != showRaw)
        {
            m_checkRawImage.SetCheck(showRaw ? BST_CHECKED : BST_UNCHECKED);
            if (m_selChangedRawMsg >= WM_USER)
            {
                GetParent()->PostMessage(m_selChangedRawMsg);
            }
        }
    }

    return true;
}

void ImageLotViewImpl::UpdateUIState()
{
    BOOL enable_Pane = FALSE;
    BOOL showRaw = FALSE;
    long imageFrameCount = getImageFrameCount();

    const long horPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayX.size();
    const long verPaneCount = (long)m_inspectionAreaInfo.m_pocketArrayY.size();
    const long paneCount = horPaneCount * verPaneCount;

    if (m_enabled_paneWindow && paneCount > 0)
    {
        enable_Pane = TRUE;
    }

    if (m_mode != ImageLotView::Mode::SpecifyImage)
    {
        if (m_mode == ImageLotView::Mode::LotRawAndInspImage)
        {
            // Raw와 Insp Image을 모두 보여주는 모드 일때만
            // Raw 버튼을 보여준다
            showRaw = TRUE;
        }

        if (m_mode == ImageLotView::Mode::LotInspImage)
        {
            // InspImage 모드에서는 Pane Roi를 표시하지 않는다
            m_coreView->ImageOverlayHide(LAYER_PANE_ROI);
        }
    }
    else
    {
        // SpecifyImage 모드에서는 Pane Roi를 표시하지 않는다
        m_coreView->ImageOverlayHide(LAYER_PANE_ROI);
    }

    m_comboPane.EnableWindow(enable_Pane);
    m_comboPaneX.EnableWindow(enable_Pane);
    m_comboPaneY.EnableWindow(enable_Pane);
    m_checkRawImage.ShowWindow(showRaw ? SW_SHOW : SW_HIDE);
    m_buttonPrev.EnableWindow(m_currentImageIndex > 0 && m_currentImageIndex < imageFrameCount && imageFrameCount > 0);
    m_buttonNext.EnableWindow(
        m_currentImageIndex >= 0 && m_currentImageIndex < imageFrameCount - 1 && imageFrameCount > 0);
}

bool ImageLotViewImpl::getCurrentRawMode() const
{
    switch (m_mode)
    {
        case ImageLotView::Mode::LotRawAndInspImage:
            return (m_checkRawImage.GetCheck() == BST_CHECKED);

        case ImageLotView::Mode::LotRawImage:
            return true;

        case ImageLotView::Mode::LotInspImage:
            return false;
    }

    return false;
}

long ImageLotViewImpl::getImageFrameCount(const enSideVisionModule i_eSideVisionModule) const
{
    switch (m_mode)
    {
        case ImageLotView::Mode::LotRawAndInspImage:
        case ImageLotView::Mode::LotRawImage:
        case ImageLotView::Mode::SpecifyImage:
            return m_imageLot->GetImageFrameCount(i_eSideVisionModule);

        case ImageLotView::Mode::LotInspImage:
        {
            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP) // Color Image Control - 2021.02
            {
                return long(m_imageLotInsp->m_vecColorImages.size());
            }
            else
            {
                return long(m_imageLotInsp->m_vecImages[i_eSideVisionModule].size());
            }
        }
    }

    return 0;
}

Ipvm::Image8u ImageLotViewImpl::getCurrentImageFrame() const
{
    if (m_eSideVisionModule < enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        return getImageFrame(m_currentImageIndex);
    }
    else if (m_eSideVisionModule > enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        AfxMessageBox(_T("ImageLot::getCurrent"));
    }

    return getImageFrame(m_currentImageIndex, m_eSideVisionModule);
}

Ipvm::Image8u3 ImageLotViewImpl::getCurrentColorImageFrame() const
{
    return getColorImageFrame(m_currentImageIndex);
}

Ipvm::Image8u ImageLotViewImpl::getImageFrame(long frameIndex, const enSideVisionModule i_eSideVisionModule) const
{
    Ipvm::Image8u image;

    if (frameIndex < 0)
    {
        return image;
    }

    if (getCurrentRawMode())
    {
        if (m_imageLot && frameIndex < m_imageLot->GetImageFrameCount())
        {
            image = m_imageLot->GetImageFrame(frameIndex, i_eSideVisionModule);
        }
    }
    else
    {
        if (m_imageLotInsp && frameIndex < long(m_imageLotInsp->m_vecImages[i_eSideVisionModule].size()))
        {
            image = m_imageLotInsp->m_vecImages[i_eSideVisionModule][frameIndex];
        }
    }

    return image;
}

Ipvm::Image8u3 ImageLotViewImpl::getColorImageFrame(long frameIndex) const
{
    Ipvm::Image8u3 Colorimage;

    if (frameIndex < 0)
    {
        return Colorimage;
    }

    if (getCurrentRawMode())
    {
        if (m_imageLot && frameIndex < m_imageLot->GetImageFrameCount())
        {
            Colorimage = m_imageLot->GetColorImageFrame(frameIndex);
        }
    }
    else
    {
        if (m_imageLotInsp && frameIndex < long(m_imageLotInsp->m_vecColorImages.size()))
        {
            Colorimage = m_imageLotInsp->m_vecColorImages[frameIndex];
        }
    }

    return Colorimage;
}

Ipvm::Image32r ImageLotViewImpl::getZmapFrame()
{
    Ipvm::Image32r image;

    if (getCurrentRawMode())
    {
        if (m_imageLot)
        {
            image = m_imageLot->GetZmapImage();
        }
    }
    else
    {
        if (m_imageLotInsp)
        {
            image = m_imageLotInsp->m_zmapImage;
        }
    }

    return image;
}

Ipvm::Image16u ImageLotViewImpl::getVmapFrame()
{
    Ipvm::Image16u image;

    if (getCurrentRawMode())
    {
        if (m_imageLot)
        {
            image = m_imageLot->GetVmapImage();
        }
    }
    else
    {
        if (m_imageLotInsp)
        {
            image = m_imageLotInsp->m_vmapImage;
        }
    }

    return image;
}

void ImageLotViewImpl::CalcGrayTargetSubAreaIntensities(Ipvm::Image8u3 image)
{
    std::vector<Ipvm::Rect32s> vecROIs;

    std::vector<double> vecMeanR;
    std::vector<double> vecMeanG;
    std::vector<double> vecMeanB;

    vecMeanR.clear();
    vecMeanG.clear();
    vecMeanB.clear();

    double dMeanValue[3];

    CalcGrayTargetSubArea(image, 10, 10, vecROIs);

    for (long nIndex = 0; nIndex < long(vecROIs.size()); nIndex++)
    {
        Ipvm::Image8u3 calcImage(image, vecROIs[nIndex]);

        Ipvm::ImageProcessing::GetMean(calcImage, Ipvm::Rect32s(calcImage), dMeanValue);

        vecMeanR.push_back(dMeanValue[0]);
        vecMeanG.push_back(dMeanValue[1]);
        vecMeanB.push_back(dMeanValue[2]);
    }

    SaveUniformity(vecMeanR, vecMeanG, vecMeanB);
}

void ImageLotViewImpl::CalcGrayTargetSubArea(
    const Ipvm::Image8u3& image, long i_nAreaCountX, long i_nAreaCountY, std::vector<Ipvm::Rect32s>& rtROIs)
{
    const int rectSizeY = CAST_INT(image.GetSizeY() / i_nAreaCountY);
    const int rectSizeX = CAST_INT(image.GetSizeX() / i_nAreaCountX);

    rtROIs.clear();

    for (int32_t nRectY = 400; nRectY < image.GetSizeY() - 400; nRectY += rectSizeY)
    {
        for (int32_t nRectX = 400; nRectX < image.GetSizeX() - 400; nRectX += rectSizeX)
        {
            rtROIs.push_back(Ipvm::Rect32s(nRectY, nRectX, nRectY + rectSizeY, nRectX + rectSizeX));
        }
    }
}

void ImageLotViewImpl::SaveUniformity(
    std::vector<double> i_vecMeanR, std::vector<double> i_vecMeanG, std::vector<double> i_vecMeanB)
{
    CString strFileName;

    strFileName.Format(_T("C:\\Color Uniformity.csv"));

    FILE* fp = nullptr;
    _tfopen_s(&fp, strFileName, _T("a"));

    fprintf(fp, "\n");
    fprintf(fp, "Red, Green, Blue\n");

    for (int i = 0; i < i_vecMeanR.size(); i++)
    {
        CStringA str;
        str.Format("%d,%f,%f,%f\n", i, i_vecMeanR[i], i_vecMeanG[i], i_vecMeanB[i]);
        fprintf(fp, str);
    }

    fclose(fp);
}

void ImageLotViewImpl::OnBnClickedButtonColorUnifrom()
{
    //{{ Intensity Uniformity Test
    CalcGrayTargetSubAreaIntensities(getCurrentColorImageFrame());
    //}}
}

void ImageLotViewImpl::SetSideVisionModule(const enSideVisionModule i_eSideVisionModule)
{
    if ((i_eSideVisionModule < enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        || (i_eSideVisionModule > enSideVisionModule::SIDE_VISIONMODULE_REAR))
    {
        m_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT;
        return;
    }

    m_eSideVisionModule = i_eSideVisionModule;
}

void ImageLotViewImpl::SetRawImageCheckBox(bool i_bCheck) // Stitch Section Image Combine
{
    m_checkRawImage.SetCheck(i_bCheck);
}