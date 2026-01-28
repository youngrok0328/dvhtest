//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FrameGrabber3D_Dummy.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>
#include <Ipvm/SbtCore/SbtCoreGpu.h>
#include <Ipvm/SbtCore/SbtCorePara.h>
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEFAULT_IMAGE_SIZE_X 4096
#define DEFAULT_IMAGE_SIZE_Y 200
#define DEFAULT_IMAGE_FULL_SIZE_Y 3072

//CPP_7_________________________________ Implementation body
//
FrameGrabber3D_Dummy::FrameGrabber3D_Dummy()
    : m_profileNumber(0)
    , m_pReliability(nullptr)
    , m_pIntensity(nullptr)
    , m_ppRawFirstBuff(nullptr)
    , m_nGrabbedImageCount(0)
{
    set_grabber_num(1);
    set_camera_icf(0, 0, 0);

    unsigned int width(0), height(0);

    width = DEFAULT_IMAGE_SIZE_X / PersonalConfig::getInstance().getImageSampling();
    height = DEFAULT_IMAGE_SIZE_Y / PersonalConfig::getInstance().getImageSampling();

    set_grab_image_width(0, 0, width);
    set_grab_image_height(0, 0, height);

    live_buffer_mono->Create(width, height);
}

FrameGrabber3D_Dummy::~FrameGrabber3D_Dummy(void)
{
}

//#define USE_GPU
//#define FULL_IMAGE_TEST
//#define FORWARD_SCAN
//#define NORMAL_IMAGE

bool FrameGrabber3D_Dummy::grab(const int grabber_idx, const int camera_idx, const int stitchIndex,
    Ipvm::Image32r& zmapImage, Ipvm::Image16u& vmapImage, float& height_range_min, float& height_range_max,
    Ipvm::AsyncProgress* progress)
{
    UNREFERENCED_PARAMETER(grabber_idx);
    UNREFERENCED_PARAMETER(camera_idx);
    UNREFERENCED_PARAMETER(height_range_max);
    UNREFERENCED_PARAMETER(height_range_min);
    //#ifdef USE_GPU
    //	static Ipvm::SbtCoreGpu core;
    //#else
    //	static Ipvm::SbtCore core();
    //#endif
    static std::vector<Ipvm::Image8u> images;

    progress->Post(_T("Load images"));

    static bool init = false;

    if (init == false)
    {
        long xxx = 0;

#ifdef FORWARD_SCAN
#ifdef FULL_IMAGE_TEST
        for (long idx = 12999; idx >= 0; idx--)
#else
        for (long idx = 6999; idx >= 6000; idx--)
#endif
#else
#ifdef FULL_IMAGE_TEST
        for (long idx = 0; idx < 13000; idx++)
#else
        for (long idx = 0; idx < 8664; idx++)
#endif
#endif
        {
            CString str;
            str.Format(_T("C:\\IntekPlus\\iPIS-500I\\Vision\\Image\\20210511_111259\\%05d.png"), idx);

            progress->Post(str);

#ifdef USE_GPU
            Ipvm::Image8u* image = dynamic_cast<Ipvm::Image8u*>(Ipvm::Image::MakeFrom(
                str, Ipvm::SbtCoreGpu::NonPagedMemory::Allocate, Ipvm::SbtCoreGpu::NonPagedMemory::Free));
#else
            Ipvm::Image8u* image = dynamic_cast<Ipvm::Image8u*>(Ipvm::ImageFile::CreateImage(str));
#endif

            if (image == nullptr)
            {
                break;
            }

            Ipvm::Image8u subImage(
                *image, Ipvm::Rect32s(image->GetSizeX() - 500, 0, image->GetSizeX(), image->GetSizeY()));

            str.Format(_T("C:\\IntekPlus\\iPIS-500I\\Vision\\Image\\%05d.png"), xxx++);

            Ipvm::ImageFile::SaveAsPng(subImage, str, 9);

#ifdef NORMAL_IMAGE
            Ipvm::ImageProcessing::MirrorByAxisX(Ipvm::Rect32s(*image), *image);
#endif

            images.push_back(Ipvm::Image8u(*image));

            Ipvm::ImageFile::Destory(image);
        }

        const auto& firstImage = images[0];
        const long imageWidth = firstImage.GetSizeX();
        const long imageHeight = firstImage.GetSizeY();
        m_profileNumber = (long)images.size();

#ifdef USE_GPU
        Ipvm::SbtCoreGpuParameters para;
#else
        Ipvm::SbtCorePara para;
#endif
        para.m_imageSizeX = imageWidth;
        para.m_imageSizeY = imageHeight;
        para.m_scanLength = m_profileNumber;
        para.m_imageBuffers = images.data();
        para.m_imageBufferCount = m_profileNumber;
        para.m_noiseThresholdPre_gv = 2.f;
        para.m_noiseThresholdFinal_gv = 1.f;
        para.m_beamThickness_um = 200.f;
        para.m_lateralResolution_um_per_px = SystemConfig::GetInstance().Get3DImageVerticalScale_px2um();
        para.m_scanStep_um = 10.f;
        para.m_illuminationAngle_deg = SystemConfig::GetInstance().m_slitbeamIlluminationAngle_deg;
        para.m_cameraAngle_deg = SystemConfig::GetInstance().m_slitbeamCameraAngle_deg;
        para.m_heightScaling = 1.f;
        para.m_useHeightValueQuantization = true;
        para.m_heightValueQuantum_um = 0.1f;

#ifdef NORMAL_IMAGE
        //para.m_verticalFlippedImage = false;
#else
        //para.m_verticalFlippedImage = true;
#endif

#ifdef FORWARD_SCAN
        para.m_forwardScan = true;
#else
        para.m_forwardScan = false;
#endif
        para.m_noiseValue = Ipvm::k_noiseValue32r;

#ifdef USE_GPU
//		para.m_cudaProfiling = false;
#endif

        //	auto coreResult = core.Setup(para);

        init = true;
    }

    progress->Post(_T("Calculate images"));

    Ipvm::Image32r m_zmapImage(images[0].GetSizeX(), (long)images.size(), true);
    Ipvm::Image32r m_vmapImage(images[0].GetSizeX(), (long)images.size(), true);

    //heightRangeMin = -0.5f * core.GetHeightRange_um();
    //	heightRangeMax = 0.5f * core.GetHeightRange_um();

    Ipvm::TimeCheck timeCheck;
    //	core.ProcessSync(nullptr, m_zmapImage.GetMem(), m_vmapImage.GetMem());

    //	core.ProcessAsync(m_zmapImage.GetMem(), m_vmapImage.GetMem());
    //	for (long idx = 0; idx < m_profileNumber; idx++)
    //	{
    //		Ipvm::SbtCore::GrabCallback(idx + 1, &core);
    ////		Ipvm::SbtCoreGpu::GrabCallback(idx + 1, &core);
    //	}
    //	core.Wait(60000);
    float coreCalcTime_ms = CAST_FLOAT(timeCheck.Elapsed_ms());

    const int scanWidth = m_zmapImage.GetSizeX();
    const int scanLength = m_zmapImage.GetSizeY();

    Ipvm::Rect32s imageRect(0, scanWidth * stitchIndex, scanLength, scanWidth * (stitchIndex + 1));

    Ipvm::Image32r childResultZmapImage(zmapImage, imageRect);
    Ipvm::Image16u childResultVmapImage(vmapImage, imageRect);

    timeCheck.Reset();
#ifdef USE_GPU
    Ipvm::ImageProcessingGpu::RotateCw90(m_zmapImage, childResultZmapImage);
#else
    Ipvm::ImageProcessing::RotateCw90(m_zmapImage, childResultZmapImage);
#endif
    float rotation1Time_ms = CAST_FLOAT(timeCheck.Elapsed_ms());

    timeCheck.Reset();
#ifdef USE_GPU
    Ipvm::ImageProcessingGpu::RotateCw90(m_vmapImage, 255.f, childResultVmapImage);
#else
    Ipvm::ImageProcessing::RotateCw90(m_vmapImage, 255.f, childResultVmapImage);
#endif
    float rotation2Time_ms = CAST_FLOAT(timeCheck.Elapsed_ms());

    //for (auto *image : images)
    //{
    //	delete image;
    //}

    CString str;
    str.Format(_T("%.2f ms, %.2f ms, %.2f ms"), coreCalcTime_ms, rotation1Time_ms, rotation2Time_ms);
    ::AfxMessageBox(str);

    return true;
}

void FrameGrabber3D_Dummy::set_live_image_size(
    const int& grabber_idx, const int& camera_idx, const bool& /*use_binning*/, const bool& is_full_size)
{
    const unsigned int& width = get_grab_image_width(grabber_idx, camera_idx);
    const unsigned int& hegiht = get_grab_image_height(grabber_idx, camera_idx);
    live_buffer_mono->Create(width, is_full_size ? DEFAULT_IMAGE_FULL_SIZE_Y : hegiht);
}

bool FrameGrabber3D_Dummy::LiveCallback(const int& /*graaber_id*/, const int& /*camera_id*/)
{
    BYTE rnd = rand() % 128 + 64;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(*live_buffer_mono), rnd, *live_buffer_mono);

    return true;
}

bool FrameGrabber3D_Dummy::SetTriggerMode(int nMode)
{
    UNREFERENCED_PARAMETER(nMode);

    return true;
}

bool FrameGrabber3D_Dummy::OpenCameraComm(const int& /*camera_id*/)
{
    return true;
}

bool FrameGrabber3D_Dummy::CloseCameraComm()
{
    return true;
}

bool FrameGrabber3D_Dummy::SetDigitalGainValue(float i_fInputGain, BOOL bInline)
{
    UNREFERENCED_PARAMETER(i_fInputGain);
    UNREFERENCED_PARAMETER(bInline);

    return true;
}

float FrameGrabber3D_Dummy::GetCurrentCameraGain()
{
    return 0.f;
}