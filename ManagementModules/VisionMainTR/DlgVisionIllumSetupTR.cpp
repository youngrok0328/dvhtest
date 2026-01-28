//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionIllumSetupTR.h"

//CPP_2_________________________________ This project's headers
#include "VisionMainTR.h"

//CPP_3_________________________________ Other projects' headers
#include "../../HardwareModules/dPI_Framegrabber/FrameGrabber.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h" //kircheis_WB
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedComponent/Persistence/IniHelper.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspFrameIndex.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image32u.h>
#include <Ipvm/Base/Image8u3.h> //kircheis_WB
#include <Ipvm/Widget/ProfileView.h>

//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_PROPERTY_GRID 101
#define UM_GRAB_END_EVENT (WM_USER + 1054)

//CPP_7_________________________________ Implementation body
//
enum enumGridItem
{
    enumGridItem_TurnOffLight = 1,
    enumGridItem_FirstID,
};

IMPLEMENT_DYNAMIC(DlgVisionIllumSetupTR, CDialog)

DlgVisionIllumSetupTR::DlgVisionIllumSetupTR(
    VisionMainTR& visionMain, VisionUnit& visionUnit, CWnd* pParent, const enSideVisionModule i_eSideVisionModule)
    : CDialog(DlgVisionIllumSetupTR::IDD, pParent)
    , m_visionMain(visionMain)
    , m_visionUnit(visionUnit)
    , m_imageView(nullptr)
    , m_propertyGrid(nullptr)
    , m_profileView(nullptr)
    , m_grabImage(new Ipvm::Image8u)
    , m_histogram(new Ipvm::Image32u(256, 1))
    , m_illumView(LED_ILLUM_FRAME_MAX)
    , main_type(m_visionMain.m_mainType)
{
    UNREFERENCED_PARAMETER(i_eSideVisionModule);
}

DlgVisionIllumSetupTR::~DlgVisionIllumSetupTR()
{
    delete m_profileView;
    delete m_propertyGrid;
    delete m_imageView;
    delete m_grabImage;
    delete m_histogram;
}

void DlgVisionIllumSetupTR::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ILLUM_VIEW, m_illumView);
}

BEGIN_MESSAGE_MAP(DlgVisionIllumSetupTR, CDialog)
ON_MESSAGE(UM_GRAB_END_EVENT, &DlgVisionIllumSetupTR::OnGrabEndEvent)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_WM_DESTROY()
END_MESSAGE_MAP()

// DlgVisionIllumSetupTR 메시지 처리기입니다.
BOOL DlgVisionIllumSetupTR::OnInitDialog()
{
    CDialog::OnInitDialog();

    //------------------------------------------------------------------------------------
    // 검사항목이 없어져서 바로 조명이 사라지는 일이 없게 임시로 Pacakge Spec에다가
    // 모든 Frame을 부여해 놓자. 정말 해당 Frame을 지우고 싶다면 끄고 지우면 되니까
    //------------------------------------------------------------------------------------

    for (long nSpecID = 0; nSpecID < m_visionUnit.GetVisionProcessingCount(); nSpecID++)
    {
        auto* visionProcessing = m_visionUnit.GetVisionProcessing(nSpecID);
        std::vector<long>& vecnIllumInfo = visionProcessing->GetImageFrameList();

        if (visionProcessing->m_moduleGuid != _VISION_INSP_GUID_PACKAGE_SPEC)
            continue;

        vecnIllumInfo.clear();
        for (long illumIndex = 0; illumIndex < illumJob().getTotalFrameCount(); illumIndex++)
        {
            vecnIllumInfo.push_back(illumIndex);
        }
    }

    SetGeneralVisionIllumUI();

    SetIllumiParameter(-1);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgVisionIllumSetupTR::SetGeneralVisionIllumUI()
{
    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(0, 0, rtParentClient.Width(), rtParentClient.Height(), FALSE);

    CRect rtClient;
    GetClientRect(rtClient);
    rtClient.DeflateRect(5, 5);

    GetDlgItem(IDC_STATIC_ILLUM_RGB)->MoveWindow(::GetContentSubArea(rtClient, 0, 1, 6, 0, 2, 5));

    m_propertyGrid = new CXTPPropertyGrid();
    m_propertyGrid->Create(::GetContentSubArea(rtClient, 1, 2, 6, 0, 3, 5), this, IDC_PROPERTY_GRID);
    m_propertyGrid->EnableWindow(m_visionUnit.m_systemConfig.m_nCurrentAccessMode < _ENGINEER ? FALSE : TRUE);

    m_illumView.MoveWindow(::GetContentSubArea(rtClient, 0, 1, 3, 3, 5, 5));
    m_illumView.setCallback_changedSelectFrame(this, callBack_changedSelectFrame);
    m_illumView.setCallback_changedSelectItem(this, callBack_changedSelectItem);
    m_illumView.setCallback_changedItemFrame(this, callBack_changedItemFrame);
    m_illumView.setCallback_deletedFrame(this, callBack_deletedFrame);
    m_illumView.setCallback_insertedFrame(this, callBack_insertedFrame);
    m_nCurrentFrameIndex = -1;

    updateSpecToIllumView();

    GetDlgItem(IDC_STATIC_SIDE_FRONT_ILLUM)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_SIDE_REAR_ILLUM)->ShowWindow(SW_HIDE);

    m_imageView = new ImageViewEx(GetSafeHwnd(), Ipvm::FromMFC(::GetContentSubArea(rtClient, 1, 3, 3, 0, 1, 1)), 0,
        enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    Ipvm::Rect32s roi(
        0, 0, FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    roi.DeflateRect(roi.Width() / 4, roi.Height() / 4);

    m_imageView->ROISet(_T("Histogram area"), _T("Histogram area"), roi, RGB(255, 0, 0));
    m_imageView->ROIShow();
    m_imageView->RegisterCallback_ROIChangeEnd(GetSafeHwnd(), this, callBack_roiChanged);

    m_profileView
        = new Ipvm::ProfileView(GetSafeHwnd(), Ipvm::FromMFC(::GetContentSubArea(rtClient, 0, 1, 6, 2, 3, 5)));
    m_profileView->SetColor_GraphBackground(RGB(0, 0, 0));
    m_profileView->SetColor_GridBackground(RGB(0, 0, 0));
    m_profileView->SetColor_GridTopHorizontalAxisLine(RGB(0, 0, 0));
    m_profileView->SetColor_GridBottomHorizontalAxisLine(RGB(128, 128, 128));
    m_profileView->SetColor_GridLeftVerticalAxisLine(RGB(128, 128, 128));
    m_profileView->SetColor_GridRightVerticalAxisLine(RGB(0, 0, 0));
    m_profileView->SetColor_GridHorizontalLine(RGB(0, 0, 0));
    m_profileView->SetColor_GridVerticalLine(RGB(0, 0, 0));
    m_profileView->SetColor_HorizontalScaleText(RGB(255, 255, 255));
    m_profileView->SetColor_VerticalScaleText(RGB(255, 255, 255));

    ShowWindow(SW_SHOW);
}

BOOL DlgVisionIllumSetupTR::UpdateIllumChSetupUI(long nFrame, BOOL bRecommendSetForColor, long nModifyCh) //kircheis_WB
{
    UNREFERENCED_PARAMETER(bRecommendSetForColor);
    UNREFERENCED_PARAMETER(nModifyCh);

    m_propertyGrid->SetRedraw(FALSE);
    m_propertyGrid->ResetContent();

    if (nFrame < 0 || illumJob().getTotalFrameCount() <= nFrame)
    {
        m_propertyGrid->HighlightChangedItems(TRUE);
        m_propertyGrid->SetRedraw(TRUE);
        m_propertyGrid->Refresh();
        return FALSE;
    }

    static const float fLowLimit = 0.3f;
    static const float fToleranceRange = 0.005f;

    if (auto* category = m_propertyGrid->AddCategory(_T("Illuminations")))
    {
        LPCTSTR channelNames[] = {
            _T("CH01"),
            _T("CH02"),
            _T("CH03"),
            _T("CH04"),
            _T("CH05"),
            _T("CH06"),
            _T("CH07"),
            _T("CH08"),
            _T("CH09"),
            _T("CH10"),
            _T("CH11"),
            _T("CH12"),
            _T("CH13"),
            _T("CH14"),
            _T("CH15"),
            _T("CH16"),
        };

        static_assert(sizeof(channelNames) / sizeof(LPCTSTR) == LED_ILLUM_CHANNEL_MAX, "Array size error");

        COLORREF channelColors[LED_ILLUM_CHANNEL_MAX] = { 0, };

        if (main_type == enTRVisionmodule::TR_VISIONMODULE_INPOCKET)
        {
            COLORREF channelColors_Inpocket[] = {
                RGB(255, 0, 0),
                RGB(0, 0, 255),
                RGB(255, 0, 0),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
            };

            memcpy(channelColors, channelColors_Inpocket, sizeof(channelColors_Inpocket));
        }
        else
        {
            COLORREF channelColors_OTI[] = {
                RGB(255, 0, 0),
                RGB(0, 0, 255),
                RGB(0, 0, 0),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
                RGB(128, 128, 128),
            };

            memcpy(channelColors, channelColors_OTI, sizeof(channelColors_OTI));
        }

        static_assert(sizeof(channelColors) / sizeof(COLORREF) == LED_ILLUM_CHANNEL_MAX, "Array size error");

        CString strChannelName(""); //kircheis_WB

        category->AddChildItem(
            new CXTPPropertyGridItem(_T("Name"), illumJob().getIllumName(nFrame), illumJob().getIllumNamePtr(nFrame)));

        const long nChannelMaxNum = SystemConfig::GetInstance().GetMaxIllumChannelCount(enTRVisionmodule(main_type));
        for (long idx = 0; idx < nChannelMaxNum; idx++)
        {
            strChannelName.Format(_T("%s"), channelNames[idx]);

            if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                    (LPCTSTR)strChannelName /*channelNames[idx]*/, illumJob().getIllumChannel_ms(nFrame, idx),
                    _T("%.3lf ms"), illumJob().getIllumChannel_ms(nFrame, idx), 0.f, 65.f, 0.002f)))
            {
                item->SetID(enumGridItem_FirstID + idx);

                if (auto* metric = item->GetMetrics(TRUE))
                {
                    metric->m_clrFore = channelColors[idx];
                }
                if (auto* metric = item->GetMetrics(FALSE))
                {
                    metric->m_clrFore = channelColors[idx];
                }
            }
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Turn off light"), TRUE, FALSE)))
        {
            item->SetID(enumGridItem_TurnOffLight);
        }

        category->Expand();

        strChannelName.Empty();
    }

    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->SetRedraw(TRUE);
    m_propertyGrid->Refresh();

    return TRUE;
}

void DlgVisionIllumSetupTR::SetIllumiParameter(long nFrame)
{
    nFrame = min(nFrame, illumJob().getTotalFrameCount());

    if (UpdateIllumChSetupUI(nFrame, illumJob().m_2D_colorFrame) == FALSE) //kircheis_WB
        return;

    if (m_visionUnit.m_systemConfig.IsHardwareExist())
    {
        SyncController::GetInstance().SetIllumiParameter(
            nFrame, illumJob().getIllum(nFrame), true, false, 0, enTRVisionmodule(main_type));
        SyncController::GetInstance().TurnOnLight(nFrame, enTRVisionmodule(main_type));
        FrameGrabber::GetInstance().live_on(GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enTRVisionmodule(main_type));

        //long nProbePosZ = illumJob().isHighFrame(nFrame) ? _MULTI_GRAB_POS_HIGH : _MULTI_GRAB_POS_NORMAL;

        //m_visionMain.iPIS_Send_MultiGrabProbePosZ(nProbePosZ);
    }
    else
    {
        long nFrameNum = (long)m_visionUnit.getImageLot().GetImageFrameCount();
        // SDY 설정시 Job의 셋업된 이미지보다 뒤의 Frame의 조명값을 수정하면 마지막 프레임의 조명값이 변경되는 문제 수정
        // {
        int nImageFrameNum; // 출력용 이미지의 프레임
        if (nFrame >= nFrameNum)
        {
            nImageFrameNum
                = nFrameNum - 1; // 현재 프레임이 최대 프레임보다 많을 경우 마지막 프레임의 이미지를 출력한다.
        }
        else
        {
            nImageFrameNum = nFrame; // 해당 프레임은 이미지가 있으므로 해당 이미지를 출력한다.
        }
        m_imageView->SetImage(
            m_visionUnit.getImageLot().GetImageFrame(nImageFrameNum, enSideVisionModule::SIDE_VISIONMODULE_FRONT)); // 이미지 출력
        // }
        calcHistogram(&m_imageView->GetImage_8u_C1());
    }

    m_nCurrentFrameIndex = nFrame;
}

void DlgVisionIllumSetupTR::updateSpecToIllumView()
{
    m_illumView.resetData();
    m_illumView.addFrameGroup(0, _T("Normal Position"), RGB(45, 79, 119), RGB(255, 255, 255));

    //if (SystemConfig::GetInstance().GetHandlerType() != HANDLER_TYPE_380BRIDGE)
    //{
    //    m_illumView.addFrameGroup(1, _T("High Position"), RGB(164, 201, 221), RGB(45, 79, 119));
    //}

    long procssingCount = m_visionUnit.GetVisionProcessingCount();

    for (long itemIndex = 0; itemIndex < procssingCount; itemIndex++)
    {
        auto* visionProcessing = m_visionUnit.GetVisionProcessing(itemIndex);
        auto& vecnIllumInfo = visionProcessing->GetImageFrameList();

        // 만약 Frame 설정이 되어있지 않다면 강제로 0번 Frame을 설정해주도록 한다.
        if ((long)vecnIllumInfo.size() <= 0)
            vecnIllumInfo.push_back(0);

        bool onlyOneFrameCanBeUsed = true;

        if (visionProcessing->m_moduleGuid == _VISION_INSP_GUID_PACKAGE_SPEC
            || visionProcessing->m_moduleGuid == _VISION_INSP_GUID_FOV
            || visionProcessing->m_moduleGuid == _VISION_INSP_GUID_ALIGN_2D
            || visionProcessing->m_moduleGuid == _VISION_INSP_GUID_BGA_BALL_2D
            || visionProcessing->m_moduleGuid == _VISION_INSP_GUID_PASSIVE_2D
            || visionProcessing->m_moduleGuid == _VISION_INSP_GUID_KOZ_2D || //kircheis_KOZ
            visionProcessing->m_moduleGuid == _VISION_INSP_GUID_2D_MATRIX)
        {
        }

        if (visionProcessing->m_moduleGuid == _VISION_INSP_GUID_PACKAGE_SPEC)
            onlyOneFrameCanBeUsed = false;
        {
            // Total Frame Count와 vecnIllumInfo 가 안맞는 경우에 대해
            // IllumControl2D가 제대로 고려가 안되었다.
            // 일단 그런 경우가 Package Spec 사용 Frame을 모두 켜줘서 안생기게 여기다 땜빵후
            // 나중에 잘 정리하자

            vecnIllumInfo.clear();
            for (long frameIndex = 0; frameIndex < illumJob().getTotalFrameCount(); frameIndex++)
            {
                vecnIllumInfo.push_back(frameIndex);
            }
        }

        m_illumView.addItem(itemIndex, visionProcessing->m_strModuleName, onlyOneFrameCanBeUsed);

        for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
        {
            long frameIndex = vecnIllumInfo[illumID];
            m_illumView.addItemFrame(itemIndex, frameIndex);
        }
    }

    for (long frameIndex = 0; frameIndex < LED_ILLUM_FRAME_MAX; frameIndex++)
    {
        m_illumView.setFrameGroup(frameIndex, illumJob().isHighFrame(frameIndex) ? 1 : 0);
    }

    m_illumView.setEditMode(m_visionUnit.m_systemConfig.m_nCurrentAccessMode >= _ENGINEER);
}

IllumInfo2D& DlgVisionIllumSetupTR::illumJob()
{
    return m_visionMain.m_illum2D;
}

void DlgVisionIllumSetupTR::callBack_roiChanged(
    void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(keyLength);

    auto& image = ((DlgVisionIllumSetupTR*)userData)->m_imageView->GetImage_8u_C1();
    ((DlgVisionIllumSetupTR*)userData)->calcHistogram(&image);
}

void DlgVisionIllumSetupTR::callBack_changedSelectFrame(LPVOID userData, long realFrameIndex)
{
    ((DlgVisionIllumSetupTR*)userData)->callBack_changedSelectFrame(realFrameIndex);
}

void DlgVisionIllumSetupTR::callBack_changedSelectItem(LPVOID userData, long itemID)
{
    ((DlgVisionIllumSetupTR*)userData)->callBack_changedSelectItem(itemID);
}

void DlgVisionIllumSetupTR::callBack_changedItemFrame(LPVOID userData, long itemID, long realFrameIndex, bool checked)
{
    ((DlgVisionIllumSetupTR*)userData)->callBack_changedItemFrame(itemID, realFrameIndex, checked);
}

void DlgVisionIllumSetupTR::callBack_deletedFrame(LPVOID userData, long realFrameIndex, bool isBackup)
{
    ((DlgVisionIllumSetupTR*)userData)->callBack_deletedFrame(realFrameIndex, isBackup);
}

void DlgVisionIllumSetupTR::callBack_insertedFrame(LPVOID userData, long groupID, long realFrameIndex, bool isBackup)
{
    ((DlgVisionIllumSetupTR*)userData)->callBack_insertedFrame(groupID, realFrameIndex, isBackup);
}

void DlgVisionIllumSetupTR::callBack_changedSelectFrame(long realFrameIndex)
{
    SetIllumiParameter(realFrameIndex);
}

void DlgVisionIllumSetupTR::callBack_changedSelectItem(long /*itemID*/)
{
}

void DlgVisionIllumSetupTR::callBack_changedItemFrame(long itemID, long realFrameIndex, bool checked)
{
    long procssingCount = m_visionUnit.GetVisionProcessingCount();

    if (itemID < procssingCount)
    {
        auto* visionProcessing = m_visionUnit.GetVisionProcessing(itemID);
        auto& vecnIllumInfo = visionProcessing->GetImageFrameList();

        long illumDataIndex = -1;

        for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
        {
            if (vecnIllumInfo[illumID] == realFrameIndex)
            {
                illumDataIndex = illumID;
                break;
            }
        }

        if (checked)
        {
            if (illumDataIndex < 0)
            {
                vecnIllumInfo.push_back(realFrameIndex);
                std::sort(vecnIllumInfo.begin(), vecnIllumInfo.end());
            }
        }
        else
        {
            if (illumDataIndex >= 0)
            {
                vecnIllumInfo.erase(vecnIllumInfo.begin() + illumDataIndex);
            }
        }
    }

    if (realFrameIndex < illumJob().getTotalFrameCount())
    {
        SetIllumiParameter(realFrameIndex);
    }
}

void DlgVisionIllumSetupTR::callBack_deletedFrame(long realFrameIndex, bool isBackup)
{
    long procssingCount = m_visionUnit.GetVisionProcessingCount();

    if (isBackup)
    {
        m_backupInfo_name = illumJob().getIllumName(realFrameIndex);
        m_backupInfo_items.clear();
        m_backupInfo_items_inspFrames.clear();

        for (long n = 0; n < 16; n++)
        {
            m_backupInfo_illuminations_ms[n] = illumJob().getIllumChannel_ms(realFrameIndex, n);
        }
    }

    for (long itemIndex = 0; itemIndex < procssingCount; itemIndex++)
    {
        auto* visionProcessing = m_visionUnit.GetVisionProcessing(itemIndex);
        auto& vecnIllumInfo = visionProcessing->GetImageFrameList();

        long eraseIllumIndex = -1;

        for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
        {
            if (vecnIllumInfo[illumID] == realFrameIndex)
            {
                eraseIllumIndex = illumID;

                if (isBackup)
                {
                    m_backupInfo_items[itemIndex] = TRUE;
                }
            }
        }

        auto& inspFrameMap = visionProcessing->getInspFrameMap();

        for (auto& inspFrame : inspFrameMap)
        {
            if (isBackup)
            {
                if (inspFrame.second->getFrameIndex() == realFrameIndex)
                {
                    m_backupInfo_items_inspFrames[itemIndex][inspFrame.first] = inspFrame.second->useAllFrame();
                }
            }

            // 삭제로 인한 Frame Index가 영향 받을 검사 Frame Index들을 변경해 준다

            if (inspFrame.second->useAllFrame())
            {
                if (inspFrame.second->getFrameIndex() > realFrameIndex)
                {
                    inspFrame.second->setFrameIndex(inspFrame.second->getFrameIndex() - 1);
                }
            }
            else
            {
                if (eraseIllumIndex >= 0)
                {
                    if (inspFrame.second->getFrameListIndex() > eraseIllumIndex)
                    {
                        inspFrame.second->setFrameListIndex(inspFrame.second->getFrameListIndex() - 1);
                    }
                }
            }
        }

        if (eraseIllumIndex >= 0)
        {
            vecnIllumInfo.erase(vecnIllumInfo.begin() + eraseIllumIndex);
        }

        for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
        {
            if (vecnIllumInfo[illumID] > realFrameIndex)
            {
                vecnIllumInfo[illumID]--;
            }
        }
    }

    illumJob().eraseIllum(realFrameIndex);

    inspFrameInvalidCheck();
}

void DlgVisionIllumSetupTR::callBack_insertedFrame(long groupID, long realFrameIndex, bool isBackup)
{
    long procssingCount = m_visionUnit.GetVisionProcessingCount();

    if (isBackup)
    {
        illumJob().insertFrame(realFrameIndex, (groupID == 1), m_backupInfo_name);
        for (long n = 0; n < 16; n++)
        {
            illumJob().getIllumChannel_ms(realFrameIndex, n) = m_backupInfo_illuminations_ms[n];
        }
    }
    else
    {
        illumJob().insertFrame(realFrameIndex, false, _T(""));
    }

    for (long itemIndex = 0; itemIndex < procssingCount; itemIndex++)
    {
        auto* visionProcessing = m_visionUnit.GetVisionProcessing(itemIndex);
        auto& vecnIllumInfo = visionProcessing->GetImageFrameList();

        auto& inspFrameMap = visionProcessing->getInspFrameMap();

        long insertIllumIndex = -1;

        if (isBackup && m_backupInfo_items.find(itemIndex) != m_backupInfo_items.end())
        {
            insertIllumIndex = long(vecnIllumInfo.size()); // 없으면 마지막 위치에 Insert

            // 검사를 사용하는 Frame 이 삽입되었다
            for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
            {
                if (vecnIllumInfo[illumID] >= realFrameIndex)
                {
                    insertIllumIndex = illumID;
                    break;
                }
            }
        }

        for (auto& inspFrame : inspFrameMap)
        {
            // 삽입으로 인한 Frame Index가 영향 받을 검사 Frame Index들을 변경해 준다

            if (inspFrame.second->useAllFrame())
            {
                if (inspFrame.second->getFrameIndex() >= realFrameIndex)
                {
                    inspFrame.second->setFrameIndex(inspFrame.second->getFrameIndex() + 1);
                }
            }
            else
            {
                if (insertIllumIndex >= 0)
                {
                    if (inspFrame.second->getFrameListIndex() >= insertIllumIndex)
                    {
                        inspFrame.second->setFrameListIndex(inspFrame.second->getFrameListIndex() + 1);
                    }
                }
            }
        }

        if (isBackup)
        {
            for (auto& inspFrame : m_backupInfo_items_inspFrames[itemIndex])
            {
                auto* inspFrameIndex = visionProcessing->findInspFrame(inspFrame.first);
                if (inspFrameIndex != nullptr)
                {
                    if (inspFrameIndex->useAllFrame())
                    {
                        inspFrameIndex->setFrameIndex(realFrameIndex);
                    }
                    else
                    {
                        inspFrameIndex->setFrameListIndex(insertIllumIndex);
                    }
                }
            }
        }

        for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
        {
            if (vecnIllumInfo[illumID] >= realFrameIndex)
            {
                vecnIllumInfo[illumID]++;
            }
        }

        if (insertIllumIndex >= 0)
        {
            vecnIllumInfo.insert(vecnIllumInfo.begin() + insertIllumIndex, realFrameIndex);
        }

        std::sort(vecnIllumInfo.begin(), vecnIllumInfo.end());
    }
}

void DlgVisionIllumSetupTR::inspFrameInvalidCheck()
{
    // 혹시 검사 프레임중 프레임 삭제등의 이유로 선택이 안된 것이 있다면 재정리해 주자

    long procssingCount = m_visionUnit.GetVisionProcessingCount();
    for (long itemIndex = 0; itemIndex < procssingCount; itemIndex++)
    {
        auto* visionProcessing = m_visionUnit.GetVisionProcessing(itemIndex);

        auto& inspFrameMap = visionProcessing->getInspFrameMap();

        for (auto& inspFrame : inspFrameMap)
        {
            if (inspFrame.second->useAllFrame())
            {
                if (inspFrame.second->getFrameIndex() < 0)
                {
                    inspFrame.second->setFrameIndex(0);
                }
            }
            else
            {
                if (inspFrame.second->getFrameListIndex() < 0)
                {
                    inspFrame.second->setFrameListIndex(0);
                }
            }
        }
    }
}

void DlgVisionIllumSetupTR::calcHistogram(const Ipvm::Image8u* image)
{
    Ipvm::Rect32s roi;

    m_imageView->ROIGet(_T("Histogram area"), roi);

    Ipvm::Rect32s validRoi = roi & Ipvm::Rect32s(*image);

    if (validRoi.IsRectEmpty())
    {
        m_histogram->FillZero();
    }
    else
    {
        Ipvm::ImageProcessing::MakeHistogram(*image, validRoi, *m_histogram);
    }

    std::vector<Ipvm::Point32r2> data(256);

    double sum = 0.;

    for (long idx = 0; idx < 256; idx++)
    {
        data[idx].m_x = (float)idx;
        data[idx].m_y = (float)(*m_histogram->GetMem(idx, 0));

        sum += data[idx].m_x * data[idx].m_y;
    }

    if (validRoi.Width() * validRoi.Height() > 0)
    {
        sum /= validRoi.Width() * validRoi.Height();
    }

    m_profileView->SetData(0, &data[0], 256, -FLT_MAX, RGB(255, 255, 255), RGB(200, 200, 200));

    CString str;
    str.Format(_T("Mean : %.2lf"), sum);

    m_imageView->WindowOverlayClear();
    m_imageView->WindowOverlayAdd(Ipvm::Point32s2(0, 20), str, RGB(255, 0, 0), 32);
    m_imageView->WindowOverlayShow();

    str.Empty();
}

LRESULT DlgVisionIllumSetupTR::OnGrabEndEvent(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    FrameGrabber::GetInstance().get_live_image(*m_grabImage);

    m_imageView->SetImage(*m_grabImage);

    calcHistogram(m_grabImage);

    return 1L;
}

LRESULT DlgVisionIllumSetupTR::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        const long channel = item->GetID() - enumGridItem_FirstID;

        auto* value = dynamic_cast<XTPPropertyGridItemCustomFloat<float>*>(item);

        if (item->GetID() == enumGridItem_TurnOffLight)
        {
            FrameGrabber::GetInstance().live_off();
            SyncController::GetInstance().TurnOffLight(enTRVisionmodule(main_type));
        }
        else if (value)
        {
            SyncController::GetInstance().SetIllumiParameter(m_nCurrentFrameIndex, channel,
                illumJob().getIllumChannel_ms(m_nCurrentFrameIndex, channel), true, false, 0,
                enTRVisionmodule(main_type));
            SyncController::GetInstance().TurnOnLight(m_nCurrentFrameIndex, enTRVisionmodule(main_type));

            if (SystemConfig::GetInstance().m_bHardwareExist
                && FrameGrabber::GetInstance().IsLive() == FALSE) //kircheis_WB
                FrameGrabber::GetInstance().live_on(
                    GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enTRVisionmodule(main_type));

            //long nProbePosZ
            //    = illumJob().isHighFrame(m_nCurrentFrameIndex) ? _MULTI_GRAB_POS_HIGH : _MULTI_GRAB_POS_NORMAL;

            //m_visionMain.iPIS_Send_MultiGrabProbePosZ(nProbePosZ);
            UpdateIllumChSetupUI(m_nCurrentFrameIndex, illumJob().m_2D_colorFrame, channel); //kircheis_WB
        }
    }

    return 0;
}

void DlgVisionIllumSetupTR::OnDestroy()
{
    FrameGrabber::GetInstance().live_off();
    SyncController::GetInstance().TurnOffLight(enTRVisionmodule(main_type));

    CDialog::OnDestroy();
}

BOOL DlgVisionIllumSetupTR::callJobOpen()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strFileType;
    CString strFileExtension;

    strFileType = _T("illumination 2D file format(*.i2D)|*.i2D|all files(*.*)|*.*|");
    strFileExtension = _T("*.i2D");

    CFileDialog dlg(
        TRUE, strFileExtension, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, strFileType);

    if (dlg.DoModal() != IDOK)
        return FALSE;

    CString filePath = dlg.GetPathName();
    auto illumLoad = illumJob();

    IllumLink(filePath, false, illumLoad);

    if (illumLoad.getTotalFrameCount() != illumJob().getTotalFrameCount())
    {
        CString message;
        message.Format(_T("The number of frames is not the same. (Current : %d, File : %d)"),
            illumJob().getTotalFrameCount(), illumLoad.getTotalFrameCount());

        MessageBox(message);

        message.Empty();

        return false;
    }

    illumJob() = illumLoad;

    updateSpecToIllumView();
    SetIllumiParameter(-1);

    strFileType.Empty();
    strFileExtension.Empty();
    filePath.Empty();

    return TRUE;
}

BOOL DlgVisionIllumSetupTR::callJobSave()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strFileType;
    CString strFileExtension;

    strFileType = _T("illumination 2D file format(*.i2D)|*.i2D|all files(*.*)|*.*|");
    strFileExtension = _T("*.i2D");

    CFileDialog dlg(
        FALSE, strFileExtension, NULL, OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, strFileType);

    if (dlg.DoModal() != IDOK)
        return FALSE;

    CString filePath = dlg.GetPathName();

    IllumLink(filePath, true, illumJob());

    strFileType.Empty();
    strFileExtension.Empty();
    filePath.Empty();

    return TRUE;
}

void DlgVisionIllumSetupTR::IllumLink(LPCTSTR filePath, bool save, IllumInfo2D& io_illum)
{
    long frameCount = io_illum.getTotalFrameCount();

    IniHelper::Link(save, filePath, _T("INFORMATION"), _T("Frame Count"), &frameCount);
    if (!save)
    {
        io_illum.resetInfo();
        for (long n = 0; n < frameCount; n++)
        {
            io_illum.insertFrame(io_illum.getTotalFrameCount(), false, _T(""));
        }
    }

    for (long frameIndex = 0; frameIndex < frameCount; frameIndex++)
    {
        auto& frameIllum = io_illum.getIllum(frameIndex);
        CString category;
        category.Format(_T("Frame%d"), frameIndex + 1);

        CString name = io_illum.getIllumName(frameIndex);
        IniHelper::Link(save, filePath, category, _T("Name"), &name);

        if (!save)
        {
            io_illum.setIllumName(frameIndex, name);
        }

        for (long channel = 0; channel < LED_ILLUM_CHANNEL_MAX; channel++)
        {
            CString key;
            key.Format(_T("Channel%d"), channel + 1);

            IniHelper::Link(save, filePath, category, key, &frameIllum[channel]);
        }
    }
}