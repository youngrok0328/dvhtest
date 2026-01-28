//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SyncCalculator.h"

//CPP_2_________________________________ This project's headers
#include "IlluminationTable.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define INTERPOLATION_RANGE 1

//CPP_7_________________________________ Implementation body
//
CSyncCalculator::CSyncCalculator()
    //: m_pHardware(SystemConfig::GetInstance().IsHardwareExist() ? new CHardware_PCISync((SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP || SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_TR || SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP), SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE) : nullptr)
    : m_arfInputExpTime{0}
    , m_arfOutputExpTime{0}
    , m_illuminationGains{0}
    , m_nMaxChannelCount(0)
    , m_nUseChannelCount(0)
    , m_bIsCalibrated2DIllumMirror(FALSE)
{
    //BOOL bIsNGRV = SystemConfig::GetInstance().IsVisionTypeNGRV(); //kircheis_LongExp

    // 2D Vision 혹은 NGRV Vision 일 때만 조명 보정 세팅을 실시한다 - JHB_NGRV_2021.05.25
    static const long nVisionType = CAST_LONG(SystemConfig::GetInstance().GetVisionType()); //kircheis_SWIR

    if (nVisionType == VISIONTYPE_2D_INSP || nVisionType == VISIONTYPE_NGRV_INSP || nVisionType == VISIONTYPE_SWIR_INSP
        || nVisionType == VISIONTYPE_TR) //kircheis_SWIR
    {
        CalibrateIllumination2D();
    }
    else if (nVisionType == VISIONTYPE_SIDE_INSP)
    {
        CalibrateIlluminationSide();
    }

    m_nMaxChannelCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();
    m_nUseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
}

CSyncCalculator::~CSyncCalculator()
{
    //delete m_pHardware;
}

CSyncCalculator& CSyncCalculator::GetInstance()
{
    static CSyncCalculator singleton;

    return singleton;
}

float CSyncCalculator::CalcExposureTimeByGain(const long nFrame, const long nChannel, const float illum_ms,
    const bool adjustIntensity, const bool isVerification, const BYTE verificationType,
    const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(isVerification);
    UNREFERENCED_PARAMETER(verificationType);

    auto& systemConfig = SystemConfig::GetInstance();
    //조명 보정을 사용하는지, 노출이 0보다 큰지, 조명 보정을 타입을 확인하고 아니면 입력값을 그대로 리턴
    if (adjustIntensity == false || illum_ms <= 0.f || systemConfig.m_nIlluminationCalType != IllumCalType_Gain)
        return illum_ms;

    float duration_ms = illum_ms;
    float filluminationGains = CAST_FLOAT(m_illuminationGains[nChannel]);

    //SDY_Side Vision의 경우 Cur에 해당하는 데이터를 바꾸어준다.
    if (i_eVisionModuleidx == SIDE_VISIONMODULE_REAR)
    {
        filluminationGains = CAST_FLOAT(m_illuminationGains_R[nChannel]);
    }

    duration_ms = CAST_FLOAT(illum_ms * filluminationGains);

    return duration_ms;
}

float CSyncCalculator::CalcExposureTimeByOnlyLine(const long nFrame, const long nChannel, const float illum_ms,
    const bool adjustIntensity, const bool isVerification, const BYTE verificationType,
    const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(isVerification);
    UNREFERENCED_PARAMETER(verificationType);

    auto& systemConfig = SystemConfig::GetInstance();
    //조명 보정을 사용하는지, 노출이 0보다 큰지, 조명 보정을 타입을 확인하고 아니면 입력값을 그대로 리턴
    if (adjustIntensity == false || illum_ms <= 0.f || systemConfig.m_nIlluminationCalType != IllumCalType_OnlyLine)
        return illum_ms;
    //필요한 조명 보정 파라미터를 가져온다
    float filluminationGains = CAST_FLOAT(m_illuminationGains[nChannel]);
    auto& vecIllumLineEqRef = systemConfig.m_vecIllumFittingRef;
    auto& vecIllumLineEqCur = systemConfig.m_vecIllumFittingCur;

    //SDY_Side Vision의 경우 Cur에 해당하는 데이터를 바꾸어준다.
    if (i_eVisionModuleidx == SIDE_VISIONMODULE_REAR)
    {
        filluminationGains = CAST_FLOAT(m_illuminationGains_R[nChannel]);
        vecIllumLineEqCur = systemConfig.m_vecIllumFittingCur_R;
    }

    auto lineEqRef = vecIllumLineEqRef[nChannel];
    auto lineEqCur = vecIllumLineEqCur[nChannel];

    float duration_ms = illum_ms;

    float fDestIntensity = (-lineEqRef.m_a / lineEqRef.m_b) * duration_ms - (lineEqRef.m_c / lineEqRef.m_b);
    duration_ms = (-lineEqCur.m_b / lineEqCur.m_a) * fDestIntensity - (lineEqCur.m_c / lineEqCur.m_a);
    if (duration_ms <= 0.f)
        duration_ms = (float)(illum_ms * filluminationGains);

    return duration_ms;
}

float CSyncCalculator::CalcExposureTimeByCurve_Line(const long nFrame, const long nChannel, const float illum_ms,
    const bool adjustIntensity, const bool isVerification, const BYTE verificationType,
    const enSideVisionModule& i_eVisionModuleidx)
{
    UNREFERENCED_PARAMETER(nFrame);
    UNREFERENCED_PARAMETER(isVerification);
    UNREFERENCED_PARAMETER(verificationType);

    auto& systemConfig = SystemConfig::GetInstance();
    //조명 보정을 사용하는지, 노출이 0보다 큰지, 조명 보정을 타입을 확인하고 아니면 입력값을 그대로 리턴
    if (adjustIntensity == false || illum_ms <= 0.f || systemConfig.m_nIlluminationCalType != IllumCalType_Curve_Line)
        return illum_ms;

    //{{ //kircheis_WB
    float duration_ms = illum_ms;
    float filluminationGains = CAST_FLOAT(m_illuminationGains[nChannel]);
    auto& vecIllumLineEqRef = systemConfig.m_vecIllumFittingRef;
    auto& vecIllumLineEqCur = systemConfig.m_vecIllumFittingCur;
    auto& vecIllumCurveEqRef = systemConfig.m_vecIllumCurveFittingRef; //kircheis_IllumCal
    auto& vecIllumCurveEqCur = systemConfig.m_vecIllumCurveFittingCur; //kircheis_IllumCal
    auto& vecIllumCurveEqCurReverse = systemConfig.m_vecIllumCurveFittingCurReverse; //kircheis_IllumCal
    long nCurveEqRefNum = (long)vecIllumCurveEqRef.size(); //kircheis_IllumCal
    long nCurveEqCurNum = (long)vecIllumCurveEqCurReverse.size(); //kircheis_IllumCal
    BOOL bUseCurveEq = ((nCurveEqRefNum > 0) && (nCurveEqRefNum == nCurveEqCurNum)); //kircheis_IllumCal
    static BOOL bUseIllumCalType2 = systemConfig.m_bUseIllumCalType2;
    float fLowestX = 0.f;
    //}}

    //SDY_Side Vision의 경우 Cur에 해당하는 데이터를 바꾸어준다.
    if (i_eVisionModuleidx == SIDE_VISIONMODULE_REAR)
    {
        filluminationGains = CAST_FLOAT(m_illuminationGains_R[nChannel]);
        vecIllumLineEqCur = systemConfig.m_vecIllumFittingCur_R;
        vecIllumCurveEqCur = systemConfig.m_vecIllumCurveFittingCur_R; //kircheis_IllumCal
    }

    auto lineEqRef = vecIllumLineEqRef[nChannel];
    auto lineEqCur = vecIllumLineEqCur[nChannel];

    auto curveEqRef = vecIllumCurveEqRef[nChannel];
    auto curveEqCur = vecIllumCurveEqCur[nChannel];
    auto curveEqCurReverse = vecIllumCurveEqCurReverse[nChannel];
    //{{//kircheis_ImproveIllumCal
    //bool bIsInvalidExpTimeInCurve = false;
    auto& vecCalibrationAuxData = systemConfig.m_vecCalibrationAuxData;
    bool bIsCurveValid
        = (vecCalibrationAuxData.size() > 0 && vecCalibrationAuxData.size() == vecIllumCurveEqRef.size());
    fLowestX = (float)((-curveEqRef.m_b) / (curveEqRef.m_c * 2.));
    //}}

    float fDestIntensity = (-lineEqRef.m_a / lineEqRef.m_b) * illum_ms - (lineEqRef.m_c / lineEqRef.m_b);
    if (bUseCurveEq && bIsCurveValid && illum_ms >= fLowestX) //kircheis_IllumCal
    {
        float fThreshExpTime = vecCalibrationAuxData[nChannel].m_fThreshExpTime_Ref;
        float fThreshIntensity = vecCalibrationAuxData[nChannel].m_fThreshIntensity_Cur;
        float fCalcDurationTime_ms = -1.f;
        if (illum_ms <= fThreshExpTime)
            fDestIntensity
                = (float)((illum_ms * illum_ms * curveEqRef.m_c) + (illum_ms * curveEqRef.m_b) + curveEqRef.m_a);

        if (fDestIntensity <= fThreshIntensity)
        {
            double dInRoot = (4. * curveEqCur.m_c * fDestIntensity) - (4. * curveEqCur.m_c * curveEqCur.m_a)
                + (curveEqCur.m_b * curveEqCur.m_b);
            if (dInRoot > 0.)
                fCalcDurationTime_ms = (float)((sqrt(dInRoot) - curveEqCur.m_b) / (2. * curveEqCur.m_c));

            if (fCalcDurationTime_ms < 0.f)
                fCalcDurationTime_ms = (float)((fDestIntensity * fDestIntensity * curveEqCurReverse.m_c)
                    + (fDestIntensity * curveEqCurReverse.m_b) + curveEqCurReverse.m_a);

            duration_ms = fCalcDurationTime_ms;

            if (illum_ms > duration_ms && illum_ms < 1.f && duration_ms < 0.2f)
                duration_ms = illum_ms;

            //duration_ms = (illum_ms + duration_ms) * 0.5f;
        }
        else
            duration_ms = (-lineEqCur.m_b / lineEqCur.m_a) * fDestIntensity - (lineEqCur.m_c / lineEqCur.m_a);
    }
    else if (fDestIntensity > 2.f) //목표 Intensity가 너무 낮거나 음수면 보정을 안쓰는게 낫다
        duration_ms = (-lineEqCur.m_b / lineEqCur.m_a) * fDestIntensity - (lineEqCur.m_c / lineEqCur.m_a);

    return duration_ms;
}

float CSyncCalculator::CalcExposureTimeByLinearPlus(const long nFrame, const long nChannel, const float illum_ms,
    const bool adjustIntensity, const bool isVerification, const BYTE verificationType,
    const enSideVisionModule& i_eVisionModuleidx) //kircheis_IllumCalType
{
    auto& systemConfig = SystemConfig::GetInstance();
    //조명 보정을 사용하는지, 노출이 0보다 큰지, 하이브리드타입 조명 보정을 사용하는지 확인하고 아니면 입력값을 그대로 리턴해버리자
    if (adjustIntensity == false || illum_ms <= 0.f || systemConfig.m_nIlluminationCalType != IllumCalType_Linear_Plus)
        return illum_ms;

    static const bool bIsSideVision = (bool)(systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);
    const bool bIsRearModule = (bIsSideVision && i_eVisionModuleidx == SIDE_VISIONMODULE_REAR);

    //필요한 조명 보정 파라미터를 가져온다
    float filluminationGains = CAST_FLOAT(m_illuminationGains[nChannel]);
    auto& vecIllumLineEqRef = systemConfig.m_vecIllumFittingRef;
    auto& vecIllumLineEqCur
        = (bIsRearModule == false) ? systemConfig.m_vecIllumFittingCur : systemConfig.m_vecIllumFittingCur_R;
    long nRefLineNum = (long)vecIllumLineEqRef.size();
    long nCurLineNum = (long)vecIllumLineEqCur.size();

    auto& vecIllumLineEqRefMirror = systemConfig.m_vecIllumFittingRefMirror; //SDY_DualCal
    auto& vecIllumLineEqCurMirror = (bIsRearModule == false) ? systemConfig.m_vecIllumFittingCurMirror
                                                             : systemConfig.m_vecIllumFittingCurMirror_R; //SDY_DualCal

    //SDY_Side Vision의 경우 Cur에 해당하는 데이터를 바꾸어준다.
    if (i_eVisionModuleidx == SIDE_VISIONMODULE_REAR)
    {
        filluminationGains = CAST_FLOAT(m_illuminationGains_R[nChannel]);
    }

    float duration_ms = illum_ms;
    float fGrayExposureTime = illum_ms;
    float fMirrorExposureTime = illum_ms;

    //사용하려는 채널이 해당 프레임의 첫 채널인지 확인 한다.
    bool bIsFirstCh = true;
    long nUsedFirstCh = 0;
    for (long nCh = 0; nCh < nChannel; nCh++)
    {
        if (m_arfInputExpTime[nFrame][nCh] > 0.f)
        {
            bIsFirstCh = false;
            nUsedFirstCh = nCh;
            break;
        }
    }

    if (nRefLineNum <= nChannel || nCurLineNum <= nChannel)
    {
        return duration_ms;
    }

    //첫번째 채널이면 라인보정 결과 리턴하고 끝.
    if (bIsFirstCh)
    {
        //auto& vecCalibrationAuxData = systemConfig.m_vecCalibrationAuxData; //kircheis_illumLUT

        //BOOL bIsCoaxial = (nChannel >= 11 && nChannel <= 12);
        //BOOL bIsHybridWithLCG_Coax = (systemConfig.m_nIlluminationCalType == IllumCalType_Linear_Plus && bIsCoaxial);
        //BOOL bIsHybridWithLCG_NoCoax = (systemConfig.m_nIlluminationCalType == IllumCalType_Linear_Plus && !bIsCoaxial);

        auto lineEqRef = vecIllumLineEqRef[nChannel];
        auto lineEqCur = vecIllumLineEqCur[nChannel];
        float fRefA = (-lineEqRef.m_a / lineEqRef.m_b);
        float fRefB = (-lineEqRef.m_c / lineEqRef.m_b);
        float fCurAr = (-lineEqCur.m_b / lineEqCur.m_a);
        float fCurBr = (-lineEqCur.m_c / lineEqCur.m_a);

        float fDestIntensity = fRefA * illum_ms + fRefB;
        fGrayExposureTime = fCurAr * fDestIntensity + fCurBr;

        if (systemConfig.m_fUseCoaxialLowCut > 0
            && illum_ms < systemConfig
                    .m_fUseCoaxialLowCut) //SDY_DualCal Low Cut이 설정되어 있을때 해당 GV에서 컷해서 0ms를 내보낸다.
        {
            fGrayExposureTime = 0.f;
        }
        else if (fGrayExposureTime < 0.f
            && systemConfig.m_fUseCoaxialLowCut == 0) //SDY_DualCal Low Cut이 0일때 밝깃값이 0 이하면 0ms를 내보낸다.
        {
            fGrayExposureTime = 0.f;
        }
        else if (fGrayExposureTime < 0.f
            && systemConfig.m_fUseCoaxialLowCut < 0) //SDY_DualCal Low cut이 0 이하 (ex)-1) 이면 기존 방식대로 내보낸다.
        {
            fGrayExposureTime = (float)(illum_ms * filluminationGains / 10);
        }
        //SDY_DualCal 로우컷이 고정값일 경우 버그 발생해서 예외상황일 경우 이전과 같이 연산하도록 수정
        if (fGrayExposureTime < 0.f)
        {
            fGrayExposureTime = (float)(illum_ms * filluminationGains / 10);
        }

        if ((isVerification == true)
            && (verificationType
                == IllumMirrorCalType_GrayTargetOnly)) // SDY_DualCal Verification 과정에서 Graytarget일 경우 Gray 연산 결과를 전달한다.
        {
            duration_ms = fGrayExposureTime;
            return duration_ms;
        }

        // SDY_DualCal 동축의 경우에만 Mirror 보정을 사용한다.
        if ((systemConfig.GetIllumType(nChannel) == IllumType_Coaxial)
            && (systemConfig.m_nCoaxialIllumMirrorCalType > IllumMirrorCalType_GrayTargetOnly)
            && (m_bIsCalibrated2DIllumMirror == TRUE))
        {
            //SDY_DualCal Mirror Target 보정
            auto lineEqRefMirror = vecIllumLineEqRefMirror[nChannel];
            auto lineEqCurMirror = vecIllumLineEqCurMirror[nChannel];
            float fRefAMirror = (-lineEqRefMirror.m_a / lineEqRefMirror.m_b);
            float fRefBMirror = (-lineEqRefMirror.m_c / lineEqRefMirror.m_b);
            float fCurArMirror = (-lineEqCurMirror.m_b / lineEqCurMirror.m_a);
            float fCurBrMirror = (-lineEqCurMirror.m_c / lineEqCurMirror.m_a);

            float fDestIntensityMirror = fRefAMirror * illum_ms + fRefBMirror;
            fMirrorExposureTime = fCurArMirror * fDestIntensityMirror + fCurBrMirror;

            if (illum_ms < systemConfig.m_fUseCoaxialLowCut
                && systemConfig.m_fUseCoaxialLowCut
                    > 0) //SDY_DualCal Low Cut이 설정되어 있을때 해당 GV에서 컷해서 0ms를 내보낸다.
            {
                fMirrorExposureTime = 0.f;
            }
            else if (fMirrorExposureTime < 0.f
                && systemConfig.m_fUseCoaxialLowCut
                    == 0) //SDY_DualCal Low Cut이 0일때 밝깃값이 0 이하면 0ms를 내보낸다.
            {
                fMirrorExposureTime = 0.f;
            }
            else if (fMirrorExposureTime < 0.f
                && systemConfig.m_fUseCoaxialLowCut
                    < 0) //SDY_DualCal Low cut이 0 이하 (ex)-1) 이면 기존 방식대로 내보낸다.
            {
                fMirrorExposureTime = (float)(illum_ms * filluminationGains / 10);
            }
            // SDY_DualCal 로우컷이 고정값일 경우 버그 발생해서 예외상황일 경우 이전과 같이 연산하도록 수정
            if (fMirrorExposureTime < 0.f)
            {
                fMirrorExposureTime = (float)(illum_ms * filluminationGains / 10);
            }

            if ((isVerification == true)
                && (verificationType
                    == IllumMirrorCalType_MirrorOnly)) // SDY_DualCal Verification 과정에서 Graytarget일 경우 Gray 연산 결과를 전달한다.
            {
                duration_ms = fMirrorExposureTime;
                return duration_ms;
            }

            // DualCalibration을 사용할때는 Mirror Target에서 얻은 데이터를 같이 활용한다.
            if ((systemConfig.m_nCoaxialIllumMirrorCalType == IllumMirrorCalType_DualCalibration)
                && (m_bIsCalibrated2DIllumMirror == TRUE))
            {
                float fInterpolationRate
                    = (systemConfig.m_fInterpolationStart + INTERPOLATION_RANGE - illum_ms) / INTERPOLATION_RANGE;
                fInterpolationRate = min(1, max(0, fInterpolationRate));

                // 두가지의 조명을 비율로 배합
                if (systemConfig.m_bUsingDualCalInterpolationType == TRUE)
                {
                    duration_ms
                        = (fGrayExposureTime * (1 - fInterpolationRate)) + (fMirrorExposureTime * fInterpolationRate);
                }

                // interpolation 모드가 아니므로 해당 영역을 기준으로 나눠서 사용한다.
                else
                {
                    duration_ms
                        = systemConfig.m_fInterpolationStart < illum_ms ? fGrayExposureTime : fMirrorExposureTime;
                }
            }

            else
            {
                duration_ms = fMirrorExposureTime; // DualCal을 사용하지 않을 경우 그냥 Mirror 결과를 보낸다.
            }
        }

        // 동축이 아닌 경우에는 그냥 Gray 방식으로 계산한 1st Frame용 데이터를 출력한다.
        else
        {
            duration_ms = fGrayExposureTime;
        }

        return duration_ms;
    }

    //첫번째가 아니면 Stack 방식으로 광량을 누적 시키는 방식으로 노출 시간을 계산한다.
    float fLastInstensity = 0.f;
    float fLastExpTime = 0.f;
    for (long nCh = nUsedFirstCh; nCh <= nChannel; nCh++)
    {
        if (m_arfInputExpTime[nFrame][nCh] <= 0.f) //노출 시간이 0이면, 계산할 필요가 없다.
            continue;

        auto lineEqRef = vecIllumLineEqRef[nCh];
        auto lineEqCur = vecIllumLineEqCur[nCh];
        float fRefA = (-lineEqRef.m_a / lineEqRef.m_b);
        float fRefB = (-lineEqRef.m_c / lineEqRef.m_b);
        float fRefAr = (-lineEqRef.m_b / lineEqRef.m_a);
        float fRefBr = (-lineEqRef.m_c / lineEqRef.m_a);
        float fCurA = (-lineEqCur.m_a / lineEqCur.m_b);
        float fCurB = (-lineEqCur.m_c / lineEqCur.m_b);
        float fCurAr = (-lineEqCur.m_b / lineEqCur.m_a);
        float fCurBr = (-lineEqCur.m_c / lineEqCur.m_a);

        if (fLastInstensity
            == 0.f) //첫번째 노출 채널 일 때 예외 상황으로 계산된 노출 시간이 음수 일수도 있다. 이를 위한 예외 처리를 위해 분기
        {
            float fDestIntensity = fRefA * m_arfInputExpTime[nFrame][nCh] + fRefB;
            duration_ms = fCurAr * fDestIntensity + fCurBr;
            if (duration_ms < 0.f)
                duration_ms = (float)(m_arfInputExpTime[nFrame][nCh] * filluminationGains);

            fLastExpTime = m_arfInputExpTime[nFrame][nCh];
            fLastInstensity = fCurA * duration_ms + fCurB;
            continue;
        }

        float fStartExpRef = fRefAr * fLastInstensity
            + fRefBr; //마지막 Target Intensity를 기준으로 현재 Setting하는 채널의 Ref 노출 시간 확인
        float fDestExpTimeRef = fStartExpRef + m_arfInputExpTime[nFrame][nCh]; //Ref 기준으로 누적 노출 시간 계산
        float fIntensityGap = (fRefA * fDestExpTimeRef + fRefB)
            - fLastInstensity; //계산된 누적 노출 시간으로 최종  Intensity를 계산한 후 시작 Intensity와의 광량 차이를 계산.

        float fCurStartExp = fCurAr * fLastInstensity + fCurBr; //현 장비에서 시작 Exp Time을 계산
        float fEndIntensity = fLastInstensity + fIntensityGap; //현 채널의 누적 Intensity 계산
        float fCurEndExp = fCurAr * fEndIntensity + fCurBr; //누적 Intensity를 내기위한 노출 시간 계산
        fGrayExposureTime
            = fCurEndExp - fCurStartExp; // 누적 노출 시간에서 시작 노출 시간을 빼서 현 채널 만의 순수 노출 시간 계산

        fLastExpTime = fDestExpTimeRef; //다음 채널 연산을 위해 Ref의 마지막 누적 노출 시간 저장
        fLastInstensity = fEndIntensity; //다음 채널 연산을 위해 마지막 누적 Intensity 저장

        // SDY_MIRROR 동축의 경우에만 Mirror 보정 데이터를 덮어 씌워서 적용한다.
        if ((systemConfig.GetIllumType(nCh) == IllumType_Coaxial)
            && (systemConfig.m_nCoaxialIllumMirrorCalType > IllumMirrorCalType_GrayTargetOnly)
            && (m_bIsCalibrated2DIllumMirror == TRUE)) // 해당 부분에 사용가능 여부 확인 필수
        {
            auto lineEqRefMirror = vecIllumLineEqRefMirror[nChannel];
            auto lineEqCurMirror = vecIllumLineEqCurMirror[nChannel];
            float fMirrorRefA = (-lineEqRefMirror.m_a / lineEqRefMirror.m_b);
            float fMirrorRefB = (-lineEqRefMirror.m_c / lineEqRefMirror.m_b);
            float fMirrorRefAr = (-lineEqRefMirror.m_b / lineEqRefMirror.m_a);
            float fMirrorRefBr = (-lineEqRefMirror.m_c / lineEqRefMirror.m_a);
            //float fMirrorCurA = (-lineEqCurMirror.m_a / lineEqCurMirror.m_b);
            //float fMirrorCurB = (-lineEqCurMirror.m_c / lineEqCurMirror.m_b);
            float fMirrorCurAr = (-lineEqCurMirror.m_b / lineEqCurMirror.m_a);
            float fMirrorCurBr = (-lineEqCurMirror.m_c / lineEqCurMirror.m_a);

            fStartExpRef = fMirrorRefAr * fLastInstensity
                + fMirrorRefBr; //마지막 Target Intensity를 기준으로 현재 Setting하는 채널의 Ref 노출 시간 확인
            fDestExpTimeRef = fStartExpRef + m_arfInputExpTime[nFrame][nCh]; //Ref 기준으로 누적 노출 시간 계산
            fIntensityGap = (fMirrorRefA * fDestExpTimeRef + fMirrorRefB)
                - fLastInstensity; //계산된 누적 노출 시간으로 최종  Intensity를 계산한 후 시작 Intensity와의 광량 차이를 계산.

            fCurStartExp = fMirrorCurAr * fLastInstensity + fMirrorCurBr; //현 장비에서 시작 Exp Time을 계산
            fEndIntensity = fLastInstensity + fIntensityGap; //현 채널의 누적 Intensity 계산
            fCurEndExp = fMirrorCurAr * fEndIntensity + fMirrorCurBr; //누적 Intensity를 내기위한 노출 시간 계산
            fMirrorExposureTime = fCurEndExp
                - fCurStartExp; // 누적 노출 시간에서 시작 노출 시간을 빼서 현 채널 만의 순수 노출 시간 계산

            fLastExpTime = fDestExpTimeRef; //다음 채널 연산을 위해 Ref의 마지막 누적 노출 시간 저장
            fLastInstensity = fEndIntensity; //다음 채널 연산을 위해 마지막 누적 Intensity 저장

            if ((systemConfig.m_nCoaxialIllumMirrorCalType == IllumMirrorCalType_DualCalibration)
                && (m_bIsCalibrated2DIllumMirror == TRUE))
            {
                float fInterpolationRate
                    = (systemConfig.m_fInterpolationStart + INTERPOLATION_RANGE - illum_ms) / INTERPOLATION_RANGE;
                fInterpolationRate = min(1, max(0, fInterpolationRate));

                // 두가지의 조명을 비율로 배합
                if (systemConfig.m_bUsingDualCalInterpolationType == TRUE)
                {
                    duration_ms
                        = (fGrayExposureTime * (1 - fInterpolationRate)) + (fMirrorExposureTime * fInterpolationRate);
                }

                // interpolation 모드가 아니므로 해당 영역을 기준으로 나눠서 사용한다.
                else
                {
                    duration_ms
                        = systemConfig.m_fInterpolationStart < illum_ms ? fGrayExposureTime : fMirrorExposureTime;
                }
            }
            else
            {
                duration_ms = fMirrorExposureTime;
            }
        }
        else
        {
            duration_ms = fGrayExposureTime;
        }
    }

    return duration_ms;
}

bool CSyncCalculator::CalibrateIllumination2D()
{
    m_bIsCalibrated2DIllumAll = m_bIsCalibrated2DIllumExisting = false;
    m_bIsCalibrated2DIllumMirror = false; //SDY_DualCal

    CString strRef = DynamicSystemPath::get(DefineFile::IllumTableRefTable);
    CString strCur = DynamicSystemPath::get(DefineFile::IllumTableTable);
    CString strRefAll = DynamicSystemPath::get(DefineFile::IllumTableRefTableAll);
    CString strCurAll = DynamicSystemPath::get(DefineFile::IllumTableTableAll);
    CString strRefMirror = DynamicSystemPath::get(DefineFile::IllumTableRefTableAllMirror); //SDY_DualCal
    CString strCurAllMirror = DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror); //SDY_DualCal

    CFileFind fileFind;
    BOOL bIsExistingRef = fileFind.FindFile(strRef);
    BOOL bIsExistingCur = fileFind.FindFile(strCur);
    BOOL bIsRefAll = fileFind.FindFile(strRefAll);
    BOOL bIsCurAll = fileFind.FindFile(strCurAll);
    BOOL bIsRefMirror = fileFind.FindFile(strRefMirror); //SDY_DualCal
    BOOL bIsCurAllMirror = fileFind.FindFile(strCurAllMirror); //SDY_DualCal

    if (bIsRefAll && bIsCurAll)
    {
        m_bIsCalibrated2DIllumAll = CalibrateIllumination2DAll();

        //SDY_DualCal 미러 보정 가능한지 여부에 대해서 먼저 확인한다. 미러 모드가 켜있고 테이블을 가져올 수 없는 경우에는 로그만 띄워주고 Gray 보정으로 작동하게 만든다.
        if (bIsRefMirror && bIsCurAllMirror)
        {
            m_bIsCalibrated2DIllumMirror = CalibrateIlluminationMirror2D();
        }

        return m_bIsCalibrated2DIllumAll;
    }
    else if (bIsExistingRef && bIsExistingCur)
    {
        m_bIsCalibrated2DIllumExisting = CalibrateIllumination2DExisting();

        //SDY_DualCal 미러 보정 가능한지 여부에 대해서 먼저 확인한다. 미러 모드가 켜있고 테이블을 가져올 수 없는 경우에는 로그만 띄워주고 Gray 보정으로 작동하게 만든다.
        if (bIsRefMirror && bIsCurAllMirror)
        {
            m_bIsCalibrated2DIllumMirror = CalibrateIlluminationMirror2D();
        }

        return m_bIsCalibrated2DIllumExisting;
    }

    return false;
}

//SDY_SideVision 사이드 비전 Calibration의 경우 기존 보정과 이질적이라서 함수를 따로 가져감
bool CSyncCalculator::CalibrateIlluminationSide()
{
    //m_bIsCalibrated2DIllumAll = m_bIsCalibrated2DIllumExisting = false;
    //m_bIsCalibrated2DIllumMirror = false; //SDY_DualCal

    CString strRefAll = DynamicSystemPath::get(DefineFile::IllumTableRefTableAll);
    CString strRefMirror = DynamicSystemPath::get(DefineFile::IllumTableRefTableAllMirror); //SDY_DualCal

    //SDY_Side Vision Calibration
    CString strIsCurIncludedCurve_F = DynamicSystemPath::get(DefineFile::IllumTableTableAll_F);
    CString strIsCurIncludedCurveMirror_F = DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror_F);

    CString strIsCurIncludedCurve_R = DynamicSystemPath::get(DefineFile::IllumTableTableAll_R);
    CString strIsCurIncludedCurveMirror_R = DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror_R);

    CFileFind fileFind;
    BOOL bIsRefAll = fileFind.FindFile(strRefAll);
    BOOL bIsRefMirror = fileFind.FindFile(strRefMirror); //SDY_DualCal

    //SDY_Side Vision Calibration
    BOOL bIsCurIncludedCurve_F = fileFind.FindFile(strIsCurIncludedCurve_F);
    BOOL bIsCurIncludedCurveMirror_F = fileFind.FindFile(strIsCurIncludedCurveMirror_F);

    //BOOL bIsCurIncludedCurve_R = fileFind.FindFile(strIsCurIncludedCurve_R);
    BOOL bIsCurIncludedCurveMirror_R = fileFind.FindFile(strIsCurIncludedCurveMirror_R);

    if (bIsRefAll && bIsCurIncludedCurve_F)
    {
        m_bIsCalibrated2DIllumAll = CalibrateIlluminationSide2D();

        //SDY_DualCal 미러 보정 가능한지 여부에 대해서 먼저 확인한다. 미러 모드가 켜있고 테이블을 가져올 수 없는 경우에는 로그만 띄워주고 Gray 보정으로 작동하게 만든다.
        if (bIsRefMirror && bIsCurIncludedCurveMirror_F && bIsCurIncludedCurveMirror_R)
        {
            m_bIsCalibrated2DIllumMirror = CalibrateIlluminationSideMirror2D();
        }

        return m_bIsCalibrated2DIllumAll;
    }

    CalibrateIllumination2DAll(); //kircheis_IllumCal //Error MSG 발생용으로 호출하는거다.
    CalibrateIlluminationSide2D();
    return false;
}

bool CSyncCalculator::CalibrateIllumination2DExisting() //kircheis_IllumCal
{
    BOOL isIpis380B = (SystemConfig::GetInstance().m_nHandlerType == HANDLER_TYPE_380BRIDGE);

    for (long channel = 0; channel < m_nMaxChannelCount; channel++)
    {
        m_illuminationGains[channel] = 1.;
    }

    CString errorDesc;

    IlluminationTable tableRef;

    if (tableRef.Load(DynamicSystemPath::get(DefineFile::IllumTableRefTable), errorDesc) == false)
    {
        if (!isIpis380B /* && m_pHardware != nullptr*/)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            ::AfxMessageBox(errorDesc, MB_ICONERROR | MB_OK);
        }
        return false;
    }

    IlluminationTable table;

    if (table.Load(DynamicSystemPath::get(DefineFile::IllumTableTable), errorDesc) == false)
    {
        if (!isIpis380B)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, errorDesc);
        }
        return false;
    }

    table.CalcIlluminationGain(tableRef, m_illuminationGains);

    CFile file;
    if (file.Open(DynamicSystemPath::get(DefineFolder::Temp) + _T("IlluminationGain.csv"),
            CFile::modeCreate | CFile::modeWrite))
    {
        CArchive ar(&file, CArchive::store);

        CString str;

        for (auto& gain : m_illuminationGains)
        {
            str.Format(_T("%.5lf\r\n"), gain);
            ar.WriteString(str);
        }
    }

    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();
    auto& systemConfig = SystemConfig::GetInstance(); //kircheis_IllumCal

    systemConfig.ClearIllumCoeffNormal();

    CFile file2;
    if (file2.Open(DynamicSystemPath::get(DefineFolder::System) + _T("IlluminationTableCoef.csv"),
            CFile::modeCreate | CFile::modeWrite))
    {
        CArchive ar(&file2, CArchive::store);
        auto& vecIllumLineEqRef = systemConfig.m_vecIllumFittingRef;
        auto& vecIllumLineEqCur = systemConfig.m_vecIllumFittingCur;

        CString str;

        str.Format(_T(",Coeff,Ref,Ref-1,Target,Target-1\r\n"));
        ar.WriteString(str);

        for (long channel = 0; channel < nChannelMaxCount; channel++)
        {
            auto lineEqRef = vecIllumLineEqRef[channel];
            auto lineEqCur = vecIllumLineEqCur[channel];

            float fRefLineA = (-lineEqRef.m_a / lineEqRef.m_b);
            float fRefLineB = -(lineEqRef.m_c / lineEqRef.m_b);
            float fRefLineRevA = (1 / fRefLineA);
            float fRefLineRevB = -(fRefLineB / fRefLineA);
            float fCurLineA = (-lineEqCur.m_a / lineEqCur.m_b);
            float fCurLineB = -(lineEqCur.m_c / lineEqCur.m_b);
            float fCurLineRevA = (1 / fCurLineA);
            float fCurLineRevB = -(fCurLineB / fCurLineA);

            systemConfig.m_vecfCoeffA_Ref.push_back(fRefLineA); //kircheis_IllumCalBug
            systemConfig.m_vecfCoeffB_Ref.push_back(fRefLineB);
            systemConfig.m_vecfCoeffA_Cur.push_back(fCurLineA); //kircheis_IllumCalBug
            systemConfig.m_vecfCoeffB_Cur.push_back(fCurLineB);

            str.Format(
                _T("CH%02d,Line A, %f, %f, %f, %f\r\n"), channel + 1, fRefLineA, fRefLineRevA, fCurLineA, fCurLineRevA);
            ar.WriteString(str);
            str.Format(_T(",Line B, %f, %f, %f, %f\r\n"), fRefLineB, fRefLineRevB, fCurLineB, fCurLineRevB);
            ar.WriteString(str);
        }
        ar.Close();
        file2.Close();
    }

    return true;
}

bool CSyncCalculator::CalibrateIllumination2DAll() //kircheis_IllumCal
{
    BOOL isIpis380B = (SystemConfig::GetInstance().m_nHandlerType == HANDLER_TYPE_380BRIDGE);

    for (long channel = 0; channel < m_nMaxChannelCount; channel++)
    {
        m_illuminationGains[channel] = 1.;
    }

    CString errorDesc;

    IlluminationTable tableRef;

    if (tableRef.Load(DynamicSystemPath::get(DefineFile::IllumTableRefTableAll), errorDesc) == false)
    {
        if (!isIpis380B /* && m_pHardware != nullptr*/)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            ::AfxMessageBox(errorDesc, MB_ICONERROR | MB_OK);
        }
        return false;
    }

    IlluminationTable table;

    if (table.Load(DynamicSystemPath::get(DefineFile::IllumTableTableAll), errorDesc) == false)
    {
        if (!isIpis380B)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, errorDesc);
        }
        return false;
    }

    table.CalcIlluminationGain(tableRef, m_illuminationGains);

    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();
    auto& systemConfig = SystemConfig::GetInstance(); //kircheis_IllumCal

    systemConfig.ClearIllumCoeffNormal();

    // Coef 만들기
    CString strCoefPath;
    strCoefPath.Format(DynamicSystemPath::get(DefineFolder::System) + _T("IlluminationTableCoef.csv"));
    MakeIllumCalibrationCoef(strCoefPath, nChannelMaxCount, tableRef, table);

    return true;
}

bool CSyncCalculator::MakeIllumCalibrationCoef(CString strCoefPath, long ChannelMaxCount, IlluminationTable tableRef,
    IlluminationTable tableAll, long nSideVisionModule) //SDY_DualCal
{
    auto& systemConfig = SystemConfig::GetInstance(); //kircheis_IllumCal

    static const bool bIsSideVision = (bool)(systemConfig.GetVisionType() == VISIONTYPE_SIDE_INSP);
    const bool bIsRearModule = (bIsSideVision && nSideVisionModule == SIDE_VISIONMODULE_REAR);

    CFile file;
    if (file.Open(strCoefPath, CFile::modeCreate | CFile::modeWrite))
    {
        CArchive ar(&file, CArchive::store);
        auto& vecIllumLineEqRef = systemConfig.m_vecIllumFittingRef;
        auto& vecIllumCurveEqRef = systemConfig.m_vecIllumCurveFittingRef; //kircheis_IllumCal
        auto& vecIllumCurveEqRefReverse = systemConfig.m_vecIllumCurveFittingRefReverse; //kircheis_IllumCal
        auto& vecIllumLineEqCur
            = (bIsRearModule == false) ? systemConfig.m_vecIllumFittingCur : systemConfig.m_vecIllumFittingCur_R;
        auto& vecIllumCurveEqCur = (bIsRearModule == false)
            ? systemConfig.m_vecIllumCurveFittingCur
            : systemConfig.m_vecIllumCurveFittingCur_R; //kircheis_IllumCal
        auto& vecIllumCurveEqCurReverse = (bIsRearModule == false)
            ? systemConfig.m_vecIllumCurveFittingCurReverse
            : systemConfig.m_vecIllumCurveFittingCurReverse_R; //kircheis_IllumCal

        //{{//kircheis_ImproveIllumCal
        auto& vecCalibrationAuxData = systemConfig.m_vecCalibrationAuxData;
        vecCalibrationAuxData.resize(ChannelMaxCount);
        long nRefIntenNum(0), nCurIntenNum(0);
        float fCurIntensity(0.f), fCurveCalcInten(0.f), fLineCalcInten(0.f);
        float fExpTime(0.f), fCalcCurveExpTime(0.f), fPeakOfCurrentReverseCurveEquation(0.f);
        //}}

        CString str;

        str.Format(_T(",Coeff,Ref,Ref-1,Target,Target-1, RefExpBoundary,TargetIntenBoundary\r\n"));
        ar.WriteString(str);

        for (long channel = 0; channel < ChannelMaxCount; channel++)
        {
            auto lineEqRef = vecIllumLineEqRef[channel];
            auto lineEqCur = vecIllumLineEqCur[channel];
            auto curveEqRef = vecIllumCurveEqRef[channel];
            auto curveEqRefReverse = vecIllumCurveEqRefReverse[channel];
            auto curveEqCur = vecIllumCurveEqCur[channel];
            auto curveEqCurReverse = vecIllumCurveEqCurReverse[channel];

            float fRefLineA = (-lineEqRef.m_a / lineEqRef.m_b);
            float fRefLineB = -(lineEqRef.m_c / lineEqRef.m_b);
            float fRefLineRevA = (1 / fRefLineA);
            float fRefLineRevB = -(fRefLineB / fRefLineA);
            float fCurLineA = (-lineEqCur.m_a / lineEqCur.m_b);
            float fCurLineB = -(lineEqCur.m_c / lineEqCur.m_b);
            float fCurLineRevA = (1 / fCurLineA);
            float fCurLineRevB = -(fCurLineB / fCurLineA);
            systemConfig.m_vecfCoeffA_Ref.push_back(fRefLineA); //kircheis_IllumCalBug
            systemConfig.m_vecfCoeffB_Ref.push_back(fRefLineB);

            if (nSideVisionModule == SIDE_VISIONMODULE_FRONT)
            {
                systemConfig.m_vecfCoeffA_Cur.push_back(fCurLineA); //kircheis_IllumCalBug
                systemConfig.m_vecfCoeffB_Cur.push_back(fCurLineB);
            }
            else
            {
                systemConfig.m_vecfCoeffA_Cur_SideRear.push_back(fCurLineA); //kircheis_IllumCalBug
                systemConfig.m_vecfCoeffB_Cur_SideRear.push_back(fCurLineB);
            }

            //{{//kircheis_ImproveIllumCal
            //Reference의 Intensity와 곡선방정식과 직선방정식으로 계산된 Intensity와의 편차를 각각 계산한 후 각 편차의 오차가 역전되는 위치를 구한다.
            //각 편차의 오차가 역전되는 위치는 곡선 영역과 직선 영역이 바뀌는 부분이다.
            nRefIntenNum = (long)tableRef.m_intensities_ms[channel].size();
            for (long nIdx = nRefIntenNum - 1; nIdx >= 0; nIdx--)
            {
                fExpTime = (float)tableRef.m_illuminationTimes_ms[nIdx];
                fCurIntensity = (float)tableRef.m_intensities_ms[channel][nIdx];
                if (fCurIntensity <= 0.f)
                    continue;

                fCurveCalcInten
                    = (float)(curveEqRef.m_c * fExpTime * fExpTime + curveEqRef.m_b * fExpTime + curveEqRef.m_a);
                fLineCalcInten = fRefLineA * fExpTime + fRefLineB;
                if (fabs(fCurIntensity - fCurveCalcInten) < fabs(fCurIntensity - fLineCalcInten))
                {
                    vecCalibrationAuxData[channel].m_fThreshExpTime_Ref = fExpTime;
                    break;
                }
            }

            //현 Probe의 Intensity와 곡선방정식과 직선방정식으로 계산된 Intensity와의 편차를 각각 계산한 후 각 편차의 오차가 역전되는 위치를 구한다.
            nCurIntenNum = (long)tableAll.m_intensities_ms[channel].size();
            //아래의 변수는 곡선방정식 역함수의 결과가 사용 가능한 마지막 지점으로 이차 방정식의 미분방정식의 결과가 0인 지점이다.
            //계산되는 노출 시간이 이전까지는 상승하다가 아래 변수 값이상인 지점부터는 하강한다. (역동작)
            fPeakOfCurrentReverseCurveEquation = (float)(-curveEqCurReverse.m_b / (2.f * curveEqCurReverse.m_c));

            for (long nIdx = nCurIntenNum - 1; nIdx >= 0; nIdx--)
            {
                fExpTime = (float)tableAll.m_illuminationTimes_ms[nIdx];
                fCurIntensity = (float)tableAll.m_intensities_ms[channel][nIdx];
                if (fCurIntensity <= 0.f || fCurIntensity > fPeakOfCurrentReverseCurveEquation)
                    continue;
                //Reference와는 다르게 현 Probe는 역함수가 중요한 역할을 한다.  (Ref방정식 -> 현 Probe방정식의 역함수 -> 결과)
                //그러나 곡선방정식의 역함수는 완벽한 역함수가 아닌 관계로 (직선의 방정식과 다르게 2차방정식은 역함수계산이 어려워 X,Y축을 바꿔 피팅)
                //역함수의 결과를 원래의 함수에 입력할 경우 특정 구간을 벗어나면 그 결과가 일치하지 않는다.  => if(x > N)  f( f^-1(x) ) != x
                //그렇기 때문에 이경우에는 노출 시간을 역함수로 계산하여계산된 노출시간을 곡선 방정식에 대입하여 유효성을 판별토록한다.
                //즉, 곡선방정식의 역함수가 유효한 지점이( f( f^-1(x) ) 의 결과가  x에 근사치) 현 Probe의 곡선과 직선의 경계가 된다.
                fCalcCurveExpTime = (float)(curveEqCurReverse.m_c * fCurIntensity * fCurIntensity
                    + curveEqCurReverse.m_b * fCurIntensity + curveEqCurReverse.m_a);
                fCurveCalcInten = (float)(curveEqCur.m_c * fCalcCurveExpTime * fCalcCurveExpTime
                    + curveEqCur.m_b * fCalcCurveExpTime + curveEqCur.m_a);
                fLineCalcInten = fCurLineA * fExpTime + fCurLineB;
                if (fabs(fCurIntensity - fCurveCalcInten) < fabs(fCurIntensity - fLineCalcInten))
                {
                    vecCalibrationAuxData[channel].m_fThreshIntensity_Cur = fCurIntensity;
                    break;
                }
            }

            str.Format(_T("CH%02d,Line A, %f, %f, %f, %f, %f, %f\r\n"), channel + 1, fRefLineA, fRefLineRevA, fCurLineA,
                fCurLineRevA, vecCalibrationAuxData[channel].m_fThreshExpTime_Ref, fCurIntensity);
            ar.WriteString(str);
            str.Format(_T(",Line B, %f, %f, %f, %f\r\n"), fRefLineB, fRefLineRevB, fCurLineB, fCurLineRevB);
            ar.WriteString(str);
            str.Format(_T(",Curve A, %f, %f, %f, %f\r\n"), curveEqRef.m_c, curveEqRefReverse.m_c, curveEqCur.m_c,
                curveEqCurReverse.m_c);
            ar.WriteString(str);
            str.Format(_T(",Curve B, %f, %f, %f, %f\r\n"), curveEqRef.m_b, curveEqRefReverse.m_b, curveEqCur.m_b,
                curveEqCurReverse.m_b);
            ar.WriteString(str);
            str.Format(_T(",Curve C, %f, %f, %f, %f\r\n\n"), curveEqRef.m_a, curveEqRefReverse.m_a, curveEqCur.m_a,
                curveEqCurReverse.m_a);
            ar.WriteString(str);
        }
        //}}
        ar.Close();
        file.Close();
    }

    return true;
}

//SDY_DualCal 미러 테이블을 받는 것은 따로 결과를 던져주는 것이 듀얼 보정 on/off에 좀더 유리해서 기존 TABLE과 분리하였다.
bool CSyncCalculator::CalibrateIlluminationMirror2D() //SDY_DualCal
{
    BOOL isIpis380B = (SystemConfig::GetInstance().m_nHandlerType == HANDLER_TYPE_380BRIDGE);
    BOOL bIsDualCal = TRUE;

    for (long channel = 0; channel < m_nMaxChannelCount; channel++)
    {
        m_illuminationGainsMirror[channel] = 1.;
    }

    CString errorDesc;

    IlluminationTable tableRefMirror;

    if (tableRefMirror.Load(DynamicSystemPath::get(DefineFile::IllumTableRefTableAllMirror), errorDesc) == false)
    {
        if (!isIpis380B /* && m_pHardware != nullptr*/)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            ::AfxMessageBox(errorDesc, MB_ICONERROR | MB_OK);
        }
        bIsDualCal
            = FALSE; //SDY_DualCal MirrorRef 및 MirrorTable 중 하나라도 테이블이 없으면 DualCal을 사용하지 못하게 하였다.
    }

    IlluminationTable tableMirror;

    if (tableMirror.Load(DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror), errorDesc) == false)
    {
        if (!isIpis380B)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, errorDesc);
        }
        bIsDualCal
            = FALSE; //SDY_DualCal MirrorRef 및 MirrorTable 중 하나라도 테이블이 없으면 DualCal을 사용하지 못하게 하였다.
    }

    tableMirror.CalcIlluminationGain(tableRefMirror,
        m_illuminationGainsMirror); //BUG: m_illuminationGainsMirror크기는 10인데, CalcIlluminationGain()는 16을 기대함

    long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    //long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();

    auto& systemConfig = SystemConfig::GetInstance();

    systemConfig.m_vecfCoeffA_RefMirror.clear(); //SDY_MIRROR
    systemConfig.m_vecfCoeffA_CurMirror.clear(); //SDY_MIRROR

    // Front side Coef 만들기
    CString strCoefPath;
    strCoefPath.Format(DynamicSystemPath::get(DefineFolder::System) + _T("IlluminationTableCoef.csv"));
    MakeIllumCalibrationMirrorCoef(strCoefPath, UseChannelCount, tableRefMirror, tableMirror);

    return bIsDualCal;
}

bool CSyncCalculator::MakeIllumCalibrationMirrorCoef(CString strCoefPath, long ChannelMaxCount,
    IlluminationTable tableRef, IlluminationTable tableAll, long nSideVisionModule)
{
    auto& systemConfig = SystemConfig::GetInstance(); //kircheis_IllumCal

    // SDY_DualCal 미러 계산의 결과를 IlluminationTableCoef의 뒤에 붙여서 저장한다.
    CFile fileMirror;
    if (fileMirror.Open(strCoefPath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
    {
        fileMirror.SeekToEnd();

        CArchive arMirror(&fileMirror, CArchive::store);
        auto& vecIllumLineEqRef = systemConfig.m_vecIllumFittingRefMirror;
        auto& vecIllumLineEqCur = systemConfig.m_vecIllumFittingCurMirror;

        if (nSideVisionModule == SIDE_VISIONMODULE_REAR)
        {
            vecIllumLineEqCur = systemConfig.m_vecIllumFittingCurMirror_R;
        }

        //{{//kircheis_ImproveIllumCal
        auto& vecCalibrationAuxData = systemConfig.m_vecCalibrationAuxDataMirror;
        vecCalibrationAuxData.resize(ChannelMaxCount);
        //long nRefIntenNum(0), nCurIntenNum(0);
        //float fCurIntensity(0.f), fLineCalcInten(0.f);
        //float fExpTime(0.f);
        //}}

        CString str;

        str.Format(_T("\r\n\r\nOptical Mirror,Coeff,Ref,Ref-1,Target,Target-1\r\n"));
        arMirror.WriteString(str);

        //mc_Mirror는 Ring조명을 보정을 하지 못하니 결과를 제외하여 준다
        //for (long channel = LED_ILLUM_CHANNEL_OBLIQUE; channel < nChannelMaxCount - SystemConfig().GetInstance().m_nExtension_illumCount; channel++) //ASNC 협의 이후 수정필요
        for (long channel = LED_ILLUM_CHANNEL_SIDE_OBLIQUE; channel < ChannelMaxCount; channel++)
        {
            auto lineEqRef = vecIllumLineEqRef[channel];
            auto lineEqCur = vecIllumLineEqCur[channel];

            float fRefLineA = (-lineEqRef.m_a / lineEqRef.m_b);
            float fRefLineB = -(lineEqRef.m_c / lineEqRef.m_b);
            float fRefLineRevA = (1 / fRefLineA);
            float fRefLineRevB = -(fRefLineB / fRefLineA);
            float fCurLineA = (-lineEqCur.m_a / lineEqCur.m_b);
            float fCurLineB = -(lineEqCur.m_c / lineEqCur.m_b);
            float fCurLineRevA = (1 / fCurLineA);
            float fCurLineRevB = -(fCurLineB / fCurLineA);

            //mc_ASNC Bug
            systemConfig.m_vecfCoeffA_RefMirror.push_back(fRefLineA); //SDY_MIRROR
            systemConfig.m_vecfCoeffB_RefMirror.push_back(fRefLineB);
            if (nSideVisionModule == SIDE_VISIONMODULE_FRONT)
            {
                systemConfig.m_vecfCoeffA_CurMirror.push_back(fCurLineA); //SDY_MIRROR
                systemConfig.m_vecfCoeffB_CurMirror.push_back(fCurLineB);
            }
            else
            {
                systemConfig.m_vecfCoeffA_CurMirror_SideRear.push_back(fCurLineA); //SDY_MIRROR
                systemConfig.m_vecfCoeffB_CurMirror_SideRear.push_back(fCurLineB);
            }

            str.Format(
                _T("CH%02d,Line A, %f, %f, %f, %f\r\n"), channel + 1, fRefLineA, fRefLineRevA, fCurLineA, fCurLineRevA);
            arMirror.WriteString(str);
            str.Format(_T(",Line B, %f, %f, %f, %f\r\n\n"), fRefLineB, fRefLineRevB, fCurLineB, fCurLineRevB);
            arMirror.WriteString(str);
        }

        arMirror.Close();
        fileMirror.Close();
    }

    return true;
}

bool CSyncCalculator::CalibrateIlluminationSide2D() //kircheis_IllumCal
{
    BOOL isIpis380B = (SystemConfig::GetInstance().m_nHandlerType == HANDLER_TYPE_380BRIDGE);

    for (long channel = 0; channel < LED_ILLUM_CHANNEL_SIDE_DEFAULT; channel++)
    {
        m_illuminationGains[channel] = 1.;
        m_illuminationGains_R[channel] = 1.;
    }

    CString errorDesc;

    IlluminationTable tableRef;

    if (tableRef.Load(DynamicSystemPath::get(DefineFile::IllumTableRefTableAll), errorDesc) == false)
    {
        if (!isIpis380B /* && m_pHardware != nullptr*/)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            ::AfxMessageBox(errorDesc, MB_ICONERROR | MB_OK);
        }
        return false;
    }

    IlluminationTable table_F;

    if (table_F.Load(DynamicSystemPath::get(DefineFile::IllumTableTableAll_F), errorDesc) == false)
    {
        if (!isIpis380B)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, errorDesc);
        }
        return false;
    }

    IlluminationTable table_R;

    if (table_R.Load(DynamicSystemPath::get(DefineFile::IllumTableTableAll_R), errorDesc) == false)
    {
        if (!isIpis380B)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, errorDesc);
        }
        return false;
    }

    table_F.CalcIlluminationGain(tableRef, m_illuminationGains);
    table_R.CalcIlluminationGainRear(tableRef, m_illuminationGains_R);

    long UseChannelCount = LED_ILLUM_CHANNEL_SIDE_DEFAULT;
    long nChannelMaxCount = UseChannelCount;
    auto& systemConfig = SystemConfig::GetInstance(); //kircheis_IllumCal

    systemConfig.m_vecfCoeffA_Ref.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffA_Cur.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffA_Cur_SideRear.clear(); //kircheis_IllumCalBug

    systemConfig.m_vecfCoeffB_Ref.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffB_Cur.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffB_Cur_SideRear.clear(); //kircheis_IllumCalBug

    // Front side Coef 만들기
    CString strCoefPath;
    strCoefPath.Format(DynamicSystemPath::get(DefineFolder::System) + _T("IlluminationTableCoef_F.csv"));
    MakeIllumCalibrationCoef(strCoefPath, nChannelMaxCount, tableRef, table_F, SIDE_VISIONMODULE_FRONT);

    systemConfig.m_vecfCoeffA_Ref.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffB_Ref.clear(); //kircheis_IllumCalBug

    // Rear side Coef 만들기
    strCoefPath.Format(DynamicSystemPath::get(DefineFolder::System) + _T("IlluminationTableCoef_R.csv"));
    MakeIllumCalibrationCoef(strCoefPath, nChannelMaxCount, tableRef, table_R, SIDE_VISIONMODULE_REAR);

    return true;
}

bool CSyncCalculator::CalibrateIlluminationSideMirror2D() //SDY_SideVision DualCal
{
    BOOL isIpis380B = (SystemConfig::GetInstance().m_nHandlerType == HANDLER_TYPE_380BRIDGE);
    BOOL bIsDualCal = TRUE;

    for (long channel = 0; channel < LED_ILLUM_CHANNEL_SIDE_DEFAULT; channel++)
    {
        m_illuminationGainsMirror[channel] = 1.;
        m_illuminationGainsMirror_R[channel] = 1.;
    }

    CString errorDesc;

    IlluminationTable tableRefMirror;

    if (tableRefMirror.Load(DynamicSystemPath::get(DefineFile::IllumTableRefTableAllMirror), errorDesc) == false)
    {
        if (!isIpis380B /* && m_pHardware != nullptr*/)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            ::AfxMessageBox(errorDesc, MB_ICONERROR | MB_OK);
        }
        bIsDualCal
            = FALSE; //SDY_DualCal MirrorRef 및 MirrorTable 중 하나라도 테이블이 없으면 DualCal을 사용하지 못하게 하였다.
    }

    IlluminationTable tableMirror_F;

    if (tableMirror_F.Load(DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror_F), errorDesc) == false)
    {
        if (!isIpis380B)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, errorDesc);
        }
        bIsDualCal
            = FALSE; //SDY_DualCal MirrorRef 및 MirrorTable 중 하나라도 테이블이 없으면 DualCal을 사용하지 못하게 하였다.
    }

    IlluminationTable tableMirror_R;

    if (tableMirror_R.Load(DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror_R), errorDesc) == false)
    {
        if (!isIpis380B)
        {
            // 일단 380Bridge 설비는 Table이 없으므로 에러메시지를 뜨지 않게 하였다
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, errorDesc);
        }
        bIsDualCal
            = FALSE; //SDY_DualCal MirrorRef 및 MirrorTable 중 하나라도 테이블이 없으면 DualCal을 사용하지 못하게 하였다.
    }

    tableMirror_F.CalcIlluminationGain(tableRefMirror,
        m_illuminationGainsMirror); //BUG: m_illuminationGainsMirror 크기는 10인데, CalcIlluminationGain()는 16을 기대함
    tableMirror_R.CalcIlluminationGainRear(tableRefMirror,
        m_illuminationGainsMirror_R); //BUG: m_illuminationGainsMirror_R 크기는 10인데, CalcIlluminationGainRear()는 16을 기대함

    long UseChannelCount = LED_ILLUM_CHANNEL_SIDE_DEFAULT;
    long nChannelMaxCount = UseChannelCount;
    auto& systemConfig = SystemConfig::GetInstance(); //kircheis_IllumCal

    systemConfig.m_vecfCoeffA_RefMirror.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffA_CurMirror.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffA_CurMirror_SideRear.clear(); //kircheis_IllumCalBug

    systemConfig.m_vecfCoeffB_RefMirror.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffB_CurMirror.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffB_CurMirror_SideRear.clear(); //kircheis_IllumCalBug

    // Front side Coef 만들기
    CString strCoefPath;
    strCoefPath.Format(DynamicSystemPath::get(DefineFolder::System) + _T("IlluminationTableCoef_F.csv"));
    MakeIllumCalibrationMirrorCoef(strCoefPath, nChannelMaxCount, tableRefMirror, tableMirror_F);

    // 조명 갯수를 초기화 한다.
    systemConfig.m_vecfCoeffA_RefMirror.clear(); //kircheis_IllumCalBug
    systemConfig.m_vecfCoeffB_RefMirror.clear(); //kircheis_IllumCalBug

    // Rear side Coef 만들기
    strCoefPath.Format(DynamicSystemPath::get(DefineFolder::System) + _T("IlluminationTableCoef_R.csv"));
    MakeIllumCalibrationMirrorCoef(strCoefPath, nChannelMaxCount, tableRefMirror, tableMirror_R);

    return bIsDualCal;
}

bool CSyncCalculator::IsCalibrated2DAll()
{
    return m_bIsCalibrated2DIllumAll;
}

bool CSyncCalculator::IsCalibrated2DExisting()
{
    return m_bIsCalibrated2DIllumExisting;
}

bool CSyncCalculator::IsCalibrated2DMirror()
{
    return m_bIsCalibrated2DIllumMirror;
}
