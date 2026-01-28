//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "AutoSetting.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingSpec.h"
#include "VisionProcessingSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CAutoSetting::CAutoSetting(CPackageSpec& packageSpec, CDlgVisionProcessingSpec* pVPSpec, ImageViewEx* imageView)
    : m_packageSpec(packageSpec)
{
    m_pVPSpec = pVPSpec;
    m_pVPSpec = pVPSpec;
    m_imageView = imageView;
}

CAutoSetting::~CAutoSetting(void)
{
}

std::vector<Ipvm::Rect32s> CAutoSetting::GetBlobObjectROI(
    const Ipvm::Image8u& image, Ipvm::Rect32s rtBlobROI, long nThresholdValue)
{
    std::vector<Ipvm::Rect32s> vecBlobROI;

    auto* psBlobInfo = m_pVPSpec->m_pVisionInsp->getReusableMemory().GetBlobInfo();
    CPI_Blob* pBlob = m_pVPSpec->m_pVisionInsp->getReusableMemory().GetBlob();

    CString strTemp;

    Ipvm::Image8u imageThreshold;
    Ipvm::Image32s imageLabel;

    m_pVPSpec->m_pVisionInsp->getReusableMemory().GetByteImage(imageThreshold, image.GetSizeX(), image.GetSizeY());
    m_pVPSpec->m_pVisionInsp->getReusableMemory().GetLongImage(imageLabel, image.GetSizeX(), image.GetSizeY());

    CippModules::Binarize(image, imageThreshold, rtBlobROI, nThresholdValue, FALSE);

    int32_t nBlobNum = BLOB_INFO_MEDIUM_SIZE;

    pBlob->DoBlob(imageThreshold, rtBlobROI, BLOB_INFO_MEDIUM_SIZE, imageLabel, psBlobInfo, nBlobNum, 50);

    pBlob->FilteringROI(imageLabel, rtBlobROI, 10, psBlobInfo, nBlobNum);

    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        vecBlobROI.push_back(psBlobInfo[nBlob].m_roi);
        //strTemp.Format(_T("%d"), nBlob);
        //m_imageView->Overlay_AddText(CPoint(psBlobInfo[nBlob].rtROI.right, psBlobInfo[nBlob].rtROI.bottom), strTemp, RGB(255, 0, 0));
    }

    for (auto& roi : vecBlobROI)
    {
        m_imageView->ImageOverlayAdd(roi, RGB(255, 0, 0));
    }
    m_imageView->ImageOverlayShow();

    strTemp.Empty();

    return vecBlobROI;
}

void CAutoSetting::ShowThresholdImage(
    const Ipvm::Image8u& image, Ipvm::Rect32s rtBodyROI, Ipvm::Rect32s rtBlobROI, long nThresoldValue)
{
    Ipvm::Image8u thresholdImage;
    if (!m_pVPSpec->m_pVisionInsp->getReusableMemory().GetByteImage(thresholdImage, image.GetSizeX(), image.GetSizeY()))
    {
        return;
    }

    Ipvm::ImageProcessing::Copy(image, rtBodyROI, thresholdImage);
    CippModules::Binarize(image, thresholdImage, rtBlobROI, nThresoldValue, FALSE);

    m_imageView->SetImage(thresholdImage);
}
