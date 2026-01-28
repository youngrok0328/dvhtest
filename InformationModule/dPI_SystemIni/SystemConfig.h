#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "SystemConfigForTR.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h> //kircheis_WB

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#define VISION_TOP_2D 0
#define VISION_BTM_2D 1
#define VISION_TOP_3D 10
#define VISION_BTM_3D 11

//a + bX + cX^2 = Y//kircheis_IllumCal
struct Curve2DEq
{
    double m_a;
    double m_b;
    double m_c;

    void InitData(double i_a, double i_b, double i_c)
    {
        m_a = i_a;
        m_b = i_b;
        m_c = i_c;
    }
};

struct CalibrationAuxDataEachCh //kircheis_ImproveIllumCal
{
    float m_fThreshExpTime_Ref;
    float m_fThreshIntensity_Cur;

    void Init()
    {
        m_fThreshExpTime_Ref = 0.f;
        m_fThreshIntensity_Cur = 0.f;
    }
};

enum enumIllumCalType //kircheis_IllumCalType
{
    IllumCalType_Start = 0,
    IllumCalType_Gain
        = IllumCalType_Start, //IlluminationTable.csv나 IlluminationTableAll.csv를 사용해도 무조건 초창기 Gain 적용 방식을 사용
    IllumCalType_OnlyLine, //IlluminationTableAll.csv를 사용해도 Line 방정식만 사용한다. IlluminationTable.csv를사용하면 그냥 그대로 사용.
    IllumCalType_Curve_Line, //IlluminationTableAll.csv를 사용하는 경우 Curve+Line 방정식을 사용한다.
    IllumCalType_Linear_Plus, //첫 채널은 Line으로 계산, 이후 추가되는 채널은 직선 계수 b를 제외하고 계산//기존의 Hybrid
    IllumCalType_End,
};

enum enumIllumMirrorCalType //SDY_DualCal
{
    IllumMirrorCalType_Start = 0,
    IllumMirrorCalType_GrayTargetOnly = IllumMirrorCalType_Start,
    IllumMirrorCalType_MirrorOnly,
    IllumMirrorCalType_DualCalibration,
    IllumMirrorCalType_End,
};

enum enumIllumType //SDY_DualCal
{
    IllumType_Start = 0,
    IllumType_Oblique = IllumType_Start,
    IllumType_Coaxial,
    IllumType_Ring,
    IllumType_UV,
    IllumType_IR,
    IllumType_End,
};

//{{mc_2023.03.14_H/W Info.
enum eMemoryType
{
    eMemoryType_PC,
    eMemoryType_iPack,
};

enum eMemoryDataType
{
    eMemoryDataType_VirtualMemory, //가상
    eMemoryDataType_PhysicalMemory, //물리
};

enum eCPUInfoDataType
{
    eCPUInfoDataType_CPU_Usage,
    eCPUInfoDataType_CPU_Temperature,
};
//}}

//mc_CPU Test
struct sVIsionInspTime
{
    long m_nTrayindex;
    long m_nScanID;
    long m_nPaneID;
    long m_threadIndex;
    float m_fTotalVisionInspTime_ms;

    void init()
    {
        m_nTrayindex = -1;
        m_nScanID = -1;
        m_nPaneID = -1;
        m_fTotalVisionInspTime_ms = -1.f;
        m_threadIndex = -1;
    }

    void SetInspTimeData(const long i_nTrayindex, const long i_nScanID, const long i_nPaneID, const long i_nthreadIndex,
        const float i_fVisionInspTime)
    {
        m_nTrayindex = i_nTrayindex;
        m_nScanID = i_nScanID;
        m_nPaneID = i_nPaneID;
        m_threadIndex = i_nthreadIndex;
        m_fTotalVisionInspTime_ms = i_fVisionInspTime;
    }

    CString GetInspTimeData()
    {
        CString strInspTimeData("");

        strInspTimeData.Format(
            _T("%d, %d, %d, %d, %f"), m_nTrayindex, m_nScanID, m_nPaneID, m_threadIndex, m_fTotalVisionInspTime_ms);

        return strInspTimeData;
    }
};

// Shared Info용
enum enumRecipeOpenType // Recipe Open Type
{
    RECIPE_OPEN_TYPE_START = 0,
    RECIPE_OPEN_TYPE_INIT = RECIPE_OPEN_TYPE_START,
    RECIPE_OPEN_TYPE_VISION_OPEN,
    RECIPE_OPEN_TYPE_NORMAL,
    RECIPE_OPEN_TYPE_BYPASS,
    RECIPE_OPEN_TYPE_END,
};

//무한 Crash Dump
enum eVisionCondition
{
    VisionCondition_Wrong = -1,
    VisionCondition_FIne = 1,
};

enum e2DIDCroppingimageSaveOption
{
    e2DID_Cropping_image_SaveOption_NotSave,
    e2DID_Cropping_image_SaveOption_SaveAll,
    e2DID_Cropping_image_SaveOption_SaveNotMatchedimage,
};

class DPI_SYSTEMINISLITDEF_API SystemConfig
{
public:
    SystemConfig();
    ~SystemConfig(void);

    static SystemConfig& GetInstance();

    void Set2DScale(const float px2um_x, const float px2um_y,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);

public:
    void SaveIni(CString strIniPath, BOOL bBackupFile = TRUE);
    void SaveBackupIni();
    void SaveCurrentScale(CString strIniPath);

    BOOL IsHardwareExist() const;

    int GetVisionType() const;
    int Get2DVisionCameraType() const;
    int Get3DVisionCameraType() const;
    int Get3DVisionCameraNumber() const;
    bool IsDual3DGrabMode() const;
    CString GetVisionTypeText() const;
    CString GetVisionTypeText(int visionType) const;
    int IsVisionType2D() const; //kircheis_3DImageSizeIssue
    int IsVisionType3D() const; //kircheis_3DImageSizeIssue
    int IsVisionTypeNGRV() const; // NGRV
    int IsVisionTypeSide() const; //kircheis_SideInsp
    int IsVisionTypeSWIR() const; //kircheis_SWIR
    bool IsVisionTypeTR() const;
    bool IsVisionType2DorSide() const;
    bool IsVisionTypeBased2D()
        const; //이 함수는 후일을 위해 만든 함수로 지금의 2D Vision과 Side Vision의 경우 2D 기반의 알고리즘을 돌려야하는 다른 비전 등장시 활용하도록 한다. 20240523 현재는 IsVisionType2DorSide()와 동일하게 동작
    int GetHandlerType() const;
    int GetGrabDuration() const;

    const VisionScale& GetScale(
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    float Get2DScaleX(const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    float Get2DScaleY(const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    float Get3DImageVerticalScale_px2um() const;

    int GetThreadNum();
    int GetGrabBufferNum(bool actualValueUsed = true);

    int GetSyncPort();
    float GetScale3D();
    CString GetPrevJob();

    void GetColorGains(float& o_fNormalChannelGainR, float& o_fNormalChannelGainG, float& o_fNormalChannelGainB,
        float& o_fReverseChannelGainR, float& o_fReverseChannelGainG, float& o_fReverseChannelGainB);

    void SetScale3D(float fScale);
    void SetSystemConfig(BOOL bExist, int nVisionType, int nCameraNum, int nLEDDuration, int nGRAB_Duration,
        int nHandlerType, int n2DCameraType, int n3DCameraType, int n3DCameraNumber);
    void SetThreadNum(int nThreadNum);
    void SetGrabBufferNum(int nGrabBufferNum);
    void SetPrevJob(CString strPrevJob);

    void SetReviewImageSaveDriveString();

    void SaveCurrentScaleXY();

    void SetVisionInfo(CString strVisionInfo, long nVisionInfo_NumType); //kk Host에서 Data받을 때 사용함수

    BOOL IsBtm2DVision(); //kircheis_NGRV_ByPass
    BOOL IsTop2DVision(); //kircheis_NGRVAF
    BOOL IsBtm3DVision(); //kircheis_NGRVAF
    BOOL IsTop3DVision(); //kircheis_NGRVAF

    BOOL IsReverseChannel(); // JHB_Reverse Channal Usage - 2022.01.17

public:
    int m_nSendHostFrameNumber;
    BOOL m_bHardwareExist;

    BOOL m_saveInlineRawImage;
    BOOL m_saveSlitbeamOriginalImage;
    int m_nReviewImageSaveDrive;
    CString m_strSaveDrive;
    BOOL m_bUseMarkMultiTeaching;

    int m_nSurfaceRejectReportPatchCount_X;
    int m_nSurfaceRejectReportPatchCount_Y;

    float m_slitbeamIlluminationAngle_deg;
    float m_slitbeamImage_px2um_y; /// 3D 원본 이미지의 Y 방향 스케일
    float m_slitbeamCameraAngle_deg;
    float m_slitbeamHeightScaling;
    int m_slitbeamCameraOffsetY;
    const float m_slitbeamScanStartOffset_um;
    double m_slitbeamScanSpeedDownRatio;
    BOOL m_slitbeamUseEncoderMultiply; // 엔코더 4체배 사용 여부
    BOOL m_slitbeamCameraRotation; // 카메라 회전 여부. 탑비전에서 임시로 사용함
    BOOL m_slitbeamOpticsReversal; // 광학계 카메라 위치 : 오른편에 있으면 TRUE
    double m_slitbeamIlluminationGain;

    CString m_strPrevJob;

    int m_nThreadNum;
    int m_nGrabBufferNum;

    int m_nVisionType{}; // 0 : 2D, 1 : 3D, 2 : NGRV
    int m_nHandlerType; // 0:380Bridge,	1:500I(for 인텔),   2:500A (for 인텔 이외)

    int m_n2DVisionCameraType; // 0 : Laon People (25M), 1 : DALSA NANO (67M)
    int m_n3DVisionCameraType; // 0 : Vieworks, 1 : Mikrotron
    int m_n3DVisionCameraNumber; // 0, 1

    int m_nGrabDuration; // 노출값 < 조명주기
    int m_nCameraTransferTimeMS; //kircheis_CamTrans
    int m_nLastDownloadDuration; //kircheis_CamTrans

    int m_nSyncPort; // 3D는 sync와 RS232통신을 한다.

    int m_nStitchGrabDirection; //Grab Direction 전달 : P1 : 0 or 1  0-Normal(Btm Vision)   1-Revierse(TopVision)

    int m_nGrabDirection; //kircheis_GrabDir

    //////////////////////////////////////////////
    // 영훈 [Login Mode] 20130816 :
    int m_nCurrentAccessMode;

    BOOL m_bUseMarkMatchRateAlgoritm2; //kircheis_MatchRate 수정

    //{{ //kircheis_SiteOption
    BOOL m_bUseMarkTeachResultView; //kircheis_HwaMark
    //}}

    BOOL m_bUseMarkTeachImageSplitSave; //kirchies_MarkTeachSplit

    bool m_bIsExchangedPackageSpec;
    bool m_bIsAnalysisResult; // 검사시 분석관련 루틴을 수행할 것인지 (VisionHostCommon 의 Analysis를 만드는데 쓰인다)

    int GetDefaultScaleX_3DVision();
    void SetDefaultScaleX_3DVision(int nValue);

    //{{ //kircheis_WB
    BOOL m_bUseIllumCalType2;
    long m_nIlluminationCalType; //kircheis_IllumCalType. 우선 순위는 m_bUsing142VersionillumCalType 얘가 높다.
    BOOL m_bLockIllumCalLinearPlus;
    void SaveIniIllumCalType();

    //SDY_DualCal
    long m_nCoaxialIllumMirrorCalType; // Dual 보정을 사용하는지 여부를 확인하기 위해서 사용한다.
    BOOL m_bUsingDualCalInterpolationType; // Dual 보정을 하면서 interpolation 여부를 체크하기 위해서 사용된다.
    float
        m_fUseCoaxialLowCut; // Dual 보정에서 나타날 수 있는 저조도 부분에서 0로 만들지에 대한 설정을 한다. -1 = 미사용, 0 = 계산결과 <0인 경우만 사용, 값이 들어있을 경우 해당 값 이하는 제거
    float
        m_fInterpolationStart; // 동축 Dual 보정에서 Interpolation이 몇 ms 부터 시작할지에 대해 설정을 한다. (기본값 4ms)
    float
        m_fInterpolationEnd; // 동축 Dual 보정에서 Interpolation이 언제까지 interpolation을 할지에 대해 설정한다. (기본값 5ms)

    double m_dEndOfCurveIntensity; //kircheis_IllumCal

    //Shared Information//Ini File에 저장하지 않으며 각 DLL간 원활한 정보 공유를 위해 system Config에 무임 승차.. 나중에 이런 넘들이 늘어나면 dPI_SharedInformaion 하나 만들자.
    std::vector<Ipvm::LineEq32r> m_vecIllumFittingRef; //Shared Information
    std::vector<Ipvm::LineEq32r> m_vecIllumFittingCur; //Shared Information
    std::vector<Ipvm::LineEq32r> m_vecIllumFittingCur_R; //SDY_Side Vision Calibration

    std::vector<Ipvm::LineEq32r> m_vecIllumFittingRefMirror; //SDY_DualCal
    std::vector<Ipvm::LineEq32r> m_vecIllumFittingCurMirror; //SDY_DualCal
    std::vector<Ipvm::LineEq32r> m_vecIllumFittingCurMirror_R; //SDY_Side Vision Calibration

    //{{//Shared Information//kircheis_IllumCal
    std::vector<Curve2DEq> m_vecIllumCurveFittingRef;
    std::vector<Curve2DEq> m_vecIllumCurveFittingCur;
    std::vector<Curve2DEq> m_vecIllumCurveFittingCur_R; //SDY_Side Vision Calibration

    std::vector<Curve2DEq> m_vecIllumCurveFittingRefReverse;
    std::vector<Curve2DEq> m_vecIllumCurveFittingCurReverse;
    std::vector<Curve2DEq> m_vecIllumCurveFittingCurReverse_R; //SDY_Side Vision Calibration

    std::vector<CalibrationAuxDataEachCh> m_vecCalibrationAuxData; //kircheis_ImproveIllumCal
    std::vector<CalibrationAuxDataEachCh> m_vecCalibrationAuxDataMirror; //SDY_DualCal

    std::vector<float> m_vecfCoeffA_Ref; //kircheis_IllumCalBug
    std::vector<float> m_vecfCoeffB_Ref;
    std::vector<float> m_vecfCoeffA_Cur; //kircheis_IllumCalBug
    std::vector<float> m_vecfCoeffB_Cur;
    std::vector<float> m_vecfCoeffA_Cur_SideRear;
    std::vector<float> m_vecfCoeffB_Cur_SideRear;
    void ClearIllumCoeffNormal();

    std::vector<float> m_vecfCoeffA_RefMirror; //SD_DualCal
    std::vector<float> m_vecfCoeffB_RefMirror;
    std::vector<float> m_vecfCoeffA_CurMirror; //SD_DualCal
    std::vector<float> m_vecfCoeffB_CurMirror;
    std::vector<float> m_vecfCoeffA_CurMirror_SideRear;
    std::vector<float> m_vecfCoeffB_CurMirror_SideRear;
    void ClearIllumCoeffMirror();

    std::vector<CalibrationAuxDataEachCh> m_vecCalibrationAuxData_R; //kircheis_ImproveIllumCal
    std::vector<CalibrationAuxDataEachCh> m_vecCalibrationAuxDataMirror_R; //SDY_DualCal

    BOOL m_bIsNGRVRun; //kircheis_NGRV_MSG
    long m_nRecipeOpenType; // 0 : 초기 상태, 1 : 직접 오픈, 2 : 일반 다운로드, 3 : ByPass download,
    //}}

    CString m_strRecipeVersion; //kircheis_RFV 저장 안함.

    //kk Vision Info Connect시 Host한태서 데이터를 받습니다. 아무것도 받지 않는 경우 Default 값으로 들어가 있습니다.
    CString m_strVisionInfo; //Default Data = Unkown
    long m_nVisionInfo_NumType; //Default Data = -1
        //Unknown -> -1, Top -> 0, BTM -> 1, Side1 -> 2, Side 2 -> 3
        //2D -> 0, 3D -> 10
        //ex> 2D TOP = 0 + 0 = 0, 3D TOP = 10 + 0 = 10

    CString m_strGrabVerifyMatchingCount[23] = {_T("Not Use"), _T("3"), _T("5"), _T("7"), _T("10"), _T("15"), _T("20"),
        _T("25"), _T("30"), _T("35"), _T("40"), _T("45"), _T("50"), _T("55"), _T("60"), _T("65"), _T("70"), _T("75"),
        _T("80"), _T("85"), _T("90"), _T("95"), _T("100")};
    long m_nGrabVerifyMatchingCount;
    bool m_bUseGrabRetry; //kircheis_3DCalcRetry

    float m_f3DIllumGain; //kircheis_3DLensIssue

    CString m_str3DCameraSN;
    int m_n3DCameraAnalogOffset1Origin;
    int m_n3DCameraAnalogOffset2Origin;
    int m_n3DCameraAnalogOffset1;
    int m_n3DCameraAnalogOffset2;
    void Save3DCameraAnalogOffset();

    // ksy 2nd Inspection 사용 유무 10.26.20 KSY
    BOOL m_bUseAiInspection;
    long m_nDLWaitTimeSendRejectData;
    BOOL m_bStateiDL; // iDL 프로그램 실행상태
    BOOL m_bUseSaveCollectMVRejectData;

    BOOL m_bIsCalIllum2D;
    BOOL getUsing142VersionillumCalType(); //mc_PGAT issue_Using142VersionCalType

    //{{NGRV Save image type parameter
    int m_nSaveImageTypeForNGRV; // 0 : BMP, 1 : PNG, 2 : JPG
    CString m_strSaveImageTypeForNGRV;
    BOOL m_bUseBayerPatternGPU;

    //{{ Color Gains - JHB_NGRV
    float m_fNormal_Channel_GainR;
    float m_fNormal_Channel_GainG;
    float m_fNormal_Channel_GainB;

    float m_fReverse_Channel_GainR;
    float m_fReverse_Channel_GainG;
    float m_fReverse_Channel_GainB;
    //}}

    //{{ NGRV Camera Gains - JHB_NGRV
    float m_fDefault_Camera_Gain;
    float m_fIR_Camera_Gain;
    float m_fUV_Camera_Gain;
    //}}

    //{{ NGRV Reverse Channel Usage - JHB_NGRV
    BOOL m_bUseReverseChannel;
    //}}

    //{{//NGRV Long Exp//kircheis_LongExp
    BOOL m_bUseLongExposureNGRV;
    const long m_nNgrvCoaxialID = 11;
    long m_nNgrvUVchID;
    long m_nNgrvIRchID;

    BOOL m_bIsByPassIRBIT;

    //{{//kircheis_20230308
    float m_fSharedInfoMotorSpeedMMperSEC; //SharedInfo
    float m_fSharedInfoScanLengthMM; //SharedInfo
    int m_n3DGrabTimeOutBufferTime; //IniSave
    //}}

    //}}

    //{{mc_23.04.26 Grab Fail Popup
    CString m_strGrabFailforimageZeroLimitCount[4] = {_T("4"), _T("6"), _T("8"), _T("10")};
    CString m_strGrabFailforCableErrorLimitCount[5] = {_T("2"), _T("4"), _T("6"), _T("8"), _T("10")};

    long m_nGrabFailforimageZeroLimitCount_idx;
    long m_nGrabFailforCableErrorLimitCount_idx;

    const long Get_GrabFailforimageZeroLimitCount();
    const long Get_nGrabFailforCableErrorLimitCount();
    //}}

    SystemConfigForTR m_tr;

private:
    VisionScale m_scale[enSideVisionModule::SIDE_VISIONMODULE_END];

    int m_nDefaultScaleX_3DVision; //Probe의 Pixel Resolution을 정수화 하여 사용한다, 최대한 Real Scale에 가까운 정수로 반올림한다
    BOOL m_bUsing142VersionillumCalType;

    void LoadIni(CString strIniPath);

    //{{mc_CPU Test

public:
    void logger_memcheck();
    void SetVisionInspTime(const CString i_strVisionInspTime);
    void SaveVisionInspTime(const long i_nReportidx);
    void ResetVisionInspTime();

    void Set_CPU_CALC_SAVE_LOG(const bool i_bSaveEnable);
    const bool Get_Enable_CPU_CALC_SAVE_LOG();
    void Set_GPU_CALC_SAVE_LOG(const bool i_bSaveEnable);
    const bool Get_Enable_GPU_CALC_SAVE_LOG();

    void Set_CPU_SavebyInspectionCount(const long i_nInspectionCount);
    void Set_CPU_SaveReportMaximumCount(const long i_nMaximumReportCount);

    const long Get_CPU_SavebyInspectionCount();
    const long Get_CPU_SaveReportMaximumCount();

private:
    std::vector<CString> m_vecstrVisionInspTime;
    bool m_bSave_CPU_CalcLog;
    long m_nCPU_SavebyInspectionCount;
    long m_nCPU_SaveReportMaximumCount;
    bool m_bSave_GPU_CalcLog;
    //}}

    //{{mc_Log Test

public:
    void Set_SaveSequenceAndInspectionTimeLog(const bool i_bSavEnable);
    const bool Get_SaveSequenceAndInspectionTimeLog();

    void Set_LotIDFromHost(const CString i_strLotID);
    CString Get_Lot_IDFromHost();
    void Set_Lot_Start_TimeFromHost(const CTime i_timeLotStart);
    CTime Get_Lot_Start_TimeFromHost();

    void Set_TimeLog_SaveFolderPath(const CString i_strSaveFolder);
    void Replace_TimeLog_Drive(const CString i_strDriveName);
    CString Get_TimeLog_SaveFolderPath();

    void Set_TimeLog_SavePath(const CString i_strSavePath);
    CString Get_Time_Log_SavePath();

    void Save_SequenceTimeLog(const CString i_strLog);

    UINT GetIllumType(long nIllumNum); // 각 채널별로 조명의 종류를 리턴한다.

    //mc_SendResult LogTest
    void Set_SaveDeviceSendResultTimeLog(const bool i_bSavEnable);
    const bool Get_SaveDeviceSendResultTimeLog();
    void Save_DeviceSendResultTimeLog(const long i_nTrayID, const long i_nPocketID, const long i_nPaneID,
        const SYSTEMTIME i_inspectionStartTime, const BOOL i_bSendSuccess, const bool i_bSaveLog);

private:
    bool m_bSaveDeviceResultSendResultTimeLog;
    float GetDiffTime_Second(const SYSTEMTIME i_SystemTime_Cur, const SYSTEMTIME i_SystemTime_Target);
    //

    bool m_bSaveSequenceAndInspectionTimeLog;
    CString m_strLotIDFromHost;
    CTime m_cTime_Lot_StartFromHost;
    CString m_strTimeLog_SaveFolderPath;
    CString m_strTimeLog_SavePath;
    //}}

    bool m_biGrab_Board_Temperature_Log_save;
    bool m_bInspectionResultTimeoutSaveRawimage;
    long m_nInsepctionTimeoutTime_ms;

    //mc_무한 Crash Dump
    long m_nVisionCondition; //1은 정상 -1은 비정상

    int32_t m_ngrvCameraType; //kircheis_NGRV_CamType

public:
    void SetNGRVOpticsType(const int& i_nCameraType);
    int GetNGRVOpticsType() const;
    bool IsNgrvColorOptics() const;
    bool IsNgrvSwirOptics() const;

    void SetVisionCondition(const eVisionCondition i_eVisionCondition);
    long GetVisionCondition();

    void SetiGrab_Board_Temperature_Log_save_Vision(const bool i_bisSave);
    bool GetiGrab_Board_Temperature_Log_save_Vision();

    void SetInspectionResultTimeoutSaveRawimage(const bool i_bEnableSave);
    bool GetInspectionResultTimeoutSaveRawimage();

    void SetInspectionResultTimeoutTime_ms(const long i_nTImeoutTime_ms);
    long GetInspectionResultTimeoutTime_ms();

    void SetGrabTimeoutCalcParam(const float i_fMotorSpeedMMperSEC, const float i_fScanLengthMM);
    DWORD GetGrabTimeoutTime();

public: //mc_2023.03.14_H/W Info.
    float Get_UsageMemoryData(const eMemoryType i_eMemoryType, const eMemoryDataType i_eeMemoryDataType);
    float Get_CPU_InfoData(const eCPUInfoDataType i_eCPUInfoDataType);

public: //mc_2023.05.22_Vision Log
    void Save_VisionLog_GrabSequenceTimeLog(const long i_nTrayID, const long i_nScanID, const CString i_strWorkLog);
    void Save_VisionLog_InspectionTimeLog(const long i_nThreadID, const long i_nTrayID, const long i_nScanID,
        const long i_nPaneID, const CString i_strWorkLog);
    bool GetFileNamesinDirecotry(const CString i_strSearchDirectoryPath, const CString i_strFileNameExtension,
        std::vector<CString>& o_vecFileNames, std::vector<CString>& o_vecFilePath);

private: //mc_23.05.24_2D Matrix Cropping image Save
    long m_n2DIDCroppingimageSaveOptionidx;

public:
    void Set2DIDCroppingimageSaveOption(const long i_nOptionidx);
    e2DIDCroppingimageSaveOption Get2DIDCroppingimageSaveOption();

public: //mc_23.06.07_iGrab F/W & Lib. Mismatch
    void Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(const bool i_bisMismatch);
    bool Get_is_iGrabFirmware_and_LibraryVersion_Mismatch();

private:
    bool m_bis_iGrabFirmware_and_LibraryVersion_Mismatch; //mc_Mismatch면 true Matching이 되면 false

private: //mc_23.06.19 ASNC Ring illumination
    bool m_bExistRingillumiation;

public:
    void SetExistRingillumination(const bool i_bEnable);
    bool GetExistRingillumination();

    // SDY_Side Vision에서 달라진 Illum 갯수 받기 위한 함수 추가
    long GetMaxIllumChannelCount();
    long GetMaxIllumChannelCount(const enTRVisionmodule TR_visionmodule);
    long GetUseIllumChannelCount();

private: // SDY_Camera noise test 진행
    bool m_bUse3DCameraNoiseTest;

public:
    void SetUse3DCameraNoiseTest(const bool i_bEnable);
    bool GetUse3DCameraNoiseTest();

public:
    // SDY Automation 관련 시스템 변수
    CString m_strAutomationRecipeFileName;
    CString m_strAutomationImageFilePath;
    long m_nAutomationSeqRunMode;
    long m_nAutomationMode;
    long m_nAutomationTestMode;

    //{{//kircheis_SideInsp

private:
    int m_nSideVisionNumber; //Side Vision 1 or 2. System.ini에 저장. Private로 Get/Set 함수로 제어.초기값 -1(SIDE_VISIONNUMBER_NOSIDEVISION)
    int m_nSideVisionFrontCameraOffsetY;
    int m_nSideVisionRearCameraOffsetY;

public:
    int GetSideVisionNumber() const;
    void SetSideVisionNumber(const int i_nSideVisionNumber); //VisionType이 Side가 아니면 입력값에 관계 없이 NoSide 적용
    //IsSideVision1/2 같은 함수는 안만든다.. 이함수들을 만들면 if(IsSideVision1()) ... else ... 이 될수 있는데 이 조건은 Not Side(-1)이 있을 수도 있어 위험하다

    int GetSideVisionFrontCameraOffsetY();
    void SetSideVisionFrontCameraOffsetY(int nFrontCameraOffsetY);
    int GetSideVisionRearCameraOffsetY();
    void SetSideVisionRearCameraOffsetY(int nRearCameraOffsetY);
    //}}

    //{{ //mc_SpaceTimeSamplingScaleFactor

private:
    float m_fSpaceTimeSamplingScaleFactor;

public:
    void Set_SpaceTimeSamplingScaleFactor(const float i_fScaleFactor);
    float Get_SpaceTimeSamplingScaleFactor();
    //}}

private: //kircheis_SWIR
    int m_fgType; //Frame Grabber Type //Default는 iGrabG2 시리즈
    CString m_cameraModel; //Camera Model Name //Default는 "LPMVC-CL25M"

public: //kircheis_SWIR
    void SetFrameGrabberType(const int i_fgType);
    int GetFrameGrabberType() const;
    bool IsFrameGrabberTypeiGrabG2() const; //iGrabG2 시리즈인지 확인
    bool IsFrameGrabberTypeiGrabXE() const; //iGrabXE인지 확인
    bool IsFrameGrabberTypeiGrabXQ() const; //iGrabXQ인지 확인
    void SetCameraModel(const CString& i_cameraModel);
    CString GetCameraModel() const;
    bool IsNGRVInspectionMode() const; // NGRV Inspection Mode 여부 확인
};
