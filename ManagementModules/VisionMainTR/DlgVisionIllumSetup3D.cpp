//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionIllumSetup3D.h"

//CPP_2_________________________________ This project's headers
#include "VisionMainTR.h"

//CPP_3_________________________________ Other projects' headers
#include "../../HardwareModules/dPI_Framegrabber/FrameGrabber.h"
#include "../../HardwareModules/dPI_SyncController/SlitIlluminator.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomNumber.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_PROPERTY_GRID 101
#define UM_GRAB_END_EVENT (WM_USER + 1054)

//CPP_7_________________________________ Implementation body
//
enum
{
    PROPERTY_GRID_ITEM_EXPOSURE_HIGH_TIME = 10,
    PROPERTY_GRID_ITEM_EXPOSURE_INTENSITY_VALUE,
    PROPERTY_GRID_ITEM_CAMERA_VERTICAL_BINNING,
    PROPERTY_GRID_ITEM_SCAN_DEPTH,
    PROPERTY_GRID_ITEM_VIEW_PROFILE_OVERLAY,
    PROPERTY_GRID_ITEM_LIVE_VIEW_INTENSITY_SCALING,
};

enum ProfileOverlayMode
{
    None,
    IntensityProfile,
    MaxIntensityProfile,
    VerticalSharpnessProfile,
    HorizontalSharpnessProfile,
};

IMPLEMENT_DYNAMIC(DlgVisionIllumSetup3D, CDialog)

DlgVisionIllumSetup3D::DlgVisionIllumSetup3D(VisionMainTR& visionMain, CWnd* pParent)
    : CDialog(DlgVisionIllumSetup3D::IDD, pParent)
    , m_visionMain(visionMain)
    , m_imageView(nullptr)
    , m_propertyGrid(nullptr)
    , m_profileOverlayMode(ProfileOverlayMode::None)
    , m_sharpnessProfileScaling(16)
    , m_intensityScaling(1)
{
}

DlgVisionIllumSetup3D::~DlgVisionIllumSetup3D()
{
    delete m_propertyGrid;
    delete m_imageView;
}

void DlgVisionIllumSetup3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgVisionIllumSetup3D, CDialog)
ON_MESSAGE(UM_GRAB_END_EVENT, &DlgVisionIllumSetup3D::OnGrabEndEvent)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_WM_DESTROY()
END_MESSAGE_MAP()

// DlgVisionIllumSetup3D 메시지 처리기입니다.
BOOL DlgVisionIllumSetup3D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(0, 0, rtParentClient.Width(), rtParentClient.Height(), FALSE);

    static const BYTE byMinScanStep
        = (BYTE)(SystemConfig::GetInstance().Get2DScaleX(enSideVisionModule::SIDE_VISIONMODULE_FRONT));

    CRect rtClient;
    GetClientRect(rtClient);
    rtClient.DeflateRect(5, 5);

    m_propertyGrid = new CXTPPropertyGrid();
    m_propertyGrid->Create(::GetContentSubArea(rtClient, 0, 1, 6, 0, 1, 1), this, IDC_PROPERTY_GRID);
    double limitLineGPU = (double)((long)SystemConfig::GetInstance().GetScale3D() * 512);

    auto& para = *m_visionMain.m_pSlitBeam3DCommon;

    if (auto* category = m_propertyGrid->AddCategory(_T("Illumination parameters")))
    {
        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(_T("Illumination time"),
                para.m_illuminationTime_ms, _T("%.3lf ms"), para.m_illuminationTime_ms, 0.016, 200., 0.001)))
        {
            item->SetID(PROPERTY_GRID_ITEM_EXPOSURE_HIGH_TIME);
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<BYTE>(
                _T("Illumination value"), para.m_illuminationValue, _T("%d"), para.m_illuminationValue, 1, 255)))
        {
            if (auto* slider = item->AddSliderControl())
            {
                slider->SetMin(0);
                slider->SetMax(256);
            }

            if (auto* spin = item->AddSpinButton())
            {
                spin->SetMin(0);
                spin->SetMax(256);
            }

            item->SetID(PROPERTY_GRID_ITEM_EXPOSURE_INTENSITY_VALUE);
        }

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Scanning parameters")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(
                _T("Camera vertical binning"), para.m_cameraVerticalBinning, &para.m_cameraVerticalBinning)))
        {
            item->SetID(PROPERTY_GRID_ITEM_CAMERA_VERTICAL_BINNING);
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<BYTE>(
                _T("Scan step"), para.m_scanStep_um, _T("%d um"), para.m_scanStep_um, byMinScanStep, 40)))
        {
        }

        const double scanDepthMax_um = para.m_cameraVerticalBinning ? 6000. : limitLineGPU;

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(
                _T("Scan depth"), para.m_scanDepth_um, _T("%.0lf um"), para.m_scanDepth_um, 500., scanDepthMax_um, 1.)))
        {
            item->SetID(PROPERTY_GRID_ITEM_SCAN_DEPTH);
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(
                _T("Scan length"), para.m_scanLength_mm, _T("%.0lf mm"), para.m_scanLength_mm, 10., 135., 1.)))
        {
            if (m_visionMain.m_pMessageSocket->IsConnected())
            {
                item->SetReadOnly(TRUE);
            }
        }

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Algorithm parameters")))
    {
        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(
                _T("Noise threshold"), para.m_noiseThreshold, _T("%.1lf gv"), para.m_noiseThreshold, 0.1, 30., 0.1)))
        {
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(
                _T("Noise threshold (Final filtering)"), para.m_noiseThreshold_FinalFiltering, _T("%.1lf gv"),
                para.m_noiseThreshold_FinalFiltering, 0.1, 30., 0.1))) //kircheis_SDK127
        {
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<BYTE>(
                _T("Slitbeam thickness"), para.m_beamThickness_um, _T("%d um"), para.m_beamThickness_um, 50, 250)))
        {
        }

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Miscellaneous")))
    {
        if (auto* item = category->AddChildItem(new ::CXTPPropertyGridItemEnum(
                _T("View profile overlay"), m_profileOverlayMode, &m_profileOverlayMode)))
        {
            item->GetConstraints()->AddConstraint(_T("None"), ProfileOverlayMode::None);
            item->GetConstraints()->AddConstraint(_T("Intensity profile"), ProfileOverlayMode::IntensityProfile);
            item->GetConstraints()->AddConstraint(_T("Max intensity profile"), ProfileOverlayMode::MaxIntensityProfile);
            item->GetConstraints()->AddConstraint(
                _T("Vertical sharpness profile"), ProfileOverlayMode::VerticalSharpnessProfile);
            item->GetConstraints()->AddConstraint(
                _T("Horizontal sharpness profile"), ProfileOverlayMode::HorizontalSharpnessProfile);
            item->SetID(PROPERTY_GRID_ITEM_VIEW_PROFILE_OVERLAY);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Sharpness profile scaling"), m_sharpnessProfileScaling, &m_sharpnessProfileScaling)))
        {
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Live view intensity scaling"), m_intensityScaling, &m_intensityScaling)))
        {
            item->SetID(PROPERTY_GRID_ITEM_LIVE_VIEW_INTENSITY_SCALING);
        }
    }

    m_propertyGrid->HighlightChangedItems(TRUE);

    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(::GetContentSubArea(rtClient, 1, 6, 6, 0, 1, 1)), 0,
        enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    m_imageView->ROISet(_T("ROI"), _T("Profile"), Ipvm::Rect32s(0, 0, 100, 100), RGB(255, 0, 0));

    FrameGrabber::GetInstance().set_live_image_size(0, 0, para.m_cameraVerticalBinning, true);

    SlitIlluminator::GetInstance().Download(INSTRUCTION_CHANNEL0_INTENSITY, para.m_illuminationValue);
    SyncController::GetInstance().TurnOnLight(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    FrameGrabber::GetInstance().live_on(
        GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    ShowWindow(SW_SHOW);

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

LRESULT DlgVisionIllumSetup3D::OnGrabEndEvent(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    static Ipvm::Image8u grabImage;

    FrameGrabber::GetInstance().get_live_image(grabImage);

    const int imageSizeX = grabImage.GetSizeX();
    const int imageSizeY = grabImage.GetSizeY();

    Ipvm::ImageProcessing::Multiply(grabImage, BYTE(m_intensityScaling), Ipvm::Rect32s(grabImage), 0, grabImage);

    static BOOL bFirst = FALSE;
    if (SystemConfig::GetInstance().m_bHardwareExist || !bFirst)
        m_imageView->SetImage(grabImage);
    bFirst = true;

    auto& para = *m_visionMain.m_pSlitBeam3DCommon;

    const int grabImageHeight = FrameGrabber::GetInstance().get_grab_image_height_3D(
        para.m_scanDepth_um, para.m_cameraVerticalBinning, SystemConfig::GetInstance().m_slitbeamHeightScaling);

    Ipvm::Rect32s rtFullImage(grabImage);
    const int offsetY = (rtFullImage.Height() - grabImageHeight) / 2
        + SystemConfig::GetInstance().m_slitbeamCameraOffsetY / (para.m_cameraVerticalBinning ? 2 : 1);

    Ipvm::Rect32s rtRoiImage(0, offsetY, rtFullImage.m_right, offsetY + grabImageHeight);

    Ipvm::LineSeg32r centerLine = {0.f, 0.5f * (rtRoiImage.m_top + rtRoiImage.m_bottom - 1), rtFullImage.m_right - 1.f,
        0.5f * (rtRoiImage.m_top + rtRoiImage.m_bottom - 1)};

    m_imageView->ImageOverlayClear();
    m_imageView->ImageOverlayAdd(rtRoiImage, RGB(0, 255, 0));
    m_imageView->ImageOverlayAdd(centerLine, RGB(255, 0, 0));

    if (m_profileOverlayMode == ProfileOverlayMode::IntensityProfile)
    {
        Ipvm::Rect32s profileRoi;

        m_imageView->ROIGet(_T("ROI"), profileRoi);

        profileRoi &= rtFullImage;

        if (!profileRoi.IsRectEmpty())
        {
            std::vector<double> horizontalSums(imageSizeY, 0.);

            for (long y = profileRoi.m_top; y < profileRoi.m_bottom; y++)
            {
                auto* img = grabImage.GetMem(0, y);

                auto& horizontalSum = horizontalSums[y];

                for (long x = profileRoi.m_left; x < profileRoi.m_right; x++)
                {
                    horizontalSum += img[x];
                }

                horizontalSum /= profileRoi.Width();
            }

            Ipvm::Rect32s drawingROI(profileRoi);
            drawingROI.m_top--;
            drawingROI.m_bottom++;
            drawingROI.m_right = 256;
            drawingROI.m_left = -1;

            m_imageView->ImageOverlayAdd(drawingROI, RGB(255, 255, 0));

            for (long y = profileRoi.m_top; y < profileRoi.m_bottom; y++)
            {
                Ipvm::LineSeg32r xxxx = {0.f, float(y), float(horizontalSums[y]), float(y)};
                m_imageView->ImageOverlayAdd(xxxx, RGB(255, 0, 255));
            }
        }
    }
    else if (m_profileOverlayMode == ProfileOverlayMode::MaxIntensityProfile)
    {
        Ipvm::Rect32s profileRoi;

        m_imageView->ROIGet(_T("ROI"), profileRoi);

        profileRoi &= rtFullImage;

        if (!profileRoi.IsRectEmpty())
        {
            std::vector<unsigned char> intensityMaxValues(imageSizeX, 0);

            for (long y = profileRoi.m_top; y < profileRoi.m_bottom; y++)
            {
                auto* img = grabImage.GetMem(0, y);

                for (long x = 0; x < imageSizeX; x++)
                {
                    if (img[x] > intensityMaxValues[x])
                    {
                        intensityMaxValues[x] = img[x];
                    }
                }
            }

            long intensitySum = 0;

            for (auto value : intensityMaxValues)
            {
                intensitySum += value;
            }

            const float meanIntensity = float(double(intensitySum) / imageSizeX);

            Ipvm::Rect32s drawingROI(0, 0, imageSizeX, 256);

            m_imageView->ImageOverlayAdd(drawingROI, RGB(255, 255, 0));

            CString str;
            str.Format(_T("Mean intensity : %.1f"), meanIntensity);

            m_imageView->ImageOverlayAdd(Ipvm::Point32s2(0, 260), LPCTSTR(str), RGB(255, 255, 0), 32);

            for (long x = 0; x < imageSizeX; x++)
            {
                Ipvm::LineSeg32r xxxx = {float(x), float(0.f), float(x), float(intensityMaxValues[x])};
                m_imageView->ImageOverlayAdd(xxxx, RGB(255, 0, 255));
            }

            str.Empty();
        }
    }
    else if (m_profileOverlayMode == ProfileOverlayMode::VerticalSharpnessProfile)
    {
        for (long y = 0; y < imageSizeY; y++)
        {
            auto* img = grabImage.GetMem(0, y);

            double horizontalSumLeft = 0.;
            double horizontalDiffSumLeft = 0.;

            for (long x = 1; x < imageSizeX / 4; x++)
            {
                horizontalSumLeft += img[x - 1] + img[x];
                horizontalDiffSumLeft += fabs(img[x - 1] - img[x]);
            }

            const double sharpnessProfileLeft
                = horizontalSumLeft ? (256 * m_sharpnessProfileScaling * horizontalDiffSumLeft / horizontalSumLeft) : 0;

            Ipvm::LineSeg32r left = {0.f, float(y), float(sharpnessProfileLeft), float(y)};
            m_imageView->ImageOverlayAdd(left, RGB(255, 0, 255));

            double horizontalSumRight = 0.;
            double horizontalDiffSumRight = 0.;

            for (long x = imageSizeX / 4; x < imageSizeX - 1; x++)
            {
                horizontalSumRight += img[x - 1] + img[x];
                horizontalDiffSumRight += fabs(img[x - 1] - img[x]);
            }

            const double sharpnessProfileRight = horizontalSumRight
                ? (256 * m_sharpnessProfileScaling * horizontalDiffSumRight / horizontalSumRight)
                : 0;

            Ipvm::LineSeg32r right
                = {float(imageSizeX - 1), float(y), float(imageSizeX - 1 - sharpnessProfileRight), float(y)};
            m_imageView->ImageOverlayAdd(right, RGB(255, 0, 255));
        }
    }
    else if (m_profileOverlayMode == ProfileOverlayMode::HorizontalSharpnessProfile)
    {
        std::vector<double> sharpnessProfile(imageSizeX, 0.);

        for (long x = 0; x < imageSizeX; x++)
        {
            double verticalSum = 0.;
            double verticalDiffSum = 0.;

            for (long y = 1; y < imageSizeY - 1; y++)
            {
                verticalSum += grabImage[y - 1][x] + grabImage[y + 1][x];
                verticalDiffSum += fabs(grabImage[y - 1][x] - grabImage[y + 1][x]);
            }

            sharpnessProfile[x] = verticalSum ? (256 * m_sharpnessProfileScaling * verticalDiffSum / verticalSum) : 0;
        }

        for (long x = 0; x < imageSizeX; x++)
        {
            Ipvm::LineSeg32r xxxx = {float(x), float(0.f), float(x), float(sharpnessProfile[x])};
            m_imageView->ImageOverlayAdd(xxxx, RGB(255, 0, 255));
        }
    }

    m_imageView->ImageOverlayShow();

    return 1L;
}

LRESULT DlgVisionIllumSetup3D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    switch (item->GetID())
    {
        case PROPERTY_GRID_ITEM_EXPOSURE_INTENSITY_VALUE:
            switch (wparam)
            {
                case XTP_PGN_SELECTION_CHANGED:
                    if (auto* control = item->GetInplaceControls()->GetAt(0))
                    {
                        auto* slider = dynamic_cast<CXTPPropertyGridInplaceSlider*>(control);

                        if (slider)
                        {
                            slider->GetSliderCtrl()->SetPageSize(8);
                        }
                    }
                    break;
            }
            break;
    }

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        const long id = item->GetID();

        switch (id)
        {
            case PROPERTY_GRID_ITEM_EXPOSURE_HIGH_TIME:
            {
                double validFramePeriod_ms = 0.;
                FrameGrabber::GetInstance().set_exposure_time(
                    0, 0, m_visionMain.m_pSlitBeam3DCommon->m_illuminationTime_ms, validFramePeriod_ms);
            }
            break;

            case PROPERTY_GRID_ITEM_EXPOSURE_INTENSITY_VALUE:
            {
                FrameGrabber::GetInstance().live_off();
                SlitIlluminator::GetInstance().Download(
                    INSTRUCTION_CHANNEL0_INTENSITY, m_visionMain.m_pSlitBeam3DCommon->m_illuminationValue);
                FrameGrabber::GetInstance().live_on(
                    GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            }
            break;

            case PROPERTY_GRID_ITEM_CAMERA_VERTICAL_BINNING:
                FrameGrabber::GetInstance().live_off();
                FrameGrabber::GetInstance().set_live_image_size(
                    0, 0, m_visionMain.m_pSlitBeam3DCommon->m_cameraVerticalBinning, true);

                if (XTPPropertyGridItemCustomFloat<double>* scanDepth
                    = dynamic_cast<XTPPropertyGridItemCustomFloat<double>*>(
                        m_propertyGrid->FindItem(PROPERTY_GRID_ITEM_SCAN_DEPTH)))
                {
                    if (m_visionMain.m_pSlitBeam3DCommon->m_cameraVerticalBinning)
                    {
                        scanDepth->SetMinMaxDelta(500., 6000., 1.);
                    }
                    else
                    {
                        scanDepth->SetMinMaxDelta(500., 3000., 1.);
                    }
                }
                FrameGrabber::GetInstance().live_on(
                    GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                break;
            case PROPERTY_GRID_ITEM_VIEW_PROFILE_OVERLAY:
                if (m_profileOverlayMode == ProfileOverlayMode::IntensityProfile
                    || m_profileOverlayMode == ProfileOverlayMode::MaxIntensityProfile)
                {
                    m_imageView->ROIShow();
                }
                else
                {
                    m_imageView->ROIHide();
                }
                break;
        }
    }

    return 0;
}

void DlgVisionIllumSetup3D::OnDestroy()
{
    FrameGrabber::GetInstance().live_off();
    SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    FrameGrabber::GetInstance().set_live_image_size(
        0, 0, m_visionMain.m_pSlitBeam3DCommon->m_cameraVerticalBinning, false);

    CDialog::OnDestroy();
}