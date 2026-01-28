#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
struct S3DMergeResult
{
    std::vector<FPI_RECT> vecsfrtRawDevice;

    std::vector<Ipvm::LineSeg32r> vecsRoughMatchLine;

    std::vector<FPI_RECT> vecsfrtSplit; // Stiching 조각 영역
    std::vector<FPI_RECT> vecsfrtSplitVirtualDevice; //  조각 영역기준으로 가상의 Package 영역

    std::vector<Ipvm::Rect32s> vecrtSpecObj;
    std::vector<Ipvm::Rect32s> vecrtSearchObj;
    std::vector<Ipvm::Rect32s> vecrtObj;
    std::vector<Ipvm::Point32r2> vecfptObj;

    std::vector<Ipvm::Rect32s> vecrt2DMatch;
    std::vector<Ipvm::LineSeg32r> vecsSplitCrossLine;

    std::vector<Ipvm::Rect32s> vecrtOverlap;
    std::vector<Ipvm::Rect32s> vecrtMatchOverlap;

    std::vector<Ipvm::Rect32s> vecrtCalc;

    std::vector<Ipvm::Rect32s> vecrtMeasure;
    std::vector<Ipvm::Rect32s> vecrtMeasured;

    std::vector<Ipvm::LineSeg32r> vecsCrossLine;
    std::vector<Ipvm::Rect32s> vecrtPane;

    std::vector<Ipvm::Rect32s> vecrtDevice;

    void Clear()
    {
        vecsfrtRawDevice.clear();
        vecsRoughMatchLine.clear();

        vecsfrtSplit.clear();
        vecsfrtSplitVirtualDevice.clear();

        vecrtSpecObj.clear();
        vecrtSearchObj.clear();
        vecrtObj.clear();
        vecfptObj.clear();

        vecrt2DMatch.clear();
        vecsSplitCrossLine.clear();

        vecrtOverlap.clear();
        vecrtMatchOverlap.clear();

        vecrtCalc.clear();
        vecrtMeasure.clear();
        vecrtMeasured.clear();

        vecsCrossLine.clear();
        vecrtPane.clear();

        vecrtDevice.clear();
    }
};
