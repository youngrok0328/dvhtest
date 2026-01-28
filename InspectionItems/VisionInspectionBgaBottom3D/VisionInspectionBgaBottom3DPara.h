#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ParaDefine.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class BallGroupInsp
{
public:
    CString BallCoplName;
    CString BallUnitCoplName;
    CString BallHeightName;
    CString BallWarpageName;
    CString BallUnitWarpageName;

public:
    void SetName(LPCTSTR group_id);
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

struct BallLevelParameter
{
    BallLevelParameter();

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Copy(BallLevelParameter Data);

    // Ball Search 용 파라메터
    double m_fBallSearchAreaPercent;

    // Ball Z 값 계산용 파라메터
    struct GatherBallZParameter
    {
        double m_calcAreaPercent;
        double m_validPixelHeightThreshold_Percent;
        double m_topPercentAverage;
        double m_maxValidPointsPercentOfSpecRadius;
        double m_dAcceptableInvalidBallCount; //kircheis_AIBC
    };

    GatherBallZParameter m_paraBallZ[long(enumGetherBallZType::END)];

    // SR 계산용 파라메터
    double m_fBallIgnorePercent;
    double m_fBallIgnoreExpandPercent;
    double m_fSRCalcAreaPercent;
};

class VisionInspectionBgaBottom3DPara
{
public:
    VisionInspectionBgaBottom3DPara(void);
    ~VisionInspectionBgaBottom3DPara(void);

    void Init();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db, const long ballDiameterTypeCount);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    std::vector<BallLevelParameter> m_parameters;

    double m_logicalPixelVisibilityLowerStartBound_ball;
    double m_logicalPixelVisibilityLowerEndBound_ball;
    double m_logicalPixelVisibilityUpperStartBound_ball;
    double m_logicalPixelVisibilityUpperEndBound_ball;
    double m_visibilityLowerBound_sr;

    //kircheis_Tan
    int m_nSR_Algorithm;
    double m_SR_topPercentAverage_Min;
    double m_SR_topPercentAverage_Max;

    double m_SR_IgnoreOffsetXum;
    double m_SR_IgnoreOffsetYum;

    int m_nInvalidateSinglePointData; //kircheis_loss3D //1이면 주변에 아무것도 없는 3D Point를 날리고, 0이면 쓴다.

    std::vector<BallGroupInsp> m_vecstrGroupInspName;

    //UnitCopl Group 관리
    std::vector<std::vector<long>> m_vec2UnitCoplGroup;
    std::vector<CString> m_vecstrGroupUnitCoplName;
    long m_nGroupUnitCoplCount;

    //spec 관리 변수
    std::vector<VisionInspectionSpec> m_vecVisionInspectionSpecs;
};
