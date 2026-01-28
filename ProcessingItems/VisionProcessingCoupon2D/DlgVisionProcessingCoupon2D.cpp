//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingCoupon2D.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingCoupon2D.h"
#include "VisionProcessingCoupon2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_CHANGED (WM_USER + 1011)
#define UM_ROI_CHANGED (WM_USER + 1012)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_RAW (WM_USER + 1013)
#define UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE (WM_USER + 1014)

//CPP_7_________________________________ Implementation body
//
enum enumCouponAlignParamID
{
    COUPON_PARAM_START = 1,
    COUPON_PARAM_IMAGECOMBINE = COUPON_PARAM_START,
    COUPON_THRESHOLD,
    COUPON_MIN_BLOB_SIZE,
    COUPON_MAX_BLOB_COUNT,
    COUPON_BLOB_RESULT_TEXT,
    COUPON_MAX_BLOB_SIZE,
    COUPON_PARAM_END,
};

IMPLEMENT_DYNAMIC(CDlgVisionProcessingCoupon2D, CDialog)

CDlgVisionProcessingCoupon2D::CDlgVisionProcessingCoupon2D(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingCoupon2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionProcessingCoupon2D::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionInsp->ResetResult(); //kircheis_2DMatVMSDK
    m_pVisionPara = m_pVisionInsp->m_VisionPara;

    m_nImageID = 0;
}

CDlgVisionProcessingCoupon2D::~CDlgVisionProcessingCoupon2D()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_imageLotView;
    delete m_propertyGrid;
}

void CDlgVisionProcessingCoupon2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgVisionProcessingCoupon2D, CDialog)
//	ON_WM_CREATE()
//	ON_WM_CLOSE()
ON_WM_DESTROY()
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
//	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &DlgVisionProcessingAlign2D::OnTcnSelchangeTabResult)
//	ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_CHANGED, &DlgVisionProcessingAlign2D::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_ROI_CHANGED, &CDlgVisionProcessingCoupon2D::OnImageLotViewRoiChanged)
END_MESSAGE_MAP()

// CDlgVisionProcessingCoupon2D 메시지 처리기입니다.

BOOL CDlgVisionProcessingCoupon2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, false, false, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_CHANGED);
    m_imageLotView->SetMessage_SelChangedRaw(UM_IMAGE_LOT_VIEW_SELCHANGED_RAW);
    m_imageLotView->SetMessage_SelChangedImage(UM_IMAGE_LOT_VIEW_SELCHANGED_IMAGE);

    m_propertyGrid = new CXTPPropertyGrid;
    CRect rtPara = m_procDlgInfo.m_rtParaArea;

    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, CAST_UINT(IDC_STATIC));

    // Dialog Control 위치 설정
    UpdatePropertyGrid();

    CRect rtTab = m_procDlgInfo.m_rtDataArea;

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);

    // ROI 표시
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), TRUE);
    m_imageLotView->ZoomImageFit();

    // ROI가 초기값일 경우 1st 프레임 센터로 초기화
    if (m_pVisionPara->m_CouponROI.m_bottom == -1)
    {
        m_pVisionPara->m_CouponROI = Ipvm::Rect32s(1280, 1280, 3840, 3840);
    }

    SetROI();
    ShowThresholdImage(true);

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionProcessingCoupon2D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionProcessingCoupon2D::GetROI()
{
    CString strTemp;
    strTemp.Format(_T("Coupon Check ROI"));
    m_imageLotView->ROI_Get(strTemp, m_pVisionInsp->m_VisionPara->m_CouponROI);
}

void CDlgVisionProcessingCoupon2D::SetROI()
{
    CString strTemp;
    strTemp.Format(_T("Coupon Check ROI"));
    m_imageLotView->ROI_Add(strTemp, strTemp, m_pVisionPara->m_CouponROI, RGB(255, 0, 0), TRUE, TRUE);

    m_imageLotView->ROI_Show(TRUE);
}

void CDlgVisionProcessingCoupon2D::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Image Select parameters")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(COUPON_PARAM_IMAGECOMBINE);

        //m_pVisionPara->m_calcFrameIndex.setFrameIndex(m_pVisionPara->m_calcFrameIndex.getFrameIndex());
        //m_pVisionPara->m_calcFrameIndex.makePropertyGridItem(category, _T("Select Frame ID"), COUPON_PARAM_FRAMENUM);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Blob paramters")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Image Threshold"), m_pVisionPara->m_nCouponThreshold, &m_pVisionPara->m_nCouponThreshold)))
        {
            if (auto* slider = item->AddSliderControl())
            {
                slider->SetMin(0);
                slider->SetMax(255);
            }

            if (auto* spin = item->AddSpinButton())
            {
                spin->SetMin(0);
                spin->SetMax(255);
            }
            item->SetID(COUPON_THRESHOLD);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Min blob size"), m_pVisionPara->m_nMinBlobSize, &m_pVisionPara->m_nMinBlobSize)))
        {
            item->SetID(COUPON_MIN_BLOB_SIZE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Min blob count"), m_pVisionPara->m_nMinBlobCount, &m_pVisionPara->m_nMinBlobCount)))
        {
            item->SetID(COUPON_MAX_BLOB_COUNT);
        }

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Blob results")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItem(
                _T("Blob Result text"), m_strBlobResultText, &m_strBlobResultText))) // 해당하는 데이터로 변경 필요
        {
            item->SetID(COUPON_BLOB_RESULT_TEXT);
            item->SetReadOnly(TRUE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItem(
                _T("Max blob size"), m_strMaxBlobSizeText, &m_strMaxBlobSizeText))) // 해당하는 데이터로 변경 필요
        {
            item->SetID(COUPON_MAX_BLOB_SIZE);
            item->SetReadOnly(TRUE);
        }
        category->Expand();
    }

    SetROI();
    ShowThresholdImage(true);

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionProcessingCoupon2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CString strTemp;
    //SystemConfig& systemConfig = m_pVisionInsp->m_visionUnit.m_systemConfig;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        //long nSearchLowLimit_um = 500;
        //const long nSearchHighLimit_um = 2000;

        switch (item->GetID())
        {
            // ImageCombine 사용
            case COUPON_PARAM_IMAGECOMBINE:
            {
                ClickedButtonImageCombine();
                break;
            }

            // Threshold 사용하여 데이터 저장
            case COUPON_THRESHOLD:
            {
                ShowThresholdImage(true);
                break;
            }
        }
    }

    strTemp.Empty();

    SetROI();
    // 이진화 이미지를 내보내면 Image combine이 잘 되었는지 확인 불가능? >> 이진화 이미지 바로 필요하면 주석 해제
    //ShowThresholdImage(true);

    return 0;
}

void CDlgVisionProcessingCoupon2D::ClickedButtonImageCombine()
{
    auto* proc = m_pVisionInsp;

    if (::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), true, &proc->m_VisionPara->m_ImageProcManagePara)
        == IDOK)
    {
        ////////////////////////
        auto processor = m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_COUPON_2D);
        auto& memory = processor->getReusableMemory();

        // Raw 이미지를 받아온다.
        auto frameIndex = m_pVisionInsp->GetImageFrameIndex(0);
        m_pVisionPara->m_calcFrameIndex.setFrameIndex(frameIndex);
        const auto& image = m_pVisionInsp->getImageLot().GetImageFrame(
            m_pVisionPara->m_calcFrameIndex.getFrameIndex(), m_pVisionInsp->GetCurVisionModule_Status());
        if (image.GetMem() == nullptr)
        {
            return;
        }

        Ipvm::Image8u combineImage;
        Ipvm::Rect32s rtROI = m_pVisionInsp->m_VisionPara->m_CouponROI;

        // 이미지 raw로 받아옴
        if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
            return;

        // Combine 이미지 생성 > 기존 값은 원본 이미지로 하였다.
        Ipvm::ImageProcessing::Copy(image, Ipvm::Rect32s(image), combineImage);
        Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());

        // 이미지 combine 해서 결과 표시
        if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, true, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_ImageProcManagePara, combineImage))
        {
            return;
        }

        // 이미지 표시
        m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), TRUE);
        m_imageLotView->SetImage(combineImage, _T("Combined Image **"));
    }
}

LRESULT CDlgVisionProcessingCoupon2D::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    // ROI 수령
    GetROI();
    Ipvm::Rect32s rtROI = m_pVisionInsp->m_VisionPara->m_CouponROI;
    ShowThresholdImage(true);
    SetROI();

    return 0L;
}

void CDlgVisionProcessingCoupon2D::ShowThresholdImage(bool thresholdDisplay)
{
    // ROI를 움직였을때 ThresholdImage가 나오도록 만들기 위한 함수
    auto processor = m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_COUPON_2D);
    auto& memory = processor->getReusableMemory();

    // Raw 이미지를 받아온다.
    auto frameIndex = m_pVisionInsp->GetImageFrameIndex(0);
    m_pVisionPara->m_calcFrameIndex.setFrameIndex(frameIndex);
    const auto& image = m_pVisionInsp->getImageLot().GetImageFrame(
        m_pVisionPara->m_calcFrameIndex.getFrameIndex(), m_pVisionInsp->GetCurVisionModule_Status());
    if (image.GetMem() == nullptr)
    {
        return;
    }

    // 영상처리에 사용할 버퍼 생성
    Ipvm::Image8u combineImage;
    Ipvm::Image8u thresholdImage;
    Ipvm::Rect32s rtROI = m_pVisionInsp->m_VisionPara->m_CouponROI;

    if (!memory.GetByteImage(combineImage, image.GetSizeX(), image.GetSizeY()))
        return;
    if (!memory.GetByteImage(thresholdImage, image.GetSizeX(), image.GetSizeY()))
        return;

    // ROI 설정
    Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());

    // 이미지 combine 해서 결과 표시

    if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, true, rtProcessingROI,
            m_pVisionInsp->m_VisionPara->m_ImageProcManagePara, combineImage))
    {
        return;
    }

    // combine 이미지로 변경
    Ipvm::ImageProcessing::Copy(combineImage, Ipvm::Rect32s(image), thresholdImage);

    // 영상 이진화
    if (thresholdDisplay)
    {
        Ipvm::ImageProcessing::BinarizeGreaterEqual(
            combineImage, rtROI, CAST_UINT8T(m_pVisionInsp->m_VisionPara->m_nCouponThreshold), thresholdImage);
    }

    // 이미지 표시
    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), TRUE);
    m_imageLotView->SetImage(thresholdImage, _T("Threshold Image **"));
}

void CDlgVisionProcessingCoupon2D::AfterInspect()
{
    // 디버그 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();

    // Blob 결과를 바탕으로 쿠폰인지 Pass인지 여부를 판단한다.
    m_strBlobResultText = m_pVisionInsp->m_nBlobResult ? _T("COUPON") : _T("PASS");
    m_strMaxBlobSizeText.Format(_T("%d"), m_pVisionInsp->m_nMaxBlobSize);
    m_propertyGrid->FindItem(COUPON_BLOB_RESULT_TEXT)->SetValue(m_strBlobResultText);
    m_propertyGrid->FindItem(COUPON_MAX_BLOB_SIZE)->SetValue(m_strMaxBlobSizeText);

    // Blob된 ROI들을 추가해준다.
    for (auto& roi : m_pVisionInsp->m_vecrtBlobROI)
    {
        m_imageLotView->Overlay_AddRectangle(roi, m_pVisionInsp->m_nBlobResult ? RGB(255, 0, 0) : RGB(0, 255, 0));
    }

    m_imageLotView->Overlay_Show(TRUE);
}