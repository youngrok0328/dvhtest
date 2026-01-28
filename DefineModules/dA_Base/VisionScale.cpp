//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionScale.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionScale::VisionScale()
{
    setScale(0.f, 0.f, 0.f);
}

VisionScale::~VisionScale()
{
}

void VisionScale::setScale(const float px2um_x, const float px2um_y, const float px2um_z)
{
    setScaleX(px2um_x);
    setScaleY(px2um_y);
    setScaleZ(px2um_z);
}

void VisionScale::setScaleX(const float px2um)
{
    m_px2um.m_x = px2um;
    m_px2mm.m_x = px2um / 1000.f;
    m_px2mil.m_x = px2um / 25.4f;

    if (px2um == 0.f)
    {
        m_um2px.m_x = m_mm2px.m_x = m_mil2px.m_x = 0.f;
    }
    else
    {
        m_um2px.m_x = 1 / px2um;
        m_mm2px.m_x = 1000.f / px2um;
        m_mil2px.m_x = 25.4f / px2um;
    }
}

void VisionScale::setScaleY(const float px2um)
{
    m_px2um.m_y = px2um;
    m_px2mm.m_y = px2um / 1000.f;
    m_px2mil.m_y = px2um / 25.4f;

    if (px2um == 0.f)
    {
        m_um2px.m_y = m_mm2px.m_y = m_mil2px.m_y = 0.f;
    }
    else
    {
        m_um2px.m_y = 1 / px2um;
        m_mm2px.m_y = 1000.f / px2um;
        m_mil2px.m_y = 25.4f / px2um;
    }
}

void VisionScale::setScaleZ(const float px2um)
{
    m_px2um_z = px2um;
}

float VisionScale::convert_umToPixelX(const float value) const
{
    return value * m_um2px.m_x;
}

float VisionScale::convert_umToPixelY(const float value) const
{
    return value * m_um2px.m_y;
}

float VisionScale::convert_mmToPixelX(const float value) const
{
    return value * m_mm2px.m_x;
}

float VisionScale::convert_mmToPixelY(const float value) const
{
    return value * m_mm2px.m_y;
}

float VisionScale::convert_milToPixelX(const float value) const
{
    return value * m_mil2px.m_x;
}

float VisionScale::convert_milToPixelY(const float value) const
{
    return value * m_mil2px.m_y;
}

double VisionScale::convert_umToPixelX(const double value) const
{
    return value * m_um2px.m_x;
}

double VisionScale::convert_umToPixelY(const double value) const
{
    return value * m_um2px.m_y;
}

float VisionScale::convert_pixelToUmX(const float value) const
{
    return value * m_px2um.m_x;
}

float VisionScale::convert_pixelToUmY(const float value) const
{
    return value * m_px2um.m_y;
}

double VisionScale::convert_pixelToUmX(const double value) const
{
    return value * m_px2um.m_x;
}

double VisionScale::convert_pixelToUmY(const double value) const
{
    return value * m_px2um.m_y;
}

Ipvm::Rect32s VisionScale::convert_umToPixel(const Ipvm::Rect32r& roi) const
{
    Ipvm::Rect32s newRoi;
    newRoi.m_left = long(roi.m_left * m_um2px.m_x + 0.5f);
    newRoi.m_top = long(roi.m_top * m_um2px.m_y + 0.5f);
    newRoi.m_right = long(roi.m_right * m_um2px.m_x + 0.5f);
    newRoi.m_bottom = long(roi.m_bottom * m_um2px.m_y + 0.5f);

    return newRoi;
}

Ipvm::Point32s2 VisionScale::convert_umToPixel(const Ipvm::Point32r2& roi) const
{
    Ipvm::Point32s2 newRoi;
    newRoi.m_x = long(roi.m_x * m_um2px.m_x + 0.5f);
    newRoi.m_y = long(roi.m_y * m_um2px.m_y + 0.5f);

    return newRoi;
}

Ipvm::Rect32s VisionScale::convert_mmToPixel(const Ipvm::Rect32r& roi) const
{
    Ipvm::Rect32s newRoi;
    newRoi.m_left = long(roi.m_left * m_mm2px.m_x + 0.5f);
    newRoi.m_top = long(roi.m_top * m_mm2px.m_y + 0.5f);
    newRoi.m_right = long(roi.m_right * m_mm2px.m_x + 0.5f);
    newRoi.m_bottom = long(roi.m_bottom * m_mm2px.m_y + 0.5f);

    return newRoi;
}

Ipvm::Point32s2 VisionScale::convert_mmToPixel(const Ipvm::Point32r2& roi) const
{
    Ipvm::Point32s2 newRoi;
    newRoi.m_x = long(roi.m_x * m_mm2px.m_x + 0.5f);
    newRoi.m_y = long(roi.m_y * m_mm2px.m_y + 0.5f);

    return newRoi;
}

Ipvm::Rect32s VisionScale::convert_BCUToPixel(
    const Ipvm::Rect32r& roi_BCU, const Ipvm::Point32r2& imageCenter, float samplingX, float samplingY) const
{
    Ipvm::Rect32s newRoi;
    newRoi.m_left = long(roi_BCU.m_left * m_um2px.m_x / samplingX + imageCenter.m_x + 0.5f);
    newRoi.m_top = long(roi_BCU.m_top * m_um2px.m_y / samplingY + imageCenter.m_y + 0.5f);
    newRoi.m_right = long(roi_BCU.m_right * m_um2px.m_x / samplingX + imageCenter.m_x + 0.5f);
    newRoi.m_bottom = long(roi_BCU.m_bottom * m_um2px.m_y / samplingY + imageCenter.m_y + 0.5f);

    return newRoi;
}

Ipvm::Point32s2 VisionScale::convert_BCUToPixel(
    const Ipvm::Point32r2& roi_BCU, const Ipvm::Point32r2& imageCenter, float samplingX, float samplingY) const
{
    Ipvm::Point32s2 newRoi;
    newRoi.m_x = long(roi_BCU.m_x * m_um2px.m_x / samplingX + imageCenter.m_x + 0.5f);
    newRoi.m_y = long(roi_BCU.m_y * m_um2px.m_y / samplingY + imageCenter.m_y + 0.5f);

    return newRoi;
}

std::vector<Ipvm::Point32s2> VisionScale::convert_BCUToPixel(
    const std::vector<Ipvm::Point32r2>& arrayData_BCU, const Ipvm::Point32r2& imageCenter) const
{
    std::vector<Ipvm::Point32s2> newArray;
    newArray.resize(arrayData_BCU.size());

    for (long index = 0; index < long(arrayData_BCU.size()); index++)
    {
        newArray[index].m_x = long(arrayData_BCU[index].m_x * m_um2px.m_x + imageCenter.m_x + 0.5f);
        newArray[index].m_y = long(arrayData_BCU[index].m_y * m_um2px.m_y + imageCenter.m_y + 0.5f);
    }

    return newArray;
}

Ipvm::Rect32r VisionScale::convert_pixelToUm(const Ipvm::Rect32s& roi) const
{
    Ipvm::Rect32r newRoi;
    newRoi.m_left = roi.m_left * m_px2um.m_x;
    newRoi.m_top = roi.m_top * m_px2um.m_y;
    newRoi.m_right = roi.m_right * m_px2um.m_x;
    newRoi.m_bottom = roi.m_bottom * m_px2um.m_y;

    return newRoi;
}

Ipvm::Point32r2 VisionScale::convert_pixelToUm(const Ipvm::Point32s2& roi) const
{
    Ipvm::Point32r2 newRoi;
    newRoi.m_x = roi.m_x * m_px2um.m_x;
    newRoi.m_y = roi.m_y * m_px2um.m_y;

    return newRoi;
}

Ipvm::Point32r2 VisionScale::convert_pixelToUm(const Ipvm::Point32r2& roi) const
{
    Ipvm::Point32r2 newRoi;
    newRoi.m_x = roi.m_x * m_px2um.m_x;
    newRoi.m_y = roi.m_y * m_px2um.m_y;

    return newRoi;
}

Ipvm::Rect32r VisionScale::convert_PixelToBCU(const Ipvm::Rect32s& roi, const Ipvm::Point32r2& imageCenter) const
{
    Ipvm::Rect32r newRoi;
    newRoi.m_left = (roi.m_left - imageCenter.m_x) * m_px2um.m_x;
    newRoi.m_top = (roi.m_top - imageCenter.m_y) * m_px2um.m_y;
    newRoi.m_right = (roi.m_right - imageCenter.m_x) * m_px2um.m_x;
    newRoi.m_bottom = (roi.m_bottom - imageCenter.m_y) * m_px2um.m_y;

    return newRoi;
}

Ipvm::Point32r2 VisionScale::convert_PixelToBCU(const Ipvm::Point32s2& roi, const Ipvm::Point32r2& imageCenter) const
{
    Ipvm::Point32r2 newRoi;
    newRoi.m_x = (roi.m_x - imageCenter.m_x) * m_px2um.m_x;
    newRoi.m_y = (roi.m_y - imageCenter.m_y) * m_px2um.m_y;

    return newRoi;
}

std::vector<Ipvm::Point32r2> VisionScale::convert_PixelToBCU(
    const std::vector<Ipvm::Point32s2>& arrayData, const Ipvm::Point32r2& imageCenter) const
{
    std::vector<Ipvm::Point32r2> newArray;
    newArray.resize(arrayData.size());

    for (long index = 0; index < long(arrayData.size()); index++)
    {
        newArray[index].m_x = (arrayData[index].m_x - imageCenter.m_x) * m_px2um.m_x;
        newArray[index].m_y = (arrayData[index].m_y - imageCenter.m_y) * m_px2um.m_y;
    }

    return newArray;
}

float VisionScale::umToPixelXY() const
{
    return (m_um2px.m_x + m_um2px.m_y) * 0.5f;
}

float VisionScale::mmToPixelXY() const
{
    return (m_mm2px.m_x + m_mm2px.m_y) * 0.5f;
}

float VisionScale::pixelToUmXY() const
{
    return (m_px2um.m_x + m_px2um.m_y) * 0.5f;
}

const Ipvm::Point32r2& VisionScale::umToPixel() const
{
    return m_um2px;
}

const Ipvm::Point32r2& VisionScale::mmToPixel() const
{
    return m_mm2px;
}

const Ipvm::Point32r2& VisionScale::milToPixel() const
{
    return m_mil2px;
}

const Ipvm::Point32r2& VisionScale::pixelToUm() const
{
    return m_px2um;
}

const Ipvm::Point32r2& VisionScale::pixelToMm() const
{
    return m_px2mm;
}

const Ipvm::Point32r2& VisionScale::pixelToMil() const
{
    return m_px2mil;
}

float VisionScale::pixelToUmZ() const
{
    return m_px2um_z;
}
