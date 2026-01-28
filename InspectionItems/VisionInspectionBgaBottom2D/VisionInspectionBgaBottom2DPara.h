#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspFrameIndex.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
enum BallEdgeCountMode //kircheis_BallFlux
{
    enumBallEdgeFastMode,
    enumBallEdgeDetailMode,
};

enum BallEdgeReSearchMode //kircheis_BallFlux
{
    enumBallEdgeReSearchNotUse,
    enumBallEdgeReSearchUse,
};

enum BallRoughAlignMethod //kircheis_LKF
{
    enumBallRoughAlignMethod_DefaultCircleFitting = 0,
    enumBallRoughAlignMethod_UsePeakCenterOfBallInCoaxialImage,
};

class Ball2DGroupInsp
{
public:
    CString BallOffsetXName;
    CString BallOffsetYName;
    CString BallOffsetRName;
    CString BallGridOffsetXName;
    CString BallGridOffsetYName;
    CString BallWidthName;
    CString BallQualityName;
    CString BallContrastName;
    CString BallEllipticityName;

public:
    void SetName(LPCTSTR groupID);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

struct BallLevelParameter
{
    BallLevelParameter();

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Copy(BallLevelParameter Data);

    // Ball Width
    long m_nBallWidthMethod; //kircheis_20160602
    long m_nBallEdgeCountMode; //kircheis_BallFlux
    double m_widthRatio;
    double m_ballWidthRoughSearchLengthRatio;
    double m_ballWidthDetailSearchLengthRatio; //kircheis_BallFlux
    double m_ballWidthSearchIgnoreLengthRatio; //kk
    long m_nBallEdgeReSearchMode; //kircheis_BallFlux
    double m_ballReSearchLengthRatioInner; //kircheis_BallFlux
    double m_ballReSearchLengthRatioOuter; //kircheis_BallFlux
    double m_ballReSearchNoiseFilterThresholdRatio; //kircheis_BallFluxModify
    long m_nRoughAlignMethod; //kircheis_LKF

    // Ball Quality
    long m_qualityThresholdValue;
    long m_qualityCheckMethod; // (Ball Quality Check)
    double m_qualityCheckOuterVoid; // (Ball Quality Check)
    double m_qualityCheckInnerVoid; // (Ball Quality Check)
    double m_qualityCheckSamplingInterval; // (Ball Quality Check)
    double m_qualityCheckRangeMin; // (Normal Method)
    double m_qualityCheckRangeMax; // (Normal Method)
    double m_qualityRingThicknessRate; // (Ring Thickness)
    double m_qualityRingRadius_px;
};

class VisionInspectionBgaBottom2DPara
{
public:
    enum enumAlignResultUseCenter
    {
        enumAlignResultUseCenter_BodyAlign,
        enumAlignResultUseCenter_PadAlign,
    };
    enum enumCombineMode
    {
        enumCombineMode_Default, // Oblique - Coaxial
        enumCombineMode_Oblique,
        enumCombineMode_Self, // Apply Sobel Edge Filter
        enumCombineMode_InvertCoaxial, // kircheis_Shiny
        enumCombineMode_ReformedInvertCoaxial, // kircheis_InvertCoax
        enumCombineMode_ReformedCoaxial, // kircheis_BKShiny
        enumCombineMode_ImprovedDefault, //kircheis_ImproveBall2D
    };

    enum enumQualityCombineMode
    {
        enumQualityCombineMode_Default, // Oblique - Coaxial
        enumQualityCombineMode_Oblique,
        enumQualityCombineMode_Self, // Apply Sobel Edge Filter
    };

    enum BallQualityCheckMethod
    {
        enumNormalMethod,
        enumRingThickness,
    };

    VisionInspectionBgaBottom2DPara(VisionProcessing& parent);
    ~VisionInspectionBgaBottom2DPara(void);

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    //Common Param
    long m_nImageCombineMode;
    VisionInspFrameIndex m_obliqueImageIndex;
    VisionInspFrameIndex m_coaxialImageIndex;

    // Ball Quality CommonParam
    VisionInspFrameIndex m_qualityObliqueImageIndex;
    VisionInspFrameIndex m_qualityCoaxialImageIndex;

    //Group Param
    std::vector<BallLevelParameter> m_parameters;

    // Ball Quality
    long m_qualityCombineMode;
    // 2nd Inspection
    BOOL m_use2ndInspection; // 2nd 검사 사용 유무 (딥러닝)
    CString m_str2ndInspCode; // 딥러닝 모델과 매칭을 위한 코드

    std::vector<Ball2DGroupInsp> m_vecstrGroupInspName;

    //spec 관리 변수
    std::vector<VisionInspectionSpec> m_vecVisionInspectionSpecs;
};
