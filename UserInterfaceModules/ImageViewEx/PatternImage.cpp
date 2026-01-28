//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PatternImage.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point8u3.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
class PatternValue
{
public:
    PatternValue()
    {
        m_values[0] = {255, 0, 0}; // Dark 는 Blue로 표시
        m_values[254] = {0, 255, 0}; // Background 는 Green로 표시
        m_values[255] = {0, 0, 255}; // Bright 는 Red로 표시

        uint8_t unsignedIntensity = 0;
        for (long intensity = 1; intensity < 254; intensity++)
        {
            unsignedIntensity = static_cast<uint8_t>(intensity);
            m_values[intensity].m_x = unsignedIntensity;
            m_values[intensity].m_y = unsignedIntensity;
            m_values[intensity].m_z = unsignedIntensity;
        }
    }

    Ipvm::Point8u3 m_values[256];
};

PatternValue g_pattern;

PatternImage::PatternImage()
{
}

PatternImage::~PatternImage()
{
}

void PatternImage::setImage(const Ipvm::Image8u& image)
{
    Create(image.GetSizeX(), image.GetSizeY());

    Ipvm::ImageProcessing::ConvertGrayToBGR(image, Ipvm::Rect32s(image), *this);
}

bool PatternImage::setThresholdImage(const Ipvm::Image8u& image)
{
    if (Create(image.GetSizeX(), image.GetSizeY()) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return Ipvm::Status::e_ok
        == Ipvm::ImageProcessing::ConvertGrayToBGR(image, Ipvm::Rect32s(image), g_pattern.m_values, *this);
}

bool PatternImage::addPattern(const Ipvm::Image8u& mask, COLORREF rgb)
{
    Ipvm::Point8u3 rgbValue = {GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)};

    Ipvm::Rect32s roi(*this);

    if ((Ipvm::Rect32s(mask) & roi) != roi)
    {
        ASSERT(!_T("??"));
        return false;
    }

    for (long y = 0; y < mask.GetSizeY(); y++)
    {
        auto* srcImageY = mask.GetMem(0, y);
        auto* dstImageY = GetMem(0, y);

        for (long x = 0; x < mask.GetSizeX(); x++)
        {
            if (!srcImageY[x])
                continue;

            dstImageY[x] = rgbValue;
        }
    }

    return true;
}

bool PatternImage::addPattern(const Ipvm::Image32s& label, long labelindex, const Ipvm::Rect32s& roi, COLORREF rgb)
{
    Ipvm::Point8u3 rgbValue = {GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)};

    if ((Ipvm::Rect32s(*this) & roi) != roi)
    {
        ASSERT(!_T("??"));
        return false;
    }

    if ((Ipvm::Rect32s(label) & roi) != roi)
    {
        ASSERT(!_T("??"));
        return false;
    }

    for (long y = roi.m_top; y < roi.m_bottom; y++)
    {
        auto* srcImageY = label.GetMem(0, y);
        auto* dstImageY = GetMem(0, y);

        for (long x = roi.m_left; x < roi.m_right; x++)
        {
            if (srcImageY[x] != labelindex)
                continue;

            dstImageY[x] = rgbValue;
        }
    }

    return true;
}
