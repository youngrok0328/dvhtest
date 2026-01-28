//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionPrimaryUI.h"

//CPP_2_________________________________ This project's headers
#include "HostCommand.h"
#include "VisionPrimaryUI.h"

//CPP_3_________________________________ Other projects' headers
#include "../../HardwareModules/dPI_Framegrabber/FrameGrabber.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMark.h"
#include "../../ManagementModules/VisionMain/VisionMain.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/iPIS_MsgDefinitions.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/DevelopmentLog.h"
#include "../../InformationModule/dPI_DataBase/SlitBeam3DParameters.h"
#include <Ipvm/Algorithm/ImageProcessing.h>

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Image8u3.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "Psapi.lib")

void CDlgVisionPrimaryUI::OnMessageSocketDataReceived(const Ipvm::SocketMessaging::MSG_INFO& msgInfo)
{
    const long nMessageID = msgInfo.m_messageIndex;
    const long nMessageLength = msgInfo.m_messageLength;
    const BYTE* pData = (const BYTE*)msgInfo.m_message;

    static const bool isBased2D = SystemConfig::GetInstance().IsVisionTypeBased2D(); //kircheis_SWIR

    switch (nMessageID)
    {
        case MSG_PROBE_TEACH_SKIP_MotionEnd: // 영훈 20140116 : Skip후 프로브 이동이 끝나면 Grab을 해준다.
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_TEACH_SKIP_MotionEnd"));
            m_visionMainAgent.OnImageGrab();
            break;

        case MSG_PROBE_IMAGE_LIVE: // Host에서 보낼 Live 영상을 시작한다 ( Timer )
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_IMAGE_LIVE"));
            // 영훈 20140107 : 2D Vision에서만 실행하도록 하며 타이머가 실행되지 않았을 경우에만 실행하도록 한다.
            if (isBased2D) //kircheis_SideVision고려 //kircheis_SWIR
            {
                SetTimerFlag(Timer_Send_Host_Live_Image, 500);
            }
            break;

        case MSG_PROBE_IMAGE_FREEZE: // Host에서 보낼 Live 영상을 종료한다.
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_IMAGE_FREEZE"));
            SetTimerFlag(Timer_Send_Host_Live_Image, 0);
            break;

            //  현식 마크티치관련 수정
        case MSG_MARKTEACH_START:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_MARKTEACH_START"));
            {
                BOOL bExistMark = FALSE;

                if (SystemConfig::GetInstance().m_nCurrentAccessMode == _OPERATOR)
                {
                    bExistMark = m_visionUnit.IsMarkExist();
                }
                else
                {
                    bExistMark = TRUE;
                }

                if (bExistMark)
                {
                    m_bMarkTeachFlag = TRUE;
                    m_visionMainAgent.m_bMarkTeach = TRUE;
                    //m_visionMainAgent.OnImageGrab();//이제 Teach의 Grab은 Host에서 관장한다.

                    Sleep(100);
                    //m_visionMainAgent.ShowProcessingTeachingUI(NULL, "Mark Inspection");

                    long nValidPaneID = GetValidPaneID(); //kircheis_201611XX
                    if (nValidPaneID >= 0)
                        m_visionUnit.SetCurrentPaneID(nValidPaneID);
                    else
                    {
                        //영훈 [ AutoTeaching_CurrentPane ] 20130820 : Mark Teaching시 항상 이쪽 Pane은 Package가 있다고 가정한다.
                        long nPaneX
                            = (long)m_visionUnit.getInspectionAreaInfo().m_pocketArrayX.size(); //kircheis_201611XX
                        long nPaneY = (long)m_visionUnit.getInspectionAreaInfo().m_pocketArrayY.size();
                        long nPaneNum = (nPaneX * nPaneY) - nPaneX;
                        m_visionUnit.SetCurrentPaneID(nPaneNum);
                    }

                    ::SendMessage(m_visionMainAgent.m_hwndMainFrame, m_visionMainAgent.m_msgShowTeach, 0, 0);
                }
            }
            break;
            // inteksjs Mark AutoTeaching 추가 121204
            // 1. Mark OCR 정보를 받아온다
            //  2. job에 의해 Teaching을 한다(Handler에서는 Teaching 정보를 따로 받아 오지 않는다)
        case MSG_PROBE_MARK_TEACH:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_MARK_TEACH"));
            {
                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
                CString strRefOCR;

                try
                {
                    ar >> strRefOCR;
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

                //VisionInspectionMark m_pVisionInspection = *m_visionUnit.m_VisionMarkInspection;
                ////m_pVisionInspection.m_VisionPara.m_strTeachingVerificationRefString = strRefOCR;

                //if(MarkAutoTeaching() == 0)
                //{
                //	auto &srcImage = m_visionUnit.m_pImageLot->m_vecImages[0];

                //	iPIS_Send_Image(srcImage);

                //	m_LabelResult.SetTextColor(Result2Color(PASS));
                //	m_LabelResult.SetText(_T("Complete"));
                //}
                //else
                //{
                //	m_LabelResult.SetTextColor(Result2Color(REJECT));
                //	m_LabelResult.SetText(_T("InComplete"));
                //}

                //if(m_pVisionInspection.DoTeach())
                //{
                //	m_pMessageSocket->Write(MSG_PROBE_MARK_MULTI_TEACH_ACK, 0, NULL);
                //}
            }
            break;

        case MSG_PROBE_AUTO_MARK_TEACH: //kircheis_AutoTeach
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_AUTO_MARK_TEACH"));
            {
                BOOL bOK = FALSE; // 뭘 시도 했던지 성공 여부
                DEFMSGPARAM turnData = {0, 0, 0, 0, 0, 0, 0, 0};

                //먼저 Grab을 하고
                //m_visionMainAgent.OnImageGrab();//이제 Teach의 Grab은 Host에서 관장한다.

                //Teach Option을 확인
                CHostCommand HostCommand(nMessageID, (BYTE*)pData, nMessageLength);
                DEFMSGPARAM* pDatagram = (DEFMSGPARAM*)HostCommand.m_pBuffer;
                long nAutoTeachMode = (long)(pDatagram->P1);

                //{{Working Proccess
                if (nAutoTeachMode == AutoMarkTestMode) //Test 모드이면
                    bOK = AutoMarkInspSeq(); //모든 Pane에서 Align까지와 Mark만 검사한다. 하나라도 불량이 있으면 FALSE
                else if (nAutoTeachMode == AutoMarkTeachMode) //Teach 모드 이면
                {
                    bOK = AutoMarkTeachSeq(); //Auto Mark Teach하고

                    if (bOK) //성공 했으면 Job Send & Image Send
                    {
                        m_visionMainAgent.SendToHostMarkTeachImage(); //Host로 Teach 영상 보내고
                        if (m_visionMainAgent.SaveJobFile(nullptr, VisionMainAgent::SaveJobMode::Normal, TRUE) == FALSE)
                        {
                            //Job을 저장하고 Job Upload하는데 실패하면
                            bOK = FALSE; //Teach도 실패로 간주 한다
                        }
                    }
                }
                //}}

                //결과 리턴
                turnData.P1 = (long)bOK;
                m_visionMainAgent.m_pMessageSocket->Write(
                    MSG_PROBE_AUTO_MARK_TEACH_RESULT, sizeof(DEFMSGPARAM), (BYTE*)&turnData);
            }
            break;

        case MSG_PROBE_JOBTEACH_DISABLE:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_JOBTEACH_DISABLE"));
            {
                m_bJobTeachEnable = false;
            }
            break;
        case MSG_PROBE_JOBTEACH_ENABLE:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_JOBTEACH_ENABLE"));
            {
                m_bJobTeachEnable = true;
            }
            break;

        case MSG_PROBE_GRAB_START:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_GRAB_START"));
            {
                m_bMarkTeachFlag = TRUE;
                m_visionMainAgent.OnImageGrab();
            }
            break;

        case MSG_PROBE_SET_ACCESS_MODE:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_SET_ACCESS_MODE"));
            {
                CHostCommand HostCommand(nMessageID, (BYTE*)pData, nMessageLength);
                DEFMSGPARAM* pDatagram = (DEFMSGPARAM*)HostCommand.m_pBuffer;

                SystemConfig::GetInstance().m_nCurrentAccessMode = (long)(pDatagram->P1);

                switch (SystemConfig::GetInstance().m_nCurrentAccessMode)
                {
                    case _OPERATOR:
                        m_Label_AccessMode2.SetText(_T("Operator"));
                        break;
                    case _ENGINEER:
                        m_Label_AccessMode2.SetText(_T("Engineer"));
                        break;
                    case _INTEKPLUS:
                        m_Label_AccessMode2.SetText(_T("Intekplus"));
                        break;
                }
            }
            break;

        case MSG_PROBE_INLINE_START:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_INLINE_START"));
            {
                // 영훈 20130107 : Live중이면 꺼주도록 한다.
                SetTimerFlag(Timer_Send_Host_Live_Image, 0);

                m_Label_Connect2.SetText(_T("Connected_Inline"));
            }
            break;

        case MSG_PROBE_INLINE_STOP:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_INLINE_STOP"));
            {
                // 영훈 20130107 : Live중이면 꺼주도록 한다.
                SetTimerFlag(Timer_Send_Host_Live_Image, 0);

                m_Label_Connect2.SetText(_T("Connected"));
            }
            break;

        case MSG_PROBE_LOT_START:
            ::SendMessage(m_visionMainAgent.m_hwndMainFrame, m_visionMainAgent.m_msgShowMain, 0, 0);
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_LOT_START"));
            {
                m_bMarkSkipMode = TRUE;
                m_visionMainAgent.ResetInlineGrabTime();
            }
            break;

        case MSG_PROBE_LOT_END:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_LOT_END"));
            {
                m_bMarkSkipMode = FALSE;
                if (SystemConfig::GetInstance().Get_Enable_GPU_CALC_SAVE_LOG() == true)
                    m_visionMainAgent.SaveInlineGrabTime();
            }

            break;

        case MSG_PROBE_START_TEACH:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_START_TEACH"));
            {
                BOOL bExistMark = FALSE;

                if (SystemConfig::GetInstance().m_nCurrentAccessMode == _OPERATOR)
                {
                    bExistMark = m_visionUnit.IsMarkExist();
                }
                else
                {
                    bExistMark = TRUE;
                }

                if (bExistMark)
                {
                    long nValidPaneID = GetValidPaneID(); //kircheis_201611XX
                    if (nValidPaneID >= 0)
                        m_visionUnit.SetCurrentPaneID(nValidPaneID);

                    ::SendMessage(m_visionMainAgent.m_hwndMainFrame, m_visionMainAgent.m_msgShowTeach, 0, 0);
                }
            }
            break;

        case MSG_PROBE_IMAGE_SAVE_OPTION:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_IMAGE_SAVE_OPTION"));
            if (1)
            {
                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);

                ImageSaveOptionData saveOption;

                saveOption.Serialize(ar);

                m_visionMainAgent.SetImageSaveOption(saveOption);
            }
            break;

        case MSG_PROBE_SCAN_SPEC:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_SCAN_SPEC"));
            if (1)
            {
                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);

                double scanLength_mm = 0.;
                ar >> scanLength_mm;

                m_visionMainAgent.SetScanSpec(CAST_FLOAT(scanLength_mm));
            }
            break;
        case MSG_PROBE_VISION_INFO:
            DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("PrimaryUI-Received::MSG_PROBE_VISION_INFO"));
            if (1)
            {
                CMemFile memFile;
                memFile.Attach((byte*)pData, nMessageLength);

                ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);

                CString strVisionInfo;
                long nVisionInfo_NumType;

                ar >> strVisionInfo;
                ar >> nVisionInfo_NumType;

                SystemConfig::GetInstance().SetVisionInfo(strVisionInfo, nVisionInfo_NumType);
            }
            break;
        default:
            break;
    }
}

void CDlgVisionPrimaryUI::OnMessageSocketConnectionUpdated(const Ipvm::SocketMessaging::EVT_INFO& evtInfo)
{
    if (evtInfo.m_connected)
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Host connected"));
        m_Label_Connect2.SetBkColor(RGB(240, 240, 200));
        m_Label_Connect2.SetText(_T("Connected"));
    }
    else if (evtInfo.m_disconnected)
    {
        DevelopmentLog::AddLog(DevelopmentLog::Type::TCP, _T("Host disconnected"));
        m_Label_Connect2.SetBkColor(RGB(255, 255, 255));
        m_Label_Connect2.SetText(_T("Disconnected"));

        SetTimerFlag(Timer_Send_Host_Live_Image, 0);
    }
}

LRESULT CDlgVisionPrimaryUI::OnGrabEndEvent(WPARAM wparam, LPARAM lparam)
{
    UNREFERENCED_PARAMETER(wparam);
    UNREFERENCED_PARAMETER(lparam);

    static Ipvm::Image8u grabImage;
    static Ipvm::Image8u grabImage2;
    static Ipvm::Image_8u_C3 combineImage;
    static Ipvm::Image8u3 grabColorImage;

    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP
        && SystemConfig::GetInstance().m_bUseBayerPatternGPU == TRUE)
    {
        FrameGrabber::GetInstance().get_live_image(grabImage);
        FrameGrabber::GetInstance().get_live_image(grabColorImage);

        m_imageLotView->SetImage(grabColorImage, _T(""));
    }
    else if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && SystemConfig::GetInstance().IsDual3DGrabMode()) // 3D 복합광학계
    {
        int cameraGrabMode(0);
        if (auto* pMain = dynamic_cast<CVisionMain*>(&m_visionMainAgent))
        {
            if (pMain->m_pSlitBeam3DCommon != nullptr)
                cameraGrabMode = pMain->m_pSlitBeam3DCommon->m_cameraGrabMode;
        }

        switch (cameraGrabMode) // 3D 복합광학계 Grab Mode설정
        {
            case en3DVisionGrabMode::VISION_3D_GRABMODE_SPECULAR:
            {
                FrameGrabber::GetInstance().get_live_image(grabImage);
                m_imageLotView->SetImage(grabImage, _T(""));
                break;
            }
            case en3DVisionGrabMode::VISION_3D_GRABMODE_DEFUSED:
            {
                FrameGrabber::GetInstance().get_live_image2(grabImage);
                m_imageLotView->SetImage(grabImage, _T(""));
                break;
            }
            case en3DVisionGrabMode::VISION_3D_GRABMODE_INTEGRATED:
            {
                FrameGrabber::GetInstance().get_live_image(grabImage);
                FrameGrabber::GetInstance().get_live_image2(grabImage2);

                const int imageSizeX = grabImage.GetSizeX();
                const int imageSizeY = grabImage.GetSizeY();
                Ipvm::ImageProcessing::Multiply(grabImage, 1, Ipvm::Rect32s(grabImage), 0, grabImage);

                Ipvm::Image_8u_C1 temp;
                temp.Create(imageSizeX, imageSizeY);
                temp.FillZero();
                combineImage.Create(imageSizeX, imageSizeY);

                Ipvm::ImageProcessing::CombineRGB(grabImage, grabImage2, temp, Ipvm::Rect(combineImage), combineImage);

                m_imageLotView->SetImage(combineImage, _T(""));
                break;
            }
            default:
                break;
        }
    }
    else
    {
        FrameGrabber::GetInstance().get_live_image(grabImage);

        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP
            && m_visionMainAgent.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
        {
            m_imageLotViewRearSide->SetImage(grabImage, _T(""));
        }
        else
        {
            m_imageLotView->SetImage(grabImage, _T(""));
        }
    }

    return 1L;
}

long CDlgVisionPrimaryUI::MarkAutoTeaching()
{
    BOOL bReturnValue = TRUE;
    long nPaneNum = (long)m_visionUnit.getInspectionAreaInfo().m_unitIndexList.size();

    for (long nPaneID = 0; nPaneID < nPaneNum; nPaneID++)
    {
        m_visionUnit.SetCurrentPaneID(nPaneID);

        for (long i = 0; i < m_visionUnit.GetVisionProcessingCount(); i++)
        {
            auto* visionProcessing = m_visionUnit.GetVisionProcessing(i);

            if (visionProcessing->m_strModuleName == _T("Mark Inspection"))
            {
                break;
            }
            else
            {
                bReturnValue = visionProcessing->DoInspection(true);
                if (!bReturnValue)
                {
                    break;
                }
            }
        }

        if (bReturnValue)
        {
            //if (m_visionUnit.m_VisionMarkInspection->DoTeach())
            {
                break;
            }
        }
    }

    return (bReturnValue ? 0 : -1);
}

BOOL CDlgVisionPrimaryUI::InspectSingleVisionProcess(long nID) //kircheis_AutoTeach
{
    VisionUnit& visionUnit
        = m_visionUnit; //함수 내에서 포인터->포인터->포인터->(변수/함수) 로 길어지는 부분 보기 싫어서 쓴거임

    BOOL bRes = FALSE;

    if (nID < 0 || nID >= visionUnit.GetVisionProcessingCount())
        return bRes;

    return visionUnit.GetVisionProcessing(nID)->DoInspection(true);
}

BOOL CDlgVisionPrimaryUI::
    AutoMarkInspSeq() //kircheis_AutoTeach    //각 Device를 검사하고 Mark Pass 여부를 확인하는데, 이 과정에서 다른 검사들은 방해되니 Align 이전과정과 Mark만 검사
{
    BOOL bStepResult
        = TRUE; //본 시퀀스에서 수행하는 각 Insp 단계(0 ~ Edge Align과 Mark들)의 성공 여부 누적 저장 (하나라도 실패면 FALSE)
    long nPaneNum = (long)m_visionUnit.getInspectionAreaInfo().m_unitIndexList.size();
    long nInspNum = m_visionUnit.GetVisionProcessingCount(); //검사 갯수
    long nAlignPassNum = 0;
    long nMarkPassNum = 0;
    long nAlignID = -1;
    std::vector<long> vecnMarkInspID(0); //Mark Insp의 ID 저장
    long nNumOfMarkInsp = -1; //Mark Insp의 갯수
    long nCurMarkID = -1;

    CString strModuleName, strModuleGUID;
    for (long nInsp = 0; nInsp < nInspNum; nInsp++) //Edge Align과 Mark의 검사 ID를 먼저 찾자.
    {
        strModuleName = m_visionUnit.GetVisionProcessing(nInsp)->m_strModuleName;
        strModuleGUID = m_visionUnit.GetVisionProcessing(nInsp)->m_moduleGuid;

        if (nAlignID < 0 && strModuleGUID == m_visionUnit.GetVisionAlignProcessingModuleGUID())
        {
            // Align ID를 찾았으면 CString으로 Align 안찾게
            nAlignID = nInsp;
        }
        else if (strModuleName.Find(_T("Mark_")) == 0)
        {
            // "Mark_XXXX"일 때만 Mark 검사로 인정
            vecnMarkInspID.push_back(nInsp);
        }
    }
    nNumOfMarkInsp = (long)vecnMarkInspID.size();
    if (nAlignID < 0 || nNumOfMarkInsp <= 0) // Align이 없거나 Mark 검사가 없으면 뒤에꺼 할 필요 없잖아.
        return FALSE;

    for (long nPane = 0; nPane < nPaneNum; nPane++)
    {
        m_visionUnit.SetCurrentPaneID(nPane);

        bStepResult = TRUE; // '&='연산할꺼니까 미리 TRUE로 만들기
        for (long nInsp = 0; nInsp <= nAlignID && bStepResult;
            nInsp++) // Align 까지 진행을 하는데 중간에 실패한 넘이 나오면 중단
            bStepResult &= InspectSingleVisionProcess(nInsp);

        if (bStepResult) // Align까지 다 성공 했고
        {
            nAlignPassNum++;
            for (long i = 0; i < nNumOfMarkInsp;
                i++) //Mark만 검사 하자. 중간에 실패한 넘이 나와도 계속한다. 검사 과정에서 Mark Insp 내부 변수 초기화하는 효과가 있다. 꼭 필요!!
            {
                nCurMarkID = vecnMarkInspID[i];
                bStepResult &= InspectSingleVisionProcess(nCurMarkID);
            }
            nMarkPassNum += bStepResult ? 1 : 0;
            //			if(!bStepResult)  //시간 절약 하려면 윗 줄 대신 이렇게... Align 성공한 넘에서 Mark 불량이 나오면  return FALSE로 함수 종료
            //			{
            //				vecnMarkInspID.clear();
            //				return FALSE;
            //			}
        }
    }
    vecnMarkInspID.clear();

    if (nAlignPassNum > 0
        && nAlignPassNum
            == nMarkPassNum) //Align 성공한게 1개 이상이고, Align에 성공한 Package는 모두 Mark 검사 Pass 나야 성공으로 간주
        return TRUE;

    return FALSE; //뭐가 됐던 실패
}

BOOL CDlgVisionPrimaryUI::AutoMarkTeachSeq() //kircheis_AutoTeach
{
    BOOL bPassSkipTeach = FALSE; //혹시나 해서 넣은 시퀀스
    if (AutoMarkInspSeq()
        && bPassSkipTeach) //일단 검사는 한번 해야한다(일종의 활성화). 그외에 검사해서 All Pass면 걍 Teach 안하고 넘어가는건 나중에 생각해보자 이건 일단 불리안으로 막음
        return TRUE;

    VisionInspectionMark* pVisionMarkInspection = NULL;

    BOOL bStepResult = TRUE; //본 시퀀스에서 수행하는 각 Insp 단계의 성공 여부 (0 ~ Edge Align)
    long nPaneNum = (long)m_visionUnit.getInspectionAreaInfo().m_unitIndexList.size();
    long nInspNum = m_visionUnit.GetVisionProcessingCount(); //검사 갯수
    long nAlignID = -1;
    std::vector<long> vecnMarkInspID(0); //Mark Insp의 ID 저장
    long nNumOfMarkInsp = -1; //Mark Insp의 갯수
    long nCurMarkID = -1;

    CString strModuleName, strModuleGUID;
    for (long nInsp = 0; nInsp < nInspNum; nInsp++) // Align과 Mark의 검사 ID를 먼저 찾자.
    {
        strModuleName = m_visionUnit.GetVisionProcessing(nInsp)->m_strModuleName;
        strModuleGUID = m_visionUnit.GetVisionProcessing(nInsp)->m_moduleGuid;

        if (nAlignID < 0 && strModuleGUID == m_visionUnit.GetVisionAlignProcessingModuleGUID())
        {
            // Align ID를 찾았으면 CString으로 Align 안찾게
            nAlignID = nInsp;
        }
        else if (strModuleName.Find(_T("Mark_")) == 0)
        {
            // "Mark_XXXX"일 때만 Mark 검사로 인정
            vecnMarkInspID.push_back(nInsp);
        }
    }
    nNumOfMarkInsp = (long)vecnMarkInspID.size();
    if (nAlignID < 0 || nNumOfMarkInsp <= 0) // Align이 없거나 Mark 검사가 없으면 뒤에꺼 할 필요 없잖아.
        return FALSE;

    for (long nPane = 0; nPane < nPaneNum; nPane++)
    {
        m_visionUnit.SetCurrentPaneID(nPane);

        bStepResult = TRUE; // '&='연산할꺼니까 미리 TRUE로 만들기
        for (long nInsp = 0; nInsp <= nAlignID && bStepResult;
            nInsp++) // Align 까지 진행을 하는데 중간에 실패한 넘이 나오면 중단
            bStepResult &= InspectSingleVisionProcess(nInsp);

        if (bStepResult) // Align까지 다 성공 했으면 Teaching 하자
        {
            for (long i = 0; i < nNumOfMarkInsp && bStepResult;
                i++) //Mark만 Teach하는거다. 중간에 실패한 넘이 나오면 중단
            {
                nCurMarkID = vecnMarkInspID[i];
                pVisionMarkInspection = ((VisionInspectionMark*)(m_visionUnit.GetVisionProcessing(nCurMarkID)));

                if (pVisionMarkInspection != NULL)
                {
                    pVisionMarkInspection->m_bAutoTeachMode
                        = TRUE; //Mark Inspection에 지금 AutoTeach라는 걸 알린다. 이 변수 중요 체크[헤더의 주석 확인]
                    bStepResult &= pVisionMarkInspection->DoTeach(true);
                    pVisionMarkInspection->m_bAutoTeachMode = FALSE; //종료되면 리셋
                }
            }

            if (bStepResult) //Edge Align과 Mark Teach 둘 다 성공 했으면
                bStepResult &= AutoMarkInspSeq(); //모든 Pane을 검사해보고 모두 Pass가 나야만 성공으로 간주하자

            if (bStepResult) //이물등이 Teaching 되어 다른 Package에 영향을 줄수 있으니까 모든 Package Pass가 아니면 다음 Package를 Teach하도록 하자
            {
                vecnMarkInspID.clear();
                m_visionUnit.SetCurrentPaneID(
                    nPane); // Host로 Teach영상을 보내야하는데 AutoMarkInspSeq()함수후에는 Pane 번호가 마지막으로 바뀐다. 이를 다시 Teach한 Pane으로 설정
                return TRUE;
            }
        }
    }

    vecnMarkInspID.clear();
    return FALSE; //여기에 왔다는건 뭐가 됐든 Teach 실패라는 의미
}

long CDlgVisionPrimaryUI::GetValidPaneID() //kircheis_201611XX
{
    BOOL bStepResult
        = TRUE; //본 시퀀스에서 수행하는 각 Insp 단계(0 ~ Edge Align)의 성공 여부 누적 저장 (하나라도 실패면 FALSE)
    long nPaneNum = (long)m_visionUnit.getInspectionAreaInfo().m_unitIndexList.size();
    long nInspNum = m_visionUnit.GetVisionProcessingCount(); //검사 갯수
    long nAlignID = -1; // Align의 검사 ID

    //{{//Edge Align의 검사 ID를 먼저 찾자.
    CString strModuleName, strModuleGUID;
    for (long nInsp = 0; nInsp < nInspNum; nInsp++)
    {
        strModuleName = m_visionUnit.GetVisionProcessing(nInsp)->m_strModuleName;
        strModuleGUID = m_visionUnit.GetVisionProcessing(nInsp)->m_moduleGuid;

        if (strModuleGUID == m_visionUnit.GetVisionAlignProcessingModuleGUID())
        {
            // Align ID를 찾았으면 검사 ID 저장하고 루프 정지
            nAlignID = nInsp;
            break;
        }
    }
    if (nAlignID < 0) // Align이 검사에 없으면
        return -1;
    //}}

    //{{ //각 Pane별로 Align까지의 검사를 진행, Align까지 정상적으로 수행한 첫 번째 Pane의 ID를 리턴
    for (long nPane = 0; nPane < nPaneNum; nPane++)
    {
        m_visionUnit.SetCurrentPaneID(nPane);

        bStepResult = TRUE; // '&='연산할꺼니까 미리 TRUE로 만들기
        for (long nInsp = 0; nInsp <= nAlignID && bStepResult;
            nInsp++) // Align 까지 진행을 하는데 중간에 실패한 넘이 나오면 중단
            bStepResult &= InspectSingleVisionProcess(nInsp);

        if (bStepResult) // Align까지 다 성공 했으면
            return nPane; //현재 Pane ID를 리턴
    }
    //}}

    return -1; //예까지 온거는 모두 다 실패 한거임
}