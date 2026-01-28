//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "FOVImageGrabberNGRV.h"

//CPP_2_________________________________ This project's headers
#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/AlgorithmGpu/ImageProcessingGpu.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Point8u3.h>
#include <Ipvm/Base/TimeCheck.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//

#define SINGLE_RUN_GRAB_RETRY_COUNT 3

FOVImageGrabberNGRV::FOVImageGrabberNGRV(CVisionMain* visionMain)
    : FOVImageGrabber(visionMain)
    , m_totalFrameCount(0)
    , m_highPosFrameCount(0)
{
    long imageSizeX = FrameGrabber::GetInstance().get_grab_image_width();
    long imageSizeY = FrameGrabber::GetInstance().get_grab_image_height();

    for (long index = 0; index < m_bufferMaxCount; index++)
    {
        auto buffer = std::make_shared<FOVBuffer>();

        for (long frameIndex = 0; frameIndex < LED_ILLUM_NGRV_CHANNEL_MAX; frameIndex++)
        {
            buffer->m_imagePtrs[frameIndex] = new BYTE[imageSizeX * imageSizeY];
        }

        m_buffers.push_back(buffer);
    }
}

FOVImageGrabberNGRV::~FOVImageGrabberNGRV()
{
    for (auto& buffer : m_buffers)
    {
        for (long frameIndex = 0; frameIndex < LED_ILLUM_NGRV_CHANNEL_MAX; frameIndex++)
        {
            delete[] buffer->m_imagePtrs[frameIndex];
        }
    }
}

void FOVImageGrabberNGRV::allocateBuffer(long totalFrameCount, long highPosFrameCount)
{
    m_totalFrameCount = totalFrameCount;
    m_highPosFrameCount = highPosFrameCount;
}

BOOL FOVImageGrabberNGRV::GetPackageID(long& o_nPackageID, CString& o_strPackageID) //kircheis_NGRV
{
    o_nPackageID = -1;
    long nPackageID = 0;
    o_strPackageID = "";

    auto& syncInstance = SyncController::GetInstance();

    BOOL bIsOnReady = syncInstance.GetReadySignal();
    BOOL bIsGrabIO = syncInstance.GetSignalNGRV_GrabID();
    DevelopmentLog::AddLog(
        DevelopmentLog::Type::Develop, _T("NGRV_Grab - Ready:%d   GrabIO:%d"), (long)bIsOnReady, (long)bIsGrabIO);

    if (syncInstance.GetSignalNGRV_GrabID() == TRUE)
    {
        m_visionMain->iPIS_Send_ErrorMessageForLog(
            _T("Inline Grab"), _T("Grab ID Signal is on while receiving PKG ID"));
        m_visionMain->iPIS_Send_ErrorMessageForPopup(_T("Grab ID Signal is on while receiving PKG ID"));
        return false;
    }

    nPackageID = (long)syncInstance.GetFovIndex();

    DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV_Grab - PackageID:%03d"), nPackageID);

    auto vecPackageGrabInfo = m_visionMain->m_vecPackageGrabInfo;
    long nPackageNum = (long)vecPackageGrabInfo.size();

    static long nReceivedPackageID = -1;

    if (nPackageNum <= nPackageID)
    {
        CString strErrorMessage;
        strErrorMessage.Format(
            _T("Vision recieved the invalid package ID\nLimit: %03d, Input: %03d"), nPackageNum - 1, nPackageID);

        if (nReceivedPackageID != nPackageID)
        {
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), strErrorMessage);
            m_visionMain->iPIS_Send_ErrorMessageForPopup(strErrorMessage);
        }
        nReceivedPackageID = nPackageID;
        return false;
    }
    nReceivedPackageID = -1;

    o_strPackageID = vecPackageGrabInfo[nPackageID].m_strPackageID;

    o_nPackageID = nPackageID;

    return TRUE;
}

BOOL FOVImageGrabberNGRV::GetGrabPointInfo(long i_nPackageID, long& o_nGrabPointID, long& o_nStitchID, long& o_nStitchX,
    long& o_nStitchY, std::vector<long>& o_vecnFrameID, BOOL& o_bUseIR, BOOL& o_bIsFinalGrab) //kircheis_NGRV
{
    o_nGrabPointID = -1;
    o_nStitchID = -1;
    o_nStitchX = 1;
    o_nStitchY = 1;
    o_vecnFrameID.clear();
    o_vecnFrameID.push_back(0);
    o_bUseIR = FALSE;
    auto& syncInstance = SyncController::GetInstance();

    auto vecPackageGrabInfo = m_visionMain->m_vecPackageGrabInfo;

    Ipvm::TimeCheck timeCheckWaitReadGrabID;
    BOOL bReadGrabIDOn = FALSE;
    while (timeCheckWaitReadGrabID.Elapsed_ms() < 5000) // Grab Id를 읽으라는 신호를 약 5초 동안 기다린다
    {
        bReadGrabIDOn = SyncController::GetInstance().GetSignalNGRV_GrabID(); ////kircheis_NGRV_Sync
        if (bReadGrabIDOn)
        {
            SyncController::GetInstance().SetSignalNGRV_VisionRecvGrabID(bReadGrabIDOn);
            break;
        }

        Sleep(1);
    }
    if (bReadGrabIDOn == false)
    {
        SyncController::GetInstance().SetSignalNGRV_VisionRecvGrabID(bReadGrabIDOn);
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Not recieved [Grab ID On]"));
        m_visionMain->iPIS_Send_ErrorMessageForPopup(_T("Not recieved [Grab ID On]"));
        DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV_Grab - Didn't recieved GrabPointID"));
        return false;
    }

    SyncController::GetInstance().SetSignalNGRV_VisionRecvGrabID(FALSE);

    long nGrabID = (long)syncInstance.GetFovIndex();
    DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV_Grab - GrabPointID:%03d"), nGrabID);
    long nGrabPointNum = m_visionMain->m_isNGRVSingleRun == TRUE
        ? (long)m_visionMain->GetNGRVSingleRunInfo(m_visionMain->m_isNGRVSingleRunVisionType)
              .m_vecOnePointGrabInfo.size()
        : (long)vecPackageGrabInfo[i_nPackageID].m_vecGrabPointInfo.size();

    if (nGrabPointNum <= nGrabID)
    {
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Vision recieved the invalid grab ID"));
        m_visionMain->iPIS_Send_ErrorMessageForPopup(_T("Vision recieved the invalid grab ID"));
        DevelopmentLog::AddLog(
            DevelopmentLog::Type::Develop, _T("NGRV_Grab - Recieved wrong GrabPointID:%03d"), nGrabID);
        return false;
    }

    // Normal NGRV Run일 때와, SIngleRun 일때의 Inspection Name 설정이 다름
    BOOL bIsValid = FALSE;
    CString strCurInspModuleName = _T("");

    if (m_visionMain->m_isNGRVSingleRun == TRUE)
    {
        auto& currentNGRVSingleRunInfo = m_visionMain->GetNGRVSingleRunInfo(m_visionMain->m_isNGRVSingleRunVisionType);
        strCurInspModuleName = currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabID].m_defectPositionName;
        bIsValid = m_visionMain->GetFrameInfoNGRV_SingleRun(strCurInspModuleName, o_bUseIR, o_vecnFrameID);

        o_nStitchX = currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabID].m_stitchX;
        o_nStitchY = currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabID].m_stitchY;
    }
    else
    {
        auto currentGrabPointInfo = vecPackageGrabInfo[i_nPackageID].m_vecGrabPointInfo[nGrabID];
        strCurInspModuleName = currentGrabPointInfo.m_strInspModuleName;
        bIsValid = m_visionMain->GetFrameInfoNGRV(strCurInspModuleName, o_bUseIR, o_vecnFrameID);

        o_nStitchX = currentGrabPointInfo.m_nStitchingCountX;
        o_nStitchY = currentGrabPointInfo.m_nStitchingCountY;
    }

    if (bIsValid == FALSE)
    {
        CString strModuleName;
        strModuleName.Format(_T("Can't find InspModuleName, Input : %s"), (LPCTSTR)strCurInspModuleName);
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), strModuleName);
        m_visionMain->iPIS_Send_ErrorMessageForPopup(strModuleName);

        o_bUseIR = FALSE;
        o_vecnFrameID.clear();
        o_vecnFrameID.push_back(0);
        return FALSE;
    }
    o_nGrabPointID = nGrabID;

    long nStitchID = (long)syncInstance.GetIndexStitchIDNGRV();
    long nMaxStitchID = o_nStitchX * o_nStitchY - 1;
    o_nStitchID = (long)min(nStitchID, nMaxStitchID);

    o_bIsFinalGrab = (nMaxStitchID == o_nStitchID);

    return TRUE;
}

BOOL FOVImageGrabberNGRV::GetGrabPointInfo(long i_nPackageID, long& o_nGrabPointID, long& o_nStitchID, long& o_nStitchX,
    long& o_nStitchY, std::vector<long>& o_vecnFrameID, std::vector<long>& o_vecnUVIRFrameID, BOOL& o_bUseIR,
    BOOL& o_bUseUV, BOOL& o_bIsFinalGrab) //kircheis_NGRVGain
{
    o_nGrabPointID = -1;
    o_nStitchID = -1;
    o_nStitchX = 1;
    o_nStitchY = 1;
    o_vecnFrameID.clear();
    o_vecnFrameID.push_back(0);
    o_vecnUVIRFrameID.clear();
    o_bUseIR = FALSE;
    o_bUseUV = FALSE;

    auto& syncInstance = SyncController::GetInstance();

    auto vecPackageGrabInfo = m_visionMain->m_vecPackageGrabInfo;

    Ipvm::TimeCheck timeCheckWaitReadGrabID;
    BOOL bReadGrabIDOn = FALSE;
    while (timeCheckWaitReadGrabID.Elapsed_ms() < 5000) // Grab Id를 읽으라는 신호를 약 5초 동안 기다린다
    {
        bReadGrabIDOn = SyncController::GetInstance().GetSignalNGRV_GrabID(); ////kircheis_NGRV_Sync
        if (bReadGrabIDOn)
        {
            SyncController::GetInstance().SetSignalNGRV_VisionRecvGrabID(bReadGrabIDOn);
            break;
        }

        Sleep(1);
    }
    if (bReadGrabIDOn == false)
    {
        SyncController::GetInstance().SetSignalNGRV_VisionRecvGrabID(bReadGrabIDOn);
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Not recieved [Grab ID On]"));
        m_visionMain->iPIS_Send_ErrorMessageForPopup(_T("Not recieved [Grab ID On]"));
        DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV_Grab - Didn't recieved GrabPointID"));
        return false;
    }

    SyncController::GetInstance().SetSignalNGRV_VisionRecvGrabID(FALSE);

    long nGrabID = (long)syncInstance.GetFovIndex();
    DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV_Grab - GrabPointID:%03d"), nGrabID);
    long nGrabPointNum = static_cast<long>(m_visionMain->m_isNGRVSingleRun
            ? m_visionMain->GetNGRVSingleRunInfo(m_visionMain->m_isNGRVSingleRunVisionType).m_vecOnePointGrabInfo.size()
            : (long)vecPackageGrabInfo[i_nPackageID].m_vecGrabPointInfo.size());

    if (nGrabPointNum <= nGrabID)
    {
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Vision recieved the invalid grab ID"));
        m_visionMain->iPIS_Send_ErrorMessageForPopup(_T("Vision recieved the invalid grab ID"));
        DevelopmentLog::AddLog(
            DevelopmentLog::Type::Develop, _T("NGRV_Grab - Recieved wrong GrabPointID:%03d"), nGrabID);
        return false;
    }

    // Grab Point Info는 공유하지만 Inspection ModuleName/Defect Name은 Normal Run과 Single Run이 다르다... MED#6
    BOOL bIsValid = FALSE;
    CString strCurInspModuleName;

    //NGRV Normal Run
    if (m_visionMain->m_isNGRVSingleRun == FALSE)
    {
        auto currentGrabPointInfo = vecPackageGrabInfo[i_nPackageID].m_vecGrabPointInfo[nGrabID];
        strCurInspModuleName = currentGrabPointInfo.m_strInspModuleName;
        bIsValid = m_visionMain->GetFrameInfoNGRV(
            strCurInspModuleName, o_bUseIR, o_bUseUV, o_vecnFrameID, o_vecnUVIRFrameID);

        o_nStitchX = currentGrabPointInfo.m_nStitchingCountX;
        o_nStitchY = currentGrabPointInfo.m_nStitchingCountY;
    }
    else //NGRV SingleRun, PackageID가 한 PKG에 설정된 ROI 개수로 표현이 되야 맞을 것 같은데...?
    {
        auto& currentNGRVSingleRunInfo = m_visionMain->GetNGRVSingleRunInfo(m_visionMain->m_isNGRVSingleRunVisionType);
        strCurInspModuleName = currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabID].m_defectPositionName;
        bIsValid = m_visionMain->GetFrameInfoNGRV_SingleRun(
            strCurInspModuleName, o_bUseIR, o_bUseUV, o_vecnFrameID, o_vecnUVIRFrameID);

        o_nStitchX = currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabID].m_stitchX;
        o_nStitchY = currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabID].m_stitchY;
    }

    if (bIsValid == FALSE)
    {
        CString strModuleName;
        strModuleName.Format(_T("Can't find InspModuleName, Input : %s"), (LPCTSTR)strCurInspModuleName);
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), strModuleName);
        m_visionMain->iPIS_Send_ErrorMessageForPopup(strModuleName);

        o_bUseIR = FALSE;
        o_bUseUV = FALSE;
        o_vecnFrameID.clear();
        o_vecnFrameID.push_back(0);
        o_vecnUVIRFrameID.clear();

        return FALSE;
    }
    o_nGrabPointID = nGrabID;

    long nStitchID = (long)syncInstance.GetIndexStitchIDNGRV();
    long nMaxStitchID = o_nStitchX * o_nStitchY - 1;
    o_nStitchID = (long)min(nStitchID, nMaxStitchID);

    o_bIsFinalGrab = (nMaxStitchID == o_nStitchID);

    return TRUE;
}

void FOVImageGrabberNGRV::SetVisionAcquisitionNGRV(BOOL bActive) //kircheis_NGRV
{
    SyncController::GetInstance().SetGrabAcquisition(bActive);
    SyncController::GetInstance().SetGrabExposure(bActive);
}

bool FOVImageGrabberNGRV::buffer_grab(
    long bufferIndex, const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
    // NGRV SingleRun일 때는 여기로 바로 탄다... MED#6_JHB
    if (m_visionMain->m_isNGRVSingleRun == TRUE)
    {
        return buffer_grab_SingleRun(bufferIndex, fovID, i_eSideVisionModule);
    }

    BOOL bIsCameraGrabRetry = SystemConfig::GetInstance().m_bUseGrabRetry; //kircheis_3DCalcRetry

    //{{//kircheis_NGRV // Package ID를 다시 읽자. 여기까지 제대로 전달이 안되는듯 하다
    long nPackageID;
    CString strPackageID;
    if (GetPackageID(nPackageID, strPackageID) == FALSE)
        return false;
    //}}

    SyncController::GetInstance().SetReadySignal(FALSE); //kircheis_NGRV_Sync

    //{{//kircheis_NGRV // Grab Id를 확인 한 후 IR 조명이 필요한 상황인지 확인
    std::vector<long> vecnFrameID(0);
    std::vector<long> vecnIRUVFrameID(0);
    std::vector<long> vecnSaveImageFrameID; // 총 할당 해야할 Save Image Buffer 개수를 받아오는 벡터 - JHB_NGRV
    BOOL bUseIR = FALSE;
    BOOL bUseUV = FALSE;
    BOOL bIsFinalGrab = FALSE;
    long nGrabPointID = -1;
    long nStitchID = -1;
    long nStitchX = 1;
    long nStitchY = 1;

    vecnSaveImageFrameID.clear(); // 할당 Buffer 초기화

    BOOL bIsValid = GetGrabPointInfo(nPackageID, nGrabPointID, nStitchID, nStitchX, nStitchY, vecnFrameID,
        vecnIRUVFrameID, bUseIR, bUseUV, bIsFinalGrab);
    long nReverseFrameID = -1;
    long nNormalFrameNum = (long)vecnFrameID.size();
    for (long i = 0; i < nNormalFrameNum; i++)
    {
        if (vecnFrameID[i] == NGRV_FrameID_Reverse)
        {
            nReverseFrameID = i;
            break;
        }
    }

    //{{ Normal Frame 및 IR/UV Frame이 Empty가 아닐 때, 저장할 총 Image의 Frame 수를 PushBack해서 받아온다 - JHB_NGRV
    if (!vecnFrameID.empty())
    {
        for (long nNormalFrameSize = 0; nNormalFrameSize < nNormalFrameNum; nNormalFrameSize++)
        {
            vecnSaveImageFrameID.push_back(vecnFrameID[nNormalFrameSize]);
        }
    }
    if (!vecnIRUVFrameID.empty())
    {
        for (long nIRUVFrameSize = 0; nIRUVFrameSize < (long)vecnIRUVFrameID.size(); nIRUVFrameSize++)
        {
            vecnSaveImageFrameID.push_back(vecnFrameID[nIRUVFrameSize]);
        }
    }
    //}}

    if (bIsValid == FALSE || nGrabPointID < 0)
    {
        CString strGrabID;
        strGrabID.Format(_T("Wrong GrabID %03d"), nGrabPointID);

        SyncController::GetInstance().SetReadySignal(true); //kircheis_NGRV_Sync

        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), strGrabID);
        m_visionMain->iPIS_Send_ErrorMessageForPopup(strGrabID);

        return FALSE;
    }
    SetVisionAcquisitionNGRV(TRUE); //kircheis_NGRV_Sync

    const int& idx_grabber = 0;
    const int& idx_camera = 0;
    long nCameraImageSizeX
        = FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera); // 카메라의 이미지 X 크기
    long nCameraImageSizeY
        = FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera); // 카메라의 이미지 Y 크기

    if (nStitchID == 0)
    {
        // 영상 버퍼 크기 다시 설정
        long nFullImageSizeX = nCameraImageSizeX * nStitchX;
        long nFullImageSizeY = nCameraImageSizeY * nStitchY;

        auto vecPackageGrabInfo = m_visionMain->m_vecPackageGrabInfo;

        long nTrayID = vecPackageGrabInfo[nPackageID].m_nTrayID;

        //if (m_visionMain->m_isNGRVSingleRun == TRUE) //기존 Logic과 같은 Seq.로 동작하고, Set정보만 다르면되지
        //{
        //    auto currentNGRVSingleRunInfo
        //        = m_visionMain->GetNGRVSingleRunInfo(m_visionMain->m_isNGRVSingleRunVisionType);

        //    m_visionMain->m_ngrvImageSetBuffer.SetInfo(nTrayID, nPackageID, strPackageID, nGrabPointID,
        //        currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabPointID].m_defectPositionName,
        //        m_visionMain->m_isNGRVSingleRunVisionType == VISION_BTM_2D ? _T("BTM") : _T("TOP"),
        //        vecnSaveImageFrameID, nStitchX, nStitchY, nFullImageSizeX, nFullImageSizeY);
        //}
        //else
        //{
        auto currentGrabPointInfo = vecPackageGrabInfo[nPackageID].m_vecGrabPointInfo[nGrabPointID];

        m_visionMain->m_ngrvImageSetBuffer.SetInfo(nTrayID, nPackageID, strPackageID, nGrabPointID,
            currentGrabPointInfo.m_strInspModuleName, currentGrabPointInfo.m_strReviewSide, vecnSaveImageFrameID,
            nStitchX, nStitchY, nFullImageSizeX, nFullImageSizeY);
        //}
    }

    m_visionMain->HardwareSetupForNGRV(vecnFrameID, FALSE, TRUE);
    //}}

    SyncController::GetInstance().SetSignalNGRV_VisionMoveIRPos(bUseIR);

    //{{ //kircheis_MultiGrab
    short normalFrameCount = (short)vecnFrameID.size(); // -(bUseIR ? 1 : 0);
    short uvirFrameCount = (short)vecnIRUVFrameID.size();
    short totalFrameCount = (short)(vecnFrameID.size() + uvirFrameCount);

    short highPosGrabCount = bUseIR ? 1 : 0;
    short normalGrabCount = totalFrameCount - (bUseUV ? uvirFrameCount : highPosGrabCount);

    auto& bufferList = m_buffers[bufferIndex]->m_imagePtrs;
    bool success = true;

    if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[0], normalGrabCount, nullptr))
    {
        SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
        {
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

            if (bIsCameraGrabRetry)
            {
                if (FrameGrabber::GetInstance().StartGrab2D(
                        idx_grabber, idx_camera, &bufferList[0], normalGrabCount, nullptr))
                {
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                    if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
                    {
                        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));
                        success = false;
                    }
                    else
                    {
                        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                    }
                }
                else
                {
                    m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Re-StartGrabNGRV Fail"));
                    success = false;
                }
            }
        }
    }
    else
    {
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrabNGRV Fail"));
        success = false;
    }

    //여기에 UV 관련 조명 Setting이 필요하다
    std::vector<long> vecnUVFrameID(0);
    std::vector<long> vecnIRFrameID(0);
    if (bUseUV)
    {
        vecnUVFrameID.push_back(vecnIRUVFrameID[0]);
        //Gain Up for UV
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fUV_Camera_Gain, TRUE);

        m_visionMain->HardwareSetupForNGRV(vecnUVFrameID, FALSE, TRUE);
        if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[normalFrameCount], 1, nullptr))
        {
            SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
            {
                m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

                if (bIsCameraGrabRetry)
                {
                    if (FrameGrabber::GetInstance().StartGrab2D(
                            idx_grabber, idx_camera, &bufferList[normalFrameCount], 1, nullptr))
                    {
                        SyncController::GetInstance().StartSyncBoard(
                            FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                        if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
                        {
                            m_visionMain->iPIS_Send_ErrorMessageForLog(
                                _T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));
                            success = false;
                        }
                        else
                        {
                            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                        }
                    }
                    else
                    {
                        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Re-StartGrabNGRV Fail"));
                        success = false;
                    }
                }
            }
        }
        else
        {
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrabNGRV Fail"));
            success = false;
        }
        //Gain Down for UV
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, TRUE);
    }

    // 영상 획득이 빨라서 시퀀스에서 놓치는 경우가 있으므로, Start signal 이 떨어지기를 기다린 후 Acquisition 을 떨어뜨린다.
    {
        Ipvm::TimeCheck timeCheck;
        while (timeCheck.Elapsed_ms() < 5000)
        {
            if (SyncController::GetInstance().GetSignalNGRV_GrabID() == FALSE
                && SyncController::GetInstance().GetStartSignal() == FALSE)
                break;
        }
    }

    SetVisionAcquisitionNGRV(FALSE); //kircheis_NGRV_Sync
    SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    if (bUseIR == false)
    {
        SetNGRVSaveImageBuffer(&bufferList[0], totalFrameCount, nCameraImageSizeX, nCameraImageSizeY, nStitchID,
            bIsFinalGrab, nReverseFrameID); // NGRV Image Buffer를 채운다 - JHB_NGRV

        return success;
    }

    //Gain Up for IR Channel
    FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fIR_Camera_Gain, TRUE);

    vecnIRFrameID.push_back(vecnIRUVFrameID[bUseUV ? 1 : 0]);
    m_visionMain->HardwareSetupForNGRV(vecnIRFrameID, FALSE, TRUE);

    //여기에 High Bit를 기다리는 Code 필요
    Ipvm::TimeCheck timeCheckWaitHighBit;
    BOOL bHighPosBitOn = FALSE;
    if (SystemConfig::GetInstance().m_bIsByPassIRBIT == FALSE)
    {
        while (timeCheckWaitHighBit.Elapsed_ms() < 5000)
        {
            bHighPosBitOn
                = SyncController::GetInstance().GetSignalNGRV_StartIR(); //GetGrabHighPosGrabBit();//kircheis_NGRV_Sync
            if (bHighPosBitOn)
                break;
        }
    }
    else
        bHighPosBitOn = TRUE;

    SyncController::GetInstance().SetSignalNGRV_VisionMoveIRPos(FALSE);
    // 싱크에 IR  조명 값을 입력해 줘야 함

    if (bHighPosBitOn)
    {
        SetVisionAcquisitionNGRV(TRUE); //kircheis_NGRV_Sync

        if (FrameGrabber::GetInstance().StartGrab2D(
                idx_grabber, idx_camera, &bufferList[normalFrameCount + (bUseUV ? 1 : 0)], 1, nullptr))
        {
            SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
            {
                m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

                if (FrameGrabber::GetInstance().StartGrab2D(
                        idx_grabber, idx_camera, &bufferList[normalGrabCount + (bUseUV ? 1 : 0)], 1, nullptr))
                {
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                    if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
                    {
                        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));
                        success = false;
                    }
                    else
                    {
                        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                    }
                }
                else
                {
                    m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-StartGrabNGRV Fail"));
                    success = false;
                }
            }
        }
        else
        {
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrabNGRV Fail"));
            success = false;
        }
        //Gain Down for IR Channel
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, TRUE);

        Ipvm::TimeCheck timeCheck;
        while (timeCheck.Elapsed_ms() < 5000) //제대로 꺼지는 지 확인
        {
            if (!SyncController::GetInstance().GetSignalNGRV_StartIR()) //GetGrabHighPosGrabBit())//kircheis_NGRV_Sync
                break;
        }
        SetVisionAcquisitionNGRV(FALSE); //kircheis_NGRV_Sync
        SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }
    else
    {
        memset(bufferList[normalFrameCount + (bUseUV ? 1 : 0)], 0, nCameraImageSizeX * nCameraImageSizeY);
        //Gain Down for IR Channel
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, TRUE);
    }

    SetNGRVSaveImageBuffer(&bufferList[0], totalFrameCount, nCameraImageSizeX, nCameraImageSizeY, nStitchID,
        bIsFinalGrab, nReverseFrameID); // NGRV Image Buffer를 채운다 - JHB_NGRV

    return success;
}

bool FOVImageGrabberNGRV::buffer_grab_SingleRun(
    long bufferIndex, const FOVID& fovID, const enSideVisionModule i_eSideVisionModule)
{
    UNREFERENCED_PARAMETER(fovID);
    UNREFERENCED_PARAMETER(i_eSideVisionModule);

    BOOL bIsCameraGrabRetry = SystemConfig::GetInstance().m_bUseGrabRetry; //kircheis_3DCalcRetry

    //{{//kircheis_NGRV // Package ID를 다시 읽자. 여기까지 제대로 전달이 안되는듯 하다
    long nPackageID;
    CString strPackageID;
    if (GetPackageID(nPackageID, strPackageID) == FALSE)
        return false;
    //}}

    SyncController::GetInstance().SetReadySignal(FALSE); //kircheis_NGRV_Sync

    //{{//kircheis_NGRV // Grab Id를 확인 한 후 IR 조명이 필요한 상황인지 확인
    std::vector<long> vecnFrameID(0);
    std::vector<long> vecnIRUVFrameID(0);
    std::vector<long> vecnSaveImageFrameID; // 총 할당 해야할 Save Image Buffer 개수를 받아오는 벡터 - JHB_NGRV
    BOOL bUseIR = FALSE;
    BOOL bUseUV = FALSE;
    BOOL bIsFinalGrab = FALSE;
    long nGrabPointID = -1;
    long nStitchID = -1;
    long nStitchX = 1;
    long nStitchY = 1;

    vecnSaveImageFrameID.clear(); // 할당 Buffer 초기화

    BOOL bIsValid = GetGrabPointInfo(nPackageID, nGrabPointID, nStitchID, nStitchX, nStitchY, vecnFrameID,
        vecnIRUVFrameID, bUseIR, bUseUV, bIsFinalGrab);
    long nReverseFrameID = -1;
    long nNormalFrameNum = (long)vecnFrameID.size();
    for (long i = 0; i < nNormalFrameNum; i++)
    {
        if (vecnFrameID[i] == NGRV_FrameID_Reverse)
        {
            nReverseFrameID = i;
            break;
        }
    }

    //{{ Normal Frame 및 IR/UV Frame이 Empty가 아닐 때, 저장할 총 Image의 Frame 수를 PushBack해서 받아온다 - JHB_NGRV
    if (!vecnFrameID.empty())
    {
        for (long nNormalFrameSize = 0; nNormalFrameSize < nNormalFrameNum; nNormalFrameSize++)
        {
            vecnSaveImageFrameID.push_back(vecnFrameID[nNormalFrameSize]);
        }
    }

    if (!vecnIRUVFrameID.empty())
    {
        for (long nIRUVFrameSize = 0; nIRUVFrameSize < (long)vecnIRUVFrameID.size(); nIRUVFrameSize++)
        {
            //vecnSaveImageFrameID.push_back(vecnFrameID[nIRUVFrameSize]);
            vecnSaveImageFrameID.push_back(vecnIRUVFrameID[nIRUVFrameSize]);
        }
    }
    //}}

    if (bIsValid == FALSE || nGrabPointID < 0)
    {
        CString strGrabID;
        strGrabID.Format(_T("Wrong GrabID %03d"), nGrabPointID);

        SyncController::GetInstance().SetReadySignal(true); //kircheis_NGRV_Sync

        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), strGrabID);
        m_visionMain->iPIS_Send_ErrorMessageForPopup(strGrabID);

        return FALSE;
    }
    SetVisionAcquisitionNGRV(TRUE); //kircheis_NGRV_Sync

    const int& idx_grabber = 0;
    const int& idx_camera = 0;

    long nCameraImageSizeX
        = FrameGrabber::GetInstance().get_grab_image_width(idx_grabber, idx_camera); // 카메라의 이미지 X 크기
    long nCameraImageSizeY
        = FrameGrabber::GetInstance().get_grab_image_height(idx_grabber, idx_camera); // 카메라의 이미지 Y 크기

    if (nStitchID == 0)
    {
        // 영상 버퍼 크기 다시 설정
        long nFullImageSizeX = nCameraImageSizeX * nStitchX;
        long nFullImageSizeY = nCameraImageSizeY * nStitchY;

        long nTrayID;

        auto vecPackageGrabInfo = m_visionMain->m_vecPackageGrabInfo;

        nTrayID = vecPackageGrabInfo[nPackageID].m_nTrayID;

        auto currentNGRVSingleRunInfo = m_visionMain->GetNGRVSingleRunInfo(m_visionMain->m_isNGRVSingleRunVisionType);

        m_visionMain->m_ngrvImageSetBuffer.SetInfo(nTrayID, nPackageID, strPackageID, nGrabPointID,
            currentNGRVSingleRunInfo.m_vecOnePointGrabInfo[nGrabPointID].m_defectPositionName,
            m_visionMain->m_isNGRVSingleRunVisionType == VISION_BTM_2D ? _T("BTM") : _T("TOP"), vecnSaveImageFrameID,
            nStitchX, nStitchY, nFullImageSizeX, nFullImageSizeY);
    }
    //}}
    SyncController::GetInstance().SetSignalNGRV_VisionMoveIRPos(bUseIR);

    //{{ //kircheis_MultiGrab
    short uvirFrameCount = (short)vecnIRUVFrameID.size();
    short normalFrameCount = (short)(NGRV_FrameID_Reverse + 1); //8장 고정, 뒤에는 UV, IR

    short totalFrameCount = (short)(normalFrameCount + uvirFrameCount);
    short normalGrabCount = totalFrameCount - uvirFrameCount;

    auto& bufferList = m_buffers[bufferIndex]->m_imagePtrs;
    bool success = true;

    std::vector<long> vecNormalFrameID;
    for (long nidx = 0; nidx < normalGrabCount; nidx++)
        vecNormalFrameID.push_back(nidx);

    m_visionMain->HardwareSetupForNGRV(vecNormalFrameID, FALSE, TRUE);

    if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[0], normalGrabCount, nullptr))
    {
        SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
        {
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

            if (bIsCameraGrabRetry)
            {
                success = grab_retry(SINGLE_RUN_GRAB_RETRY_COUNT, normalGrabCount, &bufferList[0]);

                //if (FrameGrabber::GetInstance().StartGrabNGRV(&bufferList[0], normalGrabCount, nullptr))
                //{
                //    SyncController::GetInstance().StartSyncBoard();
                //    if (!FrameGrabber::GetInstance().WaitGrabEnd(nullptr))
                //    {
                //        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));
                //        success = false;
                //    }
                //    else
                //    {
                //        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                //    }
                //}
                //else
                //{
                //    m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Re-StartGrabNGRV Fail"));
                //    success = false;
                //}
            }
        }
    }
    else
    {
        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrabNGRV Fail"));
        success = false;
    }

    //여기에 UV 관련 조명 Setting이 필요하다
    std::vector<long> vecnUVFrameID(0);
    std::vector<long> vecnIRFrameID(0);

    //UV
    if (bUseUV)
    {
        vecnUVFrameID.push_back(vecnIRUVFrameID[0]);
        //Gain Up for UV
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fUV_Camera_Gain, TRUE);

        m_visionMain->HardwareSetupForNGRV(vecnUVFrameID, FALSE, TRUE);
        if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[NGRV_FrameID_UV], 1, nullptr))
        {
            SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
            {
                m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

                if (bIsCameraGrabRetry)
                {
                    success = grab_retry(SINGLE_RUN_GRAB_RETRY_COUNT, 1, &bufferList[NGRV_FrameID_UV]);

                    //if (FrameGrabber::GetInstance().StartGrabNGRV(&bufferList[NGRV_FrameID_UV], 1, nullptr))
                    //{
                    //    SyncController::GetInstance().StartSyncBoard();
                    //    if (!FrameGrabber::GetInstance().WaitGrabEnd(nullptr))
                    //    {
                    //        m_visionMain->iPIS_Send_ErrorMessageForLog(
                    //            _T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));
                    //        success = false;
                    //    }
                    //    else
                    //    {
                    //        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                    //    }
                    //}
                    //else
                    //{
                    //    m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Re-StartGrabNGRV Fail"));
                    //    success = false;
                    //}
                }
            }
        }
        else
        {
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrabNGRV Fail"));
            success = false;
        }
        //Gain Down for UV
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, TRUE);
    }

    // 영상 획득이 빨라서 시퀀스에서 놓치는 경우가 있으므로, Start signal 이 떨어지기를 기다린 후 Acquisition 을 떨어뜨린다.
    {
        Ipvm::TimeCheck timeCheck;
        while (timeCheck.Elapsed_ms() < 5000)
        {
            if (SyncController::GetInstance().GetSignalNGRV_GrabID() == FALSE
                && SyncController::GetInstance().GetStartSignal() == FALSE)
                break;
        }
    }

    SetVisionAcquisitionNGRV(FALSE); //kircheis_NGRV_Sync
    SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    if (bUseIR == false)
    {
        SetNGRVSaveImageBuffer(&bufferList[0], totalFrameCount, nCameraImageSizeX, nCameraImageSizeY, nStitchID,
            bIsFinalGrab, nReverseFrameID); // NGRV Image Buffer를 채운다 - JHB_NGRV

        return success;
    }

    //Gain Up for IR Channel
    FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fIR_Camera_Gain, TRUE);

    vecnIRFrameID.push_back(vecnIRUVFrameID[bUseUV ? 1 : 0]);
    m_visionMain->HardwareSetupForNGRV(vecnIRFrameID, FALSE, TRUE);

    //여기에 High Bit를 기다리는 Code 필요
    Ipvm::TimeCheck timeCheckWaitHighBit;
    BOOL bHighPosBitOn = FALSE;
    if (SystemConfig::GetInstance().m_bIsByPassIRBIT == FALSE)
    {
        while (timeCheckWaitHighBit.Elapsed_ms() < 5000)
        {
            bHighPosBitOn
                = SyncController::GetInstance().GetSignalNGRV_StartIR(); //GetGrabHighPosGrabBit();//kircheis_NGRV_Sync
            if (bHighPosBitOn)
                break;
        }
    }
    else
        bHighPosBitOn = TRUE;

    SyncController::GetInstance().SetSignalNGRV_VisionMoveIRPos(FALSE);
    // 싱크에 IR  조명 값을 입력해 줘야 함

    if (bHighPosBitOn)
    {
        SetVisionAcquisitionNGRV(TRUE); //kircheis_NGRV_Sync

        if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &bufferList[NGRV_FrameID_IR], 1, nullptr))
        {
            SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
            {
                m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("Wait Grab End Fail"));

                if (FrameGrabber::GetInstance().StartGrab2D(
                        idx_grabber, idx_camera, &bufferList[normalGrabCount + (bUseUV ? 1 : 0)], 1, nullptr))
                {
                    SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
                    if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
                    {
                        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-Wait Grab End Fail"));
                        success = false;
                    }
                    else
                    {
                        m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab Success"));
                    }
                }
                else
                {
                    m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("ReGrab-StartGrabNGRV Fail"));
                    success = false;
                }
            }
        }
        else
        {
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), _T("StartGrabNGRV Fail"));
            success = false;
        }

        //Gain Down for IR Channel
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, TRUE);

        Ipvm::TimeCheck timeCheck;
        while (timeCheck.Elapsed_ms() < 5000) //제대로 꺼지는 지 확인
        {
            if (!SyncController::GetInstance().GetSignalNGRV_StartIR()) //GetGrabHighPosGrabBit())//kircheis_NGRV_Sync
                break;
        }
        SetVisionAcquisitionNGRV(FALSE); //kircheis_NGRV_Sync
        SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }
    else
    {
        memset(bufferList[normalFrameCount + (bUseUV ? 1 : 0)], 0, nCameraImageSizeX * nCameraImageSizeY);
        //Gain Down for IR Channel
        FrameGrabber::GetInstance().SetDigitalGainValue(SystemConfig::GetInstance().m_fDefault_Camera_Gain, TRUE);
    }

    SetNGRVSaveImageBuffer(&bufferList[0], totalFrameCount, nCameraImageSizeX, nCameraImageSizeY, nStitchID,
        bIsFinalGrab, nReverseFrameID); // NGRV Image Buffer를 채운다 - JHB_NGRV

    return success;
}

void FOVImageGrabberNGRV::buffer_paste(long bufferIndex, long stitchIndex, long stitchCountX, long stitchCountY,
    std::shared_ptr<VisionImageLot> inspectionImage)
{
    auto& bufferList = m_buffers[bufferIndex]->m_imagePtrs;

    // 스티칭 영상을 붙여넣자.
    inspectionImage->PasteNGRV(stitchIndex, &bufferList[0], m_totalFrameCount, stitchCountX, stitchCountY, -1,
        enSideVisionModule::SIDE_VISIONMODULE_FRONT);
}

void FOVImageGrabberNGRV::ClearImageGrabber_GrabTime()
{
    return;
}

void FOVImageGrabberNGRV::SaveImageGrabber_GrabTime(const CString i_strSavePath)
{
    return;
}

void FOVImageGrabberNGRV::SetNGRVSaveImageBuffer(BYTE** i_pbyGrabbedImage, long i_nTotalFrameCount,
    long i_nCameraImageSizeX, long i_nCameraImageSizeY, long i_nStitchID, BOOL i_bFinalGrab, long i_nReverseFrameID)
{
    std::vector<Ipvm::Image8u3> vecImageConversion;
    vecImageConversion.resize(i_nTotalFrameCount);

    auto system = SystemConfig::GetInstance();

    static float fGainRed;
    static float fGainGreen;
    static float fGainBlue;

    CNGRVImageSet ngrvBuffer;
    ngrvBuffer.Init();

    auto ngrvImageSetBuffer = m_visionMain->m_ngrvImageSetBuffer;

    for (long nFrame = 0; nFrame < i_nTotalFrameCount; nFrame++)
    {
        vecImageConversion[nFrame].Create(i_nCameraImageSizeX, i_nCameraImageSizeY);
        vecImageConversion[nFrame].FillZero();

        Ipvm::Image8u image(i_nCameraImageSizeX, i_nCameraImageSizeY, i_pbyGrabbedImage[nFrame], i_nCameraImageSizeX);

        if (nFrame == i_nReverseFrameID)
        {
            fGainRed = system.m_fReverse_Channel_GainR;
            fGainGreen = system.m_fReverse_Channel_GainG;
            fGainBlue = system.m_fReverse_Channel_GainB;
        }
        else
        {
            fGainRed = system.m_fNormal_Channel_GainR;
            fGainGreen = system.m_fNormal_Channel_GainG;
            fGainBlue = system.m_fNormal_Channel_GainB;
        }

        if (Ipvm::ImageProcessingGpu::DemosaicBilinear(image, Ipvm::Rect32s(image), Ipvm::BayerPattern::e_gbrg,
                fGainRed, fGainGreen, fGainBlue, vecImageConversion[nFrame])
            != Ipvm::Status::e_ok)
        {
            CString strConversionFail;
            strConversionFail.Format(_T("NGRV_Grab - Color Conversion Fail_Frame%03d"), nFrame);

            Ipvm::ImageProcessing::Fill(
                Ipvm::Rect32s(vecImageConversion[nFrame]), Ipvm::Point8u3(128, 128, 128), vecImageConversion[nFrame]);

            DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, strConversionFail);
        }
    }

    if ((long)vecImageConversion.size() > 0)
    {
        m_visionMain->SetColorImageToInlineUIforNGRV(vecImageConversion[0]); // Show Image in InlineUI
    }

    CString strStitchID;
    strStitchID.Format(_T("NGRV_Grab - %s_%s_StitchID%03d_FinalGrab:%d"), (LPCTSTR)ngrvImageSetBuffer.m_strPackageID,
        (LPCTSTR)ngrvImageSetBuffer.m_strInspectionItemName, i_nStitchID, (long)i_bFinalGrab);

    DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, strStitchID);

    ngrvImageSetBuffer.SetImage(i_nStitchID, vecImageConversion, m_visionMain->m_isNGRVSingleRun);

    if (i_bFinalGrab)
    {
        long nImageSizeX = ngrvImageSetBuffer.m_vecImageForSave[0].GetSizeX(); //Copy할 이미지의 ImageSizeX
        long nImageSizeY = ngrvImageSetBuffer.m_vecImageForSave[0].GetSizeY(); //Copy할 이미지의 ImageSizeY

        ngrvBuffer.SetImageBufferSize(
            ngrvImageSetBuffer.m_vecnFrameID, nImageSizeX, nImageSizeY); // Copy할 Image Buffer 공간 할당

        ngrvBuffer.m_nTrayID = ngrvImageSetBuffer.m_nTrayID; // Tray ID
        ngrvBuffer.m_nPackageID = ngrvImageSetBuffer.m_nPackageID; // Package ID(Number)
        ngrvBuffer.m_nGrabPointID = ngrvImageSetBuffer.m_nGrabPointID; // Grab Point ID
        ngrvBuffer.m_strPackageID = ngrvImageSetBuffer.m_strPackageID; // Package ID(String)
        ngrvBuffer.m_strInspectionItemName = ngrvImageSetBuffer.m_strInspectionItemName; // Inspection Module Name
        ngrvBuffer.m_strReviewSideName = ngrvImageSetBuffer.m_strReviewSideName; // Defected Vision Name : BOTRV, TOPRV

        for (long nFrame = 0; nFrame < (long)ngrvImageSetBuffer.m_vecImageForSave.size(); nFrame++)
        {
            Ipvm::ImageProcessing::Copy(ngrvImageSetBuffer.m_vecImageForSave[nFrame],
                Ipvm::Rect32s(ngrvImageSetBuffer.m_vecImageForSave[nFrame]), ngrvBuffer.m_vecImageForSave[nFrame]);
        }
        ngrvImageSetBuffer.Init();

        m_visionMain->m_mutNGRV.Lock();
        m_visionMain->m_vecNGRVImageSet.push_back(ngrvBuffer);
        m_visionMain->m_mutNGRV.Unlock();

        long nImageBufCount = (long)m_visionMain->m_vecNGRVImageSet.size();
        if (nImageBufCount >= 80)
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV Save Buffer is FULL"));
            while (1)
            {
                SyncController::GetInstance().SetSignalNGRV_VisionSaveImageReady(TRUE);
                nImageBufCount = (long)m_visionMain->m_vecNGRVImageSet.size();
                if (nImageBufCount < 30)
                {
                    DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("NGRV Save Buffer Cleared"));
                    SyncController::GetInstance().SetSignalNGRV_VisionSaveImageReady(FALSE);
                    return;
                }
                Sleep(500);
            }
        }
        SyncController::GetInstance().SetSignalNGRV_VisionSaveImageReady(FALSE);
    }
}

bool FOVImageGrabberNGRV::grab_retry(const long& retry_count, const short& total_grab_frame, BYTE** buffer)
{
    const int& idx_grabber = 0;
    const int& idx_camera = 0;

    //입력받은 횟수만큼 grab retry를 진행한다
    for (long try_count = 0; try_count < retry_count; try_count++)
    {
        CString message("");
        if (FrameGrabber::GetInstance().StartGrab2D(idx_grabber, idx_camera, &buffer[0], total_grab_frame, nullptr))
        {
            SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            if (!FrameGrabber::GetInstance().wait_grab_end(idx_grabber, idx_camera, nullptr))
            {
                message.Format(_T("ReGrab[%d]-Wait Grab End Fail"), try_count);
                m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), message);
            }
            else
            {
                message.Format(_T("ReGrab[%d] Success"), try_count);
                m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), message);
                message.Empty();
                return true;
            }
        }
        else
        {
            message.Format(_T("Re-StartGrabNGRV[%d] Fail"), try_count);
            m_visionMain->iPIS_Send_ErrorMessageForLog(_T("Inline Grab"), message);
        }

        message.Empty();
    }

    return false;
}