//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionIllumSetup2D.h"

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
    enumGridItem_UseColorFrame = 1,
    enumGridItem_TurnOffLight,
    enumGridItem_GrabMakeColor, //kircheis_WB
    enumGridItem_FirstID,
};

IMPLEMENT_DYNAMIC(DlgVisionIllumSetup2D, CDialog)

DlgVisionIllumSetup2D::DlgVisionIllumSetup2D(
    VisionMainTR& visionMain, VisionUnit& visionUnit, CWnd* pParent, const enSideVisionModule i_eSideVisionModule)
    : CDialog(DlgVisionIllumSetup2D::IDD, pParent)
    , m_visionMain(visionMain)
    , m_visionUnit(visionUnit)
    , m_imageView(nullptr)
    , m_imageViewRearSide(nullptr)
    , m_propertyGrid(nullptr)
    , m_profileView(nullptr)
    , m_grabImage(new Ipvm::Image8u)
    , m_histogram(new Ipvm::Image32u(256, 1))
    , m_illumView(LED_ILLUM_FRAME_MAX)
    , m_colorgrabImage(new Ipvm::Image8u3)
    , m_eCurVisionModule(i_eSideVisionModule)
{
}

DlgVisionIllumSetup2D::~DlgVisionIllumSetup2D()
{
    delete m_profileView;
    delete m_propertyGrid;
    delete m_imageView;
    delete m_imageViewRearSide;
    delete m_grabImage;
    delete m_histogram;
    delete m_colorgrabImage;
}

void DlgVisionIllumSetup2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_ILLUM_VIEW, m_illumView);
    DDX_Control(pDX, IDC_STATIC_SIDE_FRONT_ILLUM, m_Label_Side_Front_Status);
    DDX_Control(pDX, IDC_STATIC_SIDE_REAR_ILLUM, m_Label_Side_Rear_Status);
}

BEGIN_MESSAGE_MAP(DlgVisionIllumSetup2D, CDialog)
ON_MESSAGE(UM_GRAB_END_EVENT, &DlgVisionIllumSetup2D::OnGrabEndEvent)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_WM_DESTROY()
ON_STN_CLICKED(IDC_STATIC_SIDE_FRONT_ILLUM, &DlgVisionIllumSetup2D::OnStnClickedStaticSideFrontIllum)
ON_STN_CLICKED(IDC_STATIC_SIDE_REAR_ILLUM, &DlgVisionIllumSetup2D::OnStnClickedStaticSideRearIllum)
END_MESSAGE_MAP()

// DlgVisionIllumSetup2D 메시지 처리기입니다.
BOOL DlgVisionIllumSetup2D::OnInitDialog()
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

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP) // Side Vision UI 구분 - JHB_2024.04.15
    {
        SetGeneralVisionIllumUI();
    }
    else
    {
        SetSideVisionIllumUI();
    }

    //{{ //kircheis_WB
    m_vecfDestIllumSet.clear();
    m_vecfDestIllumSet.resize(13);
    memset(&m_vecfDestIllumSet[0], 0, 13 * sizeof(float));
    //}}

    SetIllumiParameter(-1);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgVisionIllumSetup2D::SetGeneralVisionIllumUI()
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

    m_imageView = new ImageViewEx(
        GetSafeHwnd(), Ipvm::FromMFC(::GetContentSubArea(rtClient, 1, 3, 3, 0, 1, 1)), 0, m_eCurVisionModule);

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

void DlgVisionIllumSetup2D::SetSideVisionIllumUI()
{
    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(0, 0, rtParentClient.Width(), rtParentClient.Height(), FALSE);

    CRect rtClient, rtImageIllum;
    GetClientRect(rtClient);
    rtClient.DeflateRect(5, 5);
    rtImageIllum.CopyRect(rtClient);

    // Side Vision Test
    const long labelHeight = 43;
    //const long labelGap = 3;

    CRect rtSideSelectorLabel;
    rtClient.top = rtClient.top + labelHeight + 1;

    rtSideSelectorLabel.top = rtImageIllum.top;
    rtSideSelectorLabel.bottom = rtClient.top - 2;
    rtSideSelectorLabel.left = rtClient.left + (rtClient.Width() / 6);
    rtSideSelectorLabel.right = rtSideSelectorLabel.left + rtClient.left + (rtClient.Width() / 6) - 2;

    GetDlgItem(IDC_STATIC_SIDE_FRONT_ILLUM)->MoveWindow(::GetContentSubArea(rtSideSelectorLabel, 0, 1, 2, 0, 1, 1));
    GetDlgItem(IDC_STATIC_SIDE_REAR_ILLUM)->MoveWindow(::GetContentSubArea(rtSideSelectorLabel, 1, 2, 2, 0, 1, 1));

    SetDlgItemText(IDC_STATIC_SIDE_FRONT_ILLUM, _T("SIDE FRONT"));
    SetDlgItemText(IDC_STATIC_SIDE_REAR_ILLUM, _T("SIDE REAR"));

    // Selector 자리를 확보 후 Selection Label에 대한 색깔 부여
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
        m_Label_Side_Rear_Status.SetBkColor(defColor);
    }
    else
    {
        m_Label_Side_Front_Status.SetBkColor(defColor);
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
    }
    ////

    GetDlgItem(IDC_STATIC_ILLUM_RGB)->MoveWindow(::GetContentSubArea(rtImageIllum, 0, 1, 6, 0, 2, 5));

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

    m_imageView = new ImageViewEx(GetSafeHwnd(),
        Ipvm::FromMFC(CRect(
            (rtImageIllum.Width() / 3) + 2, rtImageIllum.top, rtImageIllum.right, (rtImageIllum.Height() / 2) - 2)),
        0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    m_imageViewRearSide = new ImageViewEx(GetSafeHwnd(),
        Ipvm::FromMFC(CRect((rtImageIllum.Width() / 3) + 2, (rtImageIllum.Height() / 2) + 2, rtImageIllum.right,
            (rtImageIllum.Height() - 2))),
        0, enSideVisionModule::SIDE_VISIONMODULE_REAR);

    Ipvm::Rect32s roi(
        0, 0, FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    roi.DeflateRect(roi.Width() / 4, roi.Height() / 4);

    if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        m_imageView->ROISet(_T("Histogram area Side Front"), _T("Histogram area"), roi, RGB(255, 0, 0));
        m_imageView->ROIShow();
    }
    else
    {
        m_imageViewRearSide->ROISet(_T("Histogram area Side Rear"), _T("Histogram area"), roi, RGB(255, 0, 0));
        m_imageViewRearSide->ROIShow();
    }

    m_imageView->RegisterCallback_ROIChangeEnd(GetSafeHwnd(), this, callBack_roiChanged);
    m_imageViewRearSide->RegisterCallback_ROIChangeEnd(GetSafeHwnd(), this, callBack_roiChanged);

    m_profileView = new Ipvm::ProfileView(
        GetSafeHwnd(), Ipvm::FromMFC(::GetContentSubArea(/*rtClient*/ rtImageIllum, 0, 1, 6, 2, 3, 5)));
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

BOOL DlgVisionIllumSetup2D::UpdateIllumChSetupUI(long nFrame, BOOL bRecommendSetForColor, long nModifyCh) //kircheis_WB
{
    m_propertyGrid->SetRedraw(FALSE);
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Common")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(
                _T("Use Color Frame"), illumJob().m_2D_colorFrame, &illumJob().m_2D_colorFrame)))
        {
            item->SetID(enumGridItem_UseColorFrame);
        }

        category->Expand();
    }

    if (nFrame < 0 || illumJob().getTotalFrameCount() <= nFrame)
    {
        m_propertyGrid->HighlightChangedItems(TRUE);
        m_propertyGrid->SetRedraw(TRUE);
        m_propertyGrid->Refresh();
        return FALSE;
    }

    //{{kircheis_WB
    auto& sysConfig = SystemConfig::GetInstance();
    const long nChannelMaxNum = sysConfig.GetVisionType() != VISIONTYPE_SIDE_INSP ? LED_ILLUM_CHANNEL_DEFAULT
                                                                                  : LED_ILLUM_CHANNEL_SIDE_DEFAULT;
    if (sysConfig.m_vecIllumFittingRef.size() < nChannelMaxNum)
        bRecommendSetForColor = FALSE;
    static const float fLowLimit = 0.3f;
    static const float fToleranceRange = 0.005f;
    BOOL bIsRedCh
        = ((illumJob().m_2D_colorFrameIndex_red == nFrame) && (nModifyCh == 0 || nModifyCh == 3 || nModifyCh == 6));
    BOOL bIsGreenCh
        = ((illumJob().m_2D_colorFrameIndex_green == nFrame) && (nModifyCh == 1 || nModifyCh == 4 || nModifyCh == 7));
    BOOL bIsBlueCh
        = ((illumJob().m_2D_colorFrameIndex_blue == nFrame) && (nModifyCh == 2 || nModifyCh == 5 || nModifyCh == 8));
    BOOL bDisplayIllumSet = FALSE;
    std::vector<float> vecfDestIllumSet(nChannelMaxNum);
    memset(&vecfDestIllumSet[0], 0, nChannelMaxNum * sizeof(float));
    if (bRecommendSetForColor
        && (illumJob().m_2D_colorFrameIndex_red == nFrame || illumJob().m_2D_colorFrameIndex_green == nFrame
            || illumJob().m_2D_colorFrameIndex_blue == nFrame))
    {
        if (nModifyCh >= 0)
        {
            float fCurChSetValue = illumJob().getIllumChannel_ms(nFrame, nModifyCh);
            auto lineEqRef = sysConfig.m_vecIllumFittingRef[nModifyCh];
            float fDestIntensity = (-lineEqRef.m_a / lineEqRef.m_b) * fCurChSetValue - (lineEqRef.m_c / lineEqRef.m_b);

            if (bIsRedCh || bIsGreenCh || bIsBlueCh)
            {
                for (long nCh = 0; nCh < 9; nCh++)
                {
                    m_vecfDestIllumSet[nCh] = (float)max(0.f,
                        (-sysConfig.m_vecIllumFittingRef[nCh].m_b / sysConfig.m_vecIllumFittingRef[nCh].m_a)
                                * fDestIntensity
                            - (sysConfig.m_vecIllumFittingRef[nCh].m_c / sysConfig.m_vecIllumFittingRef[nCh].m_a));
                    m_vecfDestIllumSet[nCh] = m_vecfDestIllumSet[nCh] < fLowLimit ? 0.f : m_vecfDestIllumSet[nCh];
                }
            }

            if (illumJob().m_2D_colorFrameIndex_red == nFrame)
            {
                bDisplayIllumSet = TRUE;
                vecfDestIllumSet[0] = m_vecfDestIllumSet[0] > 0.f ? m_vecfDestIllumSet[0] : -2.f;
                vecfDestIllumSet[3] = m_vecfDestIllumSet[3] > 0.f ? m_vecfDestIllumSet[3] : -2.f;
                vecfDestIllumSet[6] = m_vecfDestIllumSet[6] > 0.f ? m_vecfDestIllumSet[6] : -2.f;

                if (bIsRedCh)
                    vecfDestIllumSet[nModifyCh] = -1.f;
            }
            else if (illumJob().m_2D_colorFrameIndex_green == nFrame)
            {
                bDisplayIllumSet = TRUE;
                vecfDestIllumSet[1] = m_vecfDestIllumSet[1] > 0.f ? m_vecfDestIllumSet[1] : -3.f;
                vecfDestIllumSet[4] = m_vecfDestIllumSet[4] > 0.f ? m_vecfDestIllumSet[4] : -3.f;
                vecfDestIllumSet[7] = m_vecfDestIllumSet[7] > 0.f ? m_vecfDestIllumSet[7] : -3.f;

                if (bIsGreenCh)
                    vecfDestIllumSet[nModifyCh] = -1.f;
            }
            else if (illumJob().m_2D_colorFrameIndex_blue == nFrame)
            {
                bDisplayIllumSet = TRUE;
                vecfDestIllumSet[2] = m_vecfDestIllumSet[2] > 0.f ? m_vecfDestIllumSet[2] : -4.f;
                vecfDestIllumSet[5] = m_vecfDestIllumSet[5] > 0.f ? m_vecfDestIllumSet[5] : -4.f;
                vecfDestIllumSet[8] = m_vecfDestIllumSet[8] > 0.f ? m_vecfDestIllumSet[8] : -4.f;

                if (bIsBlueCh)
                    vecfDestIllumSet[nModifyCh] = -1.f;
            }
        }
        else if (nModifyCh < 0)
        {
            bDisplayIllumSet = TRUE;
            if (illumJob().m_2D_colorFrameIndex_red == nFrame)
            {
                if (m_vecfDestIllumSet[0] <= fLowLimit)
                    vecfDestIllumSet[0] = vecfDestIllumSet[3] = vecfDestIllumSet[6] = -2.f;
                else
                {
                    vecfDestIllumSet[0] = m_vecfDestIllumSet[0];
                    vecfDestIllumSet[3] = m_vecfDestIllumSet[3];
                    vecfDestIllumSet[6] = m_vecfDestIllumSet[6];
                }
            }
            else if (illumJob().m_2D_colorFrameIndex_green == nFrame)
            {
                if (m_vecfDestIllumSet[1] <= fLowLimit)
                    vecfDestIllumSet[1] = vecfDestIllumSet[4] = vecfDestIllumSet[7] = -3.f;
                else
                {
                    vecfDestIllumSet[1] = m_vecfDestIllumSet[1];
                    vecfDestIllumSet[4] = m_vecfDestIllumSet[4];
                    vecfDestIllumSet[7] = m_vecfDestIllumSet[7];
                }
            }
            else if (illumJob().m_2D_colorFrameIndex_blue == nFrame)
            {
                if (m_vecfDestIllumSet[2] <= fLowLimit)
                    vecfDestIllumSet[2] = vecfDestIllumSet[5] = vecfDestIllumSet[8] = -4.f;
                else
                {
                    vecfDestIllumSet[2] = m_vecfDestIllumSet[2];
                    vecfDestIllumSet[5] = m_vecfDestIllumSet[5];
                    vecfDestIllumSet[8] = m_vecfDestIllumSet[8];
                }
            }
            else
                bDisplayIllumSet = FALSE;
        }
    }
    //}}

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

        COLORREF channelColors[] = {
            RGB(255, 0, 0),
            RGB(0, 200, 0),
            RGB(0, 0, 255),
            RGB(255, 0, 0),
            RGB(0, 200, 0),
            RGB(0, 0, 255),
            RGB(255, 0, 0),
            RGB(0, 200, 0),
            RGB(0, 0, 255),
            RGB(255, 0, 0),
            RGB(255, 0, 0),
            RGB(255, 0, 0),
            RGB(0, 0, 255),
            RGB(255, 0, 0),
            RGB(255, 0, 0),
            RGB(255, 0, 0),
        };

        static_assert(sizeof(channelColors) / sizeof(COLORREF) == LED_ILLUM_CHANNEL_MAX, "Array size error");

        CString strChannelName; //kircheis_WB

        category->AddChildItem(
            new CXTPPropertyGridItem(_T("Name"), illumJob().getIllumName(nFrame), illumJob().getIllumNamePtr(nFrame)));

        long UseChannelCount(0);
        switch (SystemConfig::GetInstance().GetVisionType())
        {
            case VISIONTYPE_2D_INSP:
                UseChannelCount = SystemConfig::GetInstance().GetExistRingillumination() == true
                    ? LED_ILLUM_CHANNEL_RING_MAX + LED_ILLUM_CHANNEL_DEFAULT
                    : LED_ILLUM_CHANNEL_DEFAULT;
                break;
            case VISIONTYPE_SIDE_INSP:
                UseChannelCount = LED_ILLUM_CHANNEL_SIDE_DEFAULT;
                break;
            case VISIONTYPE_TR:
                UseChannelCount = SystemConfig::GetInstance().GetExistRingillumination() == true
                    ? LED_ILLUM_CHANNEL_RING_MAX + LED_ILLUM_CHANNEL_DEFAULT
                    : LED_ILLUM_CHANNEL_DEFAULT;
                break;
        }

        for (long idx = 0; idx < UseChannelCount; idx++)
        {
            float illumChannel_ms = illumJob().getIllumChannel_ms(nFrame, idx);
            //{{//kircheis_WB
            if (bDisplayIllumSet)
            {
                if (vecfDestIllumSet[idx] > 0.f)
                {
                    if (vecfDestIllumSet[idx] - fToleranceRange <= illumChannel_ms
                        && vecfDestIllumSet[idx] + fToleranceRange >= illumChannel_ms)
                        strChannelName.Format(_T("%s  [Advice : OK]"), channelNames[idx]);
                    else
                        strChannelName.Format(_T("%s  [Advice : %.3f]"), channelNames[idx], vecfDestIllumSet[idx]);
                }
                else if (vecfDestIllumSet[idx] == 0.f)
                {
                    if (illumChannel_ms > 0.f)
                        strChannelName.Format(_T("%s  [Advice : Off (0)]"), channelNames[idx]);
                    else
                        strChannelName.Format(_T("%s"), channelNames[idx]);
                }
                else if (vecfDestIllumSet[idx] == -1.f)
                    strChannelName.Format(_T("%s  [Reference]"), channelNames[idx]);
                else if (vecfDestIllumSet[idx] == -2.f)
                    strChannelName.Format(_T("%s  [Red Frame]"), channelNames[idx]);
                else if (vecfDestIllumSet[idx] == -3.f)
                    strChannelName.Format(_T("%s  [Green Frame]"), channelNames[idx]);
                else if (vecfDestIllumSet[idx] == -4.f)
                    strChannelName.Format(_T("%s  [Blue Frame]"), channelNames[idx]);
                else
                    strChannelName.Format(_T("%s"), channelNames[idx]);
            }
            else
                strChannelName.Format(_T("%s"), channelNames[idx]);
            //}}
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

        if (auto* item = category->AddChildItem(
                new CCustomItemButton(_T("Grab && Display color image"), TRUE, FALSE))) //kircheis_WB
        {
            item->SetID(enumGridItem_GrabMakeColor);
        }

        category->Expand();

        strChannelName.Empty();
    }

    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->SetRedraw(TRUE);
    m_propertyGrid->Refresh();

    return TRUE;
}

void DlgVisionIllumSetup2D::SetIllumiParameter(long nFrame)
{
    nFrame = min(nFrame, illumJob().getTotalFrameCount());

    if (UpdateIllumChSetupUI(nFrame, illumJob().m_2D_colorFrame) == FALSE) //kircheis_WB
        return;

    if (m_visionUnit.m_systemConfig.IsHardwareExist())
    {
        SyncController::GetInstance().SetIllumiParameter(
            nFrame, illumJob().getIllum(nFrame), true, false, 0, m_eCurVisionModule);
        SyncController::GetInstance().TurnOnLight(nFrame, m_eCurVisionModule);
        FrameGrabber::GetInstance().live_on(GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, m_eCurVisionModule);

        long nProbePosZ = illumJob().isHighFrame(nFrame) ? _MULTI_GRAB_POS_HIGH : _MULTI_GRAB_POS_NORMAL;

        m_visionMain.iPIS_Send_MultiGrabProbePosZ(nProbePosZ);
    }
    else
    {
        if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP) // Side Vision 구분 - JHB_2024.04.15
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
                m_visionUnit.getImageLot().GetImageFrame(nImageFrameNum, m_eCurVisionModule)); // 이미지 출력
            // }
            calcHistogram(&m_imageView->GetImage_8u_C1());
        }
        else
        {
            long nFrameNum = m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_FRONT
                ? (long)m_visionUnit.getImageLot().GetImageFrameCount(enSideVisionModule::SIDE_VISIONMODULE_FRONT)
                : (long)m_visionUnit.getImageLot().GetImageFrameCount(enSideVisionModule::SIDE_VISIONMODULE_REAR);

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

            if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_FRONT)
            {
                m_imageView->SetImage(m_visionUnit.getImageLot().GetImageFrame(
                    nImageFrameNum, enSideVisionModule::SIDE_VISIONMODULE_FRONT)); // 이미지 출력
                // }

                calcHistogram(&m_imageView->GetImage_8u_C1());
            }
            else
            {
                m_imageViewRearSide->SetImage(m_visionUnit.getImageLot().GetImageFrame(
                    nImageFrameNum, enSideVisionModule::SIDE_VISIONMODULE_REAR)); // 이미지 출력
                // }

                calcHistogram(&m_imageViewRearSide->GetImage_8u_C1());
            }
        }
    }

    m_nCurrentFrameIndex = nFrame;
}

void DlgVisionIllumSetup2D::updateSpecToIllumView()
{
    m_illumView.resetData();
    m_illumView.addFrameGroup(0, _T("Normal Position"), RGB(45, 79, 119), RGB(255, 255, 255));

    if (SystemConfig::GetInstance().GetHandlerType() != HANDLER_TYPE_380BRIDGE)
    {
        m_illumView.addFrameGroup(1, _T("High Position"), RGB(164, 201, 221), RGB(45, 79, 119));
    }

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
            onlyOneFrameCanBeUsed = false;
        }

        if (visionProcessing->m_moduleGuid == _VISION_INSP_GUID_PACKAGE_SPEC)
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

    if (illumJob().m_2D_colorFrame)
    {
        long id_r = m_illumView.getItemCount();
        long id_g = m_illumView.getItemCount() + 1;
        long id_b = m_illumView.getItemCount() + 2;

        m_illumView.addItem(id_r, _T("Color RED"), true);
        m_illumView.addItem(id_g, _T("Color GREEN"), true);
        m_illumView.addItem(id_b, _T("Color BLUE"), true);

        auto& index_r = illumJob().m_2D_colorFrameIndex_red;
        auto& index_g = illumJob().m_2D_colorFrameIndex_green;
        auto& index_b = illumJob().m_2D_colorFrameIndex_blue;

        index_r = min(LED_ILLUM_FRAME_MAX - 1, max(0, index_r));
        index_g = min(LED_ILLUM_FRAME_MAX - 1, max(0, index_g));
        index_b = min(LED_ILLUM_FRAME_MAX - 1, max(0, index_b));

        m_illumView.addItemFrame(id_r, index_r);
        m_illumView.addItemFrame(id_g, index_g);
        m_illumView.addItemFrame(id_b, index_b);
    }

    for (long frameIndex = 0; frameIndex < LED_ILLUM_FRAME_MAX; frameIndex++)
    {
        m_illumView.setFrameGroup(frameIndex, illumJob().isHighFrame(frameIndex) ? 1 : 0);
    }

    m_illumView.setEditMode(m_visionUnit.m_systemConfig.m_nCurrentAccessMode >= _ENGINEER);
}

IllumInfo2D& DlgVisionIllumSetup2D::illumJob()
{
    return m_visionMain.m_illum2D;
}

void DlgVisionIllumSetup2D::callBack_roiChanged(
    void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(key);

    // Side Vision 분리 - JHB_2024.04.15
    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        auto& image = ((DlgVisionIllumSetup2D*)userData)->m_imageView->GetImage_8u_C1();
        ((DlgVisionIllumSetup2D*)userData)->calcHistogram(&image);
    }
    else
    {
        auto& image = ((DlgVisionIllumSetup2D*)userData)->m_visionMain.GetSideVisionSection()
                == enSideVisionModule::SIDE_VISIONMODULE_FRONT
            ? ((DlgVisionIllumSetup2D*)userData)->m_imageView->GetImage_8u_C1()
            : ((DlgVisionIllumSetup2D*)userData)->m_imageViewRearSide->GetImage_8u_C1();

        ((DlgVisionIllumSetup2D*)userData)->calcHistogram(&image);
    }
}

void DlgVisionIllumSetup2D::callBack_changedSelectFrame(LPVOID userData, long realFrameIndex)
{
    ((DlgVisionIllumSetup2D*)userData)->callBack_changedSelectFrame(realFrameIndex);
}

void DlgVisionIllumSetup2D::callBack_changedSelectItem(LPVOID userData, long itemID)
{
    ((DlgVisionIllumSetup2D*)userData)->callBack_changedSelectItem(itemID);
}

void DlgVisionIllumSetup2D::callBack_changedItemFrame(LPVOID userData, long itemID, long realFrameIndex, bool checked)
{
    ((DlgVisionIllumSetup2D*)userData)->callBack_changedItemFrame(itemID, realFrameIndex, checked);
}

void DlgVisionIllumSetup2D::callBack_deletedFrame(LPVOID userData, long realFrameIndex, bool isBackup)
{
    ((DlgVisionIllumSetup2D*)userData)->callBack_deletedFrame(realFrameIndex, isBackup);
}

void DlgVisionIllumSetup2D::callBack_insertedFrame(LPVOID userData, long groupID, long realFrameIndex, bool isBackup)
{
    ((DlgVisionIllumSetup2D*)userData)->callBack_insertedFrame(groupID, realFrameIndex, isBackup);
}

void DlgVisionIllumSetup2D::callBack_changedSelectFrame(long realFrameIndex)
{
    SetIllumiParameter(realFrameIndex);
}

void DlgVisionIllumSetup2D::callBack_changedSelectItem(long /*itemID*/)
{
}

void DlgVisionIllumSetup2D::callBack_changedItemFrame(long itemID, long realFrameIndex, bool checked)
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
    else
    {
        switch (itemID - procssingCount)
        {
            case 0:
                illumJob().m_2D_colorFrameIndex_red = checked ? realFrameIndex : -1;
                break; // R
            case 1:
                illumJob().m_2D_colorFrameIndex_green = checked ? realFrameIndex : -1;
                break; // G
            case 2:
                illumJob().m_2D_colorFrameIndex_blue = checked ? realFrameIndex : -1;
                break; // B
        }
    }

    if (realFrameIndex < illumJob().getTotalFrameCount())
    {
        SetIllumiParameter(realFrameIndex);
    }
}

void DlgVisionIllumSetup2D::callBack_deletedFrame(long realFrameIndex, bool isBackup)
{
    long procssingCount = m_visionUnit.GetVisionProcessingCount();

    if (isBackup)
    {
        m_backupInfo_name = illumJob().getIllumName(realFrameIndex);
        m_backupInfo_items.clear();
        m_backupInfo_items_inspFrames.clear();

        m_backupInfo_colorFrame[0] = false;
        m_backupInfo_colorFrame[1] = false;
        m_backupInfo_colorFrame[2] = false;

        for (long n = 0; n < 16; n++)
        {
            m_backupInfo_illuminations_ms[n] = illumJob().getIllumChannel_ms(realFrameIndex, n);
        }

        if (illumJob().m_2D_colorFrameIndex_red == realFrameIndex)
            m_backupInfo_colorFrame[0] = true;
        if (illumJob().m_2D_colorFrameIndex_green == realFrameIndex)
            m_backupInfo_colorFrame[1] = true;
        if (illumJob().m_2D_colorFrameIndex_blue == realFrameIndex)
            m_backupInfo_colorFrame[2] = true;
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

    if (illumJob().m_2D_colorFrame)
    {
        if (illumJob().m_2D_colorFrameIndex_red == realFrameIndex)
        {
            illumJob().m_2D_colorFrameIndex_red = -1;
        }
        else if (illumJob().m_2D_colorFrameIndex_red > realFrameIndex)
        {
            illumJob().m_2D_colorFrameIndex_red--;
        }

        if (illumJob().m_2D_colorFrameIndex_green == realFrameIndex)
        {
            illumJob().m_2D_colorFrameIndex_green = -1;
        }
        else if (illumJob().m_2D_colorFrameIndex_green > realFrameIndex)
        {
            illumJob().m_2D_colorFrameIndex_green--;
        }

        if (illumJob().m_2D_colorFrameIndex_blue == realFrameIndex)
        {
            illumJob().m_2D_colorFrameIndex_blue = -1;
        }
        else if (illumJob().m_2D_colorFrameIndex_blue > realFrameIndex)
        {
            illumJob().m_2D_colorFrameIndex_blue--;
        }
    }

    inspFrameInvalidCheck();
}

void DlgVisionIllumSetup2D::callBack_insertedFrame(long groupID, long realFrameIndex, bool isBackup)
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

    if (isBackup && illumJob().m_2D_colorFrame)
    {
        if (m_backupInfo_colorFrame[0])
            illumJob().m_2D_colorFrameIndex_red = realFrameIndex;
        if (m_backupInfo_colorFrame[1])
            illumJob().m_2D_colorFrameIndex_green = realFrameIndex;
        if (m_backupInfo_colorFrame[2])
            illumJob().m_2D_colorFrameIndex_blue = realFrameIndex;
    }
}

void DlgVisionIllumSetup2D::inspFrameInvalidCheck()
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

void DlgVisionIllumSetup2D::calcHistogram(const Ipvm::Image8u* image)
{
    Ipvm::Rect32s roi;

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    // JHB_2024.04.15
    if (bIsSideVision == false)
    {
        m_imageView->ROIGet(_T("Histogram area"), roi);
    }
    else
    {
        if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
        {
            m_imageView->WindowOverlayClear();
            m_imageView->WindowOverlayShow();
            m_imageViewRearSide->ROIGet(_T("Histogram area Side Rear"), roi);
        }
        else
        {
            if (m_imageViewRearSide != nullptr)
                m_imageViewRearSide->WindowOverlayClear();
            if (m_imageViewRearSide != nullptr)
                m_imageViewRearSide->WindowOverlayShow();
            m_imageView->ROIGet(_T("Histogram area Side Front"), roi);
        }
    }
    ////////////////

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

    if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_imageViewRearSide->WindowOverlayClear();
        m_imageViewRearSide->WindowOverlayAdd(Ipvm::Point32s2(0, 20), str, RGB(255, 0, 0), 32);
        m_imageViewRearSide->WindowOverlayShow();
    }
    else
    {
        m_imageView->WindowOverlayClear();
        m_imageView->WindowOverlayAdd(Ipvm::Point32s2(0, 20), str, RGB(255, 0, 0), 32);
        m_imageView->WindowOverlayShow();
    }

    str.Empty();
}

LRESULT DlgVisionIllumSetup2D::OnGrabEndEvent(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    FrameGrabber::GetInstance().get_live_image(*m_grabImage);

    if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_FRONT)
        m_imageView->SetImage(*m_grabImage);
    else
        m_imageViewRearSide->SetImage(*m_grabImage);

    calcHistogram(m_grabImage);

    return 1L;
}

LRESULT DlgVisionIllumSetup2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        const long channel = item->GetID() - enumGridItem_FirstID;

        auto* value = dynamic_cast<XTPPropertyGridItemCustomFloat<float>*>(item);

        if (item->GetID() == enumGridItem_UseColorFrame)
        {
            updateSpecToIllumView();

            //{{ //kircheis_WB
            auto& sysConfig = SystemConfig::GetInstance();
            long nUseChannel_Default_Num = sysConfig.GetVisionType() != VISIONTYPE_SIDE_INSP
                ? LED_ILLUM_CHANNEL_DEFAULT
                : LED_ILLUM_CHANNEL_SIDE_DEFAULT;
            if (illumJob().m_2D_colorFrame && sysConfig.m_vecIllumFittingRef.size() < nUseChannel_Default_Num)
                ::SimpleMessage(_T("Calibration of the illumination is required to facilitate color image creation."));
            //}}
        }
        else if (item->GetID() == enumGridItem_TurnOffLight)
        {
            FrameGrabber::GetInstance().live_off();
            SyncController::GetInstance().TurnOffLight(m_eCurVisionModule);
        }
        else if (item->GetID() == enumGridItem_GrabMakeColor) //kircheis_WB
        {
            if (SystemConfig::GetInstance().m_bHardwareExist)
            {
                FrameGrabber::GetInstance().live_off();
                SyncController::GetInstance().TurnOffLight(m_eCurVisionModule);
            }

            GrabAndDisplayColorImage();
        }
        else if (value)
        {
            SyncController::GetInstance().SetIllumiParameter(m_nCurrentFrameIndex, channel,
                illumJob().getIllumChannel_ms(m_nCurrentFrameIndex, channel), true, false, 0, m_eCurVisionModule);
            SyncController::GetInstance().TurnOnLight(m_nCurrentFrameIndex, m_eCurVisionModule);

            if (SystemConfig::GetInstance().m_bHardwareExist
                && FrameGrabber::GetInstance().IsLive() == FALSE) //kircheis_WB
                FrameGrabber::GetInstance().live_on(GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, m_eCurVisionModule);

            long nProbePosZ
                = illumJob().isHighFrame(m_nCurrentFrameIndex) ? _MULTI_GRAB_POS_HIGH : _MULTI_GRAB_POS_NORMAL;

            m_visionMain.iPIS_Send_MultiGrabProbePosZ(nProbePosZ);
            UpdateIllumChSetupUI(m_nCurrentFrameIndex, illumJob().m_2D_colorFrame, channel); //kircheis_WB
        }
    }

    return 0;
}

void DlgVisionIllumSetup2D::OnDestroy()
{
    FrameGrabber::GetInstance().live_off();
    SyncController::GetInstance().TurnOffLight(m_eCurVisionModule);

    CDialog::OnDestroy();
}

void DlgVisionIllumSetup2D::GrabAndDisplayColorImage() //kircheis_WB
{
    if (illumJob().m_2D_colorFrame == FALSE)
        return;

    long nFrameIDs[3];
    nFrameIDs[COLOR_RED] = illumJob().m_2D_colorFrameIndex_red;
    nFrameIDs[COLOR_GREEN] = illumJob().m_2D_colorFrameIndex_green;
    nFrameIDs[COLOR_BLUE] = illumJob().m_2D_colorFrameIndex_blue;

    // Side Vision 선택 - JHB_2024.04.15
    m_eCurVisionModule = m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_FRONT
        ? enSideVisionModule::SIDE_VISIONMODULE_FRONT
        : enSideVisionModule::SIDE_VISIONMODULE_REAR;

    auto& imageLot = m_visionUnit.getImageLot();
    BYTE* imageMems[3];
    if (SystemConfig::GetInstance().m_bHardwareExist == FALSE)
    {
        long nImageNum = imageLot.GetImageFrameCount();
        if (nImageNum < 1)
            return;

        Ipvm::Image8u3 imageColor(imageLot.GetImageSizeX(), imageLot.GetImageSizeY());
        for (long i = COLOR_BLUE; i <= COLOR_RED; i++)
            nFrameIDs[i] = (long)min(nImageNum - 1, nFrameIDs[i]);

        Ipvm::ImageProcessing::CombineRGB(imageLot.GetImageFrame(nFrameIDs[COLOR_RED], m_eCurVisionModule),
            imageLot.GetImageFrame(nFrameIDs[COLOR_GREEN], m_eCurVisionModule),
            imageLot.GetImageFrame(nFrameIDs[COLOR_BLUE], m_eCurVisionModule), Ipvm::Rect32s(imageColor), imageColor);

        // JHB_2024.04.15
        if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
        {
            m_imageViewRearSide->SetImage(imageColor);
        }
        else
        {
            m_imageView->SetImage(imageColor);
        }
        /////////////////

        return;
    }

    Ipvm::Image8u3 imageColor(
        FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    Ipvm::Image8u imageRed(
        FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    Ipvm::Image8u imageGreen(
        FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    Ipvm::Image8u imageBlue(
        FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    imageMems[COLOR_RED] = imageRed.GetMem();
    imageMems[COLOR_GREEN] = imageGreen.GetMem();
    imageMems[COLOR_BLUE] = imageBlue.GetMem();

    SyncController::GetInstance().SetFrameCount(1, 0, m_eCurVisionModule);
    const long nVisionType = SystemConfig::GetInstance().GetVisionType();
    for (long nColor = COLOR_BLUE; nColor <= COLOR_RED; nColor++)
    {
        SyncController::GetInstance().SetIllumiParameter(
            0, illumJob().getIllum(nFrameIDs[nColor]), true, false, 0, m_eCurVisionModule);
        Sleep(10);

        FrameGrabber::GetInstance().StartGrab2D(0, (int)m_eCurVisionModule, &imageMems[nColor], 1, nullptr);
        SyncController::GetInstance().StartSyncBoard(FALSE, m_eCurVisionModule);
        FrameGrabber::GetInstance().wait_grab_end(0, (int)m_eCurVisionModule, nullptr);

        Sleep(100);
    }
    SyncController::GetInstance().SetIllumiParameter(0, illumJob().getIllum(0), true, false, 0, m_eCurVisionModule);

    Ipvm::ImageProcessing::CombineRGB(imageRed, imageGreen, imageBlue, Ipvm::Rect32s(imageRed), imageColor);

    // 2024.04.15
    if (m_visionMain.GetSideVisionSection() == enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_imageViewRearSide->SetImage(imageColor);
    }
    else
    {
        m_imageView->SetImage(imageColor);
    }
    /////////////
}

BOOL DlgVisionIllumSetup2D::callJobOpen()
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

BOOL DlgVisionIllumSetup2D::callJobSave()
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

void DlgVisionIllumSetup2D::IllumLink(LPCTSTR filePath, bool save, IllumInfo2D& io_illum)
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

void DlgVisionIllumSetup2D::OnStnClickedStaticSideFrontIllum()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_visionMain.GetSideVisionSection() != enSideVisionModule::SIDE_VISIONMODULE_FRONT)
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
        m_Label_Side_Rear_Status.SetBkColor(defColor);

        SyncController::GetInstance().TurnOffLight(m_eCurVisionModule);
        FrameGrabber::GetInstance().live_off();

        m_eCurVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT;

        m_visionMain.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
    }
    else
    {
        m_Label_Side_Front_Status.SetBkColor(RGB(0, 255, 0));
    }

    calcHistogram(&m_imageView->GetImage_8u_C1());

    Ipvm::Rect32s roi(
        0, 0, FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    roi.DeflateRect(roi.Width() / 4, roi.Height() / 4);

    if (m_imageViewRearSide != nullptr)
        m_imageViewRearSide->ROIClear();

    m_imageView->ROISet(_T("Histogram area Side Front"), _T("Histogram area"), roi, RGB(255, 0, 0));
    m_imageView->ROIShow();

    SetIllumiParameter(m_nCurrentFrameIndex);
}

void DlgVisionIllumSetup2D::OnStnClickedStaticSideRearIllum()
{
    // 기본 색깔 설정
    const auto defColor = ::GetSysColor(COLOR_3DFACE);

    if (m_visionMain.GetSideVisionSection() != enSideVisionModule::SIDE_VISIONMODULE_REAR)
    {
        m_Label_Side_Front_Status.SetBkColor(defColor);
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));

        SyncController::GetInstance().TurnOffLight(m_eCurVisionModule);
        FrameGrabber::GetInstance().live_off();

        m_eCurVisionModule = enSideVisionModule::SIDE_VISIONMODULE_REAR;

        m_visionMain.SetSideVisionSection(enSideVisionModule::SIDE_VISIONMODULE_REAR);
    }
    else
    {
        m_Label_Side_Rear_Status.SetBkColor(RGB(0, 255, 0));
    }

    if (m_imageViewRearSide != nullptr)
        calcHistogram(&m_imageViewRearSide->GetImage_8u_C1());

    Ipvm::Rect32s roi(
        0, 0, FrameGrabber::GetInstance().get_grab_image_width(), FrameGrabber::GetInstance().get_grab_image_height());
    roi.DeflateRect(roi.Width() / 4, roi.Height() / 4);

    m_imageView->ROIClear();

    if (m_imageViewRearSide != nullptr)
        m_imageViewRearSide->ROISet(_T("Histogram area Side Rear"), _T("Histogram area"), roi, RGB(255, 0, 0));
    if (m_imageViewRearSide != nullptr)
        m_imageViewRearSide->ROIShow();

    SetIllumiParameter(m_nCurrentFrameIndex);
}
