//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionMarkOperator.h"

//CPP_2_________________________________ This project's headers
#include "DlgMarkTeachResultViewer.h" //kircheis_HwaMark
#include "DlgVisionInspectionMark.h"
#include "InspResult.h"
#include "VisionInspectionMark.h"
#include "VisionInspectionMarkPara.h"
#include "VisionInspectionMarkSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Widget/ImageView.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum enumPID
{
    enumPID_SelectUseMap = 1,
    enumPID_MarkImportMap,
    enumPID_CharMergeBoxNumer,
    enumPID_CharIgnoreBoxNumber,
    enumPID_TeachPara_WhiteBackground,
    enumPID_TeachPara_Threshold,
    enumPID_ButtonTeachSplitChar,
    enumPID_ButtonTeach,
    enumPID_ButtonTest,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionMarkOperator, CDialog)

CDlgVisionInspectionMarkOperator::CDlgVisionInspectionMarkOperator(const VisionScale& scale, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionMarkOperator::IDD, pParent)
    , m_scale(scale)
    , m_propertyGrid(nullptr)
    , m_nTeachSlot(0)
{
    m_pVisionInspDlg = (CDlgVisionInspectionMark*)pParent;
    m_pVisionPara = m_pVisionInspDlg->m_pVisionInsp->m_VisionPara;

    for (long idx = 0; idx < NUM_OF_MARKMULTI; idx++)
    {
        m_imageViews[idx] = nullptr;
    }

    m_bBtnSplitePush = FALSE;
}

CDlgVisionInspectionMarkOperator::~CDlgVisionInspectionMarkOperator()
{
    for (long idx = 0; idx < NUM_OF_MARKMULTI; idx++)
    {
        delete m_imageViews[idx];
    }

    delete m_propertyGrid;
}

void CDlgVisionInspectionMarkOperator::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RADIO_MULTI_TEACHING_SLOT_1, m_BtnSelectedSpec);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionMarkOperator, CDialog)
ON_WM_HSCROLL()

ON_BN_CLICKED(IDC_BTN_REMOVE_SLOT_2, &CDlgVisionInspectionMarkOperator::OnBnClickedButtonRemoveSlot2)
ON_BN_CLICKED(IDC_BTN_REMOVE_SLOT_3, &CDlgVisionInspectionMarkOperator::OnBnClickedButtonRemoveSlot3)
ON_BN_CLICKED(IDC_RADIO_MULTI_TEACHING_SLOT_1, &CDlgVisionInspectionMarkOperator::OnBnClickedRadioMultiTeachingSlot1)
ON_BN_CLICKED(IDC_RADIO_MULTI_TEACHING_SLOT_2, &CDlgVisionInspectionMarkOperator::OnBnClickedRadioMultiTeachingSlot2)
ON_BN_CLICKED(IDC_RADIO_MULTI_TEACHING_SLOT_3, &CDlgVisionInspectionMarkOperator::OnBnClickedRadioMultiTeachingSlot3)

ON_WM_SIZE()
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

void CDlgVisionInspectionMarkOperator::SetPoint(Ipvm::Point32s2 ptDispoint)
{
    if (m_bBtnSplitePush)
    {
        m_vecptSplitePoint.push_back(ptDispoint);
        SetROI(m_pVisionPara->m_nROISettingMethod);
    }
    else
        return;
}

BOOL CDlgVisionInspectionMarkOperator::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(CRect(0, 0, 0, 0), this, 65535);

    m_nTeachSlot = 0;

    for (long idx = 0; idx < NUM_OF_MARKMULTI; idx++)
    {
        m_imageViews[idx] = new Ipvm::ImageView(GetSafeHwnd(), Ipvm::Rect32s());
    }

    Ipvm::Point32r2 imageCenter(m_pVisionInspDlg->m_pVisionInsp->getReusableMemory().GetInspImageSizeX() * 0.5f,
        m_pVisionInspDlg->m_pVisionInsp->getReusableMemory().GetInspImageSizeY() * 0.5f);

    for (long nMultiID = 0; nMultiID < NUM_OF_MARKMULTI; nMultiID++)
    {
        auto* spec = m_pVisionInspDlg->m_pVisionInsp->m_VisionSpec[nMultiID];

        BOOL bTeachDone = spec->m_bTeachDone;
        if (bTeachDone == TRUE && spec->m_specImage.GetMem() != nullptr)
        {
            Ipvm::Image8u specImage = spec->m_specImage;
            Ipvm::Rect32s rtTeachROI = m_scale.convert_BCUToPixel(spec->m_rtMarkTeachROI_BCU, imageCenter);

            m_imageViews[nMultiID]->SetImage(Ipvm::Image8u(specImage, rtTeachROI));
            m_imageViews[nMultiID]->ZoomImageFit();
        }
    }

    m_BtnSelectedSpec.SetCheck(TRUE);

    generatePropertyGrid();

    InitializeBtnCtrl(); //Simple Inspection일때는 Mulit Teaching 및 나머지 기능은 사용못하도록 한다.

    UpdateData(FALSE);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgVisionInspectionMarkOperator::InitializeBtnCtrl()
{
    // 영훈 20150106_UseMarkMultiTeaching : Mark Multi Teacing 사용 여부에 따라 화면에 표시해주는 정보를 달리 한다.
    if (SystemConfig::GetInstance().m_bUseMarkMultiTeaching == FALSE)
    {
        GetDlgItem(IDC_BTN_REMOVE_SLOT_2)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_REMOVE_SLOT_3)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO_MULTI_TEACHING_SLOT_2)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO_MULTI_TEACHING_SLOT_3)->EnableWindow(FALSE);

        ::EnableWindow(m_imageViews[1]->GetSafeHwnd(), FALSE);
        ::EnableWindow(m_imageViews[2]->GetSafeHwnd(), FALSE);
    }

    if (m_pVisionPara->m_nMarkInspMode == MarkInspectionMode_Simple)
    {
        GetDlgItem(IDC_BTN_REMOVE_SLOT_2)->EnableWindow(FALSE);
        GetDlgItem(IDC_BTN_REMOVE_SLOT_3)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO_MULTI_TEACHING_SLOT_2)->EnableWindow(FALSE);
        GetDlgItem(IDC_RADIO_MULTI_TEACHING_SLOT_3)->EnableWindow(FALSE);

        m_propertyGrid->FindItem(enumPID_ButtonTeachSplitChar)->SetReadOnly(TRUE);
        m_propertyGrid->FindItem(enumPID_ButtonTeach)->SetReadOnly(TRUE);

        ::EnableWindow(m_imageViews[1]->GetSafeHwnd(), FALSE);
        ::EnableWindow(m_imageViews[2]->GetSafeHwnd(), FALSE);
    }
    else
    {
        GetDlgItem(IDC_BTN_REMOVE_SLOT_2)->EnableWindow(TRUE);
        GetDlgItem(IDC_BTN_REMOVE_SLOT_3)->EnableWindow(TRUE);
        GetDlgItem(IDC_RADIO_MULTI_TEACHING_SLOT_2)->EnableWindow(TRUE);
        GetDlgItem(IDC_RADIO_MULTI_TEACHING_SLOT_3)->EnableWindow(TRUE);

        m_propertyGrid->FindItem(enumPID_ButtonTeachSplitChar)->SetReadOnly(FALSE);
        m_propertyGrid->FindItem(enumPID_ButtonTeach)->SetReadOnly(FALSE);

        ::EnableWindow(m_imageViews[1]->GetSafeHwnd(), TRUE);
        ::EnableWindow(m_imageViews[2]->GetSafeHwnd(), TRUE);
    }

    generatePropertyGrid_teachPara();
}

void CDlgVisionInspectionMarkOperator::SetMultiImage()
{
    const int nSizeX = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const int nSizeY = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    auto* spec = getCurrentSpec();
    BOOL bTeachDone = spec->m_bTeachDone;
    Ipvm::Image8u specImage = spec->m_specImage;
    Ipvm::Rect32s rtTeachROI = m_scale.convert_BCUToPixel(spec->m_rtMarkTeachROI_BCU, imageCenter);

    if (bTeachDone == TRUE) //초기화가 안된 쓰레기값이 들어갈때가있다..
    {
        //{{ //kircheis_MarkTeachImageBug
        long nWidth = rtTeachROI.Width();
        long nHeight = rtTeachROI.Height();
        long nGap = (nHeight - nWidth) / 2;
        float fRatio = (float)nHeight / (float)nWidth;
        static float fCutoffRatio = 1.2f;
        static BOOL bResizeROI = TRUE;

        if (bResizeROI && (fRatio > fCutoffRatio))
        {
            rtTeachROI.InflateRect(nGap, 0);
            Ipvm::Rect32s rtFOV(0, 0, nSizeX - 1, nSizeY - 1);
            rtTeachROI &= rtFOV;
        }
        //}}

        m_imageViews[m_nTeachSlot]->SetImage(Ipvm::Image8u(specImage, rtTeachROI));
        m_imageViews[m_nTeachSlot]->ZoomImageFit();
    }
}

void CDlgVisionInspectionMarkOperator::ReSetMarkSlot(long nMarkSlot)
{
    auto* spec = m_pVisionInspDlg->m_pVisionInsp->m_VisionSpec[nMarkSlot];
    spec->Init();

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(spec->m_specImage), 0, spec->m_specImage);

    m_imageViews[nMarkSlot]->SetImage(Ipvm::Image8u());

    m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    m_pVisionInspDlg->m_imageLotView->Overlay_Show(SW_SHOW);
}
void CDlgVisionInspectionMarkOperator::OnBnClickedButtonRemoveSlot2()
{
    ReSetMarkSlot(enumTeachSlot2);
}
void CDlgVisionInspectionMarkOperator::OnBnClickedButtonRemoveSlot3()
{
    ReSetMarkSlot(enumTeachSlot3);
}

void CDlgVisionInspectionMarkOperator::OnBnClickedRadioMultiTeachingSlot1()
{
    MultiTeachingSlot(enumTeachSlot1);
    SetCurSpecValue(enumTeachSlot1);
}

void CDlgVisionInspectionMarkOperator::OnBnClickedRadioMultiTeachingSlot2()
{
    MultiTeachingSlot(enumTeachSlot2);
    SetCurSpecValue(enumTeachSlot2);
}

void CDlgVisionInspectionMarkOperator::OnBnClickedRadioMultiTeachingSlot3()
{
    MultiTeachingSlot(enumTeachSlot3);
    SetCurSpecValue(enumTeachSlot3);
}

void CDlgVisionInspectionMarkOperator::SetCurSpecValue(long nTeachSlot)
{
    m_nTeachSlot = nTeachSlot;
    SetMultiImage();

    SetROI(m_pVisionPara->m_nROISettingMethod);

    auto threshold = getCurrentSpec()->m_nMarkThreshold;
    ((CXTPPropertyGridItemNumber*)m_propertyGrid->FindItem(enumPID_TeachPara_Threshold))->SetNumber(threshold);
}

void CDlgVisionInspectionMarkOperator::MultiTeachingSlot(long nTeachSlot)
{
    m_nTeachSlot = nTeachSlot;
}

void CDlgVisionInspectionMarkOperator::clickButton_teach()
{
    m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();

    GetROI(m_pVisionPara->m_nROISettingMethod);
    BeginWaitCursor(); //kircheis_UIB
    m_vecptSplitePoint.clear();
    m_pVisionPara->m_vecrtUserSplite.clear();

    /// Threshold Value 저장.
    getCurrentSpec()->m_nMarkThreshold
        = ((CXTPPropertyGridItemNumber*)m_propertyGrid->FindItem(enumPID_TeachPara_Threshold))->GetNumber();
    m_pVisionInspDlg->m_pVisionInsp->m_bMarkSpliteOptionTeach = FALSE;
    BOOL bTeach = m_pVisionInspDlg->m_pVisionInsp->DoTeach(true);

    //{{ kircheis_HwaMark 위치 변경
    m_pVisionInspDlg->m_pVisionInsp->m_bTeach = bTeach;

    SetMultiImage();

    SetROI(m_pVisionPara->m_nROISettingMethod);

    if (!bTeach)
    {
        EndWaitCursor(); //kircheis_UIB
        SimpleMessage(_T("Teach Failed!!"));
        return;
    }
    //}}

    long nImageSizeX = m_pVisionInspDlg->m_pVisionInsp->getReusableMemory().GetInspImageSizeX();
    long nImageSizeY = m_pVisionInspDlg->m_pVisionInsp->getReusableMemory().GetInspImageSizeY();
    Ipvm::Point32r2 imageCenter(nImageSizeX * 0.5f, nImageSizeY * 0.5f);

    //{{ //kircheis_HwaMark
    if (SystemConfig::GetInstance().m_bUseMarkTeachResultView)
    {
        CDlgMarkTeachResultViewer dlgResultViewer;

        Ipvm::Image8u specImage = m_pVisionInspDlg->m_pVisionInsp->m_VisionTempSpec->m_specImage;
        long nMarkCharNum = m_pVisionInspDlg->m_pVisionInsp->m_VisionTempSpec->m_nCharNum;
        Ipvm::Rect32s rtViewArea = m_scale.convert_BCUToPixel(
            m_pVisionInspDlg->m_pVisionInsp->m_VisionTempSpec->m_rtMarkTeachROI_BCU, imageCenter);

        std::vector<Ipvm::Rect32s> vecrtEachCharROI(nMarkCharNum);
        for (long i = 0; i < nMarkCharNum; i++)
        {
            vecrtEachCharROI[i] = m_pVisionInspDlg->m_pVisionInsp->m_VisionTempSpec->m_plTeachCharROI[i]
                + Ipvm::Conversion::ToPoint32s2(imageCenter);
        }

        dlgResultViewer.SetImageData(specImage, rtViewArea, nMarkCharNum, &vecrtEachCharROI[0]);

        EndWaitCursor(); //kircheis_UIB
        dlgResultViewer.DoModal();
    }
    else
        EndWaitCursor(); //kircheis_UIB
    //}}
}

void CDlgVisionInspectionMarkOperator::clickButton_test()
{
    BeginWaitCursor(); //kircheis_UIB
    m_pVisionInspDlg->OnBnClickedButtonInspect();
    //SetMarkIgnoreButtonDlgEnable();
    EndWaitCursor(); //kircheis_UIB
}

void CDlgVisionInspectionMarkOperator::ShowCombineImage(BOOL bAutoThreshold)
{
    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = m_pVisionInspDlg->m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    Ipvm::Image8u combineImage;
    Ipvm::Image8u procImage;
    Ipvm::Image8u rotateImage;
    Ipvm::Image8u thresImage;

    auto& visionReusableMemory = m_pVisionInspDlg->m_pVisionInsp->getReusableMemory();

    if (!visionReusableMemory.GetInspByteImage(combineImage))
        return;
    if (!visionReusableMemory.GetInspByteImage(procImage))
        return;
    if (!visionReusableMemory.GetInspByteImage(rotateImage))
        return;
    if (!visionReusableMemory.GetInspByteImage(thresImage))
        return;

    if (!CippModules::GrayImageProcessingManage(visionReusableMemory, &image, false, Ipvm::Rect32s(image),
            m_pVisionPara->m_ImageProcMangePara, combineImage))
    {
        return;
    }

    CString FrameToString;

    if (m_pVisionPara->m_ImageProcMangePara.isCombine())
    {
        FrameToString.Format(_T("Combined Image **"));
    }
    else
    {
        FrameToString = m_pVisionInspDlg->m_pVisionInsp->getIllumInfo2D().getIllumFullName(
            m_pVisionInspDlg->m_pVisionInsp->GetImageFrameIndex(0));
    }

    m_pVisionInspDlg->m_pVisionInsp->m_bodyAlignResult = m_pVisionInspDlg->m_pVisionInsp->GetAlignInfo();
    if (m_pVisionInspDlg->m_pVisionInsp->m_bodyAlignResult == nullptr)
    {
        return;
    }

    const long nSizeX = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const long nSizeY = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    auto centerPoint = m_pVisionInspDlg->m_pVisionInsp->m_bodyAlignResult->m_center;
    float fBodyangle_deg = m_pVisionInspDlg->m_pVisionInsp->m_bodyAlignResult->m_angle_rad * ITP_RAD_TO_DEG;
    float fMarkParaAngle_deg = (float)m_pVisionPara->m_nMarkAngle_deg;
    float imageAngle_deg = fBodyangle_deg + fMarkParaAngle_deg;

    auto* spec = getCurrentSpec();
    Ipvm::Rect32s rtMarkSearchROI = m_scale.convert_BCUToPixel(spec->m_rtMarkTeachROI_BCU, imageCenter);

    if (bAutoThreshold) //원본 Image에서 Threshold값을 빼야지
    {
        BYTE nThresholdValue = (BYTE)spec->m_nMarkThreshold;
        BYTE nLowMean = 0;
        BYTE nHighMean = 0;

        Ipvm::Image8u CalcThresholdValueImage;
        if (!visionReusableMemory.GetInspByteImage(CalcThresholdValueImage))
            return;

        Ipvm::ImageProcessing::Copy(combineImage, Ipvm::Rect32s(combineImage), CalcThresholdValueImage);

        if (m_pVisionPara->m_bAlignAreaIgnore)
            m_pVisionInspDlg->m_pVisionInsp->GetIgnoreImageByDebugInfo(
                combineImage, rtMarkSearchROI, CalcThresholdValueImage);

        std::vector<BYTE> vecbyIgnoreZero(0);
        long nImageSIzeX = CalcThresholdValueImage.GetSizeX();
        long nIndex(0), nIndexY(0);
        BYTE* pbyCalcImage = CalcThresholdValueImage.GetMem();
        for (long ny = rtMarkSearchROI.m_top; ny < rtMarkSearchROI.m_bottom; ny++)
        {
            nIndexY = nImageSIzeX * ny;
            for (long nx = rtMarkSearchROI.m_left; nx < rtMarkSearchROI.m_right; nx++)
            {
                nIndex = nIndexY + nx;
                if (pbyCalcImage[nIndex] > 0)
                    vecbyIgnoreZero.push_back(pbyCalcImage[nIndex]);
            }
        }
        long nBufSize = (long)vecbyIgnoreZero.size();
        if (nBufSize <= 0)
            return;

        Ipvm::Image8u OtsuImageBuf(nBufSize, 1, &vecbyIgnoreZero[0], nBufSize);

        Ipvm::ImageProcessing::GetThresholdOtsu(OtsuImageBuf, rtMarkSearchROI, nLowMean, nHighMean, nThresholdValue);
        spec->m_nMarkThreshold = nThresholdValue;
    }

    /// Image PreProcess
    if (!m_pVisionInspDlg->m_pVisionInsp->ImplProcessing(
            combineImage, Ipvm::Rect32s(combineImage), m_pVisionPara->m_nMarkPreProcMode, procImage))
        return;

    /// Image Rotate
    Ipvm::ImageProcessing::RotateLinearInterpolation(
        procImage, Ipvm::Rect32s(procImage), centerPoint, imageAngle_deg, Ipvm::Point32r2(0.f, 0.f), rotateImage);

    thresImage.Create(procImage.GetSizeX(), procImage.GetSizeY());
    Ipvm::ImageProcessing::Copy(combineImage, Ipvm::Rect32s(combineImage), thresImage);

    Ipvm::ImageProcessing::BinarizeGreaterEqual(
        rotateImage, rtMarkSearchROI, (BYTE)min(255, max(0, spec->m_nMarkThreshold)), thresImage);

    if (m_pVisionInspDlg->m_pVisionPara->m_bWhiteBackGround)
    {
        Ipvm::ImageProcessing::BitwiseNot(rtMarkSearchROI, thresImage);
    }

    if (m_pVisionPara->m_bAlignAreaIgnore)
        m_pVisionInspDlg->m_pVisionInsp->GetIgnoreImageByDebugInfo(thresImage, rtMarkSearchROI, thresImage);

    ((CXTPPropertyGridItemNumber*)m_propertyGrid->FindItem(enumPID_TeachPara_Threshold))
        ->SetNumber(spec->m_nMarkThreshold);
    m_pVisionInspDlg->m_imageLotView->SetImage(thresImage, FrameToString);

    InitializeBtnCtrl();
    UpdateData(FALSE);

    FrameToString.Empty();
}

void CDlgVisionInspectionMarkOperator::SetInspectionImage()
{
    // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
    Ipvm::Image8u image = m_pVisionInspDlg->m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    CString FrameToString = m_pVisionInspDlg->m_pVisionInsp->getIllumInfo2D().getIllumFullName(
        m_pVisionInspDlg->m_pVisionInsp->GetImageFrameIndex(0));
    m_pVisionInspDlg->m_imageLotView->SetImage(image, FrameToString);

    FrameToString.Empty();
}

void CDlgVisionInspectionMarkOperator::SetROI()
{
    const int nSizeX = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const int nSizeY = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Rect32s imageRoi(0, 0, nSizeX, nSizeY);
    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    m_pVisionInspDlg->m_pVisionInsp->m_bAutoTeachMode
        = FALSE; //kircheis_AutoTeach // 여기에 왔다는건 AutoTeach가 아니라는거다.. 혹시 몰라 초기화
    m_pVisionInspDlg->m_imageLotView->ROI_RemoveAll();
    m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();

    Ipvm::Rect32s rtROI(0, 0, 0, 0);

    /// Mark Teach ROI
    auto* spec = getCurrentSpec();
    rtROI = m_scale.convert_BCUToPixel(spec->m_rtMarkTeachROI_BCU, imageCenter);
    rtROI &= imageRoi;
    m_pVisionInspDlg->m_imageLotView->ROI_Add(
        _T("Mark Teach ROI"), _T("Mark Teach ROI"), rtROI, RGB(0, 255, 0), TRUE, TRUE);

    //// Merge ROI 추가.
    long nMergeNum = (long)m_pVisionPara->m_teach_merge_infos.size();
    for (long nIndex = 0; nIndex < nMergeNum; nIndex++)
    {
        CString key;
        key.Format(_T("M%d"), nIndex);

        CString strName;
        strName.Format(_T("Mark Merge_%d"), nIndex);
        Ipvm::Rect32s mergeROI(
            m_pVisionPara->m_teach_merge_infos[nIndex].m_roi + Ipvm::Conversion::ToPoint32s2(imageCenter));
        m_pVisionInspDlg->m_imageLotView->ROI_Add(key, strName, mergeROI, RGB(0, 0, 255), TRUE, TRUE);

        key.Empty();
        strName.Empty();
    }

    //// Ignore ROI 추가.
    long nIgnoreNum = (long)m_pVisionPara->m_vecrtUserIgnore.size();
    for (long nIndex = 0; nIndex < nIgnoreNum; nIndex++)
    {
        CString key;
        key.Format(_T("I%d"), nIndex);

        CString strName;
        strName.Format(_T("Mark Ignore_%d"), nIndex);
        Ipvm::Rect32s ignnoreROI(m_pVisionPara->m_vecrtUserIgnore[nIndex] + Ipvm::Conversion::ToPoint32s2(imageCenter));
        m_pVisionInspDlg->m_imageLotView->ROI_Add(key, strName, ignnoreROI, RGB(255, 0, 0), TRUE, TRUE);

        key.Empty();
        strName.Empty();
    }

    //// Locater ROI 추가.
    if (m_pVisionPara->m_bLocatorTargetROI)
    {
        Ipvm::Rect32s LocatorTargetROI;
        LocatorTargetROI = spec->m_rtLocatorTargetROI[0] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        LocatorTargetROI &= imageRoi;
        m_pVisionInspDlg->m_imageLotView->ROI_Add(
            _T("L1"), _T("Locator Target 1"), LocatorTargetROI, RGB(0, 255, 0), TRUE, TRUE);

        LocatorTargetROI = spec->m_rtLocatorTargetROI[1] + Ipvm::Conversion::ToPoint32s2(imageCenter);
        LocatorTargetROI &= imageRoi;
        m_pVisionInspDlg->m_imageLotView->ROI_Add(
            _T("L1"), _T("Locator Target 2"), LocatorTargetROI, RGB(0, 255, 0), TRUE, TRUE);
    }

    /// splite ROI
    if (m_bBtnSplitePush)
    {
        /// Teching이 안되어 있으면 아무짓도 하지 말자.
        long nCharNum = m_pVisionInspDlg->m_pVisionInsp->m_VisionTempSpec->m_nCharNum;
        std::vector<Ipvm::Rect32s>& vecrtCharPosition
            = m_pVisionInspDlg->m_pVisionInsp->m_result->m_vecrtCharPositionforCalcImage;
        if (nCharNum <= 0 || nCharNum != vecrtCharPosition.size())
            return;

        m_pVisionPara->m_vecrtUserSplite.clear();
        for (long i = 0; i < nCharNum; i++)
        {
            if (spec->m_plTeachCharROI == NULL) //다른 Slot일때 이녀석의 Pointer는 NULL이기 때문에 예외처리가 필요하다
                break;

            Ipvm::Rect32s rtChar = spec->m_plTeachCharROI[i] + Ipvm::Conversion::ToPoint32s2(imageCenter);

            BOOL bSplite = FALSE;
            for (long nSpliteID = 0; nSpliteID < (long)m_vecptSplitePoint.size(); nSpliteID++)
            {
                if (rtChar.PtInRect(m_vecptSplitePoint[nSpliteID]))
                {
                    bSplite = TRUE;
                    break;
                }
            }

            if (bSplite == FALSE)
            {
                m_pVisionInspDlg->m_imageLotView->Overlay_AddRectangle(rtChar, RGB(0, 255, 0));
            }
            else
            {
                m_pVisionInspDlg->m_imageLotView->Overlay_AddRectangle(rtChar, RGB(255, 0, 0));

                rtChar = rtChar - Ipvm::Conversion::ToPoint32s2(imageCenter);
                m_pVisionPara->m_vecrtUserSplite.push_back(rtChar);
            }
        }
    }

    m_pVisionInspDlg->m_imageLotView->ROI_Show(TRUE);
    m_pVisionInspDlg->m_imageLotView->Overlay_Show(TRUE);
}

void CDlgVisionInspectionMarkOperator::GetROI()
{
    const int nSizeX = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeX();
    const int nSizeY = m_pVisionInspDlg->m_pVisionInsp->getImageLotInsp().GetImageSizeY();

    Ipvm::Rect32s imageRoi(0, 0, nSizeX, nSizeY);
    Ipvm::Point32r2 imageCenter(nSizeX * 0.5f, nSizeY * 0.5f);

    auto* spec = getCurrentSpec();

    Ipvm::Rect32s markTeachRoi;
    m_pVisionInspDlg->m_imageLotView->ROI_Get(_T("Mark Teach ROI"), markTeachRoi);

    spec->m_rtMarkTeachROI_BCU = m_scale.convert_PixelToBCU(markTeachRoi, imageCenter);

    /// Merge ROI
    long nMergeNum = (long)m_pVisionPara->m_teach_merge_infos.size();
    for (long nIndex = 0; nIndex < nMergeNum; nIndex++)
    {
        CString key;
        key.Format(_T("M%d"), nIndex);

        auto& roi = m_pVisionPara->m_teach_merge_infos[nIndex].m_roi;
        m_pVisionInspDlg->m_imageLotView->ROI_Get(key, roi);
        roi = roi - Ipvm::Conversion::ToPoint32s2(imageCenter);

        key.Empty();
    }

    /// Ignore ROI
    long nIgnoreNum = (long)m_pVisionPara->m_vecrtUserIgnore.size();
    for (long nIndex = 0; nIndex < nIgnoreNum; nIndex++)
    {
        CString key;
        key.Format(_T("I%d"), nIndex);

        m_pVisionInspDlg->m_imageLotView->ROI_Get(key, m_pVisionPara->m_vecrtUserIgnore[nIndex]);
        m_pVisionPara->m_vecrtUserIgnore[nIndex]
            = m_pVisionPara->m_vecrtUserIgnore[nIndex] - Ipvm::Conversion::ToPoint32s2(imageCenter);

        key.Empty();
    }

    /// Locater Target ROI
    if (m_pVisionPara->m_bLocatorTargetROI)
    {
        m_pVisionInspDlg->m_imageLotView->ROI_Get(_T("L1"), spec->m_rtLocatorTargetROI[0]);
        spec->m_rtLocatorTargetROI[0] = spec->m_rtLocatorTargetROI[0] - Ipvm::Conversion::ToPoint32s2(imageCenter);

        m_pVisionInspDlg->m_imageLotView->ROI_Get(_T("L2"), spec->m_rtLocatorTargetROI[1]);
        spec->m_rtLocatorTargetROI[1] = spec->m_rtLocatorTargetROI[1] - Ipvm::Conversion::ToPoint32s2(imageCenter);
    }

    //ShowCombineImage(m_pVisionPara->sMarkAlgoParam.nThresholdMode);
}

void CDlgVisionInspectionMarkOperator::UpdateMergeROI()
{
    GetROI(m_pVisionPara->m_nROISettingMethod);

    long nCountMerge = max(0, m_pVisionPara->m_teach_merge_num);
    long nCountMergeOld = (long)m_pVisionPara->m_teach_merge_infos.size();

    /// 바뀌었다면 갱신
    if (nCountMerge != nCountMergeOld)
    {
        m_pVisionPara->m_teach_merge_infos.resize(nCountMerge);

        if (nCountMergeOld < nCountMerge)
        {
            for (long nCount = nCountMergeOld; nCount < nCountMerge; nCount++)
            {
                m_pVisionPara->m_teach_merge_infos[nCount].m_roi.SetRect(-50, -50, 50, 50);
            }
        }
    }

    SetROI(m_pVisionPara->m_nROISettingMethod);
}

void CDlgVisionInspectionMarkOperator::UpdateIgnoreROI()
{
    GetROI(m_pVisionPara->m_nROISettingMethod);

    long nCurrIgnoreNum = m_pVisionPara->m_nIgnoreROINum;
    long nOldIgnoreNum = (long)m_pVisionPara->m_vecrtUserIgnore.size();

    /// 바뀌었다면 갱신
    if (nCurrIgnoreNum != nOldIgnoreNum)
    {
        m_pVisionPara->m_vecrtUserIgnore.resize(nCurrIgnoreNum);

        if (nOldIgnoreNum < nCurrIgnoreNum)
        {
            m_pVisionPara->m_vecrtUserIgnore.resize(nCurrIgnoreNum);

            for (long nCount = nOldIgnoreNum; nCount < nCurrIgnoreNum; nCount++)
            {
                m_pVisionPara->m_vecrtUserIgnore[nCount].SetRect(-50, -50, 50, 50);
            }
        }
    }

    SetROI(m_pVisionPara->m_nROISettingMethod);
}

void CDlgVisionInspectionMarkOperator::clickButton_charSpliteMode()
{
    auto* spec = getCurrentSpec();

    //버튼 클릭시 무조건 Ignore는 초기화 된다.
    if (!m_bBtnSplitePush)
    {
        m_bBtnSplitePush = !m_bBtnSplitePush;
        m_vecptSplitePoint.clear();
        m_pVisionPara->m_vecrtUserSplite.clear();

        /// Threshold Value 저장.
        spec->m_nMarkThreshold
            = ((CXTPPropertyGridItemNumber*)m_propertyGrid->FindItem(enumPID_TeachPara_Threshold))->GetNumber();
        m_pVisionInspDlg->m_pVisionInsp->m_bMarkSpliteOptionTeach = FALSE;
        BOOL bTeach = m_pVisionInspDlg->m_pVisionInsp->DoTeach(true);

        if (!bTeach)
        {
            m_pVisionInspDlg->m_pVisionInsp->m_bMarkSpliteOptionTeach = FALSE;
            m_bBtnSplitePush = FALSE;

            CString strMessage;
            strMessage.Format(_T("Can not get the Reference Data \n Please!!. Check Parameter."));
            SimpleMessage(strMessage);

            strMessage.Empty();
            return;
        }

        SetROI(m_pVisionPara->m_nROISettingMethod);

        m_propertyGrid->FindItem(enumPID_ButtonTeach)->SetReadOnly(TRUE);
        m_propertyGrid->FindItem(enumPID_ButtonTest)->SetReadOnly(TRUE);

        Ipvm::Image8u RotateImage;
        m_pVisionInspDlg->PreImageViewSeq(RotateImage);

        m_pVisionInspDlg->m_imageLotView->SetImage(RotateImage);
    }
    else
    {
        m_bBtnSplitePush = !m_bBtnSplitePush;

        /// Threshold Value 저장.
        m_pVisionInspDlg->m_pVisionInsp->m_VisionSpec[0]->m_nMarkThreshold
            = ((CXTPPropertyGridItemNumber*)m_propertyGrid->FindItem(enumPID_TeachPara_Threshold))->GetNumber();
        m_pVisionInspDlg->m_pVisionInsp->m_bMarkSpliteOptionTeach = TRUE;
        BOOL bTeach = m_pVisionInspDlg->m_pVisionInsp->DoTeach(true);

        SetROI(m_pVisionPara->m_nROISettingMethod);

        m_vecptSplitePoint.clear();
        m_pVisionPara->m_vecrtUserSplite.clear();

        m_propertyGrid->FindItem(enumPID_ButtonTeach)->SetReadOnly(FALSE);
        m_propertyGrid->FindItem(enumPID_ButtonTest)->SetReadOnly(FALSE);

        Ipvm::Image8u image = m_pVisionInspDlg->m_pVisionInsp->GetInspectionFrameImage();
        m_pVisionInspDlg->m_imageLotView->SetImage(image);

        if (!bTeach)
        {
            CString strMessage;
            strMessage.Format(_T("Can not get the Reference Data \n Please!!. Check Parameter."));
            SimpleMessage(strMessage);

            strMessage.Empty();
        }
    }
}

void CDlgVisionInspectionMarkOperator::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);

    if (m_propertyGrid->GetSafeHwnd() == NULL)
    {
        return;
    }

    long totalMarkImageRegionY = 280;
    long markImageSizeY = totalMarkImageRegionY / NUM_OF_MARKMULTI;

    for (long id = 0; id < NUM_OF_MARKMULTI; id++)
    {
        int radioID = 0;
        int removeID = 0;

        switch (id)
        {
            case enumTeachSlot1:
                radioID = IDC_RADIO_MULTI_TEACHING_SLOT_1;
                break;
            case enumTeachSlot2:
                radioID = IDC_RADIO_MULTI_TEACHING_SLOT_2;
                removeID = IDC_BTN_REMOVE_SLOT_2;
                break;
            case enumTeachSlot3:
                radioID = IDC_RADIO_MULTI_TEACHING_SLOT_3;
                removeID = IDC_BTN_REMOVE_SLOT_3;
                break;
        }

        if (radioID == 0)
            continue;

        CRect radioRegion;
        GetDlgItem(radioID)->GetWindowRect(radioRegion);
        ScreenToClient(radioRegion);

        ::MoveWindow(m_imageViews[id]->GetSafeHwnd(), radioRegion.right + 5, markImageSizeY * id + 5,
            cx - radioRegion.right - 10, markImageSizeY - 5, TRUE);

        long centerY = markImageSizeY * id + markImageSizeY / 2;
        GetDlgItem(radioID)->MoveWindow(
            radioRegion.left, centerY - radioRegion.Height() / 2, radioRegion.Width(), radioRegion.Height());

        if (removeID)
        {
            CRect region;
            GetDlgItem(removeID)->GetWindowRect(region);
            ScreenToClient(region);

            GetDlgItem(removeID)->MoveWindow(
                region.left, centerY - region.Height() / 2, region.Width(), region.Height());
        }
    }

    m_propertyGrid->MoveWindow(5, totalMarkImageRegionY + 5, cx - 10, cy - totalMarkImageRegionY - 10);
}

void CDlgVisionInspectionMarkOperator::generatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Reference modify tools")))
    {
        if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("ROI Setting Method"),
                m_pVisionPara->m_nROISettingMethod, (int*)&m_pVisionPara->m_nROISettingMethod)))
        {
            Item->GetConstraints()->AddConstraint(_T("Manual"), ROISettingMethod_Manual);
            Item->GetConstraints()->AddConstraint(_T("Map Data"), ROISettingMethod_Mapdata);

            Item->SetID(enumPID_SelectUseMap);
        }

        if (auto* Item = category->AddChildItem(new CCustomItemButton(_T("Import Mark Map Data"), TRUE, FALSE)))
            Item->SetID(enumPID_MarkImportMap);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Char merge box number"),
                m_pVisionPara->m_teach_merge_num, (int*)&m_pVisionPara->m_teach_merge_num)))
        {
            item->GetConstraints()->AddConstraint(_T("None"), 0);
            item->GetConstraints()->AddConstraint(_T("1"), 1);
            item->GetConstraints()->AddConstraint(_T("2"), 2);
            item->GetConstraints()->AddConstraint(_T("3"), 3);

            item->SetID(enumPID_CharMergeBoxNumer);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Char ignore box number"), m_pVisionPara->m_nIgnoreROINum, (int*)&m_pVisionPara->m_nIgnoreROINum)))
        {
            item->GetConstraints()->AddConstraint(_T("None"), 0);
            item->GetConstraints()->AddConstraint(_T("1"), 1);
            item->GetConstraints()->AddConstraint(_T("2"), 2);
            item->GetConstraints()->AddConstraint(_T("3"), 3);

            item->SetID(enumPID_CharIgnoreBoxNumber);
        }

        category->Expand();
    }

    generatePropertyGrid_teachPara();

    if (auto* category = m_propertyGrid->AddCategory(_T("Function")))
    {
        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Teach (Split char.)"), TRUE, TRUE)))
        {
            item->SetID(enumPID_ButtonTeachSplitChar);
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Teach"), TRUE, TRUE)))
        {
            item->SetID(enumPID_ButtonTeach);
        }

        if (auto* item = category->AddChildItem(new CCustomItemButton(_T("Test"), TRUE, TRUE)))
        {
            item->SetID(enumPID_ButtonTest);
        }

        category->Expand();
    }
}

void CDlgVisionInspectionMarkOperator::generatePropertyGrid_teachPara()
{
    CString categoryName = _T("Teach para");
    CXTPPropertyGridItem* category = m_propertyGrid->FindItem(categoryName);
    CXTPPropertyGridItem* item = nullptr;

    if (category == nullptr)
    {
        category = m_propertyGrid->AddCategory(categoryName);
    }

    item = m_propertyGrid->FindItem(enumPID_TeachPara_WhiteBackground);
    if (item)
    {
        ((CXTPPropertyGridItemBool*)item)->SetBool(m_pVisionPara->m_bWhiteBackGround);
    }
    else
    {
        item = category->AddChildItem(new CXTPPropertyGridItemBool(
            _T("White background"), m_pVisionPara->m_bWhiteBackGround, &m_pVisionPara->m_bWhiteBackGround));
        item->SetID(enumPID_TeachPara_WhiteBackground);
    }

    auto* spec = getCurrentSpec();
    //Threshold : Slider Bar
    item = m_propertyGrid->FindItem(enumPID_TeachPara_Threshold);
    if (item)
    {
        ((CXTPPropertyGridItemNumber*)item)->SetNumber(spec->m_nMarkThreshold);
        // 알고리즘을 정확히 몰라서 Threshold 파라메터가 언제 필요하고 언제 안쓰는지 모르겠다.
        // 정확한 타이밍 아는사람이 고치세요
        //item->SetReadOnly(m_pVisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL);
    }
    else
    {
        item = category->AddChildItem(
            new CXTPPropertyGridItemNumber(_T("Threshold"), spec->m_nMarkThreshold, &spec->m_nMarkThreshold));

        if (auto* ThresholdSlider = item->AddSliderControl())
        {
            ThresholdSlider->SetMin(0);
            ThresholdSlider->SetMax(256);
        }

        if (auto* SpinButton = item->AddSpinButton())
        {
            SpinButton->SetMin(0);
            SpinButton->SetMax(256);
        }

        // 알고리즘을 정확히 몰라서 Threshold 파라메터가 언제 필요하고 언제 안쓰는지 모르겠다.
        // 정확한 타이밍 아는사람이 고치세요
        //item->SetReadOnly(m_pVisionPara->sMarkAlgoParam.nThresholdMode != ENUM_THRESHOLD_MANUAL);
        item->SetID(enumPID_TeachPara_Threshold);
    }

    category->Expand();
    categoryName.Empty();
}

VisionInspectionMarkSpec* CDlgVisionInspectionMarkOperator::getCurrentSpec()
{
    long Index = 0;

    if (m_pVisionPara->m_nROISettingMethod == ROISettingMethod_Manual)
        Index = m_nTeachSlot;
    else
        Index = enumTeachSlot1;

    return m_pVisionInspDlg->m_pVisionInsp->m_VisionSpec[Index];
}

LRESULT CDlgVisionInspectionMarkOperator::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* Item = (CXTPPropertyGridItem*)lparam;
    auto* spec = getCurrentSpec();

    const auto& sMarkMapInfo_px = m_pVisionInspDlg->m_pVisionInsp->m_result->m_sMarkMapInfo_px;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (Item->GetID())
        {
            case enumPID_CharMergeBoxNumer:
                if (auto* itemV = dynamic_cast<CXTPPropertyGridItemEnum*>(Item))
                {
                    m_pVisionPara->m_teach_merge_num = itemV->GetEnum();
                    UpdateMergeROI();
                }
                break;

            case enumPID_CharIgnoreBoxNumber:
                if (auto* itemV = dynamic_cast<CXTPPropertyGridItemEnum*>(Item))
                {
                    m_pVisionPara->m_nIgnoreROINum = itemV->GetEnum();
                    UpdateIgnoreROI();
                }
                break;

            case enumPID_TeachPara_WhiteBackground:
                if (auto* itemV = dynamic_cast<CXTPPropertyGridItemBool*>(Item))
                {
                    m_pVisionPara->m_bWhiteBackGround = itemV->GetBool();
                    ShowCombineImage(m_pVisionPara->sMarkAlgoParam.nThresholdMode);
                }
                break;

            case enumPID_TeachPara_Threshold:
                if (auto* itemV = dynamic_cast<CXTPPropertyGridItemNumber*>(Item))
                {
                    spec->m_nMarkThreshold = itemV->GetNumber();
                }
                GetROI(m_pVisionPara->m_nROISettingMethod);
                break;

            case enumPID_ButtonTeachSplitChar:
                clickButton_charSpliteMode();
                break;

            case enumPID_ButtonTeach:
                clickButton_teach();
                break;

            case enumPID_ButtonTest:
                clickButton_test();
                break;

            case enumPID_SelectUseMap:
                SetRoiParameter();

                if (m_pVisionPara->m_nROISettingMethod != ROISettingMethod_Manual)
                {
                    m_pVisionPara->m_nIgnoreROINum = (long)sMarkMapInfo_px.vecIgnoreROI_Info.size();
                    m_pVisionPara->m_teach_merge_num = (long)sMarkMapInfo_px.vecMergeROI_Info.size();

                    RefreshData();
                }
                break;

            case enumPID_MarkImportMap:
                m_pVisionInspDlg->ClickedButtonImportMarkROI_MapData();
                m_pVisionInspDlg->DrawMarkMapROI(m_bBtnSplitePush);

                m_pVisionPara->m_nIgnoreROINum = (long)sMarkMapInfo_px.vecIgnoreROI_Info.size();
                m_pVisionPara->m_teach_merge_num = (long)sMarkMapInfo_px.vecMergeROI_Info.size();

                m_propertyGrid->Refresh();
                break;

            default:
                break;
        }

        m_pVisionInspDlg->RefreshData();
    }

    return 0;
}

void CDlgVisionInspectionMarkOperator::RefreshData()
{
    m_propertyGrid->Refresh();
    SetMultiImage();
}

void CDlgVisionInspectionMarkOperator::SetRoiParameter()
{
    if (m_pVisionPara->m_nROISettingMethod == ROISettingMethod_Manual)
    {
        m_propertyGrid->GetItem(enumPID_MarkImportMap)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(enumPID_CharMergeBoxNumer)->SetReadOnly(FALSE);
        m_propertyGrid->GetItem(enumPID_CharIgnoreBoxNumber)->SetReadOnly(FALSE);

        GetROI(m_pVisionPara->m_nROISettingMethod);
        SetROI(m_pVisionPara->m_nROISettingMethod);

        m_pVisionInspDlg->m_imageLotView->Overlay_RemoveAll();
        m_pVisionInspDlg->m_imageLotView->ROI_Show(TRUE);
    }
    else
    {
        m_propertyGrid->GetItem(enumPID_MarkImportMap)->SetReadOnly(FALSE);
        m_propertyGrid->GetItem(enumPID_CharMergeBoxNumer)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(enumPID_CharIgnoreBoxNumber)->SetReadOnly(TRUE);

        GetROI(m_pVisionPara->m_nROISettingMethod);
        SetROI(m_pVisionPara->m_nROISettingMethod);

        m_pVisionInspDlg->m_imageLotView->ROI_Show(FALSE);
    }

    m_pVisionInspDlg->m_imageLotView->Overlay_Show(TRUE);
}

void CDlgVisionInspectionMarkOperator::SetROI(long nMode)
{
    if (nMode == ROISettingMethod_Manual)
    {
        SetROI();
    }
    else
    {
        m_pVisionInspDlg->DrawMarkMapROI(m_bBtnSplitePush);
    }
}

void CDlgVisionInspectionMarkOperator::GetROI(long nMode)
{
    if (nMode == ROISettingMethod_Manual)
    {
        GetROI();
    }
    else
    {
        m_pVisionInspDlg->SetROI_MapData();
    }
}