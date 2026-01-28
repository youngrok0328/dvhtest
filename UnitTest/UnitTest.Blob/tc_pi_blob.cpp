//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Blob/dPI_BlobLib.h"
#include "../../DefineModules/dA_Base/BlobAttribute.h"
#include "../UnitTestCommon.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/BlobDetection.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/BlobInfo.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace UnitTestBlob
{
TEST_CLASS (pi_blob)
{
public:
    TEST_METHOD (DoBlob)
    {
        BYTE pbyImage[]
            = {0, 0, 0, 0, 0, 0, 0, 255, 0, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 255, 0, 0, 0, 0, 0, 0, 0, 0};

        Ipvm::Image8u image(6, 6, 5, pbyImage);
        Ipvm::Image32s imageLabel;
        imageLabel.Create(6, 5);

        Ipvm::BlobInfo blobInfos[100];
        CPI_Blob blob;
        int32_t findBlobCount = 100;
        Assert::AreEqual(
            0L, blob.DoBlob(image, Ipvm::Rect32s(0, 0, 6, 5), findBlobCount, imageLabel, blobInfos, findBlobCount));
        Assert::AreEqual(1, findBlobCount);
        Assert::AreEqual(Ipvm::Rect32s(1, 1, 5, 4), blobInfos[0].m_roi);
    }

    TEST_METHOD (GetBlobMassCenter)
    {
        BYTE pbyImage[] = {0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 0, 255, 255, 255, 255, 255,
            255, 0, 0, 255, 255, 255, 255, 255, 255, 0, 255, 255, 255, 255, 255, 255, 0, 0};

        Ipvm::Image8u image(8, 8, 5, pbyImage);
        Ipvm::Image32s imageLabel(8, 5);

        if (1)
        {
            // 기존 Blob Edge Enegry 계산
            CPI_Blob blob;

            Ipvm::BlobInfo blobInfos[100];
            BlobAttribute blobAttribute[100];
            int32_t blobNum = 100;

            Assert::AreEqual(0L, blob.DoBlob(image, Ipvm::Rect32s(image), blobNum, imageLabel, blobInfos, blobNum));

            Assert::AreEqual(1, blobNum);

            Assert::AreEqual(true, blob.CalcBlobMassCenter(imageLabel, blobInfos, blobNum, blobAttribute));
            Assert::AreEqual(2.96154f, blobAttribute[0].m_massCenter.m_x, 0.0001f);
            Assert::AreEqual(2.23077f, blobAttribute[0].m_massCenter.m_y, 0.0001f);
        }

        if (1)
        {
            // VisionMadangSDK MassCenter 계산
            Ipvm::BlobDetection blobDetection;
            Ipvm::BlobInfo blobInfos[100];

            int32_t blobNum = 100;
            Assert::AreEqual(Ipvm::Status::e_ok,
                blobDetection.DetectBlob(image, Ipvm::Rect32s(image), true, 0, 100, blobInfos, blobNum, imageLabel));
            Assert::AreEqual(1, blobNum);

            Ipvm::Point32r2 massCenter;
            Assert::AreEqual(Ipvm::Status::e_ok,
                Ipvm::BlobDetection::GetBlobMassCenter(imageLabel, blobInfos, blobNum, &massCenter));
            Assert::AreEqual(2.96154f, massCenter.m_x, 0.0001f);
            Assert::AreEqual(2.23077f, massCenter.m_y, 0.0001f);
        }
    }

    TEST_METHOD (CalcLocalArea)
    {
        // Mask Blob Area과 Area차이 계산
        // 대응하는 VisionMadangSDK 없음
        // SDK 에는 안넣고 그냥 코드를 유지하는게 좋지 않을까 생각
    }

    TEST_METHOD (CalcBlobLengthXY)
    {
        // Blob ROI 의 SizeX, SizeY 이나
        // 옵션에 따라 긴방향을 LengthY로 바꾸는 파라메터 존재
        // 대응하는 VisionMadangSDK 없음
    }

    TEST_METHOD (CalcDefect_ROI_Ratio)
    {
        // Area / (LengthX * LengthY) * 100.f
        // 대응하는 VisionMadangSDK 없음
    }

    TEST_METHOD (CalcBlobWidthLength)
    {
        BYTE pbyImage[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 20, 30, 10, 5, 0, 0, 0, 0, 14, 15, 20,
            7, 0, 0, 0, 0, 15, 16, 10, 5, 0, 0};

        Ipvm::Image8u image(8, 8, 5, pbyImage);
        Ipvm::Image8u imageBinary(8, 5);
        Ipvm::Image32s imageLabel(8, 5);

        Ipvm::ImageProcessing::BinarizeGreater(image, Ipvm::Rect32s(image), 5, imageBinary);

        if (1)
        {
            // 기존 Blob Edge Enegry 계산
            CPI_Blob blob;

            Ipvm::BlobInfo blobInfos[100];
            BlobAttribute blobAttributes[100];

            int32_t blobNum = 100;

            Assert::AreEqual(
                0L, blob.DoBlob(imageBinary, Ipvm::Rect32s(image), blobNum, imageLabel, blobInfos, blobNum));

            Assert::AreEqual(1, blobNum);

            Assert::AreEqual(true, blob.CalcBlobWidthLength(imageLabel, blobInfos, blobNum, blobAttributes));
            Assert::AreEqual(3.f, blobAttributes[0].m_width, 0.0001f);
            Assert::AreEqual(4.f, blobAttributes[0].m_length, 0.0001f);
        }

        if (1)
        {
            // VisionMadangSDK Width Length 계산
            // 문제 1 : Angle 리턴값이 없음 (현재 iPack Blob에 변수는 있으나 사용되지 않음)

            Ipvm::BlobDetection blobDetection;
            Ipvm::BlobInfo blobInfos[100];

            int32_t blobNum = 100;
            Assert::AreEqual(Ipvm::Status::e_ok,
                blobDetection.DetectBlob(
                    imageBinary, Ipvm::Rect32s(image), true, 0, 100, blobInfos, blobNum, imageLabel));
            Assert::AreEqual(1, blobNum);

            float width;
            float length;
            Assert::AreEqual(Ipvm::Status::e_ok,
                Ipvm::BlobDetection::GetBlobWidthLength(imageLabel, blobInfos, blobNum, &width, &length));
            Assert::AreEqual(3.f, width, 0.0001f);
            Assert::AreEqual(4.f, length, 0.0001f);
        }
    }
};

} // namespace UnitTestBlob