#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "IlluminationTable.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CHardware_PCISync;
class SystemConfig;

//HDR_6_________________________________ Header body
//
class DPI_SYNCCONTROLLERDEF_API CSyncCalculator
{
public:
    CSyncCalculator();
    ~CSyncCalculator();

    static CSyncCalculator& GetInstance();

public:
    // 조명 보정 알고리즘 추가
    float CalcExposureTimeByGain(const long nFrame, const long nChannel, const float illum_ms,
        const bool adjustIntensity, const bool isVerification = false, const BYTE verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    float CalcExposureTimeByOnlyLine(const long nFrame, const long nChannel, const float illum_ms,
        const bool adjustIntensity, const bool isVerification = false, const BYTE verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    float CalcExposureTimeByCurve_Line(const long nFrame, const long nChannel, const float illum_ms,
        const bool adjustIntensity, const bool isVerification = false, const BYTE verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx = enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    float CalcExposureTimeByLinearPlus(const long nFrame, const long nChannel, const float illum_ms,
        const bool adjustIntensity, const bool isVerification = false, const BYTE verificationType = 0,
        const enSideVisionModule& i_eVisionModuleidx
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); //kircheis_IllumCalType

    bool CalibrateIllumination2D();
    bool CalibrateIlluminationSide();

    bool CalibrateIllumination2DExisting();
    bool CalibrateIllumination2DAll();
    bool CalibrateIlluminationMirror2D(); //SDY_DualCal 미러 보정용 함수
    bool CalibrateIlluminationSide2D();
    bool CalibrateIlluminationSideMirror2D();

    double m_illuminationGains[LED_ILLUM_CHANNEL_MAX]{};
    double m_illuminationGains_R[LED_ILLUM_CHANNEL_MAX]{}; //SDY_SideVision
    double m_illuminationGainsMirror[LED_ILLUM_CHANNEL_SIDE_DEFAULT]{}; //SDY_DualCal Mirror Gain
    double m_illuminationGainsMirror_R[LED_ILLUM_CHANNEL_SIDE_DEFAULT]{}; //SDY_SideVision

    bool IsCalibrated2DAll();
    bool IsCalibrated2DExisting();
    bool IsCalibrated2DMirror();

private:
    //SDY_Side Coef 생성 부분 함수화
    bool MakeIllumCalibrationCoef(CString strCoefPath, long ChannelMaxCount, IlluminationTable tableRef,
        IlluminationTable tableAll, long nSideVisionModule = SIDE_VISIONMODULE_FRONT);
    bool MakeIllumCalibrationMirrorCoef(CString strCoefPath, long ChannelMaxCount, IlluminationTable tableRef,
        IlluminationTable tableAll, long nSideVisionModule = SIDE_VISIONMODULE_FRONT);

    bool m_bIsCalibrated2DIllumAll;
    bool m_bIsCalibrated2DIllumExisting;
    bool m_bIsCalibrated2DIllumMirror; //SDY_DualCal 미러 보정가능 여부 확인용
    //}}

    void SetLongExposureCh(long nIRChID, long nUVChID); //kircheis_LongExp
    BOOL IsLongExposueCh(long nCh); //kircheis_LongExp

    // Implementation

private:
    //CHardware_PCISync* m_pHardware;

    float m_arfInputExpTime[10][16]; //kircheis_IllumHybrid
    float m_arfOutputExpTime[10][16]; //kircheis_IllumHybrid

    long m_nMaxChannelCount;
    long m_nUseChannelCount;
};
