//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CDlgVisionIllumSetupNGRV.h"

//CPP_2_________________________________ This project's headers
#include "VisionMain.h"

//CPP_3_________________________________ Other projects' headers
#include "../../HardwareModules/dPI_Framegrabber/FrameGrabber.h"
#include "../../HardwareModules/dPI_SyncController/SyncController.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h" //kircheis_WB
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedComponent/Persistence/IniHelper.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"

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

enum enumSeparateFrameNum
{
    Reverse_Frame = 7,
    UV_Frame,
    IR_Frame,
};

enum enumSeparateChannelInfo
{
    UV_Channel = 1,
    IR_Channel = 3,
    Reverse_Channel = 8,
};

enum enumUnuseChannelInfo
{
    Channel_10 = 9,
    Channel_11,
};

IMPLEMENT_DYNAMIC(CDlgVisionIllumSetupNGRV, CDialog)

CDlgVisionIllumSetupNGRV::CDlgVisionIllumSetupNGRV(CVisionMain& visionMain, VisionUnit& visionUnit, CWnd* pParent)
    : CDialog(CDlgVisionIllumSetupNGRV::IDD, pParent)
    , m_visionMain(visionMain)
    , m_visionUnit(visionUnit)
    , m_imageView(nullptr)
    , m_propertyGrid(nullptr)
    , m_profileView(nullptr)
    , m_grabImage(new Ipvm::Image8u)
    , m_bayerImage(new Ipvm::Image8u3)
    , m_histogram(new Ipvm::Image32u(256, 1))
    , m_histogramRed(new Ipvm::Image32u(256, 1))
    , m_histogramGreen(new Ipvm::Image32u(256, 1))
    , m_histogramBlue(new Ipvm::Image32u(256, 1))
    , m_NGRV_illumView(LED_ILLUM_NGRV_CHANNEL_MAX)
    , m_ImageRed(new Ipvm::Image8u)
    , m_ImageGreen(new Ipvm::Image8u)
    , m_ImageBlue(new Ipvm::Image8u)
    , m_pParent(pParent)
    , m_bUseGain(FALSE)
{
}

CDlgVisionIllumSetupNGRV::~CDlgVisionIllumSetupNGRV()
{
    delete m_imageView;
    delete m_propertyGrid;
    delete m_profileView;
    delete m_grabImage;
    delete m_bayerImage;
    delete m_histogram;
    delete m_ImageRed;
    delete m_ImageGreen;
    delete m_ImageBlue;
}

void CDlgVisionIllumSetupNGRV::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_NGRV_ILLUM_VIEW, m_NGRV_illumView);
}

BEGIN_MESSAGE_MAP(CDlgVisionIllumSetupNGRV, CDialog)
ON_MESSAGE(UM_GRAB_END_EVENT, &CDlgVisionIllumSetupNGRV::OnGrabEndEvent)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_WM_DESTROY()
END_MESSAGE_MAP()

// CDlgVisionIllumSetupNGRV 메시지 처리기

BOOL CDlgVisionIllumSetupNGRV::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    m_pParent->GetClientRect(rtParentClient);

    MoveWindow(0, 0, rtParentClient.Width(), rtParentClient.Height(), FALSE);

    CRect rtClient;
    GetClientRect(rtClient);
    rtClient.DeflateRect(5, 5);

    GetDlgItem(IDC_STATIC_ILLUM_NGRV)->MoveWindow(::GetContentSubArea(rtClient, 0, 1, 6, 0, 2, 5));

    m_propertyGrid = new CXTPPropertyGrid();
    m_propertyGrid->Create(::GetContentSubArea(rtClient, 1, 2, 6, 0, 3, 5), this, IDC_PROPERTY_GRID);
    m_propertyGrid->EnableWindow(m_visionUnit.m_systemConfig.m_nCurrentAccessMode < _ENGINEER ? FALSE : TRUE);

    m_NGRV_illumView.MoveWindow(::GetContentSubArea(rtClient, 0, 1, 3, 3, 5, 5));
    m_NGRV_illumView.setCallback_changedSelectFrame(this, callBack_changedSelectFrame);
    m_NGRV_illumView.setCallback_changedSelectItem(this, callBack_changedSelectItem);
    m_NGRV_illumView.setCallback_changedItemFrame(this, callBack_changedItemFrame);
    m_NGRV_illumView.setCallback_deletedFrame(this, callBack_deletedFrame);
    m_NGRV_illumView.setCallback_insertedFrame(this, callBack_insertedFrame);
    m_nCurrentFrameIndex = -1;

    UpdateSpecToIllumView();

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

    //{{ //kircheis_WB
    m_vecfDestIllumSet.clear();
    m_vecfDestIllumSet.resize(LED_ILLUM_NGRV_CHANNEL_MAX);
    memset(&m_vecfDestIllumSet[0], 0, LED_ILLUM_NGRV_CHANNEL_MAX * sizeof(float));
    //}}

    SetIllumParameter(-1);

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionIllumSetupNGRV::OnDestroy()
{
    CDialog::OnDestroy();

    FrameGrabber::GetInstance().live_off();
    SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    CDialog::OnDestroy();
}

BOOL CDlgVisionIllumSetupNGRV::UpdateIllumChSetupUI(long nFrame, BOOL bRecommendSetForColor, long nModifyCh)
{
    UNREFERENCED_PARAMETER(nModifyCh);

    m_propertyGrid->SetRedraw(FALSE);
    m_propertyGrid->ResetContent();

    //{{//kircheis_NGRV
    if (illumJob().getTotalFrameCount() == 0)
    {
        for (long i = 0; i < LED_ILLUM_NGRV_CHANNEL_MAX; i++)
            illumJob().insertFrame(i, false, _T(""));
    }
    //}}
    if (nFrame < 0 || illumJob().getTotalFrameCount() <= nFrame)
    {
        m_propertyGrid->HighlightChangedItems(TRUE);
        m_propertyGrid->SetRedraw(TRUE);
        m_propertyGrid->Refresh();
        return FALSE;
    }

    auto& sysConfig = SystemConfig::GetInstance();

    if (sysConfig.m_vecIllumFittingRef.size() < LED_ILLUM_NGRV_CHANNEL_MAX)
    {
        bRecommendSetForColor = FALSE;
    }

    static const float fLowLimit = 0.3f;
    static const float fToleranceRange = 0.005f;

    std::vector<float> vecfDestIllumSet(LED_ILLUM_NGRV_CHANNEL_MAX);
    memset(&vecfDestIllumSet[0], 0, LED_ILLUM_NGRV_CHANNEL_MAX * sizeof(float));

    if (auto* category = m_propertyGrid->AddCategory(_T("Illuminations")))
    {
        LPCTSTR channelNames[] = {_T("CH01"), _T("CH02"), _T("CH03"), _T("CH04"), _T("CH05"), _T("CH06"), _T("CH07"),
            _T("CH08"), _T("CH09"), _T("CH10"), _T("CH11"), _T("CH12")};

        static_assert(sizeof(channelNames) / sizeof(LPCTSTR) == LED_ILLUM_NGRV_CHANNEL_MAX, "Array size error");

        /*COLORREF channelColors[] =
		{
			RGB(0, 0, 0), RGB(159, 13, 183), RGB(0, 0, 0), RGB(220, 60, 0), RGB(0, 0, 0),
			RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
			RGB(0, 0, 0), RGB(0, 0, 0),
		};*/

        COLORREF channelColors[12];

        for (long nIndex = 0; nIndex < LED_ILLUM_NGRV_CHANNEL_MAX; nIndex++)
        {
            if (nIndex == SystemConfig::GetInstance().m_nNgrvUVchID)
            {
                channelColors[nIndex] = RGB(159, 13, 183);
                continue;
            }

            if (nIndex == SystemConfig::GetInstance().m_nNgrvIRchID)
            {
                channelColors[nIndex] = RGB(220, 60, 0);
                continue;
            }

            channelColors[nIndex] = RGB(0, 0, 0);
        }

        static_assert(sizeof(channelColors) / sizeof(COLORREF) == LED_ILLUM_NGRV_CHANNEL_MAX, "Array size error");

        CString strChannelName; //kircheis_WB

        category->AddChildItem(
            new CXTPPropertyGridItem(_T("Name"), illumJob().getIllumName(nFrame), illumJob().getIllumNamePtr(nFrame)));

        long UseChannelCount = LED_ILLUM_NGRV_CHANNEL_MAX;
        //long ChannelMax = UseChannelCount > LED_ILLUM_NGRV_CHANNEL_MAX ? LED_ILLUM_NGRV_CHANNEL_MAX : UseChannelCount;

        //float fTotalExposureTime_EachFrame = 0.f; // 각 Frame별 Exposure time의 총합 : 최대 값을 24 msec로 고정해야함
        //float fTotalExposureTimeMaxValue_EachFrame = 24.f;
        //float fTotalExposureTimeMaxValue_UVIRFrame = 20.f;
        //float fMaxExposureTime_EachChannel = 0;
        //float fMaximumExposeTime(65.f);
        //long nCoaxialChannelidx = 11; //index는 0번부터 시작이니, 12번 Channel의 index는 11이다.
        for (long idx = 0; idx < UseChannelCount; idx++)
        {
            //float illumChannel_ms = illumJob().getIllumChannel_ms(nFrame, idx);

            strChannelName.Format(_T("%s"), channelNames[idx]);

            //{{//kircheis_LongExp
            static const BOOL bUseLongExposure = SystemConfig::GetInstance().m_bUseLongExposureNGRV;
            static const long nIRCh = SystemConfig::GetInstance().m_nNgrvIRchID;
            static const long nUVCh = SystemConfig::GetInstance().m_nNgrvUVchID;
            const BOOL bIsLongExposure = bUseLongExposure && (idx == nIRCh || idx == nUVCh);
            const float fExpLimit = bIsLongExposure ? 650.f : 65.f;
            const float fExpMinGap = bIsLongExposure ? 0.02f : 0.002f;
            //const float fCameraExpLimit
            //    = 20.f; // 카메라 때문에 발생하는 강제 노출시간 한계점 : 카메라 수정 될 때 까지 임시로 해놓은 Parameter
            const CString strText = bIsLongExposure ? _T("%.2lf ms") : _T("%.3lf ms");
            //}}

            if ((nFrame == Reverse_Frame && idx != Reverse_Channel) || (nFrame == UV_Frame && idx != nUVCh)
                || (nFrame == IR_Frame && idx != nIRCh) || (nFrame != Reverse_Frame && idx == Reverse_Channel)
                || (nFrame != UV_Frame && idx == nUVCh) || (nFrame != IR_Frame && idx == nIRCh) || idx == Channel_10
                || idx == Channel_11) // Reverse Frame을 쓰면서 Reverse Channel이 아니면 모두 Read Only로 바꾼다
            {
                if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                        (LPCTSTR)strChannelName, illumJob().getIllumChannel_ms(nFrame, idx), _T("Unuse"),
                        illumJob().getIllumChannel_ms(nFrame, idx), 0.f, 65.f, 0.002f)))
                {
                    item->SetID(enumGridItem_FirstID + idx);

                    item->SetReadOnly(TRUE);

                    if (auto* metric = item->GetMetrics(TRUE))
                    {
                        metric->m_clrFore = RGB(128, 128, 128);
                    }
                    if (auto* metric = item->GetMetrics(FALSE))
                    {
                        metric->m_clrFore = RGB(128, 128, 128);
                    }
                }
            }
            else
            {
                if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(
                        (LPCTSTR)strChannelName, illumJob().getIllumChannel_ms(nFrame, idx), strText,
                        illumJob().getIllumChannel_ms(nFrame, idx), 0.f, fExpLimit, fExpMinGap))) //kircheis_LongExp
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
                //if (idx == nUVCh || idx == nIRCh)
                //{
                //	fTotalExposureTime_EachFrame = illumChannel_ms;		// IR or UV는 한 채널만 쓰기 때문에 해당 노출 값을 바로 가져온다
                //	if (fTotalExposureTime_EachFrame <= fTotalExposureTimeMaxValue_UVIRFrame)	// IR or UV 채널에 입력한 노출 값이 최대 노출 값 보다 작거나 같으면 해당 채널의 최대 노출 값을 20msec으로 고정
                //	{
                //		fMaxExposureTime_EachChannel = 20.f;
                //	}
                //	else	// IR or UV 채널의 입력 노출 값이 최대 허용 노출 값(24msec)보다 클 경우, 알람을 띄우고 최대 값인 20msec로 변경해준다.
                //	{
                //		AfxMessageBox(_T("Used more than Maximum Exposure Time Value, please use exposure time not more than 20 msec total in IR/UV frame"));

                //		illumChannel_ms = fTotalExposureTimeMaxValue_UVIRFrame;
                //	}

                //	if (auto *item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>((LPCTSTR)strChannelName, illumJob().getIllumChannel_ms(nFrame, idx), strText, illumJob().getIllumChannel_ms(nFrame, idx), 0.f, fCameraExpLimit, fExpMinGap)))//kircheis_LongExp
                //	{
                //		item->SetID(enumGridItem_FirstID + idx);

                //		if (auto *metric = item->GetMetrics(TRUE))
                //		{
                //			metric->m_clrFore = channelColors[idx];
                //		}
                //		if (auto *metric = item->GetMetrics(FALSE))
                //		{
                //			metric->m_clrFore = channelColors[idx];
                //		}
                //	}
                //}
                //else
                //{
                //	if (idx == nCoaxialChannelidx)
                //		fMaximumExposeTime = 8.f; //TD 요청사항

                //	fTotalExposureTime_EachFrame += illumChannel_ms; // 노출시간 값 누적 계산

                //	if (fTotalExposureTime_EachFrame <= fTotalExposureTimeMaxValue_EachFrame)
                //	{
                //		fMaxExposureTime_EachChannel = 20.f;
                //	}
                //	else
                //	{
                //		AfxMessageBox(_T("Use more than Maximum Exposure Time Value, please use exposure time not more than 24 msec total in each frame"));	// 최대 값 보다 더 크기 때문에 알람을 띄움
                //		fTotalExposureTime_EachFrame -= illumChannel_ms;		// 계산되어진 누적 값에서 현재 채널의 입력 값을 뺀다
                //		fMaxExposureTime_EachChannel = (fTotalExposureTimeMaxValue_EachFrame - fTotalExposureTime_EachFrame) > 0 ? (fTotalExposureTimeMaxValue_EachFrame - fTotalExposureTime_EachFrame) : 0.f;

                //		illumChannel_ms = fMaxExposureTime_EachChannel;
                //	}

                //	float &fModifiedillumValue = illumChannel_ms;
                //	float fMaximumValue = idx == nCoaxialChannelidx ? fMaximumExposeTime : fMaxExposureTime_EachChannel;

                //	//if (auto *item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>((LPCTSTR)strChannelName, illumJob().getIllumChannel_ms(nFrame, idx), strText, illumJob().getIllumChannel_ms(nFrame, idx), 0.f, fMaxExposureTime_EachChannel, 0.002f)))
                //	if (auto *item = category->AddChildItem(new XTPPropertyGridItemCustomFloat<float>((LPCTSTR)strChannelName, illumJob().getIllumChannel_ms(nFrame, idx), strText, illumJob().getIllumChannel_ms(nFrame, idx), 0.f, fMaximumValue, 0.002f)))
                //	{
                //		item->SetID(enumGridItem_FirstID + idx);

                //		if (auto *metric = item->GetMetrics(TRUE))
                //		{
                //			metric->m_clrFore = channelColors[idx];
                //		}
                //		if (auto *metric = item->GetMetrics(FALSE))
                //		{
                //			metric->m_clrFore = channelColors[idx];
                //		}
                //	}
                //}
            }
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Turn off light"), TRUE, FALSE)))
        {
            item->SetID(enumGridItem_TurnOffLight);
        }

        category->Expand();
    }

    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->SetRedraw(TRUE);
    m_propertyGrid->Refresh();

    return TRUE;
}

void CDlgVisionIllumSetupNGRV::SetIllumParameter(long i_nFrame)
{
    i_nFrame = min(i_nFrame, illumJob().getTotalFrameCount());

    if (UpdateIllumChSetupUI(i_nFrame, illumJob().m_2D_colorFrame) == FALSE)
    {
        return;
    }

    if (m_visionUnit.m_systemConfig.IsHardwareExist())
    {
        SyncController::GetInstance().SetIllumiParameter(
            i_nFrame, illumJob().getIllum(i_nFrame), true, false, FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        SyncController::GetInstance().TurnOnLight(i_nFrame, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        FrameGrabber::GetInstance().live_on(
            GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

        long nProbePosZ = illumJob().isHighFrame(i_nFrame) ? _MULTI_GRAB_POS_HIGH : _MULTI_GRAB_POS_NORMAL;

        m_visionMain.iPIS_Send_MultiGrabProbePosZ(nProbePosZ);
    }
    else
    {
        long nFrameNum = (long)m_visionUnit.getImageLot().GetImageFrameCount();
        if (i_nFrame >= nFrameNum)
            i_nFrame = nFrameNum - 1;

        m_imageView->SetImage(m_visionUnit.getImageLot().GetColorImageFrame(i_nFrame));
        calcHistogramRGB(&m_imageView->GetImage_8u_C3());
    }

    m_nCurrentFrameIndex = i_nFrame;
}

void CDlgVisionIllumSetupNGRV::UpdateSpecToIllumView()
{
    m_NGRV_illumView.resetNGRVData();
    m_NGRV_illumView.addFrameGroup(0, _T(""), RGB(45, 79, 119), RGB(255, 255, 255));

    if (SystemConfig::GetInstance().GetHandlerType() != HANDLER_TYPE_380BRIDGE)
    {
        m_NGRV_illumView.addFrameGroup(1, _T(""), RGB(164, 201, 221), RGB(45, 79, 119));
        m_NGRV_illumView.addFrameGroup(2, _T(""), RGB(255, 96, 255), RGB(255, 255, 255));
        m_NGRV_illumView.addFrameGroup(3, _T(""), RGB(255, 100, 100), RGB(255, 255, 255));
    }

    //{{//kircheis_NGRV
    long nItemNum = (long)m_visionMain.m_vecGrabItemFrameInfo.size();

    for (long nItem = 0; nItem < nItemNum; nItem++)
    {
        //{{ 해당 검사를 사용하지 않으면 Item을 List에 올려놓을 필요가 없음 - JHB_NGRV
        if (!m_visionMain.m_vecGrabItemFrameInfo[nItem].m_bIsSaveImage)
        {
            continue;
        }
        //}}

        auto& grabInfo = m_visionMain.m_vecGrabItemFrameInfo[nItem];
        auto& vecnIllumInfo = grabInfo.m_vecnFrameID;

        if ((long)vecnIllumInfo.size() <= 0)
            vecnIllumInfo.push_back(0);

        m_NGRV_illumView.addItem(nItem, grabInfo.m_strGrabItemName, false);

        for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
        {
            long frameIndex = vecnIllumInfo[illumID];
            m_NGRV_illumView.addItemFrame(nItem, frameIndex);
        }
    }

    for (long frameIndex = 0; frameIndex < LED_ILLUM_FRAME_MAX; frameIndex++)
    {
        if (frameIndex < NORMAL_FRAME_NUM)
        {
            m_NGRV_illumView.setFrameGroup(frameIndex, 0);
        }
        else if (frameIndex + 1 == REVERSE_FRAME_NUM)
        {
            m_NGRV_illumView.setFrameGroup(frameIndex, 1);
        }
        else if (frameIndex + 1 == UV_FRAME_NUM)
        {
            m_NGRV_illumView.setFrameGroup(frameIndex, 2);
        }
        else if (frameIndex + 1 == IR_FRAME_NUM)
        {
            m_NGRV_illumView.setFrameGroup(frameIndex, 3);
        }
    }

    m_NGRV_illumView.setEditMode(m_visionUnit.m_systemConfig.m_nCurrentAccessMode >= _ENGINEER);
}

IllumInfo2D& CDlgVisionIllumSetupNGRV::illumJob()
{
    return m_visionMain.m_illum2D;
}

void CDlgVisionIllumSetupNGRV::callBack_roiChanged(
    void* userData, const int32_t id, const wchar_t* key, const size_t keyLength)
{
    UNREFERENCED_PARAMETER(id);
    UNREFERENCED_PARAMETER(key);
    UNREFERENCED_PARAMETER(keyLength);

    auto& image = ((CDlgVisionIllumSetupNGRV*)userData)->m_imageView->GetImage_8u_C3();
    ((CDlgVisionIllumSetupNGRV*)userData)->calcHistogramRGB(&image); // Histogram을 RGB 각각 확인 - JHB_NGRV
}

void CDlgVisionIllumSetupNGRV::callBack_changedSelectFrame(LPVOID userData, long realFrameIndex)
{
    ((CDlgVisionIllumSetupNGRV*)userData)->callBack_changedSelectFrame(realFrameIndex);
}

void CDlgVisionIllumSetupNGRV::callBack_changedSelectItem(LPVOID userData, long itemID)
{
    ((CDlgVisionIllumSetupNGRV*)userData)->callBack_changedSelectItem(itemID);
}

void CDlgVisionIllumSetupNGRV::callBack_changedItemFrame(
    LPVOID userData, long itemID, long realFrameIndex, bool checked)
{
    ((CDlgVisionIllumSetupNGRV*)userData)->callBack_changedItemFrame(itemID, realFrameIndex, checked);
}

void CDlgVisionIllumSetupNGRV::callBack_deletedFrame(LPVOID userData, long realFrameIndex, bool isBackup)
{
    ((CDlgVisionIllumSetupNGRV*)userData)->callBack_deletedFrame(realFrameIndex, isBackup);
}

void CDlgVisionIllumSetupNGRV::callBack_insertedFrame(LPVOID userData, long groupID, long realFrameIndex, bool isBackup)
{
    ((CDlgVisionIllumSetupNGRV*)userData)->callBack_insertedFrame(groupID, realFrameIndex, isBackup);
}

void CDlgVisionIllumSetupNGRV::callBack_changedSelectFrame(long realFrameIndex)
{
    static const bool isNgrvColorOptics = SystemConfig::GetInstance().IsNgrvColorOptics();

    if (m_visionUnit.m_systemConfig.IsHardwareExist() && isNgrvColorOptics == true
        && (realFrameIndex == NGRV_FrameID_IR || realFrameIndex == NGRV_FrameID_UV || m_bUseGain))
    {
        if ((realFrameIndex == NGRV_FrameID_IR) || (realFrameIndex == NGRV_FrameID_UV)) // UV/IR을 사용하는 경우
        {
            if ((FrameGrabber::GetInstance().GetCurrentCameraGain() == SystemConfig::GetInstance().m_fIR_Camera_Gain)
                || (FrameGrabber::GetInstance().GetCurrentCameraGain()
                    == SystemConfig::GetInstance().m_fUV_Camera_Gain))
            {
                // 이전에 저장된 Camera Gain이 같으면 Gain 값을 Set해주지 않아도 됨 - JHB_2023.02.07
            }
            else
            {
                switch (realFrameIndex)
                {
                    case NGRV_FrameID_IR:
                        FrameGrabber::GetInstance().SetDigitalGainValue(
                            SystemConfig::GetInstance().m_fIR_Camera_Gain, FALSE);
                        m_bUseGain = TRUE;
                        break;

                    case NGRV_FrameID_UV:
                        FrameGrabber::GetInstance().SetDigitalGainValue(
                            SystemConfig::GetInstance().m_fUV_Camera_Gain, FALSE);
                        m_bUseGain = TRUE;
                        break;
                }
            }
        }
        else // 일반 채널을 사용하는 경우
        {
            if (FrameGrabber::GetInstance().GetCurrentCameraGain()
                == SystemConfig::GetInstance().m_fDefault_Camera_Gain)
            {
                // 이전에 저장된 Camera Gain이 같으면 Gain 값을 Set해주지 않아도 됨 - JHB_2023.02.07
            }
            else
            {
                FrameGrabber::GetInstance().SetDigitalGainValue(
                    SystemConfig::GetInstance().m_fDefault_Camera_Gain, FALSE);
                m_bUseGain = FALSE;
            }
        }
    }

    //{{ Reverse Channel을 사용할 때, Live에도 영향을 주기 때문에 확인한 후, 올바른 White Balance Gain 값을 부여하기 위한 판별 변수
    if (m_visionUnit.m_systemConfig.IsHardwareExist() && isNgrvColorOptics == true
        && (realFrameIndex == NGRV_FrameID_Reverse))
    {
        m_visionUnit.m_systemConfig.m_bUseReverseChannel = TRUE;
    }
    else
    {
        m_visionUnit.m_systemConfig.m_bUseReverseChannel = FALSE;
    }
    //}}

    SetIllumParameter(realFrameIndex);
}

void CDlgVisionIllumSetupNGRV::callBack_changedSelectItem(long /*itemID*/)
{
}

void CDlgVisionIllumSetupNGRV::callBack_changedItemFrame(long itemID, long realFrameIndex, bool checked)
{
    long nItemNum = (long)m_visionMain.m_vecGrabItemFrameInfo.size();

    if (itemID < nItemNum)
    {
        auto& grabInfo = m_visionMain.m_vecGrabItemFrameInfo[itemID];
        auto& vecnIllumInfo = grabInfo.m_vecnFrameID;

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
        switch (itemID - nItemNum)
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
        SetIllumParameter(realFrameIndex);
    }
}

void CDlgVisionIllumSetupNGRV::callBack_deletedFrame(long realFrameIndex, bool isBackup)
{
    long nItemNum = (long)m_visionMain.m_vecGrabItemFrameInfo.size();

    if (isBackup)
    {
        m_backupInfo_name = illumJob().getIllumName(realFrameIndex);
        m_backupInfo_items.clear();
        m_backupInfo_items_inspFrames.clear();

        for (long n = 0; n < LED_ILLUM_NGRV_CHANNEL_MAX; n++)
        {
            m_backupInfo_illuminations_ms[n] = illumJob().getIllumChannel_ms(realFrameIndex, n);
        }
    }

    for (long itemIndex = 0; itemIndex < nItemNum; itemIndex++)
    {
        auto& grabInfo = m_visionMain.m_vecGrabItemFrameInfo[itemIndex];
        auto& vecnIllumInfo = grabInfo.m_vecnFrameID;

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

        for (long illumID = 0; illumID < (long)vecnIllumInfo.size(); illumID++)
        {
            if (vecnIllumInfo[illumID] > realFrameIndex)
            {
                vecnIllumInfo[illumID]--;
            }
        }
    }

    inspFrameInvalidCheck();
}

void CDlgVisionIllumSetupNGRV::callBack_insertedFrame(long groupID, long realFrameIndex, bool isBackup)
{
    long nItemNum = (long)m_visionMain.m_vecGrabItemFrameInfo.size();

    if (isBackup)
    {
        illumJob().insertFrame(realFrameIndex, (groupID == 1), m_backupInfo_name);
        for (long n = 0; n < LED_ILLUM_NGRV_CHANNEL_MAX; n++)
        {
            illumJob().getIllumChannel_ms(realFrameIndex, n) = m_backupInfo_illuminations_ms[n];
        }
    }
    else
    {
        illumJob().insertFrame(realFrameIndex, false, _T(""));
    }

    for (long itemIndex = 0; itemIndex < nItemNum; itemIndex++)
    {
        auto& grabInfo = m_visionMain.m_vecGrabItemFrameInfo[itemIndex];
        auto& vecnIllumInfo = grabInfo.m_vecnFrameID;

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

void CDlgVisionIllumSetupNGRV::inspFrameInvalidCheck()
{
    // 혹시 검사 프레임중 프레임 삭제등의 이유로 선택이 안된 것이 있다면 재정리해 주자

    /*long procssingCount = m_visionUnit.GetVisionProcessingCount();
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
	}*/
}

void CDlgVisionIllumSetupNGRV::calcHistogram(const Ipvm::Image8u* image)
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
}

LRESULT CDlgVisionIllumSetupNGRV::OnGrabEndEvent(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    FrameGrabber::GetInstance().get_live_image(*m_grabImage);
    FrameGrabber::GetInstance().get_live_image(*m_bayerImage);

    //m_imageView->SetImage(*m_grabImage);
    m_imageView->SetImage(*m_bayerImage);

    calcHistogram(m_grabImage);

    return 1L;
}

LRESULT CDlgVisionIllumSetupNGRV::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        const long channel = item->GetID() - enumGridItem_FirstID;

        auto* value = dynamic_cast<XTPPropertyGridItemCustomFloat<float>*>(item);

        if (item->GetID() == enumGridItem_UseColorFrame)
        {
            UpdateSpecToIllumView();

            //{{ //kircheis_WB
            auto& sysConfig = SystemConfig::GetInstance();
            if (illumJob().m_2D_colorFrame && sysConfig.m_vecIllumFittingRef.size() < 10)
                ::SimpleMessage(_T("Calibration of the illumination is required to facilitate color image creation."));
            //}}
        }
        else if (item->GetID() == enumGridItem_TurnOffLight)
        {
            FrameGrabber::GetInstance().live_off();
            SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        }
        else if (item->GetID() == enumGridItem_GrabMakeColor) //kircheis_WB
        {
            if (SystemConfig::GetInstance().m_bHardwareExist)
            {
                FrameGrabber::GetInstance().live_off();
                SyncController::GetInstance().TurnOffLight(enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            }

            GrabAndDisplayColorImage();
        }
        else if (value)
        {
            SyncController::GetInstance().SetIllumiParameter(m_nCurrentFrameIndex, channel,
                illumJob().getIllumChannel_ms(m_nCurrentFrameIndex, channel), true, false, FALSE,
                enSideVisionModule::SIDE_VISIONMODULE_FRONT);
            SyncController::GetInstance().TurnOnLight(
                m_nCurrentFrameIndex, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

            if (SystemConfig::GetInstance().m_bHardwareExist
                && FrameGrabber::GetInstance().IsLive() == FALSE) //kircheis_WB
                FrameGrabber::GetInstance().live_on(
                    GetSafeHwnd(), UM_GRAB_END_EVENT, 500L, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

            long nProbePosZ
                = illumJob().isHighFrame(m_nCurrentFrameIndex) ? _MULTI_GRAB_POS_HIGH : _MULTI_GRAB_POS_NORMAL;

            m_visionMain.iPIS_Send_MultiGrabProbePosZ(nProbePosZ);
            UpdateIllumChSetupUI(m_nCurrentFrameIndex, illumJob().m_2D_colorFrame, channel); //kircheis_WB
        }
    }

    return 0;
}

void CDlgVisionIllumSetupNGRV::GrabAndDisplayColorImage() //kircheis_WB
{
    if (illumJob().m_2D_colorFrame == FALSE)
        return;

    long nFrameIDs[3];
    nFrameIDs[COLOR_RED] = illumJob().m_2D_colorFrameIndex_red;
    nFrameIDs[COLOR_GREEN] = illumJob().m_2D_colorFrameIndex_green;
    nFrameIDs[COLOR_BLUE] = illumJob().m_2D_colorFrameIndex_blue;

    auto& imageLot = m_visionUnit.getImageLot();
    BYTE* imageMems[3];
    if (SystemConfig::GetInstance().m_bHardwareExist == FALSE)
    {
        long nImageNum = (long)imageLot.GetImageFrameCount();
        if (nImageNum < 1)
            return;

        Ipvm::Image8u3 imageColor(imageLot.GetImageSizeX(), imageLot.GetImageSizeY());
        for (long i = COLOR_BLUE; i <= COLOR_RED; i++)
            nFrameIDs[i] = (long)min(nImageNum - 1, nFrameIDs[i]);

        Ipvm::ImageProcessing::CombineRGB(
            imageLot.GetImageFrame(nFrameIDs[COLOR_RED], enSideVisionModule::SIDE_VISIONMODULE_FRONT),
            imageLot.GetImageFrame(nFrameIDs[COLOR_GREEN], enSideVisionModule::SIDE_VISIONMODULE_FRONT),
            imageLot.GetImageFrame(nFrameIDs[COLOR_BLUE], enSideVisionModule::SIDE_VISIONMODULE_FRONT),
            Ipvm::Rect32s(imageColor), imageColor);
        m_imageView->SetImage(imageColor);

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

    SyncController::GetInstance().SetFrameCount(1, 0, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    for (long nColor = COLOR_BLUE; nColor <= COLOR_RED; nColor++)
    {
        SyncController::GetInstance().SetIllumiParameter(
            0, illumJob().getIllum(nFrameIDs[nColor]), true, false, FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        Sleep(10);
        FrameGrabber::GetInstance().StartGrab2D(0, 0, &imageMems[nColor], 1, nullptr);
        SyncController::GetInstance().StartSyncBoard(FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);
        FrameGrabber::GetInstance().wait_grab_end(0, 0, nullptr);
        Sleep(100);
    }
    SyncController::GetInstance().SetIllumiParameter(
        0, illumJob().getIllum(0), true, false, FALSE, enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    Ipvm::ImageProcessing::CombineRGB(imageRed, imageGreen, imageBlue, Ipvm::Rect32s(imageRed), imageColor);

    m_imageView->SetImage(imageColor);
}

BOOL CDlgVisionIllumSetupNGRV::callJobOpen()
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
        return false;
    }

    illumJob() = illumLoad;

    UpdateSpecToIllumView();
    SetIllumParameter(-1);

    return TRUE;
}

BOOL CDlgVisionIllumSetupNGRV::callJobSave()
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

    return TRUE;
}

void CDlgVisionIllumSetupNGRV::IllumLink(LPCTSTR filePath, bool save, IllumInfo2D& io_illum)
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

        for (long channel = 0; channel < LED_ILLUM_NGRV_CHANNEL_MAX; channel++)
        {
            CString key;
            key.Format(_T("Channel%d"), channel + 1);

            IniHelper::Link(save, filePath, category, key, &frameIllum[channel]);
        }
    }
}

void CDlgVisionIllumSetupNGRV::calcHistogramRGB(const Ipvm::Image8u3* image)
{
    Ipvm::Rect32s roi;
    m_imageView->ROIGet(_T("Histogram area"), roi);

    Ipvm::Rect32s validRoi = roi & Ipvm::Rect32s(*image);

    Ipvm::Image8u imgRed(image->GetSizeX(), image->GetSizeY());
    Ipvm::Image8u imgGreen(image->GetSizeX(), image->GetSizeY());
    Ipvm::Image8u imgBlue(image->GetSizeX(), image->GetSizeY());

    auto& imageRed = imgRed;
    auto& imageGreen = imgGreen;
    auto& imageBlue = imgBlue;

    if (validRoi.IsRectEmpty())
    {
        m_histogramRed->FillZero();
        m_histogramGreen->FillZero();
        m_histogramBlue->FillZero();
    }
    else
    {
        Ipvm::ImageProcessing::SplitRGB(
            *image, Ipvm::Rect32s(*image), imageRed, imageGreen, imageBlue); // Color Image Split RGB

        Ipvm::ImageProcessing::MakeHistogram(imageRed, validRoi, *m_histogramRed);
        Ipvm::ImageProcessing::MakeHistogram(imageGreen, validRoi, *m_histogramGreen);
        Ipvm::ImageProcessing::MakeHistogram(imageBlue, validRoi, *m_histogramBlue);
    }

    std::vector<Ipvm::Point32r2> vecPointDataRed(256);
    std::vector<Ipvm::Point32r2> vecPointDataGreen(256);
    std::vector<Ipvm::Point32r2> vecPointDataBlue(256);

    double dSumRed = 0.;
    double dSumGreen = 0.;
    double dSumBlue = 0.;

    for (long idx = 0; idx < 256; idx++)
    {
        vecPointDataRed[idx].m_x = (float)idx;
        vecPointDataGreen[idx].m_x = (float)idx;
        vecPointDataBlue[idx].m_x = (float)idx;

        vecPointDataRed[idx].m_y = (float)(*m_histogramRed->GetMem(idx, 0));
        vecPointDataGreen[idx].m_y = (float)(*m_histogramGreen->GetMem(idx, 0));
        vecPointDataBlue[idx].m_y = (float)(*m_histogramBlue->GetMem(idx, 0));

        dSumRed += vecPointDataRed[idx].m_x * vecPointDataRed[idx].m_y;
        dSumGreen += vecPointDataGreen[idx].m_x * vecPointDataGreen[idx].m_y;
        dSumBlue += vecPointDataBlue[idx].m_x * vecPointDataBlue[idx].m_y;
    }

    if (validRoi.Width() * validRoi.Height() > 0)
    {
        dSumRed /= validRoi.Width() * validRoi.Height();
        dSumGreen /= validRoi.Width() * validRoi.Height();
        dSumBlue /= validRoi.Width() * validRoi.Height();
    }

    m_profileView->SetData(0, &vecPointDataGreen[0], 256, -FLT_MAX, RGB(255, 255, 255), RGB(200, 200, 200));

    CString str;
    str.Format(_T("R:%.2lf, G:%.2lf, B:%.2lf"), dSumRed, dSumGreen, dSumBlue);

    m_imageView->WindowOverlayClear();
    m_imageView->WindowOverlayAdd(Ipvm::Point32s2(0, 20), str, RGB(255, 0, 0), 32);
    m_imageView->WindowOverlayShow();
}