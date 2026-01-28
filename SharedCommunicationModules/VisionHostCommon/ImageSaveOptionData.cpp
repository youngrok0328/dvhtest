//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageSaveOptionData.h"

//CPP_2_________________________________ This project's headers
#include "ArchiveAllType.h"
#include "ImageSaveOption.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ImageSaveOptionData::ImageSaveOptionData()
{
    Init();
}

ImageSaveOptionData::~ImageSaveOptionData()
{
}

void ImageSaveOptionData::Init()
{
    m_nVisionType = ImageSaveOption::VisionType_2D_TOP;
    m_bisSaveReviewImage = FALSE;
    m_bisSaveCroppingImage = FALSE;
    m_vecRawImageSaveOption.resize(0);
    m_bReviewImageOption_Pass = FALSE;
    m_bReviewImageOption_Reject = FALSE;
    m_bReviewImageOption_Marginal = FALSE;
    m_nReviewImageROI = ImageSaveOption::ReviewROI_Not;
    m_bReviewImageCombined = FALSE;
    m_nReviewImageQuality = 30;
    m_nReviewImageMargin = 250;
    m_nCroppingImageSaveOption = ImageSaveOption::CroppingImage_All;
    m_nCroppingImageROI = ImageSaveOption::CroppingROI_Not;
    m_nCroppingImageFrame_Ball = ImageSaveOption::CroppingFrame_NotSave;
    m_nCroppingImageFrame_Land = ImageSaveOption::CroppingFrame_NotSave;
    m_nCroppingImageFrame_Comp = ImageSaveOption::CroppingFrame_NotSave;
    m_nCroppingImageFrame_Surface = ImageSaveOption::CroppingFrame_NotSave;
    m_nCroppingImageQuality = 30;
}

void ImageSaveOptionData::Serialize(ArchiveAllType& ar)
{
    if (ar.IsStoring())
    {
        ar << m_nVisionType;
        ar << m_bisSaveReviewImage;
        ar << m_bisSaveCroppingImage;
        ar << (long)(m_vecRawImageSaveOption.size());
        for (int n = 0; n < (long)(m_vecRawImageSaveOption.size()); n++)
        {
            ar << m_vecRawImageSaveOption[n];
        }
        ar << m_bReviewImageOption_Pass;
        ar << m_bReviewImageOption_Reject;
        ar << m_bReviewImageOption_Marginal;
        ar << m_nReviewImageROI;
        ar << m_bReviewImageCombined;
        ar << m_nReviewImageQuality;
        ar << m_nReviewImageMargin;
        ar << m_nCroppingImageSaveOption;
        ar << m_nCroppingImageROI;
        ar << m_nCroppingImageFrame_Ball;
        ar << m_nCroppingImageFrame_Land;
        ar << m_nCroppingImageFrame_Comp;
        ar << m_nCroppingImageFrame_Surface;
        ar << m_nCroppingImageQuality;
    }
    else
    {
        long Count;
        ar >> m_nVisionType;
        ar >> m_bisSaveReviewImage;
        ar >> m_bisSaveCroppingImage;
        ar >> Count;
        m_vecRawImageSaveOption.resize(Count);
        for (int n = 0; n < Count; n++)
        {
            ar >> m_vecRawImageSaveOption[n];
        }
        ar >> m_bReviewImageOption_Pass;
        ar >> m_bReviewImageOption_Reject;
        ar >> m_bReviewImageOption_Marginal;
        ar >> m_nReviewImageROI;
        ar >> m_bReviewImageCombined;
        ar >> m_nReviewImageQuality;
        ar >> m_nReviewImageMargin;
        ar >> m_nCroppingImageSaveOption;
        ar >> m_nCroppingImageROI;
        ar >> m_nCroppingImageFrame_Ball;
        ar >> m_nCroppingImageFrame_Land;
        ar >> m_nCroppingImageFrame_Comp;
        ar >> m_nCroppingImageFrame_Surface;
        ar >> m_nCroppingImageQuality;
    }
}
