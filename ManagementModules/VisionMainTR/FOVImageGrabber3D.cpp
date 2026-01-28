//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FOVImageGrabber3D.h"

//CPP_2_________________________________ This project's headers
#include "VisionMainTR.h"

//CPP_3_________________________________ Other projects' headers
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber_Base.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedBuildConfiguration/iGrabXE/iManXDef.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/TimeCheck.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
FOVImageGrabber3D::FOVImageGrabber3D(VisionMainTR* visionMain)
    : FOVImageGrabber(visionMain)
    , m_bufferSize(0)
{
    for (long index = 0; index < m_bufferMaxCount; index++)
    {
        auto buffer = std::make_shared<FOVBuffer>();
        buffer->m_bufferFloat = new float[m_bufferSize];
        buffer->m_bufferShort = new USHORT[m_bufferSize];
        buffer->m_imageZmap = Ipvm::Image32r(0, 0, buffer->m_bufferFloat, 0);
        buffer->m_imageVmap = Ipvm::Image16u(0, 0, buffer->m_bufferShort, 0);

        m_buffers.push_back(buffer);
    }
}

FOVImageGrabber3D::~FOVImageGrabber3D()
{
}

void FOVImageGrabber3D::allocateBuffer(long logicalScanLength_px)
{
    m_imageVerifyData.InitData();

    long imageSizeX = logicalScanLength_px;
    long imageSizeY = FrameGrabber::GetInstance().get_grab_image_width();

    if (m_bufferSize < imageSizeX * imageSizeY)
    {
        // 버퍼 Size를 다시 할당해야 한다
        m_bufferSize = imageSizeX * imageSizeY;

        for (auto& buffer : m_buffers)
        {
            delete[] buffer->m_bufferFloat;
            delete[] buffer->m_bufferShort;

            buffer->m_bufferFloat = new float[m_bufferSize];
            buffer->m_bufferShort = new USHORT[m_bufferSize];
        }
    }

    // Buffer Image 갱신
    for (auto& buffer : m_buffers)
    {
        buffer->m_imageZmap = Ipvm::Image32r(imageSizeX, imageSizeY, buffer->m_bufferFloat, imageSizeX * sizeof(float));
        buffer->m_imageVmap
            = Ipvm::Image16u(imageSizeX, imageSizeY, buffer->m_bufferShort, imageSizeX * sizeof(USHORT));
    }
}

//kk GPU Error로 인한 Image에 문제가 있는지 확인하는 함수
bool FOVImageGrabber3D::grab3D_Image_Verification(
    const Ipvm::Image32r& zmap, const Ipvm::Image16u& vmap, Ipvm::AsyncProgress* progress)
{
    if (progress)
    {
        progress->Post(_T("Start Image Integrity Verification"));
    }

    bool isSameData = FALSE; //kk flag가 변경되는 순간 모든 데이터를 다 확인하기 전까진 True Return 안함
    long nSameDataCount = 0; //kk 10개 이상의 같은 포인트가 있으면 실패로 판정
    BOOL isNotUse = SystemConfig::GetInstance().m_nGrabVerifyMatchingCount == 0;
    CString strTemp = SystemConfig::GetInstance()
                          .m_strGrabVerifyMatchingCount[SystemConfig::GetInstance().m_nGrabVerifyMatchingCount];
    const long nMaxErrorCount = isNotUse ? 10 : _ttoi(strTemp);

    //Image Base Information
    const long nImageSizeX = zmap.GetSizeX();

    //현재 Grab한 Image의 정보를 담을 vector들
    std::vector<float> zValueData;
    std::vector<unsigned short> intensityData;

    //현재 Grab한 Image의 정보들을 담는다.
    const long nStartPositionY = 1000;
    const long nEndPositionY = 3000;
    const long nGapY = 100;

    const long nStartPositionX = long((float)nImageSizeX * 0.1f);
    const long nEndPositionX = long((float)nImageSizeX * 0.5f);
    const long nGapX = 100;

    for (int y = nStartPositionY; y < nEndPositionY; y += nGapY)
    {
        for (int x = nStartPositionX; x < nEndPositionX; x += nGapX)
        {
            float zData = 0.f;
            unsigned short intenData = 0;

            zData = *(zmap.GetMem(x, y));
            intenData = *(vmap.GetMem(x, y));

            zValueData.push_back(zData);
            intensityData.push_back(intenData);
        }
    }

    //kk 만약 비교할 변수에 데이터가 없다면 현재 데이터를 넣고 종료한다.
    if (m_imageVerifyData.vecZValueData.size() == 0 || m_imageVerifyData.vecIntensityData.size() == 0)
    {
        m_imageVerifyData.CopyData(zValueData, intensityData);
        return true;
    }

    //kk 검증하기 위해 Data Report 뽑는 용도
    //Save_VerifyReport(zValueData, intensityData);

    //kk 검증 부분
    long nDataSize = (long)zValueData.size();

    for (int i = 0; i < nDataSize; i++)
    {
        if (nSameDataCount >= nMaxErrorCount)
        {
            return false;
        }

        if (intensityData[i] == 0 && m_imageVerifyData.vecIntensityData[i] == 0)
            continue;
        else if (!isSameData) //kk flag 변경되기 전
        {
            if (intensityData[i] != m_imageVerifyData.vecIntensityData[i])
            {
                m_imageVerifyData.CopyData(zValueData, intensityData);
                return true; //kk Intensity Data가 다르다면 다른 이미지로 판정한다.
            }
            else
            {
                if (zValueData[i] != m_imageVerifyData.vecZValueData[i])
                {
                    m_imageVerifyData.CopyData(zValueData, intensityData);
                    return true; //kk Intensity Data가 같더라도 같은 위치의 Z Value가 다르다면 다른 이미지로 판정한다.
                }
                else //kk flag 변경.. 같은 데이터의 수를 파악하기 시작한다.
                {
                    isSameData = TRUE;
                    nSameDataCount += 1;
                }
            }
        }
        else if (isSameData) //kk flag 변경된 후
        {
            if (intensityData[i] != m_imageVerifyData.vecIntensityData[i])
                continue; //kk Intensity Data가 다르다면 다른 이미지로 판정한다. 하지만 같았던 적이 있으므로 더 찾아본다.
            else
            {
                if (zValueData[i] != m_imageVerifyData.vecZValueData[i])
                    continue; //kk Intensity Data가 같더라도 같은 위치의 Z Value가 다르다면 다른 이미지로 판정한다. 하지만 같았던 적이 있으므로 더 찾아본다.
                else
                {
                    nSameDataCount += 1;
                }
            }
        }
    }

    m_imageVerifyData.CopyData(zValueData, intensityData); //kk 같은 부분이 있었지만, 10개 미만일 경우
    return true;
}

bool FOVImageGrabber3D::save_VerifyReport(
    std::vector<float> i_vecZValueData, std::vector<unsigned short> i_vecIntensityData)
{
    if (i_vecZValueData.size() == 0)
    {
        // 저장할게 없다
        return true;
    }

    CString saveFolder = _T("D:\\VerifyReport_3DGrab\\");
    Ipvm::CreateDirectories(saveFolder);

    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));
    CString strTotalFilePath;
    strTotalFilePath.Format(_T("%s%sVerifyData.csv"), (LPCTSTR)saveFolder, (LPCTSTR)strTime);

    FILE* fp = nullptr;
    _tfopen_s(&fp, strTotalFilePath, _T("w"));

    if (fp == nullptr)
    {
        return false;
    }

    fprintf(fp, "index,ZValue_Before,ZValue_After,Inten_Before,Inten_After\n");

    for (int i = 0; i < i_vecZValueData.size(); i++)
    {
        CStringA str;
        str.Format("%d,%f,%f,%d,%d\n", i, m_imageVerifyData.vecZValueData[i], i_vecZValueData[i],
            m_imageVerifyData.vecIntensityData[i], i_vecIntensityData[i]);
        fprintf(fp, str);
    }

    CStringA str_Info;
    str_Info.Format("size,%d,%d,%d,%d\n", m_imageVerifyData.vecZValueData.size(), i_vecZValueData.size(),
        m_imageVerifyData.vecIntensityData.size(), i_vecIntensityData.size());
    fprintf(fp, str_Info);

    fclose(fp);

    return true;
}

static bool sbisGrabRetryMode = false;
static long snGrabFailforimageZeroCount = 0;
static long snGrabFailforCableErrorCount = 0;
static long snGrabFailforFIFOOverFolowCount = 0;

bool FOVImageGrabber3D::buffer_grab(long bufferIndex, const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
    const int& idx_grabber = 0;
    const int& idx_camera = 0;
    const bool& save_slibeam_images = (bool)SystemConfig::GetInstance().m_saveSlitbeamOriginalImage;

    std::shared_ptr<Ipvm::AsyncProgress> progress;
    if (!IsDebuggerPresent())
    {
        // 디버그인 경우 자꾸여기서 멈추는 경우가 있어서 표시하지 말자
        progress = std::make_shared<Ipvm::AsyncProgress>(_T("Inline 3D grab started"));
    }

    bool bCameraGrabSuccess = true; //kircheis_3DCalcRetry
    bool bGpuCalcSuccess = true; //kircheis_3DCalcRetry

    auto buffer = m_buffers[bufferIndex];

    SyncController::GetInstance().TurnOnLight(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    SyncController::GetInstance().SetReadySignal(FALSE);
    SyncController::GetInstance().SetGrabDirection(
        CAST_BYTE(SystemConfig::GetInstance().m_nGrabDirection)); //kircheis_GrabDir

    //kk Grab Retry 사용할 것 인가?
    BOOL bUseGpuCalcRetry = SystemConfig::GetInstance().m_nGrabVerifyMatchingCount == 0;
    BOOL bIsCameraGrabRetry = SystemConfig::GetInstance().m_bUseGrabRetry; //kircheis_3DCalcRetry

    sGrab3DErrorLogMessageFormat
        sGrab3DErrorLogMessageFormat; //mc_Grab3D Error LogFormat은 이거에 맞춘다 자세한 사항은 구조체 참조
    sGrab3DErrorLogMessageFormat.InitData();

    sGrab3DErrorLogMessageFormat.m_strLotID = m_visionMain->m_lotID;
    sGrab3DErrorLogMessageFormat.m_nTrayIndex = fovID.m_trayIndex;
    sGrab3DErrorLogMessageFormat.m_nFOVID = fovID.m_fovIndex;

    CString strCallStartGrab(""), strExeStartGrab(""), strOnIO(""), strOffVisionStart(""), strGrabEnd("");

    strOffVisionStart.Empty();
    strCallStartGrab.Format(_T("CSG[%02d:%02d:%02d]"), CTime::GetCurrentTime().GetHour(),
        CTime::GetCurrentTime().GetMinute(), CTime::GetCurrentTime().GetSecond());
    sGrab3DErrorLogMessageFormat.m_strCSG_Time = strCallStartGrab;

    m_sInline3D_GrabTime.InitData();

    if (FrameGrabber::GetInstance().StartGrab3D(idx_grabber, idx_camera, 0, buffer->m_imageZmap, buffer->m_imageVmap,
            buffer->m_heightRangeMin, buffer->m_heightRangeMax, progress.get()))
    {
        strExeStartGrab.Format(_T("ESG[%02d:%02d:%02d]"), CTime::GetCurrentTime().GetHour(),
            CTime::GetCurrentTime().GetMinute(), CTime::GetCurrentTime().GetSecond());
        sGrab3DErrorLogMessageFormat.m_strESG_Time = strExeStartGrab;

        SyncController::GetInstance().SetGrabExposure(TRUE);
        SyncController::GetInstance().SetGrabAcquisition(TRUE);

        strOnIO.Format(_T("OAS[%02d:%02d:%02d]"), CTime::GetCurrentTime().GetHour(),
            CTime::GetCurrentTime().GetMinute(), CTime::GetCurrentTime().GetSecond());
        sGrab3DErrorLogMessageFormat.m_strOAS_Time = strOnIO;

        bool bisVisionStartOff = false;
        for (long nT = 0; nT < 2000; nT++)
        {
            Sleep(1);

            if (nT % 5 == 0)
            {
                SyncController::GetInstance().SetGrabExposure(TRUE);
                SyncController::GetInstance().SetGrabAcquisition(TRUE);
            }

            if (SyncController::GetInstance().GetStartSignal() || SyncController::GetInstance().GetStartSignal())
                continue;
            else
            {
                strOffVisionStart.Format(_T("OffVStart[%02d:%02d:%02d]"), CTime::GetCurrentTime().GetHour(),
                    CTime::GetCurrentTime().GetMinute(), CTime::GetCurrentTime().GetSecond());
                sGrab3DErrorLogMessageFormat.m_strOffVStart_Time = strOffVisionStart;
                SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
                    fovID.m_trayIndex, fovID.m_fovIndex, _T("3D Vision Grab Start"));
                bisVisionStartOff = true;
                break;
            }
        }

        if (bisVisionStartOff == false)
        {
            //Handler에 프로토콜 전송
            m_visionMain->iPIS_Send_3D_Vision_StartIO_NotOff();
        }

        if (strOffVisionStart.IsEmpty())
        {
            strOffVisionStart.Format(_T("OffVStart_NotOff2sec"));
            sGrab3DErrorLogMessageFormat.m_strOffVStart_Time = strOffVisionStart;
        }

        Ipvm::TimeCheck WaitGrabEnd_Time;
        if (FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, progress.get()))
        {
            FrameGrabber::GetInstance().set_grabber_temperature(0); //mc_GrabEnd가 된 시점에 온도를 기록하여 준다

            SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
                fovID.m_trayIndex, fovID.m_fovIndex, _T("3D Vision Grab End"));

            m_sInline3D_GrabTime.m_fWaitGrabEnd_Time = CAST_FLOAT(WaitGrabEnd_Time.Elapsed_ms());
            m_sInline3D_GrabTime.m_bWatiGrabEndSuccess = true;

            SyncController::GetInstance().SetGrabExposure(FALSE);
            SyncController::GetInstance().SetGrabAcquisition(FALSE);
            SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

            bCameraGrabSuccess = true; //kirchies_3DCalcRetry

            snGrabFailforimageZeroCount = 0; //WaitGrabEnd가 True가 되었다라는것은 Grab은 성고안 것이니 초기화하여 준다
            snGrabFailforCableErrorCount = 0; //WaitGrabEnd가 True가 되었다라는것은 Grab은 성고안 것이니 초기화하여 준다
            snGrabFailforFIFOOverFolowCount
                = 0; //WaitGrabEnd가 True가 되었다라는것은 Grab은 성고안 것이니 초기화하여 준다

            Ipvm::TimeCheck WaitCalcEnd_Time;

            if (!FrameGrabber::GetInstance().wait_calc_end(
                    idx_grabber, idx_camera, save_slibeam_images, progress.get()))
            {
                iGrab3D_Error_LOG siGrab3D_Error_Log = FrameGrabber::GetInstance().GetiGrab3D_Error_LOG();

                sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Calc 3D Fail");
                sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailReturnCode
                    = FrameGrabber::GetInstance().Get3DGrabErrorMessage();
                sGrab3DErrorLogMessageFormat.m_nGrabberRecivedExternalTriggerCount
                    = siGrab3D_Error_Log.m_nExternalTrigger;
                sGrab3DErrorLogMessageFormat.m_nGrabberRecive2CameraFrameNum
                    = siGrab3D_Error_Log.m_nReciveFrametoCamera;
                sGrab3DErrorLogMessageFormat.m_nGrabberCustomGrabEventCount
                    = siGrab3D_Error_Log.m_nCustomGrabEventCount;
                sGrab3DErrorLogMessageFormat.m_nGrabSuccessCount = FrameGrabber::GetInstance().get_scan_count();
                sGrab3DErrorLogMessageFormat.m_nGrabLineCount = FrameGrabber::GetInstance().get_scan_length_px();
                sGrab3DErrorLogMessageFormat.m_strGrabberCalbeConnectStatus = siGrab3D_Error_Log.m_strCxpLinkCableError;
                sGrab3DErrorLogMessageFormat.m_strGrabberPacketStatus = siGrab3D_Error_Log.m_strPacketError;

                //2023.02.23_DEV. Debug Log
                sGrab3DErrorLogMessageFormat.m_hard_intr_count_s = siGrab3D_Error_Log.m_hard_intr_count_s;
                sGrab3DErrorLogMessageFormat.m_hard_intr_count_e = siGrab3D_Error_Log.m_hard_intr_count_e;
                sGrab3DErrorLogMessageFormat.m_soft_intr_count_s = siGrab3D_Error_Log.m_soft_intr_count_s;
                sGrab3DErrorLogMessageFormat.m_soft_intr_count_e = siGrab3D_Error_Log.m_soft_intr_count_e;
                sGrab3DErrorLogMessageFormat.m_cb_call_count = siGrab3D_Error_Log.m_cb_call_count;
                sGrab3DErrorLogMessageFormat.m_cb_sum_count = siGrab3D_Error_Log.m_cb_sum_count;
                //

                //2023.03.08_DEV. Debug Log
                sGrab3DErrorLogMessageFormat.m_dwDbgRunState = siGrab3D_Error_Log.m_dwDbgRunState;
                //

                //2023.05.30_DEV. Debug Log
                sGrab3DErrorLogMessageFormat.m_dwDbgMultGrabProcessReturnVal
                    = siGrab3D_Error_Log.m_dwDbgMultGrabProcessReturnVal;
                //

                //2023.03.14_H/W Info
                sGrab3DErrorLogMessageFormat.m_fCPU_Temperature = siGrab3D_Error_Log.m_fCPU_Temperature;
                sGrab3DErrorLogMessageFormat.m_fPC_UsagePhysicalMemory = siGrab3D_Error_Log.m_fPC_UsagePhysicalMemory;
                sGrab3DErrorLogMessageFormat.m_fiPack_UsagePhysicalMemory
                    = siGrab3D_Error_Log.m_fiPack_UsagePhysicalMemory;
                //

                Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);

                //2023.04.26_Grab Fail Count
                IncreaseGrabFailCount(sbisGrabRetryMode, siGrab3D_Error_Log);
                //
            }

            m_sInline3D_GrabTime.m_fWaitCalcEnd_Time = CAST_FLOAT(WaitCalcEnd_Time.Elapsed_ms());
            m_sInline3D_GrabTime.m_bWatiCalcEndSuccess = true;
        }
        else
        {
            FrameGrabber::GetInstance().set_grabber_temperature(0); //mc_GrabEnd가 된 시점에 온도를 기록하여 준다

            bCameraGrabSuccess = false; //kirchies_3DCalcRetry

            // SDY 3D Grab fail의 경우 이전 이미지가 들어가는 문제 발생하기도 한다. > grab retry를 하지 않을 경우 zero fill을 진행해서 검은색으로 표시
            buffer->m_imageZmap.FillZero();
            buffer->m_imageVmap.FillZero();

            int nEncoderCntNew(0);
            long nEncoderCntExist = SyncController::GetInstance().ReadCount(nEncoderCntNew);

            strGrabEnd.Format(_T("GrabEnd[%02d:%02d:%02d]"), CTime::GetCurrentTime().GetHour(),
                CTime::GetCurrentTime().GetMinute(), CTime::GetCurrentTime().GetSecond());
            sGrab3DErrorLogMessageFormat.m_strGrabEnd_Time = strGrabEnd;

            iGrab3D_Error_LOG siGrab3D_Error_Log = FrameGrabber::GetInstance().GetiGrab3D_Error_LOG();

            sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Wait Grab End Fail");
            sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailReturnCode
                = FrameGrabber::GetInstance().Get3DGrabErrorMessage();
            sGrab3DErrorLogMessageFormat.m_nGrabberRecivedExternalTriggerCount = siGrab3D_Error_Log.m_nExternalTrigger;
            sGrab3DErrorLogMessageFormat.m_nGrabberRecive2CameraFrameNum = siGrab3D_Error_Log.m_nReciveFrametoCamera;
            sGrab3DErrorLogMessageFormat.m_nGrabberCustomGrabEventCount = siGrab3D_Error_Log.m_nCustomGrabEventCount;
            sGrab3DErrorLogMessageFormat.m_nGrabSuccessCount = FrameGrabber::GetInstance().get_scan_count();
            sGrab3DErrorLogMessageFormat.m_nGrabLineCount = FrameGrabber::GetInstance().get_scan_length_px();
            sGrab3DErrorLogMessageFormat.m_nGrabTriggerCount = nEncoderCntNew;
            sGrab3DErrorLogMessageFormat.m_nSyncTriggerCount = nEncoderCntExist;
            sGrab3DErrorLogMessageFormat.m_strGrabberCalbeConnectStatus = siGrab3D_Error_Log.m_strCxpLinkCableError;
            sGrab3DErrorLogMessageFormat.m_strGrabberPacketStatus = siGrab3D_Error_Log.m_strPacketError;

            //2023.02.23_DEV. Debug Log
            sGrab3DErrorLogMessageFormat.m_hard_intr_count_s = siGrab3D_Error_Log.m_hard_intr_count_s;
            sGrab3DErrorLogMessageFormat.m_hard_intr_count_e = siGrab3D_Error_Log.m_hard_intr_count_e;
            sGrab3DErrorLogMessageFormat.m_soft_intr_count_s = siGrab3D_Error_Log.m_soft_intr_count_s;
            sGrab3DErrorLogMessageFormat.m_soft_intr_count_e = siGrab3D_Error_Log.m_soft_intr_count_e;
            sGrab3DErrorLogMessageFormat.m_cb_call_count = siGrab3D_Error_Log.m_cb_call_count;
            sGrab3DErrorLogMessageFormat.m_cb_sum_count = siGrab3D_Error_Log.m_cb_sum_count;
            //

            //2023.03.08_DEV. Debug Log
            sGrab3DErrorLogMessageFormat.m_dwDbgRunState = siGrab3D_Error_Log.m_dwDbgRunState;
            //

            //2023.05.30_DEV. Debug Log
            sGrab3DErrorLogMessageFormat.m_dwDbgMultGrabProcessReturnVal
                = siGrab3D_Error_Log.m_dwDbgMultGrabProcessReturnVal;
            //

            //2023.03.14_H/W Info
            sGrab3DErrorLogMessageFormat.m_fCPU_Temperature = siGrab3D_Error_Log.m_fCPU_Temperature;
            sGrab3DErrorLogMessageFormat.m_fPC_UsagePhysicalMemory = siGrab3D_Error_Log.m_fPC_UsagePhysicalMemory;
            sGrab3DErrorLogMessageFormat.m_fiPack_UsagePhysicalMemory = siGrab3D_Error_Log.m_fiPack_UsagePhysicalMemory;
            //

            Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);

            //2023.04.26_Grab Fail Count
            IncreaseGrabFailCount(sbisGrabRetryMode, siGrab3D_Error_Log);
            //

            DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("Inline Grab3D - Wait Grab Fail (T%d:FOV%d)"),
                fovID.m_trayIndex, fovID.m_fovIndex);
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("Inline Grab3D - Wait Grab Fail (T%d:FOV%d)"),
                fovID.m_trayIndex, fovID.m_fovIndex);
        }
    }
    else
    {
        iGrab3D_Error_LOG siGrab3D_Error_Log = FrameGrabber::GetInstance().GetiGrab3D_Error_LOG();

        sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("StartGrab3D Fail");
        sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailReturnCode
            = FrameGrabber::GetInstance().Get3DGrabErrorMessage();
        sGrab3DErrorLogMessageFormat.m_nGrabberRecivedExternalTriggerCount = siGrab3D_Error_Log.m_nExternalTrigger;
        sGrab3DErrorLogMessageFormat.m_nGrabberRecive2CameraFrameNum = siGrab3D_Error_Log.m_nReciveFrametoCamera;
        sGrab3DErrorLogMessageFormat.m_nGrabberCustomGrabEventCount = siGrab3D_Error_Log.m_nCustomGrabEventCount;
        sGrab3DErrorLogMessageFormat.m_nGrabSuccessCount = FrameGrabber::GetInstance().get_scan_count();
        sGrab3DErrorLogMessageFormat.m_nGrabLineCount = FrameGrabber::GetInstance().get_scan_length_px();
        sGrab3DErrorLogMessageFormat.m_strGrabberCalbeConnectStatus = siGrab3D_Error_Log.m_strCxpLinkCableError;
        sGrab3DErrorLogMessageFormat.m_strGrabberPacketStatus = siGrab3D_Error_Log.m_strPacketError;

        //2023.02.23_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_hard_intr_count_s = siGrab3D_Error_Log.m_hard_intr_count_s;
        sGrab3DErrorLogMessageFormat.m_hard_intr_count_e = siGrab3D_Error_Log.m_hard_intr_count_e;
        sGrab3DErrorLogMessageFormat.m_soft_intr_count_s = siGrab3D_Error_Log.m_soft_intr_count_s;
        sGrab3DErrorLogMessageFormat.m_soft_intr_count_e = siGrab3D_Error_Log.m_soft_intr_count_e;
        sGrab3DErrorLogMessageFormat.m_cb_call_count = siGrab3D_Error_Log.m_cb_call_count;
        sGrab3DErrorLogMessageFormat.m_cb_sum_count = siGrab3D_Error_Log.m_cb_sum_count;
        //

        //2023.03.08_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_dwDbgRunState = siGrab3D_Error_Log.m_dwDbgRunState;
        //

        //2023.05.30_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_dwDbgMultGrabProcessReturnVal
            = siGrab3D_Error_Log.m_dwDbgMultGrabProcessReturnVal;
        //

        //2023.03.14_H/W Info
        sGrab3DErrorLogMessageFormat.m_fCPU_Temperature = siGrab3D_Error_Log.m_fCPU_Temperature;
        sGrab3DErrorLogMessageFormat.m_fPC_UsagePhysicalMemory = siGrab3D_Error_Log.m_fPC_UsagePhysicalMemory;
        sGrab3DErrorLogMessageFormat.m_fiPack_UsagePhysicalMemory = siGrab3D_Error_Log.m_fiPack_UsagePhysicalMemory;
        //

        Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);

        //2023.04.26_Grab Fail Count
        IncreaseGrabFailCount(sbisGrabRetryMode, siGrab3D_Error_Log);
        //

        DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab, _T("Inline Grab3D - StartGrab3D Fail (T%d:FOV%d)"),
            fovID.m_trayIndex, fovID.m_fovIndex);
        DevelopmentLog::AddLog(DevelopmentLog::Type::Warning, _T("Inline Grab3D - StartGrab3D Fail (T%d:FOV%d)"),
            fovID.m_trayIndex, fovID.m_fovIndex);

        bCameraGrabSuccess = true; //kirchies_3DCalcRetry
    }

    //검증 (Image가 정상적으로 들어왔는지), success일 경우에만 사용
    if (bCameraGrabSuccess && bUseGpuCalcRetry) //kircheis_3DCalcRetry
    {
        bGpuCalcSuccess = grab3D_Image_Verification(buffer->m_imageZmap, buffer->m_imageVmap, progress.get());

        if (!bGpuCalcSuccess)
        {
            sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("VerifyGrab3D Fail");

            Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);

            DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
                _T("Inline Grab3D - VerifyGrab3D Fail (%s:T%d:FOV%d)"), (LPCTSTR)m_visionMain->m_lotID,
                fovID.m_trayIndex, fovID.m_fovIndex);
            DevelopmentLog::AddLog(DevelopmentLog::Type::Warning,
                _T("Inline Grab3D - VerifyGrab3D Fail (%s:T%d:FOV%d)"), (LPCTSTR)m_visionMain->m_lotID,
                fovID.m_trayIndex, fovID.m_fovIndex);

            if (!FrameGrabber::GetInstance().re_calc_cuda_3D())
            {
                bGpuCalcSuccess = grab3D_Image_Verification(buffer->m_imageZmap, buffer->m_imageVmap, progress.get());
                if (bGpuCalcSuccess == false)
                {
                    sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Re-Calc Cuda->SameFailed");
                    Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);
                }
                else
                {
                    sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Re-Calc Cuda->Ok");
                    Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);
                    bGpuCalcSuccess = true;
                }
            }
            else
            {
                sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Re-Calc Cuda->CalcFailed");
                sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailReturnCode
                    = FrameGrabber::GetInstance().Get3DGrabErrorMessage();
                Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);
            }
        }
        else
        {
            SyncController::GetInstance().SetGrabExposure(FALSE);
            SyncController::GetInstance().SetGrabAcquisition(FALSE);
        }
    }

    //mc_Grab Retry시 성공하였을때에도 Log를 남겨준다
    if (sbisGrabRetryMode == true && bCameraGrabSuccess == true)
    {
        FrameGrabber::GetInstance().SetiGrab3D_Error_LOG();

        iGrab3D_Error_LOG siGrab3D_Error_Log = FrameGrabber::GetInstance().GetiGrab3D_Error_LOG();

        sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("GrabRetry->Grab Success");
        sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailReturnCode
            = FrameGrabber::GetInstance().Get3DGrabErrorMessage();
        sGrab3DErrorLogMessageFormat.m_nGrabberRecivedExternalTriggerCount = siGrab3D_Error_Log.m_nExternalTrigger;
        sGrab3DErrorLogMessageFormat.m_nGrabberRecive2CameraFrameNum = siGrab3D_Error_Log.m_nReciveFrametoCamera;
        sGrab3DErrorLogMessageFormat.m_nGrabberCustomGrabEventCount = siGrab3D_Error_Log.m_nCustomGrabEventCount;
        sGrab3DErrorLogMessageFormat.m_nGrabSuccessCount = FrameGrabber::GetInstance().get_scan_count();
        sGrab3DErrorLogMessageFormat.m_nGrabLineCount = FrameGrabber::GetInstance().get_scan_length_px();
        sGrab3DErrorLogMessageFormat.m_strGrabberCalbeConnectStatus = siGrab3D_Error_Log.m_strCxpLinkCableError;
        sGrab3DErrorLogMessageFormat.m_strGrabberPacketStatus = siGrab3D_Error_Log.m_strPacketError;

        //2023.02.23_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_hard_intr_count_s = siGrab3D_Error_Log.m_hard_intr_count_s;
        sGrab3DErrorLogMessageFormat.m_hard_intr_count_e = siGrab3D_Error_Log.m_hard_intr_count_e;
        sGrab3DErrorLogMessageFormat.m_soft_intr_count_s = siGrab3D_Error_Log.m_soft_intr_count_s;
        sGrab3DErrorLogMessageFormat.m_soft_intr_count_e = siGrab3D_Error_Log.m_soft_intr_count_e;
        sGrab3DErrorLogMessageFormat.m_cb_call_count = siGrab3D_Error_Log.m_cb_call_count;
        sGrab3DErrorLogMessageFormat.m_cb_sum_count = siGrab3D_Error_Log.m_cb_sum_count;
        //

        //2023.03.08_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_dwDbgRunState = siGrab3D_Error_Log.m_dwDbgRunState;
        //

        //2023.05.30_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_dwDbgMultGrabProcessReturnVal
            = siGrab3D_Error_Log.m_dwDbgMultGrabProcessReturnVal;
        //

        //2023.03.14_H/W Info
        sGrab3DErrorLogMessageFormat.m_fCPU_Temperature = siGrab3D_Error_Log.m_fCPU_Temperature;
        sGrab3DErrorLogMessageFormat.m_fPC_UsagePhysicalMemory = siGrab3D_Error_Log.m_fPC_UsagePhysicalMemory;
        sGrab3DErrorLogMessageFormat.m_fiPack_UsagePhysicalMemory = siGrab3D_Error_Log.m_fiPack_UsagePhysicalMemory;
        //

        Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);

        sbisGrabRetryMode = false; //Grab Retry를 성공하였으면 초기화를 해준다

        snGrabFailforimageZeroCount = 0; //mc_GrabRetry가 성공이면 Grab 성공한 것이니 초기화하여 준다
        snGrabFailforCableErrorCount = 0; //mc_GrabRetry가 성공이면 Grab 성공한 것이니 초기화하여 준다
        snGrabFailforFIFOOverFolowCount = 0; //mc_GrabRetry가 성공이면 Grab 성공한 것이니 초기화하여 준다
    }

    //kk Grab Retry를 사용할 경우
    if (bIsCameraGrabRetry) //kircheis_3DCalcRetry
    {
        //GrabFail 전송 및 대기
        if (bCameraGrabSuccess == false && m_visionMain->m_bGrabRetry == FALSE)
        {
            sbisGrabRetryMode = true;

            m_visionMain->iPIS_Send_GrabRetry();

            long nBreakCount = 0;

            while (1)
            {
                if (nBreakCount >= 1000) //kk 50초 대기 후 안나오면 break
                {
                    m_visionMain->m_bGrabRetry
                        = TRUE; //kk 50초동안 호스트에서 Ack이 안날라온 것은 줄 수 없는 환경이란 의미이므로 버리고 다음으로 넘어간다.

                    sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Not received [Grab retry ack]");
                    Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);

                    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
                        _T("Inline Grab3D - Not received [Grab retry ack] (%s:T%d:FOV%d)"),
                        LPCTSTR(m_visionMain->m_lotID), fovID.m_trayIndex, fovID.m_fovIndex);
                    DevelopmentLog::AddLog(DevelopmentLog::Type::Warning,
                        _T("Inline Grab3D - Not received [Grab retry ack] (%s:T%d:FOV%d)"),
                        LPCTSTR(m_visionMain->m_lotID), fovID.m_trayIndex, fovID.m_fovIndex);
                    break;
                }

                if (m_visionMain->m_bGrabRetry == TRUE)
                {
                    DevelopmentLog::AddLog(DevelopmentLog::Type::ImageGrab,
                        _T("Inline Grab3D - Received [Grab retry ack] (%s:T%d:FOV%d)"), LPCTSTR(m_visionMain->m_lotID),
                        fovID.m_trayIndex, fovID.m_fovIndex);
                    DevelopmentLog::AddLog(DevelopmentLog::Type::Notice,
                        _T("Inline Grab3D - Received [Grab retry ack] (%s:T%d:FOV%d)"), LPCTSTR(m_visionMain->m_lotID),
                        fovID.m_trayIndex, fovID.m_fovIndex);
                    break;
                }

                Sleep(50);
                nBreakCount++;
            }
        }
        else if (bCameraGrabSuccess == false && m_visionMain->m_bGrabRetry == TRUE)
        {
            bCameraGrabSuccess = true;
            m_visionMain->m_bGrabRetry = FALSE;

            //여기까지 왔다라는건 GrabRetry를 했음에도 CameraGrab Fail인 상태이다
            //Log를 남길수 있도록 보내준다
            sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("GrabRetry->Grab Failed");

            Send_Grab3D_Error_LogMessage(_T("Inline Grab"), sGrab3DErrorLogMessageFormat);

            sbisGrabRetryMode = false;
        }
    }
    else
    {
        //kk 사용 안할 경우 GrabFail이더라도 true로 강제 변환한다.
        bCameraGrabSuccess = true;
    }

    SyncController::GetInstance().SetGrabExposure(FALSE);
    SyncController::GetInstance().SetGrabAcquisition(FALSE);
    SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    m_sInline3D_GrabTime.m_nTrayidx = fovID.m_trayIndex;
    m_sInline3D_GrabTime.m_nFovidx = fovID.m_fovIndex;
    m_sInline3D_GrabTime.m_bCameraGrabSuccess = bCameraGrabSuccess;

    if (SystemConfig::GetInstance().Get_Enable_GPU_CALC_SAVE_LOG() == true)
        m_vecsInline3D_GrabTime.push_back(m_sInline3D_GrabTime);

    //{{ATM Site Test
    FrameGrabber::GetInstance().SetiGrab3D_Error_LOG();
    iGrab3D_Error_LOG siGrab3D_Error_Log = FrameGrabber::GetInstance().GetiGrab3D_Error_LOG();

    //if (siGrab3D_Error_Log.m_dwDbgRunState == -1 || siGrab3D_Error_Log.m_dwDbgRunState == 0)
    if (siGrab3D_Error_Log.m_dwDbgRunState == 0)
    {
        int nEncoderCntNew(0);
        long nEncoderCntExist = SyncController::GetInstance().ReadCount(nEncoderCntNew);

        if (nEncoderCntNew >= nEncoderCntExist)
        {
            nEncoderCntNew = 0;
            nEncoderCntExist = 0;

            nEncoderCntExist = SyncController::GetInstance().ReadCount(nEncoderCntNew);
        }

        sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Warning");
        sGrab3DErrorLogMessageFormat.m_strGrabberGrabFailReturnCode = _T("Excute F/G Exception Function");
        sGrab3DErrorLogMessageFormat.m_nGrabberRecivedExternalTriggerCount = siGrab3D_Error_Log.m_nExternalTrigger;
        sGrab3DErrorLogMessageFormat.m_nGrabberRecive2CameraFrameNum = siGrab3D_Error_Log.m_nReciveFrametoCamera;
        sGrab3DErrorLogMessageFormat.m_nGrabberCustomGrabEventCount = siGrab3D_Error_Log.m_nCustomGrabEventCount;
        sGrab3DErrorLogMessageFormat.m_nGrabSuccessCount = FrameGrabber::GetInstance().get_scan_count();
        sGrab3DErrorLogMessageFormat.m_nGrabLineCount = FrameGrabber::GetInstance().get_scan_length_px();
        sGrab3DErrorLogMessageFormat.m_strGrabberCalbeConnectStatus = siGrab3D_Error_Log.m_strCxpLinkCableError;
        sGrab3DErrorLogMessageFormat.m_strGrabberPacketStatus = siGrab3D_Error_Log.m_strPacketError;
        sGrab3DErrorLogMessageFormat.m_nGrabTriggerCount = nEncoderCntNew;
        sGrab3DErrorLogMessageFormat.m_nSyncTriggerCount = nEncoderCntExist;

        //2023.02.23_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_hard_intr_count_s = siGrab3D_Error_Log.m_hard_intr_count_s;
        sGrab3DErrorLogMessageFormat.m_hard_intr_count_e = siGrab3D_Error_Log.m_hard_intr_count_e;
        sGrab3DErrorLogMessageFormat.m_soft_intr_count_s = siGrab3D_Error_Log.m_soft_intr_count_s;
        sGrab3DErrorLogMessageFormat.m_soft_intr_count_e = siGrab3D_Error_Log.m_soft_intr_count_e;
        sGrab3DErrorLogMessageFormat.m_cb_call_count = siGrab3D_Error_Log.m_cb_call_count;
        sGrab3DErrorLogMessageFormat.m_cb_sum_count = siGrab3D_Error_Log.m_cb_sum_count;
        //

        //2023.03.08_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_dwDbgRunState = siGrab3D_Error_Log.m_dwDbgRunState;
        //

        //2023.05.30_DEV. Debug Log
        sGrab3DErrorLogMessageFormat.m_dwDbgMultGrabProcessReturnVal
            = siGrab3D_Error_Log.m_dwDbgMultGrabProcessReturnVal;
        //

        //2023.03.14_H/W Info
        sGrab3DErrorLogMessageFormat.m_fCPU_Temperature = siGrab3D_Error_Log.m_fCPU_Temperature;
        sGrab3DErrorLogMessageFormat.m_fPC_UsagePhysicalMemory = siGrab3D_Error_Log.m_fPC_UsagePhysicalMemory;
        sGrab3DErrorLogMessageFormat.m_fiPack_UsagePhysicalMemory = siGrab3D_Error_Log.m_fiPack_UsagePhysicalMemory;
        //

        Send_Grab3D_Error_LogMessage(_T("F/G Exception Function"), sGrab3DErrorLogMessageFormat);

        snGrabFailforimageZeroCount = 0; //mc_Exception은 Grab이 성공한 것이니 초기화하여 준다
        snGrabFailforCableErrorCount = 0; //mc_Exception은 Grab이 성공한 것이니 초기화하여 준다
        snGrabFailforFIFOOverFolowCount = 0; //mc_Exception은 Grab이 성공한 것이니 초기화하여 준다
    }
    //}}

    //mc_2023.04.26_연속 Grab Fail인 경우 설비 Stop
    //if (snGrabFailforCableErrorCount >= SystemConfig::GetInstance().Get_nGrabFailforCableErrorLimitCount()) //Calbe Connection Error가 설정한 Fail 수보다 많거나 같으면
    if (snGrabFailforCableErrorCount > 0) //1번이라도 발생하였으면 Error이니 사용자에게 알려준다
    {
        m_visionMain->iPIS_Send_ErrorMessageForPopup(
            _T("\nAbnormal sequence detected in CXP Link Cable Error.\nPlease restart the Vision S/W, after checking ")
            _T("the CXP Cable."));
        //Handler는 이미 Stop한 상태이니 초기화 하여 준다
        snGrabFailforCableErrorCount = 0;
        snGrabFailforimageZeroCount = 0;
        snGrabFailforFIFOOverFolowCount = 0;
    }

    if (snGrabFailforimageZeroCount >= SystemConfig::GetInstance()
            .Get_GrabFailforimageZeroLimitCount()) //Grab Fail이 설정한 Fail 수보다 많거나 같으면
    {
        m_visionMain->iPIS_Send_ErrorMessageForPopup(_T("\nAbnormal sequence detected in Camera.\nPlease restart the ")
                                                     _T("Vision S/W, after checking the camera light sensor status."));
        //Handler는 이미 Stop한 상태이니 초기화 하여 준다
        snGrabFailforCableErrorCount = 0;
        snGrabFailforimageZeroCount = 0;
        snGrabFailforFIFOOverFolowCount = 0;
    }

    if (snGrabFailforFIFOOverFolowCount >= 2) //FIFO OverFlow가 2회이상이면
    {
        //iPack을 재시작 할 것이니
        m_visionMain->iPIS_Send_ErrorMessageForPopup(
            _T("\nAbnormal sequence detected in PC.\nPlease restart the Vision S/W"));

        //Handler는 이미 Stop한 상태이니 초기화 하여 준다
        snGrabFailforCableErrorCount = 0;
        snGrabFailforimageZeroCount = 0;
        snGrabFailforFIFOOverFolowCount = 0;
    }

    //

    return bCameraGrabSuccess;
}

void FOVImageGrabber3D::buffer_paste(long bufferIndex, long stitchIndex, long /*stitchCountX*/, long /*stitchCountY*/,
    std::shared_ptr<VisionImageLot> inspectionImage)
{
    auto bufferList = m_buffers[bufferIndex];

    // 스티칭 영상을 붙여넣자.
    inspectionImage->Set3DScanInfo(
        bufferList->m_heightRangeMin, bufferList->m_heightRangeMax, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    inspectionImage->Paste3D(
        stitchIndex, bufferList->m_imageZmap, bufferList->m_imageVmap, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
}

void FOVImageGrabber3D::ClearImageGrabber_GrabTime()
{
    m_vecsInline3D_GrabTime.clear();
}

void FOVImageGrabber3D::SaveImageGrabber_GrabTime(const CString i_strSavePath)
{
    if (m_vecsInline3D_GrabTime.size() <= 0)
        return;

    CString strSaveFilePath = i_strSavePath + _T("3D_GrabTimeLog.csv");
    FILE* fp = nullptr;

    _tfopen_s(&fp, strSaveFilePath, _T("w"));

    if (fp == nullptr)
        return;

    fprintf(fp,
        "Fovidx,Trayidx,WaitGrabEnd_Success,WaitGrabEnd_Time_ms,WaitCalcEnd_Success,WaitCalcEnd_Time_ms,CameraGrab_"
        "Success\n");

    for (const auto& Inline3D_GrabTime : m_vecsInline3D_GrabTime)
    {
        CString strGrabEnd_Status = Inline3D_GrabTime.m_bWatiGrabEndSuccess ? _T("TRUE") : _T("FALSE");
        CString strCalcEnd_Status = Inline3D_GrabTime.m_bWatiCalcEndSuccess ? _T("TRUE") : _T("FALSE");
        CString strGrabFail_Status = Inline3D_GrabTime.m_bCameraGrabSuccess ? _T("TRUE") : _T("FALSE");

        CStringA str;
        str.Format(("%d,%d,%s,%f,%s,%f,%s\n"), Inline3D_GrabTime.m_nFovidx, Inline3D_GrabTime.m_nTrayidx,
            (LPCTSTR)strGrabEnd_Status, Inline3D_GrabTime.m_fWaitGrabEnd_Time, (LPCTSTR)strCalcEnd_Status,
            Inline3D_GrabTime.m_fWaitCalcEnd_Time, (LPCTSTR)strGrabFail_Status);
        fprintf(fp, str);
    }

    fclose(fp);
}

void FOVImageGrabber3D::Send_Grab3D_Error_LogMessage(
    const CString i_strCategory, const sGrab3DErrorLogMessageFormat i_sGrab3DErrorForamt)
{
    CString strSendErrorLogMessage("");

    auto log = i_sGrab3DErrorForamt;

    //strSendErrorLogMessage.AppendFormat(_T("%s,%d,%d,%s,%s,%05d,%05d,%d,%05d,%05d,%05d,%05d,%s,%s,%s,%s,%s,%s,%s,%.1f"), (LPCTSTR)log.m_strLotID, log.m_nTrayIndex, log.m_nFOVID
    //																										,(LPCTSTR)log.m_strGrabberGrabFailLog, (LPCTSTR)log.m_strGrabberGrabFailReturnCode
    //																										,log.m_nGrabberRecivedExternalTriggerCount, log.m_nGrabberRecive2CameraFrameNum, log.m_nGrabberCustomGrabEventCount
    //																										,log.m_nGrabSuccessCount, log.m_nGrabLineCount
    //																										,log.m_nGrabTriggerCount, log.m_nSyncTriggerCount
    //																										,(LPCTSTR)log.m_strCSG_Time, (LPCTSTR)log.m_strESG_Time, (LPCTSTR)log.m_strOAS_Time, (LPCTSTR)log.m_strOffVStart_Time, (LPCTSTR)log.m_strGrabEnd_Time
    //																										,(LPCTSTR)log.m_strGrabberCalbeConnectStatus, (LPCTSTR)log.m_strGrabberPacketStatus
    //																										,FrameGrabber::GetInstance().GetiGrabBoardTemperature());

    ////2023.02.23_DEV. Debug Log
    //strSendErrorLogMessage.AppendFormat(_T("%s,%d,%d,%s,%s,%05d,%05d,%d,%05d,%05d,%05d,%05d,%s,%s,%s,%s,%s,%s,%s,%.1f,%05d,%05d,%05d,%05d,%05d,%05d"),(LPCTSTR)log.m_strLotID, log.m_nTrayIndex, log.m_nFOVID
    //																																				,(LPCTSTR)log.m_strGrabberGrabFailLog, (LPCTSTR)log.m_strGrabberGrabFailReturnCode
    //																																				,log.m_nGrabberRecivedExternalTriggerCount, log.m_nGrabberRecive2CameraFrameNum, log.m_nGrabberCustomGrabEventCount
    //																																				,log.m_nGrabSuccessCount, log.m_nGrabLineCount
    //																																				,log.m_nGrabTriggerCount, log.m_nSyncTriggerCount
    //																																				,(LPCTSTR)log.m_strCSG_Time, (LPCTSTR)log.m_strESG_Time, (LPCTSTR)log.m_strOAS_Time, (LPCTSTR)log.m_strOffVStart_Time, (LPCTSTR)log.m_strGrabEnd_Time
    //																																				,(LPCTSTR)log.m_strGrabberCalbeConnectStatus, (LPCTSTR)log.m_strGrabberPacketStatus
    //																																				,FrameGrabber::GetInstance().GetiGrabBoardTemperature()
    //																																				,log.m_hard_intr_count_s, log.m_hard_intr_count_e
    //																																				,log.m_soft_intr_count_s, log.m_soft_intr_count_e
    //																																				,log.m_cb_call_count, log.m_cb_sum_count);
    ////

    ////2023.03.08_DEV. Debug Log
    //strSendErrorLogMessage.AppendFormat(_T("%s,%d,%d,%s,%s,%05d,%05d,%d,%05d,%05d,%05d,%05d,%s,%s,%s,%s,%s,%s,%s,%.1f,%05d,%05d,%05d,%05d,%05d,%05d,%05d"), (LPCTSTR)log.m_strLotID, log.m_nTrayIndex, log.m_nFOVID
    //																																					, (LPCTSTR)log.m_strGrabberGrabFailLog, (LPCTSTR)log.m_strGrabberGrabFailReturnCode
    //																																					, log.m_nGrabberRecivedExternalTriggerCount, log.m_nGrabberRecive2CameraFrameNum, log.m_nGrabberCustomGrabEventCount
    //																																					, log.m_nGrabSuccessCount, log.m_nGrabLineCount
    //																																					, log.m_nGrabTriggerCount, log.m_nSyncTriggerCount
    //																																					, (LPCTSTR)log.m_strCSG_Time, (LPCTSTR)log.m_strESG_Time, (LPCTSTR)log.m_strOAS_Time, (LPCTSTR)log.m_strOffVStart_Time, (LPCTSTR)log.m_strGrabEnd_Time
    //																																					, (LPCTSTR)log.m_strGrabberCalbeConnectStatus, (LPCTSTR)log.m_strGrabberPacketStatus
    //																																					, FrameGrabber::GetInstance().GetiGrabBoardTemperature()
    //																																					, log.m_hard_intr_count_s, log.m_hard_intr_count_e
    //																																					, log.m_soft_intr_count_s, log.m_soft_intr_count_e
    //																																					, log.m_cb_call_count, log.m_cb_sum_count
    //																																					, log.m_dwDbgRunState);
    ////

    ////2023.03.14_DEV. Debug Log
    //strSendErrorLogMessage.AppendFormat(_T("%s,%d,%d,%s,%s,%05d,%05d,%d,%05d,%05d,%05d,%05d,%s,%s,%s,%s,%s,%s,%s,%.1f,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%.3f,%.3f"), (LPCTSTR)log.m_strLotID, log.m_nTrayIndex, log.m_nFOVID
    //																																									, (LPCTSTR)log.m_strGrabberGrabFailLog, (LPCTSTR)log.m_strGrabberGrabFailReturnCode
    //																																									, log.m_nGrabberRecivedExternalTriggerCount, log.m_nGrabberRecive2CameraFrameNum, log.m_nGrabberCustomGrabEventCount
    //																																									, log.m_nGrabSuccessCount, log.m_nGrabLineCount
    //																																									, log.m_nGrabTriggerCount, log.m_nSyncTriggerCount
    //																																									, (LPCTSTR)log.m_strCSG_Time, (LPCTSTR)log.m_strESG_Time, (LPCTSTR)log.m_strOAS_Time, (LPCTSTR)log.m_strOffVStart_Time, (LPCTSTR)log.m_strGrabEnd_Time
    //																																									, (LPCTSTR)log.m_strGrabberCalbeConnectStatus, (LPCTSTR)log.m_strGrabberPacketStatus
    //																																									, FrameGrabber::GetInstance().GetiGrabBoardTemperature()
    //																																									, log.m_hard_intr_count_s, log.m_hard_intr_count_e
    //																																									, log.m_soft_intr_count_s, log.m_soft_intr_count_e
    //																																									, log.m_cb_call_count, log.m_cb_sum_count
    //																																									, log.m_dwDbgRunState
    //																																									, log.m_fPC_UsagePhysicalMemory, log.m_fiPack_UsagePhysicalMemory);
    ////

    //2023.05.30_DEV. Debug Log
    strSendErrorLogMessage.AppendFormat(_T("%s,%d,%d,%s,%s,%05d,%05d,%d,%05d,%05d,%05d,%05d,%s,%s,%s,%s,%s,%s,%s,%.1f,")
                                        _T("%05d,%05d,%05d,%05d,%05d,%05d,%05d,%05d,%.3f,%.3f"),
        (LPCTSTR)log.m_strLotID, log.m_nTrayIndex, log.m_nFOVID, (LPCTSTR)log.m_strGrabberGrabFailLog,
        (LPCTSTR)log.m_strGrabberGrabFailReturnCode, log.m_nGrabberRecivedExternalTriggerCount,
        log.m_nGrabberRecive2CameraFrameNum, log.m_nGrabberCustomGrabEventCount, log.m_nGrabSuccessCount,
        log.m_nGrabLineCount, log.m_nGrabTriggerCount, log.m_nSyncTriggerCount, (LPCTSTR)log.m_strCSG_Time,
        (LPCTSTR)log.m_strESG_Time, (LPCTSTR)log.m_strOAS_Time, (LPCTSTR)log.m_strOffVStart_Time,
        (LPCTSTR)log.m_strGrabEnd_Time, (LPCTSTR)log.m_strGrabberCalbeConnectStatus,
        (LPCTSTR)log.m_strGrabberPacketStatus, FrameGrabber::GetInstance().get_grabber_temperature(0),
        log.m_hard_intr_count_s, log.m_hard_intr_count_e, log.m_soft_intr_count_s, log.m_soft_intr_count_e,
        log.m_cb_call_count, log.m_cb_sum_count, log.m_dwDbgRunState, log.m_dwDbgMultGrabProcessReturnVal,
        log.m_fPC_UsagePhysicalMemory, log.m_fiPack_UsagePhysicalMemory);
    //

    m_visionMain->iPIS_Send_ErrorMessageForLog(i_strCategory, strSendErrorLogMessage);
}

void FOVImageGrabber3D::IncreaseGrabFailCount(const bool i_bisGrabRetryMode, const iGrab3D_Error_LOG i_sErrorLog)
{
    //GrabRetry Mode일 경우는 Grab Fail을 Count하지 않는다
    if (i_bisGrabRetryMode == true)
        return;

    if (i_sErrorLog.m_byCxpLinkCableState != 0x00)
        snGrabFailforCableErrorCount++;

    if (i_sErrorLog.m_nReciveFrametoCamera == 0
        && FrameGrabber::GetInstance().get_scan_count()
            == 0) //Camera로 부터 받은 Frame이 0 이고, Grab에 성공한 Count가 0이면
        snGrabFailforimageZeroCount++;

    if (i_sErrorLog.m_dwMultGrabWaitReturnVal == STATUS_GRAB_FIFO_OVERFLOW) //FIFO OverFlow 상황을 알아야한다..
        snGrabFailforFIFOOverFolowCount++;
}