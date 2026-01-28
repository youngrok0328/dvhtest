#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/BlobInfo.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class BlobAttribute;

//HDR_6_________________________________ Header body
//
class DPI_BLOB_API CPI_Blob
{
public:
    CPI_Blob();
    ~CPI_Blob();

public:
    long DoBlob(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI, long maxBlobCount,
        Ipvm::Image32s& o_imageLabel, Ipvm::BlobInfo* blobInfos, int32_t& o_blobCount, long minArea = 0);

    long FilteringROI(Ipvm::Image32s& io_imageLabel, const Ipvm::Rect32s& rtROI, long roiOffset,
        Ipvm::BlobInfo* blobInfos, int32_t& o_blobCount);
    long MergeBlobsByDistance(
        Ipvm::BlobInfo* psBlobs, int32_t& nBlobNum, float fMergeDistance, Ipvm::Image32s& io_imageLabel);
    long MixBlobGroup(Ipvm::Image32s& io_imageLabel, Ipvm::BlobInfo* blobInfos, int32_t& blobCount,
        const Ipvm::BlobInfo* blobInfos2, const int32_t blobCount2, const Ipvm::Image32s& imageLabel2);

    bool GetBlobs_HitROIMerge(Ipvm::Image32s& io_imageLabel, Ipvm::BlobInfo* psObjInfo, int32_t& nBlobNum);

    bool GetBlobs_MultiMergeBox(Ipvm::Image32s& io_imageLabel, Ipvm::BlobInfo* psObjInfo, const Ipvm::Rect32s& rtROI,
        const std::vector<Ipvm::Rect32s>& vecrtMerge, const std::vector<Ipvm::Rect32s>& vecrtNumIgnore,
        const std::vector<Ipvm::Rect32s>& vecrtIgnore, Ipvm::Rect32s rtExcluded, short nWidthUL, short nWidthLL,
        short nHeightUL, short nHeightLL, int32_t& nBlobNum);

    // Property 계산
    bool CalcBlobMassCenter(const Ipvm::Image32s& imageLabel, const Ipvm::BlobInfo* blobInfos, const long nBlobNum,
        BlobAttribute* attribute);
    long CalcBlobContrast(const Ipvm::Image8u& imageGray, const Ipvm::Image8u& imageBackground,
        const Ipvm::Image32s& imageLabel, const Ipvm::BlobInfo* blobInfos, const long nBlobNum,
        float fMinAreaDarkContrast, float fMinAreaBrightContrast, BlobAttribute* attribute);
    bool CalcBlobEdgeEnergy(const Ipvm::Image8u& imageGray, const Ipvm::Image32s& imageLabel,
        const Ipvm::BlobInfo* blobInfos, const long nBlobNum, const Ipvm::Image8u& validPattern,
        BlobAttribute* attribute);
    bool CalcBlobEdgeEnergy(const Ipvm::Image8u& imageGray, const Ipvm::Image32s& imageLabel,
        const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute);
    bool CalcBlobWidthLength(const Ipvm::Image32s& imageLabel, const Ipvm::BlobInfo* blobInfos, const long nBlobNum,
        BlobAttribute* attribute);
    long CalcBlobLengthXY(const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute);
    long CalcLocalArea(const Ipvm::BlobInfo* psMaskBlob, const long nMaskBlobNum, const Ipvm::BlobInfo* blobInfos,
        const long nBlobNum, BlobAttribute* attribute);
    long CalcDefect_ROI_Ratio(const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute);
    bool CalcBlobThickness(const Ipvm::BlobInfo* blobInfos, const Ipvm::Image32s& imageLabel, const long nBlobNum,
        Ipvm::Image8u& buffer1, Ipvm::Image32r floatBuffer, BlobAttribute* attributes);

private:
    Ipvm::BlobDetection* m_blobDetection;
};
