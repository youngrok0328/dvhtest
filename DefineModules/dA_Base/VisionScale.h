#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class _DA_BASE_MODULE_API VisionScale
{
public:
    VisionScale();
    ~VisionScale();

    void setScale(const float px2um_x, const float px2um_y, const float px2um_z);
    void setScaleX(const float px2um);
    void setScaleY(const float px2um);
    void setScaleZ(const float px2um);

    float convert_umToPixelX(const float value) const;
    float convert_umToPixelY(const float value) const;
    float convert_mmToPixelX(const float value) const;
    float convert_mmToPixelY(const float value) const;
    float convert_milToPixelX(const float value) const;
    float convert_milToPixelY(const float value) const;
    double convert_umToPixelX(const double value) const;
    double convert_umToPixelY(const double value) const;

    float convert_pixelToUmX(const float value) const;
    float convert_pixelToUmY(const float value) const;
    double convert_pixelToUmX(const double value) const;
    double convert_pixelToUmY(const double value) const;

    Ipvm::Rect32s convert_umToPixel(const Ipvm::Rect32r& roi) const;
    Ipvm::Point32s2 convert_umToPixel(const Ipvm::Point32r2& roi) const;

    Ipvm::Rect32s convert_mmToPixel(const Ipvm::Rect32r& roi) const;
    Ipvm::Point32s2 convert_mmToPixel(const Ipvm::Point32r2& roi) const;

    Ipvm::Rect32s convert_BCUToPixel(const Ipvm::Rect32r& roi_BCU, const Ipvm::Point32r2& imageCenter,
        float samplingX = 1.f, float samplingY = 1.f) const;
    Ipvm::Point32s2 convert_BCUToPixel(const Ipvm::Point32r2& roi_BCU, const Ipvm::Point32r2& imageCenter,
        float samplingX = 1.f, float samplingY = 1.f) const;
    std::vector<Ipvm::Point32s2> convert_BCUToPixel(
        const std::vector<Ipvm::Point32r2>& arrayData_BCU, const Ipvm::Point32r2& imageCenter) const;

    Ipvm::Rect32r convert_pixelToUm(const Ipvm::Rect32s& roi) const;
    Ipvm::Point32r2 convert_pixelToUm(const Ipvm::Point32s2& roi) const;
    Ipvm::Point32r2 convert_pixelToUm(const Ipvm::Point32r2& roi) const;

    Ipvm::Rect32r convert_PixelToBCU(const Ipvm::Rect32s& roi, const Ipvm::Point32r2& imageCenter) const;
    Ipvm::Point32r2 convert_PixelToBCU(const Ipvm::Point32s2& roi, const Ipvm::Point32r2& imageCenter) const;
    std::vector<Ipvm::Point32r2> convert_PixelToBCU(
        const std::vector<Ipvm::Point32s2>& arrayData, const Ipvm::Point32r2& imageCenter) const;

    // X, Y Scale의 평균
    float umToPixelXY() const;
    float mmToPixelXY() const;
    float pixelToUmXY() const;

    const Ipvm::Point32r2& umToPixel() const;
    const Ipvm::Point32r2& mmToPixel() const;
    const Ipvm::Point32r2& milToPixel() const;
    const Ipvm::Point32r2& pixelToUm() const;
    const Ipvm::Point32r2& pixelToMm() const;
    const Ipvm::Point32r2& pixelToMil() const;

    float pixelToUmZ() const;

private:
    float m_px2um_z;
    Ipvm::Point32r2 m_px2um;
    Ipvm::Point32r2 m_px2mm;
    Ipvm::Point32r2 m_px2mil;
    Ipvm::Point32r2 m_um2px;
    Ipvm::Point32r2 m_mm2px;
    Ipvm::Point32r2 m_mil2px;
};
