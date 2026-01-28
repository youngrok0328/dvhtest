//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dPI_BlobLib.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/BlobAttribute.h"
#include "../../DefineModules/dA_Base/semiinfo.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/BlobDetection.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
static AFX_EXTENSION_MODULE DPI_BlobDLL = {NULL, NULL};

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    // Remove this if you use lpReserved
    UNREFERENCED_PARAMETER(lpReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        TRACE0("DPI_BLOB.DLL Initializing!\n");

        // Extension DLL one-time initialization
        if (!AfxInitExtensionModule(DPI_BlobDLL, hInstance))
            return 0;

        // Insert this DLL into the resource chain
        // NOTE: If this Extension DLL is being implicitly linked to by
        //  an MFC Regular DLL (such as an ActiveX Control)
        //  instead of an MFC application, then you will want to
        //  remove this line from DllMain and put it in a separate
        //  function exported from this Extension DLL.  The Regular DLL
        //  that uses this Extension DLL should then explicitly call that
        //  function to initialize this Extension DLL.  Otherwise,
        //  the CDynLinkLibrary object will not be attached to the
        //  Regular DLL's resource chain, and serious problems will
        //  result.

        new CDynLinkLibrary(DPI_BlobDLL);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        TRACE0("DPI_BLOB.DLL Terminating!\n");
        // Terminate the library before destructors are called
        AfxTermExtensionModule(DPI_BlobDLL);
    }
    return 1; // ok
}

/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
CPI_Blob::CPI_Blob()
    : m_blobDetection(new Ipvm::BlobDetection)
{
}

CPI_Blob::~CPI_Blob()
{
    delete m_blobDetection;
}

long CPI_Blob::DoBlob(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI, long maxBlobCount,
    Ipvm::Image32s& o_imageLabel, Ipvm::BlobInfo* blobInfos, int32_t& o_blobCount, long minArea)
{
    if (m_blobDetection->DetectBlob(i_image, rtROI, true, minArea, maxBlobCount, blobInfos, o_blobCount, o_imageLabel)
        != Ipvm::Status::e_ok)
    {
        return -1;
    }

    return 0;
}

bool CPI_Blob::GetBlobs_HitROIMerge(Ipvm::Image32s& io_imageLabel, Ipvm::BlobInfo* psObjInfo, int32_t& nBlobNum)
{
    for (long blobIndex1 = 0; blobIndex1 < nBlobNum; blobIndex1++)
    {
        auto& blob1 = psObjInfo[blobIndex1];
        if (blob1.m_area == 0)
            continue;

        for (long blobIndex2 = blobIndex1 + 1; blobIndex2 < nBlobNum; blobIndex2++)
        {
            auto& blob2 = psObjInfo[blobIndex2];
            if (blob2.m_area == 0)
                continue;

            if ((blob1.m_roi & blob2.m_roi).IsRectEmpty())
            {
                continue;
            }

            for (long y = blob2.m_roi.m_top; y < blob2.m_roi.m_bottom; y++)
            {
                auto* label_y = io_imageLabel.GetMem(0, y);

                for (long x = blob2.m_roi.m_left; x < blob2.m_roi.m_right; x++)
                {
                    if (label_y[x] == blob2.m_label)
                    {
                        label_y[x] = blob1.m_label;
                    }
                }
            }

            blob1.m_roi |= blob2.m_roi;
            blob1.m_area = blob1.m_area + blob2.m_area;
            blob2.m_area = 0;
        }
    }

    if (m_blobDetection->MergeBlob(nBlobNum, io_imageLabel, psObjInfo, nBlobNum) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool CPI_Blob::GetBlobs_MultiMergeBox(Ipvm::Image32s& io_imageLabel, Ipvm::BlobInfo* psObjInfo,
    const Ipvm::Rect32s& rtROI, const std::vector<Ipvm::Rect32s>& vecrtMerge,
    const std::vector<Ipvm::Rect32s>& vecrtNumIgnore, const std::vector<Ipvm::Rect32s>& vecrtIgnore,
    Ipvm::Rect32s rtExcluded, short nWidthUL, short nWidthLL, short nHeightUL, short nHeightLL, int32_t& nBlobNum)
{
    if (m_blobDetection->MergeBlob(nBlobNum, io_imageLabel, psObjInfo, nBlobNum) != Ipvm::Status::e_ok)
    {
        return false;
    }

    // Filtering
    auto roi = rtROI;
    auto roi_excluded = rtExcluded;

    for (long i = 0; i < nBlobNum; i++)
    {
        auto& blob_info = psObjInfo[i];

        short nHeight = static_cast<short>(blob_info.m_roi.Height());
        short nWidth = static_cast<short>(blob_info.m_roi.Width());

        if (nHeight > nHeightUL || nHeight < nHeightLL || nWidth > nWidthUL || nWidth < nWidthLL)
        {
            psObjInfo[i].m_area = 0;
            continue;
        }

        if ((blob_info.m_roi & roi) != blob_info.m_roi)
        {
            psObjInfo[i].m_area = 0;
            continue;
        }

        if ((blob_info.m_roi & roi_excluded) == blob_info.m_roi)
        {
            psObjInfo[i].m_area = 0;
            continue;
        }
    }

    for (long n = 0; n < (long)vecrtMerge.size(); n++)
    {
        int nFirstID = -1;
        int nMergeNum = 0;

        for (long i = 0; i < nBlobNum; i++)
        {
            if (psObjInfo[i].m_roi.m_left < vecrtMerge[n].m_left)
                continue;
            if (psObjInfo[i].m_roi.m_right > vecrtMerge[n].m_right)
                continue;
            if (psObjInfo[i].m_roi.m_top < vecrtMerge[n].m_top)
                continue;
            if (psObjInfo[i].m_roi.m_bottom > vecrtMerge[n].m_bottom)
                continue;
            if (psObjInfo[i].m_area == 0)
                continue;

            if (nMergeNum == 0)
            {
                nFirstID = i;
            }
            else
            {
                for (long y = psObjInfo[i].m_roi.m_top; y < psObjInfo[i].m_roi.m_bottom; y++)
                {
                    auto* label_y = io_imageLabel.GetMem(0, y);

                    for (long x = psObjInfo[i].m_roi.m_left; x < psObjInfo[i].m_roi.m_right; x++)
                    {
                        if (label_y[x] == psObjInfo[i].m_label)
                        {
                            label_y[x] = psObjInfo[nFirstID].m_label;
                        }
                    }
                }

                psObjInfo[nFirstID].m_roi |= psObjInfo[i].m_roi;
                psObjInfo[nFirstID].m_area = psObjInfo[nFirstID].m_area + psObjInfo[i].m_area;
                psObjInfo[i].m_label = psObjInfo[nFirstID].m_label;
                psObjInfo[i].m_area = 0;
            }

            nMergeNum++;
        }
    }

    /// Ignore Box
    for (long n = 0; n < (long)vecrtNumIgnore.size(); n++)
    {
        for (long i = 0; i < nBlobNum; i++)
        {
            if (psObjInfo[i].m_roi.m_left < vecrtNumIgnore[n].m_left)
                continue;
            if (psObjInfo[i].m_roi.m_right > vecrtNumIgnore[n].m_right)
                continue;
            if (psObjInfo[i].m_roi.m_top <= vecrtNumIgnore[n].m_top)
                continue;
            if (psObjInfo[i].m_roi.m_bottom >= vecrtNumIgnore[n].m_bottom)
                continue;

            psObjInfo[i].m_area = 0;
        }
    }

    /// Ignore Box
    for (long n = 0; n < (long)vecrtIgnore.size(); n++)
    {
        for (long i = 0; i < nBlobNum; i++)
        {
            if (psObjInfo[i].m_roi.m_left < vecrtIgnore[n].m_left)
                continue;
            if (psObjInfo[i].m_roi.m_right > vecrtIgnore[n].m_right)
                continue;
            if (psObjInfo[i].m_roi.m_top <= vecrtIgnore[n].m_top)
                continue;
            if (psObjInfo[i].m_roi.m_bottom >= vecrtIgnore[n].m_bottom)
                continue;

            psObjInfo[i].m_area = 0;
        }
    }

    if (m_blobDetection->MergeBlob(nBlobNum, io_imageLabel, psObjInfo, nBlobNum) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

long CPI_Blob::FilteringROI(Ipvm::Image32s& io_imageLabel, const Ipvm::Rect32s& rtROI, long roiOffset,
    Ipvm::BlobInfo* blobInfos, int32_t& o_blobCount)
{
    Ipvm::Rect32s deflateROI = rtROI;
    deflateROI.DeflateRect(roiOffset, roiOffset);
    for (long nBlob = 0; nBlob < o_blobCount; nBlob++)
    {
        if ((deflateROI & blobInfos[nBlob].m_roi) != blobInfos[nBlob].m_roi)
        {
            blobInfos[nBlob].m_area = 0;
        }
    }

    if (m_blobDetection->MergeBlob(o_blobCount, io_imageLabel, blobInfos, o_blobCount) != Ipvm::Status::e_ok)
    {
        return -1;
    }

    return 0;
}

long CPI_Blob::MergeBlobsByDistance(
    Ipvm::BlobInfo* psBlobs, int32_t& nBlobNum, float fMergeDistance, Ipvm::Image32s& io_imageLabel)
{
    if (m_blobDetection->LinkBlob(io_imageLabel, nBlobNum, fMergeDistance, psBlobs) != Ipvm::Status::e_ok)
    {
        return -1;
    }

    if (m_blobDetection->MergeBlob(nBlobNum, io_imageLabel, psBlobs, nBlobNum) != Ipvm::Status::e_ok)
    {
        return -1;
    }

    return 0;
}

long CPI_Blob::MixBlobGroup(Ipvm::Image32s& io_imageLabel, Ipvm::BlobInfo* blobInfos, int32_t& blobCount,
    const Ipvm::BlobInfo* blobInfos2, const int32_t blobCount2, const Ipvm::Image32s& imageLabel2)
{
    int32_t blobCount1 = blobCount;
    int32_t lastBlobCount = blobCount + blobCount2;
    long maxSourceBlobLabel = 0;

    //------------------------------------------------------------------------
    // 원본 Blob 중에서 가장 큰 라벨 Index를 확인한다
    //------------------------------------------------------------------------

    for (long i = 0; i < blobCount1; i++)
    {
        if (maxSourceBlobLabel < blobInfos[i].m_label)
        {
            maxSourceBlobLabel = blobInfos[i].m_label;
        }
    }

    for (long i = 0; i < blobCount2; i++)
    {
        if (blobCount1 + i >= BLOB_INFO_MAX_SIZE) //kircheis_20210202 Crash
            break;

        blobInfos[blobCount1 + i] = blobInfos2[i];

        long curLabel = maxSourceBlobLabel + blobInfos2[i].m_label;

        blobInfos[blobCount1 + i].m_label = curLabel;

        if (blobInfos2[i].m_area == 0)
        {
            continue;
        }

        long nBeginY = blobInfos2[i].m_roi.m_top;
        long nEndY = blobInfos2[i].m_roi.m_bottom;
        long nBeginX = blobInfos2[i].m_roi.m_left;
        long nEndX = blobInfos2[i].m_roi.m_right;

        for (long y = nBeginY; y < nEndY; y++)
        {
            auto* label_y = io_imageLabel.GetMem(0, y);
            auto* label2_y = imageLabel2.GetMem(0, y);

            for (long x = nBeginX; x < nEndX; x++)
            {
                if (label2_y[x] == blobInfos2[i].m_label)
                {
                    label_y[x] = curLabel;
                }
            }
        }
    }

    blobCount = lastBlobCount;

    return 0;
}

bool CPI_Blob::CalcBlobMassCenter(
    const Ipvm::Image32s& imageLabel, const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute)
{
    if (Ipvm::BlobDetection::GetBlobMassCenter(
            imageLabel, blobInfos, nBlobNum, sizeof(BlobAttribute), &attribute->m_massCenter)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

long CPI_Blob::CalcBlobContrast(const Ipvm::Image8u& imageGray, const Ipvm::Image8u& imageBackground,
    const Ipvm::Image32s& imageLabel, const Ipvm::BlobInfo* blobInfos, const long nBlobNum, float fMinAreaDarkContrast,
    float fMinAreaBrightContrast, BlobAttribute* attribute)
{
    long pnHistoDist[256];

    float fMinAreaContrast(0);

    for (long j = 0; j < nBlobNum; j++)
    {
        auto& blob_info = blobInfos[j];
        auto& blob_attribute = attribute[j];

        // 블럽 fContrast 초기화
        blob_attribute.m_keyContrast = 0;
        blob_attribute.m_averageContrast = 0;

        if (blob_info.m_area == 0)
            continue;

        // Histogram 계산
        long nTotalCount = 0;
        long nDarkCount = 0;
        long nBrightCount = 0;

        // 초기화
        memset(pnHistoDist, 0, sizeof(long) * 256);

        for (long y = blob_info.m_roi.m_top; y < blob_info.m_roi.m_bottom; y++)
        {
            const auto* back_y = imageBackground.GetMem(0, y);
            const auto* gray_y = imageGray.GetMem(0, y);
            const auto* label_y = imageLabel.GetMem(0, y);

            for (long x = blob_info.m_roi.m_left; x < blob_info.m_roi.m_right; x++)
            {
                if (label_y[x] == blob_info.m_label)
                {
                    long value = gray_y[x];
                    long back_value = back_y[x];

                    if (value <= back_value)
                    {
                        pnHistoDist[back_value - value]++;
                        nDarkCount++;
                    }
                    else
                    {
                        pnHistoDist[value - back_value]++;
                        nBrightCount++;
                    }

                    nTotalCount++;
                }
            }
        }

        // m_blobType 및 fContrast 계산
        if (nBrightCount >= nDarkCount) // Bright
        {
            blob_attribute.m_blobType = BlobAttribute::BRIGHT;
            fMinAreaContrast = fMinAreaBrightContrast;
        }
        else // Dark
        {
            blob_attribute.m_blobType = BlobAttribute::DARK;
            fMinAreaContrast = fMinAreaDarkContrast;
        }

        long nAccHisto = 0;

        // Find Key Contrast
        for (long nIntensity = 255; nIntensity >= 0; nIntensity--)
        {
            nAccHisto += pnHistoDist[nIntensity];
            if (nAccHisto >= fMinAreaContrast)
            {
                blob_attribute.m_keyContrast = (float)nIntensity;
                break;
            }
        }

        // Find Average Contrast
        nAccHisto = 0;
        for (long nIntensity = 255; nIntensity >= 0; nIntensity--)
        {
            nAccHisto += (pnHistoDist[nIntensity] * nIntensity);
        }

        blob_attribute.m_averageContrast = nAccHisto / float(nTotalCount);
    }

    return 0;
}

bool CPI_Blob::CalcBlobEdgeEnergy(const Ipvm::Image8u& imageGray, const Ipvm::Image32s& imageLabel,
    const Ipvm::BlobInfo* blobInfos, const long nBlobNum, const Ipvm::Image8u& validPattern, BlobAttribute* attribute)
{
    auto imageROI = Ipvm::Rect32s(imageGray);
    long gray_step = imageGray.GetWidthBytes();

    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        auto& blob_info = blobInfos[nBlob];
        auto& blob_attribute = attribute[nBlob];

        blob_attribute.m_edgeEnergy = 0.f;

        if (blob_info.m_area == 0)
        {
            continue;
        }

        long nBeginY = blob_info.m_roi.m_top;
        long nEndY = min(blob_info.m_roi.m_bottom, (imageROI.Height() - 1));
        long nBeginX = blob_info.m_roi.m_left;
        long nEndX = min(blob_info.m_roi.m_right, (imageROI.Width() - 1));

        long nBlobLabel = blob_info.m_label;
        long nEdgeEnergy = 0;

        for (long y = nBeginY; y < nEndY; y++)
        {
            const auto* label_y = imageLabel.GetMem(0, y);
            const auto* gray_y = imageGray.GetMem(0, y);
            const auto* gray_right_y = gray_y + 1;
            const auto* gray_down_y = gray_y + gray_step;
            const auto* valid_right_y = validPattern.GetMem(1, y);
            const auto* valid_down_y = validPattern.GetMem(1, y + 1);

            for (long x = nBeginX; x < nEndX; x++)
            {
                if (label_y[x] != nBlobLabel)
                    continue;

                if (valid_down_y[x])
                {
                    nEdgeEnergy += abs(gray_y[x] - gray_down_y[x]);
                }

                if (valid_right_y[x])
                {
                    nEdgeEnergy += abs(gray_y[x] - gray_right_y[x]);
                }
            }
        }

        blob_attribute.m_edgeEnergy = float(nEdgeEnergy) / blob_info.m_area;
    }

    return true;
}

bool CPI_Blob::CalcBlobEdgeEnergy(const Ipvm::Image8u& imageGray, const Ipvm::Image32s& imageLabel,
    const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute)
{
    if (Ipvm::BlobDetection::GetBlobEdgeEnergy(
            imageLabel, blobInfos, nBlobNum, imageGray, sizeof(BlobAttribute), &attribute->m_edgeEnergy)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool CPI_Blob::CalcBlobWidthLength(
    const Ipvm::Image32s& imageLabel, const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute)
{
    float totalLength = 0.f;
    if (Ipvm::BlobDetection::GetBlobWidthLength(
            imageLabel, blobInfos, nBlobNum, sizeof(BlobAttribute), &attribute->m_width, &attribute->m_length)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    for (long i = 0; i < nBlobNum; i++)
    {
        auto& blob_info = blobInfos[i];
        auto& blob_attribute = attribute[i];

        if (blob_info.m_area == 0)
        {
            continue;
        }

        totalLength += blob_attribute.m_length;
    }

    // 영훈 : TotalLength는 미리 계산을 해둔다.
    for (long i = 0; i < nBlobNum; i++)
    {
        auto& blob_attribute = attribute[i];

        blob_attribute.m_totalLength = totalLength;
    }

    return true;
}

long CPI_Blob::CalcBlobLengthXY(const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute)
{
    for (long i = 0; i < nBlobNum; i++)
    {
        auto& blob_info = blobInfos[i];
        auto& blob_attribute = attribute[i];

        if (blob_info.m_area == 0)
        {
            blob_attribute.m_lengthX = 0.f;
            blob_attribute.m_lengthY = 0.f;
            continue;
        }

        blob_attribute.m_lengthX = (float)blob_info.m_roi.Width();
        blob_attribute.m_lengthY = (float)blob_info.m_roi.Height();
    }

    return 0;
}

long CPI_Blob::CalcLocalArea(const Ipvm::BlobInfo* psMaskBlob, long nMaskBlobNum, const Ipvm::BlobInfo* blobInfos,
    const long nBlobNum, BlobAttribute* attribute)
{
    float fSumBlob = 0.f; // Mask에 속한 모든 Blob의 Area를 저장한다.
    std::vector<long> vecnBlobID; // Mask에 속한 Blob의 ID를 저장한다.

    for (long nMaskID = 0; nMaskID < nMaskBlobNum; nMaskID++) // 마스크 내부에 포함된 블랍을 찾는다.
    {
        auto& MaskBlob = psMaskBlob[nMaskID];
        Ipvm::Rect32s rtMaskBlobROI = MaskBlob.m_roi;

        vecnBlobID.clear();
        fSumBlob = 0.f;

        for (long nTargetID = 0; nTargetID < nBlobNum; nTargetID++)
        {
            auto& blob_info = blobInfos[nTargetID];
            auto& blob_attribute = attribute[nTargetID];

            auto ptBlobCenter = Ipvm::Point32s2(
                long(blob_attribute.m_massCenter.m_x + 0.5f), long(blob_attribute.m_massCenter.m_y + 0.5f));

            if (rtMaskBlobROI.PtInRect(ptBlobCenter))
            {
                vecnBlobID.push_back(nTargetID);
                fSumBlob += blob_info.m_area;
                blob_attribute.m_localAreaPercent = 100.f * ((float)blob_info.m_area / (float)MaskBlob.m_area);
            }
        }

        for (long nBlobID = 0; nBlobID < (long)vecnBlobID.size();
            nBlobID++) // 이건 모두 같은 값을 가진다. 결과는 어떻게 나올지 모르겠다. 아마 각자 빨간 ROI가 생기겠지
        {
            long nID = vecnBlobID[nBlobID];
            auto& blob_attribute = attribute[nID];

            blob_attribute.m_localAreaPercent_All = 100.f * ((float)fSumBlob / (float)MaskBlob.m_area);
        }
    }

    return 0;
}

long CPI_Blob::CalcDefect_ROI_Ratio(const Ipvm::BlobInfo* blobInfos, const long nBlobNum, BlobAttribute* attribute)
{
    for (long i = 0; i < nBlobNum; i++)
    {
        auto& blob_info = blobInfos[i];
        auto& blob_attribute = attribute[i];

        if (blob_info.m_area == 0)
        {
            blob_attribute.m_defectROIRatio = 0.f;
            continue;
        }

        float fROIArea = blob_attribute.m_lengthX * blob_attribute.m_lengthY;
        blob_attribute.m_defectROIRatio = (float)blob_info.m_area / fROIArea * 100.f;
    }

    return 0;
}

bool CPI_Blob::CalcBlobThickness(const Ipvm::BlobInfo* blobInfos, const Ipvm::Image32s& imageLabel, const long nBlobNum,
    Ipvm::Image8u& buffer1, Ipvm::Image32r floatBuffer, BlobAttribute* attributes)
{
    for (long nBlob = 0; nBlob < nBlobNum; nBlob++)
    {
        attributes[nBlob].m_thickness = 0.f;
        attributes[nBlob].m_locus = 0.f;

        auto& area = blobInfos[nBlob].m_area;

        if (area == 0)
        {
            continue;
        }

        auto rtROI = blobInfos[nBlob].m_roi;
        rtROI.InflateRect(3, 3, 3, 3);
        rtROI &= Ipvm::Rect32s(imageLabel);

        const long nBlobLabel = blobInfos[nBlob].m_label;

        // Make Source Image
        Ipvm::Image8u imageThin(buffer1);
        Ipvm::Image32r imageDistance(floatBuffer);

        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            auto* mask_y = imageThin.GetMem(0, y);
            auto* label_y = imageLabel.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (label_y[x] == nBlobLabel)
                {
                    mask_y[x] = 255;
                }
                else
                {
                    mask_y[x] = 0;
                }
            }
        }

        Ipvm::ImageProcessing::Fill(rtROI, 0.f, imageDistance);
        if (Ipvm::ImageProcessing::DistanceTransform3x3L2(imageThin, rtROI, imageDistance) != Ipvm::Status::e_ok)
        {
            return false;
        }

        // Make Thinned Image
        if (Ipvm::ImageProcessing::ThinningZhangSuen(rtROI, true, imageThin) != Ipvm::Status::e_ok)
        {
            return false;
        }

        float fDistanceSum = 0.f;
        long nPixelCount = 0;
        float fMax(-1.f);

        for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
        {
            auto* thin_y = imageThin.GetMem(0, y);
            auto* dist_y = imageDistance.GetMem(0, y);

            for (long x = rtROI.m_left; x < rtROI.m_right; x++)
            {
                if (thin_y[x])
                {
                    if (fMax < dist_y[x])
                    {
                        fMax = dist_y[x];
                    }

                    fDistanceSum += dist_y[x];
                    nPixelCount++;
                }
            }
        }

        //mc_VNT Site Criteria Value -Nan
        if (nPixelCount > 0 && fDistanceSum > 0)
            attributes[nBlob].m_thickness = 2.f * fDistanceSum / (float)nPixelCount;
        else
            attributes[nBlob].m_thickness = 0.f;

        //완전 꼼수 //kircheis_RoundPKG Locus
        //kircheis_RoundPKG ////kircheis_RoundPKG Locus //1을 더해주는게 맞다
        //mc_VNT Site Criteria Value -Nan
        if (attributes[nBlob].m_thickness > 0 && area > 0)
            attributes[nBlob].m_locus = (float)area / (float)attributes[nBlob].m_thickness + 1.f;
        else
            attributes[nBlob].m_locus = 0.f;
    }

    return true;
}
