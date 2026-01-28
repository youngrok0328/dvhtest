#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ArchiveAllType;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ ImageSaveOptionData
{
public:
    ImageSaveOptionData();
    ~ImageSaveOptionData();
    void Init();
    void Serialize(ArchiveAllType& ar);

    long m_nVisionType;
    BOOL m_bisSaveReviewImage;
    BOOL m_bisSaveCroppingImage;

    //RawImage

public:
    std::vector<long> m_vecRawImageSaveOption;

    //ReviewImage

public:
    BOOL m_bReviewImageOption_Pass;
    BOOL m_bReviewImageOption_Reject;
    BOOL m_bReviewImageOption_Marginal;
    long m_nReviewImageROI;
    BOOL m_bReviewImageCombined;
    long m_nReviewImageQuality;
    long m_nReviewImageMargin;

    //CroppingImage

public:
    long m_nCroppingImageROI;
    long m_nCroppingImageSaveOption;
    long m_nCroppingImageFrame_Ball;
    long m_nCroppingImageFrame_Land;
    long m_nCroppingImageFrame_Comp;
    long m_nCroppingImageFrame_Surface;
    long m_nCroppingImageQuality;
};
