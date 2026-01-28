//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FOVImageGrabberSide.h"

//CPP_2_________________________________ This project's headers
#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber_Base.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
FOVImageGrabberSide::FOVImageGrabberSide(CVisionMain* visionMain)
    : FOVImageGrabber(visionMain)
    , m_totalFrameCount(0)
    , m_highPosFrameCount(0)
{
    long imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    for (long nidx = 0; nidx < enSideVisionModule::SIDE_VISIONMODULE_END; nidx++)
    {
        for (long index = 0; index < m_bufferMaxCount; index++)
        {
            auto buffer = std::make_shared<FOVBuffer>();

            for (long frameIndex = 0; frameIndex < LED_ILLUM_FRAME_MAX; frameIndex++)
            {
                buffer->m_imagePtrs[frameIndex] = new BYTE[imageSizeX * imageSizeY];
            }

            m_buffers[enSideVisionModule(nidx)].push_back(buffer);
        }
    }
}

FOVImageGrabberSide::~FOVImageGrabberSide()
{
    for (auto& buffer : m_buffers)
    {
        for (long nidx = 0; nidx < enSideVisionModule::SIDE_VISIONMODULE_END; nidx++)
        {
            for (long frameIndex = 0; frameIndex < LED_ILLUM_FRAME_MAX; frameIndex++)
            {
                delete[] buffer[enSideVisionModule(nidx)]->m_imagePtrs[frameIndex];
            }
        }
    }
}

void FOVImageGrabberSide::allocateBuffer(long totalFrameCount, long highPosFrameCount)
{
    m_totalFrameCount = totalFrameCount;
    m_highPosFrameCount = highPosFrameCount;
}

bool FOVImageGrabberSide::buffer_grab(
    long bufferIndex, const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
    const int& idx_grabber = 0;
    const int& idx_camera = (int)i_eSideVisionModule;

    const bool bisAcqOff = i_eSideVisionModule == enSideVisionModule::SIDE_VISIONMODULE_FRONT
        ? false
        : true; //Front일경우 Acq. Off하지 않는다

    BOOL bIsCameraGrabRetry = SystemConfig::GetInstance().m_bUseGrabRetry; //kircheis_3DCalcRetry

    // 이젠 Ready 가 아니다.
    SyncController::GetInstance().SetReadySignal(FALSE);
    SyncController::GetInstance().SetGrabExposure(TRUE);

    sGrab2DErrorLogMessageFormat
        sGrab2DErrorLogMessageFormat; //mc_Grab3D Error LogFormat은 이거에 맞춘다 자세한 사항은 구조체 참조
    sGrab2DErrorLogMessageFormat.InitData();

    sGrab2DErrorLogMessageFormat.m_strLotID = m_visionMain->m_lotID;
    sGrab2DErrorLogMessageFormat.m_nTrayIndex = fovID.m_trayIndex;
    sGrab2DErrorLogMessageFormat.m_nFOVID = fovID.m_fovIndex;

    //{{ //kircheis_MultiGrab
    static const bool bIs380Bridge = SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE;
    short totalrameCount = (short)m_totalFrameCount;
    short highPosGrabCount = bIs380Bridge ? 0 : (short)m_highPosFrameCount;
    short normalGrabCount = totalrameCount - highPosGrabCount;

    auto& bufferList = m_buffers[i_eSideVisionModule][bufferIndex]->m_imagePtrs;
    bool success = true;

    if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[0], normalGrabCount, nullptr)
        == TRUE)
    {
        SyncController::GetInstance().StartSyncBoard(FALSE, i_eSideVisionModule);

        SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Side Vision Grab Start"));

        if (FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr) == FALSE)
        {
            sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Wait Grab End Fail");
            Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
            //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

            if (bIsCameraGrabRetry == TRUE)
            {
                if (FrameGrabber::GetInstance().StartGrab2D(
                        idx_grabber, idx_camera, &bufferList[0], normalGrabCount, nullptr)
                    == TRUE)
                {
                    SyncController::GetInstance().StartSyncBoard(FALSE, i_eSideVisionModule);
                    if (FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr) == FALSE)
                    {
                        sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("ReGrab-Wait Grab End Fail");
                        Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
                        //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));

                        for (long i = 0; i < normalGrabCount; i++)
                        {
                            // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
                            memset(bufferList[i], 128,
                                FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                                    * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                        }

                        success = false;
                    }
                    else
                    {
                        FrameGrabber::GetInstance().set_grabber_temperature(
                            0); //mc_GrabEnd가 된 시점에 온도를 기록하여 준다
                        sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("ReGrab Success");
                        Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
                        //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                    }
                }
                else
                {
                    sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("Re-StartGrab2D Fail");
                    Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
                    //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Re-StartGrab2D Fail"));

                    for (long i = 0; i < normalGrabCount; i++)
                    {
                        // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
                        memset(bufferList[i], 128,
                            FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                                * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                    }

                    success = false;
                }
            }
            else
            {
                for (long i = 0; i < normalGrabCount; i++)
                {
                    // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
                    memset(bufferList[i], 128,
                        FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                            * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                }

                success = false;
            }
        }

        SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Side Vision Grab End"));

        FrameGrabber::GetInstance().set_grabber_temperature(0); //mc_GrabEnd가 된 시점에 온도를 기록하여 준다
    }
    else
    {
        sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("StartGrab2D Fail");
        Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
        //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrab2D Fail"));

        for (long i = 0; i < normalGrabCount; i++)
        {
            // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
            memset(bufferList[i], 128,
                FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                    * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
        }

        success = false;
    }

    // 영상 획득이 빨라서 시퀀스에서 놓치는 경우가 있으므로, Start signal 이 떨어지기를 기다린 후 Acquisition 을 떨어뜨린다.
    {
        Ipvm::TimeCheck timeCheck;
        while (timeCheck.Elapsed_ms() < 5000)
        {
            if (SyncController::GetInstance().GetStartSignal() == FALSE)
                break;
        }
    }

    if (bisAcqOff == true)
        SyncController::GetInstance().SetGrabExposure(FALSE);
    SyncController::GetInstance().TurnOffLight(i_eSideVisionModule);

    if (bIs380Bridge || highPosGrabCount <= 0)
    {
        return success;
    }

    //여기에 High Bit를 기다리는 Code 필요
    Ipvm::TimeCheck timeCheckWaitHighBit;
    BOOL bHighPosBitOn = FALSE;
    while (timeCheckWaitHighBit.Elapsed_ms() < 5000)
    {
        bHighPosBitOn = SyncController::GetInstance().GetGrabHighPosGrabBit();
        if (bHighPosBitOn)
            break;
    }

    if (bHighPosBitOn)
    {
        SyncController::GetInstance().SetGrabExposure(TRUE); //후에 수정

        SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Side Vision High Pos Grab Start"));

        if (FrameGrabber::GetInstance().StartGrab2D(
                idx_grabber, idx_camera, &bufferList[normalGrabCount], highPosGrabCount, nullptr)
            == TRUE)
        {
            SyncController::GetInstance().StartSyncBoardHighPosGrab(FALSE, i_eSideVisionModule);
            if (FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr) == FALSE)
            {
                sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("High Pos_Wait Grab End Fail");
                Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
                //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

                if (bIsCameraGrabRetry == TRUE)
                {
                    if (FrameGrabber::GetInstance().StartGrab2D(
                            idx_grabber, idx_camera, &bufferList[normalGrabCount], highPosGrabCount, nullptr)
                        == TRUE)
                    {
                        SyncController::GetInstance().StartSyncBoardHighPosGrab(FALSE, i_eSideVisionModule);
                        if (FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr) == FALSE)
                        {
                            sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog
                                = _T("High Pos_ReGrab-Wait Grab End Fail");
                            Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
                            //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));

                            for (long i = normalGrabCount; i < normalGrabCount + highPosGrabCount; i++)
                            {
                                // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
                                memset(bufferList[i], 128,
                                    FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                                        * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                            }

                            success = false;
                        }
                        else
                        {
                            FrameGrabber::GetInstance().set_grabber_temperature(
                                0); //mc_GrabEnd가 된 시점에 온도를 기록하여 준다
                            sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("High Pos_ReGrab Success");
                            Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
                            //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                        }
                    }
                    else
                    {
                        sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("High Pos_ReGrab-StartGrab2D Fail");
                        Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
                        //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-StartGrab2D Fail"));

                        for (long i = normalGrabCount; i < normalGrabCount + highPosGrabCount; i++)
                        {
                            // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
                            memset(bufferList[i], 128,
                                FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                                    * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                        }

                        success = false;
                    }
                }
                else
                {
                    for (long i = normalGrabCount; i < normalGrabCount + highPosGrabCount; i++)
                    {
                        // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
                        memset(bufferList[i], 128,
                            FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                                * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                    }

                    success = false;
                }
            }

            SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
                fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Vision High Pos Grab End"));

            FrameGrabber::GetInstance().set_grabber_temperature(0); //mc_GrabEnd가 된 시점에 온도를 기록하여 준다
        }
        else
        {
            sGrab2DErrorLogMessageFormat.m_strGrabberGrabFailLog = _T("High Pos_StartGrab2D Fail");
            Send_Grab2D_Error_LogMessage(_T("Inline Grab"), sGrab2DErrorLogMessageFormat);
            //m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrab2D Fail"));

            for (long i = normalGrabCount; i < normalGrabCount + highPosGrabCount; i++)
            {
                // Grab Fail일 경우 이미지를 128로 채운다, 0으로 채울 경우 Grab Fail과 조명을 설정하지 않았을 때의 이미지를 구분할 수 없음 - 2023.07.12_JHB
                memset(bufferList[i], 128,
                    FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera)
                        * FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
            }

            success = false;
        }

        Ipvm::TimeCheck timeCheck;
        while (timeCheck.Elapsed_ms() < 5000) //제대로 꺼지는 지 확인
        {
            if (!SyncController::GetInstance().GetGrabHighPosGrabBit())
                break;
        }

        if (bisAcqOff == true)
            SyncController::GetInstance().SetGrabExposure(FALSE);
        SyncController::GetInstance().TurnOffLight(i_eSideVisionModule);
    }

    return success;
}

void FOVImageGrabberSide::buffer_paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
    std::shared_ptr<VisionImageLot> inspectionImage)
{
    for (long nidx = 0; nidx < enSideVisionModule::SIDE_VISIONMODULE_END; nidx++)
    {
        auto& bufferList = m_buffers[enSideVisionModule(nidx)][bufferIndex]->m_imagePtrs;

        // 스티칭 영상을 붙여넣자.
        inspectionImage->Paste2D(
            stitchIndex, &bufferList[0], m_totalFrameCount, stitchCountX, stitchCountY, enSideVisionModule(nidx));
    }
}

void FOVImageGrabberSide::ClearImageGrabber_GrabTime()
{
    return;
}

void FOVImageGrabberSide::SaveImageGrabber_GrabTime(const CString i_strSavePath)
{
    return;
}

void FOVImageGrabberSide::Send_Grab2D_Error_LogMessage(
    const CString i_strCategory, const sGrab2DErrorLogMessageFormat i_sGrab2DErrorForamt)
{
    CString strSendErrorLogMessage("");

    auto log = i_sGrab2DErrorForamt;

    strSendErrorLogMessage.AppendFormat(_T("%s,%d,%d,%s"), (LPCTSTR)log.m_strLotID, log.m_nTrayIndex, log.m_nFOVID,
        (LPCTSTR)log.m_strGrabberGrabFailLog);

    m_visionMain->iPIS_Send_ErrorMessageForLog(i_strCategory, strSendErrorLogMessage);
}
