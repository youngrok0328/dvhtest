#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class CDlgVisionProcessingSpec;
class ImageViewEx;

//HDR_6_________________________________ Header body
//
class CAutoSetting
{
public:
    CAutoSetting(CPackageSpec& packageSpec, CDlgVisionProcessingSpec* pVPSpec, ImageViewEx* imageView);
    virtual ~CAutoSetting(void);

public:
    CPackageSpec& m_packageSpec;
    CDlgVisionProcessingSpec* m_pVPSpec;
    ImageViewEx* m_imageView;

public:
    std::vector<Ipvm::Rect32s> GetBlobObjectROI(
        const Ipvm::Image8u& image, Ipvm::Rect32s rtBlobROI, long nThresholdValue);
    void ShowThresholdImage(
        const Ipvm::Image8u& image, Ipvm::Rect32s rtBodyROI, Ipvm::Rect32s rtBlobROI, long nThresoldValue);
};
