//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FOVImageGrabber2D.h"

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
#include <Ipvm/AlgorithmGpu/ImageProcessingGpu.h>
#include <Ipvm/Base/Enum.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
FOVImageGrabber2D::FOVImageGrabber2D(CVisionMain* visionMain)
    : FOVImageGrabber(visionMain)
    , m_totalFrameCount(0)
    , m_highPosFrameCount(0)
{
    const unsigned int& imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    const unsigned int& imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    for (long index = 0; index < m_bufferMaxCount; index++)
    {
        auto buffer = std::make_shared<FOVBuffer>();

        for (long frameIndex = 0; frameIndex < LED_ILLUM_FRAME_MAX; frameIndex++)
        {
            buffer->m_imagePtrs[frameIndex] = new BYTE[imageSizeX * imageSizeY];
        }

        m_buffers.push_back(buffer);
    }
}

FOVImageGrabber2D::~FOVImageGrabber2D()
{
    for (auto& buffer : m_buffers)
    {
        for (long frameIndex = 0; frameIndex < LED_ILLUM_FRAME_MAX; frameIndex++)
        {
            delete[] buffer->m_imagePtrs[frameIndex];
        }
    }
}

void FOVImageGrabber2D::allocateBuffer(long totalFrameCount, long highPosFrameCount)
{
    m_totalFrameCount = totalFrameCount;
    m_highPosFrameCount = highPosFrameCount;
}

bool FOVImageGrabber2D::buffer_grab(long bufferIndex, const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
    const int& idx_grabber = 0;
    const int& idx_camera = 0;

    bool success(false);
    if (SystemConfig::GetInstance().IsNGRVInspectionMode() == true)
        success = buffer_grab_NGRV_insepction(idx_grabber, idx_camera, bufferIndex, fovID, i_eSideVisionModule);
    else
        success = buffer_grab_2D(idx_grabber, idx_camera, bufferIndex, fovID, i_eSideVisionModule);

    return success;
}

void FOVImageGrabber2D::buffer_paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
    std::shared_ptr<VisionImageLot> inspectionImage)
{
    auto& bufferList = m_buffers[bufferIndex]->m_imagePtrs;

    // 스티칭 영상을 붙여넣자.
    inspectionImage->Paste2D(stitchIndex, &bufferList[0], m_totalFrameCount, stitchCountX, stitchCountY,
        enSideVisionModule::SIDE_VISIONMODULE_FRONT);
}

bool FOVImageGrabber2D::buffer_grab_2D(const int& idx_grabber, const int& idx_camera, long bufferIndex,
    const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
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

    auto& bufferList = m_buffers[bufferIndex]->m_imagePtrs;
    bool success = true;

    if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[0], normalGrabCount, nullptr)
        == TRUE)
    {
        SyncController::GetInstance().StartSyncBoard(FALSE, i_eSideVisionModule);

        SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Vision Grab Start"));

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
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Vision Grab End"));

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
        SyncController::GetInstance().SetGrabExposure(TRUE);

        SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Vision High Pos Grab Start"));

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
        SyncController::GetInstance().SetGrabExposure(FALSE);
        SyncController::GetInstance().TurnOffLight(i_eSideVisionModule);
    }

    return success;
}

bool FOVImageGrabber2D::buffer_grab_NGRV_insepction(const int& idx_grabber, const int& idx_camera, long bufferIndex,
    const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
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
    int totalrameCount = (int)m_totalFrameCount;
    //short highPosGrabCount = bIs380Bridge ? 0 : (short)m_highPosFrameCount;
    //mc_reverse와 UV, IR 다 고려를 해야한다
    int normalGrabCount = totalrameCount; // - highPosGrabCount;

    auto& bufferList = m_buffers[bufferIndex]->m_imagePtrs;
    bool success = true;

    if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[0], normalGrabCount, nullptr)
        == TRUE)
    {
        SyncController::GetInstance().StartSyncBoard(FALSE, i_eSideVisionModule);

        SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Vision Grab Start"));

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
            fovID.m_trayIndex, fovID.m_fovIndex, _T("2D Vision Grab End"));

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

    SyncController::GetInstance().SetGrabExposure(FALSE);
    SyncController::GetInstance().TurnOffLight(i_eSideVisionModule);

    //Reverse, IR, UV는 사용하지 않는다
    success = Conversion_image_green(&bufferList[0], bufferIndex, normalGrabCount);

    return success;
}

bool FOVImageGrabber2D::Conversion_image_green(BYTE** Grabbedimage, long bufferIndex, const int& totalFrameCount)
{
    const unsigned int& imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    const unsigned int& imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    static float Gain_red = SystemConfig::GetInstance().m_fNormal_Channel_GainR;
    static float Gain_green = SystemConfig::GetInstance().m_fNormal_Channel_GainG;
    static float Gain_blue = SystemConfig::GetInstance().m_fNormal_Channel_GainB;

    Ipvm::Image8u image_red(imageSizeX, imageSizeY);
    Ipvm::Image8u image_green(imageSizeX, imageSizeY);
    Ipvm::Image8u image_blue(imageSizeX, imageSizeY);

    for (long Frame = 0; Frame < totalFrameCount; Frame++)
    {
        if (Grabbedimage[Frame] == NULL)
            return false;

        Ipvm::Image8u image(imageSizeX, imageSizeY, Grabbedimage[Frame], imageSizeX);

        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(image, Ipvm::Rect32s(image), Ipvm::BayerPattern::e_gbrg,
                Gain_red, Gain_green, Gain_blue, image_red, image_green, image_blue)
            != Ipvm::Status::e_ok)
            return false;

        memcpy(m_buffers[bufferIndex]->m_imagePtrs[Frame], image_green.GetMem(), imageSizeX * imageSizeY);
    }

    return true;
}

void FOVImageGrabber2D::ClearImageGrabber_GrabTime()
{
    return;
}

void FOVImageGrabber2D::SaveImageGrabber_GrabTime(const CString i_strSavePath)
{
    return;
}
void FOVImageGrabber2D::Send_Grab2D_Error_LogMessage(
    const CString i_strCategory, const sGrab2DErrorLogMessageFormat i_sGrab2DErrorForamt)
{
    CString strSendErrorLogMessage("");

    auto log = i_sGrab2DErrorForamt;

    strSendErrorLogMessage.AppendFormat(_T("%s,%d,%d,%s"), (LPCTSTR)log.m_strLotID, log.m_nTrayIndex, log.m_nFOVID,
        (LPCTSTR)log.m_strGrabberGrabFailLog);

    m_visionMain->iPIS_Send_ErrorMessageForLog(i_strCategory, strSendErrorLogMessage);
}
