//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../UnitTestCommon.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace UnitTestippModules
{
// Float Array Sorting 함수
int floatcomp(const void* elem1, const void* elem2)
{
    if (*(const float*)elem1 < *(const float*)elem2)
        return -1;
    return *(const float*)elem1 > *(const float*)elem2;
}

TEST_CLASS (tc_ippModules)
{
public:
    TEST_METHOD (GetMaxRange_Float)
    {
        float imageBuffer[]
            = {9.3f, 10.2f, 13.9f, -14.1f, 20.1f, 11.1f, 21.7f, 9.1f, 15.6f, 16.2f, 20.1f, 12.1f, 13.4f, 10.0f, 0.5f,
                17.4f, 20.1f, 255.1f, 4.3f, 12.5f, 18.1f, 14.2f, 21.1f, 23.1f, 9.1f, 11.8f, 19.2f, 12.2f, 24.1f, 21.1f};

        // Float Image 안의 min,max 범위에서 가장 큰값을 찾는 함수이다
        // 가장큰값을(Ipvm::Rect32s 안에 min=13.0에서 max=23.0사이에서) 제대로 찾는지 확인한다

        Ipvm::Image32r image(6 * sizeof(float), 6, 5, imageBuffer);
        Assert::AreEqual(21.1f, CippModules::GetMaxRange_Float(image, 13.0f, 23.0f, Ipvm::Rect32s(3, 3, 5, 5)));

        // ROI가 Image영역을 벗어나서 시도하면 실패한다
        if (1)
        {
            //Assert::AreNotEqual(21.1f, CippModules::GetMaxRange_Float(imageBuffer, 6, 5, 13.0f, 23.0f,  Ipvm::Rect32s(3, 3, 7, 5)));
        }
    }

    TEST_METHOD (GetMeanInIntensityRange_BYTE)
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 10, 0, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        long nMinPixelCount;
        long nDarkPixelArea;
        nMinPixelCount = 4;

        Ipvm::Image8u image(6, 6, 5, imageBuffer);

        //  9에서작은 값 25에서 큰 값들을 버린다
        float mean_value = (float)(9 + 10 + 21 + 9) / (float)4;
        // BYTE Image 안의 min, max 범위에서 평균값을 찾는 함수이다 //bLow<= Inten <= nHigh 인 녀석들의 AVG
        // 평균값을 제대로 찾는지 확인한다
        Assert::AreEqual(mean_value,
            CippModules::GetMeanInIntensityRange_BYTE(
                image, Ipvm::Rect32s(0, 0, 2, 2), 9, 25, nMinPixelCount, nDarkPixelArea));

        if (1)
        {
            // min Pixel값을 5로 하고 돌리면 4개의 pixel 있어서 실패한 경우
            nMinPixelCount = 5;
            Assert::AreNotEqual(12.25f,
                CippModules::GetMeanInIntensityRange_BYTE(
                    image, Ipvm::Rect32s(0, 0, 2, 2), 9, 25, nMinPixelCount, nDarkPixelArea));
        }
    }

    TEST_METHOD (Binarize)
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 10, 0, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        // Image를 Threshold 하는 함수 reverse = FALSE
        // T 값을 15로 설정해서 테스트 진행

        Ipvm::Image8u image(6, 6, 5, imageBuffer);
        Ipvm::Image8u imageDst(6, 5);
        Assert::AreEqual(1, CippModules::Binarize(image, imageDst, Ipvm::Rect32s(0, 0, 6, 5), 15, FALSE));

        if (1)
        {
            for (long y = 0; y < image.GetSizeY(); y++)
            {
                auto* src_y = image.GetMem(0, y);
                auto* dst_y = imageDst.GetMem(0, y);

                for (long x = 0; x < image.GetSizeX(); x++)
                {
                    CString msg;
                    msg.Format(
                        _T("x,y : %d,%d input value: %d, dest value: %d"), x, y, (int)round(src_y[x]), (int)dst_y[x]);

                    int a = (int)src_y[x];
                    int b = (int)dst_y[x];

                    // T 값이 15이기 때문에 15보다 작거나 같은 값들은 0 아니면 255로 설정
                    if (a <= 15)
                        a = 0;
                    else
                        a = 255;
                    // 결과가 정확한지 확인
                    Assert::AreEqual(a, b, msg);
                }
            }
        }
    }

    TEST_METHOD (GetLineData) // ???
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 9, 9, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        BYTE imageDst_b[] = {0, 0, 0, 0, 0, 0};

        Ipvm::Image8u image(6, 6, 5, imageBuffer);

        // BYTE 영상의 Line 데이터를 가져오는 함수
        Assert::AreNotEqual((long)0, CippModules::GetLineData(image, 0.f, 0.f, 3.f, 3.f, 45.f, 45.f, imageDst_b));
        for (int i = 0; i < 6 * 5; i++)
        {
            CString msg;
            msg.Format(_T("\n x,y : %d,%d \n input value: %d"), i % 6, i / 6, (int)imageBuffer[i]);
        }
    }

    TEST_METHOD (SetInvalidPattern_with_constant)
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 10, 0, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        BYTE validPattertn[] = {0, 0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255, 255, 0, 0, 255, 255, 255,
            255, 0, 0, 255, 255, 255, 255, 0};

        // 임시 코드임..
        // 사용하지 않는 픽셀을 특정 값으로 해야 하는데
        // 반대로 하고 있어서 일단 마스트를 반전함.

        Ipvm::Image8u imageSource(6, 6, 5, imageBuffer);
        Ipvm::Image8u imageTest(6, 5);
        Ipvm::Image8u imageValid(6, 6, 5, validPattertn);

        for (int fillIntensity = 0; fillIntensity < 255; fillIntensity++)
        {
            Ipvm::ImageProcessing::Copy(imageSource, Ipvm::Rect32s(imageSource), imageTest);
            Assert::AreEqual(TRUE,
                CippModules::SetInvalidPattern_with_constant(
                    imageTest, imageValid, Ipvm::Rect32s(imageTest), fillIntensity));

            for (long y = 0; y < imageTest.GetSizeY(); y++)
            {
                auto* src_y = imageSource.GetMem(0, y);
                auto* val_y = imageValid.GetMem(0, y);
                auto* dst_y = imageTest.GetMem(0, y);

                for (long x = 0; x < imageTest.GetSizeX(); x++)
                {
                    CString msg;
                    msg.Format(_T("\n P%d - x,y : %d,%d \n input : %d, output : %d \n"), fillIntensity, x, y,
                        (int)src_y[x], (int)dst_y[x]);

                    if (val_y[x])
                    {
                        Assert::AreEqual((int)dst_y[x], (int)src_y[x], msg);
                    }
                    else
                    {
                        Assert::AreEqual((int)dst_y[x], fillIntensity, msg);
                    }
                }
            }
        }

        // ROI가 Image영역을 벗어나서 시도하면 실패한다
        if (1)
        {
            Assert::AreEqual(FALSE,
                CippModules::SetInvalidPattern_with_constant(imageSource, imageValid, Ipvm::Rect32s(1, 0, 7, 5), 5));
        }
    }

    TEST_METHOD (EdgeDetect_Magnitude_Sobel)
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 10, 0, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        BYTE imageDst_b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        Ipvm::Image8u image(6, 6, 5, imageBuffer);
        Ipvm::Image8u imageDst(6, 6, 5, imageDst_b);

        // EdgeDetect_Magnitude_Sobel
        if (1)
        {
            Assert::AreNotEqual(-1L, CippModules::EdgeDetect_Magnitude_Sobel(image, Ipvm::Rect32s(image), imageDst));

            for (int i = 0; i < 6 * 5; i++)
            {
                CString msg;
                msg.Format(_T("\n x,y : %d,%d \n input value: %d\n output value: %d\n"), i % 6, i / 6,
                    (int)imageBuffer[i], (int)imageDst_b[i]);
            }

            int mag_edgeValX;
            int mag_edgeValY;
            int mag_edgeValXY;
            mag_edgeValX = (13 + (2 * 10) + 0) - (9 + (2 * 10) + 13);
            mag_edgeValY = (13 + (2 * 15) + 0) - (9 + (2 * 21) + 13);
            mag_edgeValXY = abs(mag_edgeValX) + abs(mag_edgeValY);

            Assert::AreEqual(mag_edgeValXY, (int)imageDst_b[7]);
        }

        // ROI가 Image영역을 벗어나서 시도하면 실패한다
        if (1)
        {
            Assert::AreEqual(-1L, CippModules::EdgeDetect_Magnitude_Sobel(image, Ipvm::Rect32s(0, 0, 9, 10), imageDst));
        }
    }

    TEST_METHOD (MakeAlignedGrayImage)
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 10, 0, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        BYTE imageDst_b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        BYTE imageResult[]
            = {0, 0, 0, 0, 0, 0, 15, 96, 26, 21, 0, 0, 19, 18, 16, 14, 0, 0, 14, 13, 6, 18, 0, 0, 12, 12, 8, 11, 10, 0};

        Ipvm::Image8u image(6, 6, 5, imageBuffer);
        Ipvm::Image8u imageDst(6, 6, 5, imageDst_b);

        // MakeAlignedGrayImage
        if (1)
        {
            float vecfptCentrode_x = 3.f;
            float vecfptCentrode_y = 3.f;

            Assert::AreEqual(TRUE,
                CippModules::MakeAlignedGrayImage(image, Ipvm::Rect32s(image), Ipvm::Rect32s(image), 360.0f,
                    vecfptCentrode_x, vecfptCentrode_y, imageDst));

            for (int i = 0; i < 6 * 5; i++)
            {
                CString msg;
                msg.Format(_T("\n x,y : %d,%d \n input value: %d\n output value: %d\n"), i % 6, i / 6,
                    (int)imageBuffer[i], (int)imageDst_b[i]);
                Assert::AreEqual(imageResult[i], imageDst_b[i], msg);
            }
        }
    }

    TEST_METHOD (GetHistogramPeaks)
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 10, 0, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        Ipvm::Image8u image(6, 6, 5, imageBuffer);

        if (1)
        {
            Ipvm::Image32s histogram;

            long nPeakNum = 256;
            long* pnPeakID = new long[nPeakNum];
            long* pnPeakCount = new long[nPeakNum];
            long nCountThresh = (long)((float)5.0f * 0.8f + .5f);

            //GetHistogramPeaks
            Assert::AreEqual(TRUE,
                CippModules::GetHistogramPeaks(image, Ipvm::Rect32s(0, 0, 6, 5), histogram, pnPeakID, pnPeakCount,
                    nPeakNum, nCountThresh, 20, 230));
        }

        // pnPeakID 값이 NULL 일 경우 시도하면 실패한다
        if (1)
        {
            Ipvm::Image32s histogram;

            long nPeakNum = 256;
            long* pnPeakID = new long[nPeakNum];
            long* pnPeakCount = new long[nPeakNum];
            long nCountThresh = (long)((float)5.0f * 0.8f + .5f);
            pnPeakID = NULL;

            Assert::AreEqual(FALSE,
                CippModules::GetHistogramPeaks(image, Ipvm::Rect32s(0, 0, 7, 6), histogram, pnPeakID, pnPeakCount,
                    nPeakNum, nCountThresh, 20, 230));
        }
    }

    TEST_METHOD (GetHistogramMassCenter) // ???
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 21, 9, 15, 16, 20, 12, 13, 10, 0, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 9, 11, 19, 12, 24, 21};

        Ipvm::Image8u image(6, 6, 5, imageBuffer);

        if (1)
        {
            // 히스토그램 계산하는 함수
            Ipvm::Image32s histogram;

            long nPeakNum = 256;
            long* pnPeakID = new long[nPeakNum];
            long* pnPeakCount = new long[nPeakNum];
            long nCountThresh = (long)((float)5.0f * 0.8f + .5f);

            //GetHistogramMassCenter
            Assert::AreEqual(TRUE,
                CippModules::GetHistogramPeaks(image, Ipvm::Rect32s(0, 0, 6, 5), histogram, pnPeakID, pnPeakCount,
                    nPeakNum, nCountThresh, 20, 230));

            for (long nID = 0; nID < nPeakNum; nID++)
            {
                CippModules::GetHistogramMassCenter(
                    histogram, pnPeakID[nID], 50); //Peak기준 상하 +-50um기준으로 무게 중심을 이용해 높이를 재산출
            }
        }
    }

    TEST_METHOD (ConvertFloattoByteData)
    {
        float imageBuffer[] = {9.333f, 0.2f, 0.9f, 14.1f, 20.1f, 11.1f, 21.7f, 9.1f, 15.6f, 16.2f, 20.1f, 12.1f, 13.4f,
            10.0f, 0.5f, 17.4f, 20.1f, 255.1f, 4.3f, 12.5f, 18.1f, 14.2f, 21.1f, 255.0f, 9.1f, 11.8f, 19.2f, 12.2f,
            24.1f, 21.1f};

        BYTE imageDst_b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

        Ipvm::Image32r image(6 * sizeof(float), 6, 5, imageBuffer);
        Ipvm::Image8u dst(6, 6, 5, imageDst_b);

        // Float 영상을 BYTE 영상으로 컨버팅 하는 함수
        CippModules::ConvertFloattoByteData(image, 1.f, Ipvm::Rect32s(0, 0, 6, 5), dst);

        for (int i = 0; i < 6 * 5; i++)
        {
            CString msg;
            msg.Format(_T("x,y : %d,%d input value: %d, dest value: %d"), i % 6, i / 6, (int)imageBuffer[i],
                (int)imageDst_b[i]);

            float inputTmpData = imageBuffer[i];
            if (imageBuffer[i] > 255)
            {
                inputTmpData = 0;
            }
            // 결과 영상의 정확하게 컨버팅 되는지 확인
            Assert::AreEqual((int)(inputTmpData), (int)imageDst_b[i], msg);
        }
    }

    TEST_METHOD (TemplateMatching_SurfacePeak) // ???
    {
        BYTE imageBuffer[] = {9, 10, 13, 14, 20, 11, 14, 20, 11, 21, 9, 15, 16, 20, 12, 16, 20, 12, 21, 9, 15, 16, 20,
            12, 16, 20, 12, 21, 9, 15, 16, 20, 12, 16, 20, 12, 13, 10, 0, 17, 20, 255, 17, 20, 255, 4, 12, 18, 14, 21,
            23, 14, 21, 23, 4, 12, 18, 14, 21, 23, 14, 21, 23, 4, 12, 18, 14, 21, 23, 14, 21, 23, 9, 11, 19, 12, 24, 21,
            12, 24, 21};

        BYTE imageDst_b[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 255, 17, 20, 255, 0, 0, 0, 0, 0, 23, 14, 21, 23, 0, 0, 0, 0, 0, 23, 14, 21,
            23, 0, 0, 0, 0, 0, 23, 14, 21, 23, 0, 0, 0, 0, 0, 21, 12, 24, 21};

        // TemplateMatching_SurfacePeak
        Ipvm::Image8u image(9, 9, 9, imageBuffer);
        Ipvm::Image8u imageDst(9, 9, 9, imageDst_b);
        if (1)
        {
            Ipvm::Image32r floatBuffer;
            Ipvm::Point32r2 targetPoint;
            float fDstMinScore;
            float fGapScore;

            floatBuffer.Create(9, 9);
            Assert::AreEqual(TRUE,
                CippModules::TemplateMatching_SurfacePeak(
                    image, Ipvm::Rect32s(0, 0, 9, 9), floatBuffer, imageDst, targetPoint, fDstMinScore, fGapScore));

            for (int i = 0; i < 9 * 9; i++)
            {
                CString msg;
                msg.Format(_T("\n x,y : %d,%d \n input value: %d\n output value: %d\n"), i % 9, i / 9,
                    (int)imageBuffer[i], (int)imageDst_b[i]);
                //Assert::AreEqual(imageResult[i], imageDst_b[i], msg);
            }
        }

        // ROI가 Image영역을 벗어나서 시도하면 실패한다
        if (1)
        {
            //float* m_templateMatchingFloatBuffer;
            //Ipvm::Point32r2 fptSamplingOffset = Ipvm::Point32r2(0.f, 0.f);
            //Ipvm::Point32r2 targetPoint;
            //float fDstMinScore;

            //Assert::AreEqual(FALSE, CippModules::TemplateMatching_SurfacePeak(imageBuffer, 1, 6, 5, Ipvm::Rect32s(0, 0, 6, 5), (void*)m_templateMatchingFloatBuffer, imageDst_b, 1, 6, 5, 5, targetPoint, fDstMinScore));
        }
    }
};
} // namespace UnitTestippModules