//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionImageLotInsp.h"

//CPP_2_________________________________ This project's headers
#include "VisionImageLot.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
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
VisionImageLotInsp::VisionImageLotInsp()
    : m_paneIndexForCalculationImage(-1)
    , m_heightRangeMin(-1000.f)
    , m_heightRangeMax(1000.f)
{
}

VisionImageLotInsp::~VisionImageLotInsp()
{
}

void VisionImageLotInsp::Set(const VisionImageLot& imageLot, const enSideVisionModule i_eSideVisionModule)
{
    BOOL bColor = FALSE;

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        bColor = TRUE;
    }

    long imageSizeX = imageLot.GetImageSizeX(i_eSideVisionModule);
    long imageSizeY = imageLot.GetImageSizeY(i_eSideVisionModule);

    if (!bColor)
    {
        m_vecImages[i_eSideVisionModule].resize(imageLot.GetImageFrameCount(i_eSideVisionModule));

        for (long idx = 0; idx < imageLot.GetImageFrameCount(i_eSideVisionModule); idx++)
        {
            m_vecImages[i_eSideVisionModule][idx].Create(imageSizeX, imageSizeY);
            Ipvm::ImageProcessing::Copy(imageLot.GetImageFrame(idx, i_eSideVisionModule),
                Ipvm::Rect32s(imageLot.GetImageFrame(idx, i_eSideVisionModule)), m_vecImages[i_eSideVisionModule][idx]);
        }
    }
    else
    {
        m_vecImages[i_eSideVisionModule].resize(imageLot.GetImageFrameCount(i_eSideVisionModule));
        m_vecColorImages.resize(imageLot.GetImageFrameCount());

        for (long idx = 0; idx < imageLot.GetImageFrameCount(i_eSideVisionModule); idx++)
        {
            m_vecImages[i_eSideVisionModule][idx].Create(imageSizeX, imageSizeY);
            m_vecColorImages[idx].Create(imageSizeX, imageSizeY);
            Ipvm::ImageProcessing::Copy(imageLot.GetImageFrame(idx, i_eSideVisionModule),
                Ipvm::Rect32s(imageLot.GetImageFrame(idx, i_eSideVisionModule)), m_vecImages[i_eSideVisionModule][idx]);
            Ipvm::ImageProcessing::Copy(imageLot.GetColorImageFrame(idx),
                Ipvm::Rect32s(imageLot.GetColorImageFrame(idx)), m_vecColorImages[idx]);
        }
    }

    if (imageLot.IsExistZMap(i_eSideVisionModule))
    {
        m_heightRangeMin = imageLot.GetInfo(i_eSideVisionModule).m_heightRangeMin;
        m_heightRangeMax = imageLot.GetInfo(i_eSideVisionModule).m_heightRangeMax;

        m_zmapImage.Create(imageSizeX, imageSizeY);
        m_vmapImage.Create(imageSizeX, imageSizeY);

        Ipvm::ImageProcessing::Copy(imageLot.GetZmapImage(i_eSideVisionModule),
            Ipvm::Rect32s(imageLot.GetZmapImage(i_eSideVisionModule)), m_zmapImage);
        Ipvm::ImageProcessing::Copy(imageLot.GetVmapImage(i_eSideVisionModule),
            Ipvm::Rect32s(imageLot.GetVmapImage(i_eSideVisionModule)), m_vmapImage);
    }

    m_paneIndexForCalculationImage = -1;
}

void VisionImageLotInsp::CopyFrom(const VisionImageLotInsp& object, const enSideVisionModule i_eSideVisionModule)
{
    BOOL bColor = FALSE;

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        bColor = TRUE;
    }

    long imageSizeX = object.GetImageSizeX(i_eSideVisionModule);
    long imageSizeY = object.GetImageSizeY(i_eSideVisionModule);

    if (!bColor)
    {
        m_vecImages[i_eSideVisionModule].resize(object.GetImageFrameCount());

        for (long idx = 0; idx < object.GetImageFrameCount(i_eSideVisionModule); idx++)
        {
            m_vecImages[i_eSideVisionModule][idx].Create(imageSizeX, imageSizeY);
            Ipvm::ImageProcessing::Copy(object.m_vecImages[i_eSideVisionModule][idx],
                Ipvm::Rect32s(object.m_vecImages[i_eSideVisionModule][idx]), m_vecImages[i_eSideVisionModule][idx]);
        }
    }
    else
    {
        m_vecImages[i_eSideVisionModule].resize(object.GetImageFrameCount(i_eSideVisionModule));
        m_vecColorImages.resize(object.GetColorImageFrameCount());

        for (long idx = 0; idx < object.GetColorImageFrameCount(); idx++)
        {
            m_vecImages[i_eSideVisionModule][idx].Create(imageSizeX, imageSizeY);
            m_vecColorImages[idx].Create(imageSizeX, imageSizeY);
            Ipvm::ImageProcessing::Copy(object.m_vecImages[i_eSideVisionModule][idx],
                Ipvm::Rect32s(object.m_vecImages[i_eSideVisionModule][idx]), m_vecImages[i_eSideVisionModule][idx]);
            Ipvm::ImageProcessing::Copy(
                object.m_vecColorImages[idx], Ipvm::Rect32s(object.m_vecColorImages[idx]), m_vecColorImages[idx]);
        }
    }

    if (m_zmapImage.GetSizeX() > 0)
    {
        m_heightRangeMin = object.m_heightRangeMin;
        m_heightRangeMax = object.m_heightRangeMax;

        m_zmapImage.Create(imageSizeX, imageSizeY);
        m_vmapImage.Create(imageSizeX, imageSizeY);

        Ipvm::ImageProcessing::Copy(object.m_zmapImage, Ipvm::Rect32s(object.m_zmapImage), m_zmapImage);
        Ipvm::ImageProcessing::Copy(object.m_vmapImage, Ipvm::Rect32s(object.m_vmapImage), m_vmapImage);
    }

    m_paneIndexForCalculationImage = object.m_paneIndexForCalculationImage;
}

const Ipvm::Image8u* VisionImageLotInsp::GetSafeImagePtr(long index, const enSideVisionModule i_eSideVisionModule) const
{
    if (index < 0)
        return nullptr;
    if (index >= long(m_vecImages[i_eSideVisionModule].size()))
        return nullptr;

    return &m_vecImages[i_eSideVisionModule][index];
}

Ipvm::Rect32s VisionImageLotInsp::GetImageRect(const enSideVisionModule i_eSideVisionModule) const
{
    return Ipvm::Rect32s(0, 0, GetImageSizeX(i_eSideVisionModule), GetImageSizeY(i_eSideVisionModule));
}

int VisionImageLotInsp::GetImageFrameCount(const enSideVisionModule i_eSideVisionModule) const
{
    return (long)m_vecImages[i_eSideVisionModule].size();
}

int VisionImageLotInsp::GetImageSizeX(const enSideVisionModule i_eSideVisionModule) const
{
    if (m_vecImages[i_eSideVisionModule].size())
    {
        return m_vecImages[i_eSideVisionModule][0].GetSizeX();
    }

    return 0;
}

int VisionImageLotInsp::GetImageSizeY(const enSideVisionModule i_eSideVisionModule) const
{
    if (m_vecImages[i_eSideVisionModule].size())
    {
        return m_vecImages[i_eSideVisionModule][0].GetSizeY();
    }

    return 0;
}

const Ipvm::Image8u3* VisionImageLotInsp::GetSafeColorImagePtr(long index) const
{
    if (index < 0)
        return nullptr;
    if (index >= long(m_vecColorImages.size()))
        return nullptr;

    return &m_vecColorImages[index];
}

int VisionImageLotInsp::GetColorImageFrameCount() const
{
    return (long)m_vecColorImages.size();
}

int VisionImageLotInsp::GetColorImageSizeX() const
{
    if (m_vecColorImages.size())
    {
        return m_vecColorImages[0].GetSizeX();
    }

    return 0;
}

int VisionImageLotInsp::GetColorImageSizeY() const
{
    if (m_vecColorImages.size())
    {
        return m_vecColorImages[0].GetSizeY();
    }

    return 0;
}