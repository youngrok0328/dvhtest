//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingCoupon2D.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionProcessingCoupon2D.h"
#include "VisionProcessingCoupon2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingCoupon2D::OnInspection()
{
    // 검사 진행
    BOOL bResult = DoInspection(true);
    m_pVisionInspDlg->AfterInspect();

    return bResult;
}

BOOL VisionProcessingCoupon2D::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    m_bCoupon = FALSE; // SDY_Coupon2D
    //}}

    Ipvm::TimeCheck time_Insp;

    BOOL bResult = TRUE;

    // Init...
    ResetResult();

    if (m_pVisionInspDlg != NULL && m_pVisionInspDlg->IsWindowVisible())
    {
        // Teaching
        if (detailSetupMode)
        {
            m_pVisionInspDlg->GetROI();
        }

        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    }

    // 기본 데이터 수집
    auto processor = m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_COUPON_2D);
    auto& memory = processor->getReusableMemory();

    // Raw 이미지를 받아온다.
    auto frameIndex = GetImageFrameIndex(0);
    m_VisionPara->m_calcFrameIndex.setFrameIndex(frameIndex);
    Ipvm::Image8u image
        = getImageLot().GetImageFrame(m_VisionPara->m_calcFrameIndex.getFrameIndex(), GetCurVisionModule_Status());
    if (image.GetMem() == nullptr)
        return false;

    // 복사용 이미지 불러오기
    Ipvm::Image8u combineImage;
    if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
        return false;
    if (combineImage.GetMem() == nullptr)
        return false;

    // imagecombine 해서 이미지를 불러온다.
    if (!CippModules::GrayImageProcessingManage(
            getReusableMemory(), &image, true, m_VisionPara->m_ImageProcManagePara, combineImage))
    {
        return FALSE;
    }

    // 검사 시작 ////////////////////////////////////////////
    bResult = DoInsp(detailSetupMode, combineImage);
    /////////////////////////////////////////////////////////

    m_fCalcTime = CAST_FLOAT(time_Insp.Elapsed_ms());

    return bResult;
}

BOOL VisionProcessingCoupon2D::DoInsp(const bool detailSetupMode, Ipvm::Image8u& image)
{
    //결과 초기화
    m_nBlobResult = 0;

    // 검사 파라미터 및 데이터 수집
    auto processor = m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_COUPON_2D);

    Ipvm::Image8u thresholdImage;
    Ipvm::Image8u bufferImage;

    auto& memory = processor->getReusableMemory();

    if (!memory.GetByteImage(thresholdImage, image.GetSizeX(), image.GetSizeY()))
        return FALSE;
    if (!memory.GetByteImage(bufferImage, image.GetSizeX(), image.GetSizeY()))
        return FALSE;

    long nMinBlobSize = m_VisionPara->m_nMinBlobSize;
    long nMinBlobCount = m_VisionPara->m_nMinBlobCount;
    BYTE tabThresholdValue = (BYTE)min(255, max(0, m_VisionPara->m_nCouponThreshold));
    Ipvm::Rect32s rtTabThresholdROI = m_VisionPara->m_CouponROI;

    rtTabThresholdROI &= Ipvm::Rect32s(image);

    bufferImage.FillZero();
    thresholdImage.FillZero();
    Ipvm::ImageProcessing::Copy(image, rtTabThresholdROI, bufferImage);

    // Binarization 실행
    Ipvm::ImageProcessing::BinarizeGreaterEqual(bufferImage, rtTabThresholdROI, tabThresholdValue, thresholdImage);

    // Blob에 필요한 데이터 수집
    Ipvm::Rect32s rtROI = rtTabThresholdROI;
    rtROI.InflateRect(-5, -5);
    double fMean = 0.;
    Ipvm::ImageProcessing::GetMean(image, rtROI, fMean);

    //Blob 추출
    int32_t nBlobNum = BLOB_INFO_SMALL_SIZE;

    Ipvm::Image32s imageLabel;
    if (!memory.GetLongImage(imageLabel, image.GetSizeX(), image.GetSizeY()))
    {
        return false;
    }

    // Blob 진행
    auto* blobClass = memory.GetBlob();
    auto* blobInfos = memory.GetBlobInfo();

    blobClass->DoBlob(
        thresholdImage, rtTabThresholdROI, BLOB_INFO_SMALL_SIZE, imageLabel, blobInfos, nBlobNum, long(nMinBlobSize));

    // 필터링을 하면 ROI 모서리에 닿는게 Blob되지 않는다. >> Coupon 내부에서 영역 검사하게 만들기 위해 일단 주석처리
    // blobClass->FilteringROI(imageLabel, rtTabThresholdROI, 1, blobInfos, nBlobNum);

    // Blob ROI 저장
    m_nBlobCount = nBlobNum;

    // 디버그 및 결과용 인포 생성
    setBlobRois(blobInfos, nBlobNum);
    getMaxBlobSizes(blobInfos, nBlobNum);

    // 디버그 인포 넣기
    SetDebugInfoItem(detailSetupMode, _T("Coupon Check ROI"), m_VisionPara->m_CouponROI, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Coupon Blob Rect"), m_vecrtBlobROI, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Coupon Blob Count"), m_nBlobCount, TRUE);
    SetDebugInfoItem(detailSetupMode, _T("Coupon Max Blob Size"), m_nMaxBlobSize, TRUE);

    // 결과 저장
    // 쿠폰여부를 판단하여 데이터를 던져준다.
    if (nBlobNum > nMinBlobCount)
    {
        m_nBlobResult = 1;
        m_bCoupon = TRUE;
        return false;
    }

    // 쿠폰이 아니면 그냥 pass
    return true;
}

void VisionProcessingCoupon2D::setBlobRois(const Ipvm::BlobInfo* blobInfos, long nblobCount)
{
    //Blob ROI를 저장하기 위한 함수
    m_vecrtBlobROI.clear();
    m_vecrtBlobROI.resize(nblobCount);

    for (long n = 0; n < nblobCount; n++)
    {
        m_vecrtBlobROI[n] = blobInfos[n].m_roi;
    }
}

void VisionProcessingCoupon2D::getMaxBlobSizes(const Ipvm::BlobInfo* blobInfos, long nblobCount)
{
    // MaxBlobSize를 만들기 위함
    m_nMaxBlobSize = 0;

    for (long n = 0; n < nblobCount; n++)
    {
        if (m_nMaxBlobSize < blobInfos[n].m_area)
        {
            m_nMaxBlobSize = blobInfos[n].m_area;
        }
    }
}