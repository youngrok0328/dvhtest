//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SystemConfig.h"

//CPP_2_________________________________ This project's headers
#include "PersonalConfig.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedComponent/Persistence/IniHelper.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Pdh.h>
#include <Wbemidl.h>
#include <Windows.h>
#include <afxmt.h>
#include <psapi.h>

//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NGRV_SAVE_IMAGE_FORMAT 1

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "Pdh.lib")
#pragma comment(lib, "wbemuuid.lib")

SystemConfig::SystemConfig()
    : m_bHardwareExist(FALSE)
    , m_strPrevJob(_T(""))
    , m_nSyncPort(0)
    , m_nThreadNum(1)
    , m_nGrabBufferNum(0)
    , m_nGrabDuration(37)
    , m_nCameraTransferTimeMS(26) //kircheis_CamTrans
    , m_nLastDownloadDuration(0) //kircheis_CamTrans
    , m_nSendHostFrameNumber(0)
    , m_saveInlineRawImage(FALSE)
    , m_saveSlitbeamOriginalImage(FALSE)
    , m_nReviewImageSaveDrive(0)
    , m_slitbeamIlluminationAngle_deg(45.0)
    , m_slitbeamCameraAngle_deg(45.0)
    , m_slitbeamImage_px2um_y(10.f)
    , m_slitbeamHeightScaling(1.f)
    , m_slitbeamCameraOffsetY(0)
    , m_slitbeamScanStartOffset_um(1000.f)
    , m_slitbeamScanSpeedDownRatio(1.1)
    , m_slitbeamUseEncoderMultiply(FALSE)
    , m_slitbeamCameraRotation(FALSE)
    , m_slitbeamOpticsReversal(FALSE)
    , m_slitbeamIlluminationGain(1.)
    , m_nCurrentAccessMode(0)
    , m_bUseMarkMultiTeaching(TRUE)
    , m_nSurfaceRejectReportPatchCount_X(3)
    , m_nSurfaceRejectReportPatchCount_Y(3)
    , m_bUseMarkMatchRateAlgoritm2(FALSE) //kircheis_MatchRate 수정
    , m_bUseMarkTeachResultView(FALSE) //kircheis_HwaMark
    , m_bUseMarkTeachImageSplitSave(FALSE) //kirchies_MarkTeachSplit
    , m_nStitchGrabDirection(_STITCH_GRAB_NORMAL)
    , m_bIsExchangedPackageSpec(false)
    , m_bIsAnalysisResult(false)
    , m_nDefaultScaleX_3DVision(12)
    , m_bUseIllumCalType2(false) //kircheis_WB
    , m_strVisionInfo(_T("Unknown")) //kk
    , m_nVisionInfo_NumType(-1) //kk
    , m_nGrabVerifyMatchingCount(0) //kk
    , m_bUseGrabRetry(false) //kircheis_3DCalcRetry
    , m_f3DIllumGain(1.f)
    , m_dEndOfCurveIntensity(30.) //kircheis_IllumCal
    , m_bSave_CPU_CalcLog(false) //mc_CPU Test
    , m_bSave_GPU_CalcLog(false) //mc_GPU Test
    , m_nCPU_SavebyInspectionCount(-1)
    , m_nCPU_SaveReportMaximumCount(-1)
    , m_bUseAiInspection(FALSE)
    , m_bUseSaveCollectMVRejectData(FALSE) // 09.28.20 KSY
    , m_bStateiDL(FALSE)
    , m_nDLWaitTimeSendRejectData(5000)
    , m_bUsing142VersionillumCalType(TRUE)
    , m_bIsCalIllum2D(FALSE)
    , m_bUseBayerPatternGPU(FALSE)
    , m_fNormal_Channel_GainR(1.f)
    , m_fNormal_Channel_GainG(1.f)
    , m_fNormal_Channel_GainB(1.f)
    , m_fReverse_Channel_GainR(1.f)
    , m_fReverse_Channel_GainG(1.f)
    , m_fReverse_Channel_GainB(1.f)
    , m_bIsNGRVRun(FALSE) //kircheis_NGRV_MSG
    , m_nRecipeOpenType(RECIPE_OPEN_TYPE_INIT)
    , m_bUseLongExposureNGRV(FALSE) //kircheis_LongExp
    , m_nNgrvIRchID(-1) //kircheis_LongExp
    , m_nNgrvUVchID(-1) //kircheis_LongExp
    , m_fDefault_Camera_Gain(0.f)
    , m_fIR_Camera_Gain(0.f)
    , m_fUV_Camera_Gain(0.f)
    , m_bIsByPassIRBIT(FALSE)
    , m_nIlluminationCalType(IllumCalType_Linear_Plus) //kircheis_IllumCalType
    , m_bLockIllumCalLinearPlus(TRUE)
    , m_bUseReverseChannel(FALSE) //JHB_NGRV Reverse Channel Usage - 2022.01.17
    , m_strLotIDFromHost("")
    , m_strTimeLog_SaveFolderPath("")
    , m_strTimeLog_SavePath("")
    , m_nCoaxialIllumMirrorCalType(IllumMirrorCalType_GrayTargetOnly) //SDY_DualCal
    , m_bUsingDualCalInterpolationType(TRUE) //SDY_DualCal
    , m_fUseCoaxialLowCut(0.1f) //SDY_DualCal
    , m_fInterpolationStart(4.f) //SDY_DualCal
    , m_fInterpolationEnd(5.f)
    , m_nSaveImageTypeForNGRV(NGRV_SAVE_IMAGE_FORMAT) // JHB_Default는 무조건 JPG = 1
    , m_nVisionCondition(eVisionCondition::VisionCondition_FIne)
    , m_biGrab_Board_Temperature_Log_save(false)
    , m_bInspectionResultTimeoutSaveRawimage(false)
    , m_nInsepctionTimeoutTime_ms(50000)
    , m_fSharedInfoMotorSpeedMMperSEC(40.f) //kircheis_20230308//SharedInfo
    , m_fSharedInfoScanLengthMM(135.f) //kircheis_20230308//SharedInfo
    , m_n3DGrabTimeOutBufferTime(2500) //kircheis_20230308//IniSave
    , m_bSaveDeviceResultSendResultTimeLog(false)
    , m_nGrabFailforimageZeroLimitCount_idx(0)
    , m_nGrabFailforCableErrorLimitCount_idx(0)
    , m_n2DIDCroppingimageSaveOptionidx(e2DIDCroppingimageSaveOption::e2DID_Cropping_image_SaveOption_SaveAll)
    , m_bis_iGrabFirmware_and_LibraryVersion_Mismatch(false)
    , m_bExistRingillumiation(false)
    , m_bUse3DCameraNoiseTest(true)
    , m_strAutomationRecipeFileName("")
    , m_strAutomationImageFilePath("")
    , m_nAutomationSeqRunMode(0)
    , m_nAutomationMode(0)
    , m_nAutomationTestMode(0)
    , m_nSideVisionNumber(SIDE_VISIONNUMBER_NOSIDEVISION) //kircheis_SideInsp
    , m_fSpaceTimeSamplingScaleFactor(1.f) //mc_SpaceTimeSamplingScaleFactor
    , m_nNgrvCoaxialID(11)
    , m_nSideVisionFrontCameraOffsetY(0)
    , m_nSideVisionRearCameraOffsetY(0)
    , m_fgType(enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES) //kircheis_SWIR
    , m_cameraModel(_T("LPMVC-CL25M")) //kircheis_SWIR
    , m_n2DVisionCameraType(en2DVisionCameraType::CAMERATYPE_LAON_PEOPLE_CXP25M) // 2D Vision Camera Type
    , m_n3DVisionCameraType(en3DVisionCameraType::CAMERATYPE_VIEWWORKS_12MX) // 2D Vision Camera Type
    , m_n3DVisionCameraNumber(1)
{
    // 먼저 파일이 있는지 확인한다.
    CString strSystemFolderPath = DynamicSystemPath::get(DefineFolder::System);
    ::CreateDirectory(strSystemFolderPath, NULL);

    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);

    CFileFind ff;
    if (ff.FindFile(systemFilePath) == FALSE)
    {
        SaveIni(systemFilePath, FALSE);
    }

    LoadIni(systemFilePath);

    SetReviewImageSaveDriveString();

    m_vecIllumFittingRef.clear();
    m_vecIllumFittingCur.clear();
    //{{//kircheis_IllumCal
    m_vecIllumCurveFittingRef.clear();
    m_vecIllumCurveFittingCur.clear();
    m_vecIllumCurveFittingRefReverse.clear();
    m_vecIllumCurveFittingCurReverse.clear();

    m_vecCalibrationAuxData.clear(); //kircheis_ImproveIllumCal

    ClearIllumCoeffNormal();
    ClearIllumCoeffMirror();

    //{{//SDY_DualCal
    m_vecIllumFittingRefMirror.clear();
    m_vecIllumFittingCurMirror.clear();

    m_vecCalibrationAuxData.clear(); //SDY_DualCal
    m_vecCalibrationAuxDataMirror.clear(); //SDY_DualCal
    //}}

    m_vecstrVisionInspTime.clear(); //mc_CPU Test
}

SystemConfig::~SystemConfig(void)
{
    m_vecIllumFittingRef.clear();
    m_vecIllumFittingCur.clear();
    //{{//kircheis_IllumCal
    m_vecIllumCurveFittingRef.clear();
    m_vecIllumCurveFittingCur.clear();
    m_vecIllumCurveFittingRefReverse.clear();
    m_vecIllumCurveFittingCurReverse.clear();

    m_vecCalibrationAuxData.clear(); //kircheis_ImproveIllumCal

    ClearIllumCoeffNormal();
    ClearIllumCoeffMirror();

    //{{//SDY_DualCal
    m_vecIllumFittingRefMirror.clear();
    m_vecIllumFittingCurMirror.clear();

    m_vecCalibrationAuxData.clear(); //SDY_DualCal
    m_vecCalibrationAuxDataMirror.clear(); //SDY_DualCal
    //}}

    m_vecstrVisionInspTime.clear(); //mc_CPU Test
}

SystemConfig& SystemConfig::GetInstance()
{
    static SystemConfig singleton;

    return singleton;
}

void SystemConfig::Set2DScale(const float px2um_x, const float px2um_y, const enSideVisionModule i_eSideVisionModule)
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);

    auto imageSampling = PersonalConfig::getInstance().getImageSampling();

    switch (m_nVisionType)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_NGRV_INSP:
        case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
        case VISIONTYPE_TR:
        {
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("2D Scale X"), px2um_x);
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("2D Scale Y"), px2um_y);

            m_scale[i_eSideVisionModule].setScaleX(px2um_x * imageSampling);
            m_scale[i_eSideVisionModule].setScaleY(px2um_y * imageSampling);
        }
        break;
        case VISIONTYPE_3D_INSP:
        {
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("3D Scale X"), (float)m_nDefaultScaleX_3DVision);
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("3D Scale Y"), px2um_y);

            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX(
                (float)(m_nDefaultScaleX_3DVision * imageSampling));
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleY(px2um_y * imageSampling);
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            CString strSideScaleX("2D Scale X"), strSideScaleY("2D Scale Y");
            switch (i_eSideVisionModule)
            {
                case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                    strSideScaleX += _T("_F");
                    strSideScaleY += _T("_F");
                    break;
                case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                    strSideScaleX += _T("_R");
                    strSideScaleY += _T("_R");
                    break;
            }
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), (LPCTSTR)strSideScaleX, px2um_x);
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), (LPCTSTR)strSideScaleY, px2um_y);

            m_scale[i_eSideVisionModule].setScaleX(px2um_x * imageSampling);
            m_scale[i_eSideVisionModule].setScaleY(px2um_y * imageSampling);
        }
        break;
    }
}

void SystemConfig::SaveCurrentScaleXY()
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);

    switch (m_nVisionType)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_NGRV_INSP:
        case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
        case VISIONTYPE_TR:
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("2D Scale X"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_x);
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("2D Scale Y"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_y);
            break;

        case VISIONTYPE_3D_INSP:
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX((float)m_nDefaultScaleX_3DVision);
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("3D Scale X"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_x);
            IniHelper::SaveFLOAT(systemFilePath, _T("Calibration"), _T("3D Scale Y"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_y);
            break;
        case VISIONTYPE_SIDE_INSP:
            for (long nSideModuleidx = enSideVisionModule::SIDE_VISIONMODULE_START;
                nSideModuleidx < enSideVisionModule::SIDE_VISIONMODULE_END; nSideModuleidx++)
            {
                CString strSideScaleX("2D Scale X"), strSideScaleY("2D Scale Y");
                switch (nSideModuleidx)
                {
                    case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                        strSideScaleX += _T("_F");
                        strSideScaleY += _T("_F");
                        break;
                    case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                        strSideScaleX += _T("_R");
                        strSideScaleY += _T("_R");
                        break;
                }
                IniHelper::SaveFLOAT(
                    systemFilePath, _T("Calibration"), (LPCTSTR)strSideScaleX, m_scale[nSideModuleidx].pixelToUm().m_x);
                IniHelper::SaveFLOAT(
                    systemFilePath, _T("Calibration"), (LPCTSTR)strSideScaleY, m_scale[nSideModuleidx].pixelToUm().m_y);
            }
            break;

        default:
            break;
    }
}

void SystemConfig::SaveCurrentScale(CString strIniPath)
{
    switch (m_nVisionType)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_NGRV_INSP:
        case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
        case VISIONTYPE_TR:
            IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("2D Scale X"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_x);
            IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("2D Scale Y"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_y);
            break;

        case VISIONTYPE_3D_INSP:
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX((float)m_nDefaultScaleX_3DVision);
            IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("3D Scale X"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_x);
            IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("3D Scale Y"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_y);
            IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("3D Scale"),
                m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUmZ());
            break;
        case VISIONTYPE_SIDE_INSP:
            for (long nSideModuleidx = enSideVisionModule::SIDE_VISIONMODULE_START;
                nSideModuleidx < enSideVisionModule::SIDE_VISIONMODULE_END; nSideModuleidx++)
            {
                CString strSideScaleX("2D Scale X"), strSideScaleY("2D Scale Y");
                switch (nSideModuleidx)
                {
                    case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                        strSideScaleX += _T("_F");
                        strSideScaleY += _T("_F");
                        break;
                    case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                        strSideScaleX += _T("_R");
                        strSideScaleY += _T("_R");
                        break;
                }
                IniHelper::SaveFLOAT(
                    strIniPath, _T("Calibration"), (LPCTSTR)strSideScaleX, m_scale[nSideModuleidx].pixelToUm().m_x);
                IniHelper::SaveFLOAT(
                    strIniPath, _T("Calibration"), (LPCTSTR)strSideScaleY, m_scale[nSideModuleidx].pixelToUm().m_y);
            }
            break;

        default:
            break;
    }
}

void SystemConfig::Save3DCameraAnalogOffset()
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);

    IniHelper::SaveSTRING(systemFilePath, _T("Hardware"), _T("3D Camera Serial Number"), m_str3DCameraSN);
    IniHelper::SaveINT(
        systemFilePath, _T("Hardware"), _T("3D Camera Analog Offset1 Origin"), m_n3DCameraAnalogOffset1Origin);
    IniHelper::SaveINT(systemFilePath, _T("Hardware"), _T("3D Camera Analog Offset1"), m_n3DCameraAnalogOffset1);
    IniHelper::SaveINT(
        systemFilePath, _T("Hardware"), _T("3D Camera Analog Offset2 Origin"), m_n3DCameraAnalogOffset2Origin);
    IniHelper::SaveINT(systemFilePath, _T("Hardware"), _T("3D Camera Analog Offset2"), m_n3DCameraAnalogOffset2);
}

void SystemConfig::SetReviewImageSaveDriveString()
{
    if (m_nReviewImageSaveDrive == 0)
        m_strSaveDrive = _T("C:\\");
    if (m_nReviewImageSaveDrive == 1)
        m_strSaveDrive = _T("D:\\");
    if (m_nReviewImageSaveDrive == 2)
        m_strSaveDrive = _T("E:\\");
    if (m_nReviewImageSaveDrive == 3)
        m_strSaveDrive = _T("F:\\");
    if (m_nReviewImageSaveDrive == 4)
        m_strSaveDrive = _T("G:\\");
    if (m_nReviewImageSaveDrive == 5)
        m_strSaveDrive = _T("H:\\");
}

void SystemConfig::SetSystemConfig(BOOL bExist, int nVisionType, int nCameraNum, int nLEDDuration, int nGRAB_Duration,
    int nHandlerType, int n2DCameraType, int n3DCameraType, int n3DCameraNumber)
{
    UNREFERENCED_PARAMETER(nCameraNum);
    UNREFERENCED_PARAMETER(nLEDDuration);

    m_bHardwareExist = bExist;
    m_nVisionType = nVisionType;
    m_nGrabDuration = nGRAB_Duration;
    m_nHandlerType = nHandlerType;
    m_n2DVisionCameraType = n2DCameraType;
    m_n3DVisionCameraType = n3DCameraType;
    m_n3DVisionCameraNumber = n3DCameraNumber;
}

int SystemConfig::GetDefaultScaleX_3DVision()
{
    return m_nDefaultScaleX_3DVision;
}

void SystemConfig::SetDefaultScaleX_3DVision(int nValue)
{
    m_nDefaultScaleX_3DVision = nValue;
}

void SystemConfig::LoadIni(CString strIniPath)
{
    m_nSendHostFrameNumber = IniHelper::LoadINT(strIniPath, _T("Vision_Setting"), _T("Send_Host_Frame_Num"), 0);
    m_nReviewImageSaveDrive = IniHelper::LoadINT(strIniPath, _T("Vision_Setting"), _T("ReviewImageSaveDrive"), 0);
    m_nSurfaceRejectReportPatchCount_X
        = IniHelper::LoadINT(strIniPath, _T("Vision_Setting"), _T("Surface_Reject_Report_Patch_Count_X"), 3);
    m_nSurfaceRejectReportPatchCount_Y
        = IniHelper::LoadINT(strIniPath, _T("Vision_Setting"), _T("Surface_Reject_Report_Patch_Count_Y"), 3);

    m_nVisionType = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("Vision_Type"), VISIONTYPE_2D_INSP);
    m_strVisionInfo = IniHelper::LoadSTRING(strIniPath, _T("Setting"), _T("Vision_TypeName"), _T("Unknown"));
    m_nHandlerType = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("Handler_Type"), HANDLER_TYPE_500I);
    m_nSideVisionNumber = IniHelper::LoadINT(
        strIniPath, _T("Setting"), _T("SideVision_Number"), SIDE_VISIONNUMBER_NOSIDEVISION); //kircheis_SideInsp

    m_ngrvCameraType = IniHelper::LoadINT(
        strIniPath, _T("Setting"), _T("NGRV_Camera_Type"), NGRV_VISION_OPTICS_TYPE_COLOR); //kircheis_NGRV_Type

    m_n2DVisionCameraType = IniHelper::LoadINT(
        strIniPath, _T("Setting"), _T("2D Vision Camera Type"), CAMERATYPE_LAON_PEOPLE_CXP25M);

    m_n3DVisionCameraType
        = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("3D Vision Camera Type"), CAMERATYPE_VIEWWORKS_12MX);

    m_n3DVisionCameraNumber
        = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("3D Vision Camera Number"), 1);

    m_nGrabDirection = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("3DGrabDirection"), 1); //kircheis_GrabDir
    //m_nGrabDirection = (int)min(1, m_nGrabDirection);
    //m_nGrabDirection = (int)max(0, m_nGrabDirection);

    m_nStitchGrabDirection
        = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("StitchGrabDirection"), _STITCH_GRAB_NORMAL);
    if (m_nHandlerType == HANDLER_TYPE_380BRIDGE)
        m_nStitchGrabDirection = _STITCH_GRAB_NORMAL;

    m_bHardwareExist = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Hardware Exist"), FALSE);

    m_fgType = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Frame Grabber Type"),
        static_cast<int>(enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES)); //kircheis_SWIR
    m_cameraModel
        = IniHelper::LoadSTRING(strIniPath, _T("Hardware"), _T("Camera Model"), _T("LPMVC-CL25M")); //kircheis_SWIR

    m_nAutomationTestMode = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("AutomationTestMode"), FALSE);

    if (PersonalConfig::getInstance().isPersonalInstallationEnabled())
    {
        // 개인용도로 설치했을 때
        // 1. 하드웨어는 항상 없는 걸로 하자
        // 2. INTEKPLUS 모드로 접속해 주자

        m_bHardwareExist = FALSE;
        m_nCurrentAccessMode = _INTEKPLUS;
    }
    else
    {
        m_bHardwareExist = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Hardware Exist"), FALSE);
        m_nThreadNum = IniHelper::LoadINT(strIniPath, _T("ThreadNum"), _T("ThreadNum"), 1);
        m_nThreadNum = (long)max(m_nThreadNum, 1); //누군가 실수로 ThreadNum을 0으로 지정한 경우가 있다
        m_nThreadNum = (long)min(m_nThreadNum, 10); //ThreadNum은 10개를 넘기자 말자
        m_nGrabBufferNum = IniHelper::LoadINT(strIniPath, _T("Grab"), _T("BufferNum"), 0);
        m_nCurrentAccessMode = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("Vision_AccessMode"), 0);
    }

    //{{//kircheis_LongExp //Loaing전 초기화
    m_nNgrvIRchID = -1;
    m_nNgrvUVchID = -1;
    //}}

    //{{ NGRV Vision을 사용하지 않을 경우에는 FALSE - JHB_2024.02.15
    m_bUseLongExposureNGRV = TRUE;
    m_bUseBayerPatternGPU = TRUE;
    //}}

    switch (m_nVisionType)
    {
        case VISIONTYPE_2D_INSP:
        case VISIONTYPE_SWIR_INSP: //kircheis_SWIR //NeedRefac
        {
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("2D Scale X"), 10.f));
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleY(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("2D Scale Y"), 10.f));
            m_n2DIDCroppingimageSaveOptionidx
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("2DID Cropping image SaveOption Index"),
                    e2DIDCroppingimageSaveOption::
                        e2DID_Cropping_image_SaveOption_SaveAll); //mc_2DID Cropping image Save Option

            long nExistExtension_illumCount = IniHelper::LoadINT(strIniPath, _T("Hardware"),
                _T("Extension Illumination Num"), FALSE); //기존에 있던 값을 Converting 해야하니..

            if (nExistExtension_illumCount > 0) //Default 값이 아니면 이전에 사용했던것이니 강제로 true로 만들어준다
                m_bExistRingillumiation = true;
            else //Default 값이면 ,이번에 추가한걸로 읽는다
            {
                m_bExistRingillumiation
                    = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Exist Ring Illumination"), FALSE);
                IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Extension Illumination Num"), -1);
            }
        }
        break;

        case VISIONTYPE_3D_INSP:
        {
            m_nDefaultScaleX_3DVision = IniHelper::LoadINT(strIniPath, _T("Calibration"), _T("Default 3D Scale X"), 12);
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX((float)
                    m_nDefaultScaleX_3DVision); // IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("3D Scale X"), 10.f);
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleY(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("3D Scale Y"), 10.f));
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleZ(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("3D Scale"), 1.f));

            m_str3DCameraSN = IniHelper::LoadSTRING(strIniPath, _T("Hardware"), _T("3D Camera Serial Number"), _T(""));
            m_n3DCameraAnalogOffset1Origin
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("3D Camera Analog Offset1 Origin"), 0);
            m_n3DCameraAnalogOffset1
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("3D Camera Analog Offset1"), 0);
            m_n3DCameraAnalogOffset2Origin
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("3D Camera Analog Offset2 Origin"), 0);
            m_n3DCameraAnalogOffset2
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("3D Camera Analog Offset2"), 0);
            m_n3DGrabTimeOutBufferTime = IniHelper::LoadINT(
                strIniPath, _T("Hardware"), _T("3D Grab Time Out Buffer(msec)"), 2500); //kircheis_20230308
            if (m_n3DGrabTimeOutBufferTime < 1000)
            {
                m_n3DGrabTimeOutBufferTime = 1000;
                IniHelper::SaveINT(
                    strIniPath, _T("Hardware"), _T("3D Grab Time Out Buffer(msec)"), m_n3DGrabTimeOutBufferTime);
            }
            else if (m_n3DGrabTimeOutBufferTime > 5000)
            {
                m_n3DGrabTimeOutBufferTime = 2500;
                IniHelper::SaveINT(
                    strIniPath, _T("Hardware"), _T("3D Grab Time Out Buffer(msec)"), m_n3DGrabTimeOutBufferTime);
            }

            m_bUse3DCameraNoiseTest = IniHelper::LoadINT(
                strIniPath, _T("Hardware"), _T("Use 3D Camera Noise Check"), TRUE); //kircheis_20230308
            m_fSpaceTimeSamplingScaleFactor = IniHelper::LoadFLOAT(
                strIniPath, _T("Hardware"), _T("SpaceTimeSamplingScaleFactor"), 1.f); //mc_SpaceTimeSamplingScaleFactor

            if (m_fSpaceTimeSamplingScaleFactor <= 0.8f || m_fSpaceTimeSamplingScaleFactor >= 1.1f)
                m_fSpaceTimeSamplingScaleFactor = 1.f;
        }
        break;

        case VISIONTYPE_NGRV_INSP:
        {
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("2D Scale X"), 2.f));
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleY(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("2D Scale Y"), 2.f));
            //{{ Color Gains
            m_fNormal_Channel_GainR
                = IniHelper::LoadFLOAT(strIniPath, _T("Color Option"), _T("Normal Channel Red Gain"), 1.f);
            m_fNormal_Channel_GainG
                = IniHelper::LoadFLOAT(strIniPath, _T("Color Option"), _T("Normal Channel Green Gain"), 1.f);
            m_fNormal_Channel_GainB
                = IniHelper::LoadFLOAT(strIniPath, _T("Color Option"), _T("Normal Channel Blue Gain"), 1.f);

            m_fReverse_Channel_GainR
                = IniHelper::LoadFLOAT(strIniPath, _T("Color Option"), _T("Reverse Channel Red Gain"), 1.f);
            m_fReverse_Channel_GainG
                = IniHelper::LoadFLOAT(strIniPath, _T("Color Option"), _T("Reverse Channel Green Gain"), 1.f);
            m_fReverse_Channel_GainB
                = IniHelper::LoadFLOAT(strIniPath, _T("Color Option"), _T("Reverse Channel Blue Gain"), 1.f);
            //}}

            //{{//kircheis_LongExp
            //m_bUseLongExposureNGRV = (BOOL)IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Use long exposure for NGRV"), TRUE);
            //m_bUseBayerPatternGPU = IniHelper::LoadINT(strIniPath, _T("SpecialOption"), _T("Use Bayer Pattern GPU"), TRUE);
            m_bUseLongExposureNGRV = TRUE;
            m_bUseBayerPatternGPU = TRUE;
            // 위 두 Parameter는 항상 TRUE로 받게 수정 - JHB_2024.02.15

            m_nNgrvIRchID = (long)IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("NGRV IR Channel ID(1~16)"), 4)
                - 1; //1을 빼주는 이유는 System.ini에는 1~16이나
            m_nNgrvUVchID = (long)IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("NGRV UV Channel ID(1~16)"), 2)
                - 1; //Code 내부에서는 0~15이므로

            m_bIsByPassIRBIT = (BOOL)IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("NGRV ByPass IR"), FALSE);
            //}}

            //{{ Camera Gains
            m_fDefault_Camera_Gain
                = (float)IniHelper::LoadFLOAT(strIniPath, _T("Hardware"), _T("NGRV Default Channel Gain"), 3.f);
            m_fIR_Camera_Gain
                = (float)IniHelper::LoadFLOAT(strIniPath, _T("Hardware"), _T("NGRV IR Channel Gain"), 3.f);
            m_fUV_Camera_Gain
                = (float)IniHelper::LoadFLOAT(strIniPath, _T("Hardware"), _T("NGRV UV Channel Gain"), 3.f);
            //}}
        }
        break;
        case VISIONTYPE_SIDE_INSP:
        {
            for (long nSideModuleidx = enSideVisionModule::SIDE_VISIONMODULE_START;
                nSideModuleidx < enSideVisionModule::SIDE_VISIONMODULE_END; nSideModuleidx++)
            {
                CString strSideScaleX("2D Scale X"), strSideScaleY("2D Scale Y");
                switch (nSideModuleidx)
                {
                    case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                        strSideScaleX += _T("_F");
                        strSideScaleY += _T("_F");
                        break;
                    case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                        strSideScaleX += _T("_R");
                        strSideScaleY += _T("_R");
                        break;
                }

                m_scale[nSideModuleidx].setScaleX(
                    IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), (LPCTSTR)strSideScaleX, 5.7f));
                m_scale[nSideModuleidx].setScaleY(
                    IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), (LPCTSTR)strSideScaleY, 5.7f));
            }

            m_nSideVisionFrontCameraOffsetY
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("SideCameraOffsetY_Front"), 0);
            m_nSideVisionRearCameraOffsetY
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("SideCameraOffsetY_Rear"), 0);
        }
        break;
        case VISIONTYPE_TR:
        {
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("2D Scale X"), 10.f));
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleY(
                IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("2D Scale Y"), 10.f));
            m_n2DIDCroppingimageSaveOptionidx
                = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("2DID Cropping image SaveOption Index"),
                    e2DIDCroppingimageSaveOption::
                        e2DID_Cropping_image_SaveOption_SaveAll); //mc_2DID Cropping image Save Option

            long nExistExtension_illumCount = IniHelper::LoadINT(strIniPath, _T("Hardware"),
                _T("Extension Illumination Num"), FALSE); //기존에 있던 값을 Converting 해야하니..

            if (nExistExtension_illumCount > 0) //Default 값이 아니면 이전에 사용했던것이니 강제로 true로 만들어준다
                m_bExistRingillumiation = true;
            else //Default 값이면 ,이번에 추가한걸로 읽는다
            {
                m_bExistRingillumiation
                    = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Exist Ring Illumination"), FALSE);
                IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Extension Illumination Num"), -1);
            }

            m_tr.LoadIni(strIniPath);
        }
        break;
    }

    auto imageSampling = PersonalConfig::getInstance().getImageSampling();

    if (m_nVisionType != VISIONTYPE_SIDE_INSP)
    {
        m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleX(
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_x * imageSampling);
        m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleY(
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUm().m_y * imageSampling);
    }
    else
    {
        for (long nSideModuleidx = enSideVisionModule::SIDE_VISIONMODULE_START;
            nSideModuleidx < enSideVisionModule::SIDE_VISIONMODULE_END; nSideModuleidx++)
        {
            m_scale[nSideModuleidx].setScaleX(m_scale[nSideModuleidx].pixelToUm().m_x * imageSampling);
            m_scale[nSideModuleidx].setScaleY(m_scale[nSideModuleidx].pixelToUm().m_y * imageSampling);
        }
    }

    m_strPrevJob = IniHelper::LoadSTRING(strIniPath, _T("Setting"), _T("Prev Job Name"), _T(""));
    m_nSyncPort = IniHelper::LoadINT(strIniPath, _T("SyncPort"), _T("SyncPort"), 0);
    m_nGrabDuration = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("GrabDuration"), 40);
    m_nCameraTransferTimeMS
        = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("CameraTransferTime"), 26); //kircheis_CamTrans

    m_slitbeamCameraOffsetY = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("SlitbeamCameraOffsetY"), 0);
    m_slitbeamUseEncoderMultiply = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("SlitbeamUseEncoderMultiply"), 0);
    m_slitbeamCameraRotation = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("SlitbeamCameraRotation"), 0);
    m_slitbeamOpticsReversal = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("SlitbeamOpticsReversal"), 0);
    m_slitbeamIlluminationAngle_deg
        = IniHelper::LoadFLOAT(strIniPath, _T("Setting"), _T("m_slitbeamIlluminationAngle_deg"), 45.f);
    m_slitbeamCameraAngle_deg = IniHelper::LoadFLOAT(strIniPath, _T("Setting"), _T("m_slitbeamCameraAngle_deg"), 45.f);
    m_slitbeamImage_px2um_y = IniHelper::LoadFLOAT(strIniPath, _T("Setting"), _T("m_slitbeamImage_px2um_y"), 10.f);
    m_slitbeamHeightScaling = IniHelper::LoadFLOAT(strIniPath, _T("Setting"), _T("m_slitbeamHeightScaling"), 1.f);
    m_slitbeamScanSpeedDownRatio
        = IniHelper::LoadDOUBLE(strIniPath, _T("Setting"), _T("m_slitbeamScanSpeedDownRatio"), 1.1);
    m_slitbeamIlluminationGain = IniHelper::LoadDOUBLE(strIniPath, _T("Setting"), _T("m_slitbeamIlluminationGain"), 1.);

    m_f3DIllumGain
        = IniHelper::LoadFLOAT(strIniPath, _T("Setting"), _T("3D_Illumination_Gain"), 1.f); //kircheis_3DLensIssue
    m_f3DIllumGain = (float)max(0.3, m_f3DIllumGain);

    m_bUseMarkMultiTeaching = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("Vision_MarkMultiTeaching"), TRUE);

    m_bUseMarkMatchRateAlgoritm2 = IniHelper::LoadINT(
        strIniPath, _T("Warning"), _T("UseMarkMatchRateAlgoritm2"), FALSE); //kircheis_MatchRate 수정
    m_bUseMarkTeachResultView
        = IniHelper::LoadINT(strIniPath, _T("SiteOption"), _T("Use Mark Teach Result View"), FALSE); //kircheis_HwaMark
    m_bUseMarkTeachImageSplitSave = IniHelper::LoadINT(
        strIniPath, _T("SpecialOption"), _T("UseMarkTeachImageSplitSave"), FALSE); //kirchies_MarkTeachSplit
    m_bUseIllumCalType2
        = IniHelper::LoadINT(strIniPath, _T("Calibration"), _T("Use Illum Cal Type2"), FALSE); //kircheis_WB

    m_nCoaxialIllumMirrorCalType = IniHelper::LoadINT(strIniPath, _T("Calibration"), _T("Coaxial Ilum Mirror Cal Type"),
        IllumMirrorCalType_GrayTargetOnly); //SDY_DualCal
    m_bUsingDualCalInterpolationType
        = IniHelper::LoadINT(strIniPath, _T("Calibration"), _T("Use Dual Cal Interpolation"), TRUE); //SDY_DualCal
    m_fUseCoaxialLowCut
        = IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("Use Coaxial Low Cut"), 0.1f); //SDY_DualCal
    m_fInterpolationStart
        = IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("Interpolation Start"), 4.f); //SDY_DualCal
    m_fInterpolationEnd
        = IniHelper::LoadFLOAT(strIniPath, _T("Calibration"), _T("Interpolation End"), 5.f); //SDY_DualCal

    m_nGrabVerifyMatchingCount = IniHelper::LoadINT(
        strIniPath, _T("Setting"), _T("Grab Verify Matching Count"), 0); //kk Grab Verify Error Count
    m_bUseGrabRetry = IniHelper::LoadINT(strIniPath, _T("Setting"), _T("Use Grab Retry"), FALSE); //kircheis_3DCalcRetry

    m_bUseAiInspection = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Use2ndDLInspection"), FALSE); //ksy
    m_nDLWaitTimeSendRejectData = IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Wait DL Send Result"), 5000);

    if (m_nVisionType == VISIONTYPE_2D_INSP || m_nVisionType == VISIONTYPE_TR
        || m_nVisionType == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
    {
        //m_bUsing142VersionillumCalType =  IniHelper::LoadINT(strIniPath, _T("Hardware"), _T("Use illum Cal Type 142Version"), TRUE);
        m_bLockIllumCalLinearPlus
            = IniHelper::LoadINT(strIniPath, _T("Calibration"), _T("Lock Illum Cal. to Linear+ "), TRUE);
        m_bLockIllumCalLinearPlus = TRUE; //kircheis_IllumCalType //이 Line은 추후 지워야 된다
        m_nIlluminationCalType = IniHelper::LoadINT(strIniPath, _T("Calibration"), _T("Illumination Cal. Type"),
            IllumCalType_Linear_Plus); //kircheis_IllumCalType
        if (m_bLockIllumCalLinearPlus == TRUE)
            m_nIlluminationCalType = IllumCalType_Linear_Plus;
    }

    //m_nSaveImageTypeForNGRV = IniHelper::LoadINT(strIniPath, _T("SpecialOption"), _T("Image Save Option for NGRV"), 0);
}

void SystemConfig::SaveBackupIni()
{
    CString systemFolder = DynamicSystemPath::get(DefineFolder::System);

    CString strBackupFilepath;
    strBackupFilepath = systemFolder + _T("Backup");
    CreateDirectory(strBackupFilepath, NULL);

    // Backup File
    SYSTEMTIME st;
    GetLocalTime(&st);
    // 날짜
    CString strFileName;
    strFileName.Format(
        _T("System_%04d_%02d_%02d_%02d_%02d_%02d.ini"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    CString strPath = strBackupFilepath + _T("\\") + strFileName;
    SaveIni(strPath, FALSE);
    SaveCurrentScale(strPath);
}

void SystemConfig::SaveIni(CString strIniPath, BOOL bBackupFile) //kircheis_100M
{
    if (bBackupFile)
        SaveBackupIni();

    IniHelper::SaveINT(strIniPath, _T("Vision_Setting"), _T("Send_Host_Frame_Num"), m_nSendHostFrameNumber);
    IniHelper::SaveINT(strIniPath, _T("Vision_Setting"), _T("ReviewImageSaveDrive"), m_nReviewImageSaveDrive);
    IniHelper::SaveINT(strIniPath, _T("Vision_Setting"), _T("Surface_Reject_Report_Patch_Count_X"),
        m_nSurfaceRejectReportPatchCount_X);
    IniHelper::SaveINT(strIniPath, _T("Vision_Setting"), _T("Surface_Reject_Report_Patch_Count_Y"),
        m_nSurfaceRejectReportPatchCount_Y);

    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("Vision_Type"), m_nVisionType);
    IniHelper::SaveSTRING(strIniPath, _T("Setting"), _T("Vision_TypeName"), m_strVisionInfo);
    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("Handler_Type"), m_nHandlerType);
    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("SideVision_Number"), m_nSideVisionNumber); //kircheis_SideInsp

    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("NGRV_Camera_Type"), m_ngrvCameraType); //kircheis_NGRV_Type

    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("2D Vision Camera Type"), m_n2DVisionCameraType); //kircheis_2D Vision_Type
    IniHelper::SaveINT(
        strIniPath, _T("Setting"), _T("3D Vision Camera Type"), m_n3DVisionCameraType);
    
    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("3D Vision Camera Number"), m_n3DVisionCameraNumber);

    //{{//kircheis_GrabDir
    m_nGrabDirection = (int)min(1, m_nGrabDirection);
    m_nGrabDirection = (int)max(0, m_nGrabDirection);
    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("3DGrabDirection"), m_nGrabDirection);
    //}}

    if (m_nHandlerType == HANDLER_TYPE_380BRIDGE)
        m_nStitchGrabDirection = _STITCH_GRAB_NORMAL;
    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("StitchGrabDirection"), m_nStitchGrabDirection);

    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Hardware Exist"), m_bHardwareExist);

    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Frame Grabber Type"), m_fgType); //kircheis_SWIR
    IniHelper::SaveSTRING(strIniPath, _T("Hardware"), _T("Camera Model"), m_cameraModel); //kircheis_SWIR

    if (!PersonalConfig::getInstance().isPersonalInstallationEnabled())
    {
        // 개인용도로 설치했을 때는 기존 Config의 Hardware 세팅도 건들지 않는다
        IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Hardware Exist"), m_bHardwareExist);
        IniHelper::SaveINT(strIniPath, _T("ThreadNum"), _T("ThreadNum"), m_nThreadNum);
        IniHelper::SaveINT(strIniPath, _T("Grab"), _T("BufferNum"), m_nGrabBufferNum);
        IniHelper::SaveINT(strIniPath, _T("Setting"), _T("Vision_AccessMode"), m_nCurrentAccessMode);
    }

    if (m_nVisionType == VISIONTYPE_3D_INSP)
    {
        IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("3D Scale"),
            m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUmZ());
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Default 3D Scale X"), m_nDefaultScaleX_3DVision);
        IniHelper::SaveINT(strIniPath, _T("SyncPort"), _T("SyncPort"), m_nSyncPort);

        IniHelper::SaveSTRING(strIniPath, _T("Hardware"), _T("3D Camera Serial Number"), m_str3DCameraSN);
        IniHelper::SaveINT(
            strIniPath, _T("Hardware"), _T("3D Camera Analog Offset1 Origin"), m_n3DCameraAnalogOffset1Origin);
        IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("3D Camera Analog Offset1"), m_n3DCameraAnalogOffset1);
        IniHelper::SaveINT(
            strIniPath, _T("Hardware"), _T("3D Camera Analog Offset2 Origin"), m_n3DCameraAnalogOffset2Origin);
        IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("3D Camera Analog Offset2"), m_n3DCameraAnalogOffset2);
        IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("3D Grab Time Out Buffer(msec)"),
            m_n3DGrabTimeOutBufferTime); //kircheis_20230308

        IniHelper::SaveINT(
            strIniPath, _T("Hardware"), _T("Use 3D Camera Noise Check"), m_bUse3DCameraNoiseTest); // SDY 3D_Noise
        IniHelper::SaveFLOAT(strIniPath, _T("Hardware"), _T("SpaceTimeSamplingScaleFactor"),
            m_fSpaceTimeSamplingScaleFactor); //mc_SpaceTimeSamplingScaleFactor
    }

    IniHelper::SaveSTRING(strIniPath, _T("Setting"), _T("Prev Job Name"), m_strPrevJob);

    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("GrabDuration"), m_nGrabDuration);
    IniHelper::SaveINT(
        strIniPath, _T("Setting"), _T("CameraTransferTime"), m_nCameraTransferTimeMS); //kircheis_CamTrans

    IniHelper::SaveFLOAT(
        strIniPath, _T("Setting"), _T("m_slitbeamIlluminationAngle_deg"), m_slitbeamIlluminationAngle_deg);
    IniHelper::SaveFLOAT(strIniPath, _T("Setting"), _T("m_slitbeamCameraAngle_deg"), m_slitbeamCameraAngle_deg);
    IniHelper::SaveFLOAT(strIniPath, _T("Setting"), _T("m_slitbeamImage_px2um_y"), m_slitbeamImage_px2um_y);
    IniHelper::SaveFLOAT(strIniPath, _T("Setting"), _T("m_slitbeamHeightScaling"), m_slitbeamHeightScaling);
    IniHelper::SaveDOUBLE(strIniPath, _T("Setting"), _T("m_slitbeamScanSpeedDownRatio"), m_slitbeamScanSpeedDownRatio);
    IniHelper::SaveDOUBLE(strIniPath, _T("Setting"), _T("m_slitbeamIlluminationGain"), m_slitbeamIlluminationGain);
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("SlitbeamCameraOffsetY"), m_slitbeamCameraOffsetY);
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("SlitbeamUseEncoderMultiply"), m_slitbeamUseEncoderMultiply);
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("SlitbeamCameraRotation"), m_slitbeamCameraRotation);
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("SlitbeamOpticsReversal"), m_slitbeamOpticsReversal);

    IniHelper::SaveFLOAT(strIniPath, _T("Setting"), _T("3D_Illumination_Gain"), m_f3DIllumGain); //kircheis_3DLensIssue

    //	IniHelper::SaveINT(strIniPath, _T("Setting"), _T("Vision_MarkMultiTeaching"), m_bUseMarkMultiTeaching);

    //	IniHelper::SaveINT(strIniPath, _T("Warning"), _T("UseMarkMatchRateAlgoritm2"), m_bUseMarkMatchRateAlgoritm2); //kircheis_MatchRate 수정

    //	IniHelper::SaveINT(strIniPath, _T("SiteOption"), _T("Use Mark Teach Result View"), m_bUseMarkTeachResultView);//kircheis_HwaMark
    //}}

    IniHelper::SaveINT(strIniPath, _T("SpecialOption"), _T("UseMarkTeachImageSplitSave"),
        m_bUseMarkTeachImageSplitSave); //kirchies_MarkTeachSplit

    IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Use Illum Cal Type2"), m_bUseIllumCalType2); //kircheis_WB

    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("Grab Verify Matching Count"), m_nGrabVerifyMatchingCount);

    IniHelper::SaveINT(strIniPath, _T("Setting"), _T("Use Grab Retry"), m_bUseGrabRetry); //kircheis_3DCalcRetry

    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Use2ndDLInspection"), m_bUseAiInspection);
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Wait DL Send Result"), m_nDLWaitTimeSendRejectData);

    if (m_nVisionType == VISIONTYPE_2D_INSP || m_nVisionType == VISIONTYPE_SWIR_INSP) //kircheis_SWIR
    {
        //IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Use illum Cal Type 142Version"), m_bUsing142VersionillumCalType);
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Lock Illum Cal. to Linear+ "), m_bLockIllumCalLinearPlus);
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Illumination Cal. Type"),
            m_nIlluminationCalType); //kircheis_IllumCalType
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Coaxial Ilum Mirror Cal Type"),
            m_nCoaxialIllumMirrorCalType); //SDY_DualCal
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Use Dual Cal Interpolation"),
            m_bUsingDualCalInterpolationType); //SDY_DualCal
        IniHelper::SaveFLOAT(
            strIniPath, _T("Calibration"), _T("Use Coaxial Low Cut"), m_fUseCoaxialLowCut); //SDY_DualCal
        IniHelper::SaveFLOAT(
            strIniPath, _T("Calibration"), _T("Interpolation Start"), m_fInterpolationStart); //SDY_DualCal
        IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("Interpolation End"), m_fInterpolationEnd); //SDY_DualCal

        //mc_2DID Cropping image Save Option
        IniHelper::SaveINT(
            strIniPath, _T("Hardware"), _T("2DID Cropping image SaveOption Index"), m_n2DIDCroppingimageSaveOptionidx);

        IniHelper::SaveINT(
            strIniPath, _T("Hardware"), _T("Exist Ring Illumination"), m_bExistRingillumiation); //mc_Ring illum.
    }
    else if (m_nVisionType == VISIONTYPE_NGRV_INSP)
    {
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Coaxial Ilum Mirror Cal Type"),
            m_nCoaxialIllumMirrorCalType); //SDY_DualCal
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Use Dual Cal Interpolation"),
            m_bUsingDualCalInterpolationType); //SDY_DualCal
        IniHelper::SaveFLOAT(
            strIniPath, _T("Calibration"), _T("Use Coaxial Low Cut"), m_fUseCoaxialLowCut); //SDY_DualCal
        IniHelper::SaveFLOAT(
            strIniPath, _T("Calibration"), _T("Interpolation Start"), m_fInterpolationStart); //SDY_DualCal
        IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("Interpolation End"), m_fInterpolationEnd); //SDY_DualCal
    }
    else if (m_nVisionType == VISIONTYPE_SIDE_INSP)
    {
        IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("SideCameraOffsetY_Front"), m_nSideVisionFrontCameraOffsetY);
        IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("SideCameraOffsetY_Rear"), m_nSideVisionRearCameraOffsetY);
    }
    else if (m_nVisionType == VISIONTYPE_TR)
    {
        //IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Use illum Cal Type 142Version"), m_bUsing142VersionillumCalType);
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Lock Illum Cal. to Linear+ "), m_bLockIllumCalLinearPlus);
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Illumination Cal. Type"),
            m_nIlluminationCalType); //kircheis_IllumCalType
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Coaxial Ilum Mirror Cal Type"),
            m_nCoaxialIllumMirrorCalType); //SDY_DualCal
        IniHelper::SaveINT(strIniPath, _T("Calibration"), _T("Use Dual Cal Interpolation"),
            m_bUsingDualCalInterpolationType); //SDY_DualCal
        IniHelper::SaveFLOAT(
            strIniPath, _T("Calibration"), _T("Use Coaxial Low Cut"), m_fUseCoaxialLowCut); //SDY_DualCal
        IniHelper::SaveFLOAT(
            strIniPath, _T("Calibration"), _T("Interpolation Start"), m_fInterpolationStart); //SDY_DualCal
        IniHelper::SaveFLOAT(strIniPath, _T("Calibration"), _T("Interpolation End"), m_fInterpolationEnd); //SDY_DualCal

        //mc_2DID Cropping image Save Option
        IniHelper::SaveINT(
            strIniPath, _T("Hardware"), _T("2DID Cropping image SaveOption Index"), m_n2DIDCroppingimageSaveOptionidx);

        IniHelper::SaveINT(
            strIniPath, _T("Hardware"), _T("Exist Ring Illumination"), m_bExistRingillumiation); //mc_Ring illum.

        m_tr.SaveIni(strIniPath);
    }
    //IniHelper::SaveINT(strIniPath, _T("SpecialOption"), _T("Image Save Option for NGRV"), m_nSaveImageTypeForNGRV);

    //{{ Color Gain
    IniHelper::SaveFLOAT(strIniPath, _T("Color Option"), _T("Normal Channel Red Gain"), m_fNormal_Channel_GainR);
    IniHelper::SaveFLOAT(strIniPath, _T("Color Option"), _T("Normal Channel Green Gain"), m_fNormal_Channel_GainG);
    IniHelper::SaveFLOAT(strIniPath, _T("Color Option"), _T("Normal Channel Blue Gain"), m_fNormal_Channel_GainB);

    IniHelper::SaveFLOAT(strIniPath, _T("Color Option"), _T("Reverse Channel Red Gain"), m_fReverse_Channel_GainR);
    IniHelper::SaveFLOAT(strIniPath, _T("Color Option"), _T("Reverse Channel Green Gain"), m_fReverse_Channel_GainG);
    IniHelper::SaveFLOAT(strIniPath, _T("Color Option"), _T("Reverse Channel Blue Gain"), m_fReverse_Channel_GainB);
    //}}

    //{{//kircheis_LongExp
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("Use long exposure for NGRV"), (int)m_bUseLongExposureNGRV);
    IniHelper::SaveINT(strIniPath, _T("SpecialOption"), _T("Use Bayer Pattern GPU"), m_bUseBayerPatternGPU); // JHB
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("NGRV IR Channel ID(1~16)"),
        (int)(m_nNgrvIRchID + 1)); //1을 더하는 이유는 System.ini에는 1~16이나
    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("NGRV UV Channel ID(1~16)"),
        (int)(m_nNgrvUVchID + 1)); //Code 내부에서는 0~15이므로
    //}}

    //{{ Camera Gains
    IniHelper::SaveFLOAT(strIniPath, _T("Hardware"), _T("NGRV Default Channel Gain"), m_fDefault_Camera_Gain);
    IniHelper::SaveFLOAT(strIniPath, _T("Hardware"), _T("NGRV IR Channel Gain"), m_fIR_Camera_Gain);
    IniHelper::SaveFLOAT(strIniPath, _T("Hardware"), _T("NGRV UV Channel Gain"), m_fUV_Camera_Gain);

    IniHelper::SaveINT(strIniPath, _T("Hardware"), _T("NGRV ByPass IR"), m_bIsByPassIRBIT);
    //}}
}

BOOL SystemConfig::IsHardwareExist() const
{
    return m_bHardwareExist;
}

int SystemConfig::GetVisionType() const
{
    return m_nVisionType;
}

int SystemConfig::Get2DVisionCameraType() const
{
    return m_n2DVisionCameraType;
}

int SystemConfig::Get3DVisionCameraType() const
{
    return m_n3DVisionCameraType;
}

int SystemConfig::Get3DVisionCameraNumber() const
{
    return m_n3DVisionCameraNumber;
}

bool SystemConfig::IsDual3DGrabMode() const
{
    static const bool is3DVision = IsVisionType3D();
    static const bool isFG_iGrabXQ = IsFrameGrabberTypeiGrabXQ();
    static const bool isDualCamera = m_n3DVisionCameraNumber > 1 ? true : false;

    static const bool isDualGrabMode = (is3DVision && isFG_iGrabXQ && isDualCamera);

    return isDualGrabMode;
}

CString SystemConfig::GetVisionTypeText() const
{
    return g_szVisionTypeText[GetVisionType()];
}

CString SystemConfig::GetVisionTypeText(int visionType) const
{
    if (visionType < VISIONTYPE_START || visionType >= VISIONTYPE_END)
        return _T("Unknown");

    return g_szVisionTypeText[visionType];
}

int SystemConfig::IsVisionType2D() const
{
    return (m_nVisionType == VISIONTYPE_2D_INSP);
} //kircheis_3DImageSizeIssue
int SystemConfig::IsVisionType3D() const
{
    return (m_nVisionType == VISIONTYPE_3D_INSP);
} //kircheis_3DImageSizeIssue
int SystemConfig::IsVisionTypeNGRV() const
{
    return (m_nVisionType == VISIONTYPE_NGRV_INSP);
} //JHB_NGRV
int SystemConfig::IsVisionTypeSide() const
{
    return (m_nVisionType == VISIONTYPE_SIDE_INSP);
} //kircheis_SideInsp
int SystemConfig::IsVisionTypeSWIR() const //kircheis_SWIR
{
    return (m_nVisionType == VISIONTYPE_SWIR_INSP);
} //kircheis_SWIR
bool SystemConfig::IsVisionTypeTR() const
{
    return (m_nVisionType == VISIONTYPE_TR) ? true : false;
}

bool SystemConfig::IsVisionType2DorSide() const
{
    static const bool bIsVision2D = (bool)(IsVisionType2D() == 1 ? true : false);
    static const bool bIsVisionSide = (bool)(IsVisionTypeSide() == 1 ? true : false);
    static const bool bIsVisionTR = (bool)(IsVisionTypeTR() == 1 ? true : false);
    return bool(bIsVision2D || bIsVisionSide || bIsVisionTR);
}
bool SystemConfig::IsVisionTypeBased2D() const
{
    static const bool bIsVision2D = (bool)(IsVisionType2D() == 1 ? true : false);
    static const bool bIsVisionSide = (bool)(IsVisionTypeSide() == 1 ? true : false);
    static const bool bIsVisionSWIR = (bool)(IsVisionTypeSWIR() == 1 ? true : false); //kircheis_SWIR
    static const bool bIsVisionTR = (bool)(IsVisionTypeTR() == 1 ? true : false);
    //static const bool bIsVisionXXX = (bool)(IsVisionTypeXXX() == 1 ? true : false);
    return bool(bIsVision2D || bIsVisionSide || bIsVisionTR || bIsVisionSWIR); //kircheis_SWIR
}
int SystemConfig::GetHandlerType() const
{
    return m_nHandlerType;
}
const VisionScale& SystemConfig::GetScale(const enSideVisionModule i_eSideVisionModule) const
{
    return m_scale[i_eSideVisionModule];
}
float SystemConfig::Get2DScaleX(const enSideVisionModule i_eSideVisionModule) const
{
    return m_scale[i_eSideVisionModule].pixelToUm().m_x;
}
float SystemConfig::Get2DScaleY(const enSideVisionModule i_eSideVisionModule) const
{
    return m_scale[i_eSideVisionModule].pixelToUm().m_y;
}
float SystemConfig::Get3DImageVerticalScale_px2um() const
{
    return m_slitbeamImage_px2um_y;
}

int SystemConfig::GetSideVisionNumber() const //kircheis_SideInsp
{
    if (IsVisionTypeSide() == FALSE)
        return SIDE_VISIONNUMBER_NOSIDEVISION;

    return m_nSideVisionNumber;
}
void SystemConfig::SetSideVisionNumber(
    const int i_nSideVisionNumber) //kircheis_SideInsp //VisionType이 Side가 아니면 입력값에 관계 없이 NoSide 적용
{
    if (IsVisionTypeSide() == FALSE || i_nSideVisionNumber < SIDE_VISIONNUMBER_START
        || i_nSideVisionNumber >= SIDE_VISIONNUMBER_END)
    {
        m_nSideVisionNumber = SIDE_VISIONNUMBER_NOSIDEVISION;
        return;
    }
    m_nSideVisionNumber = i_nSideVisionNumber;
}

int SystemConfig::GetSideVisionFrontCameraOffsetY()
{
    return m_nSideVisionFrontCameraOffsetY;
}

void SystemConfig::SetSideVisionFrontCameraOffsetY(int nFrontCameraOffsetY)
{
    m_nSideVisionFrontCameraOffsetY = nFrontCameraOffsetY;
}
int SystemConfig::GetSideVisionRearCameraOffsetY()
{
    return m_nSideVisionRearCameraOffsetY;
}

void SystemConfig::SetSideVisionRearCameraOffsetY(int nRearCameraOffsetY)
{
    m_nSideVisionRearCameraOffsetY = nRearCameraOffsetY;
}

int SystemConfig::GetThreadNum()
{
    static const long visionType = SystemConfig::GetInstance().GetVisionType();
    if (visionType == VISIONTYPE_TR)
        return m_nThreadNum;

    if (PersonalConfig::getInstance().isPersonalInstallationEnabled())
    {
        auto personalCoreCount = PersonalConfig::getInstance().getMultiCoreCount();
        if (personalCoreCount < 2)
            return 0;

        return personalCoreCount;
    }

    if (SystemConfig::GetInstance().GetVisionType()
        == VISIONTYPE_NGRV_INSP) // NGRV vision이면 Thread는 항상 1개로 실행한다 - JHB_NGRV
        return 1;

    //if (!m_bHardwareExist)
    //    return 0;

    return m_nThreadNum;
}

int SystemConfig::GetGrabBufferNum(bool actualValueUsed)
{
    if (!actualValueUsed)
    {
        return m_nGrabBufferNum;
    }

    return max(m_nGrabBufferNum, GetThreadNum());
}

int SystemConfig::GetGrabDuration() const
{
    return m_nGrabDuration;
}
int SystemConfig::GetSyncPort()
{
    return m_nSyncPort;
}

float SystemConfig::GetScale3D()
{
    return m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].pixelToUmZ();
}

void SystemConfig::GetColorGains(float& o_fNormalChannelGainR, float& o_fNormalChannelGainG,
    float& o_fNormalChannelGainB, float& o_fReverseChannelGainR, float& o_fReverseChannelGainG,
    float& o_fReverseChannelGainB)
{
    o_fNormalChannelGainR = m_fNormal_Channel_GainR;
    o_fNormalChannelGainG = m_fNormal_Channel_GainG;
    o_fNormalChannelGainB = m_fNormal_Channel_GainB;

    o_fReverseChannelGainR = m_fReverse_Channel_GainR;
    o_fReverseChannelGainG = m_fReverse_Channel_GainG;
    o_fReverseChannelGainB = m_fReverse_Channel_GainB;
}

void SystemConfig::SetScale3D(float fScale)
{
    m_scale[enSideVisionModule::SIDE_VISIONMODULE_FRONT].setScaleZ(fScale);
}
void SystemConfig::SetThreadNum(int nThreadNum)
{
    m_nThreadNum = nThreadNum;
}
void SystemConfig::SetGrabBufferNum(int nGrabBufferNum)
{
    m_nGrabBufferNum = nGrabBufferNum;
}

void SystemConfig::SetPrevJob(CString strPrevJob)
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);
    m_strPrevJob = strPrevJob;

    IniHelper::SaveSTRING(systemFilePath, _T("Setting"), _T("Prev Job Name"), m_strPrevJob);
}

CString SystemConfig::GetPrevJob()
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);
    m_strPrevJob = IniHelper::LoadSTRING(systemFilePath, _T("Setting"), _T("Prev Job Name"), _T(""));

    return m_strPrevJob;
}

void SystemConfig::SetVisionInfo(CString strVisionInfo, long nVisionInfo_NumType)
{
    m_strVisionInfo = strVisionInfo;
    m_nVisionInfo_NumType = nVisionInfo_NumType;

    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);
    CFileFind ff;
    if (ff.FindFile(systemFilePath) == FALSE)
    {
        SaveIni(systemFilePath, FALSE);
    }
}

BOOL SystemConfig::getUsing142VersionillumCalType()
{
    return m_bUsing142VersionillumCalType;
}

BOOL SystemConfig::IsBtm2DVision() //kircheis_NGRV_ByPass
{
    return (m_nVisionInfo_NumType == VISION_BTM_2D);
}

BOOL SystemConfig::IsTop2DVision() //kircheis_NGRVAF
{
    return (m_nVisionInfo_NumType == VISION_TOP_2D);
}

BOOL SystemConfig::IsBtm3DVision() //kircheis_NGRVAF
{
    return (m_nVisionInfo_NumType == VISION_BTM_3D);
}

BOOL SystemConfig::IsTop3DVision() //kircheis_NGRVAF
{
    return (m_nVisionInfo_NumType == VISION_TOP_3D);
}

BOOL SystemConfig::IsReverseChannel() // JHB_NGRV Reverse Channel Usage
{
    return m_bUseReverseChannel;
}

void SystemConfig::Set_CPU_CALC_SAVE_LOG(const bool i_bSaveEnable)
{
    m_bSave_CPU_CalcLog = i_bSaveEnable;
}

const bool SystemConfig::Get_Enable_CPU_CALC_SAVE_LOG()
{
    return m_bSave_CPU_CalcLog;
}

void SystemConfig::Set_GPU_CALC_SAVE_LOG(const bool i_bSaveEnable)
{
    m_bSave_GPU_CalcLog = i_bSaveEnable;
}

const bool SystemConfig::Get_Enable_GPU_CALC_SAVE_LOG()
{
    return m_bSave_GPU_CalcLog;
}

void SystemConfig::Set_CPU_SavebyInspectionCount(const long i_nInspectionCount)
{
    if (i_nInspectionCount > 0)
        m_nCPU_SavebyInspectionCount = i_nInspectionCount;
    else
        return;
}
void SystemConfig::Set_CPU_SaveReportMaximumCount(const long i_nMaximumReportCount)
{
    if (i_nMaximumReportCount > 0)
        m_nCPU_SaveReportMaximumCount = i_nMaximumReportCount;
    else
        return;
}

const long SystemConfig::Get_CPU_SavebyInspectionCount()
{
    return m_nCPU_SavebyInspectionCount;
}

const long SystemConfig::Get_CPU_SaveReportMaximumCount()
{
    return m_nCPU_SaveReportMaximumCount;
}

void SystemConfig::SetVisionInspTime(const CString i_strVisionInspTime)
{
    m_vecstrVisionInspTime.push_back(i_strVisionInspTime);
}

void SystemConfig::ResetVisionInspTime()
{
    m_vecstrVisionInspTime.clear();
}

void SystemConfig::SaveVisionInspTime(const long i_nReportidx)
{
    if (m_vecstrVisionInspTime.size() <= 0)
        return;

    CString strCurrentVisionInfo("");
    strCurrentVisionInfo.Format(
        _T("%s_InspTimeLog_%05d.csv"), (LPCTSTR)(SystemConfig::GetInstance().m_strVisionInfo), i_nReportidx);

    CString strSaveFilePath = DynamicSystemPath::get(DefineFolder::Config) + strCurrentVisionInfo;
    FILE* fp = nullptr;

    _tfopen_s(&fp, strSaveFilePath, _T("w"));

    if (fp == nullptr)
        return;

    fprintf(fp, "Trayindex,ScanID,PaneID,ThreadID,InspTime[m/s]\n");

    for (auto VisionInsp_Time : m_vecstrVisionInspTime)
    {
        CString strConvert = VisionInsp_Time + _T("\n");
        CStringA str(strConvert);
        fprintf(fp, str);
    }

    fclose(fp);
}

void SystemConfig::SaveIniIllumCalType() //kircheis_IllumCalType
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);
    IniHelper::SaveINT(systemFilePath, _T("Calibration"), _T("Illumination Cal. Type"), m_nIlluminationCalType);
}

void SystemConfig::logger_memcheck()
{
    CFile File;
    CString strFileName("");
    CString strFileSaveFolder = DynamicSystemPath::get(DefineFolder::Log) + _T("MemoryLOG\\");

    Ipvm::CreateDirectories(strFileSaveFolder);
    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    strFileName.Format(_T("%sVisionMemoryLog(%s)_%s.txt"), (LPCTSTR)strFileSaveFolder,
        (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo, (LPCTSTR)CTime::GetCurrentTime().Format(_T("%Y%m%d")));

    if (m_nAutomationMode == 1)
    {
        strFileSaveFolder.Format(_T("%sAutomationTemp\\%s\\"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)),
            (LPCTSTR)(((CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName))
                .Left(SystemConfig::GetInstance().m_strAutomationRecipeFileName.ReverseFind('.')));
        Ipvm::CreateDirectories(LPCTSTR(strFileSaveFolder)); // 결과 폴더 생성
        strFileName.Format(_T("%sVisionMemoryLog.txt"), (LPCTSTR)strFileSaveFolder);
    }

    if (!File.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
        return;
    else
    {
        // 파일의 맨 마지막을 찾는다.
        File.SeekToEnd();

        PROCESS_MEMORY_COUNTERS_EX pmc;
        GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
        SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

        CString memoryInfo;
        float kilobyte = (float)((float)virtualMemUsedByMe / 1024);
        float megabyte = (float)(kilobyte / 1024);
        float gigabyte = (float)(megabyte / 1024);

        memoryInfo.Format(_T("%u BYTE (%.3f KB, %.3f MB, %.3f GB)"), virtualMemUsedByMe, kilobyte, megabyte, gigabyte);

        CString strWriteTime("");
        CString strWriteData("");
        strWriteTime.Format(_T("[%s]\t"), (LPCTSTR)CTime::GetCurrentTime().Format(_T("%H:%M:%S")));
        strWriteData.Format(_T("[iPIS-500] %s"), (LPCTSTR)memoryInfo);

        CArchive ar(&File, CArchive::store);
        ar.WriteString(strWriteTime);
        //ar.WriteString(_T("\t"));
        ar.WriteString(strWriteData);
        //ar.WriteString(_T("\t"));

        ar.WriteString(_T("\r\n"));
        ar.Close();
        File.Close();
    }
}

void SystemConfig::Set_SaveSequenceAndInspectionTimeLog(const bool i_bSavEnable)
{
    m_bSaveSequenceAndInspectionTimeLog = i_bSavEnable;
}

const bool SystemConfig::Get_SaveSequenceAndInspectionTimeLog()
{
    return m_bSaveSequenceAndInspectionTimeLog;
}

void SystemConfig::Set_LotIDFromHost(const CString i_strLotID)
{
    m_strLotIDFromHost = i_strLotID;
}

CString SystemConfig::Get_Lot_IDFromHost()
{
    return m_strLotIDFromHost;
}

void SystemConfig::Set_Lot_Start_TimeFromHost(const CTime i_timeLotStart)
{
    m_cTime_Lot_StartFromHost = i_timeLotStart;
}

CTime SystemConfig::Get_Lot_Start_TimeFromHost()
{
    return m_cTime_Lot_StartFromHost;
}

void SystemConfig::Set_TimeLog_SaveFolderPath(const CString i_strSaveFolderPath)
{
    m_strTimeLog_SaveFolderPath = i_strSaveFolderPath;
}

void SystemConfig::Replace_TimeLog_Drive(const CString i_strDriveName)
{
    CString strSaveFolderPath = m_strTimeLog_SaveFolderPath;

    int nRemoveDriveNameidx = strSaveFolderPath.Find(_T(":"));

    strSaveFolderPath.Delete(0, nRemoveDriveNameidx);

    strSaveFolderPath.Insert(0, i_strDriveName);

    m_strTimeLog_SaveFolderPath = strSaveFolderPath;
}

CString SystemConfig::Get_TimeLog_SaveFolderPath()
{
    return m_strTimeLog_SaveFolderPath;
}

void SystemConfig::Set_TimeLog_SavePath(const CString i_strSavePath)
{
    m_strTimeLog_SavePath = i_strSavePath;
}

CString SystemConfig::Get_Time_Log_SavePath()
{
    return m_strTimeLog_SavePath;
}

void SystemConfig::Save_SequenceTimeLog(const CString i_strLog)
{
    if (Get_SaveSequenceAndInspectionTimeLog() == false)
        return;

    CFile File;
    CString strFileName("");
    CString strFileSaveFolder = DynamicSystemPath::get(DefineFolder::TimeLog) + _T("SequenceLog\\");
    CString strFileSaveFolder_LotID = strFileSaveFolder + Get_Lot_IDFromHost() + _T("\\");

    Ipvm::CreateDirectories(strFileSaveFolder);
    Ipvm::CreateDirectories(strFileSaveFolder_LotID);

    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    strFileName.Format(_T("%sVisionSequenceLog(%s)_%s.txt"), (LPCTSTR)strFileSaveFolder_LotID,
        (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo, (LPCTSTR)CTime::GetCurrentTime().Format(_T("%Y%m%d")));

    if (!File.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
        return;
    else
    {
        SYSTEMTIME cur_time;
        GetLocalTime(&cur_time);

        // 파일의 맨 마지막을 찾는다.
        File.SeekToEnd();

        CString strWriteTime("");
        CString strWriteData("");
        strWriteTime.Format(
            _T("[%02d:%02d:%02d:%03ld]\t"), cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);

        CArchive ar(&File, CArchive::store);
        ar.WriteString(strWriteTime);
        ar.WriteString(_T("\t"));
        ar.WriteString(i_strLog);

        ar.WriteString(_T("\r\n"));
        ar.Close();
        File.Close();
    }
}

UINT SystemConfig::GetIllumType(long nIllumNum) //SDY_DualCal 조명의 타입을 리턴하는 함수
{
    if (SystemConfig::IsVisionType2D())
    {
        if (nIllumNum < LED_ILLUM_CHANNEL_OBLIQUE)
        {
            return IllumType_Oblique;
        }
        else if (nIllumNum < LED_ILLUM_CHANNEL_DEFAULT)
        {
            return IllumType_Coaxial;
        }
        else
        {
            return IllumType_Ring;
        }
    }
    else if (SystemConfig::IsVisionTypeNGRV())
    {
        if (nIllumNum == m_nNgrvCoaxialID)
        {
            return IllumType_Coaxial;
        }
        else if (nIllumNum == m_nNgrvUVchID)
        {
            return IllumType_UV;
        }
        else if (nIllumNum == m_nNgrvIRchID)
        {
            return IllumType_IR;
        }
        else
        {
            return IllumType_Oblique;
        }
    }
    else if (SystemConfig::IsVisionTypeSide())
    {
        if (nIllumNum < LED_ILLUM_CHANNEL_SIDE_OBLIQUE)
        {
            return IllumType_Oblique;
        }
        else
        {
            return IllumType_Coaxial;
        }
    }
    else if (SystemConfig::IsVisionTypeSWIR()) //kircheis_SWIR
    {
        if (nIllumNum < 4) //LED_ILLUM_CHANNEL_SWIR_OBLIQUE//NeedRefac
        {
            return IllumType_Oblique;
        }
        else
        {
            return IllumType_Coaxial;
        }
    }

    return IllumType_Oblique;
}

void SystemConfig::SetVisionCondition(const eVisionCondition i_eVisionCondition)
{
    m_nVisionCondition = i_eVisionCondition;
}

long SystemConfig::GetVisionCondition()
{
    return m_nVisionCondition;
}

void SystemConfig::SetiGrab_Board_Temperature_Log_save_Vision(const bool i_bisSave)
{
    m_biGrab_Board_Temperature_Log_save = i_bisSave;
}

bool SystemConfig::GetiGrab_Board_Temperature_Log_save_Vision()
{
    return m_biGrab_Board_Temperature_Log_save;
}

void SystemConfig::SetInspectionResultTimeoutSaveRawimage(const bool i_bEnableSave)
{
    m_bInspectionResultTimeoutSaveRawimage = i_bEnableSave;
}

bool SystemConfig::GetInspectionResultTimeoutSaveRawimage()
{
    return m_bInspectionResultTimeoutSaveRawimage;
}

void SystemConfig::SetInspectionResultTimeoutTime_ms(const long i_nTImeoutTime_ms)
{
    m_nInsepctionTimeoutTime_ms = i_nTImeoutTime_ms;
}

long SystemConfig::GetInspectionResultTimeoutTime_ms()
{
    return m_nInsepctionTimeoutTime_ms;
}

void SystemConfig::SetGrabTimeoutCalcParam(const float i_fMotorSpeedMMperSEC, const float i_fScanLengthMM)
{
    m_fSharedInfoMotorSpeedMMperSEC = i_fMotorSpeedMMperSEC;
    m_fSharedInfoScanLengthMM = i_fScanLengthMM;
}

DWORD SystemConfig::GetGrabTimeoutTime()
{
    static const DWORD dwGrabTimeOutBufferTimeMSEC = m_n3DGrabTimeOutBufferTime;

    return (DWORD)((m_fSharedInfoScanLengthMM / m_fSharedInfoMotorSpeedMMperSEC) * 1100.f + 0.5f)
        + dwGrabTimeOutBufferTimeMSEC;
}

//{{mc_2023.03.14_H/W Info.
float SystemConfig::Get_UsageMemoryData(const eMemoryType i_eMemoryType, const eMemoryDataType i_eeMemoryDataType)
{
    switch (i_eMemoryType)
    {
        case eMemoryType::eMemoryType_PC:
        {
            //바이트 단위
            MEMORYSTATUS ms;
            GlobalMemoryStatus(&ms);

            // ms.dwMemoryLoad		//사용되고있는 메모리 퍼센트
            // ms.dwTotalPhys		//전체 물리적 메모리 크기
            // ms.dwAvailPhys		//남아있는 물리적 메모리
            // ms.dwTotalVirtual	//전체 가상 메모리
            // ms.dwAvailVirtual	//남아있는 가상 메모리

            if (i_eeMemoryDataType == eMemoryDataType::eMemoryDataType_VirtualMemory)
            {
                //mc_이거는 dwAvailPhys 이 변수를 사용하는게맞을꺼 같은데..
                //SIZE_T virtualMemUsedByMe = ms.dwTotalVirtual - ms.dwAvailVirtual;
                SIZE_T virtualMemUsedByMe = ms.dwTotalPhys - ms.dwAvailPhys;

                float kilobyte = (float)((float)virtualMemUsedByMe / 1024);
                float megabyte = (float)(kilobyte / 1024);
                float gigabyte = (float)(megabyte / 1024);

                return gigabyte;
            }
            else if (i_eeMemoryDataType == eMemoryDataType::eMemoryDataType_PhysicalMemory)
            {
                SIZE_T PhysicalMemUsedByMe = ms.dwTotalPhys - ms.dwAvailPhys;

                float kilobyte = (float)((float)PhysicalMemUsedByMe / 1024);
                float megabyte = (float)(kilobyte / 1024);
                float gigabyte = (float)(megabyte / 1024);

                return gigabyte;
            }
            else
                return -1.f;
        }
        break;
        case eMemoryType::eMemoryType_iPack:
        {
            PROCESS_MEMORY_COUNTERS_EX pmc;
            GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
            SIZE_T virtualMemUsedByMe = pmc.PrivateUsage;

            float kilobyte = (float)((float)virtualMemUsedByMe / 1024);
            float megabyte = (float)(kilobyte / 1024);
            float gigabyte = (float)(megabyte / 1024);

            return gigabyte;
        }
        break;
        default:
            return -1.f;
    }
}

float SystemConfig::Get_CPU_InfoData(const eCPUInfoDataType i_eCPUInfoDataType)
{
    switch (i_eCPUInfoDataType)
    {
        case eCPUInfoDataType::eCPUInfoDataType_CPU_Usage:
        {
            PDH_HQUERY cpuQuery;
            PDH_HCOUNTER cpuTotal;
            PdhOpenQuery(NULL, NULL, &cpuQuery);
            PdhAddCounter(cpuQuery, L"\\Processor(_Total)\\% Processor Time", NULL, &cpuTotal);
            PdhCollectQueryData(cpuQuery);
            PDH_FMT_COUNTERVALUE counterVal;

            const long nRepeatMaxCount = 2;

            double dCPU_usagePercent(-1.);

            for (long nidx = 0; nidx < nRepeatMaxCount; nidx++)
            {
                PdhCollectQueryData(cpuQuery);
                PdhGetFormattedCounterValue(cpuTotal, PDH_FMT_DOUBLE, NULL, &counterVal);
                dCPU_usagePercent = counterVal.doubleValue;

                Sleep(1000);
            }

            return (float)dCPU_usagePercent;
        }
        break;
        case eCPUInfoDataType::eCPUInfoDataType_CPU_Temperature:
        {
            float fTemperature(-1.f);
            HRESULT ci = CoInitializeEx(0, COINIT_MULTITHREADED);
            //HRESULT hr = CoInitializeSecurity(NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
            HRESULT hr = CoInitializeSecurity(
                NULL, -1, NULL, NULL, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE, NULL);
            if (SUCCEEDED(hr))
            {
                IWbemLocator* pLocator;
                hr = CoCreateInstance(
                    CLSID_WbemAdministrativeLocator, NULL, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLocator);
                if (SUCCEEDED(hr))
                {
                    IWbemServices* pServices;
                    BSTR ns = SysAllocString(L"root\\WMI");
                    hr = pLocator->ConnectServer(ns, NULL, NULL, NULL, 0, NULL, NULL, &pServices);
                    pLocator->Release();
                    SysFreeString(ns);
                    if (SUCCEEDED(hr))
                    {
                        BSTR query = SysAllocString(L"SELECT * FROM MSAcpi_ThermalZoneTemperature");
                        BSTR wql = SysAllocString(L"WQL");
                        IEnumWbemClassObject* pEnum;
                        hr = pServices->ExecQuery(
                            wql, query, WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pEnum);
                        SysFreeString(wql);
                        SysFreeString(query);
                        pServices->Release();
                        if (SUCCEEDED(hr))
                        {
                            IWbemClassObject* pObject;
                            ULONG returned;
                            hr = pEnum->Next(WBEM_INFINITE, 1, &pObject, &returned);
                            pEnum->Release();
                            if (SUCCEEDED(hr))
                            {
                                BSTR temp = SysAllocString(L"CurrentTemperature");
                                VARIANT v;
                                VariantInit(&v);
                                hr = pObject->Get(temp, 0, &v, NULL, NULL);
                                pObject->Release();
                                SysFreeString(temp);
                                if (SUCCEEDED(hr))
                                {
                                    fTemperature = (float)((V_I4(&v) / 10) - 273.15);
                                }
                                VariantClear(&v);
                            }
                        }
                    }
                    if (ci == S_OK)
                    {
                        CoUninitialize();
                    }
                }

                return fTemperature;
            }
            return CAST_FLOAT(hr);
        }
        break;
        default:
            return -1.f;
    }
}
//}}

void SystemConfig::Save_DeviceSendResultTimeLog(const long i_nTrayID, const long i_nPocketID, const long i_nPaneID,
    const SYSTEMTIME i_inspectionStartTime, const BOOL i_bSendSuccess, const bool i_bSaveLog)
{
    if (i_bSaveLog == false)
        return;

    CFile File;
    CString strFileName("");
    CString strFileSaveFolder = DynamicSystemPath::get(DefineFolder::TimeLog) + _T("SequenceLog\\");
    CString strFileSaveFolder_LotID = strFileSaveFolder + Get_Lot_IDFromHost() + _T("\\");

    Ipvm::CreateDirectories(strFileSaveFolder);
    Ipvm::CreateDirectories(strFileSaveFolder_LotID);

    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    strFileName.Format(_T("%sVisionDeviceSendResultLog(%s)_%s.txt"), (LPCTSTR)strFileSaveFolder_LotID,
        (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo, (LPCTSTR)CTime::GetCurrentTime().Format(_T("%Y%m%d")));

    if (!File.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
        return;
    else
    {
        SYSTEMTIME cur_time;
        GetLocalTime(&cur_time);

        // 파일의 맨 마지막을 찾는다.
        File.SeekToEnd();

        CString strWriteTime("");
        CString strWriteData("");
        strWriteTime.Format(
            _T("[%02d:%02d:%02d:%03ld]\t"), cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);

        CArchive ar(&File, CArchive::store);
        ar.WriteString(strWriteTime);
        ar.WriteString(_T("\t"));

        strWriteData.AppendFormat(_T("%d,%d,%d,%d,%02d:%02d:%02d:%03ld,%.02f"), i_nTrayID, i_nPocketID, i_nPaneID,
            i_bSendSuccess, i_inspectionStartTime.wHour, i_inspectionStartTime.wMinute, i_inspectionStartTime.wSecond,
            i_inspectionStartTime.wMilliseconds, GetDiffTime_Second(cur_time, i_inspectionStartTime));

        ar.WriteString(strWriteData);

        ar.WriteString(_T("\r\n"));
        ar.Close();
        File.Close();
    }
}

void SystemConfig::Set_SaveDeviceSendResultTimeLog(const bool i_bSavEnable)
{
    m_bSaveDeviceResultSendResultTimeLog = i_bSavEnable;
}

const bool SystemConfig::Get_SaveDeviceSendResultTimeLog()
{
    return m_bSaveDeviceResultSendResultTimeLog;
}

float SystemConfig::GetDiffTime_Second(const SYSTEMTIME i_SystemTime_Cur, const SYSTEMTIME i_SystemTime_Target)
{
    long Hour2SecondFactor = 60 * 60;
    long Minute2SecondFactor = 60;
    float Millisecond2SecondFactor = 0.001f;

    float fCurSecond = (i_SystemTime_Cur.wHour * Hour2SecondFactor) + (i_SystemTime_Cur.wMinute * Minute2SecondFactor)
        + i_SystemTime_Cur.wSecond + (i_SystemTime_Cur.wMilliseconds * Millisecond2SecondFactor);
    float fTargetSecond = (i_SystemTime_Target.wHour * Hour2SecondFactor)
        + (i_SystemTime_Target.wMinute * Minute2SecondFactor) + i_SystemTime_Target.wSecond
        + (i_SystemTime_Target.wMilliseconds * Millisecond2SecondFactor);

    float fDiffResult = fCurSecond - fTargetSecond;

    return roundf(fDiffResult * 1000.f) / 1000.f;
}

const long SystemConfig::Get_GrabFailforimageZeroLimitCount()
{
    CString strTemp = m_strGrabFailforimageZeroLimitCount[m_nGrabFailforimageZeroLimitCount_idx];

    return _ttol(strTemp);
}

const long SystemConfig::Get_nGrabFailforCableErrorLimitCount()
{
    CString strTemp = m_strGrabFailforCableErrorLimitCount[m_nGrabFailforCableErrorLimitCount_idx];

    return _ttol(strTemp);
}

static CCriticalSection g_Save_VisionLog_GrabSequenceTimeLog_CS;

void SystemConfig::Save_VisionLog_GrabSequenceTimeLog(
    const long i_nTrayID, const long i_nScanID, const CString i_strWorkLog)
{
    g_Save_VisionLog_GrabSequenceTimeLog_CS.Lock();

    CFile File;
    CString strFilePath("");

    Ipvm::CreateDirectories(DynamicSystemPath::get(DefineFolder::TimeLog));

    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    strFilePath.Format(_T("%sVisionLog_GrabSequence[%s].txt"), (LPCTSTR)DynamicSystemPath::get(DefineFolder::TimeLog),
        (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo);

    if (!File.Open(strFilePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
        return;
    else
    {
        SYSTEMTIME cur_time;
        GetLocalTime(&cur_time);

        // 파일의 맨 마지막을 찾는다.
        File.SeekToEnd();

        // Wide Char로 테스트 파일에 써지는 것을 강제로 Ansi로 써지게 한다.
        CString strTab(_T("\t"));
        CStringA strAnsi_WriteTab(strTab);

        CString strWriteTime("");
        CString strWriteData("");
        strWriteTime.Format(
            _T("%02d:%02d:%02d:%03ld\t"), cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);

        CStringA strAnsi_WriteTime(strWriteTime);

        CArchive ar(&File, CArchive::store);
        //ar.WriteString(strWriteTime);
        //ar.WriteString(_T("\t"));
        ar.Write(strAnsi_WriteTime, strAnsi_WriteTime.GetLength());
        ar.Write(strAnsi_WriteTab, strAnsi_WriteTab.GetLength());

        if (i_strWorkLog.IsEmpty() == false)
            strWriteData.AppendFormat(_T("%s,"), (LPCTSTR)i_strWorkLog);

        if (i_nTrayID != -1)
            strWriteData.AppendFormat(_T("%d,"), i_nTrayID);

        if (i_nScanID != -1)
            strWriteData.AppendFormat(_T("%d"), i_nScanID);

        CStringA strAnsi_WriteData(strWriteData);

        //ar.WriteString(strWriteData);
        ar.Write(strAnsi_WriteData, strAnsi_WriteData.GetLength());

        CString strLineChange(_T("\r\n"));
        CStringA strAnsi_WriteLineChange(strLineChange);

        //ar.WriteString(_T("\r\n"));
        ar.Write(strAnsi_WriteLineChange, strAnsi_WriteLineChange.GetLength());
        ar.Close();
        File.Close();
    }

    g_Save_VisionLog_GrabSequenceTimeLog_CS.Unlock();
}

static CCriticalSection g_Save_VisionLog_InspectionTimeLog_CS;

void SystemConfig::Save_VisionLog_InspectionTimeLog(const long i_nThreadID, const long i_nTrayID, const long i_nScanID,
    const long i_nPaneID, const CString i_strWorkLog)
{
    if (i_nThreadID < 0)
        return; //Thread ID가 0보다 작을수는 없다

    g_Save_VisionLog_InspectionTimeLog_CS.Lock();

    CFile File;
    CString strFilePath("");

    Ipvm::CreateDirectories(DynamicSystemPath::get(DefineFolder::TimeLog));

    //ThreadID별로 생성해야 한다
    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    strFilePath.Format(_T("%sVisionLog_InspectionTime[%s]_Thread%d.txt"),
        (LPCTSTR)DynamicSystemPath::get(DefineFolder::TimeLog), (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo,
        i_nThreadID);

    if (!File.Open(strFilePath, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
        return;
    else
    {
        SYSTEMTIME cur_time;
        GetLocalTime(&cur_time);

        // 파일의 맨 마지막을 찾는다.
        File.SeekToEnd();

        // Wide Char로 테스트 파일에 써지는 것을 강제로 Ansi로 써지게 한다.
        CString strTab(_T("\t"));
        CStringA strAnsi_WriteTab(strTab);

        CString strWriteTime("");
        CString strWriteData("");
        strWriteTime.Format(
            _T("%02d:%02d:%02d:%03ld\t"), cur_time.wHour, cur_time.wMinute, cur_time.wSecond, cur_time.wMilliseconds);

        CStringA strAnsi_WriteTime(strWriteTime);

        CArchive ar(&File, CArchive::store);
        //ar.WriteString(strWriteTime);
        //ar.WriteString(_T("\t"));
        ar.Write(strAnsi_WriteTime, strAnsi_WriteTime.GetLength());
        ar.Write(strTab, strTab.GetLength());

        if (i_strWorkLog.IsEmpty() == false)
            strWriteData.AppendFormat(_T("%s,"), (LPCTSTR)i_strWorkLog);

        if (i_nTrayID != -1)
            strWriteData.AppendFormat(_T("%d,"), i_nTrayID);

        if (i_nScanID != -1)
            strWriteData.AppendFormat(_T("%d,"), i_nScanID);

        if (i_nPaneID != -1)
            strWriteData.AppendFormat(_T("%d"), i_nPaneID);

        CStringA strAnsi_WriteData(strWriteData);

        ar.Write(strAnsi_WriteData, strAnsi_WriteData.GetLength());

        //ar.WriteString(strWriteData);

        CString strLineChange(_T("\r\n"));
        CStringA strAnsi_WriteLineChange(strLineChange);

        //ar.WriteString(_T("\r\n"));
        ar.Write(strAnsi_WriteLineChange, strAnsi_WriteLineChange.GetLength());
        ar.Close();
        File.Close();
    }

    g_Save_VisionLog_InspectionTimeLog_CS.Unlock();
}

bool SystemConfig::GetFileNamesinDirecotry(const CString i_strSearchDirectoryPath, const CString i_strFileNameExtension,
    std::vector<CString>& o_vecFileNames, std::vector<CString>& o_vecFilePath)
{
    if (i_strSearchDirectoryPath.IsEmpty() == true
        || i_strFileNameExtension.IsEmpty() == true) //검색할 Path와 확장자명이 없는데 뭘검색해..
        return false;

    o_vecFileNames.clear();

    CFileFind cFileFind;

    CString strSearchPath = i_strSearchDirectoryPath + i_strFileNameExtension;
    BOOL bSearchDirecotryFindSuccess = cFileFind.FindFile(strSearchPath);

    if (bSearchDirecotryFindSuccess != TRUE)
        return false;

    while (bSearchDirecotryFindSuccess)
    {
        bSearchDirecotryFindSuccess = cFileFind.FindNextFile();
        if (cFileFind.IsDirectory() && !cFileFind.IsDots())
            continue;

        o_vecFileNames.push_back(cFileFind.GetFileName());
        o_vecFilePath.push_back(cFileFind.GetFilePath());
    }

    cFileFind.Close();

    return true;
}

void SystemConfig::Set2DIDCroppingimageSaveOption(const long i_nOptionidx)
{
    m_n2DIDCroppingimageSaveOptionidx = i_nOptionidx;
}

e2DIDCroppingimageSaveOption SystemConfig::Get2DIDCroppingimageSaveOption()
{
    return e2DIDCroppingimageSaveOption(m_n2DIDCroppingimageSaveOptionidx);
}

void SystemConfig::Set_is_iGrabFirmware_and_LibraryVersion_Mismatch(const bool i_bisMismatch)
{
    m_bis_iGrabFirmware_and_LibraryVersion_Mismatch = i_bisMismatch;
}

bool SystemConfig::Get_is_iGrabFirmware_and_LibraryVersion_Mismatch()
{
    return m_bis_iGrabFirmware_and_LibraryVersion_Mismatch;
}

void SystemConfig::SetExistRingillumination(const bool i_bEnable)
{
    m_bExistRingillumiation = i_bEnable;
}

bool SystemConfig::GetExistRingillumination()
{
    return m_bExistRingillumiation;
}

void SystemConfig::SetUse3DCameraNoiseTest(const bool i_bEnable)
{
    m_bUse3DCameraNoiseTest = i_bEnable;
}

bool SystemConfig::GetUse3DCameraNoiseTest()
{
    return m_bUse3DCameraNoiseTest;
}

void SystemConfig::Set_SpaceTimeSamplingScaleFactor(const float i_fScaleFactor)
{
    m_fSpaceTimeSamplingScaleFactor = i_fScaleFactor;
} //mc_SpaceTimeSamplingScaleFactor

float SystemConfig::Get_SpaceTimeSamplingScaleFactor()
{
    return m_fSpaceTimeSamplingScaleFactor;
} //mc_SpaceTimeSamplingScaleFactor

void SystemConfig::ClearIllumCoeffNormal()
{
    m_vecfCoeffA_Ref.clear();
    m_vecfCoeffB_Ref.clear();

    m_vecfCoeffA_Cur.clear();
    m_vecfCoeffB_Cur.clear();

    m_vecfCoeffA_Cur_SideRear.clear();
    m_vecfCoeffB_Cur_SideRear.clear();
}

void SystemConfig::ClearIllumCoeffMirror()
{
    m_vecfCoeffA_RefMirror.clear();
    m_vecfCoeffB_RefMirror.clear();

    m_vecfCoeffA_CurMirror.clear();
    m_vecfCoeffB_CurMirror.clear();

    m_vecfCoeffA_CurMirror_SideRear.clear();
    m_vecfCoeffB_CurMirror_SideRear.clear();
}

long SystemConfig::GetMaxIllumChannelCount()
{
    unsigned long UseChannelCount = GetExistRingillumination() == true
        ? LED_ILLUM_CHANNEL_RING_MAX + LED_ILLUM_CHANNEL_DEFAULT
        : LED_ILLUM_CHANNEL_DEFAULT;

    switch (GetVisionType())
    {
        case VISIONTYPE_SIDE_INSP:
            UseChannelCount = LED_ILLUM_CHANNEL_SIDE_DEFAULT;
            break;
        case VISIONTYPE_SWIR_INSP:
            UseChannelCount = LED_ILLUM_CHANNEL_SWIR_DEFAULT;
            break;
        default:
            break;
    }

    return UseChannelCount;
}

long SystemConfig::GetMaxIllumChannelCount(const enTRVisionmodule TR_visionmodule)
{
    long UseChannelCount(0);

    switch (TR_visionmodule)
    {
        case enTRVisionmodule::TR_VISIONMODULE_OTI:
            UseChannelCount = LED_ILLUM_CHANNEL_OTI_DEFAULT;
            break;
        case enTRVisionmodule::TR_VISIONMODULE_INPOCKET:
            UseChannelCount = LED_ILLUM_CHANNEL_INPOCKET_DEFAULT;
            break;
        default:
            break;
    }

    return UseChannelCount;
}

long SystemConfig::GetUseIllumChannelCount()
{
    unsigned long nChannelMaxCount
        = GetMaxIllumChannelCount() > LED_ILLUM_CHANNEL_MAX ? LED_ILLUM_CHANNEL_MAX : GetMaxIllumChannelCount();
    ;

    if (GetVisionType() == VISIONTYPE_SIDE_INSP)
    {
        nChannelMaxCount = LED_ILLUM_CHANNEL_SIDE_DEFAULT;
    }

    nChannelMaxCount = max(nChannelMaxCount, LED_ILLUM_CHANNEL_MAX);

    return nChannelMaxCount;
}

void SystemConfig::SetFrameGrabberType(const int i_fgType) //kircheis_SWIR
{
    if (i_fgType < enFrameGrabberType::FG_TYPE_START || i_fgType >= enFrameGrabberType::FG_TYPE_END)
    {
        m_fgType = enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES; // Default Frame Grabber Type는 IGRAB G2 시리즈로 설정
        return;
    }

    m_fgType = i_fgType; // Frame Grabber Type는 음수값이 될수 없다
}

int SystemConfig::GetFrameGrabberType() const //kircheis_SWIR
{
    return m_fgType;
}

bool SystemConfig::IsFrameGrabberTypeiGrabG2() const //iGrabG2 시리즈인지 확인//kircheis_SWIR
{
    static const bool isFrameGrabberTypeiGrabG2
        = (GetFrameGrabberType() == enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES);
    return isFrameGrabberTypeiGrabG2;
}
bool SystemConfig::IsFrameGrabberTypeiGrabXE() const //iGrabXE인지 확인//kircheis_SWIR
{
    static const bool isFrameGrabberTypeiGrabXE = (GetFrameGrabberType() == enFrameGrabberType::FG_TYPE_IGRAB_XE);
    return isFrameGrabberTypeiGrabXE;
}
bool SystemConfig::IsFrameGrabberTypeiGrabXQ() const //kircheis_SWIR
{
    //iGrabXQ인지 확인
    static const bool isFrameGrabberTypeiGrabXQ = (GetFrameGrabberType() == enFrameGrabberType::FG_TYPE_IGRAB_XQ);
    return isFrameGrabberTypeiGrabXQ;
}

void SystemConfig::SetCameraModel(const CString& i_cameraModel) //kircheis_SWIR
{
    if (i_cameraModel.IsEmpty())
    {
        return;
    }
    m_cameraModel = i_cameraModel;
}

CString SystemConfig::GetCameraModel() const //kircheis_SWIR
{
    return m_cameraModel;
}

bool SystemConfig::IsNGRVInspectionMode()
    const //연산속도 최소화를 위해 모든 조건을 static으로 선언하여 판단은 한번만 한다.//kircheis_SWIR
{
    static const bool is2DVision = IsVisionType2D();
    static const bool isFG_iGrabXQ = IsFrameGrabberTypeiGrabXQ();
    static const bool isNGRV_Camera = GetCameraModel() == _T("VC-65MX-M/C 31");

    static const bool isNGRV_InspectionMode = (is2DVision && isFG_iGrabXQ && isNGRV_Camera);

    return isNGRV_InspectionMode;
}

void SystemConfig::SetNGRVOpticsType(const int& i_nCameraType)
{
    m_ngrvCameraType = i_nCameraType;
}

int SystemConfig::GetNGRVOpticsType() const
{
    return m_ngrvCameraType;
}

bool SystemConfig::IsNgrvColorOptics() const
{
    static const bool isNGRV = IsVisionTypeNGRV();
    static const bool isColorOptics = (GetNGRVOpticsType() == NGRV_VISION_OPTICS_TYPE_COLOR) ? true : false;
    static const bool ret = (isNGRV && isColorOptics);

    return ret;
}

bool SystemConfig::IsNgrvSwirOptics() const
{
    static const bool isNGRV = IsVisionTypeNGRV();
    static const bool isSwirOptics = (GetNGRVOpticsType() == NGRV_VISION_OPTICS_TYPE_SWIR) ? true : false;
    static const bool ret = (isNGRV && isSwirOptics);

    return ret;
}