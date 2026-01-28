//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ippModules.h"

//CPP_2_________________________________ This project's headers
#include "IPVMCandidate.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Algorithm/Mathematics.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image16s.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Base/Size32s2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL CheckRoi(const Ipvm::Rect32s& rtROI, long i_width, long i_height)
{
    if (rtROI.Width() <= 0 || rtROI.Height() <= 0 || rtROI.Width() > i_width || rtROI.Height() > i_height)
        return FALSE;
    if (rtROI.m_left < 0 || rtROI.m_top < 0 || rtROI.m_right < 0 || rtROI.m_bottom < 0)
        return FALSE;
    if (rtROI.m_left > i_width || rtROI.m_top > i_height || rtROI.m_right > i_width || rtROI.m_bottom > i_height)
        return FALSE;
    if (rtROI.m_right < rtROI.m_left || rtROI.m_bottom < rtROI.m_top)
        return FALSE;

    return TRUE;
}

long limit(long nValue)
{
    // 0과 255 사이의 값만 허용한다.
    return ((nValue > 255) ? 255 : ((nValue < 0) ? 0 : nValue));
}

float GetMean_BYTE(const Ipvm::Image8u& image, Ipvm::Rect32s rtROI)
{
    double mean = 0.;
    auto retValue = Ipvm::ImageProcessing::GetMean(image, rtROI, mean);

    VERIFY(retValue == Ipvm::Status::e_ok);

    return (float)(mean);
}

bool DivImage_NS(VisionReusableMemory& memory, const Ipvm::Image8u& i_image1, const Ipvm::Image8u& i_image2,
    const Ipvm::Rect32s& rtROI, Ipvm::Image8u& o_image)
{
    Ipvm::Rect32s rtNewROI(rtROI);

    if (CheckRoi(rtNewROI, i_image1.GetSizeX(), i_image1.GetSizeY()) == FALSE)
        return false;
    if (CheckRoi(rtNewROI, i_image2.GetSizeX(), i_image2.GetSizeY()) == FALSE)
        return false;
    if (CheckRoi(rtNewROI, o_image.GetSizeX(), o_image.GetSizeY()) == FALSE)
        return false;

    Ipvm::Image16s shortDevidee;
    Ipvm::Image16s shorDevider;
    Ipvm::Image16s shortResult;

    if (!memory.GetShortImage(shortDevidee, i_image1.GetSizeX(), i_image1.GetSizeY())
        || (rtROI & Ipvm::Rect32s(shortDevidee)) != rtROI)
        return false;
    if (!memory.GetShortImage(shorDevider, i_image1.GetSizeX(), i_image1.GetSizeY())
        || (rtROI & Ipvm::Rect32s(shorDevider)) != rtROI)
        return false;
    if (!memory.GetShortImage(shortResult, i_image1.GetSizeX(), i_image1.GetSizeY())
        || (rtROI & Ipvm::Rect32s(shortResult)) != rtROI)
        return false;

    if (Ipvm::ImageProcessing::Convert(i_image1, rtROI, shortDevidee) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Convert(i_image2, rtROI, shorDevider) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Divide(shortDevidee, shorDevider, rtROI, -7, shortResult) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Divide(128, rtROI, 0, shortResult) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Convert(shortResult, rtROI, o_image) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool SubtractImage_NS(VisionReusableMemory& memory, const Ipvm::Image8u& subtractee, const Ipvm::Image8u& subtracter,
    const Ipvm::Rect32s& rtROI, Ipvm::Image8u& o_image)
{
    if (CheckRoi(rtROI, subtractee.GetSizeX(), subtractee.GetSizeY()) == FALSE)
        return false;
    if (CheckRoi(rtROI, subtracter.GetSizeX(), subtracter.GetSizeY()) == FALSE)
        return false;
    if (CheckRoi(rtROI, o_image.GetSizeX(), o_image.GetSizeY()) == FALSE)
        return false;

    Ipvm::Image16s shortSubtractee;
    Ipvm::Image16s shortSubtracter;
    Ipvm::Image16s shortResult;

    if (!memory.GetShortImage(shortSubtractee, subtractee.GetSizeX(), subtractee.GetSizeY())
        || (rtROI & Ipvm::Rect32s(shortSubtractee)) != rtROI)
        return false;
    if (!memory.GetShortImage(shortSubtracter, subtractee.GetSizeX(), subtractee.GetSizeY())
        || (rtROI & Ipvm::Rect32s(shortSubtracter)) != rtROI)
        return false;
    if (!memory.GetShortImage(shortResult, subtractee.GetSizeX(), subtractee.GetSizeY())
        || (rtROI & Ipvm::Rect32s(shortResult)) != rtROI)
        return false;

    if (Ipvm::ImageProcessing::Convert(subtractee, rtROI, shortSubtractee) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Convert(subtracter, rtROI, shortSubtracter) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Subtract(shortSubtractee, shortSubtracter, rtROI, 0, shortResult) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Add(shortResult, 255, rtROI, 1, shortResult) != Ipvm::Status::e_ok)
    {
        return false;
    }

    if (Ipvm::ImageProcessing::Convert(shortResult, rtROI, o_image) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return 0;
}

BOOL Contrast(const Ipvm::Image8u& i_image, const Ipvm::Rect32s rtROI, const long nContrast, const long nCenter,
    Ipvm::Image8u& o_image)
{
    if (CheckRoi(rtROI, i_image.GetSizeX(), i_image.GetSizeY()) == FALSE)
        return FALSE;
    if (CheckRoi(rtROI, o_image.GetSizeX(), o_image.GetSizeY()) == FALSE)
        return FALSE;

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        auto* src_y = i_image.GetMem(0, y);
        auto* dst_y = o_image.GetMem(0, y);

        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            BYTE bValue = src_y[x];
            dst_y[x] = static_cast<BYTE>(limit(bValue + ((bValue - nCenter) * nContrast / 100)));
        }
    }

    return TRUE;
}

long ImageProcMultiplex(const Ipvm::Image8u& image, Ipvm::Image8u& imageDst, Ipvm::Image8u& buffer0,
    Ipvm::Image8u& buffer1, const Ipvm::Rect32s& rtROI, const std::vector<short>& vecnImageProc,
    const std::vector<short>& vecnImageProcParams)
{
    if (!CheckRoi(rtROI, image.GetSizeX(), image.GetSizeY()))
        return -1;
    if (!CheckRoi(rtROI, imageDst.GetSizeX(), imageDst.GetSizeY()))
        return -1;
    if (!CheckRoi(rtROI, buffer0.GetSizeX(), buffer0.GetSizeY()))
        return -1;
    if (!CheckRoi(rtROI, buffer1.GetSizeX(), buffer1.GetSizeY()))
        return -1;

    long nProcNum = (long)vecnImageProc.size();

    if (nProcNum == 0)
    {
        Ipvm::ImageProcessing::Copy(image, rtROI, imageDst);
        return 0;
    }

    Ipvm::ImageProcessing::Fill(rtROI, 0, buffer0);
    Ipvm::ImageProcessing::Fill(rtROI, 0, buffer1);

    // Processing 하면서 Image를 Dest를 계속 Swap하면서 진행할 건데
    // 마지막 Processing 후의 결과 procDest가 imageDest 가 되도록 순서를 조정해 준다

    Ipvm::Image8u procSource = image;
    Ipvm::Image8u procDest = imageDst;
    Ipvm::Image8u procDestBuffer = buffer0;
    Ipvm::Image8u procBuffer = buffer1;

    if (nProcNum % 2 == 0)
    {
        procDest = buffer0;
        procDestBuffer = imageDst;
    }

    for (long i = 0; i < nProcNum; i++)
    {
        short nProcType = vecnImageProc[i];

        //Image Proc Type 추가시 IppModule의 enumImageProc에 항목을 추가한 후, 여기에 필요함수를 호출시킴.
        //그리고 UI를 위해 iPack 소속의 DlgImageGrayProcManager.cpp에 gl_szStrAlgoName 선언부에 순서에 맞게 제목을 입력한다.
        switch (nProcType) //이 내용 안에서 Src Image를 넘기는 부분에 Dest Image를 넘기는 이유는 각 연산을 중첩하여
        { // 사용하기 위함이다.
            case enumImageProc_Median:
                Ipvm::ImageProcessing::FilterMedian3x3(procSource, rtROI, true, procDest);
                break;
            case enumImageProc_LowPass:
                Ipvm::ImageProcessing::FilterLowPass3x3(procSource, rtROI, true, procDest);
                break;
            case enumImageProc_Sharpen:
                Ipvm::ImageProcessing::FilterSharpen3x3(procSource, rtROI, true, procDest);
                break;
            case enumImageProc_Invert:
                Ipvm::ImageProcessing::BitwiseNot(procSource, rtROI, procDest);
                break;
            case enumImageProc_HistogramEqualization:
                Ipvm::ImageProcessing::EqualizeHistogram(procSource, rtROI, procDest);
                break;
            case enumImageProc_MorphologicalDilate:
                Ipvm::ImageProcessing::MorphDilate3x3(procSource, rtROI, true, procDest);
                break;
            case enumImageProc_MorphologicalErode:
                Ipvm::ImageProcessing::MorphErode3x3(procSource, rtROI, true, procDest);
                break;
            case enumImageProc_AddSelf:
                if (procSource.GetMem() != image.GetMem())
                {
                    Ipvm::ImageProcessing::Add(procSource, image, rtROI, 0, procDest);
                }
                else
                {
                    // A+A = A*2 와 같음
                    Ipvm::ImageProcessing::Multiply(procSource, 2, rtROI, 0, procDest);
                }
                break;
            case enumImageProc_AddResult:
                Ipvm::ImageProcessing::Multiply(procSource, 2, rtROI, 0, procDest);
                break;
            case enumImageProc_Offset:
                if (vecnImageProcParams[i] > 0)
                {
                    Ipvm::ImageProcessing::Add(procSource, BYTE(vecnImageProcParams[i]), rtROI, 0, procDest);
                }
                else
                {
                    Ipvm::ImageProcessing::Subtract(procSource, BYTE(-vecnImageProcParams[i]), rtROI, 0, procDest);
                }
                break;
            case enumImageProc_Contrast:
                Contrast(procSource, rtROI, vecnImageProcParams[i], 128, procDest);
                break;
        }

        if (1)
        {
            // 버퍼를 Swap 하자
            auto temp = procDest;

            if (procSource.GetMem() == image.GetMem())
            {
                // Source 가 원본 버퍼였다면 Source 버퍼를 손상시켜서는 안되므로
                // 대상 버퍼는 임시버퍼를 아용한다

                procDest = procDestBuffer;
            }
            else
            {
                procDest = procSource;
            }

            procSource = temp;
        }
    }

    return 0;
}

bool ImageProcessingManage(VisionReusableMemory& memory, const Ipvm::Image8u* image, const Ipvm::Image8u& auxImage,
    Ipvm::Image8u& buffer0, Ipvm::Image8u& buffer1, Ipvm::Image8u& buffer2, Ipvm::Image8u& buffer3,
    const Ipvm::Rect32s& rtROI, const std::vector<short>& vecnImage1Proc,
    const std::vector<short>& vecnImage1ProcParams, const std::vector<short>& vecnImage2Proc,
    const std::vector<short>& vecnImage2ProcParams, const std::vector<short>& vecnImageFinalProc,
    const std::vector<short>& vecnImageFinalProcParams, short nCombineType, Ipvm::Image8u& dst)
{
    if (image == nullptr)
        return false;
    if (image->GetMem() == nullptr)
        return false;

    if (!CheckRoi(rtROI, image->GetSizeX(), image->GetSizeY()))
        return false;
    if (!CheckRoi(rtROI, buffer0.GetSizeX(), buffer0.GetSizeY()))
        return false;
    if (!CheckRoi(rtROI, buffer1.GetSizeX(), buffer1.GetSizeY()))
        return false;
    if (!CheckRoi(rtROI, buffer2.GetSizeX(), buffer2.GetSizeY()))
        return false;
    if (!CheckRoi(rtROI, buffer3.GetSizeX(), buffer3.GetSizeY()))
        return false;
    if (!CheckRoi(rtROI, dst.GetSizeX(), dst.GetSizeY()))
        return false;

    //Image1(Src-Currrent)에 대한 영상처리 수행
    ImageProcMultiplex(*image, buffer2, buffer0, buffer1, rtROI, vecnImage1Proc, vecnImage1ProcParams);

    if (nCombineType == enumImageCombine_NotThing)
    {
        // Current에 대한 처리만 하면 될 경우 함수를 종료
        Ipvm::ImageProcessing::Copy(buffer2, rtROI, dst);
        return true;
    }

    if (auxImage.GetMem() == nullptr)
    {
        // 추가프레임에 대한 계산을 해야하는데 추가프레임이 지정되지 않았다
        return false;
    }

    if (!CheckRoi(rtROI, auxImage.GetSizeX(), auxImage.GetSizeY()))
        return false;

    //Image2(Src-Aux)에 대한 영상처리 수행
    ImageProcMultiplex(auxImage, buffer3, buffer0, buffer1, rtROI, vecnImage2Proc, vecnImage2ProcParams);

    // Combine Type 추가시 IppModule의 enumImageCombine에 항목을 추가한 후.
    // 여기에 필요함수를 호출시킴. 그리고 UI를 위해 iPack 소속의 DlgImageGrayProcManager.cpp에
    // gl_szStrCombineType 선언부에 순서에 맞게 제목을 입력한다.
    //Image Combine 알고리즘

    switch (nCombineType)
    {
        case enumImageCombine_Add: // Cur + Aux
            Ipvm::ImageProcessing::Add(buffer2, buffer3, rtROI, 0, dst);
            break;
        case enumImageCombine_Sub1_2: //Cur - Aux
            Ipvm::ImageProcessing::Subtract(buffer2, buffer3, rtROI, 0, dst);
            break;
        case enumImageCombine_Sub2_1: //Aux - Cur
            Ipvm::ImageProcessing::Subtract(buffer3, buffer2, rtROI, 0, dst);
            break;
        case enumImageCombine_And: // Cur && Aux
            Ipvm::ImageProcessing::Copy(buffer3, rtROI, dst);
            Ipvm::ImageProcessing::BitwiseAnd(buffer2, rtROI, dst);
            break;
        case enumImageCombine_Or: // Cur || Aux
            Ipvm::ImageProcessing::Copy(buffer3, rtROI, dst);
            Ipvm::ImageProcessing::BitwiseOr(buffer2, rtROI, dst);
            break;
        case enumImageCombine_Add_NS: // Cur + Aux  No Saturated   //kircheis_PLIT 091126
            Ipvm::ImageProcessing::Add(buffer2, buffer3, rtROI, 1, dst);
            break;
        case enumImageCombine_Sub1_2_NS: //Cur - Aux  No Saturated
            SubtractImage_NS(memory, buffer2, buffer3, rtROI, dst);
            break;
        case enumImageCombine_Sub2_1_NS: //Aux - Cur  No Saturated
            SubtractImage_NS(memory, buffer3, buffer2, rtROI, dst);
            break;
        case enumImageCombine_Div_NS: //Cur / Aux  No Saturated
            DivImage_NS(memory, buffer2, buffer3, rtROI, dst);
            break;
        case enumImageCombine_Mul_NS: //Cur * Aux  No Saturated
            Ipvm::ImageProcessing::MultiplyScale(buffer3, buffer2, rtROI, dst);
            break;
        case enumImageCombine_MAX: //Max Inten.(Cur, Aux) //kircheis_DieCrack
            Ipvm::ImageProcessing::Max(buffer2, buffer3, rtROI, dst);
            break;
        case enumImageCombine_MIN: //Min Inten.(Cur, Aux) //kircheis_DieCrack
            Ipvm::ImageProcessing::Min(buffer2, buffer3, rtROI, dst);
            break;
    }

    ImageProcMultiplex(dst, buffer2, buffer0, buffer1, rtROI, vecnImageFinalProc,
        vecnImageFinalProcParams); //Result Image(Combine 결과)에 대한 영상처리 수행
    Ipvm::ImageProcessing::Copy(buffer2, rtROI, dst);

    return true;
}

float CippModules::GetMaxRange_Float(const Ipvm::Image32r i_image, float fMin, float fMax, Ipvm::Rect32s rtROI)
{
    rtROI &= Ipvm::Rect32s(i_image);

    float fMaxVal = 0.f;
    int nLineDataNum = 0;

    float* pfData = new float[rtROI.Width()];

    for (long y = rtROI.m_top; y < rtROI.m_bottom; y++)
    {
        nLineDataNum = 0;
        auto* image_y = i_image.GetMem(0, y);
        for (long x = rtROI.m_left; x < rtROI.m_right; x++)
        {
            if (fMin < image_y[x] && image_y[x] < fMax)
            {
                pfData[nLineDataNum] = image_y[x];
                nLineDataNum++;
            }
        }

        if (nLineDataNum < 1)
            continue;

        float lineMax = 0.f;
        if (Ipvm::ImageProcessing::GetMax(Ipvm::Image32r(nLineDataNum, 1, pfData, nLineDataNum * sizeof(float)),
                Ipvm::Rect32s(0, 0, nLineDataNum, 1), lineMax)
            != Ipvm::Status::e_ok)
        {
            delete[] pfData; //kircheis_MemLeak
            return -1.f;
        }

        fMaxVal = max(fMaxVal, lineMax);
    }
    delete[] pfData; //kircheis_MemLeak
    return fMaxVal;
}

float CippModules::GetMeanNotUseZero_BYTE(
    const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI, long& o_nValidDataNum, float& o_fValidDataRatio)
{
    Ipvm::Rect32s modifyROI = rtROI & Ipvm::Rect32s(i_image);

    int nDataNum = 0;
    long nWidth = rtROI.Width();
    long nHeight = rtROI.Height();

    BYTE* pbData = new BYTE[nWidth * nHeight];

    for (long y = modifyROI.m_top; y < modifyROI.m_bottom; y++)
    {
        auto* image_y = i_image.GetMem(0, y);
        for (long x = modifyROI.m_left; x < modifyROI.m_right; x++)
        {
            if (image_y[x] > 1)
            {
                pbData[nDataNum] = image_y[x];
                nDataNum++;
            }
        }
    }

    o_nValidDataNum = nDataNum;

    o_fValidDataRatio = (float)nDataNum / (float)(nWidth * nHeight);

    modifyROI = Ipvm::Rect32s(0, 0, nDataNum, 1);

    float fMean = GetMean_BYTE(Ipvm::Image8u(nDataNum, 1, pbData, nDataNum), modifyROI);

    delete[] pbData;

    return fMean;
}

float CippModules::GetMeanInIntensityRange_BYTE(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI, BYTE bLow,
    BYTE bHigh, long nMinPxlCount, long& o_nDarkPixelCnt) //kircheis_20161206//bLow<= Inten <= nHigh 인 녀석들의 AVG
{
    Ipvm::Rect32s modifyROI = rtROI & Ipvm::Rect32s(i_image);

    auto nDataNum = 0;
    auto nWidth = modifyROI.Width();
    auto nHeight = modifyROI.Height();
    auto nSize = nWidth * nHeight;

    if (nSize <= 0)
    {
        o_nDarkPixelCnt = 0;
        return 0.f;
    }

    BYTE* pbData = new BYTE[nSize];

    for (auto y = modifyROI.m_top; y < modifyROI.m_bottom; y++)
    {
        const auto* image_y = i_image.GetMem(0, y);

        for (auto x = modifyROI.m_left; x < modifyROI.m_right; x++)
        {
            if (image_y[x] >= bLow && image_y[x] <= bHigh)
            {
                pbData[nDataNum] = image_y[x];
                nDataNum++;
            }
        }
    }
    o_nDarkPixelCnt = nDataNum;
    if (nDataNum == 0 || nDataNum < nMinPxlCount)
    {
        delete[] pbData;
        return 0.f;
    }

    modifyROI = Ipvm::Rect32s(0, 0, nDataNum, 1);

    float fMean = GetMean_BYTE(Ipvm::Image8u(nDataNum, 1, pbData, nDataNum), modifyROI);

    delete[] pbData;

    return fMean;
}

BOOL CippModules::Binarize(const Ipvm::Image8u& i_image, Ipvm::Image8u& o_image, const Ipvm::Rect32s& rtRoi,
    long nThresholdValue, BOOL isReverse)
{
    if (isReverse)
    {
        if (Ipvm::ImageProcessing::BinarizeLess(i_image, rtRoi, BYTE(nThresholdValue), o_image) != Ipvm::Status::e_ok)
        {
            return FALSE;
        }

        return TRUE;
    }

    if (Ipvm::ImageProcessing::BinarizeGreater(i_image, rtRoi, BYTE(nThresholdValue), o_image) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    return TRUE;
}

long CippModules::GetLineData(
    const Ipvm::Image8u& i_image, float x0, float y0, float x1, float y1, float fCos, float fSin, BYTE* pLineOut)
{
    float fdx = x1 - x0;
    float fdy = y1 - y0;
    float fdist = float(sqrt(fdx * fdx + fdy * fdy));

    long nDataNum = long(fdist + 0.5f);

    if (nDataNum == 0)
        return 0;
    if (pLineOut == NULL)
        return nDataNum;

    //{{ Get Data
    const long nImageSizeX = i_image.GetSizeX(); //kircheis_20230216
    const long nImageSizeY = i_image.GetSizeY();
    for (long i = 0; i < nDataNum; i++)
    {
        float x = x0 + i * fCos;
        float y = y0 + i * fSin;

        long nx = long(x);
        long ny = long(y);

        float h1 = y - ny;
        float h2 = ny + 1 - y;
        float w1 = x - nx;
        float w2 = nx + 1 - x;

        if (nx < 0 || nx >= nImageSizeX || ny < 0 || ny >= nImageSizeY) //kirchies_20230216
            return 0;

        unsigned char p1 = i_image[ny][nx];
        unsigned char p2 = i_image[ny][nx + 1];
        unsigned char p3 = i_image[ny + 1][nx + 1];
        unsigned char p4 = i_image[ny + 1][nx];

        pLineOut[i] = unsigned char(w2 * h2 * p1 + w1 * h2 * p2 + w1 * h1 * p3 + w2 * h1 * p4 + 0.5);
    }

    return nDataNum;
}

BOOL CippModules::SetInvalidPattern_with_constant(
    Ipvm::Image8u& io_image, const Ipvm::Image8u& i_vaildPattern, const Ipvm::Rect32s& rtROI, const BYTE nConst)
{
    if ((Ipvm::Rect32s(io_image) & rtROI) != rtROI)
        return FALSE;
    if ((Ipvm::Rect32s(i_vaildPattern) & rtROI) != rtROI)
        return FALSE;

    // 임시 코드임..
    // 사용하지 않는 픽셀을 특정 값으로 해야 하는데
    // 반대로 하고 있어서 일단 마스트를 반전함.

    auto& pattern = const_cast<Ipvm::Image8u&>(i_vaildPattern);

    Ipvm::ImageProcessing::BitwiseNot(rtROI, pattern);
    Ipvm::ImageProcessing::Fill(rtROI, nConst, pattern, io_image);
    Ipvm::ImageProcessing::BitwiseNot(rtROI, pattern);

    return TRUE;
}

long CippModules::EdgeDetect_Magnitude_Sobel(
    const Ipvm::Image8u& i_imageSrc, const Ipvm::Rect32s& rtROI, Ipvm::Image8u& o_imageDst)
{
    Ipvm::Rect32s rtTrueROI(rtROI);

    rtTrueROI.DeflateRect(1, 1, 1, 1);

    if (CheckRoi(rtTrueROI, i_imageSrc.GetSizeX(), i_imageSrc.GetSizeY()) == FALSE)
        return -1;
    if (CheckRoi(rtTrueROI, o_imageDst.GetSizeX(), o_imageDst.GetSizeY()) == FALSE)
        return -1;

    // P1 P2 P3
    // P8 P0 P4
    // P7 P6 P5

    long nOffsetP1 = -i_imageSrc.GetWidthBytes() - 1;
    long nOffsetP2 = -i_imageSrc.GetWidthBytes();
    long nOffsetP3 = -i_imageSrc.GetWidthBytes() + 1;
    long nOffsetP4 = +1;
    long nOffsetP5 = i_imageSrc.GetWidthBytes() + 1;
    long nOffsetP6 = i_imageSrc.GetWidthBytes();
    long nOffsetP7 = i_imageSrc.GetWidthBytes() - 1;
    long nOffsetP8 = -1;

    for (long y = rtTrueROI.m_top; y < rtTrueROI.m_bottom; y++)
    {
        auto* src_y = i_imageSrc.GetMem(0, y);
        auto* dst_y = o_imageDst.GetMem(0, y);

        for (long x = rtTrueROI.m_left; x < rtTrueROI.m_right; x++)
        {
            auto* pbyP0 = src_y + x;

            long dx = abs(-pbyP0[nOffsetP1] + pbyP0[nOffsetP3] - 2 * pbyP0[nOffsetP8] + 2 * pbyP0[nOffsetP4]
                - pbyP0[nOffsetP7] + pbyP0[nOffsetP5]);

            long dy = abs(pbyP0[nOffsetP1] + 2 * pbyP0[nOffsetP2] + pbyP0[nOffsetP3] - pbyP0[nOffsetP7]
                - 2 * pbyP0[nOffsetP6] - pbyP0[nOffsetP5]);

            long dxy = dx + dy;

            dst_y[x] = BYTE(dxy > 255 ? 255 : dxy);
        }
    }

    return 0;
}

void CippModules::SingleImageProcessingManage(VisionReusableMemory& memory, const Ipvm::Image8u& image,
    const Ipvm::Rect32s& rtROI, const ImageProcPara& procPara, Ipvm::Image8u& imageDst, BOOL bType)
{
    Ipvm::Image8u buffer1;
    Ipvm::Image8u buffer2;

    if (!memory.GetByteImage(buffer1, image.GetSizeX(), image.GetSizeY()) || (rtROI & Ipvm::Rect32s(buffer1)) != rtROI)
        return;
    if (!memory.GetByteImage(buffer2, image.GetSizeX(), image.GetSizeY()) || (rtROI & Ipvm::Rect32s(buffer2)) != rtROI)
        return;

    if (bType == 0)
    {
        ImageProcMultiplex(image, imageDst, buffer1, buffer2, rtROI, procPara.vecnCurProc, procPara.vecnCurProcParams);
    }
    else if (bType == 1)
    {
        ImageProcMultiplex(
            image, imageDst, buffer1, buffer2, rtROI, procPara.vecnAux1Proc, procPara.vecnAux1ProcParams);
    }
    else if (bType == 4)
    {
        ImageProcMultiplex(
            image, imageDst, buffer1, buffer2, rtROI, procPara.vecnAux2Proc, procPara.vecnAux2ProcParams);
    }
}

bool CippModules::GrayImageProcessingManage(VisionReusableMemory& memory, const Ipvm::Image8u* image, bool isRaw,
    const ImageProcPara& procPara, Ipvm::Image8u& imageDst, BOOL bType)
{
    return GrayImageProcessingManage(memory, image, isRaw, Ipvm::Rect32s(*image), procPara, imageDst, bType);
}

bool CippModules::GrayImageProcessingManage(VisionReusableMemory& memory, const Ipvm::Image8u* image, bool isRaw,
    const Ipvm::Rect32s& rtROI, const ImageProcPara& procPara, Ipvm::Image8u& imageDst, BOOL bType)
{
    Ipvm::Image8u buffer1;
    Ipvm::Image8u buffer2;
    Ipvm::Image8u buffer3;
    Ipvm::Image8u buffer4;

    const long nSizeX = image->GetSizeX();
    const long nSizeY = image->GetSizeY();

    if (!memory.GetByteImage(buffer1, nSizeX, nSizeY) || (rtROI & Ipvm::Rect32s(buffer1)) != rtROI)
        return false;
    if (!memory.GetByteImage(buffer2, nSizeX, nSizeY) || (rtROI & Ipvm::Rect32s(buffer2)) != rtROI)
        return false;
    if (!memory.GetByteImage(buffer3, nSizeX, nSizeY) || (rtROI & Ipvm::Rect32s(buffer3)) != rtROI)
        return false;
    if (!memory.GetByteImage(buffer4, nSizeX, nSizeY) || (rtROI & Ipvm::Rect32s(buffer4)) != rtROI)
        return false;

    Ipvm::Image8u aux1Image = procPara.m_aux1FrameID.getImage(isRaw);
    Ipvm::Image8u aux2Image = procPara.m_aux2FrameID.getImage(isRaw);

    if (bType == FALSE)
    {
        return ImageProcessingManage(memory, image, aux1Image, buffer1, buffer2, buffer3, buffer4, rtROI,
            procPara.vecnCurProc, procPara.vecnCurProcParams, procPara.vecnAux1Proc, procPara.vecnAux1ProcParams,
            procPara.vecnFirstCombineProc, procPara.vecnFirstCombineProcParams, procPara.nFirstCombineType, imageDst);
    }

    if (!GrayImageProcessingManage(memory, image, isRaw, rtROI, procPara, imageDst, FALSE))
    {
        return false;
    }

    if (!ImageProcessingManage(memory, &imageDst, aux2Image, buffer1, buffer2, buffer3, buffer4, rtROI,
            std::vector<short>(), std::vector<short>(), procPara.vecnAux2Proc, procPara.vecnAux2ProcParams,
            procPara.vecnSecondCombineProc, procPara.vecnSecondCombineProcParams, procPara.nSecondCombineType,
            imageDst))
    {
        return false;
    }

    return true;
}

bool CippModules::GrayImageProcessingManageForAlignUseRawInPane(VisionReusableMemory& memory,
    const Ipvm::Image8u* image, const Ipvm::Rect32s& rtPaneROI, const ImageProcPara& procPara, Ipvm::Image8u& imageDst,
    bool recursiveCall)
{
    Ipvm::Image8u buffer1;
    Ipvm::Image8u buffer2;
    Ipvm::Image8u buffer3;
    Ipvm::Image8u buffer4;

    const long nSizeX = image->GetSizeX();
    const long nSizeY = image->GetSizeY();

    if (!memory.GetByteImage(buffer1, nSizeX, nSizeY))
        return false;
    if (!memory.GetByteImage(buffer2, nSizeX, nSizeY))
        return false;
    if (!memory.GetByteImage(buffer3, nSizeX, nSizeY))
        return false;
    if (!memory.GetByteImage(buffer4, nSizeX, nSizeY))
        return false;

    Ipvm::Point32s2 ptLeftTop = rtPaneROI.TopLeft();
    Ipvm::Point32s2 ptOrigin(0, 0);
    Ipvm::Size32s2 szImage = Ipvm::Size32s2(rtPaneROI.Width(), rtPaneROI.Height());

    Ipvm::Rect32s rtCalcROI = Ipvm::Rect32s(imageDst);

    if (recursiveCall == true) //재귀 호출 시 Aux 1 관련 연산 처리하고 끝낸다
    {
        Ipvm::Image8u aux1Image;
        if (!memory.GetByteImage(aux1Image, nSizeX, nSizeY))
            return false;

        aux1Image.FillZero();
        CippModules::CopyValid(procPara.m_aux1FrameID.getImage(true), ptLeftTop, ptOrigin, szImage, aux1Image);

        return ImageProcessingManage(memory, image, aux1Image, buffer1, buffer2, buffer3, buffer4, rtCalcROI,
            procPara.vecnCurProc, procPara.vecnCurProcParams, procPara.vecnAux1Proc, procPara.vecnAux1ProcParams,
            procPara.vecnFirstCombineProc, procPara.vecnFirstCombineProcParams, procPara.nFirstCombineType, imageDst);
    }

    if (!GrayImageProcessingManageForAlignUseRawInPane(
            memory, image, rtPaneROI, procPara, imageDst, true)) //재귀호출을하는거고 재귀호출 시 Aux1 관련 연산을 한다.
    {
        return false;
    }

    //재귀 호출 작업이 끝난 후 Aux2 연산을 수행한다.
    Ipvm::Image8u aux2Image;
    if (!memory.GetByteImage(aux2Image, nSizeX, nSizeY))
        return false;

    aux2Image.FillZero();
    CippModules::CopyValid(procPara.m_aux2FrameID.getImage(true), ptLeftTop, ptOrigin, szImage, aux2Image);

    if (!ImageProcessingManage(memory, &imageDst, aux2Image, buffer1, buffer2, buffer3, buffer4, rtCalcROI,
            std::vector<short>(), std::vector<short>(), procPara.vecnAux2Proc, procPara.vecnAux2ProcParams,
            procPara.vecnSecondCombineProc, procPara.vecnSecondCombineProcParams, procPara.nSecondCombineType,
            imageDst))
    {
        return false;
    }

    return true;
}

/*
long RotateImage(	const Ipvm::Image8u& i_image,
					Ipvm::Image8u& o_image,
					const Ipvm::Rect32s &rtROI,
					const float fCenterX, const float fCenterY, const float fAngle,
					const float fShiftX, const float fShiftY)
{
	//----------------------------------------------------------------------------------
	// 이 함수를 완전히 없애려면 SrcRoi와 DstRoi를 따로 받는 함수가 IPVM에 있는 것이 좋겠다..
	//----------------------------------------------------------------------------------

	if (CheckRoi(rtROI, i_image.GetSizeX(), i_image.GetSizeY()) == FALSE) return -1;
	if (CheckRoi(rtROI, o_image.GetSizeX(), o_image.GetSizeY()) == FALSE) return -1;

	// Rotate by Body Center
	ItpiSize srcSize = { i_image.GetSizeX(), i_image.GetSizeY() };
	ItpiRect srcROI = { rtROI.m_left, rtROI.m_top, rtROI.Width(), rtROI.Height() };

	Ipvm::Rect32s rtDstROI(rtROI);
	rtDstROI.OffsetRect((long)(fShiftX), (long)(fShiftY));
	rtDstROI.InflateRect(1, 1, 1, 1);

	rtDstROI &= Ipvm::Rect32s(i_image);

	ItpiRect dstROI = { rtDstROI.m_left, rtDstROI.m_top, rtDstROI.Width(), rtDstROI.Height() };

	double xShift = fShiftX;
	double yShift = fShiftY;
	itpiAddRotateShift(fCenterX, fCenterY, fAngle, &xShift, &yShift);

	ItpStatus ret = itpiRotate_8u_C1R(i_image.GetMem(),
		srcSize,
		i_image.GetWidthBytes(),
		srcROI,
		o_image.GetMem(),
		o_image.GetWidthBytes(),
		dstROI,
		fAngle,
		xShift,
		yShift,
		ITPI_INTER_LINEAR);
	if (ret != itpStsNoErr)
	{
		return -1;
	}
	//}}

	return 0;
}
*/

BOOL CippModules::MakeAlignedGrayImage(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtBodyAlign,
    const Ipvm::Rect32s& rtPane, float fAngle, float fBodyCenterX, float fBodyCenterY, Ipvm::Image8u& o_alignedImage)
{
    if (i_image.GetSizeX() != o_alignedImage.GetSizeX())
        return FALSE;
    if (i_image.GetSizeY() != o_alignedImage.GetSizeY())
        return FALSE;

    // 원래 코드 : 회전, 이동만 하게 되어 있음.
    float fShiftX = (i_image.GetSizeX() - 1) * 0.5f - fBodyCenterX;
    float fShiftY = (i_image.GetSizeY() - 1) * 0.5f - fBodyCenterY;

    Ipvm::Rect32s rtBody(rtBodyAlign);

    long nInflateX = (rtPane.Width() - rtBody.Width()) / 2 + 1;
    long nInflateY = (rtPane.Height() - rtBody.Height()) / 2 + 1;

    nInflateX = max(nInflateX, 0);
    nInflateY = max(nInflateY, 0);

    rtBody.InflateRect(nInflateX, nInflateY, nInflateX, nInflateY);
    rtBody &= Ipvm::Rect32s(i_image);

    long nOffsetX, nOffsetY;
    nOffsetX = (long)(rtBody.CenterPoint().m_x - (i_image.GetSizeX() * 0.5f));
    nOffsetY = (long)(rtBody.CenterPoint().m_y - (i_image.GetSizeY() * 0.5f));
    Ipvm::Rect32s rtInitBinary = rtBody;
    rtInitBinary.InflateRect(15, 15, 15, 15);
    rtInitBinary.OffsetRect(-nOffsetX, -nOffsetY);

    rtInitBinary &= Ipvm::Rect32s(i_image);

    if (Ipvm::ImageProcessing::Fill(rtInitBinary, 0, o_alignedImage) != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    if (Ipvm::ImageProcessing::RotateLinearInterpolation(i_image, rtBody, Ipvm::Point32r2(fBodyCenterX, fBodyCenterY),
            -fAngle * ITP_RAD_TO_DEG, Ipvm::Point32r2(fShiftX, fShiftY), o_alignedImage)
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    //if (RotateImage(i_image, o_alignedImage, rtBody, fBodyCenterX, fBodyCenterY, fAngle*ITP_RAD_TO_DEG, fShiftX, fShiftY) != 0)
    //	return FALSE;;

    return TRUE;
}

BOOL CippModules::GetHistogramPeaks(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI,
    Ipvm::Image32s& o_histogram, long* o_pnPeakID, long* o_pnPeakCount, long& nPeakNum, long nCountThresh,
    long nLowThresh, long nHighThresh)
{
    //{{입력된 파라미터 검증
    if (o_pnPeakID == NULL || o_pnPeakCount == NULL)
    {
        nPeakNum = 0;
        return FALSE;
    }

    nLowThresh = max(nLowThresh, 0);
    nHighThresh = min(nHighThresh, 255);
    if (nLowThresh > nHighThresh || nPeakNum < (nHighThresh - nLowThresh)) //확인이 불가능한 범위이니까 아예 하지 말자
    {
        nPeakNum = 0;
        return FALSE;
    }

    Ipvm::Rect32s rtHisto = rtROI & Ipvm::Rect32s(i_image);

    long nWidth = rtHisto.Width();
    long nHeight = rtHisto.Height();

    if (nWidth < 0 || nHeight < 0) //확인이 불가능한 범위이니까 아예 하지 말자
    {
        nPeakNum = 0;
        return FALSE;
    }
    //}}

    //{{Data 초기화
    memset(o_pnPeakID, -1, nPeakNum * sizeof(long));
    memset(o_pnPeakCount, -1, nPeakNum * sizeof(long));
    //}}

    if (Ipvm::ImageProcessing::MakeHistogram(i_image, rtHisto, reinterpret_cast<unsigned int*>(o_histogram.GetMem()))
        != Ipvm::Status::e_ok)
    {
        return FALSE;
    }

    BOOL bReset = FALSE;
    long nCurSum = 0;
    long nCurMax = 0;
    long nCurMaxID = 0;
    long nCurCount = 0;
    long nPeak = 0;

    const auto* histogramValues = o_histogram.GetMem();

    for (long nIdx = nLowThresh + 1; nIdx < nHighThresh; nIdx++)
    {
        nCurSum = histogramValues[nIdx] + histogramValues[nIdx - 1] + histogramValues[nIdx + 1];

        if (nCurSum < nCountThresh && !bReset)
            continue;

        if (nCurSum < nCountThresh && bReset)
        {
            o_pnPeakID[nPeak] = nCurMaxID;
            o_pnPeakCount[nPeak] = nCurCount;
            bReset = FALSE;
            nCurMax = 0;
            nCurMaxID = 0;
            nCurCount = 0;
            nPeak++;
            continue;
        }

        if (nCurMax <= nCurSum)
        {
            if (nCurMax == nCurSum)
            {
                if (histogramValues[nIdx] < histogramValues[nIdx - 1]
                    || histogramValues[nIdx] < histogramValues[nIdx + 1])
                    continue;
            }
            nCurMax = nCurSum;
            nCurMaxID = nIdx;
            nCurCount = histogramValues[nIdx];
            bReset = TRUE;
        }
    }
    nPeakNum = nPeak;
    //}}

    return TRUE;
}

float CippModules::GetHistogramMassCenter(const Ipvm::Image32s& i_histogram, long nIdx, long nRange) //kircheis_3DEmpty
{
    if (i_histogram.GetSizeX() < 256 || nIdx < 0 || nIdx > 255)
        return -1.f;

    float fMassCenter = 0.f;
    long nStartIdx = nIdx - nRange;
    long nEndIdx = nIdx + nRange;
    nStartIdx = (long)max(nStartIdx, 0);
    nEndIdx = (long)min(nEndIdx, 255);

    long nDataNum = 0;
    double dSum = 0;
    const auto* histogramValues = i_histogram.GetMem();
    for (long i = nStartIdx; i <= nEndIdx; i++)
    {
        nDataNum += histogramValues[i];
        dSum += (double)(histogramValues[i] * i);
    }
    fMassCenter = (float)(dSum / (double)nDataNum);

    return fMassCenter;
}

bool CippModules::ConvertFloattoByteData(
    const Ipvm::Image32r& i_image, float fMultiplexData, const Ipvm::Rect32s& rtProc, Ipvm::Image8u& o_image)
{
    for (long y = rtProc.m_top; y < rtProc.m_bottom; y++)
    {
        auto* src_y = i_image.GetMem(0, y);
        auto* dst_y = o_image.GetMem(0, y);
        for (long x = rtProc.m_left; x < rtProc.m_right; x++)
        {
            if (src_y[x] == Ipvm::k_noiseValue32r)
            {
                dst_y[x] = 0;
                continue;
            }

            float fValue = (src_y[x] * fMultiplexData);
            if (fValue > 255.f || fValue < 0.f)
                fValue = 0.f;
            dst_y[x] = (BYTE)fValue;
        }
    }

    return true;
}

BOOL CippModules::TemplateMatching_SurfacePeak(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtSearchROI,
    Ipvm::Image32r& buffer, const Ipvm::Image8u& i_template, Ipvm::Point32r2& fptDstPeakPos, float& fDstMinScore,
    float& fGapScore)
{
    //-----------------------------------------------------------------------
    // Source Size 와 Template Size 검사
    //-----------------------------------------------------------------------
    if (rtSearchROI.Width() < i_template.GetSizeX() || rtSearchROI.Height() < i_template.GetSizeY())
    {
        fDstMinScore = 1.f;
        return FALSE;
    }

    if ((i_template.GetSizeX() <= 8) || (i_template.GetSizeY() <= 8))
    {
        fDstMinScore = 1.f;
        return FALSE;
    }

    int nDstWidth = rtSearchROI.Width() - i_template.GetSizeX() + 1;
    int nDstHeight = rtSearchROI.Height() - i_template.GetSizeY() + 1;
    if (nDstWidth <= 0 || nDstHeight <= 0)
    {
        return FALSE;
    }

    if (Ipvm::ImageProcessing::ProximitySqrDistanceNorm(i_image, i_template, rtSearchROI, buffer) != Ipvm::Status::e_ok)
    {
        fDstMinScore = 1.f;
        return FALSE;
    }

    //-----------------------------------------------------------------------
    // Matching ID 계산
    //-----------------------------------------------------------------------
    fDstMinScore = 1.f;

    long nMatchingX = 0;
    long nMatchingY = 0;
    BOOL bFind = FALSE;

    for (long iY = 0; iY < nDstHeight; iY++)
    {
        float* pfTemp = buffer.GetMem(0, iY);

        for (long iX = 0; iX < nDstWidth; iX++)
        {
            if (pfTemp[iX] < fDstMinScore)
            {
                fDstMinScore = pfTemp[iX];
                nMatchingX = iX;
                nMatchingY = iY;

                bFind = TRUE;
            }
        }
    }

    if (!bFind)
        return FALSE;

    //-------------------------------------------------------------------------------------------------------
    // Surface fitting - peak point 얻는다
    //-------------------------------------------------------------------------------------------------------
    const long nPeakDetectDataNum = 5;
    const long nMid = nPeakDetectDataNum / 2;

    Ipvm::Rect32s peakROI(int32_t(nMatchingX - nMid), int32_t(nMatchingY - nMid), int32_t(nMatchingX + nMid + 1),
        int32_t(nMatchingY + nMid + 1));

    if ((peakROI & Ipvm::Rect32s(0, 0, nDstWidth, nDstHeight)) != peakROI)
    {
        // 평면 Peak를 찾을 만한 충붕한 영역의 데이터가 없다
        // 최종 스케일링 복구후 완료
        fptDstPeakPos.m_x = static_cast<float>(nMatchingX + rtSearchROI.m_left);
        fptDstPeakPos.m_y = static_cast<float>(nMatchingY + rtSearchROI.m_top);

        fGapScore = 0.25; //  subpixeling 못하면 25점 감점
        return TRUE;
    }

    static const long nDataTotalNum = nPeakDetectDataNum * nPeakDetectDataNum;

    float fDstMaxScore = 0.f;
    float pfSrcZ[nDataTotalNum];
    long count = 0;
    for (long nY = peakROI.m_top; nY < peakROI.m_bottom; nY++)
    {
        float* buffer_y = buffer.GetMem(0, nY);

        for (long nX = peakROI.m_left; nX < peakROI.m_right; nX++, count++)
        {
            pfSrcZ[count] = 1.f - buffer_y[nX];
            if (buffer_y[nX] > fDstMaxScore)
                fDstMaxScore = buffer_y[nX];
        }
    }

    // Visibility 값이 클수록, 변별력이 있고 잘 찾았다고 할 수 있다.
    // fGapScore 를 정의함. visibility가 0.5 이상이면 충분히 변별력이 있다고 본다.
    fGapScore = min(1.f, 2 * (fDstMaxScore - fDstMinScore) / (fDstMaxScore + fDstMinScore));

    // 데이터 준비
    Ipvm::Point32r2 pfSrcXY[nDataTotalNum] = {Ipvm::Point32r2(-1, -1), Ipvm::Point32r2(-0.5, -1),
        Ipvm::Point32r2(0, -1), Ipvm::Point32r2(0.5, -1), Ipvm::Point32r2(1, -1), Ipvm::Point32r2(-1, -0.5),
        Ipvm::Point32r2(-0.5, -0.5), Ipvm::Point32r2(0, -0.5), Ipvm::Point32r2(0.5, -0.5), Ipvm::Point32r2(1, -0.5),
        Ipvm::Point32r2(-1, 0), Ipvm::Point32r2(-0.5, 0), Ipvm::Point32r2(0, 0), Ipvm::Point32r2(0.5, 0),
        Ipvm::Point32r2(1, 0), Ipvm::Point32r2(-1, 0.5), Ipvm::Point32r2(-0.5, 0.5), Ipvm::Point32r2(0, 0.5),
        Ipvm::Point32r2(0.5, 0.5), Ipvm::Point32r2(1, 0.5), Ipvm::Point32r2(-1, 1), Ipvm::Point32r2(-0.5, 1),
        Ipvm::Point32r2(0, 1), Ipvm::Point32r2(0.5, 1), Ipvm::Point32r2(1, 1)};

    // 평면 티칭을 진행한다. - Surface fitting
    float fPeakPosZ = 0.f;
    Ipvm::Point32r2 fptPeakPos = Ipvm::Point32r2(0.f, 0.f);

    double pfDstSurfaceCoef[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

    if (Ipvm::DataFitting::FitTo2ndOrderPolynomial3DSurface(nDataTotalNum, pfSrcXY, pfSrcZ, pfDstSurfaceCoef)
            != Ipvm::Status::e_ok
        || Ipvm::Mathematics::GetPeakPositionOf2ndOrderPolynomial3DSurface(pfDstSurfaceCoef, fptPeakPos, fPeakPosZ)
            != Ipvm::Status::e_ok)
    {
        fptPeakPos.m_x = 0.f;
        fptPeakPos.m_y = 0.f;
        fPeakPosZ = 0.f;
    }

    fptPeakPos.m_x *= 2.f;
    fptPeakPos.m_y *= 2.f;
    fptPeakPos.m_x += nMatchingX;
    fptPeakPos.m_y += nMatchingY;

    // 스케일링 복구
    fptDstPeakPos.m_x = fptPeakPos.m_x + rtSearchROI.m_left;
    fptDstPeakPos.m_y = fptPeakPos.m_y + rtSearchROI.m_top;

    return TRUE;
}

void CippModules::CopyValid(const Ipvm::Image8u& src, const Ipvm::Point32s2& srcPos, const Ipvm::Point32s2& dstPos,
    const Ipvm::Size32s2& roiSize, Ipvm::Image8u& dst)
{
    Ipvm::Rect32s srcRoi(srcPos.m_x, srcPos.m_y, srcPos.m_x + roiSize.m_w, srcPos.m_y + roiSize.m_h);
    Ipvm::Rect32s dstRoi(dstPos.m_x, dstPos.m_y, dstPos.m_x + roiSize.m_w, dstPos.m_y + roiSize.m_h);

    // 복사할 영역이 Src Image 영역을 넘지 않게 조절한다
    Ipvm::Rect32s src2 = srcRoi & Ipvm::Rect32s(src);
    Ipvm::Rect32s dst2;
    dst2.m_left = dstRoi.m_left + src2.m_left - srcRoi.m_left;
    dst2.m_top = dstRoi.m_top + src2.m_top - srcRoi.m_top;
    dst2.m_right = dstRoi.m_right + src2.m_right - srcRoi.m_right;
    dst2.m_bottom = dstRoi.m_bottom + src2.m_bottom - srcRoi.m_bottom;

    // 복사할 영역이 Dst Image 영역을 넘지 않게 조절한다
    Ipvm::Rect32s dst3 = dst2 & Ipvm::Rect32s(dst);
    Ipvm::Rect32s src3;
    src3.m_left = src2.m_left + dst3.m_left - dst2.m_left;
    src3.m_top = src2.m_top + dst3.m_top - dst2.m_top;
    src3.m_right = src2.m_right + dst3.m_right - dst2.m_right;
    src3.m_bottom = src2.m_bottom + dst3.m_bottom - dst2.m_bottom;

    auto error = Ipvm::ImageProcessing::Copy(src, src3, dst3, dst);
    VERIFY(error == Ipvm::Status::e_ok);
}

bool CippModules::MakeGray_BackgroundImageForAdaptiveThreshold(VisionReusableMemory& memory,
    const Ipvm::Image8u& i_image, const Ipvm::Image8u& imageMask, const Ipvm::Rect32s i_rtROI, const long i_nScale,
    const float i_fWindowSizeX_pxl, const float i_fWindowSizeY_pxl, Ipvm::Image8u& o_imageBackground) //kircheis_KOZ
{
    if ((Ipvm::Rect32s(i_image) & i_rtROI) != i_rtROI)
        return false;

    Ipvm::Point32r2 imageCenter(i_image.GetSizeX() * 0.5f, i_image.GetSizeY() * 0.5f);

    int scale = (int)min(1, max(8, i_nScale));

    if (scale <= 1)
    {
        Ipvm::Image32s imageIntengral;
        Ipvm::Image32s maskIntengral;

        if (!memory.GetInspLongImage(imageIntengral))
            return false;
        if (!memory.GetInspLongImage(maskIntengral))
            return false;

        if (Ipvm::ImageProcessing::IntegralInclusive(i_image, imageMask, i_rtROI, imageIntengral, maskIntengral)
            != Ipvm::Status::e_ok)
            return false;

        long win_size_x = long(i_fWindowSizeX_pxl + 0.5f);
        long win_size_y = long(i_fWindowSizeY_pxl + 0.5f);

        if (Ipvm::ImageProcessing::MakeBackgroundImage(
                imageIntengral, maskIntengral, i_rtROI, Ipvm::Size32s2(win_size_x, win_size_y), o_imageBackground)
            != Ipvm::Status::e_ok)
            return false;

        //Ipvm::Image8u(o_imageBackground, roi).SaveBmp(_T("D:\\original.bmp"));
        return true;
    }

    int scaleSizeX = i_rtROI.Width() / scale;
    int scaleSizeY = i_rtROI.Height() / scale;
    Ipvm::Rect32s roi_scale(0, 0, scaleSizeX, scaleSizeY);

    Ipvm::Image8u scale_image;
    Ipvm::Image8u scale_mask;
    Ipvm::Image8u scale_background;
    Ipvm::Image32s imageIntengral;
    Ipvm::Image32s maskIntengral;

    if (!memory.GetByteImage(scale_image, scaleSizeX, scaleSizeY))
        return false;
    if (!memory.GetByteImage(scale_mask, scaleSizeX, scaleSizeY))
        return false;
    if (!memory.GetByteImage(scale_background, scaleSizeX, scaleSizeY))
        return false;
    if (!memory.GetLongImage(imageIntengral, scaleSizeX, scaleSizeY))
        return false;
    if (!memory.GetLongImage(maskIntengral, scaleSizeX, scaleSizeY))
        return false;

    Ipvm::ImageProcessing::ResizeLinearInterpolation(Ipvm::Image8u(i_image, i_rtROI), scale_image);
    Ipvm::ImageProcessing::ResizeNearestInterpolation(Ipvm::Image8u(imageMask, i_rtROI), scale_mask);

    long windowSizeX = long((i_fWindowSizeX_pxl / (float)scale) + .5f);
    long windowSizeY = long((i_fWindowSizeY_pxl / (float)scale) + .5f);

    if (Ipvm::ImageProcessing::IntegralInclusive(scale_image, scale_mask, roi_scale, imageIntengral, maskIntengral)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    long win_size_x = long(windowSizeX + 0.5f);
    long win_size_y = long(windowSizeY + 0.5f);

    win_size_x = (long)min(roi_scale.Width(), win_size_x);
    win_size_y = (long)min(roi_scale.Height(), win_size_y);

    if (Ipvm::ImageProcessing::MakeBackgroundImage(
            imageIntengral, maskIntengral, roi_scale, Ipvm::Size32s2(win_size_x, win_size_y), scale_background)
        != Ipvm::Status::e_ok)
    {
        return false;
    }

    //Ipvm::Image8u(scale_background, roi_scale).SaveBmp(_T("D:\\scale.bmp"));
    //Ipvm::ImageProcessing::ResizeLinearInterpolation(scale_background, Ipvm::Image8u(o_imageBackground, i_rtROI));

    Ipvm::Image8u resizeImage(o_imageBackground, i_rtROI);
    Ipvm::ImageProcessing::ResizeLinearInterpolation(scale_background, resizeImage);

    return true;
}

bool CippModules::MakeGray_ThresholdedImageForAdaptiveThreshold(VisionReusableMemory& memory,
    const Ipvm::Image8u& i_image, const Ipvm::Image8u& i_imageBackground, const Ipvm::Image8u& i_maskImage,
    const Ipvm::Rect32s i_rtROI, const long i_nMinDarkContrast, const long i_nMinBrightContrast,
    const bool bDarkFillZero, Ipvm::Image8u& o_imageDarkDst, Ipvm::Image8u& o_imageBrightDst) //kircheis_KOZ
{
    Ipvm::Image8u imageTemp1;
    Ipvm::Image8u imageTemp2;
    Ipvm::Image8u imageTemp3;

    if (!memory.GetInspByteImage(imageTemp1))
        return false;
    if (!memory.GetInspByteImage(imageTemp2))
        return false;
    if (!memory.GetInspByteImage(imageTemp3))
        return false;

    BYTE minContrast_bright = BYTE(min(255, max(1, i_nMinBrightContrast)));
    BYTE minContrast_dark = BYTE(min(255, max(1, i_nMinDarkContrast)));

    int lutDark[256];
    int lutLevel[256];

    BYTE byDarkBackGroundGV = BACKGROUNDGV;
    BYTE byDarkForegroundGV = DARKOBJECTPIXELGV;
    if (bDarkFillZero == false)
    {
        byDarkBackGroundGV = DARKOBJECTPIXELGV;
        byDarkForegroundGV = BRIGHTOBJECTPIXELGV;
    }

    for (long n = 0; n < 256; n++)
    {
        lutLevel[n] = (BYTE)n;
        lutDark[n] = byDarkBackGroundGV;
    }

    for (long n = minContrast_dark; n < 256; n++)
        lutDark[n] = byDarkForegroundGV;

    Ipvm::ImageProcessing::Subtract(i_imageBackground, i_image, i_rtROI, 0, imageTemp1);
    Ipvm::ImageProcessing::ApplyLUT(imageTemp1, i_rtROI, lutDark, lutLevel, 256, o_imageDarkDst);

    Ipvm::ImageProcessing::Subtract(i_image, i_imageBackground, i_rtROI, 0, imageTemp2);
    Ipvm::ImageProcessing::BinarizeGreater(imageTemp2, i_rtROI, minContrast_bright, o_imageBrightDst);
    //Ipvm::ImageProcessing::Add(imageTemp1, i_rtROI, 0, o_imageDst);//얜 필요하면 밖에서 하면 된다.

    if (bDarkFillZero)
    {
        if (!SetInvalidPattern_with_constant(o_imageDarkDst, i_maskImage, i_rtROI, INGNOREPIXELGV))
            return false;
        if (!SetInvalidPattern_with_constant(o_imageBrightDst, i_maskImage, i_rtROI, INGNOREPIXELGV))
            return false;
    }

    return true;
}