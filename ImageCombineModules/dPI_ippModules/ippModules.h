#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageProcPara;
class VisionReusableMemory;
struct PI_RECT;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
const BYTE DARKOBJECTPIXELGV = 0;
const BYTE INGNOREPIXELGV = 253;
const BYTE BACKGROUNDGV = 254;
const BYTE BRIGHTOBJECTPIXELGV = 255;

enum
{
    COLOR_BLUE,
    COLOR_GREEN,
    COLOR_RED,
    COLOR_FULL
};

enum enumImageProc
{
    enumImageProc_Begin = 0,
    enumImageProc_Median = enumImageProc_Begin,
    enumImageProc_LowPass,
    enumImageProc_Sharpen,
    enumImageProc_Invert,
    enumImageProc_HistogramEqualization,
    enumImageProc_MorphologicalDilate,
    enumImageProc_MorphologicalErode,
    enumImageProc_AddSelf,
    enumImageProc_AddResult,
    enumImageProc_Offset,
    enumImageProc_Contrast,
    enumImageProc_End
};

class DPI_IPPMODULESDEF_API CippModules
{
public:
    //k 18.02.19 3Frame Combine을 사용할 경우 bType을 TRUE, 사용 안할 경우 FALSE
    //기본값은 TRUE로 설정
    static bool GrayImageProcessingManage(VisionReusableMemory& memory, const Ipvm::Image8u* image, bool isRaw,
        const ImageProcPara& procPara, Ipvm::Image8u& imageDst, BOOL bType = TRUE);
    static bool GrayImageProcessingManage(VisionReusableMemory& memory, const Ipvm::Image8u* image, bool isRaw,
        const Ipvm::Rect32s& rtROI, const ImageProcPara& procPara, Ipvm::Image8u& imageDst, BOOL bType = TRUE);
    static bool GrayImageProcessingManageForAlignUseRawInPane(VisionReusableMemory& memory, const Ipvm::Image8u* image,
        const Ipvm::Rect32s& rtPaneROI, const ImageProcPara& procPara, Ipvm::Image8u& imageDst,
        bool recursiveCall
        = false); //얘는 Align에서만 사용해야하는 전용 함수(Align에서만, 스티칭안하는 모델)다. Raw Image에서 Pane 영역만을 사용.
    static void SingleImageProcessingManage(VisionReusableMemory& memory, const Ipvm::Image8u& image,
        const Ipvm::Rect32s& rtROI, const ImageProcPara& procPara, Ipvm::Image8u& imageDst, BOOL bType);

    static float GetMeanInIntensityRange_BYTE(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI, BYTE bLow,
        BYTE bHigh, long nMinPxlCount,
        long& o_nDarkPixelCnt); //kircheis_20161206//bLow<= Inten <= nHigh 인 녀석들의 AVG
    static BOOL Binarize(const Ipvm::Image8u& i_image, Ipvm::Image8u& o_image, const Ipvm::Rect32s& rtRoi,
        long nThresholdValue, BOOL isReverse);

    static float GetMaxRange_Float(const Ipvm::Image32r i_image, float fMin, float fMax, Ipvm::Rect32s rtROI);

    static BOOL SetInvalidPattern_with_constant(
        Ipvm::Image8u& io_image, const Ipvm::Image8u& i_vaildPattern, const Ipvm::Rect32s& rtROI, const BYTE nConst);

    static long EdgeDetect_Magnitude_Sobel(
        const Ipvm::Image8u& i_imageSrc, const Ipvm::Rect32s& rtROI, Ipvm::Image8u& o_imageDst);
    static float GetHistogramMassCenter(const Ipvm::Image32s& i_histogram, long nIdx, long nRange);

    static BOOL TemplateMatching_SurfacePeak(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtSearchROI,
        Ipvm::Image32r& buffer, const Ipvm::Image8u& i_template, Ipvm::Point32r2& fptDstPeakPos, float& fDstMinScore,
        float& fGapScore);

    static BOOL GetHistogramPeaks(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI, Ipvm::Image32s& o_histogram,
        long* o_pnPeakID, long* o_pnPeakCount, long& nPeakNum, long nCountThresh = 0, long nLowThresh = 0,
        long nHighThresh = 255);

    static float GetMeanNotUseZero_BYTE(
        const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtROI, long& o_nValidDataNum, float& o_fValidDataRatio);

    static BOOL MakeAlignedGrayImage(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& rtBodyAlign,
        const Ipvm::Rect32s& rtPane, float fAngle, float fBodyCenterX, float fBodyCenterY,
        Ipvm::Image8u& o_alignedImage);
    static long GetLineData(
        const Ipvm::Image8u& i_image, float x0, float y0, float x1, float y1, float fCos, float fSin, BYTE* pLineOut);
    static bool ConvertFloattoByteData(
        const Ipvm::Image32r& i_image, float fMultiplexData, const Ipvm::Rect32s& rtProc, Ipvm::Image8u& o_image);
    static void CopyValid(const Ipvm::Image8u& src, const Ipvm::Point32s2& srcPos, const Ipvm::Point32s2& dstPos,
        const Ipvm::Size32s2& roiSize, Ipvm::Image8u& dst);

    //{{//kircheis_KOZ //추후 Mark Threhold에도 Adaptive Threshold가 들어가야되니 공용으로 뺀다
    static bool MakeGray_BackgroundImageForAdaptiveThreshold(VisionReusableMemory& memory, const Ipvm::Image8u& i_image,
        const Ipvm::Image8u& imageMask, const Ipvm::Rect32s i_rtROI, const long i_nScale,
        const float i_fWindowSizeX_pxl, const float i_fWindowSizeY_pxl, Ipvm::Image8u& o_imageBackground);
    static bool MakeGray_ThresholdedImageForAdaptiveThreshold(VisionReusableMemory& memory,
        const Ipvm::Image8u& i_image, const Ipvm::Image8u& i_imageBackground, const Ipvm::Image8u& i_maskImage,
        const Ipvm::Rect32s i_rtROI, const long i_nMinDarkContrast, const long i_nMinBrightContrast,
        const bool bDarkFillZero, Ipvm::Image8u& o_imageDarkDst, Ipvm::Image8u& o_imageBrightDst);
    //}}
};
