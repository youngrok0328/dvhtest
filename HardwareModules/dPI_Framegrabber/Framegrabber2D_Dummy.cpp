//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Framegrabber2D_Dummy.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Framegrabber2D_Dummy::Framegrabber2D_Dummy(void)
{
    set_grabber_num(1);
    set_camera_icf(0, 0, 0);

    static const bool isNGRV_InspectionMode = SystemConfig::GetInstance().IsNGRVInspectionMode(); //kircheis_SWIR

    unsigned int width(0), height(0);

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP)
    {
        if (isNGRV_InspectionMode == true) //kircheis_SWIR
        {
            width = 9344 / PersonalConfig::getInstance().getImageSampling();
            height = 7000 / PersonalConfig::getInstance().getImageSampling();
        }
        else
        {
            if (SystemConfig::GetInstance().Get2DVisionCameraType() == CAMERATYPE_DALSA_GENIE_NANO_CXP_67M)
            {
                width = 8192 / PersonalConfig::getInstance().getImageSampling();
                height = 8192 / PersonalConfig::getInstance().getImageSampling();
            }
            else
            {
                width = 5120 / PersonalConfig::getInstance().getImageSampling();
                height = 5120 / PersonalConfig::getInstance().getImageSampling();
            }
        }
    }
    else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        width = 9344 / PersonalConfig::getInstance().getImageSampling();
        height = 7000 / PersonalConfig::getInstance().getImageSampling();
    }
    else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
    {
        width = 8192 / PersonalConfig::getInstance().getImageSampling();
        height = 2000 / PersonalConfig::getInstance().getImageSampling();
    }
    else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
    {
        width = 2560 / PersonalConfig::getInstance().getImageSampling();
        height = 2048 / PersonalConfig::getInstance().getImageSampling();
    }
    else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_TR)
    {
        width = 2040 / PersonalConfig::getInstance().getImageSampling();
        height = 2040 / PersonalConfig::getInstance().getImageSampling();
    }

    set_grab_image_width(0, 0, width);
    set_grab_image_height(0, 0, height);

    live_buffer_mono->Create(width, height);
}

Framegrabber2D_Dummy::~Framegrabber2D_Dummy()
{
}

bool Framegrabber2D_Dummy::LiveCallback(const int& /*graaber_id*/, const int& /*camera_id*/)
{
    BYTE rnd = rand() % 128 + 64;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(*live_buffer_mono), rnd, *live_buffer_mono);

    return true;
}

bool Framegrabber2D_Dummy::SetTriggerMode(int nMode)
{
    UNREFERENCED_PARAMETER(nMode);

    return true;
}