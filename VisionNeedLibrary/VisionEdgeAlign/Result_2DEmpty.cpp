//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Result_2DEmpty.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/BlobInfo.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
using namespace VisionEdgeAlign;

Result_2DEmpty::Result_2DEmpty()
    : m_inspectioned(false)
    , m_success(false)
{
    m_sampling = Ipvm::Point32r2(1.f, 1.f);
}

Result_2DEmpty::~Result_2DEmpty()
{
}

void Result_2DEmpty::SetImage(const Ipvm::Image8u& image, const Ipvm::Image8u& imageSampling)
{
    m_image = image;
    m_imageSampling = imageSampling;

    m_sampling.m_x = float(m_image.GetSizeX()) / m_imageSampling.GetSizeX();
    m_sampling.m_y = float(m_image.GetSizeY()) / m_imageSampling.GetSizeY();
}

void Result_2DEmpty::SetImage(const Ipvm::Image8u& imageSampling, float samplingX, float samplingY)
{
    m_imageSampling = imageSampling;

    m_sampling.m_x = samplingX;
    m_sampling.m_y = samplingY;
}

void Result_2DEmpty::Reset()
{
    m_vecrtBlobROI.clear();
    m_inspectioned = false;
    m_success = false;
}

void Result_2DEmpty::FreeMemoryForCalculation()
{
    m_image.Free();
    m_sampling.m_x = 1.f;
    m_sampling.m_y = 1.f;
}

void Result_2DEmpty::setBlobRois(const Ipvm::BlobInfo* blobInfos, long blobCount)
{
    m_vecrtBlobROI.clear();
    m_vecrtBlobROI.resize(blobCount);

    for (long n = 0; n < blobCount; n++)
    {
        m_vecrtBlobROI[n] = blobInfos[n].m_roi;
    }
}

void Result_2DEmpty::getBlobRois(std::vector<Ipvm::Rect32r>& o_blobRois)
{
    o_blobRois.resize(m_vecrtBlobROI.size());

    for (long index = 0; index < long(m_vecrtBlobROI.size()); index++)
    {
        o_blobRois[index] = m_vecrtBlobROI[index] * m_sampling;
    }
}

long Result_2DEmpty::getBlobCount() const
{
    return long(m_vecrtBlobROI.size());
}
const Ipvm::Point32r2& Result_2DEmpty::getSamplingRate()
{
    return m_sampling;
}

const Ipvm::Image8u& Result_2DEmpty::getImage(bool sampling)
{
    if (sampling)
    {
        return m_imageSampling;
    }

    return m_image;
}
