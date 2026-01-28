//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "MainFrm.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../DefineModules/dA_Base/semiinfo.h"
#include "../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../HardwareModules/dPI_SyncController/SlitIlluminator.h"
#include "../HardwareModules/dPI_SyncController/SyncCalculator.h"
#include "../HardwareModules/dPI_SyncController/SyncController.h"
#include "../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../MainUiModules/VisionPrimaryUI/DlgVisionPrimaryUI.h"
#include "../MainUiModules/VisionPrimaryUI/VisionPrimaryUI.h"
#include "../ManagementModules/VisionMain/VisionMain.h"
#include "../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../SharedCommunicationModules/VisionHostCommon/VisionTuningMsgDef.h"
#include "../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ORIGINAL_NGRV_IMAGE_SIZE_X_SPLIT_COLOR 9328
#define ORIGINAL_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR 6992
#define RESIZE_NGRV_IMAGE_SIZE_X_SPLIT_COLOR 2332
#define RESIZE_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR 1748
#define CHANNEL_REVERSE 8

//CPP_7_________________________________ Implementation body
//
LRESULT CMainFrame::OnTuningSocketConnectionUpdatedFromHost(WPARAM wParam, LPARAM lParam)
{
    const auto evtInfo = Ipvm::SocketMessaging::ParseEvt(wParam, lParam);

    if (evtInfo.m_connected)
    {
    }
    else if (evtInfo.m_disconnected)
    {
        // 라이브를 하던 중일 수 있으므로 꺼준다.
        FrameGrabber::GetInstance().live_off();
        FrameGrabber::GetInstance().set_live_image_size(
            0, 0, m_visionMain.m_pSlitBeam3DCommon->m_cameraVerticalBinning, false);
        SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

        // 튜닝 모드에서 나가는 것이므로, 레시피를 다시 적용
        m_visionMain.OnJobChanged();

        // 튜닝을 위해 받아놓은 SlitBeam Parameter를 제거한다
        delete m_tuningSlitBeamParameters;
        m_tuningSlitBeamParameters = nullptr;
    }

    return 0;
}

LRESULT CMainFrame::OnTuningSocketDataReceivedFromHost(WPARAM wParam, LPARAM lParam)
{
    const auto msgInfo = Ipvm::SocketMessaging::ParseMsg(wParam, lParam);

    const MSG_H2V nMessageID = MSG_H2V(msgInfo.m_messageIndex);
    //const long nMessageLength = msgInfo.m_messageLength;

    static const bool isNGRV_Inspection = SystemConfig::GetInstance().IsNGRVInspectionMode(); //kircheis_SWIR

    switch (nMessageID)
    {
        case MSG_H2V::GRAB_EACH_CHANNEL_2D:
        {
            MSG_H2V_GRAB_EACH_CHANNEL_2D* data = (MSG_H2V_GRAB_EACH_CHANNEL_2D*)msgInfo.m_message;

            MSG_V2H_GRAB_EACH_CHANNEL_2D_ACK ackData(data->m_messageToken);
            ackData.m_result = true;

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            static long nIllumChannelMax(-1);
            switch (SystemConfig::GetInstance().GetVisionType())
            {
                case VISIONTYPE_2D_INSP:
                    nIllumChannelMax = isNGRV_Inspection == true ? LED_ILLUM_NGRV_CHANNEL_MAX
                                                                 : LED_ILLUM_CHANNEL_MAX; //kircheis_SWIR
                    break;
                case VISIONTYPE_NGRV_INSP:
                    nIllumChannelMax = LED_ILLUM_NGRV_CHANNEL_MAX;
                    break;
                case VISIONTYPE_SIDE_INSP:
                    nIllumChannelMax = LED_ILLUM_CHANNEL_SIDE_DEFAULT;
                    break;
                case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
                    nIllumChannelMax
                        = 6; //LED_ILLUM_CHANNEL_SWIR_DEFAULT; //kircheis_SWIR//NeedRefac 우선은 2D와 동일하게 처리
                    break;
                default:
                    break;
            }

            //static const long nIllumChannelMax = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP) ? LED_ILLUM_CHANNEL_MAX : LED_ILLUM_NGRV_CHANNEL_MAX;//kircheis_SideVision고려

            bool bChannelReverse = false;
            const int& idx_grabber = 0;
            const int& idx_camera = 0;

            for (long channel = 0; channel < nIllumChannelMax; channel++)
            {
                if (!data->m_useForEachChannel[channel])
                    continue;

                if (channel == CHANNEL_REVERSE) // 배사 조명인지 확인한다 - JHB_22.01.12
                    bChannelReverse = true;

                Sleep(CAST_DWORD(data->m_grabPeriod_sec * 1000));

                std::array<float, 16> illuminations_ms;
                illuminations_ms.fill(0.f);
                illuminations_ms[channel] = data->m_illuminationTime_ms;

                SystemConfig::GetInstance().m_bIsCalIllum2D = TRUE;
                SyncController::GetInstance().SetIllumiParameter(0, illuminations_ms,
                    data->m_useIlluminationCalibration, data->m_bisVerification, data->m_bIllumVerificiationType,
                    enSideVisionModule(data->m_nCurSideVisionidx));
                SystemConfig::GetInstance().m_bIsCalIllum2D = FALSE;

                Ipvm::Image8u image;
                Ipvm::Image8u3 imageColor;
                Ipvm::Image8u3 imageResizeColor;

                MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE grab_data;
                MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE* sendData = nullptr;
                unsigned char* sendBuffer{};

                if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
                    || SystemConfig::GetInstance().GetVisionType()
                        == VISIONTYPE_SWIR_INSP) //kircheis_SideVision고려 //kircheis_SWIR
                {
                    image.Create(FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera),
                        FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                    BYTE* imageMem = image.GetMem();

                    grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera);
                    grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera);
                    grab_data.m_illuminationTime_ms = data->m_illuminationTime_ms;
                    grab_data.m_messageLength
                        = sizeof(MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                    sendBuffer = new unsigned char[grab_data.m_messageLength];

                    sendData = (MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE*)sendBuffer;

                    *sendData = grab_data;

                    SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                    FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &imageMem, 1, nullptr);
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                    FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr);

                    sendData->m_channel = channel;

                    memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                    sendData->m_bisColorimage = FALSE;
                }
                else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
                {
                    image.Create(FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera),
                        FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                    imageColor.Create(FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera),
                        FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                    BYTE* imageMem = image.GetMem();

                    grab_data.m_illuminationTime_ms = data->m_illuminationTime_ms;

                    // NGRV Grab Retry 설정 : 최대 3회 실시 - 2023.03.09_JHB
                    long nIdx = 0;
                    bool bGrabSuccess = FALSE;
                    CString strGrabFailLog;

                    SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                    do
                    {
                        FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &imageMem, 1, nullptr);
                        SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                        bGrabSuccess = FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr);
                        nIdx++;

                        strGrabFailLog.Format(_T("NGRV Grab Fail_%d"), nIdx);
                        DevelopmentLog::AddLog(DevelopmentLog::Type::Event, strGrabFailLog);
                    }
                    while (bGrabSuccess == FALSE && nIdx < 3);

                    /*SyncController::GetInstance().SetFrameCount(1, 0);
				FrameGrabber::GetInstance().StartGrabNGRV(&imageMem, 1, nullptr);
				SyncController::GetInstance().StartSyncBoard();
				FrameGrabber::GetInstance().WaitGrabEnd(nullptr);	*/

                    if (data->m_bisimageResize)
                    {
                        long nOriginalImageSizeX = ORIGINAL_NGRV_IMAGE_SIZE_X_SPLIT_COLOR;
                        long nOriginalImageSizeY = ORIGINAL_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR;
                        long nResizeImageSizeX = RESIZE_NGRV_IMAGE_SIZE_X_SPLIT_COLOR;
                        long nResizeImageSizeY = RESIZE_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR;

                        std::vector<BYTE> vecbyResizeImage(nResizeImageSizeX * nResizeImageSizeY);

                        long nDstX(0), nDstY(0);
                        long nIdxY = 0;
                        for (long nY = 0; nY < nOriginalImageSizeY; nY += 4)
                        {
                            nIdxY = nDstY * nResizeImageSizeX;
                            nDstY++;
                            nDstX = 0;
                            for (long nX = 0; nX < nOriginalImageSizeX; nX += 4)
                            {
                                vecbyResizeImage[nIdxY + nDstX] = image.GetMem(nX, nY)[0];
                                nDstX++;
                            }
                        }

                        Ipvm::Image8u imageResizeMono(
                            nResizeImageSizeX, nResizeImageSizeY, &vecbyResizeImage[0], nResizeImageSizeX);

                        grab_data.m_imageSizeX = nResizeImageSizeX;
                        grab_data.m_imageSizeY = nResizeImageSizeY;

                        grab_data.m_messageLength = sizeof(MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE)
                            + (imageResizeMono.GetWidthBytes() * imageResizeMono.GetSizeY());

                        sendBuffer = new unsigned char[grab_data.m_messageLength];

                        sendData = (MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE*)sendBuffer;

                        *sendData = grab_data;

                        sendData->m_channel = channel;

                        memcpy(sendData + 1, imageResizeMono.GetMem(),
                            imageResizeMono.GetWidthBytes() * imageResizeMono.GetSizeY());

                        sendData->m_bisColorimage = TRUE;
                        sendData->m_bisGreenimage = TRUE;
                    }
                    else
                    {
                        FrameGrabber::GetInstance().DoColorConversion(image, imageColor, false, bChannelReverse);

                        grab_data.m_imageSizeX
                            = FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera);
                        grab_data.m_imageSizeY
                            = FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera);

                        grab_data.m_messageLength = sizeof(MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE)
                            + imageColor.GetWidthBytes() * imageColor.GetSizeY();

                        sendBuffer = new unsigned char[grab_data.m_messageLength];

                        sendData = (MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE*)sendBuffer;

                        *sendData = grab_data;

                        sendData->m_channel = channel;

                        memcpy(sendData + 1, imageColor.GetMem(), imageColor.GetWidthBytes() * imageColor.GetSizeY());

                        sendData->m_bisColorimage = TRUE;
                        sendData->m_bisGreenimage = FALSE;
                    }
                }
                else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
                {
                    image.Create(FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera),
                        FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera));
                    BYTE* imageMem = image.GetMem();

                    grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera);
                    grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera);
                    grab_data.m_illuminationTime_ms = data->m_illuminationTime_ms;
                    grab_data.m_messageLength
                        = sizeof(MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                    sendBuffer = new unsigned char[grab_data.m_messageLength];

                    sendData = (MSG_V2H_GRAB_EACH_CHANNEL_2D_IMAGE*)sendBuffer;

                    *sendData = grab_data;

                    SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule(data->m_nCurSideVisionidx));
                    FrameGrabber::GetInstance().StartGrab2D(
                        idx_grabber, data->m_nCurSideVisionidx, &imageMem, 1, nullptr);
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule(data->m_nCurSideVisionidx));
                    FrameGrabber::GetInstance().wait_grab_end(idx_grabber, data->m_nCurSideVisionidx, nullptr);

                    sendData->m_channel = channel;

                    memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                    sendData->m_bisColorimage = FALSE;
                }

                if (sendData == nullptr)
                {
                    ::SimpleMessage(_T("Unknown Vision Type"));
                    delete[] sendBuffer;
                    return 0;
                }
                else
                {
                    m_visionMain.m_tuningMessageSocket->Write(
                        long(sendData->m_messageIndex), sendData->m_messageLength, sendData);

                    delete[] sendBuffer;
                }
            }
        }
        break;
        case MSG_H2V::GET_ILLUMINATION_TABLE_2D:
        {
            MSG_H2V_GET_ILLUMINATION_TABLE_2D* data = (MSG_H2V_GET_ILLUMINATION_TABLE_2D*)msgInfo.m_message;
            MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK ackData(data->m_messageToken);

            CString strRef = DynamicSystemPath::get(DefineFile::IllumTableRefTable);
            CString strRefAll = DynamicSystemPath::get(DefineFile::IllumTableRefTableAll);

            CFileFind fileFind;
            BOOL bIsExistingRef = fileFind.FindFile(strRef);
            BOOL bIsRefAll = fileFind.FindFile(strRefAll);

            CString strResultPath(_T(""));
            if (bIsRefAll)
                strResultPath = DynamicSystemPath::get(DefineFile::IllumTableRefTableAll);
            else if (bIsExistingRef && bIsRefAll == FALSE)
                strResultPath = DynamicSystemPath::get(DefineFile::IllumTableRefTable);

            CFile file;
            if (file.Open(strResultPath, CFile::modeRead | CFile::shareDenyWrite))
            {
                const auto fileSize = file.GetLength();

                char* buffer = new char[fileSize];

                file.Read(buffer, CAST_UINT(fileSize));

                file.Close();

                ackData.m_messageLength = CAST_LONG(sizeof(MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK) + fileSize);
                ackData.m_size = fileSize;
                ackData.m_result = true;

                unsigned char* sendBuffer = new unsigned char[ackData.m_messageLength];

                MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK* sendData = (MSG_V2H_GET_ILLUMINATION_TABLE_2D_ACK*)sendBuffer;

                *sendData = ackData;
                memcpy(sendData + 1, buffer, fileSize);

                CString str;
                str.Format(_T("%d == %d"), ackData.m_messageLength, (long)sizeof(sendData));

                m_visionMain.m_tuningMessageSocket->Write(
                    long(sendData->m_messageIndex), sendData->m_messageLength, sendData);

                delete[] sendBuffer;
            }
            else
            {
                ackData.m_result = false;
                m_visionMain.m_tuningMessageSocket->Write(
                    long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
            }
        }
        break;
        case MSG_H2V::SET_ILLUMINATION_TABLE_2D:
        {
            MSG_H2V_SET_ILLUMINATION_TABLE_2D* data = (MSG_H2V_SET_ILLUMINATION_TABLE_2D*)msgInfo.m_message;
            MSG_V2H_SET_ILLUMINATION_TABLE_2D_ACK ackData(data->m_messageToken);

            const char* fileData = (char*)(data + 1);

            CString strRef = DynamicSystemPath::get(DefineFile::IllumTableRefTable);
            CString strRefAll = DynamicSystemPath::get(DefineFile::IllumTableRefTableAll);

            CFileFind fileFind;
            BOOL bIsExistingRef = fileFind.FindFile(strRef);
            BOOL bIsRefAll = fileFind.FindFile(strRefAll);

            CString strFileName(_T(""));
            if (bIsRefAll)
                strFileName = _T("IlluminationTableAll.csv");
            else if (bIsExistingRef && !bIsRefAll)
                strFileName = _T("IlluminationTable.csv");
            else if (!bIsExistingRef && !bIsRefAll)
                strFileName = _T("IlluminationTableNGRV.csv");

            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
            {
                switch (data->m_nCurSideVisionidx)
                {
                    case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                        strFileName = _T("IlluminationTableAll_F.csv");
                        break;
                    case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                        strFileName = _T("IlluminationTableAll_R.csv");
                        break;
                    default:
                        break;
                }
            }

            CString strResultPath(DynamicSystemPath::get(DefineFolder::System) + strFileName);

            CFile file;
            if (file.Open(strResultPath, CFile::modeCreate | CFile::modeWrite))
            {
                file.Write(fileData, CAST_LONG(data->m_messageLength - sizeof(MSG_H2V_SET_ILLUMINATION_TABLE_2D)));

                file.Close();

                ackData.m_result = true;

                if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
                    ackData.m_result = CSyncCalculator::GetInstance().CalibrateIlluminationSide();
                else
                    ackData.m_result = CSyncCalculator::GetInstance().CalibrateIllumination2D();
            }
            else
            {
                ackData.m_result = false;
            }

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::SET_ILLUMINATION_MIRROR_TABLE_2D:
        {
            MSG_H2V_SET_ILLUMINATION_MIRROR_TABLE_2D* data
                = (MSG_H2V_SET_ILLUMINATION_MIRROR_TABLE_2D*)msgInfo.m_message;
            MSG_V2H_SET_ILLUMINATION_MIRROR_TABLE_2D_ACK ackData(data->m_messageToken);

            const char* fileData = (char*)(data + 1);

            CString strRefAll = DynamicSystemPath::get(DefineFile::IllumTableRefTableAllMirror);

            CFileFind fileFind;
            //BOOL bIsRefAll = fileFind.FindFile(strRefAll);

            CString strFileName(_T(""));
            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
            {
                switch (data->m_nCurSideVisionidx)
                {
                    case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                        strFileName = _T("IlluminationTableAllMirror_F.csv");
                        break;
                    case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                        strFileName = _T("IlluminationTableAllMirror_R.csv");
                        break;
                    default:
                        break;
                }
            }
            else
                strFileName = _T("IlluminationTableAllMirror.csv");

            //		else if (!bIsExistingRef && !bIsRefAll)
            //			strFileName = _T("IlluminationTableNGRV.csv");

            CString strResultPath(DynamicSystemPath::get(DefineFolder::System) + strFileName);

            CFile file;
            if (file.Open(strResultPath, CFile::modeCreate | CFile::modeWrite))
            {
                file.Write(
                    fileData, CAST_LONG(data->m_messageLength - sizeof(MSG_H2V_SET_ILLUMINATION_MIRROR_TABLE_2D)));

                file.Close();

                if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
                    ackData.m_result = CSyncCalculator::GetInstance().CalibrateIlluminationSide();
                else
                    ackData.m_result = CSyncCalculator::GetInstance().CalibrateIllumination2D();
            }
            else
            {
                ackData.m_result = false;
            }

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::GET_ILLUMINATION_EXTENSION_COUNT:
        {
            MSG_H2V_GET_ILLUMINATION_EXTENSION_COUNT* data
                = (MSG_H2V_GET_ILLUMINATION_EXTENSION_COUNT*)msgInfo.m_message;
            MSG_V2H_GET_2D_ILLUMINATION_EXTENSION_COUNT_ACK ackData(data->m_messageToken);

            ackData.m_illumination_extension_count
                = SystemConfig::GetInstance().GetExistRingillumination() == true ? LED_ILLUM_CHANNEL_RING_MAX : 0;
            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::GET_ILLUMINATION_CAL_USE_ALLDATA:
        {
            MSG_H2V_GET_ILLUMINATION_CAL_USE_ALLDATA* data
                = (MSG_H2V_GET_ILLUMINATION_CAL_USE_ALLDATA*)msgInfo.m_message;
            MSG_V2H_GET_2D_ILLUMINATION_CAL_USE_ALLDATA_ACK ackData(data->m_messageToken);

            CString strRefPath = DynamicSystemPath::get(DefineFile::IllumTableRefTable);
            CString strRefAllPath = DynamicSystemPath::get(DefineFile::IllumTableRefTableAll);

            CFileFind fileFind;
            ackData.m_bIsExistingRef = fileFind.FindFile(strRefPath);
            ackData.m_bIsRefAll = fileFind.FindFile(strRefAllPath);
            ackData.m_result = true;

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::PREPARE_SCAN_3D:
        {
            MSG_H2V_PREPARE_SCAN_3D* data = (MSG_H2V_PREPARE_SCAN_3D*)msgInfo.m_message;

            delete m_tuningSlitBeamParameters;
            m_tuningSlitBeamParameters = new SlitBeam3DParameters;
            m_tuningSlitBeamParameters->m_cameraVerticalBinning = data->m_cameraVerticalBinning;
            m_tuningSlitBeamParameters->m_illuminationTime_ms = data->m_illuminationTime_ms;
            m_tuningSlitBeamParameters->m_illuminationValue = data->m_illuminationValue;
            m_tuningSlitBeamParameters->m_beamThickness_um = data->m_beamThickness_um;
            m_tuningSlitBeamParameters->m_noiseThreshold = data->m_noiseThreshold;
            m_tuningSlitBeamParameters->m_noiseThreshold_FinalFiltering = m_tuningSlitBeamParameters->m_noiseThreshold
                * .5; // data->m_noiseThreshold_FinalFiltering;//kircheis_SDK127
            m_tuningSlitBeamParameters->m_scanDepth_um = data->m_scanDepth_um;
            m_tuningSlitBeamParameters->m_scanLength_mm = data->m_scanLength_mm;
            m_tuningSlitBeamParameters->m_scanStep_um = data->m_scanStep_um;

            // 조명 컨트롤러 설정
            SlitIlluminator::GetInstance().Download(
                INSTRUCTION_CHANNEL0_INTENSITY, m_tuningSlitBeamParameters->m_illuminationValue);

            // Sync 보드 설정
            SyncController::GetInstance().SetEncoderScaling(m_tuningSlitBeamParameters->m_scanStep_um);
            SyncController::GetInstance().SetEncoderMultiply(SystemConfig::GetInstance().m_slitbeamUseEncoderMultiply);
            SyncController::GetInstance().SetGrabPosition(static_cast<unsigned short>(
                SystemConfig::GetInstance().m_slitbeamScanStartOffset_um / m_tuningSlitBeamParameters->m_scanStep_um
                + 0.5));

            // 스캔 설정에 따라서 이미지 크기가 바뀌게 되므로 얘를 먼저 해 주어야 한다.
            FrameGrabber::GetInstance().set_slitbeam_parameters(0, 0, *m_tuningSlitBeamParameters,
                data->m_useHeightScale ? SystemConfig::GetInstance().m_slitbeamHeightScaling : 1.f,
                data->m_useHeightQuantization, data->m_useDistortionCompensation);

            // 노출 시간을 설정한다.
            double validFramePeriod_ms = 0.;
            FrameGrabber::GetInstance().set_exposure_time(
                0, 0, m_tuningSlitBeamParameters->m_illuminationTime_ms, validFramePeriod_ms);

            // 호스트로 모터 구동 속도를 알려주자
            MSG_V2H_PREPARE_SCAN_3D_ACK ackData(data->m_messageToken);
            ackData.m_scanVelocity_mm_per_sec = m_tuningSlitBeamParameters->m_scanStep_um / validFramePeriod_ms;
            ackData.m_result = true;

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::SINGLE_GRAB_2D:
        {
            MSG_H2V_SINGLE_GRAB_2D* data = (MSG_H2V_SINGLE_GRAB_2D*)msgInfo.m_message;

            MSG_V2H_SINGLE_GRAB_2D_ACK ackData(data->m_messageToken);
            ackData.m_result = true;

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            std::array<float, 16> illumValues_ms;
            memcpy(&illumValues_ms[0], data->m_illuminationTimes_ms, sizeof(data->m_illuminationTimes_ms));

            SystemConfig::GetInstance().m_bIsCalIllum2D = TRUE;
            SyncController::GetInstance().SetIllumiParameter(0, illumValues_ms, data->m_useIlluminationCompensation,
                false, false, enSideVisionModule(data->m_nCurSideVisionidx));
            SystemConfig::GetInstance().m_bIsCalIllum2D = FALSE;

            bool bChannelReverse = false;

            if (illumValues_ms[CHANNEL_REVERSE] > 0.f)
                bChannelReverse = true;

            Ipvm::Image8u image;
            Ipvm::Image8u3 imageColor;
            Ipvm::Image8u3 imageResizeColor;

            MSG_V2H_SINGLE_GRAB_IMAGE grab_data{};
            MSG_V2H_SINGLE_GRAB_IMAGE* sendData = nullptr;
            unsigned char* sendBuffer{};

            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
                || SystemConfig::GetInstance().GetVisionType()
                    == VISIONTYPE_SWIR_INSP) //kircheis_SideVision고려 //kircheis_SWIR
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());

                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);

                memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                sendData->m_bisColorimage = FALSE;
            }
            else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                imageColor.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());

                BYTE* imageMem = (BYTE*)image.GetMem();

                // NGRV Grab Retry 설정 : 최대 3회 실시 - 2023.03.09_JHB
                long nIdx = 0;
                bool bGrabSuccess = FALSE;
                CString strGrabFailLog;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                do
                {
                    FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMem, 1, nullptr);
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                    bGrabSuccess = FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);
                    nIdx++;

                    strGrabFailLog.Format(_T("NGRV Grab Fail_%d"), nIdx);
                    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, strGrabFailLog);
                }
                while (bGrabSuccess == FALSE && nIdx < 3);

                /*SyncController::GetInstance().SetFrameCount(1, 0);
			FrameGrabber::GetInstance().StartGrabNGRV(&imageMem, 1, nullptr);			
			SyncController::GetInstance().StartSyncBoard();
			FrameGrabber::GetInstance().WaitGrabEnd(nullptr);*/

                if (data->m_bisimageResize)
                {
                    long nOriginalImageSizeX = ORIGINAL_NGRV_IMAGE_SIZE_X_SPLIT_COLOR;
                    long nOriginalImageSizeY = ORIGINAL_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR;
                    long nResizeImageSizeX = RESIZE_NGRV_IMAGE_SIZE_X_SPLIT_COLOR;
                    long nResizeImageSizeY = RESIZE_NGRV_IMAGE_SIZE_Y_SPLIT_COLOR;

                    std::vector<BYTE> vecbyResizeImage(nResizeImageSizeX * nResizeImageSizeY);

                    long nDstX(0), nDstY(0);
                    long nIdxY = 0;
                    for (long nY = 0; nY < nOriginalImageSizeY; nY += 4)
                    {
                        nIdxY = nDstY * nResizeImageSizeX;
                        nDstY++;
                        nDstX = 0;
                        for (long nX = 0; nX < nOriginalImageSizeX; nX += 4)
                        {
                            vecbyResizeImage[nIdxY + nDstX] = image.GetMem(nX, nY)[0];
                            nDstX++;
                        }
                    }

                    Ipvm::Image8u imageResizeMono(
                        nResizeImageSizeX, nResizeImageSizeY, &vecbyResizeImage[0], nResizeImageSizeX);

                    grab_data.m_imageSizeX = nResizeImageSizeX;
                    grab_data.m_imageSizeY = nResizeImageSizeY;

                    grab_data.m_messageLength = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE)
                        + imageResizeMono.GetWidthBytes() * imageResizeMono.GetSizeY();

                    sendBuffer = new unsigned char[grab_data.m_messageLength];

                    sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                    *sendData = grab_data;

                    memcpy(sendData + 1, imageResizeMono.GetMem(),
                        imageResizeMono.GetWidthBytes() * imageResizeMono.GetSizeY());

                    sendData->m_bisColorimage = TRUE;
                    sendData->m_bisGreenimage = TRUE;
                }
                else
                {
                    FrameGrabber::GetInstance().DoColorConversion(
                        image, imageColor, data->m_bisWhiteBalanceFunction, bChannelReverse);

                    grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                    grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();

                    grab_data.m_messageLength
                        = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + imageColor.GetWidthBytes() * imageColor.GetSizeY();

                    sendBuffer = new unsigned char[grab_data.m_messageLength];

                    sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                    *sendData = grab_data;

                    memcpy(sendData + 1, imageColor.GetMem(), imageColor.GetWidthBytes() * imageColor.GetSizeY());

                    sendData->m_bisColorimage = TRUE;
                    sendData->m_bisGreenimage = FALSE;
                }
            }
            else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());

                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule(data->m_nCurSideVisionidx));
                FrameGrabber::GetInstance().StartGrab2D(0, data->m_nCurSideVisionidx, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule(data->m_nCurSideVisionidx));
                FrameGrabber::GetInstance().wait_grab_end(0, data->m_nCurSideVisionidx, nullptr);

                memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                sendData->m_bisColorimage = FALSE;
            }
            if (sendData == nullptr)
            {
                ::SimpleMessage(_T("Unknown Vision Type"));
                delete[] sendBuffer;
                return 0;
            }
            else
            {
                m_visionMain.m_tuningMessageSocket->Write(
                    long(sendData->m_messageIndex), sendData->m_messageLength, sendData);

                delete[] sendBuffer;
            }

            //SyncController::GetInstance().TurnOffLight();
        }
        break;
        case MSG_H2V::SINGLE_SCAN_3D:
        {
            MSG_H2V_SINGLE_SCAN_3D* data = (MSG_H2V_SINGLE_SCAN_3D*)msgInfo.m_message;

            VisionImageLot imageLot;

            auto* slitBeamParameter = m_visionMain.m_pSlitBeam3DCommon;

            if (m_tuningSlitBeamParameters)
            {
                // MSG_H2V::PREPARE_SCAN_3D 메시지를 통해 Tuning Slit Beam Parameter를
                // 받은 경우에는 그것을 이용하자.
                //
                // 주의할 점은 AutoFocus 3D에서는 MSG_H2V::PREPARE_SCAN_3D메시지를 주지않고
                // 스캔한다. 이유는 Recipe Parameter를 그대로 이용하기 위해서다.

                slitBeamParameter = m_tuningSlitBeamParameters;
            }

            auto bufferSize = VisionImageLot::Get3DMaxBufferSize(
                FrameGrabber::GetInstance().get_grab_image_width(), 1, slitBeamParameter->GetLogicalScanLength_px());

            imageLot.Setup3D(FrameGrabber::GetInstance().get_grab_image_width(),
                FrameGrabber::GetInstance().get_grab_image_height(), bufferSize);

            MSG_V2H_SINGLE_SCAN_3D_ACK ackData(data->m_messageToken);
            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            m_visionMain.ManualGrab3DSequence(imageLot);

            const auto& zmapImage = imageLot.GetZmapImage(0);
            const auto& vmapImage = imageLot.GetVmapImage(0);

            const float HeightRangeMin = imageLot.GetInfo(enSideVisionModule::SIDE_VISIONMODULE_FRONT).m_heightRangeMin;
            const float HeightRangeMax = imageLot.GetInfo(enSideVisionModule::SIDE_VISIONMODULE_FRONT).m_heightRangeMax;

            MSG_V2H_SINGLE_ZMAP_IMAGE zmap_data{};
            zmap_data.m_imageSizeX = zmapImage.GetSizeX();
            zmap_data.m_imageSizeY = zmapImage.GetSizeY();
            zmap_data.m_heightRangeMin = HeightRangeMin;
            zmap_data.m_heightRangeMax = HeightRangeMax;
            zmap_data.m_messageLength
                = sizeof(MSG_V2H_SINGLE_ZMAP_IMAGE) + zmapImage.GetWidthBytes() * zmapImage.GetSizeY();

            if (data->m_getVmap)
            {
                zmap_data.m_messageLength += vmapImage.GetWidthBytes() * vmapImage.GetSizeY();
            }

            unsigned char* sendBuffer = new unsigned char[zmap_data.m_messageLength];

            MSG_V2H_SINGLE_ZMAP_IMAGE* sendData = (MSG_V2H_SINGLE_ZMAP_IMAGE*)sendBuffer;

            *sendData = zmap_data;

            memcpy(sendData + 1, zmapImage.GetMem(), zmapImage.GetWidthBytes() * zmapImage.GetSizeY());

            if (data->m_getVmap)
            {
                BYTE* snedData_Vmap = ((BYTE*)(sendData + 1)) + zmapImage.GetWidthBytes() * zmapImage.GetSizeY();
                memcpy(snedData_Vmap, vmapImage.GetMem(), vmapImage.GetWidthBytes() * vmapImage.GetSizeY());
            }

            m_visionMain.m_tuningMessageSocket->Write(
                long(sendData->m_messageIndex), sendData->m_messageLength, sendData);

            delete[] sendBuffer;
        }
        break;
        case MSG_H2V::START_LIVE_2D:
        {
            MSG_H2V_START_LIVE_2D* data = (MSG_H2V_START_LIVE_2D*)msgInfo.m_message;

            MSG_V2H_START_LIVE_2D_ACK ackData(data->m_messageToken);
            ackData.m_result = true;

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            std::array<float, 16> illumValues_ms;
            memcpy(&illumValues_ms[0], data->m_illuminationTimes_ms, sizeof(data->m_illuminationTimes_ms));

            SyncController::GetInstance().SetIllumiParameter(0, illumValues_ms, data->m_useIlluminationCompensation,
                false, false, enSideVisionModule(data->m_nCurSideVisionidx));
            SyncController::GetInstance().TurnOnLight(0, enSideVisionModule(data->m_nCurSideVisionidx));
            FrameGrabber::GetInstance().live_on(
                GetSafeHwnd(), UM_VISION_TUNING_LIVE_2D, 500, enSideVisionModule(data->m_nCurSideVisionidx));
        }
        break;
        case MSG_H2V::GET_2D_SCALE:
        {
            MSG_H2V_GET_2D_SCALE* data
                = (MSG_H2V_GET_2D_SCALE*)
                      msgInfo.m_message; //Data안에 Front인지 Rear인지 알려줘야한다 일단은 Front 기준으로 맞춘다

            MSG_V2H_GET_2D_SCALE_ACK ackData(data->m_messageToken);
            ackData.m_currentScaleX
                = SystemConfig::GetInstance().Get2DScaleX(enSideVisionModule(data->m_nCurSideVisionidx));
            ackData.m_currentScaleY
                = SystemConfig::GetInstance().Get2DScaleY(enSideVisionModule(data->m_nCurSideVisionidx));
            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::SET_2D_SCALE:
        {
            MSG_H2V_SET_2D_SCALE* data = (MSG_H2V_SET_2D_SCALE*)msgInfo.m_message;

            MSG_V2H_SET_2D_SCALE_ACK ackData(data->m_messageToken);
            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            //Data안에 Front인지 Rear인지 알려줘야한다 일단은 Front 기준으로 맞춘다
            SystemConfig::GetInstance().Set2DScale(
                data->m_newScaleX, data->m_newScaleY, enSideVisionModule(data->m_nCurSideVisionidx));

            m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView->UpdateState();
            m_visionMain.iPIS_Send_SystemParameters();
        }
        break;
        case MSG_H2V::START_LIVE_3D:
        {
            MSG_H2V_START_LIVE_3D* data = (MSG_H2V_START_LIVE_3D*)msgInfo.m_message;

            FrameGrabber::GetInstance().live_off();
            FrameGrabber::GetInstance().set_live_image_size(0, 0, false, true);

            SlitIlluminator::GetInstance().Download(INSTRUCTION_CHANNEL0_INTENSITY, data->m_illuminationValue);

            double validFramePeriod_ms = 0.;
            FrameGrabber::GetInstance().set_exposure_time(0, 0, data->m_illuminationTime_ms, validFramePeriod_ms);

            SyncController::GetInstance().TurnOnLight(0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            FrameGrabber::GetInstance().live_on(GetSafeHwnd(), UM_VISION_TUNING_LIVE_3D, 500L);

            MSG_V2H_START_LIVE_3D_ACK ackData(data->m_messageToken);
            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::STOP_LIVE:
        {
            MSG_H2V_STOP_LIVE* data = (MSG_H2V_STOP_LIVE*)msgInfo.m_message;

            FrameGrabber::GetInstance().live_off();
            FrameGrabber::GetInstance().set_live_image_size(
                0, 0, m_visionMain.m_pSlitBeam3DCommon->m_cameraVerticalBinning, false);
            SyncController::GetInstance().TurnOffLight(enSideVisionModule(data->m_nCurSideVisionidx));

            MSG_V2H_STOP_LIVE_ACK ackData(data->m_messageToken);
            ackData.m_result = true;

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::GET_3D_SCALE:
        {
            MSG_H2V_GET_3D_SCALE* data = (MSG_H2V_GET_3D_SCALE*)msgInfo.m_message;
            MSG_V2H_GET_3D_SCALE_ACK ackData(data->m_messageToken);
            ackData.m_result = true;
            ackData.m_slitbeamHeightScaling = SystemConfig::GetInstance().m_slitbeamHeightScaling;
            ackData.m_slitbeamCameraOffsetY = SystemConfig::GetInstance().m_slitbeamCameraOffsetY;
            ackData.m_scaleZ
                = FrameGrabber::GetInstance().get_height_resolution_px2um(false, ackData.m_slitbeamHeightScaling);

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::SET_3D_SCALE:
        {
            MSG_H2V_SET_3D_SCALE* data = (MSG_H2V_SET_3D_SCALE*)msgInfo.m_message;
            MSG_V2H_SET_3D_SCALE_ACK ackData(data->m_messageToken);

            SystemConfig::GetInstance().m_slitbeamHeightScaling = data->m_newSlitbeamHeightScaling;
            SystemConfig::GetInstance().SetScale3D(
                FrameGrabber::GetInstance().get_height_resolution_px2um(false, data->m_newSlitbeamHeightScaling));

            if (data->m_applySystemFile)
            {
                SystemConfig::GetInstance().SaveIni(DynamicSystemPath::get(DefineFile::System));
            }

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
            m_visionMain.iPIS_Send_SystemParameters();
        }
        break;

        case MSG_H2V::SET_SLITBEAM_CAMERA_OFFSET:
        {
            MSG_H2V_SET_SLITBEAM_CAMERA_OFFSET* data = (MSG_H2V_SET_SLITBEAM_CAMERA_OFFSET*)msgInfo.m_message;
            MSG_V2H_SET_SLITBEAM_CAMERA_OFFSET_ACK ackData(data->m_messageToken);

            SystemConfig::GetInstance().m_slitbeamCameraOffsetY = data->m_newSlitbeamCameraOffsetY;
            SystemConfig::GetInstance().SaveIni(DynamicSystemPath::get(DefineFile::System));

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;

        case MSG_H2V::SET_SLITBEAM_DISTORTION_PROFILE:
        {
            MSG_H2V_SET_SLITBEAM_DISTORTION_PROFILE* data = (MSG_H2V_SET_SLITBEAM_DISTORTION_PROFILE*)msgInfo.m_message;
            MSG_V2H_SET_SLITBEAM_DISTORTION_PROFILE_ACK ackData(data->m_messageToken);

            const char* fileData = (char*)(data + 1);

            CFile file;
            if (file.Open(DynamicSystemPath::get(DefineFile::SlitbeamDistortionProfile),
                    CFile::modeCreate | CFile::modeWrite))
            {
                file.Write(
                    fileData, CAST_UINT(data->m_messageLength - sizeof(MSG_H2V_SET_SLITBEAM_DISTORTION_PROFILE)));
                file.Close();

                ackData.m_result = true;
                FrameGrabber::GetInstance().init_slit_beam_distortion();
            }
            else
            {
                ackData.m_result = false;
            }

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;

        case MSG_H2V::SET_SLITBEAM_DISTORTION_FULL:
        {
            MSG_H2V_SET_SLITBEAM_DISTORTION_FULL* data = (MSG_H2V_SET_SLITBEAM_DISTORTION_FULL*)msgInfo.m_message;
            MSG_V2H_SET_SLITBEAM_DISTORTION_FULL_ACK ackData(data->m_messageToken);

            const char* fileData = (char*)(data + 1);

            CFile file;
            if (file.Open(DynamicSystemPath::getDistortionFullFilePath(
                              data->m_scanDepth_um, data->m_cameraVerticalBinning, false),
                    CFile::modeCreate | CFile::modeWrite))
            {
                file.Write(fileData, CAST_UINT(data->m_messageLength - sizeof(MSG_H2V_SET_SLITBEAM_DISTORTION_FULL)));

                file.Close();

                ackData.m_result = true;
                FrameGrabber::GetInstance().init_slit_beam_distortion();
            }
            else
            {
                ackData.m_result = false;
            }

            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;

        case MSG_H2V::SINGLE_GRAB_2D_UNIFORMITY_FOR_GV:
        {
            MSG_H2V_SINGLE_GRAB_2D_UNIFORMITY_FOR_GV* data
                = (MSG_H2V_SINGLE_GRAB_2D_UNIFORMITY_FOR_GV*)msgInfo.m_message;
            MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_GV_ACK ackData(data->m_messageToken);

            int channel_no = data->m_channel_no;
            std::array<float, 16> illumIntensity_gv;
            memcpy(&illumIntensity_gv[0], data->m_illuminationIntensity_gv, sizeof(data->m_illuminationIntensity_gv));

            float grayTargetIntensity_gv[16];
            memset(grayTargetIntensity_gv, 0, sizeof(grayTargetIntensity_gv));
            std::array<float, 16> illumValues_ms;
            memcpy(&illumValues_ms[0], grayTargetIntensity_gv, sizeof(grayTargetIntensity_gv));

            float illumTime_ms = 0.f;
            auto& sysConfig = SystemConfig::GetInstance();
            if (channel_no <= 12)
            {
                auto lineEqRef = sysConfig.m_vecIllumFittingRef[channel_no];
                illumTime_ms = float((lineEqRef.m_c / lineEqRef.m_b) + illumIntensity_gv[channel_no])
                    / (-lineEqRef.m_a / lineEqRef.m_b);
                illumValues_ms[channel_no] = illumTime_ms;
            }

            ackData.m_result = true;
            ackData.m_illumtime_ms = illumTime_ms;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            SyncController::GetInstance().SetIllumiParameter(0, illumValues_ms, data->m_useIlluminationCompensation,
                false, false, enSideVisionModule(data->m_nCurSideVisionidx));

            bool bChannelReverse = false;

            if (channel_no == CHANNEL_REVERSE)
                bChannelReverse = true;

            Ipvm::Image8u image;
            Ipvm::Image8u3 imageColor;

            MSG_V2H_SINGLE_GRAB_IMAGE grab_data{};
            MSG_V2H_SINGLE_GRAB_IMAGE* sendData = nullptr;

            unsigned char* sendBuffer{};

            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
                || SystemConfig::GetInstance().GetVisionType()
                    == VISIONTYPE_SWIR_INSP) //kircheis_SideVision고려 //kircheis_SWIR
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);

                memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                sendData->m_bisColorimage = FALSE;
            }
            else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule(data->m_nCurSideVisionidx));
                FrameGrabber::GetInstance().StartGrab2D(0, data->m_nCurSideVisionidx, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule(data->m_nCurSideVisionidx));
                FrameGrabber::GetInstance().wait_grab_end(0, data->m_nCurSideVisionidx, nullptr);

                memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                sendData->m_bisColorimage = FALSE;
            }
            else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                imageColor.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + imageColor.GetWidthBytes() * imageColor.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                // NGRV Grab Retry 설정 : 최대 3회 실시 - 2023.03.09_JHB
                long nIdx = 0;
                bool bGrabSuccess = FALSE;
                CString strGrabFailLog;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                do
                {
                    FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMem, 1, nullptr);
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                    bGrabSuccess = FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);
                    nIdx++;

                    strGrabFailLog.Format(_T("NGRV Grab Fail_%d"), nIdx);
                    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, strGrabFailLog);
                }
                while (bGrabSuccess == FALSE && nIdx < 3);

                /*SyncController::GetInstance().SetFrameCount(1, 0);
			FrameGrabber::GetInstance().StartGrabNGRV(&imageMem, 1, nullptr);
			SyncController::GetInstance().StartSyncBoard();
			FrameGrabber::GetInstance().WaitGrabEnd(nullptr);*/

                FrameGrabber::GetInstance().DoColorConversion(image, imageColor, false, bChannelReverse);

                memcpy(sendData + 1, imageColor.GetMem(), imageColor.GetWidthBytes() * imageColor.GetSizeY());

                sendData->m_bisColorimage = TRUE;
                sendData->m_bisGreenimage = FALSE;
            }
            if (sendData == nullptr)
            {
                ::SimpleMessage(_T("Unknown Vision Type"));
                delete[] sendBuffer;
                return 0;
            }
            else
            {
                m_visionMain.m_tuningMessageSocket->Write(
                    long(sendData->m_messageIndex), sendData->m_messageLength, sendData);
                delete[] sendBuffer;
            }

            //SyncController::GetInstance().TurnOffLight();
        }
        break;

        case MSG_H2V::SINGLE_GRAB_2D_UNIFORMITY_FOR_MS:
        {
            MSG_H2V_SINGLE_GRAB_2D_UNIFORMITY_FOR_MS* data
                = (MSG_H2V_SINGLE_GRAB_2D_UNIFORMITY_FOR_MS*)msgInfo.m_message;
            MSG_V2H_SINGLE_GRAB_2D_UNIFORMITY_FOR_MS_ACK ackData(data->m_messageToken);

            int channel_no = data->m_channel_no;
            std::array<float, 16> illumIntensity_ms;
            memcpy(&illumIntensity_ms[0], data->m_illuminationIntensity_ms, sizeof(data->m_illuminationIntensity_ms));

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            SyncController::GetInstance().SetIllumiParameter(0, illumIntensity_ms, data->m_useIlluminationCompensation,
                false, false, enSideVisionModule(data->m_nCurSideVisionidx));

            bool bChannelReverse = false;

            if (channel_no == CHANNEL_REVERSE)
                bChannelReverse = true;

            Ipvm::Image8u image;
            Ipvm::Image8u3 imageColor;

            MSG_V2H_SINGLE_GRAB_IMAGE grab_data{};
            MSG_V2H_SINGLE_GRAB_IMAGE* sendData = nullptr;

            unsigned char* sendBuffer{};

            if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
                || SystemConfig::GetInstance().GetVisionType()
                    == VISIONTYPE_SWIR_INSP) //kircheis_SideVision고려 //kircheis_SWIR
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);

                memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                sendData->m_bisColorimage = FALSE;
            }
            else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP)
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + image.GetWidthBytes() * image.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule(data->m_nCurSideVisionidx));
                FrameGrabber::GetInstance().StartGrab2D(0, data->m_nCurSideVisionidx, &imageMem, 1, nullptr);
                SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule(data->m_nCurSideVisionidx));
                FrameGrabber::GetInstance().wait_grab_end(0, data->m_nCurSideVisionidx, nullptr);

                memcpy(sendData + 1, image.GetMem(), image.GetWidthBytes() * image.GetSizeY());

                sendData->m_bisColorimage = FALSE;
            }
            else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
            {
                image.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                imageColor.Create(FrameGrabber::GetInstance().get_grab_image_width(),
                    FrameGrabber::GetInstance().get_grab_image_height());
                BYTE* imageMem = image.GetMem();

                grab_data.m_imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
                grab_data.m_imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();
                grab_data.m_messageLength
                    = sizeof(MSG_V2H_SINGLE_GRAB_IMAGE) + imageColor.GetWidthBytes() * imageColor.GetSizeY();

                sendBuffer = new unsigned char[grab_data.m_messageLength];

                sendData = (MSG_V2H_SINGLE_GRAB_IMAGE*)sendBuffer;

                *sendData = grab_data;

                // NGRV Grab Retry 설정 : 최대 3회 실시 - 2023.03.09_JHB
                long nIdx = 0;
                bool bGrabSuccess = FALSE;
                CString strGrabFailLog;

                SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                do
                {
                    FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMem, 1, nullptr);
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

                    bGrabSuccess = FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);
                    nIdx++;

                    strGrabFailLog.Format(_T("NGRV Grab Fail_%d"), nIdx);
                    DevelopmentLog::AddLog(DevelopmentLog::Type::Event, strGrabFailLog);
                }
                while (bGrabSuccess == FALSE && nIdx < 3);

                /*SyncController::GetInstance().SetFrameCount(1, 0);
			FrameGrabber::GetInstance().StartGrabNGRV(&imageMem, 1, nullptr);
			SyncController::GetInstance().StartSyncBoard();
			FrameGrabber::GetInstance().WaitGrabEnd(nullptr);*/

                FrameGrabber::GetInstance().DoColorConversion(image, imageColor, false, bChannelReverse);

                memcpy(sendData + 1, imageColor.GetMem(), imageColor.GetWidthBytes() * imageColor.GetSizeY());

                sendData->m_bisColorimage = TRUE;
                sendData->m_bisGreenimage = FALSE;
            }

            if (sendData == nullptr)
            {
                ::SimpleMessage(_T("Unknown Vision Type"));
                delete[] sendBuffer;
                return 0;
            }
            else
            {
                m_visionMain.m_tuningMessageSocket->Write(
                    long(sendData->m_messageIndex), sendData->m_messageLength, sendData);

                delete[] sendBuffer;
            }

            //SyncController::GetInstance().TurnOffLight();
        }
        break;

        case MSG_H2V::GET_WHITE_BALANCE_GAIN:
        {
            MSG_H2V_GET_WHITE_BALANCE_GAIN* data = (MSG_H2V_GET_WHITE_BALANCE_GAIN*)msgInfo.m_message;

            MSG_V2H_GET_WHITE_BALANCE_GAIN_ACK ackData(data->m_messageToken);

            if (!data->m_isReverseGain) // Normal Channel Gain을 보낼 때,
            {
                ackData.m_currentWhiteBalanceGainValue_R = SystemConfig::GetInstance().m_fNormal_Channel_GainR;
                ackData.m_currentWhiteBalanceGainValue_G = SystemConfig::GetInstance().m_fNormal_Channel_GainG;
                ackData.m_currentWhiteBalanceGainValue_B = SystemConfig::GetInstance().m_fNormal_Channel_GainB;
                ackData.m_isReverseGain = false;
            }
            else // Reverse Channel Gain을 보낼 때,
            {
                ackData.m_currentWhiteBalanceGainValue_R = SystemConfig::GetInstance().m_fReverse_Channel_GainR;
                ackData.m_currentWhiteBalanceGainValue_G = SystemConfig::GetInstance().m_fReverse_Channel_GainG;
                ackData.m_currentWhiteBalanceGainValue_B = SystemConfig::GetInstance().m_fReverse_Channel_GainB;
                ackData.m_isReverseGain = true;
            }

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;

        case MSG_H2V::SET_WHITE_BALANCE_GAIN:
        {
            MSG_H2V_SET_WHITE_BALANCE_GAIN* data = (MSG_H2V_SET_WHITE_BALANCE_GAIN*)msgInfo.m_message;

            MSG_V2H_SET_WHITE_BALANCE_GAIN_ACK ackData(data->m_messageToken);
            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            if (!data->m_bisReverseGain)
            {
                SystemConfig::GetInstance().m_fNormal_Channel_GainR = data->m_newWhiteBalanceGainValue_R;
                SystemConfig::GetInstance().m_fNormal_Channel_GainG = data->m_newWhiteBalanceGainValue_G;
                SystemConfig::GetInstance().m_fNormal_Channel_GainB = data->m_newWhiteBalanceGainValue_B;
            }
            else
            {
                SystemConfig::GetInstance().m_fReverse_Channel_GainR = data->m_newWhiteBalanceGainValue_R;
                SystemConfig::GetInstance().m_fReverse_Channel_GainG = data->m_newWhiteBalanceGainValue_G;
                SystemConfig::GetInstance().m_fReverse_Channel_GainB = data->m_newWhiteBalanceGainValue_B;
            }

            SystemConfig::GetInstance().SaveIni(DynamicSystemPath::get(DefineFile::System), TRUE);
        }
        break;

        case MSG_H2V::GET_UVIR_CHANNEL_IDX:
        {
            MSG_H2V_GET_UVIR_CHANNEL_IDX* data = (MSG_H2V_GET_UVIR_CHANNEL_IDX*)msgInfo.m_message;

            MSG_V2H_GET_UVIR_CHANNEL_IDX_ACK ackData(data->m_messageToken);

            ackData.m_nUV_Channel_idx = SystemConfig::GetInstance().m_nNgrvUVchID;
            ackData.m_nIR_Channel_idx = SystemConfig::GetInstance().m_nNgrvIRchID;

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::GET_2D_CAMERA_OFFSET_Y:
        {
            MSG_H2V_GET_2D_CAMERA_OFFSET_Y* data = (MSG_H2V_GET_2D_CAMERA_OFFSET_Y*)msgInfo.m_message;

            MSG_V2H_GET_2D_CAMERA_OFFSET_Y_ACK ackData(data->m_messageToken);

            switch (data->m_nCurSideVisionidx)
            {
                case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                    ackData.m_nCameraOffsetY = SystemConfig::GetInstance().GetSideVisionFrontCameraOffsetY();
                    break;
                case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                    ackData.m_nCameraOffsetY = SystemConfig::GetInstance().GetSideVisionRearCameraOffsetY();
                    break;
                default:
                    break;
            }

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;
        case MSG_H2V::SET_2D_CAMERA_OFFSET_Y:
        {
            MSG_H2V_SET_2D_CAMERA_OFFSET_Y* data = (MSG_H2V_SET_2D_CAMERA_OFFSET_Y*)msgInfo.m_message;

            MSG_V2H_SET_2D_CAMERA_OFFSET_Y_ACK ackData(data->m_messageToken);

            switch (data->m_nCurSideVisionidx)
            {
                case enSideVisionModule::SIDE_VISIONMODULE_FRONT:
                    SystemConfig::GetInstance().SetSideVisionFrontCameraOffsetY(data->m_nCameraOffsetY);
                    break;
                case enSideVisionModule::SIDE_VISIONMODULE_REAR:
                    SystemConfig::GetInstance().SetSideVisionRearCameraOffsetY(data->m_nCameraOffsetY);
                    break;
                default:
                    break;
            }

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            SystemConfig::GetInstance().SaveIni(DynamicSystemPath::get(DefineFile::System));
            FrameGrabber::GetInstance().set_grab_image_offset_Y(0, data->m_nCurSideVisionidx, data->m_nCameraOffsetY);
        }
        break;
        case MSG_H2V::GET_3D_SHEARING_FACTOR:
        {
            MSG_H2V_GET_3D_SHEARING_FACTOR* data = (MSG_H2V_GET_3D_SHEARING_FACTOR*)msgInfo.m_message;

            MSG_V2H_GET_3D_SHEARING_FACTOR_ACK ackData(data->m_messageToken);

            ackData.m_ShearingFactor = SystemConfig::GetInstance().Get_SpaceTimeSamplingScaleFactor();

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);
        }
        break;

        case MSG_H2V::SET_3D_SHEARING_FACTOR:
        {
            MSG_H2V_SET_3D_SHEARING_FACTOR* data = (MSG_H2V_SET_3D_SHEARING_FACTOR*)msgInfo.m_message;

            MSG_V2H_SET_3D_SHEARING_FACTOR_ACK ackData(data->m_messageToken);

            SystemConfig::GetInstance().Set_SpaceTimeSamplingScaleFactor(data->m_newShearingFactor);

            ackData.m_result = true;
            m_visionMain.m_tuningMessageSocket->Write(long(ackData.m_messageIndex), ackData.m_messageLength, &ackData);

            if (data->m_applySystemFile == true)
                SystemConfig::GetInstance().SaveIni(DynamicSystemPath::get(DefineFile::System));

            FrameGrabber::GetInstance().set_slitbeam_parameters(
                0, 0, *m_tuningSlitBeamParameters, SystemConfig::GetInstance().m_slitbeamHeightScaling, true, true);
        }
        break;
    }

    return 0;
}

LRESULT CMainFrame::OnTuningLive2DEvent(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);

    static Ipvm::Image8u grabImage;
    static Ipvm::Image8u3 grabImageColor;

    MSG_V2H_LIVE_IMAGE data{};

    unsigned char* sendBuffer{};

    MSG_V2H_LIVE_IMAGE* sendData = nullptr;

    static const bool bIsBased2D = SystemConfig::GetInstance().IsVisionTypeBased2D(); //kircheis_SWIR

    if (bIsBased2D) //kircheis_SideVision고려 //kircheis_SWIR
    {
        FrameGrabber::GetInstance().get_live_image(grabImage);
        m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView->SetImage(grabImage, _T(""));

        if (grabImage.GetMem() == nullptr)
            return FALSE;

        data.m_imageSizeX = grabImage.GetSizeX();
        data.m_imageSizeY = grabImage.GetSizeY();
        data.m_messageLength = sizeof(MSG_V2H_LIVE_IMAGE) + grabImage.GetWidthBytes() * grabImage.GetSizeY();
        data.m_bisColorimage = FALSE;

        sendBuffer = new unsigned char[data.m_messageLength];

        sendData = (MSG_V2H_LIVE_IMAGE*)sendBuffer;

        *sendData = data;

        memcpy(sendData + 1, grabImage.GetMem(), grabImage.GetWidthBytes() * grabImage.GetSizeY());
    }
    else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
    {
        FrameGrabber::GetInstance().get_live_image(grabImageColor);
        m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView->SetImage(grabImageColor, _T(""));

        if (grabImageColor.GetMem() == nullptr)
            return FALSE;

        data.m_imageSizeX = grabImageColor.GetSizeX();
        data.m_imageSizeY = grabImageColor.GetSizeY();
        data.m_messageLength = sizeof(MSG_V2H_LIVE_IMAGE) + grabImageColor.GetWidthBytes() * grabImageColor.GetSizeY();
        data.m_bisColorimage = TRUE;

        sendBuffer = new unsigned char[data.m_messageLength];

        sendData = (MSG_V2H_LIVE_IMAGE*)sendBuffer;

        *sendData = data;

        memcpy(sendData + 1, grabImageColor.GetMem(), grabImageColor.GetWidthBytes() * grabImageColor.GetSizeY());
    }

    if (sendData != nullptr)
    {
        m_visionMain.m_tuningMessageSocket->Write(long(sendData->m_messageIndex), sendData->m_messageLength, sendData);

        delete[] sendBuffer;
    }

    return 0;
}

LRESULT CMainFrame::OnTuningLive3DEvent(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);

    static Ipvm::Image8u grabImage;
    FrameGrabber::GetInstance().get_live_image(grabImage);
    m_visionMain.m_pVisionPrimaryUI->m_pDlg->m_imageLotView->SetImage(grabImage, _T(""));

    if (grabImage.GetMem() == nullptr)
        return FALSE;

    MSG_V2H_LIVE_IMAGE data{};
    data.m_imageSizeX = grabImage.GetSizeX();
    data.m_imageSizeY = grabImage.GetSizeY();
    data.m_messageLength = sizeof(MSG_V2H_LIVE_IMAGE) + grabImage.GetWidthBytes() * grabImage.GetSizeY();
    data.m_bisColorimage = FALSE;

    unsigned char* sendBuffer = new unsigned char[data.m_messageLength];

    MSG_V2H_LIVE_IMAGE* sendData = (MSG_V2H_LIVE_IMAGE*)sendBuffer;

    *sendData = data;

    memcpy(sendData + 1, grabImage.GetMem(), grabImage.GetWidthBytes() * grabImage.GetSizeY());

    m_visionMain.m_tuningMessageSocket->Write(long(sendData->m_messageIndex), sendData->m_messageLength, sendData);

    delete[] sendBuffer;

    return 0;
}