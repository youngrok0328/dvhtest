//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspection2DMatrix.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspection2DMatrix.h"
#include "VisionInspection2DMatrixPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomNumber.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Conversion.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_CHANGED (WM_USER + 1011)
#define UM_ROI_CHANGED (WM_USER + 1012)

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID
{
    ITEM_ID_VALID_ONLY_NUMBER_AND_ALPHABET = 1,
    ITEM_ID_MINIMUM_LENGTH_FOR_NORMAL_STRING,
    ITEM_ID_MAXIMUM_LENGTH_FOR_NORMAL_STRING,
    ITEM_ID_MAJOR_ROI_NUMBER,
    ITEM_ID_SUB_ROI_NUMBER,
    ITEM_ID_BABY_ROI_NUMBER,
    ITEM_ID_2DID_A_ID_FOR_MATCHING, //kircheis_MED5_13
    ITEM_ID_2DID_B_ID_FOR_MATCHING, //kircheis_MED5_13
};

IMPLEMENT_DYNAMIC(CDlgVisionInspection2DMatrix, CDialog)

CDlgVisionInspection2DMatrix::CDlgVisionInspection2DMatrix(
    const ProcessingDlgInfo& procDlgInfo, VisionInspection2DMatrix* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspection2DMatrix::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_imageLotView(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
    , m_propertyGrid(nullptr)
{
    m_pVisionInsp = pVisionInsp;
    m_pVisionInsp->ResetResult(); //kircheis_2DMatVMSDK
    m_pVisionPara = m_pVisionInsp->m_VisionPara;

    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
    m_nBarcodeImage = 0;
}

CDlgVisionInspection2DMatrix::~CDlgVisionInspection2DMatrix()
{
    delete m_imageLotView;
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonSpecDlg;
    delete m_propertyGrid;
    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspection2DMatrix::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspection2DMatrix, CDialog)
//ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspection2DMatrix::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspection2DMatrix::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspection2DMatrix::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspection2DMatrix::OnCbnSelchangeComboCurrpane)
ON_BN_CLICKED(IDC_BUTTON_PREIMAGE, &CDlgVisionInspection2DMatrix::OnBnClickedButtonPreimage)
ON_BN_CLICKED(IDC_BUTTON_NEXTIMAGE, &CDlgVisionInspection2DMatrix::OnBnClickedButtonNextimage)
ON_MESSAGE(UM_ROI_CHANGED, &CDlgVisionInspection2DMatrix::OnRoiChanged)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspection2DMatrix::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgVisionInspection2DMatrix::OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_CHANGED, &CDlgVisionInspection2DMatrix::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspection2DMatrix::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    CRect rtPara = m_procDlgInfo.m_rtParaArea;

    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, static_cast<UINT>(IDC_STATIC));

    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Spec."));
    m_TabResult.InsertItem(1, _T("Result"));
    m_TabResult.InsertItem(2, _T("Detail"));
    m_TabResult.InsertItem(3, _T("Debug"));
    m_TabResult.InsertItem(4, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonSpecDlg = new ProcCommonSpecDlg(this, rtTab, m_pVisionInsp->m_fixedInspectionSpecs);
    m_procCommonResultDlg = new ProcCommonResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView,
        m_pVisionInsp->m_visionUnit.GetJobFileName(), UM_SHOW_DETAIL_RESULT);
    m_procCommonDetailResultDlg
        = new ProcCommonDetailResultDlg(this, rtTab, m_pVisionInsp->m_resultGroup, *m_imageLotView);
    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonSpecDlg->ShowWindow(SW_SHOW);

    // Dialog Control 위치 설정
    UpdatePropertyGrid();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspection2DMatrix::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionInspection2DMatrix::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
}

void CDlgVisionInspection2DMatrix::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspection2DMatrix::SetInitDialog()
{
}

void CDlgVisionInspection2DMatrix::SetInitParaWindow()
{
}

void CDlgVisionInspection2DMatrix::SetInitButton()
{
    CRect rtDlg;
    GetClientRect(rtDlg);

    CRect rtButton;
    m_buttonClose.GetWindowRect(rtButton);
    long nButtonPitch = rtButton.Width() + 3;

    // Close 버튼 위치 지정
    m_buttonClose.MoveWindow(
        rtDlg.right - nButtonPitch, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());

    // Inspect 버튼 위치 지정
    m_buttonInspect.MoveWindow(
        rtDlg.right - nButtonPitch * 2, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());

    // Inspect 버튼 위치 지정
    m_buttonApply.MoveWindow(
        rtDlg.right - nButtonPitch * 3, rtDlg.bottom - rtButton.Height() - 10, rtButton.Width(), rtButton.Height());
}

void CDlgVisionInspection2DMatrix::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp, false, m_pVisionInsp->GetCurVisionModule_Status());
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());

    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();

    // 검사 결과 화면 갱신
    m_procCommonResultDlg->Refresh();

    // 검사 결과 탭 보여주기
    m_TabResult.SetCurSel(1);
    OnTcnSelchangeTabResult(nullptr, nullptr);
}

void CDlgVisionInspection2DMatrix::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonSpecDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonResultDlg->ShowWindow((nTab == 1) ? SW_SHOW : SW_HIDE);
    m_procCommonDetailResultDlg->ShowWindow((nTab == 2) ? SW_SHOW : SW_HIDE);
    m_procCommonDebugInfoDlg->ShowWindow(nTab == 3 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 4 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

void CDlgVisionInspection2DMatrix::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspection2DMatrix::ShowPrevImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID <= 0)
        return;

    m_nImageID--;

    ShowImage(TRUE);
}

void CDlgVisionInspection2DMatrix::ShowNextImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID
        >= (long)(m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size()) - 1)
        return;

    m_nImageID++;

    ShowImage(TRUE);
}

void CDlgVisionInspection2DMatrix::ShowImage(BOOL bChange)
{
    if (m_pVisionInsp->GetImageFrameCount() == 0)
    {
        return;
    }

    if (!bChange)
    {
        m_nImageID = max(0, m_pVisionInsp->GetImageFrameIndex(0));
    }

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() <= 0)
        return;

    if ((long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size()
        <= m_nImageID)
        m_nImageID
            = (long)m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() - 1;

    m_imageLotView->ShowImage(m_nImageID);
}

void CDlgVisionInspection2DMatrix::OnCbnSelchangeComboCurrpane()
{
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    long nSel = m_ComboCurrPane.GetCurSel();
    m_pVisionInsp->SetCurrentPaneID(nSel);

    if (m_pVisionInsp->m_visionUnit.RunInspection(
            m_pVisionInsp->m_visionUnit.GetPreviousVisionProcessing(m_pVisionInsp), false,
            m_pVisionInsp->GetCurVisionModule_Status()))
    {
        ShowImage();
    }

    m_pVisionInsp->m_visionUnit.GetInspectionOverlayResult().Apply(
        m_pVisionInsp->m_pVisionInspDlg->m_imageLotView->GetCoreView());

    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->Overlay_Show(TRUE);

    SetROI();
}

void CDlgVisionInspection2DMatrix::OnBnClickedButtonPreimage()
{
    ShowPrevImage();
}

void CDlgVisionInspection2DMatrix::OnBnClickedButtonNextimage()
{
    ShowNextImage();
}

LRESULT CDlgVisionInspection2DMatrix::OnRoiChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    GetROI();
    ShowCharBinImage();

    return 0;
}

BOOL CDlgVisionInspection2DMatrix::ShowCharBinImage()
{
    return TRUE;
}

LRESULT CDlgVisionInspection2DMatrix::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long itemIndex = long(wparam);

    if (itemIndex < 0)
    {
        m_TabResult.SetCurSel(1);
    }
    else
    {
        m_TabResult.SetCurSel(2);

        m_procCommonDetailResultDlg->Refresh(itemIndex);
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

void CDlgVisionInspection2DMatrix::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("2D Barcode Parameter")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(_T("Valid only number and alphabet"),
                m_pVisionPara->m_validOnlyNumberAndAlphabet, &m_pVisionPara->m_validOnlyNumberAndAlphabet)))
        {
            item->SetID(ITEM_ID_VALID_ONLY_NUMBER_AND_ALPHABET);
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
                _T("Minimum length for normal string"), m_pVisionPara->m_minimumLengthForNormalString, _T("%d"),
                m_pVisionPara->m_minimumLengthForNormalString, 0, 255)))
        {
            item->SetID(ITEM_ID_MINIMUM_LENGTH_FOR_NORMAL_STRING);
        }

        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
                _T("Maximum length for normal string"), m_pVisionPara->m_maximumLengthForNormalString, _T("%d"),
                m_pVisionPara->m_maximumLengthForNormalString, 0, 255)))
        {
            item->SetID(ITEM_ID_MAXIMUM_LENGTH_FOR_NORMAL_STRING);
        }

        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Major 2DID number"), m_pVisionPara->m_nMajorID_Count)))
        {
            item->GetConstraints()->AddConstraint(_T("0"), 0);
            item->GetConstraints()->AddConstraint(_T("1"), 1);

            item->SetID(ITEM_ID_MAJOR_ROI_NUMBER);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Sub 2DID number"), m_pVisionPara->m_nSubID_Count)))
        {
            item->GetConstraints()->AddConstraint(_T("0"), 0);
            item->GetConstraints()->AddConstraint(_T("1"), 1);
            item->GetConstraints()->AddConstraint(_T("2"), 2);

            item->SetID(ITEM_ID_SUB_ROI_NUMBER);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Baby 2DID number"), m_pVisionPara->m_nBabyID_Count)))
        {
            item->GetConstraints()->AddConstraint(_T("0"), 0);
            item->GetConstraints()->AddConstraint(_T("1"), 1);
            //item->GetConstraints()->AddConstraint(_T("2"), 2);

            item->SetID(ITEM_ID_BABY_ROI_NUMBER);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("2DID A ID for matching"), m_pVisionPara->m_n1st2DIDforMatch))) //kircheis_MED5_13
        {
            item->GetConstraints()->AddConstraint(_T("Major"), TYPE_2DID_MAJOR);
            item->GetConstraints()->AddConstraint(_T("Sub 1"), TYPE_2DID_SUB_0);
            item->GetConstraints()->AddConstraint(_T("Sub 2"), TYPE_2DID_SUB_1);
            item->GetConstraints()->AddConstraint(_T("Baby 1"), TYPE_2DID_BABY_0);
            //item->GetConstraints()->AddConstraint(_T("Baby 2"), TYPE_2DID_BABY_1);

            item->SetID(ITEM_ID_2DID_A_ID_FOR_MATCHING);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("2DID B ID for matching"), m_pVisionPara->m_n2nd2DIDforMatch))) //kircheis_MED5_13
        {
            item->GetConstraints()->AddConstraint(_T("Major"), TYPE_2DID_MAJOR);
            item->GetConstraints()->AddConstraint(_T("Sub 1"), TYPE_2DID_SUB_0);
            item->GetConstraints()->AddConstraint(_T("Sub 2"), TYPE_2DID_SUB_1);
            item->GetConstraints()->AddConstraint(_T("Baby 1"), TYPE_2DID_BABY_0);
            //item->GetConstraints()->AddConstraint(_T("Baby 2"), TYPE_2DID_BABY_1);

            item->SetID(ITEM_ID_2DID_B_ID_FOR_MATCHING);
        }

        category->Expand();
    }

    SetROI();

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspection2DMatrix::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* NumberValue = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
    {
        //long data = value->GetNumber();

        //switch (item->GetID())
        //{
        //}
    }
    else if (auto* DobuleValue = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
    {
        //double data = value->GetDouble();
        //switch (item->GetID())
        //{

        //}
    }
    else if (auto* EnumValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = EnumValue->GetEnum();
        switch (item->GetID())
        {
            case ITEM_ID_MAJOR_ROI_NUMBER:
                GetROI();
                m_pVisionPara->SetSearchMajorROI_Count(data);
                SetROI();
                break;
            case ITEM_ID_SUB_ROI_NUMBER:
                GetROI();
                m_pVisionPara->SetSearchSubROI_Count(data);
                SetROI();
                break;
            case ITEM_ID_BABY_ROI_NUMBER:
                GetROI();
                m_pVisionPara->SetSearchBabyROI_Count(data);
                SetROI();
                break;
            case ITEM_ID_2DID_A_ID_FOR_MATCHING: //kircheis_MED5_13
                if (m_pVisionPara->m_n2nd2DIDforMatch == data)
                    data = (int)((m_pVisionPara->m_n2nd2DIDforMatch + 1) % TYPE_2DID_END);
                m_pVisionPara->m_n1st2DIDforMatch = (long)max(TYPE_2DID_START, min(TYPE_2DID_END, data));
                EnumValue->SetEnum(m_pVisionPara->m_n1st2DIDforMatch);
                break;
            case ITEM_ID_2DID_B_ID_FOR_MATCHING: //kircheis_MED5_13
                if (m_pVisionPara->m_n1st2DIDforMatch == data)
                    data = (int)((m_pVisionPara->m_n1st2DIDforMatch + 1) % TYPE_2DID_END);
                m_pVisionPara->m_n2nd2DIDforMatch = (long)max(TYPE_2DID_START, min(TYPE_2DID_END, data));
                EnumValue->SetEnum(m_pVisionPara->m_n2nd2DIDforMatch);
                break;
        }
    }

    return 0;
}

void CDlgVisionInspection2DMatrix::GetROI()
{
    Ipvm::Rect32r frtBody = m_pVisionInsp->GetBodyRect();

    if ((frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        && m_pVisionInsp->m_bUseBypassMode == false) //kircheis_NGRV Bypass
        return;
    else if (m_pVisionInsp->m_bUseBypassMode)
    {
        frtBody = m_pVisionInsp->GetPaneRect();
    }

    long nMajorNum = (long)m_pVisionInsp->m_VisionPara->m_vecrtSearchMajorID_BCU.size();
    long nSubNum = (long)m_pVisionInsp->m_VisionPara->m_vecrtSearchSubID_BCU.size();
    long nBabyNum = (long)m_pVisionInsp->m_VisionPara->m_vecrtSearchBabyID_BCU.size();

    if (nMajorNum != m_pVisionInsp->m_VisionPara->m_nMajorID_Count)
    {
        ASSERT(!_T("Check the Major 2DID"));
        m_pVisionInsp->m_VisionPara->m_nMajorID_Count = nMajorNum;
    }
    if (nSubNum != m_pVisionInsp->m_VisionPara->m_nSubID_Count)
    {
        ASSERT(!_T("Check the Sub 2DID"));
        m_pVisionInsp->m_VisionPara->m_nSubID_Count = nSubNum;
    }
    if (nBabyNum != m_pVisionInsp->m_VisionPara->m_nBabyID_Count)
    {
        ASSERT(!_T("Check the Baby 2DID"));
        m_pVisionInsp->m_VisionPara->m_nBabyID_Count = nBabyNum;
    }

    for (long nROICount = 0; nROICount < nMajorNum; nROICount++)
    {
        CString key;
        key.Format(_T("Major%d"), nROICount);

        Ipvm::Rect32s rtSearchROI;
        m_imageLotView->ROI_Get(key, rtSearchROI);
        rtSearchROI &= Ipvm::Conversion::ToRect32s(frtBody);
        m_pVisionInsp->ImageRectToAbsoluteRectByBodyCenter(frtBody, rtSearchROI);

        m_pVisionInsp->m_VisionPara->m_vecrtSearchMajorID_BCU[nROICount]
            = m_pVisionInsp->getScale().convert_pixelToUm(rtSearchROI);
        key.Empty();
    }

    for (long nROICount = 0; nROICount < nSubNum; nROICount++)
    {
        CString key;
        key.Format(_T("Sub%d"), nROICount);

        Ipvm::Rect32s rtSearchROI;
        m_imageLotView->ROI_Get(key, rtSearchROI);
        rtSearchROI &= Ipvm::Conversion::ToRect32s(frtBody);
        m_pVisionInsp->ImageRectToAbsoluteRectByBodyCenter(frtBody, rtSearchROI);

        m_pVisionInsp->m_VisionPara->m_vecrtSearchSubID_BCU[nROICount]
            = m_pVisionInsp->getScale().convert_pixelToUm(rtSearchROI);
        key.Empty();
    }

    for (long nROICount = 0; nROICount < nBabyNum; nROICount++)
    {
        CString key;
        key.Format(_T("Baby%d"), nROICount);

        Ipvm::Rect32s rtSearchROI;
        m_imageLotView->ROI_Get(key, rtSearchROI);
        rtSearchROI &= Ipvm::Conversion::ToRect32s(frtBody);
        m_pVisionInsp->ImageRectToAbsoluteRectByBodyCenter(frtBody, rtSearchROI);

        m_pVisionInsp->m_VisionPara->m_vecrtSearchBabyID_BCU[nROICount]
            = m_pVisionInsp->getScale().convert_pixelToUm(rtSearchROI);
        key.Empty();
    }
}

void CDlgVisionInspection2DMatrix::SetROI()
{
    m_imageLotView->ROI_RemoveAll();
    Ipvm::Rect32r frtBody = m_pVisionInsp->GetBodyRect();

    if ((frtBody.Width() <= 0.f || frtBody.Height() <= 0.f)
        && m_pVisionInsp->m_bUseBypassMode == false) //kircheis_NGRV Bypass
        return;
    else if (m_pVisionInsp->m_bUseBypassMode)
    {
        frtBody = m_pVisionInsp->GetPaneRect();
    }

    long nMajorNum = (long)m_pVisionInsp->m_VisionPara->m_vecrtSearchMajorID_BCU.size();
    long nSubNum = (long)m_pVisionInsp->m_VisionPara->m_vecrtSearchSubID_BCU.size();
    long nBabyNum = (long)m_pVisionInsp->m_VisionPara->m_vecrtSearchBabyID_BCU.size();

    const auto& scale = m_pVisionInsp->getScale();

    if (nMajorNum != m_pVisionInsp->m_VisionPara->m_nMajorID_Count)
    {
        ASSERT(!_T("Check the Major 2DID"));
        m_pVisionInsp->m_VisionPara->m_nMajorID_Count = nMajorNum;
    }
    if (nSubNum != m_pVisionInsp->m_VisionPara->m_nSubID_Count)
    {
        ASSERT(!_T("Check the Sub 2DID"));
        m_pVisionInsp->m_VisionPara->m_nSubID_Count = nSubNum;
    }
    if (nBabyNum != m_pVisionInsp->m_VisionPara->m_nBabyID_Count)
    {
        ASSERT(!_T("Check the Baby 2DID"));
        m_pVisionInsp->m_VisionPara->m_nBabyID_Count = nBabyNum;
    }

    for (long nROICount = 0; nROICount < m_pVisionInsp->m_VisionPara->m_nMajorID_Count; nROICount++)
    {
        Ipvm::Rect32s rtSpecSearch
            = scale.convert_umToPixel(m_pVisionInsp->m_VisionPara->m_vecrtSearchMajorID_BCU[nROICount]);

        Ipvm::Rect32s rtSearchROI = rtSpecSearch;
        m_pVisionInsp->AbsoluteRectToImageRectByBodyCenter(frtBody, rtSearchROI);

        Ipvm::Rect32s rtInnerArea = Ipvm::Conversion::ToRect32s(frtBody);

        if ((rtSearchROI.Width() * rtSearchROI.Height() == 0)
            || (rtInnerArea.PtInRect(rtSearchROI.TopLeft()) == FALSE
                && rtInnerArea.PtInRect(rtSearchROI.TopRight()) == FALSE
                && rtInnerArea.PtInRect(rtSearchROI.BottomLeft()) == FALSE
                && rtInnerArea.PtInRect(rtSearchROI.BottomRight()) == FALSE))
        {
            rtSearchROI.SetRect(rtInnerArea.CenterPoint(), rtInnerArea.CenterPoint());
            rtSearchROI.InflateRect(300, 300);
        }

        rtSearchROI &= Ipvm::Conversion::ToRect32s(frtBody);

        CString key;
        key.Format(_T("Major%d"), nROICount);

        CString strROIName;
        strROIName.Format(_T("Major"));
        m_imageLotView->ROI_Add(key, strROIName, rtSearchROI, RGB(255, 255, 0), TRUE, TRUE);

        key.Empty();
        strROIName.Empty();
    }
    for (long nROICount = 0; nROICount < nSubNum; nROICount++)
    {
        Ipvm::Rect32s rtSpecSearch
            = scale.convert_umToPixel(m_pVisionInsp->m_VisionPara->m_vecrtSearchSubID_BCU[nROICount]);

        Ipvm::Rect32s rtSearchROI = rtSpecSearch;
        m_pVisionInsp->AbsoluteRectToImageRectByBodyCenter(frtBody, rtSearchROI);

        rtSearchROI &= Ipvm::Conversion::ToRect32s(frtBody);

        CString key;
        key.Format(_T("Sub%d"), nROICount);

        CString strROIName;
        strROIName.Format(_T("Sub %d"), nROICount + 1);
        m_imageLotView->ROI_Add(key, strROIName, rtSearchROI, RGB(255, 255, 0), TRUE, TRUE);
        key.Empty();
        strROIName.Empty();
    }
    for (long nROICount = 0; nROICount < nBabyNum; nROICount++)
    {
        Ipvm::Rect32s rtSpecSearch
            = scale.convert_umToPixel(m_pVisionInsp->m_VisionPara->m_vecrtSearchBabyID_BCU[nROICount]);

        Ipvm::Rect32s rtSearchROI = rtSpecSearch;
        m_pVisionInsp->AbsoluteRectToImageRectByBodyCenter(frtBody, rtSearchROI);

        rtSearchROI &= Ipvm::Conversion::ToRect32s(frtBody);

        CString key;
        key.Format(_T("Baby%d"), nROICount);

        CString strROIName;
        strROIName.Format(_T("Baby %d"), nROICount + 1);
        m_imageLotView->ROI_Add(key, strROIName, rtSearchROI, RGB(255, 255, 0), TRUE, TRUE);
        key.Empty();
        strROIName.Empty();
    }

    m_imageLotView->ROI_Show(TRUE);
}

LRESULT CDlgVisionInspection2DMatrix::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    SetROI();

    return 0L;
}
