//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "MainFrm.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../DefineModules/dA_Base/semiinfo.h"
#include "../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../HardwareModules/dPI_SyncController/SyncController.h" //kircheis_GrabFailDebug
#include "../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../MainUiModules/VisionPrimaryUI/VisionPrimaryUI.h"
#include "../ManagementModules/VisionUnit/VisionUnit.h"
#include "../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../SharedCommunicationModules/VisionHostCommon/BinaryFileData.h"
#include "../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../SharedCommunicationModules/VisionHostCommon/DebugOptionData.h"
#include "../SharedCommunicationModules/VisionHostCommon/ImageSaveOption.h"
#include "../SharedCommunicationModules/VisionHostCommon/ImageSaveOptionData.h"
#include "../SharedCommunicationModules/VisionHostCommon/VisionSystemParameters.h"
#include "../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"
#include "../UtilityMacroFunction.h"
#include "../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
static BOOL m_bIsUpdateRecipeDone = FALSE; // Recipe Update가 완료되었는지를 판단하는 Static 변수 생성 - 2022.10.13_JHB

LRESULT CMainFrame::OnConnectionUpdatedFromHost(WPARAM wParam, LPARAM lParam)
{
    const auto evtInfo = Ipvm::SocketMessaging::ParseEvt(wParam, lParam);
    m_visionMain.m_bGrabRetry = FALSE; //kk 접속을 다시 시도할 시 FALSE로 무조건 초기화한다.

    // 접속하거나 접속을 끊을 때 혹시 마지막에 켜져 있었을지도 모르니 분석결과 옵션은 일단 꺼주자
    SystemConfig::GetInstance().m_bIsAnalysisResult = false;

    if (evtInfo.m_connected)
    {
        m_visionMain.m_nSendImageSizeX = -1;
        m_visionMain.m_nSendImageSizeY = -1;

        // 영훈 20150702 : 1/8배로 압축해서 호스트로 보내도록 한다.
        long nResizeScale = SEND_IMAGE_RESCALE_FACTOR;

        // Reszie image Size 계산
        auto& visionUnit = m_visionMain.GetPrimaryVisionUnit();
        auto& imageLot = visionUnit.getImageLot();

        long nSendImageSizeX = imageLot.GetImageSizeX() / nResizeScale;
        long nSendImageSizeY = imageLot.GetImageSizeY() / nResizeScale;

        m_visionMain.iPIS_Send_ImageSize(nSendImageSizeX, nSendImageSizeY);

        m_visionMain.SendVersionInfo();

        m_visionMain.iPIS_Send_SystemParameters(); //kircheis_ASNC

        if (SystemConfig::GetInstance().Get_is_iGrabFirmware_and_LibraryVersion_Mismatch() == true
            && SystemConfig::GetInstance().IsHardwareExist() == TRUE) //Hardware 사용시 조건문 추가
            m_visionMain.iPIS_Send_VersionMismatchErrorPopup();
    }
    else if (evtInfo.m_disconnected)
    {
        m_visionMain.SetInlineStop(false);
    }

    m_visionMain.m_pVisionPrimaryUI->OnMessageSocketConnectionUpdated(evtInfo);

    return 0;
}

LRESULT CMainFrame::OnDataReceivedFromHost(WPARAM wParam, LPARAM lParam)
{
    const auto msgInfo = Ipvm::SocketMessaging::ParseMsg(wParam, lParam);

    const long nMessageID = msgInfo.m_messageIndex;
    const long nMessageLength = msgInfo.m_messageLength;
    const BYTE* pData = (const BYTE*)msgInfo.m_message;

    switch (nMessageID)
    {
        case MSG_IMAGE_SAVE_OPTION_DOWNLOAD:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_IMAGE_SAVE_OPTION_DOWNLOAD"));
            if (1)
            {
                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);
                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
                m_visionMain.m_ImageSaveOption->Serialize(ar);
                ar.Close();
            }
            break;

        case MSG_DEBUG_STOP_OPTION_DOWNLOAD:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_DEBUG_STOP_OPTION_DOWNLOAD"));
            if (1)
            {
                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);
                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
                m_visionMain.m_DebugStopOption->Serialize(ar);
                ar.Close();
            }
            break;

        case MSG_PROBE_CHECK_INLINE_MODE:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_CHECK_INLINE_MODE"));
            if (1)
            {
                DEFMSGPARAM turnData = {BOOL(m_visionMain.IsInlineMode()), 0, 0, 0, 0, 0, 0, 0};
                m_visionMain.m_pMessageSocket->Write(
                    MSG_PROBE_CHECK_INLINE_MODE, sizeof(DEFMSGPARAM), (BYTE*)&turnData);
            }
            break;

        case MSG_PROBE_TRAY_SCAN_SPEC:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_TRAY_SCAN_SPEC"));
            if (1)
            {
                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);

                VisionTrayScanSpec spec;

                try
                {
                    ar >> spec;
                    // Side Vision일 경우 아래의 함수에 진입 전, Side Vision에 맞게 TrayScanSpec 자체를 변경해줘야함
                    // spec.m_pocketNumX/Y X = X * Y, Y = 1

                    //spec.RebuildParaForSide(SystemConfig::GetInstance().GetSideVisionNumber());

                    spec.MakeInspectionInfo(); // Side Vision일 경우 다른 함수를 타야함 - SideStitch
                    DevelopmentLog::AddLog(
                        DevelopmentLog::Type::TCP, _T("Tray Scan Spec FovCount:%d"), (long)spec.m_vecFovInfo.size());

                    m_visionMain.SetTrayScanSpec(spec);
                }
                catch (CArchiveException* ae)
                {
                    ::AfxMessageBox(CString("Archive Exception : Vision Tray Scan Spec:") + ae->m_strFileName);
                    ae->Delete();
                }
                catch (CFileException* fe)
                {
                    ::AfxMessageBox(CString("File Exception : Vision Tray Scan Spec:") + fe->m_strFileName);
                    fe->Delete();
                }

                ar.Close();
            }
            break;

        case MSG_PROBE_TRAY_SCAN_INFO: // Inline 중에 진행되므로 여기 두지 않는다.
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_TRAY_SCAN_INFO"));
            if (1)
            {
                DEFMSGPARAM* pDatagram = (DEFMSGPARAM*)pData;

                m_visionMain.SetScanTrayIndex((int)(pDatagram->P1));
                m_visionMain.SetInspectionRepeatIndex((int)(pDatagram->P2));

                m_visionMain.m_pMessageSocket->Write(MSG_PROBE_TRAY_SCAN_INFO_ACK, 0, NULL);

                SystemConfig::GetInstance().Save_VisionLog_GrabSequenceTimeLog(
                    pDatagram->P1, -1, _T("Received::MSG_PROBE_TRAY_SCAN_INFO"));

                DevelopmentLog::AddLog(DevelopmentLog::Type::Develop, _T("MSG_PROBE_TRAY_SCAN_INFO:%d"), pDatagram->P1);
            }
            break;

        case MSG_PROBE_INLINE_START:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_INLINE_START"));

            if (!m_visionMain.IsInlineMode())
            {
                FrameGrabber::GetInstance().live_off();
                SyncController::GetInstance().SetGrabAcquisition(FALSE); //kircheis_GrabFailDebug
                SyncController::GetInstance().SetGrabExposure(FALSE); //kircheis_GrabFailDebug

                // 영훈 20140203 : Detail Setup Mode나 Batch inps Mode등 무언가가 열려있을 경우 닫고 시작하도록 한다.
                if (m_visionMain.IsShowVisionPrimaryUI() == false)
                {
                    OnInspectionQuit();
                }

                m_visionMain.SetInlineStart(false);
            }
            break;

        case MSG_PROBE_INLINE_STOP:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_INLINE_STOP"));
            if (1)
            {
                m_visionMain.SetInlineStop(false);
            }
            break;
        case MSG_PROBE_CHECK_READY_STATUS: //kircheis_150417
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_CHECK_READY_STATUS"));
            if (1)
            {
            }
            break;

        case MSG_PROBE_LOT_START:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_LOT_START"));
            if (1)
            {
                // SDY 3D Noise test를 위해 Lot 시작 시 Noise 탐지 알고리즘을 돌린다.

                auto& visionUnit = m_visionMain.GetPrimaryVisionUnit();
                auto& imageLot = visionUnit.getImageLot();
                if (SystemConfig::GetInstance().GetUse3DCameraNoiseTest() == true)
                {
                    m_visionMain.Check3DNoise(imageLot, false);
                }

                // 영훈 20150512_DeleteMarkImages : Inline Start시 Job에 저장된 Mark 이미지를 모두 지우고 시작하도록 한다.
                m_visionMain.DeleteMarkMultiImages();

                //	전호빈 2020.04.19 - Save SystemInfo : Lot Start시, System 폴더의 .csv파일 및 system.ini를 Raw Image 폴더에 압축하여 저장한다.
                m_visionMain.SaveSystemInfo();

                //mc_MED#3.5 AllFrameSave
                m_visionMain.ResetDevicePassCount();

                SystemConfig::GetInstance().m_bIsNGRVRun = FALSE; //kircheis_NGRV_MSG

                m_visionMain.SetScanTrayIndex(-1);

                if (m_visionMain.GetFreeSpaceCheck(70, _TEXT("D:\\")) == FALSE)
                {
                    CString strMessage = _T("The disk capacity is less than 70 GB. Check Out D Drive Capacity");
                    m_visionMain.iPIS_Send_ErrorMessageForLog(_T("Storage warning"), strMessage);
                    m_visionMain.iPIS_Send_ErrorMessageForPopup(strMessage);
                }

                if (SystemConfig::GetInstance().Get_SaveSequenceAndInspectionTimeLog() == true)
                {
                    if (m_visionMain.GetFreeSpaceCheck(20, _TEXT("C:\\")) == FALSE) //C드라이브 용량이 20기가 밑이면..
                    {
                        if (m_visionMain.GetFreeSpaceCheck(50, _TEXT("D:\\"))
                            == FALSE) //D드라이브 용량이 50기가 밑이면..
                        {
                            if (m_visionMain.GetFreeSpaceCheck(50, _TEXT("E:\\"))
                                == FALSE) //E드라이브 용량이 50기가 밑이면..
                            {
                                //방법이 없다 설비 세워야지
                                CString strMessage
                                    = _T("The disk capacity is less than 50 GB. Check Out E Drive Capacity");
                                m_visionMain.iPIS_Send_ErrorMessageForLog(_T("Storage warning"), strMessage);
                                m_visionMain.iPIS_Send_ErrorMessageForPopup(strMessage);
                            }
                            else
                                SystemConfig::GetInstance().Replace_TimeLog_Drive(_T("E:\\"));
                        }
                        else
                            SystemConfig::GetInstance().Replace_TimeLog_Drive(_T("D:\\"));
                    }
                }

                //mc_23.05.23 Lot Start시, TimeLog관련 파일은 삭제하여 준다
                std::vector<CString> vecstrVisionTimeLogFileNames, vecstrVisionTimeLogFilePath;

                if (SystemConfig::GetInstance().GetFileNamesinDirecotry(DynamicSystemPath::get(DefineFolder::TimeLog),
                        _T("*.txt"), vecstrVisionTimeLogFileNames, vecstrVisionTimeLogFilePath)
                    == true)
                {
                    for (auto DeleteFilePath : vecstrVisionTimeLogFilePath)
                        ::DeleteFile(DeleteFilePath);
                }
            }
            break;

            // 영훈 20150224 : Lot Start 정보를 받도록 한다.
        case MSG_PROBE_LOTINFO:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_LOTINFO"));
            if (1)
            {
                m_visionMain.HardwareSetup();
                m_visionMain.OnImagingConditionChanged();

                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);

                try
                {
                    long inlineRunCnt;
                    ar >> m_visionMain.m_lotID;
                    ar >> m_visionMain.m_lotReviewFileHeader;
                    ar >> inlineRunCnt;
                    ar >> m_visionMain.m_timeLotStart;

                    m_visionMain.SetInlineRunCnt(inlineRunCnt);
                }
                catch (CArchiveException* ae)
                {
                    ::AfxMessageBox(CString("Archive Exception : Probe Lot Infomation") + ae->m_strFileName);
                    ae->Delete();
                }
                catch (CFileException* fe)
                {
                    ::AfxMessageBox(CString("File Exception : Probe Lot Infomation") + fe->m_strFileName);
                    fe->Delete();
                }

                SystemConfig::GetInstance().Set_LotIDFromHost(m_visionMain.m_lotID);
                SystemConfig::GetInstance().Set_Lot_Start_TimeFromHost(m_visionMain.m_timeLotStart);

                //k Lot Start 직전에 Raw Image 저장이 설정 되어 있을 시 무조건 Save..
                //원래 목적대로라면 검사 결과와 지금 세이브 옵션을 비교해야하지만 Inline 상황에서 저장할 시 터지는 오류가 발생함
                if (SystemConfig::GetInstance().m_saveInlineRawImage
                    || m_visionMain.m_saveOption.m_vecRawImageSaveOption.size() > 0)
                {
                    const auto& lotID = m_visionMain.m_lotID;
                    const auto& lotStartTime = m_visionMain.m_timeLotStart;

                    CString strDirectory;
                    strDirectory.Format(_T("%s%04d.%02d.%02d\\%s\\%s\\"), RAW_IMAGE_DIRECTORY, lotStartTime.GetYear(),
                        lotStartTime.GetMonth(), lotStartTime.GetDay(), LPCTSTR(m_visionMain.m_strJobFileName),
                        LPCTSTR(lotID));

                    Ipvm::CreateDirectories(LPCTSTR(strDirectory));

                    CString strJobPath(strDirectory + m_visionMain.m_strJobFileName + _T(".vmjob"));

                    m_visionMain.SaveJobFile(strJobPath, VisionMainAgent::SaveJobMode::SaveOnly);
                }

                ar.Close();
            }
            break;

        case MSG_PROBE_MANUAL_INFO:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_MANUAL_INFO"));
            if (1)
            {
                SystemConfig::GetInstance().m_bIsAnalysisResult = true;
                m_visionMain.SetInlineRunCnt(0);
                m_visionMain.m_timeLotStart = CTime::GetCurrentTime();
                CString strTime = m_visionMain.m_timeLotStart.Format("%H%M");
                m_visionMain.m_lotID.Format(_T("ManualInspection_%s"), (LPCTSTR)strTime);
                m_visionMain.m_lotReviewFileHeader = m_visionMain.m_lotID + _T("_");

                if (SystemConfig::GetInstance().m_saveInlineRawImage)
                {
                    const auto& lotID = m_visionMain.m_lotID;
                    const auto& lotStartTime = m_visionMain.m_timeLotStart;

                    CString strDirectory;
                    strDirectory.Format(_T("%s%04d.%02d.%02d\\%s\\%s\\Run_%02d\\"), RAW_IMAGE_DIRECTORY,
                        lotStartTime.GetYear(), lotStartTime.GetMonth(), lotStartTime.GetDay(),
                        LPCTSTR(m_visionMain.m_strJobFileName), LPCTSTR(lotID), m_visionMain.GetInlineRunCnt());

                    Ipvm::CreateDirectories(LPCTSTR(strDirectory));

                    CString strJobPath(strDirectory + m_visionMain.m_strJobFileName + _T(".vmjob"));

                    m_visionMain.SaveJobFile(strJobPath, VisionMainAgent::SaveJobMode::SaveOnly);
                }
            }
            break;

        case MSG_PROBE_SEPARATION_HARDWARE_JOB:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_SEPARATION_HARDWARE_JOB"));
            if (1)
            {
            }
            break;

        case MSG_PROBE_STITCH_GRAB_DIRECTION:
        {
            if (SystemConfig::GetInstance().GetHandlerType() == HANDLER_TYPE_380BRIDGE)
                break;
            DEFMSGPARAM* pDatagram = (DEFMSGPARAM*)pData;

            SystemConfig::GetInstance().m_nStitchGrabDirection = (int)(pDatagram->P1);
        }
        break;
        case MSG_PROBE_SEND_PACKAGE_SPEC_TO_VISION:
        {
            if (m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.m_menuAccessUI != nullptr)
            {
                OnInspectionQuit();
            }
            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            CiDataBase jobDB;
            BOOL bJobCheck = TRUE;
            try
            {
                bJobCheck &= jobDB.Serialize(ar);
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            bJobCheck &= m_visionMain.LinkDataBase_PackageSpec(false, jobDB);

            if (!bJobCheck)
                break;

            //{{일단 Job을 저장 한 후 다시 열자. Job의 일부 항목 중에 Package Spec을 참조하는 넘이 있으므로.. 그리고 이때 Job Send도 하자.
            m_visionMain.SaveJobFile(m_visionMain.m_strJobFileName, VisionMainAgent::SaveJobMode::Normal, TRUE);
            //}}
        }
        break;
        case MSG_PROBE_JOB_DOWNLOAD:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_JOB_DOWNLOAD"));

            m_bIsUpdateRecipeDone = FALSE;

            if (1)
            {
                // 영훈 20140203 : Detail Setup Mode나 Batch inps Mode등 무언가가 열려있을 경우 닫고 시작하도록 한다.
                if (m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.m_menuAccessUI != nullptr)
                {
                    OnInspectionQuit();
                }

                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);
                if (nMessageLength < 100)
                {
                    m_visionMain.iPIS_Send_ErrorMessageForPopup(
                        _T("The vision recipe file size is invalid, Please check the vision recipe file."));
                    //break; // break 할 경우에는 Default_NGRV를 열지 않는 문제가 있다.
                }

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
                CString strJob;
                CString errorMessage;

                BOOL bJobCheck = TRUE;

                double scanLength_mm = 0.;
                BinaryFileData jobBinary;

                CString strJobFileTemp;
                CString strJobName;
                CString strJobPath;

                try
                {
                    ar >> strJobFileTemp;
                    ar >> scanLength_mm;

                    strJobName = REMOVE_FOLDER_FROM_PATHNAME(strJobFileTemp); // 영훈 20130808 : 경로를 없앤다.
                    strJobName = REMOVE_EXT_FROM_FILENAME(strJobName); // 영훈 20130808 : 확장자 명을 없앤다.

                    strJobPath = DynamicSystemPath::get(DefineFolder::Job) + strJobName + _T(".vmjob");
                    m_visionMain.SetVisionJobName(strJobPath, strJobName);

                    // 여긴 원래대로 받도록 한다.
                    bJobCheck &= jobBinary.SerializeForComm(ar);

                    strJob = strJobPath;
                }
                catch (CArchiveException* ae)
                {
                    m_visionMain.iPIS_Send_ErrorMessageForPopup(
                        _T("Vision error occured. Please change the screen and check error message."));

                    ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                    ae->Delete();
                }
                catch (CFileException* fe)
                {
                    m_visionMain.iPIS_Send_ErrorMessageForPopup(
                        _T("Vision error occured. Please change the screen and check error message."));

                    ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                    fe->Delete();
                }

                ar.Close();

                BOOL jobVersionChanged = FALSE;

                CiDataBase jobDB;

                if (memFile.GetLength() > 1024)
                {
                    jobBinary.ToDB(jobDB);
                }

                SystemConfig::GetInstance().m_nRecipeOpenType
                    = RECIPE_OPEN_TYPE_NORMAL; // Recipe Normal Open From Host - JHB_NGRV_RECIPE_TYPE
                m_visionMain.m_bHostConnected = TRUE; // Host와 연결이 되어있기 때문에 이 함수에 들어와있다

                if (!m_visionMain.LinkDataBase(FALSE, jobDB))
                {
                    errorMessage = m_visionMain.GetLastLinkDataBaseErrorMessage();
                    bJobCheck = FALSE;
                }
                else
                {
                    jobVersionChanged = m_visionMain.GetLastLinkDataBaseJobVersionChanged();
                }

                m_visionMain.SetVisionJobName(
                    strJobPath, strJobName); // Default job이 열려도 기존에 열려던 job name 으로 이름을 수정한다.

                //-----------------------------------------------------------------------
                // Host에서 저장시 항상 최신 Vision Job Version 으로
                // Job Version Changed가 TRUE이면 최신 Job을 전달하게 했었다.
                // 허나 Job Version Changed가 TRUE이면 자꾸 Host로 화면전환이 되어
                // 일단 막아 놓았다.
                jobVersionChanged = FALSE;
                //
                //-----------------------------------------------------------------------

                m_visionMain.SetScanSpec(CAST_FLOAT(scanLength_mm));

                //m_visionMain.OnJobChanged();
                m_visionMain.ReturnToPrimaryUI(FALSE);

                m_visionMain.iPis_Send_JobInfo();
                m_visionMain.iPIS_Send_JobdownloadAck(bJobCheck, errorMessage);
                m_visionMain.iPIS_Send_InspectionItemsName();

                //{{//kircheis_Comp3DHeightBug
                if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
                {
                    m_visionMain.CheckIntegrityOfCompMapData();
                }
                //}}

                if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP)
                {
                    //좌표만 재구성 함수만 구성되면 된다
                    if (m_visionMain.Init_NGRV_SingleRunInfo() == false)
                    {
                        //Send Error Msg
                    }
                    else //초기화를 실패했는데 보낼 이유는 없다
                        m_visionMain.iPIS_Send_NGRV_SingleRunInfo();
                }

                if (jobVersionChanged)
                {
                    m_visionMain.SaveJobFile(m_visionMain.m_strJobFileName, VisionMainAgent::SaveJobMode::Normal, TRUE);
                }
            }
            m_bIsUpdateRecipeDone = TRUE;
            break;

        case MSG_PROBE_SEND_SYSTEM_PARAMETERS_TO_VISION:
        {
            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            VisionSystemParameters visionSysParams;

            try
            {
                ar >> visionSysParams;
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            m_visionMain.iPIS_Send_SystemParametersMatchResult(visionSysParams);
        }
        break;
        case MSG_PROBE_SEND_SYSTEM_PARAMETERS_TO_VISION_FOR_APPLY:
        {
            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            VisionSystemParameters receiveSysParams;

            try
            {
                ar >> receiveSysParams;
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            m_visionMain.SetVisionSystemParameters(receiveSysParams);
        }
        break;
        //kk Grab Retry : Host에게 Retry하라고 전송!
        case MSG_PROBE_3D_GRAB_RETRY_ACK:
        {
            m_visionMain.m_bGrabRetry = TRUE;
        }
        break;
        case MSG_PROBE_HOST_RECIPE_UPDATED:
        {
            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            long nSampleImageOption(-1);
            try
            {
                ar >> nSampleImageOption;
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            m_visionMain.SetSampleImageOption(nSampleImageOption);
        }
        break;

        case MSG_PROBE_JOB_DOWNLOAD_FOR_BYPASS: //kircheis_NGRV_MSG
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_JOB_DOWNLOAD"));
            if (1)
            {
                // 영훈 20140203 : Detail Setup Mode나 Batch inps Mode등 무언가가 열려있을 경우 닫고 시작하도록 한다.
                if (m_visionMain.IsShowVisionPrimaryUI() == false || m_visionMain.m_menuAccessUI != nullptr)
                {
                    OnInspectionQuit();
                }

                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);

                if (nMessageLength < 100)
                {
                    m_visionMain.iPIS_Send_ErrorMessageForPopup(
                        _T("The vision recipe file size is invalid, Please check the vision recipe file."));
                    //break; // break 할 경우에는 Default_NGRV를 열지 않는 문제가 있다.
                }

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
                CString strJob;
                CString errorMessage;

                BOOL bJobCheck = TRUE;

                double scanLength_mm = 0.;
                BinaryFileData jobBinary;

                CString strJobFileTemp;
                CString strJobName;
                CString strJobPath;

                try
                {
                    ar >> strJobFileTemp;
                    ar >> scanLength_mm;

                    strJobName = REMOVE_FOLDER_FROM_PATHNAME(strJobFileTemp); // 영훈 20130808 : 경로를 없앤다.
                    strJobName = REMOVE_EXT_FROM_FILENAME(strJobName); // 영훈 20130808 : 확장자 명을 없앤다.

                    strJobPath = DynamicSystemPath::get(DefineFolder::Job) + strJobName + _T(".vmjob");
                    m_visionMain.SetVisionJobName(strJobPath, strJobName);

                    // 여긴 원래대로 받도록 한다.
                    bJobCheck &= jobBinary.SerializeForComm(ar);

                    strJob = strJobPath;
                }
                catch (CArchiveException* ae)
                {
                    m_visionMain.iPIS_Send_ErrorMessageForPopup(
                        _T("Vision error occured. Please change the screen and check error message."));

                    ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                    ae->Delete();
                }
                catch (CFileException* fe)
                {
                    m_visionMain.iPIS_Send_ErrorMessageForPopup(
                        _T("Vision error occured. Please change the screen and check error message."));

                    ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                    fe->Delete();
                }

                ar.Close();

                BOOL jobVersionChanged = FALSE;

                CiDataBase jobDB;
                jobBinary.ToDB(jobDB);

                SystemConfig::GetInstance().m_nRecipeOpenType
                    = RECIPE_OPEN_TYPE_BYPASS; // Recipe Normal Open From Host - JHB_NGRV_RECIPE_TYPE
                m_visionMain.m_bHostConnected = TRUE; // Host와 연결이 되어있기 때문에 이 함수에 들어와있다

                if (!m_visionMain.LinkDataBase(FALSE, jobDB))
                {
                    errorMessage = m_visionMain.GetLastLinkDataBaseErrorMessage();
                    bJobCheck = FALSE;
                }
                else
                {
                    jobVersionChanged = m_visionMain.GetLastLinkDataBaseJobVersionChanged();
                }

                m_visionMain.SetVisionJobName(
                    strJobPath, strJobName); // Default job이 열려도 기존에 열려던 job name 으로 이름을 수정한다.

                //-----------------------------------------------------------------------
                // Host에서 저장시 항상 최신 Vision Job Version 으로
                // Job Version Changed가 TRUE이면 최신 Job을 전달하게 했었다.
                // 허나 Job Version Changed가 TRUE이면 자꾸 Host로 화면전환이 되어
                // 일단 막아 놓았다.
                jobVersionChanged = FALSE;
                //
                //-----------------------------------------------------------------------

                m_visionMain.SetScanSpec(CAST_FLOAT(scanLength_mm));

                //m_visionMain.OnJobChanged();
                m_visionMain.ReturnToPrimaryUI(FALSE);

                m_visionMain.iPis_Send_JobInfo();
                m_visionMain.iPIS_Send_JobdownloadAck(bJobCheck, errorMessage);

                if (jobVersionChanged)
                {
                    m_visionMain.SaveJobFile(m_visionMain.m_strJobFileName, VisionMainAgent::SaveJobMode::Normal, TRUE);
                }
            }
        }
        break;

        case MSG_PROBE_NGRV_ALL_INSPECTION_LIST: //kircheis_NGRV_MSG
        {
            m_visionMain.m_vecstrInspModuleNameNGRV.clear();
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            long nSize = 0;
            try
            {
                ar >> nSize;
                CString strInspModuleName;
                for (long i = 0; i < nSize; i++)
                {
                    ar >> strInspModuleName;
                    m_visionMain.m_vecstrInspModuleNameNGRV.push_back(strInspModuleName);
                }

                if (m_visionMain.ClassifyInspItemInfo(m_visionMain.m_vecstrInspModuleNameNGRV) == false)
                    m_visionMain.iPIS_Send_NGRV_WarningVisionName();
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();
        }
        break;

        case MSG_PROBE_NGRV_PACKAGE_INFO_IN_TRAY: //kircheis_NGRV_MSG
        {
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            long nSize = 0;
            long nTrayID = 0;
            try
            {
                ar >> nTrayID;
                m_visionMain.SetScanTrayIndex((int)nTrayID);
                ar >> nSize;
                NgrvPackageGrabInfo packageGrabInfo;

                m_visionMain.m_vecPackageGrabInfo
                    .clear(); // 계속 보내주고 PushBack을 하기때문에 초기화를 한 번 해줘야 함 - JHB_NGRV

                for (long i = 0; i < nSize; i++)
                {
                    ar >> packageGrabInfo;
                    m_visionMain.m_vecPackageGrabInfo.push_back(packageGrabInfo);
                }
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            m_visionMain.SaveGrabInfoLog(m_visionMain.m_vecPackageGrabInfo, m_visionMain.m_bNGRV_SideFlip);

            if (!m_visionMain.m_bNGRV_SideFlip)
            {
                m_visionMain.m_bNGRV_SideFlip = TRUE;
            }
            else
            {
                m_visionMain.m_bNGRV_SideFlip = FALSE;
            }

            m_visionMain.iPIS_Send_PackageGrabInfo_Receive_Done(); // Receive GrabInfo Done - JHB_NGRV
        }
        break;

        case MSG_PROBE_LOT_START_FOR_NGRV: //kircheis_NGRV_MSG
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_LOT_START_FOR_NGRV"));

            if (SystemConfig::GetInstance().IsVisionType3D() == TRUE)
                break;
            else if (SystemConfig::GetInstance().m_nVisionInfo_NumType != 1)
            {
                if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                    break;
            }

            // 영훈 20150512_DeleteMarkImages : Inline Start시 Job에 저장된 Mark 이미지를 모두 지우고 시작하도록 한다.
            m_visionMain.DeleteMarkMultiImages();

            //	전호빈 2020.04.19 - Save SystemInfo : Lot Start시, System 폴더의 .csv파일 및 system.ini를 Raw Image 폴더에 압축하여 저장한다.
            m_visionMain.SaveSystemInfo();

            //mc_MED#3.5 AllFrameSave
            m_visionMain.ResetDevicePassCount();

            auto& visionUnit = m_visionMain.GetPrimaryVisionUnit();
            if (SystemConfig::GetInstance().IsBtm2DVision() == TRUE && visionUnit.Is2DMatrixReadingBypass() == false)
            {
                m_visionMain.iPIS_Send_WarningBTM2DRecipe();
            }

            SystemConfig::GetInstance().m_bIsNGRVRun = TRUE;
        }
        break;

        case MSG_PROBE_EACH_VISION_NAME_FOR_NGRV: //kircheis_NGRV_MSG
        {
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            try
            {
                ar >> m_visionMain.m_vecstrEachVisionName[0] >> m_visionMain.m_vecstrEachVisionName[1]
                    >> m_visionMain.m_vecstrEachVisionName[2] >> m_visionMain.m_vecstrEachVisionName[3];
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();
        }
        break;

        case MSG_PROBE_NGRV_AF_PLANE_REF_INFO: //kircheis_NGRVAF
        {
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            try
            {
                ar >> m_visionMain.m_ngrvAFRefInfo;

                NgrvAfRefInfo& ngrvAFRefInfo = (m_visionMain.m_ngrvAFRefInfo.m_nVisionID == VISION_BTM_2D)
                    ? m_visionMain.m_ngrvAFRefInfor_BTM
                    : m_visionMain.m_ngrvAFRefInfor_TOP;

                ngrvAFRefInfo.m_nVisionID = m_visionMain.m_ngrvAFRefInfo.m_nVisionID;
                ngrvAFRefInfo.m_bIsValidPlaneRefInfo = m_visionMain.m_ngrvAFRefInfo.m_bIsValidPlaneRefInfo;
                long nPointNum = (long)m_visionMain.m_ngrvAFRefInfo.m_vecptRefPos_UM.size();
                ngrvAFRefInfo.m_vecptRefPos_UM.clear();
                ngrvAFRefInfo.m_vecptRefPos_UM.resize(nPointNum);
                for (long nIdx = 0; nIdx < nPointNum; nIdx++)
                    ngrvAFRefInfo.m_vecptRefPos_UM[nIdx] = m_visionMain.m_ngrvAFRefInfo.m_vecptRefPos_UM[nIdx];
                ngrvAFRefInfo.m_nImageSizeX = m_visionMain.m_ngrvAFRefInfo.m_nImageSizeX;
                ngrvAFRefInfo.m_nImageSizeY = m_visionMain.m_ngrvAFRefInfo.m_nImageSizeY;
                long nImageSize = ngrvAFRefInfo.m_nImageSizeX * ngrvAFRefInfo.m_nImageSizeY * 3;
                ngrvAFRefInfo.m_vecbyImage.clear();
                ngrvAFRefInfo.m_vecbyImage.resize(nImageSize);
                memcpy(&ngrvAFRefInfo.m_vecbyImage[0], &m_visionMain.m_ngrvAFRefInfo.m_vecbyImage[0], nImageSize);
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();
        }
        break;

        case MSG_PROBE_NGRV_VISION_SINGLE_GRAB:
        {
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            long nImageSizeX = 0;
            long nImageSizeY = 0;

            long nSelectedFrameNum = 0; // Select Frame Number Initialize, Default Value
            BOOL bOriginal = FALSE;
            long nOriginalWidthByte = 0;
            long nResizeWidthByte = 0;

            DEFMSGPARAM* pDatagram = (DEFMSGPARAM*)pData;
            nSelectedFrameNum = (int)pDatagram->P1;
            bOriginal = (BOOL)pDatagram->P2;

            Ipvm::Image8u3 image;
            Ipvm::Image8u3 resizeImage;

            m_visionMain.OnImageGrab();

            auto& visionUnit = m_visionMain.GetPrimaryVisionUnit();
            auto& imageLot = visionUnit.getImageLot();

            image.Create(imageLot.GetImageSizeX(), imageLot.GetImageSizeY());

            image = imageLot.GetColorImageFrame(nSelectedFrameNum);

            if (!bOriginal)
            {
                // 영훈 20150702 : 1/8배로 압축해서 호스트로 보내도록 한다.
                long nResizeScale = SEND_IMAGE_RESCALE_FACTOR;

                // Reszie image Size 계산
                nImageSizeX = ((image.GetSizeX() / nResizeScale + 3) / 4) * 4;
                nImageSizeY = image.GetSizeY() / nResizeScale;
                nResizeWidthByte = nImageSizeX * 3;

                // image reszie시 for문 연산을 IPP 연산을 하도록 한다.
                resizeImage.Create(nImageSizeX, nImageSizeY);
                Ipvm::ImageProcessing::ResizeLinearInterpolation(image, resizeImage);

                m_visionMain.m_pMessageSocket->Write(
                    MSG_PROBE_IMAGE_SEND_3, nResizeWidthByte * nImageSizeY, resizeImage.GetMem());
            }
            else
            {
                nImageSizeX = image.GetSizeX();
                nImageSizeY = image.GetSizeY();
                nOriginalWidthByte = nImageSizeX * 3;

                m_visionMain.m_pMessageSocket->Write(
                    MSG_PROBE_IMAGE_SEND_2, nOriginalWidthByte * nImageSizeY, image.GetMem());
            }

            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Send NGRV Color Image to Host in Manual Inspection"));
        }
        break;

        case MSG_PROBE_SYSTEM_OPTION:
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_SYSTEM_OPTION"));

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);

            long nNGRVImageFormat, nTimeoutOfVisionResult_ms(0);

            try
            {
                ar >> nNGRVImageFormat;
                ar >> nTimeoutOfVisionResult_ms;

                if (nNGRVImageFormat > ImageSaveOption::NGRV_IMAGE_FORMAT_PNG
                    || nNGRVImageFormat < ImageSaveOption::NGRV_IMAGE_FORMAT_BMP)
                {
                    SystemConfig::GetInstance().m_nSaveImageTypeForNGRV = ImageSaveOption::NGRV_IMAGE_FORMAT_JPG;
                }
                else
                {
                    SystemConfig::GetInstance().m_nSaveImageTypeForNGRV = nNGRVImageFormat;
                }

                SystemConfig::GetInstance().SetInspectionResultTimeoutTime_ms(nTimeoutOfVisionResult_ms);
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();
        }
        break;

        case MSG_PROBE_REQUEST_TEXT_RECIPE: //kircheis_TxtRecipe
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_REQUEST_TEXT_RECIPE"));

            // Recipe Update가 완료되지 않으면 Text Recipe Export Message를 받지 않는 것으로 수정 : 메세지가 겹치면서 Disconnect Error 발생 - 2022.10.13_JHB
            if (m_bIsUpdateRecipeDone == TRUE)
            {
                m_visionMain.iPIS_Send_TextRecipe();
            }
        }
        break;

        case MSG_PROBE_SET_TIME_SYNCHRONIZE:
        {
            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            SYSTEMTIME HostSystemTime;

            try
            {
                ar >> HostSystemTime.wYear;
                ar >> HostSystemTime.wMonth;
                ar >> HostSystemTime.wDayOfWeek;
                ar >> HostSystemTime.wDay;
                ar >> HostSystemTime.wHour;
                ar >> HostSystemTime.wMinute;
                ar >> HostSystemTime.wSecond;
                ar >> HostSystemTime.wMilliseconds;

                SetLocalTime(&HostSystemTime);

                DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("System time set to Host PC time"));
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();
        }
        break;

        case MSG_PROBE_REQUEST_VISION_CONDITION:
            if (1)
            {
                DEFMSGPARAM VisionConditionData
                    = {SystemConfig::GetInstance().GetVisionCondition(), 0, 0, 0, 0, 0, 0, 0};

                m_visionMain.m_pMessageSocket->Write(
                    MSG_PROBE_REQUEST_VISION_CONDITION_ACK, sizeof(DEFMSGPARAM), (BYTE*)&VisionConditionData);
            }
            break;
        case MSG_PROBE_REQUEST_VISION_LOG: //mc_보내고 싶은 Log File을 모두다 적어서 보내면 된다
        {
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Main-Received::MSG_PROBE_REQUEST_VISION_LOG"));

            std::vector<CString> vecstrVisionTimeLogFileNames_Exist, vecstrVisionTimeLogFilePath_Exist;

            if (SystemConfig::GetInstance().GetFileNamesinDirecotry(DynamicSystemPath::get(DefineFolder::TimeLog),
                    _T("*.txt"), vecstrVisionTimeLogFileNames_Exist, vecstrVisionTimeLogFilePath_Exist)
                == true)
            {
                const long nFileNamesSize = (long)vecstrVisionTimeLogFileNames_Exist.size();
                const long nFilePathSize = (long)vecstrVisionTimeLogFilePath_Exist.size();

                if (nFileNamesSize != nFilePathSize) //Size가 다를수는 없다 같아야 한다
                    break;

                for (long nFileidx = 0; nFileidx < nFileNamesSize; nFileidx++)
                {
                    CString strCopyFileName(""), strCopyFilePath("");
                    strCopyFileName.AppendFormat(_T("Copy_%s"), (LPCTSTR)vecstrVisionTimeLogFileNames_Exist[nFileidx]);
                    strCopyFilePath.AppendFormat(
                        _T("%s%s"), (LPCTSTR)(DynamicSystemPath::get(DefineFolder::TimeLog)), (LPCTSTR)strCopyFileName);
                    ::CopyFile(vecstrVisionTimeLogFilePath_Exist[nFileidx], strCopyFilePath, TRUE);

                    m_visionMain.iPIS_Send_VisionLogFile(vecstrVisionTimeLogFileNames_Exist[nFileidx], strCopyFilePath);

                    ::DeleteFile(
                        strCopyFilePath); //Send했으면 삭제.. 유효성 검사를 한번하면 좋을꺼 같은데 나중에 생각해보자
                }
            }
        }
        break;
        case MSG_PROBE_NGRV_RUN_METHOD_INFO:
        {
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            try
            {
                ar >> m_visionMain.m_isNGRVSingleRun;
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            m_visionMain.iPIS_Send_NGRV_RunMode_Receive_Done();
        }
        break;

        case MSG_PROBE_NGRV_SINGLE_RUN_VISION_TYPE_INFO:
        {
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            try
            {
                ar >> m_visionMain.m_isNGRVSingleRunVisionType;
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            m_visionMain.iPIS_Send_NGRV_SingleRun_Visiontype_Receive_Done();
        }
        break;

        case MSG_PROBE_NGRV_SEND_2D_VISION_INFO:
        {
            if (SystemConfig::GetInstance().IsVisionTypeNGRV() == false)
                break;

            CMemFile memFile;
            memFile.Attach((byte*)pData, nMessageLength);

            ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
            try
            {
                ar >> m_visionMain.m_ngrv2DImageInfo;

                // VisionID 2D BTM/TOP 확인하여 해당 정보 저장
                Ngrv2DImageInfo& ngrv2DImageInfo = (m_visionMain.m_ngrv2DImageInfo.m_visionID == VISION_BTM_2D)
                    ? m_visionMain.m_ngrv2DImageInfo_BTM
                    : m_visionMain.m_ngrv2DImageInfo_TOP;

                // Vision ID
                ngrv2DImageInfo.m_visionID = m_visionMain.m_ngrv2DImageInfo.m_visionID;
                // Image Size
                ngrv2DImageInfo.m_imageSizeX = m_visionMain.m_ngrv2DImageInfo.m_imageSizeX;
                ngrv2DImageInfo.m_imageSizeY = m_visionMain.m_ngrv2DImageInfo.m_imageSizeY;
                ngrv2DImageInfo.m_totalImageSize = m_visionMain.m_ngrv2DImageInfo.m_totalImageSize;
                // Image Scale
                ngrv2DImageInfo.m_scaleX = m_visionMain.m_ngrv2DImageInfo.m_scaleX;
                ngrv2DImageInfo.m_scaleY = m_visionMain.m_ngrv2DImageInfo.m_scaleY;
                ngrv2DImageInfo.m_isImageColor = m_visionMain.m_ngrv2DImageInfo.m_isImageColor;
                // Image
                ngrv2DImageInfo.m_vecbyImage.clear();
                ngrv2DImageInfo.m_vecbyImage.resize(m_visionMain.m_ngrv2DImageInfo.m_totalImageSize);
                memcpy(&ngrv2DImageInfo.m_vecbyImage[0], &m_visionMain.m_ngrv2DImageInfo.m_vecbyImage[0],
                    m_visionMain.m_ngrv2DImageInfo.m_totalImageSize);

                m_visionMain.Set2DImageInfo_NGRV(ngrv2DImageInfo.m_visionID, ngrv2DImageInfo);
            }
            catch (CArchiveException* ae)
            {
                ::AfxMessageBox(CString("Archive Exception : ") + ae->m_strFileName);
                ae->Delete();
            }
            catch (CFileException* fe)
            {
                ::AfxMessageBox(CString("File Exception : ") + fe->m_strFileName);
                fe->Delete();
            }

            ar.Close();

            m_visionMain.iPIS_Send_NGRV_2DVisionInfo_Receive_Done();
        }
        break;
        default:
            break;
    }

    m_visionMain.m_pVisionPrimaryUI->OnMessageSocketDataReceived(msgInfo);

    return 0;
}
