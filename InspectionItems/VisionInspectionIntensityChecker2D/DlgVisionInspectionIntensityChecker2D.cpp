//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionIntensityChecker2D.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionIntensityChecker2D.h"
#include "VisionInspectionIntensityChecker2DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"
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
enum enumIntensityCheckerParamID
{
    INTENSITYCHECKER_PARAM_START = 0,
    INTENSITYCHECKER_RESET_ROI = INTENSITYCHECKER_PARAM_START,
    INTENSITYCHECKER_SET_SAMEROI,
    INTENSITYCHECKER_ROI_INTENSITY_ALGORITHM_MODE,
    INTENSITYCHECKER_ROI0_1,
    INTENSITYCHECKER_ROI0_2,
    INTENSITYCHECKER_ROI0_3,
    INTENSITYCHECKER_ROI1_1,
    INTENSITYCHECKER_ROI1_2,
    INTENSITYCHECKER_ROI1_3,
    INTENSITYCHECKER_ROI2_1,
    INTENSITYCHECKER_ROI2_2,
    INTENSITYCHECKER_ROI2_3,
    INTENSITYCHECKER_PARAM_END,
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionIntensityChecker2D, CDialog)

CDlgVisionInspectionIntensityChecker2D::CDlgVisionInspectionIntensityChecker2D(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionIntensityChecker2D* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionIntensityChecker2D::IDD, pParent)
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

CDlgVisionInspectionIntensityChecker2D::~CDlgVisionInspectionIntensityChecker2D()
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

void CDlgVisionInspectionIntensityChecker2D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
    DDX_Control(pDX, IDC_TAB_LOG, m_TabLog);
    DDX_Control(pDX, IDC_BUTTON_APPLY, m_buttonApply);
    DDX_Control(pDX, IDC_BUTTON_INSPECT, m_buttonInspect);
    DDX_Control(pDX, IDC_BUTTON_CLOSE, m_buttonClose);
    DDX_Control(pDX, IDC_COMBO_CURRPANE, m_ComboCurrPane);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionIntensityChecker2D, CDialog)
//ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionIntensityChecker2D::OnTcnSelchangeTabResult)
ON_BN_CLICKED(IDC_BUTTON_CLOSE, &CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonClose)
ON_CBN_SELCHANGE(IDC_COMBO_CURRPANE, &CDlgVisionInspectionIntensityChecker2D::OnCbnSelchangeComboCurrpane)
ON_BN_CLICKED(IDC_BUTTON_PREIMAGE, &CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonPreimage)
ON_BN_CLICKED(IDC_BUTTON_NEXTIMAGE, &CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonNextimage)
ON_MESSAGE(UM_ROI_CHANGED, &CDlgVisionInspectionIntensityChecker2D::OnRoiChanged)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionIntensityChecker2D::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgVisionInspectionIntensityChecker2D::OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_CHANGED, &CDlgVisionInspectionIntensityChecker2D::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionIntensityChecker2D::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    //void* pData = m_visionUnit.GetVisionDebugInfo(m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    //if (pData != nullptr && nDataNum > 0)
    //{
    //	*m_sEdgeAlignResult = *(VisionAlignResult *)pData;
    //	frtBodyRect = m_sEdgeAlignResult->getBodyRect();
    //}

    MoveWindow(rtParentClient, FALSE);

    // Caption
    SetWindowText(m_pVisionInsp->m_strModuleName);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    CRect rtPara = m_procDlgInfo.m_rtParaArea;

    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, CAST_UINT(IDC_STATIC));

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

    // ROI 표시
    m_imageLotView->Overlay_RemoveAll();
    m_imageLotView->ROI_RemoveAll();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    // 엣지 성분 추출
    m_pVisionInsp->m_rtPaneRect = Ipvm::Conversion::ToRect32s(m_pVisionInsp->GetBodyRect());
    m_pVisionInsp->m_ptPaneCenter = m_pVisionInsp->m_rtPaneRect.CenterPoint32r();

    // 방금 창을 연 상태인 경우 여기서 한번 초기화를 진행해준다.
    if (m_pVisionPara->m_vecROI[0].m_bottom < 0 || m_pVisionPara->m_vecROI[0].m_right < 0)
    {
        m_pVisionInsp->ResetROI();
    }

    SetROI();

    return TRUE; // return TRUE  unless you set the focus to a control
}

int CDlgVisionInspectionIntensityChecker2D::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
        return -1;

    return 0;
}

void CDlgVisionInspectionIntensityChecker2D::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
}

void CDlgVisionInspectionIntensityChecker2D::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionInspectionIntensityChecker2D::SetInitDialog()
{
}

void CDlgVisionInspectionIntensityChecker2D::SetInitParaWindow()
{
}

void CDlgVisionInspectionIntensityChecker2D::SetInitButton()
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

void CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonInspect()
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

void CDlgVisionInspectionIntensityChecker2D::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

void CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionIntensityChecker2D::ShowPrevImage()
{
    // Image Display 를 위한 테스트 코드
    if (m_pVisionInsp->getImageLotInsp().m_vecImages[m_pVisionInsp->GetCurVisionModule_Status()].size() == 0)
        return;

    if (m_nImageID <= 0)
        return;

    m_nImageID--;

    ShowImage(TRUE);
}

void CDlgVisionInspectionIntensityChecker2D::ShowNextImage()
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

void CDlgVisionInspectionIntensityChecker2D::ShowImage(BOOL bChange)
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

void CDlgVisionInspectionIntensityChecker2D::OnCbnSelchangeComboCurrpane()
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

void CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonPreimage()
{
    ShowPrevImage();
}

void CDlgVisionInspectionIntensityChecker2D::OnBnClickedButtonNextimage()
{
    ShowNextImage();
}

LRESULT CDlgVisionInspectionIntensityChecker2D::OnRoiChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    GetROI();
    ShowCharBinImage();

    return 0;
}

BOOL CDlgVisionInspectionIntensityChecker2D::ShowCharBinImage()
{
    return TRUE;
}

LRESULT CDlgVisionInspectionIntensityChecker2D::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
{
    const long nItemIndex = long(wparam);

    if (nItemIndex < 0)
    {
        m_TabResult.SetCurSel(1);
    }
    else
    {
        m_TabResult.SetCurSel(2);

        m_procCommonDetailResultDlg->Refresh(nItemIndex);
    }

    OnTcnSelchangeTabResult(nullptr, nullptr);

    return 0L;
}

void CDlgVisionInspectionIntensityChecker2D::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("ROI intensity algorithm")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Reset ROI"), TRUE, FALSE))->SetID(INTENSITYCHECKER_RESET_ROI);

        category->AddChildItem(new CCustomItemButton(_T("ROI Set to ROI0"), TRUE, FALSE))
            ->SetID(INTENSITYCHECKER_SET_SAMEROI);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Intensity algorithm mode"),
                m_pVisionPara->m_nROI_IntensityAlgorithmMode))) //, (int *)m_pVisionPara->m_nSearchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("Average"), 0);
            item->GetConstraints()->AddConstraint(_T("Calibration Method"), 1);

            item->SetID(INTENSITYCHECKER_ROI_INTENSITY_ALGORITHM_MODE);
        }

        category->Expand();
    }
    //strGUIDTemp.Format(_T("{3AA40B82-774E-4A79-B3D2-1CE6ECA8590B}%d"), nROInth);

    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        CString strROITemp;
        strROITemp.Format(_T("Set Image Frame for ROI %d"), nROInth);

        if (auto* category = m_propertyGrid->AddCategory(strROITemp))
        {
            makeROIFrameGrid(category, _T("First Image Frame ID"), INTENSITYCHECKER_ROI0_1 + (nROInth * MAX_FRAME_NUM));

            makeROIFrameGrid(
                category, _T("Second Image Frame ID"), INTENSITYCHECKER_ROI0_2 + (nROInth * MAX_FRAME_NUM));

            makeROIFrameGrid(category, _T("Third Image Frame ID"), INTENSITYCHECKER_ROI0_3 + (nROInth * MAX_FRAME_NUM));

            category->Expand();
        }
    }

    SetROI();

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionInspectionIntensityChecker2D::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        //long nSearchLowLimit_um = 500;
        //const long nSearchHighLimit_um = 2000;

        switch (item->GetID())
        {
            case INTENSITYCHECKER_RESET_ROI:
                m_pVisionInsp->ResetROI();
                break;

            case INTENSITYCHECKER_SET_SAMEROI:
                m_pVisionInsp->SetSameROI();
                break;

            case INTENSITYCHECKER_ROI_INTENSITY_ALGORITHM_MODE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nROI_IntensityAlgorithmMode = value->GetEnum();
                break;

            default:
                int nGridNum = item->GetID();
                nGridNum -= 3; // 위쪽에 항목이 몇개나 있는지에 따라 순서를 변경해줘야됨
                int nRoiNum = nGridNum / (MAX_FRAME_NUM); // 1부터 시작하게 하여 n의 배수로 만들어서 해결
                int nFrameNum = nGridNum % (MAX_FRAME_NUM);
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                {
                    m_pVisionPara->m_vec2nROIFrameNum[nRoiNum][nFrameNum] = value->GetEnum();
                }
                break;
        }
    }

    SetROI();

    return 0;
}

void CDlgVisionInspectionIntensityChecker2D::GetROI()
{
    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        CString strTemp;
        strTemp.Format(_T("ROI%d"), nROInth);
        m_imageLotView->ROI_Get(strTemp, m_pVisionInsp->m_VisionPara->m_vecROI[nROInth]);
    }
}

void CDlgVisionInspectionIntensityChecker2D::SetROI()
{
    for (int nROInth = 0; nROInth < MAX_ROI_NUM; nROInth++)
    {
        CString strTemp;
        strTemp.Format(_T("ROI%d"), nROInth);

        m_imageLotView->ROI_Add(
            strTemp, strTemp, m_pVisionInsp->m_VisionPara->m_vecROI[nROInth], RGB(255, 0, 0), TRUE, TRUE);
    }
    m_imageLotView->ROI_Show(TRUE);
}

LRESULT CDlgVisionInspectionIntensityChecker2D::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing(), false, m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    SetROI();

    return 0L;
}

CXTPPropertyGridItem* CDlgVisionInspectionIntensityChecker2D::makeROIFrameGrid(
    CXTPPropertyGridItem* parent, LPCTSTR caption, UINT id)
{
    // 테이블 만들기 위한 데이터 수집
    VisionProcessing* m_parent = m_pVisionInsp->m_visionUnit.GetPreviousVisionProcessing(m_pVisionInsp);
    long nFrameNum = m_parent->getImageLot().GetImageFrameCount();

    long nRointh = (id - INTENSITYCHECKER_ROI0_1) / MAX_FRAME_NUM;
    long nFramenth = (id - INTENSITYCHECKER_ROI0_1) % MAX_FRAME_NUM;

    // 초기값에 맞는 값이 들어갈 경우 max 프레임으로 설정
    if ((m_pVisionPara->m_vec2nROIFrameNum[nRointh][nFramenth] == nFrameNum)
        && (m_pVisionPara->m_vecROI[0].m_bottom == -1))
    {
        m_pVisionPara->m_vec2nROIFrameNum[nRointh][nFramenth] = nFrameNum - 1;
    }

    // 아닐경우 그냥 1번프레임 설정
    else if (m_pVisionPara->m_vec2nROIFrameNum[nRointh][nFramenth] >= nFrameNum)
    {
        m_pVisionPara->m_vec2nROIFrameNum[nRointh][nFramenth] = 0;
    }

    auto* itemSelectFrameID = parent->AddChildItem(
        new CXTPPropertyGridItemEnum(caption, m_pVisionPara->m_vec2nROIFrameNum[nRointh][nFramenth],
            (int*)&m_pVisionPara->m_vec2nROIFrameNum[nRointh][nFramenth]));

    //각 프레임별로 셋팅함
    for (long nFrameIndex = 0; nFrameIndex < nFrameNum; nFrameIndex++)
    {
        CString name = m_parent->getIllumInfo2D().getIllumFullName(nFrameIndex);
        itemSelectFrameID->GetConstraints()->AddConstraint(name, nFrameIndex);
    }

    itemSelectFrameID->SetID(id);

    return itemSelectFrameID;
}
