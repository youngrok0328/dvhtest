//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionProcessingOtherAlign.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingOtherAlign.h"
#include "VisionProcessingOtherAlignPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_LIST 100
#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)
#define UM_IMAGE_LOT_VIEW_ROI_CHANGED (WM_USER + 1012)

//CPP_7_________________________________ Implementation body
//
enum enumEDGEAlignParamID
{
    OTHER_ALIGN_PARAM_START = 1,
    OTHER_ALIGN_PARAM_IMAGECOMBINE = OTHER_ALIGN_PARAM_START,
    OTHER_ALIGN_PARAM_SELECT_DIRECTION,
    OTHER_ALIGN_PARAM_USE_BODY_ALIGN_RESULT,
    OTHER_ALIGN_PARAM_SEARCH_DIRECTION,
    OTHER_ALIGN_PARAM_EDGE_DIRECTION,
    OTHER_ALIGN_PARAM_EDGE_DETECT_MODE,
    OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE,
    OTHER_ALIGN_PARAM_SEARCH_LENGTH,
    OTHER_ALIGN_PARAM_END,
};

enum enumSelectDirection
{
    SELECT_DIRECTION_START = 0,
    SELECT_DIRECTION_LEFT = SELECT_DIRECTION_START,
    SELECT_DIRECTION_TOP,
    SELECT_DIRECTION_RIGHT,
    SELECT_DIRECTION_BOTTOM,
    SELECT_DIRECTION_END,
};

enum enumSearchDirection
{
    SEARCH_DIRECTION_START = 0,
    SEARCH_DIRECTION_IN_OUT = SEARCH_DIRECTION_START,
    SEARCH_DIRECTION_OUT_IN,
    SEARCH_DIRECTION_END,
};

enum enumEdgeDirection
{
    EDGE_DIRECTION_START = 0,
    EDGE_DIRECTION_BOTH = EDGE_DIRECTION_START,
    EDGE_DIRECTION_RISING,
    EDGE_DIRECTION_FALLING,
    EDGE_DIRECTION_END,
};

enum enumEdgeDetectMode
{
    EDGE_DETECT_MODE_START = 0,
    EDGE_DETECT_MODE_BEST = EDGE_DETECT_MODE_START,
    EDGE_DETECT_MODE_FIRST,
    EDGE_DETECT_MODE_END,
};

static LPCTSTR g_szEDGEAlignParamName[] = {
    _T("Edge Detect Parameter"),
    _T("Select Direction"),
    _T("Use Body Align Result"),
    _T("Search Direction"),
    _T("Edge Direction"),
    _T("Edge Detect Mode"),
    _T("First Edge Value"),
    _T("Edge Search Length"),
};

IMPLEMENT_DYNAMIC(CDlgVisionProcessingOtherAlign, CDialog)

CDlgVisionProcessingOtherAlign::CDlgVisionProcessingOtherAlign(
    const ProcessingDlgInfo& procDlgInfo, VisionProcessingOtherAlign* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionProcessingOtherAlign::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_VisionPara)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
{
    m_nImageID = 0;
}

CDlgVisionProcessingOtherAlign::~CDlgVisionProcessingOtherAlign()
{
    delete m_procCommonDebugInfoDlg;
    delete m_procCommonLogDlg;
    delete m_propertyGrid;
    delete m_imageLotView;

    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionProcessingOtherAlign::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(CDlgVisionProcessingOtherAlign, CDialog)
//ON_WM_CREATE()
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionProcessingOtherAlign::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionProcessingOtherAlign::OnImageLotViewPaneSelChanged)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_ROI_CHANGED, &CDlgVisionProcessingOtherAlign::OnImageLotViewRoiChanged)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// CDlgVisionProcessingOtherAlign 메시지 처리기입니다.
BOOL CDlgVisionProcessingOtherAlign::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_RoiChanged(UM_IMAGE_LOT_VIEW_ROI_CHANGED);
    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

    // Dialog Control 위치 설정
    CRect rtTab = m_procDlgInfo.m_rtDataArea;
    m_TabResult.MoveWindow(rtTab);

    m_TabResult.InsertItem(0, _T("Debug"));
    m_TabResult.InsertItem(1, _T("Log"));

    m_TabResult.AdjustRect(FALSE, rtTab);

    m_procCommonLogDlg = new ProcCommonLogDlg(this, rtTab);
    m_procCommonDebugInfoDlg = new ProcCommonDebugInfoDlg(
        this, rtTab, m_pVisionInsp->m_DebugInfoGroup, *m_imageLotView, *m_procCommonLogDlg);

    m_procCommonDebugInfoDlg->ShowWindow(SW_SHOW);

    UpdatePropertyGrid();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionProcessingOtherAlign::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionProcessingOtherAlign::OnClose()
{
    CDialog::OnDestroy();
}

void CDlgVisionProcessingOtherAlign::OnBnClickedButtonInspect()
{
    m_imageLotView->Overlay_RemoveAll();
    m_procCommonLogDlg->SetLogBoxText(_T(""));

    auto& visionUnit = m_pVisionInsp->m_visionUnit;

    visionUnit.RunInspection(m_pVisionInsp);
    visionUnit.GetInspectionOverlayResult().Apply(m_imageLotView->GetCoreView());
    m_procCommonLogDlg->SetLogBoxText(visionUnit.GetLastInspection_Text());
    m_imageLotView->Overlay_Show(TRUE);

    // 검사 중간 결과 화면 갱신
    m_procCommonDebugInfoDlg->Refresh();
}

void CDlgVisionProcessingOtherAlign::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_TabResult.GetCurSel();

    m_procCommonDebugInfoDlg->ShowWindow(nTab == 0 ? SW_SHOW : SW_HIDE);
    m_procCommonLogDlg->ShowWindow(nTab == 1 ? SW_SHOW : SW_HIDE);

    if (pResult)
    {
        *pResult = 0;
    }
}

LRESULT CDlgVisionProcessingOtherAlign::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(m_pVisionInsp->m_visionUnit.GetVisionAlignProcessing());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    SetROI();

    return 0L;
}

LRESULT CDlgVisionProcessingOtherAlign::OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    GetROI();

    return 0L;
}

void CDlgVisionProcessingOtherAlign::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    long nIdx = m_pVisionPara->m_nSelectDirection;

    CString strSelectDirection[4] = {_T(" (L)"), _T(" (T)"), _T(" (R)"), _T(" (B)")};

    if (auto* category = m_propertyGrid->AddCategory(_T("Other Align Paramter")))
    {
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))
            ->SetID(OTHER_ALIGN_PARAM_IMAGECOMBINE);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(g_szEDGEAlignParamName[0]))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(g_szEDGEAlignParamName[1],
                m_pVisionPara->m_nSelectDirection))) //, (int *)m_pVisionPara->m_nSelectDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("LEFT"), SELECT_DIRECTION_LEFT);
            item->GetConstraints()->AddConstraint(_T("TOP"), SELECT_DIRECTION_TOP);
            item->GetConstraints()->AddConstraint(_T("RIGHT"), SELECT_DIRECTION_RIGHT);
            item->GetConstraints()->AddConstraint(_T("BOTTOM"), SELECT_DIRECTION_BOTTOM);

            item->SetID(OTHER_ALIGN_PARAM_SELECT_DIRECTION);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemBool(g_szEDGEAlignParamName[2], m_pVisionPara->m_arbUseEdgeAlignResult[nIdx])))
        {
            item->SetID(OTHER_ALIGN_PARAM_USE_BODY_ALIGN_RESULT);
            (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();
        }
        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(g_szEDGEAlignParamName[3] + strSelectDirection[nIdx],
                m_pVisionPara->m_nSearchDirection[nIdx]))) //, (int *)m_pVisionPara->m_nSearchDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), SEARCH_DIRECTION_IN_OUT);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), SEARCH_DIRECTION_OUT_IN);

            item->SetID(OTHER_ALIGN_PARAM_SEARCH_DIRECTION);
        }
        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(g_szEDGEAlignParamName[4] + strSelectDirection[nIdx],
                m_pVisionPara->m_nEdgeDirection[nIdx]))) //, (int *)m_pVisionPara->m_nEdgeDirection)))
        {
            item->GetConstraints()->AddConstraint(_T("Both"), EDGE_DIRECTION_BOTH);
            item->GetConstraints()->AddConstraint(_T("Rising"), EDGE_DIRECTION_RISING);
            item->GetConstraints()->AddConstraint(_T("Falling"), EDGE_DIRECTION_FALLING);

            item->SetID(OTHER_ALIGN_PARAM_EDGE_DIRECTION);
        }
        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(g_szEDGEAlignParamName[5] + strSelectDirection[nIdx],
                m_pVisionPara->m_nEdgeDetectMode[nIdx]))) //, (int *)m_pVisionPara->m_nEdgeDetectMode)))
        {
            item->GetConstraints()->AddConstraint(_T("Best Edge"), EDGE_DETECT_MODE_BEST);
            item->GetConstraints()->AddConstraint(_T("First Edge"), EDGE_DETECT_MODE_FIRST);

            item->SetID(OTHER_ALIGN_PARAM_EDGE_DETECT_MODE);
        }
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[6] + strSelectDirection[nIdx], m_pVisionPara->m_nFirstEdgeValue[nIdx]))
            ->SetID(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE);
        category
            ->AddChildItem(new CXTPPropertyGridItemNumber(
                g_szEDGEAlignParamName[7] + strSelectDirection[nIdx], m_pVisionPara->m_nEdgeSearchLength[nIdx]))
            ->SetID(OTHER_ALIGN_PARAM_SEARCH_LENGTH);

        category->Expand();
    }

    if (m_pVisionPara->m_arbUseEdgeAlignResult[nIdx] == TRUE)
    {
        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_SEARCH_DIRECTION + 1)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_EDGE_DIRECTION + 1)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_EDGE_DETECT_MODE + 1)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(TRUE);
        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_SEARCH_LENGTH + 1)->SetReadOnly(TRUE);
    }

    if (m_pVisionPara->m_nEdgeDetectMode[nIdx] != EDGE_DETECT_MODE_FIRST)
    {
        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(TRUE);
    }

    SetROI();

    m_propertyGrid->SetViewDivider(0.33);
    m_propertyGrid->HighlightChangedItems(TRUE);
}

LRESULT CDlgVisionProcessingOtherAlign::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    CString strTemp;
    const auto& scale = m_pVisionInsp->getScale();

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        long nIdx = m_pVisionPara->m_nSelectDirection;
        float fScale2D
            = (m_pVisionPara->m_nSelectDirection == enum_Left || m_pVisionPara->m_nSelectDirection == enum_Right)
            ? scale.pixelToUm().m_x
            : scale.pixelToUm().m_y;
        long nSearchLowLimit_um = (long)(6.f * fScale2D + .5f);
        const long nSearchHighLimit_um = 2000;

        switch (item->GetID())
        {
            case OTHER_ALIGN_PARAM_IMAGECOMBINE:
                ClickedButtonImageCombine();
                break;
            case OTHER_ALIGN_PARAM_SELECT_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                {
                    m_pVisionPara->m_nSelectDirection = value->GetEnum();
                    UpdatePropertyGrid();
                }
                break;
            case OTHER_ALIGN_PARAM_USE_BODY_ALIGN_RESULT:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemBool*>(item))
                    m_pVisionPara->m_arbUseEdgeAlignResult[nIdx] = value->GetBool();

                if (m_pVisionPara->m_arbUseEdgeAlignResult[nIdx] == TRUE)
                {
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_SEARCH_DIRECTION + 1)->SetReadOnly(TRUE);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_EDGE_DIRECTION + 1)->SetReadOnly(TRUE);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_EDGE_DETECT_MODE + 1)->SetReadOnly(TRUE);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(TRUE);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_SEARCH_LENGTH + 1)->SetReadOnly(TRUE);
                }
                else
                {
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_SEARCH_DIRECTION + 1)->SetReadOnly(FALSE);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_EDGE_DIRECTION + 1)->SetReadOnly(FALSE);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_EDGE_DETECT_MODE + 1)->SetReadOnly(FALSE);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_SEARCH_LENGTH + 1)->SetReadOnly(FALSE);

                    if (m_pVisionPara->m_nEdgeDetectMode[nIdx] == EDGE_DETECT_MODE_FIRST)
                    {
                        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(FALSE);
                    }
                }
                break;
            case OTHER_ALIGN_PARAM_SEARCH_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nSearchDirection[nIdx] = value->GetEnum();
                break;
            case OTHER_ALIGN_PARAM_EDGE_DIRECTION:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nEdgeDirection[nIdx] = value->GetEnum();
                break;
            case OTHER_ALIGN_PARAM_EDGE_DETECT_MODE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    m_pVisionPara->m_nEdgeDetectMode[nIdx] = value->GetEnum();

                if (m_pVisionPara->m_nEdgeDetectMode[nIdx] != EDGE_DETECT_MODE_FIRST)
                {
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(TRUE);
                }
                else
                {
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetReadOnly(FALSE);
                }
                break;
            case OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    if (value->GetNumber() < 0 || value->GetNumber() > 30)
                    {
                        m_pVisionPara->m_nFirstEdgeValue[nIdx] = 0;
                        strTemp.Format(_T("%d"), m_pVisionPara->m_nFirstEdgeValue[nIdx]);
                        m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_FIRST_EDGE_VALUE + 1)->SetValue(strTemp);
                    }
                    else
                        m_pVisionPara->m_nFirstEdgeValue[nIdx] = value->GetNumber();
                break;
            case OTHER_ALIGN_PARAM_SEARCH_LENGTH:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                {
                    m_pVisionPara->m_nEdgeSearchLength[nIdx]
                        = min(nSearchHighLimit_um, max(nSearchLowLimit_um, value->GetNumber()));
                    strTemp.Format(_T("%d"), m_pVisionPara->m_nEdgeSearchLength[nIdx]);
                    m_propertyGrid->GetItem(OTHER_ALIGN_PARAM_SEARCH_LENGTH + 1)->SetValue(strTemp);
                }
                break;
        }

        UpdateROI();
    }

    return 0;
}

void CDlgVisionProcessingOtherAlign::GetROI()
{
    //{{//kircheis_USI_OA
    m_pVisionInsp->GetEdgeAlignResult();
    Ipvm::Point32r2 bodyCenter = m_pVisionInsp->m_sEdgeAlignResult->m_center;

    //{{ kircheis_USI_OA Package Center와의 거리를 저장할 필요도 있다.
    const auto& scale = m_pVisionInsp->getScale();
    long nROINum = (long)m_pVisionPara->m_vecrtSearchROI_BCU.size();

    for (long nIndex = 0; nIndex < nROINum; nIndex++)
    {
        CString key;
        key.Format(_T("%d"), nIndex);

        Ipvm::Rect32s roi;
        m_imageLotView->ROI_Get(key, roi);
        //{{//kircheis_USI_OA
        if (m_pVisionInsp->m_sEdgeAlignResult->bAvailable)
        {
            Ipvm::Point32r2 ptCenterOfROI((roi.m_left + roi.m_right) * 0.5f, (roi.m_top + roi.m_bottom) * 0.5f);
            m_pVisionPara->m_fptBodyCenterOffset_um.m_x = scale.convert_pixelToUmX(bodyCenter.m_x - ptCenterOfROI.m_x);
            m_pVisionPara->m_fptBodyCenterOffset_um.m_y = scale.convert_pixelToUmY(bodyCenter.m_y - ptCenterOfROI.m_y);
        }
        //}}

        m_pVisionPara->m_vecrtSearchROI_BCU[nIndex] = scale.convert_PixelToBCU(roi, bodyCenter);

        key.Empty();
    }
}

void CDlgVisionProcessingOtherAlign::SetROI()
{
    m_imageLotView->ROI_RemoveAll();
    m_pVisionInsp->GetEdgeAlignResult(); //kircheis_USI_OA

    Ipvm::Rect32s imageRoi(0, 0, m_pVisionInsp->getReusableMemory().GetInspImageSizeX(),
        m_pVisionInsp->getReusableMemory().GetInspImageSizeY());
    Ipvm::Rect32s rtROITemp = Ipvm::Rect32s(0, 0, 100, 100);

    //{{ kircheis_USI_OA Package Center와의 거리를 참조할 필요도 있다.
    m_pVisionInsp->GetEdgeAlignResult();
    Ipvm::Point32r2 fptBodyCenter = m_pVisionInsp->m_sEdgeAlignResult->m_center;
    Ipvm::Point32r2 fptBodyCenterOffset_um = m_pVisionPara->m_fptBodyCenterOffset_um;
    CPoint ptCenterOffsetOfROI;
    //}}

    if (m_pVisionPara->m_vecrtSearchROI_BCU.size() == 0)
    {
        m_pVisionPara->m_vecrtSearchROI_BCU.resize(1, Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f));
    }

    const auto& scale = m_pVisionInsp->getScale();

    for (long nIndex = 0; nIndex < long(m_pVisionPara->m_vecrtSearchROI_BCU.size()); nIndex++)
    {
        CString key;
        CString strName;

        key.Format(_T("%d"), nIndex);

        // TODO: BUG!!!!!!
        strName.Format(_T("Object Detect ROI %d"), nIndex);
        Ipvm::Rect32s rtROI = scale.convert_BCUToPixel(m_pVisionPara->m_vecrtSearchROI_BCU[nIndex], fptBodyCenter);
        //{{//kircheis_USI_OA
        if (fptBodyCenterOffset_um.m_x > -90000.f && fptBodyCenterOffset_um.m_y > -90000.f
            && m_pVisionInsp->m_sEdgeAlignResult->bAvailable)
        {
            ptCenterOffsetOfROI.x = rtROI.CenterPoint().m_x
                - (long)(fptBodyCenter.m_x - scale.convert_umToPixelX(fptBodyCenterOffset_um.m_x) + .5f);
            ptCenterOffsetOfROI.y = rtROI.CenterPoint().m_y
                - (long)(fptBodyCenter.m_y - scale.convert_umToPixelY(fptBodyCenterOffset_um.m_y) + .5f);
            rtROI.OffsetRect(-ptCenterOffsetOfROI.x, -ptCenterOffsetOfROI.y);
        }
        //}}
        rtROI &= imageRoi;
        m_imageLotView->ROI_Add(key, strName, rtROI, RGB(255, 0, 0), TRUE, TRUE);

        key.Empty();
        strName.Empty();
    }

    m_imageLotView->ROI_Show(TRUE);
}

void CDlgVisionProcessingOtherAlign::UpdateROI()
{
    GetROI();

    long nROINum = m_pVisionPara->m_nSearchROICount;
    m_pVisionPara->m_vecrtSearchROI_BCU.resize(nROINum, Ipvm::Rect32r(-500.f, -500.f, 500.f, 500.f));

    SetROI();
}

void CDlgVisionProcessingOtherAlign::ClickedButtonImageCombine()
{
    auto* proc = m_pVisionInsp;

    if (::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), false, &proc->m_VisionPara->m_ImageProcMangePara)
        == IDOK)
    {
        // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
        Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
        if (image.GetMem() == nullptr)
            return;

        Ipvm::Image8u combineImage;
        if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
            return;

        Ipvm::Rect32s rtProcessingROI(0, 0, image.GetSizeX(), image.GetSizeY());
        if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, rtProcessingROI,
                m_pVisionInsp->m_VisionPara->m_ImageProcMangePara, combineImage))
        {
            return;
        }

        m_imageLotView->SetImage(combineImage);
    }
}
