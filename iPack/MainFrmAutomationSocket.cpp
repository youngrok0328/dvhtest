//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "MainFrm.h"

//CPP_2_________________________________ This project's headers
#include "AutomationMsgDef.h"

//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../MainUiModules/VisionPrimaryUI/DlgVisionPrimaryUI.h"
#include "../ManagementModules/VisionMain/VisionMain.h"
#include "../UtilityMacroFunction.h"
#include "../VisionNeedLibrary/VisionCommon/VisionMainAgent.h"
#include "../VisionNeedLibrary/VisionCommon/VisionMainMenuAccess.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Ipvm/Gadget/SocketMessaging.h>

//CPP_5_________________________________ Standard library headers
#include <string>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum AutomationMode : long
{
    NormalMode = 0,
    AutomationMode,
};

LRESULT CMainFrame::OnConnectionUpdatedFromAutomation(WPARAM wParam, LPARAM lParam)
{
    const auto evtInfo = Ipvm::SocketMessaging::ParseEvt(wParam, lParam);

    Sleep(100);

    if (evtInfo.m_connected)
    {
        // 연결 시작
        m_visionMain.m_automationMessageSocket->Write((int32_t)MSG_V2A::AUTOMATION_MSG_VISION_CONNECT_ACK, NULL, NULL);
    }
    else if (evtInfo.m_disconnected)
    {
        // 연결 종료
    }

    return 0;
}

LRESULT CMainFrame::OnDataReceivedFromAutomation(WPARAM wParam, LPARAM lParam)
{
    const auto msgInfo = Ipvm::SocketMessaging::ParseMsg(wParam, lParam);

    const MSG_A2V nMessageID = MSG_A2V(msgInfo.m_messageIndex);
    //const long nMessageLength = msgInfo.m_messageLength;

    Sleep(100); // 바로 전달할 경우 받지 못하는 경우가 있어 sleep 추가

    switch (nMessageID)
    {
        case MSG_A2V::AUTOMATION_MSG_VISION_CONNECT: // 비전의 연결 여부 확인
        {
            //TODO::여기에 동작을 추가한다.

            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_VISION_CONNECT_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_RUN_INFO: // 비전의 데이터 검증
        {
            // 데이터를 받아서 저장한다. 해당 데이터의 위치는 수정 필요함
            MSG_A2V_AUTOMATION_MSG_RUN_INFO* data = (MSG_A2V_AUTOMATION_MSG_RUN_INFO*)msgInfo.m_message;

            //CString ma_strAutomationRecipeFilePath = data->m_strRecipeFilePath;
            //CString ma_strAutomationImageFilePath = data->m_strImageFilePath;
            //long ma_eAutomationSeqRunMode = (long)data->m_eAutomationRunMode;

            // 데이터 수령 부분
            SystemConfig::GetInstance().m_strAutomationRecipeFileName = data->m_strRecipeFileName;
            SystemConfig::GetInstance().m_strAutomationImageFilePath = data->m_strImageFilePath;
            SystemConfig::GetInstance().m_nAutomationSeqRunMode = (long)data->m_eAutomationRunMode;
            SystemConfig::GetInstance().m_nAutomationMode = AutomationMode;

            m_visionMain.m_automationMessageSocket->Write((int32_t)MSG_V2A::AUTOMATION_MSG_RUN_INFO_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_RECIPE_OPEN: // 비전의 레시피 오픈
        {
            // 해당 job 파일을 open 한다.
            CString strFilePath;
            strFilePath.Format((CString)SystemConfig::GetInstance().m_strAutomationImageFilePath + _T("\\")
                + (CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName);

            m_visionMain.OpenJobFile(strFilePath, (CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName);

            m_visionMain.m_automationMessageSocket->Write((int32_t)MSG_V2A::AUTOMATION_MSG_RECIPE_OPEN_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_RECIPE_SAVE_ANOTHER_NAME: // 비전에서 레시피 save as 진행
        {
            CString strAutomationFolder;
            strAutomationFolder.Format(
                _T("%s%s"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)), _T("\\AutomationTemp\\"));

            Ipvm::CreateDirectories(LPCTSTR(strAutomationFolder));

            // 해당 job 파일을 Save as 한다.
            m_visionMain.SaveJobFile((CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName,
                VisionMainAgent::SaveJobMode::SaveOnly);

            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_RECIPE_SAVE_ANOTHER_NAME_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_RECIPE_OPEN_ANOTHER_NAME: // 비전의 레시피 reopen 진행
        {
            // 이미지를 open 한다. 해당 과정에서 이미 세이브한 경로로 reopen 한다. 기존 recipe save as 에서도 동일하게 작동하지만 다른 장비 호환성을 위해 seq에 포함
            m_visionMain.OpenJobFile(DynamicSystemPath::get(DefineFolder::Log) + _T("\\AutomationTemp\\")
                    + REMOVE_EXT_FROM_FILENAME((CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName),
                (CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName);

            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_RECIPE_OPEN_ANOTHER_NAME_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_TEXT_RECIPE_EXPORT: // 비전의 레시피를 text 형식으로 export 한다.
        {
            // text recipe로 Export한다.
            std::vector<CString> vecStrTxtRecipe = m_visionMain.ExportRecipeToText();
            long nSize = (long)vecStrTxtRecipe.size();

            // txt recipe를 저장한다.
            CFileFind ff;

            CString pathName(DynamicSystemPath::get(DefineFolder::Log) + _T("\\AutomationTemp\\")
                + SystemConfig::GetInstance().m_strAutomationRecipeFileName + _T("_TextRecipe.csv"));

            // 저장

            FILE* fp = nullptr;
            _tfopen_s(&fp, pathName, _T("w"));

            if (fp == nullptr)
            {
                return false;
            }

            for (int i = 0; i < nSize; i++)
            {
                std::string strTemp = std::string(CT2CA(vecStrTxtRecipe[i]));

                CStringA str(strTemp.c_str());
                fprintf(fp, str);

                str.Format("\n");
                fprintf(fp, str);
            }

            fclose(fp);

            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_TEXT_RECIPE_EXPORT_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_BATCH_INSPECTION_UI_OPEN: // Batch inspection ui를 open한다.
        {
            //Batch inspection UI로 현재 화면을 수정한다.
            OnJobBatchinspection();

            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_BATCH_INSPECTION_UI_OPEN_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_BATCH_INSPECTION_IMAGE_ADD: // Batch inspection의 image를 load 한다.
        {
            // 해당 경로에 있는 bmp 파일을 모두 읽어서 vector화 시킨다. > 임시로 UI에 변수 추가해 접근
            CFileFind file;
            BOOL fileFounded
                = file.FindFile((CString)SystemConfig::GetInstance().m_strAutomationImageFilePath + _T("\\*.bmp"));
            CString strFolderItem, strFileExt, strTempString;
            std::vector<CString> vecstrImgList;

            while (fileFounded)
            {
                fileFounded = file.FindNextFile();
                strFolderItem = file.GetFilePath();

                if (!file
                        .IsDots()) // bmp 파일 리스트를 CString Vector에 넣어준다. > 해당 vector를 활용해 이미지를 연다.
                {
                    vecstrImgList.push_back(strFolderItem);
                }
            }

            // 해당 이미지를 추가한다.
            m_visionMain.m_menuAccessUI->callImagesOpen(vecstrImgList);

            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_BATCH_INSPECTION_IMAGE_ADD_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::
            AUTOMATION_MSG_BATCH_INSPECTION_RUN: // Batch inspection을 돌린다. > Correlation mode 및 Pressure mode를 구분하여 돌려야 한다.
        {
            // Run inspection을 진행한다 > 임시로 UI에 변수 추가해 접근
            m_visionMain.m_menuAccessUI->callInspection(SystemConfig::GetInstance().m_nAutomationSeqRunMode);
            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_BATCH_INSPECTION_RUN_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::AUTOMATION_MSG_BATCH_INSPECTION_END_ACK: // Inspection이 종료됨을 알린다.
        {
            // Manual inspection이 완료된 시점에서 할 작업을 추가한다.
        }
        break;
        case MSG_A2V::AUTOMATION_MSG_BATCH_INSPECTION_UI_CLOSE: // Batchi inspection UI를 닫는다.
        {
            //Batch inspection UI를 닫는다.
            OnInspectionQuit();

            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_BATCH_INSPECTION_UI_CLOSE_ACK, NULL, NULL);
        }
        break;

        case MSG_A2V::
            AUTOMATION_MSG_VISION_SW_TERMINATE: // iPack을 종료한다. 해당 프로세스 중지가 원활하게 이뤄지지 않을 경우에는 강제종료를 진행한다.
        {
            // ipack을 종료한다. 해당 종료가 원활하게 이뤄지지 않으면 강제 종료
            m_visionMain.m_automationMessageSocket->Write(
                (int32_t)MSG_V2A::AUTOMATION_MSG_VISION_SW_TERMINATE_ACK, NULL, NULL); // 메세지를 먼저 보낸다.

            Sleep(10000); // 너무 일찍 죽이면 통신을 automation이 받기 전에 종료됨

            quick_exit(0); // 프로세스 중지
            ::PostQuitMessage(WM_QUIT); // 안되면 강제 중지 > 메모리 누수 가능성
        }
        break;

        default:
        {
            AfxMessageBox(_T("Undefined meaasge recieved \nMessage ID : %d"),
                (long)nMessageID); // 메세지 이상한거 받으면 일단 경고 출력 > 해당 내용은 추후 삭제 필요?
        }

            return 0;
    }

    return 0;
}