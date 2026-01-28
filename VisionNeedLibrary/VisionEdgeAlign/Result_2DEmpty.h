#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
namespace VisionEdgeAlign
{
class __VISION_EDGE_ALIGN_CLASS Result_2DEmpty
{
public:
    Result_2DEmpty();
    ~Result_2DEmpty();

    void SetImage(const Ipvm::Image8u& image, const Ipvm::Image8u& imageSampling);
    void SetImage(const Ipvm::Image8u& imageSampling, float samplingX, float samplingY);
    void Reset();
    void FreeMemoryForCalculation();

    void setBlobRois(const Ipvm::BlobInfo* blobInfos, long blobCount);
    void getBlobRois(std::vector<Ipvm::Rect32r>& o_blobRois);
    long getBlobCount() const;

    const Ipvm::Point32r2& getSamplingRate();
    const Ipvm::Image8u& getImage(bool sampling);

    bool m_inspectioned;
    bool m_success;

private:
    Ipvm::Image8u m_image;
    Ipvm::Image8u m_imageSampling;
    Ipvm::Point32r2 m_sampling;

    std::vector<Ipvm::Rect32s> m_vecrtBlobROI;
};
} // namespace VisionEdgeAlign
