//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgVisionInspectionBgaBallPixelQuality.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionBgaBallPixelQuality.h"
#include "VisionInspectionBgaBallPixelQualityPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../ImageCombineModules/dPI_ImageCombine/dPI_ImageCombine.h"
#include "../../ImageCombineModules/dPI_ippModules/ippModules.h"
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ImageViewEx/PatternImage.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/EllipseEq32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100
#define UM_SHOW_DETAIL_RESULT (WM_USER + 2020)
#define UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED (WM_USER + 1011)

//CPP_7_________________________________ Implementation body
//
enum enumIntensityCheckAlgorithmParamID
{
    PIXEL_QUALITY_ALGO_PARAM_START = 0,
    PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_BEGIN_RATIO = PIXEL_QUALITY_ALGO_PARAM_START,
    PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_END_RATIO,
    PIXEL_QUALITY_ALGO_PARAM_REF_INTENSITY,
    PIXEL_QUALITY_ALGO_PARAM_DARK_PIXEL_MIN_RATIO,
    PIXEL_QUALITY_ALGO_PARAM_END,
};

static LPCTSTR g_szIntensityCheckAlgorithmParamName[] = {
    _T("Ring Option"),
    _T("Ring Width Begin Ratio"),
    _T("Ring Width End Ratio"),
    _T("Reference Threshold Value"),
    _T("Dark Pixel Min. Effective Ratio"),
};

static LPCTSTR g_szIntensityCheckAlgorithmParamEtcName[] = {
    _T(" "),
    _T(" %"),
    _T(" %"),
    _T("GV"),
    _T("%"),
};

enum enumDebugViewID
{
    _DEBUG_VIEW_START = 0,
    _DEBUG_VIEW_RING_ORIGIN = _DEBUG_VIEW_START,
    _DEBUG_VIEW_RING_MASK,
    _DEBUG_VIEW_RING_BRIGHT_PIXEL,
    _DEBUG_VIEW_RING_DARK_PIXEL,
    _DEBUG_VIEW_END,
};

enum enumControlID
{
    CONTROL_START = 100,
    CONTROL_COMBO = CONTROL_START,
    CONTROL_BUTTON,
    CONTROL_END,
};

static LPCTSTR g_szDebugViewName[] = {
    _T("Origin Image"),
    _T("Ring Mask Image"),
    _T("Ring Bright Pixel"),
    _T("Ring Dark Pixel"),
};

IMPLEMENT_DYNAMIC(CDlgVisionInspectionBgaBallPixelQuality, CDialog)

CDlgVisionInspectionBgaBallPixelQuality::CDlgVisionInspectionBgaBallPixelQuality(
    const ProcessingDlgInfo& procDlgInfo, VisionInspectionBgaBallPixelQuality* pVisionInsp, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgVisionInspectionBgaBallPixelQuality::IDD, pParent)
    , m_procDlgInfo(procDlgInfo)
    , m_pVisionInsp(pVisionInsp)
    , m_pVisionPara(pVisionInsp->m_VisionPara)
    , m_imageLotView(nullptr)
    , m_propertyGrid(nullptr)
    , m_procCommonSpecDlg(nullptr)
    , m_procCommonResultDlg(nullptr)
    , m_procCommonDetailResultDlg(nullptr)
    , m_procCommonLogDlg(nullptr)
    , m_procCommonDebugInfoDlg(nullptr)
{
    m_bShowResultDetail = FALSE;
    m_nImageID = 0;
}

CDlgVisionInspectionBgaBallPixelQuality::~CDlgVisionInspectionBgaBallPixelQuality()
{
    delete m_imageLotView;
    delete m_propertyGrid;
    delete m_procCommonSpecDlg;
    delete m_procCommonResultDlg;
    delete m_procCommonDetailResultDlg;
    delete m_procCommonLogDlg;
    delete m_procCommonDebugInfoDlg;

    m_pVisionInsp->m_pVisionInspDlg = NULL;
}

void CDlgVisionInspectionBgaBallPixelQuality::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_RESULT, m_TabResult);
}

BEGIN_MESSAGE_MAP(CDlgVisionInspectionBgaBallPixelQuality, CDialog)
ON_WM_CLOSE()
ON_WM_DESTROY()
ON_BN_CLICKED(IDC_BUTTON_INSPECT, &CDlgVisionInspectionBgaBallPixelQuality::OnBnClickedButtonInspect)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_RESULT, &CDlgVisionInspectionBgaBallPixelQuality::OnTcnSelchangeTabResult)
ON_MESSAGE(UM_SHOW_DETAIL_RESULT, &CDlgVisionInspectionBgaBallPixelQuality::OnShowDetailResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED, &CDlgVisionInspectionBgaBallPixelQuality::OnImageLotViewPaneSelChanged)
END_MESSAGE_MAP()

BOOL CDlgVisionInspectionBgaBallPixelQuality::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(rtParentClient, FALSE);

    CRect rtClient;
    GetClientRect(rtClient);

    m_imageLotView = new ImageLotView(m_procDlgInfo.m_rtImageArea, *m_pVisionInsp, GetSafeHwnd());

    m_imageLotView->SetMessage_SelChangedPane(UM_IMAGE_LOT_VIEW_PANE_SEL_CHANGED);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(m_procDlgInfo.m_rtParaArea, this, IDC_FRAME_PROPERTY_GRID);

    // Dialog Control 위치 설정
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

    UpdatePropertyGrid();

    m_imageLotView->ShowImage(m_pVisionInsp->GetImageFrameIndex(0), false);
    m_imageLotView->ZoomImageFit();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void CDlgVisionInspectionBgaBallPixelQuality::OnDestroy()
{
    CDialog::OnDestroy();

    m_procCommonDebugInfoDlg->DestroyWindow();
    m_procCommonLogDlg->DestroyWindow();
    m_procCommonResultDlg->DestroyWindow();
    m_procCommonDetailResultDlg->DestroyWindow();
    m_procCommonSpecDlg->DestroyWindow();
    m_propertyGrid->DestroyWindow();
}

void CDlgVisionInspectionBgaBallPixelQuality::OnClose()
{
    CDialog::OnDestroy();
}

long CDlgVisionInspectionBgaBallPixelQuality::GetTotalResult()
{
    long nResult = NOT_MEASURED;

    long nResultNum = (long)m_pVisionInsp->m_resultGroup.m_vecResult.size();
    for (long n = 0; n < nResultNum; n++)
    {
        nResult = max(m_pVisionInsp->m_resultGroup.m_vecResult[n].m_totalResult, nResult);
    }

    return nResult;
}

void CDlgVisionInspectionBgaBallPixelQuality::OnBnClickedButtonInspect()
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

void CDlgVisionInspectionBgaBallPixelQuality::OnTcnSelchangeTabResult(NMHDR* /*pNMHDR*/, LRESULT* pResult)
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

LRESULT CDlgVisionInspectionBgaBallPixelQuality::OnShowDetailResult(WPARAM wparam, LPARAM /*lparam*/)
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

void CDlgVisionInspectionBgaBallPixelQuality::OnBnClickedButtonClose()
{
    OnCancel();
}

void CDlgVisionInspectionBgaBallPixelQuality::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    long nNumberOfRings = (long)m_pVisionPara->m_vecsPixelQualityParam.size();
    long nOptionStartRow(0);
    CString strCurRingNumber;
    CString strTabTitle;
    CString strDebugView;

    if (auto* category = m_propertyGrid->AddCategory(_T("Ball Pixel	Quality Parameter")))
    {
        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Image Debug"), m_nImageID, (int*)&m_nImageID)))
        {
            item->GetConstraints()->AddConstraint(g_szDebugViewName[0], _DEBUG_VIEW_RING_ORIGIN);

            for (long nRing = 0; nRing < nNumberOfRings; nRing++)
            {
                nOptionStartRow = ((_DEBUG_VIEW_END - 1) * nRing);
                strCurRingNumber = m_pVisionInsp->GetStringCount(nRing + 1);
                strTabTitle.Format(_T("%s "), (LPCTSTR)strCurRingNumber);

                item->GetConstraints()->AddConstraint(
                    strTabTitle + g_szDebugViewName[1], nOptionStartRow + _DEBUG_VIEW_RING_MASK);
                item->GetConstraints()->AddConstraint(
                    strTabTitle + g_szDebugViewName[2], nOptionStartRow + _DEBUG_VIEW_RING_BRIGHT_PIXEL);
                item->GetConstraints()->AddConstraint(
                    strTabTitle + g_szDebugViewName[3], nOptionStartRow + _DEBUG_VIEW_RING_DARK_PIXEL);
            }

            item->SetID(CONTROL_COMBO);
        }
        category->AddChildItem(new CCustomItemButton(_T("Image Combine"), TRUE, FALSE))->SetID(CONTROL_BUTTON);

        category->Expand();
    }

    for (long nRing = 0; nRing < nNumberOfRings; nRing++)
    {
        nOptionStartRow = ((PIXEL_QUALITY_ALGO_PARAM_END)*nRing);
        strCurRingNumber = m_pVisionInsp->GetStringCount(nRing + 1);
        strTabTitle.Format(_T("%s %s"), (LPCTSTR)strCurRingNumber,
            g_szIntensityCheckAlgorithmParamName[PIXEL_QUALITY_ALGO_PARAM_START]);

        if (auto* category = m_propertyGrid->AddCategory(strTabTitle))
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(g_szIntensityCheckAlgorithmParamName[1],
                    m_pVisionPara->m_vecsPixelQualityParam[nRing].fRingWidthBeginRatio, _T("%.3lf %%")))
                ->SetID(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_BEGIN_RATIO);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(g_szIntensityCheckAlgorithmParamName[2],
                    m_pVisionPara->m_vecsPixelQualityParam[nRing].fRingWidthEndRatio, _T("%.3lf %%")))
                ->SetID(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_END_RATIO);
            if (auto* item
                = category->AddChildItem(new CXTPPropertyGridItemNumber(g_szIntensityCheckAlgorithmParamName[3],
                    m_pVisionPara->m_vecsPixelQualityParam[nRing].nReferenceIntensity)))
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
                item->SetID(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_REF_INTENSITY);
            }
            category
                ->AddChildItem(new CXTPPropertyGridItemNumber(g_szIntensityCheckAlgorithmParamName[4],
                    m_pVisionPara->m_vecsPixelQualityParam[nRing].nDarkPixelMinRatio))
                ->SetID(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_DARK_PIXEL_MIN_RATIO);

            category->Expand();
        }
    }

    /*if (auto *category = m_propertyGrid->AddCategory(_T("Threshold")))
	{
		if (auto *subItem = category->AddChildItem((new CCustomItemSlider(_T("Ring Thresh")))))
		{
			subItem->SetID(PIXEL_QUALITY_ALGO_PARAM_WIDTH_RATIO_TAB);
		}
	}*/

    m_propertyGrid->SetViewDivider(0.33);
    m_propertyGrid->HighlightChangedItems(TRUE);

    strCurRingNumber.Empty();
    strTabTitle.Empty();
    strDebugView.Empty();
}

LRESULT CDlgVisionInspectionBgaBallPixelQuality::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    long nNumberOfRings = (long)m_pVisionPara->m_vecsPixelQualityParam.size();
    long nMaxID = nNumberOfRings * 4;

    BOOL bDisplayThreshImage = FALSE;
    BOOL bChangedParam = FALSE;

    CString strTemp;
    long nTemp(0);

    long nCurRingID = item->GetID() / 4;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        long nOptionStartRow = ((PIXEL_QUALITY_ALGO_PARAM_END)*nCurRingID) + nCurRingID;

        switch (item->GetID() < CAST_UINT(nMaxID) ? item->GetID() % 4 : item->GetID())
        {
            case PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_BEGIN_RATIO:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    if (value->GetDouble() < 0.f || value->GetDouble() > 190.f)
                    {
                        strTemp.Format(_T("'%s' Range 0 ~ 190"),
                            g_szIntensityCheckAlgorithmParamName[PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_BEGIN_RATIO + 1]);
                        AfxMessageBox(strTemp, MB_OK | MB_ICONSTOP);
                        strTemp.Format(_T("%.3lf %%"), 30.f);
                        m_propertyGrid->GetItem(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_BEGIN_RATIO + 4)
                            ->SetValue(strTemp);
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].fRingWidthBeginRatio = 30.f;
                    }
                    else
                    {
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].fRingWidthBeginRatio
                            = (float)value->GetDouble();
                        bChangedParam = TRUE;
                        bDisplayThreshImage = TRUE;
                    }
                break;
            case PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_END_RATIO:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    if (value->GetDouble() < 10 || value->GetDouble() > 200)
                    {
                        strTemp.Format(_T("'%s' Range 10 ~ 200"),
                            g_szIntensityCheckAlgorithmParamName[PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_END_RATIO + 1]);
                        AfxMessageBox(strTemp, MB_OK | MB_ICONSTOP);
                        strTemp.Format(_T("%.3lf %%"), 90.f);
                        m_propertyGrid->GetItem(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_RING_WIDTH_END_RATIO + 4)
                            ->SetValue(strTemp);
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].fRingWidthEndRatio = 90.f;
                    }
                    else
                    {
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].fRingWidthEndRatio
                            = (float)value->GetDouble();
                        bChangedParam = TRUE;
                        bDisplayThreshImage = TRUE;
                    }
                break;
            case PIXEL_QUALITY_ALGO_PARAM_REF_INTENSITY:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    if (value->GetNumber() < 0 || value->GetNumber() > 255)
                    {
                        strTemp.Format(_T("'%s' Range 0 ~ 255"),
                            g_szIntensityCheckAlgorithmParamName[PIXEL_QUALITY_ALGO_PARAM_REF_INTENSITY + 1]);
                        AfxMessageBox(strTemp, MB_OK | MB_ICONSTOP);
                        strTemp.Format(_T("%d"), 200);
                        m_propertyGrid->GetItem(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_REF_INTENSITY + 4)
                            ->SetValue(strTemp);
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].nReferenceIntensity = 200;
                    }
                    else
                    {
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].nReferenceIntensity = value->GetNumber();
                        strTemp.Format(_T("%d"), nTemp);
                        SetDlgItemText(IDC_EDIT_Scroll1, strTemp);
                        bChangedParam = TRUE;
                        bDisplayThreshImage = TRUE;
                    }
                break;
            case PIXEL_QUALITY_ALGO_PARAM_DARK_PIXEL_MIN_RATIO:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    if (value->GetNumber() < 0 || value->GetNumber() > 100)
                    {
                        strTemp.Format(_T("'%s' Range 0 ~ 100"),
                            g_szIntensityCheckAlgorithmParamName[PIXEL_QUALITY_ALGO_PARAM_DARK_PIXEL_MIN_RATIO + 1]);
                        AfxMessageBox(strTemp, MB_OK | MB_ICONSTOP);
                        strTemp.Format(_T("%d"), 5);
                        m_propertyGrid->GetItem(nOptionStartRow + PIXEL_QUALITY_ALGO_PARAM_DARK_PIXEL_MIN_RATIO + 4)
                            ->SetValue(strTemp);
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].nDarkPixelMinRatio = 5;
                    }
                    else
                    {
                        m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].nDarkPixelMinRatio = value->GetNumber();
                        bChangedParam = TRUE;
                    }
                break;
            case CONTROL_COMBO:
                if (auto* value = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    SelchangeComboDebugView(value->GetEnum());
                break;
            case CONTROL_BUTTON:
                ClickedButtonImageCombine();
                break;
        }

        if (bChangedParam)
        {
            if (m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].fRingWidthBeginRatio
                >= m_pVisionPara->m_vecsPixelQualityParam[nCurRingID].fRingWidthEndRatio)
            {
                ::SimpleMessage(
                    _T("[Warning]\nIf 'Begin Ratio' is larger than 'End Ratio', inspection is impossible."));
            }
        }
        //{{ //kircheis_BPQDebug
        if (bDisplayThreshImage)
            UpdateThresholdImage(nCurRingID);
    }
    strTemp.Empty();

    return 0;
}

void CDlgVisionInspectionBgaBallPixelQuality::SelchangeComboDebugView(int nSelectedID)
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    m_imageLotView->Overlay_RemoveAll();
    long nSelectedRing = nSelectedID / _DEBUG_VIEW_END;
    long nSelectedData = nSelectedID % _DEBUG_VIEW_END;
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    Ipvm::Image8u combineImage;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
        return;
    if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false, Ipvm::Rect32s(image),
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
        FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
    }

    m_imageLotView->SetImage(combineImage, FrameToString);

    if (nSelectedID < 0)
        return;

    COLORREF crMask = RGB(0, 255, 0);
    COLORREF crRing = RGB(255, 0, 255);
    CString strRingID, strDebugRingName, strDebugImageName;
    strRingID = m_pVisionInsp->GetStringCount(nSelectedRing + 1);
    strDebugRingName.Format(_T("%s Ring Radius"), (LPCTSTR)strRingID);
    switch (nSelectedData)
    {
        case _DEBUG_VIEW_RING_MASK:
            strDebugImageName.Format(_T("%s Ring Mask"), (LPCTSTR)strRingID);
            break;
        case _DEBUG_VIEW_RING_BRIGHT_PIXEL:
            strDebugImageName.Format(_T("%s Ring Bright Pixel"), (LPCTSTR)strRingID);
            break;
        case _DEBUG_VIEW_RING_DARK_PIXEL:
            strDebugImageName.Format(_T("%s Ring Dark Pixel"), (LPCTSTR)strRingID);
            crMask = RGB(255, 0, 0);
            crRing = RGB(0, 255, 0);
            break;
        default:
            return;
    }

    Ipvm::Image8u* debugImage = NULL;
    std::vector<Ipvm::Rect32r> vecsEllipse(0);

    auto visionDebugInfos = m_pVisionInsp->m_visionUnit.GetVisionDebugInfos();

    long nDebugInfoNum = (long)(visionDebugInfos.size());
    BOOL bGetData = FALSE;
    VisionDebugInfo* pDebugInfo;
    for (long nIndex = 0; nIndex < nDebugInfoNum; nIndex++)
    {
        pDebugInfo = visionDebugInfos[nIndex];
        if (pDebugInfo->pData == NULL)
            continue;

        if (visionDebugInfos[nIndex]->strDebugInfoName == strDebugRingName)
        {
            long nSize = pDebugInfo->nDataNum;
            vecsEllipse.resize(nSize);
            Ipvm::EllipseEq32r* psEllipse = (Ipvm::EllipseEq32r*)(pDebugInfo->pData);
            for (long i = 0; i < nSize; i++)
                vecsEllipse[i] = Ipvm::Rect32r(psEllipse[i].m_x - psEllipse[i].m_xradius,
                    psEllipse[i].m_y - psEllipse[i].m_yradius, psEllipse[i].m_x + psEllipse[i].m_xradius,
                    psEllipse[i].m_y + psEllipse[i].m_yradius);

            if (bGetData)
                break;
            else
                bGetData = TRUE;
        }
        else if (visionDebugInfos[nIndex]->strDebugInfoName == strDebugImageName)
        {
            debugImage = (Ipvm::Image8u*)pDebugInfo->pData;
            if (bGetData)
                break;
            else
                bGetData = TRUE;
        }
    }

    if (debugImage == NULL)
        return;

    PatternImage pattern;
    pattern.setImage(combineImage);
    pattern.addPattern(*debugImage, crMask);

    m_imageLotView->Overlay_AddEllipses(vecsEllipse, crRing);
    m_imageLotView->SetImage(pattern);
    m_imageLotView->Overlay_Show(SW_SHOW);

    FrameToString.Empty();
    strRingID.Empty();
    strDebugRingName.Empty();
    strDebugImageName.Empty();
}

void CDlgVisionInspectionBgaBallPixelQuality::ClickedButtonImageCombine()
{
    auto* proc = m_pVisionInsp;

    if (::Combine_SetParameter(*proc, proc->GetImageFrameIndex(0), false, &proc->m_VisionPara->m_ImageProcMangePara)
        == IDOK)
    {
        if (m_pVisionInsp->GetImageFrameCount())
        {
            // 영훈 20150916 : 앞으로 영상 받는 부분은 통합하여 관리한다.
            Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
            if (image.GetMem() == nullptr)
                return;

            Ipvm::Image8u combineImage;
            if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
                return;

            if (!CippModules::GrayImageProcessingManage(m_pVisionInsp->getReusableMemory(), &image, false,
                    m_pVisionInsp->m_VisionPara->m_ImageProcMangePara, combineImage))
            {
                return;
            }

            CString FrameToString;

            if (m_pVisionInsp->m_VisionPara->m_ImageProcMangePara.isCombine())
            {
                FrameToString.Format(_T("Combined Image **"));
            }
            else
            {
                FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
            }

            m_imageLotView->SetImage(combineImage, FrameToString);

            FrameToString.Empty();
        }
    }
}

void CDlgVisionInspectionBgaBallPixelQuality::UpdateThresholdImage(long nRingID) //kircheis_BPQDebug
{
    if (nRingID >= m_pVisionPara->m_vecsPixelQualityParam.size() || nRingID < 0)
        return;

    //{{ Ball 영역 Data를 획득
    if (m_pVisionInsp->GetBallROI() == FALSE)
        return;

    long nBallNum = (long)m_pVisionInsp->m_vecrtBallCircleROI.size();
    //}}
    //{{ 필요한 영상 획득 단계. 본 Frame와 보조 Frame을 가져와서 영상 전처리 진행
    Ipvm::Image8u image = m_pVisionInsp->GetInspectionFrameImage();
    if (image.GetMem() == nullptr)
        return;

    m_imageLotView->Overlay_RemoveAll();

    Ipvm::Image8u combineImage;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(combineImage))
        return;

    if (!CippModules::GrayImageProcessingManage(
            m_pVisionInsp->getReusableMemory(), &image, false, m_pVisionPara->m_ImageProcMangePara, combineImage))
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
        FrameToString = m_pVisionInsp->getIllumInfo2D().getIllumFullName(m_pVisionInsp->GetImageFrameIndex(0));
    }

    //}}

    //{{ 필요한 버퍼 영상 및 ROI와 Ellipse 가져오기

    Ipvm::Image8u ringMarkImage;
    Ipvm::Image8u brightImage;
    Ipvm::Image8u darkImage;
    Ipvm::Image8u thresholdImage;

    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(ringMarkImage))
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(brightImage))
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(darkImage))
        return;
    if (!m_pVisionInsp->getReusableMemory().GetInspByteImage(thresholdImage))
        return;

    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(ringMarkImage), 0, ringMarkImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(brightImage), 0, brightImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(darkImage), 0, darkImage);
    Ipvm::ImageProcessing::Fill(Ipvm::Rect32s(thresholdImage), 0, thresholdImage);

    std::vector<Ipvm::Rect32s> vecrtRingROI(nBallNum);
    std::vector<Ipvm::EllipseEq32r> vecsBeginEllipse(nBallNum);
    std::vector<Ipvm::EllipseEq32r> vecsEndEllipse(nBallNum);
    m_pVisionInsp->MakeBallCircleROI(nBallNum, nRingID, &vecrtRingROI[0]);
    m_pVisionInsp->MakeBallEllipse(nBallNum, nRingID, &vecsBeginEllipse[0], &vecsEndEllipse[0]);
    //}}

    //{{ Display할 영상 만들기
    BOOL bEraseBegin = (m_pVisionPara->m_vecsPixelQualityParam[nRingID].fRingWidthBeginRatio > 0.f);
    BYTE threshold = (BYTE)min(255, max(0, m_pVisionPara->m_vecsPixelQualityParam[nRingID].nReferenceIntensity));
    Ipvm::Rect32s rtCurROI;
    Ipvm::EllipseEq32r sCurBeginEllipse, sCurEndEllipse;

    for (long nBall = 0; nBall < nBallNum; nBall++)
    {
        //Ref Data 만들기
        rtCurROI = vecrtRingROI[nBall];
        rtCurROI.InflateRect(2, 2);
        sCurBeginEllipse = vecsBeginEllipse[nBall];
        sCurEndEllipse = vecsEndEllipse[nBall];

        //Ring Mask 만들기
        Ipvm::ImageProcessing::Fill(sCurEndEllipse, 255, ringMarkImage); //외곽 링을 채우고
        if (bEraseBegin)
        {
            //내부 링의 Ratio가 0보다 클 때
            Ipvm::ImageProcessing::Fill(sCurBeginEllipse, 0, ringMarkImage); //내부 링을 지우고
        }

        //이진 영상 만들기
        Ipvm::ImageProcessing::BinarizeGreaterEqual(combineImage, rtCurROI, threshold, thresholdImage);

        //Bright Image만들기
        Ipvm::ImageProcessing::BitwiseAnd(ringMarkImage, thresholdImage, rtCurROI, brightImage);

        //Dark Image 만들기
        Ipvm::ImageProcessing::Subtract(ringMarkImage, thresholdImage, rtCurROI, 0, darkImage);
    }
    //}}

    //{{ Display 하기
    PatternImage pattern;
    pattern.setImage(combineImage);
    pattern.addPattern(brightImage, RGB(0, 255, 0));
    pattern.addPattern(darkImage, RGB(255, 0, 0));

    m_imageLotView->SetImage(pattern, FrameToString);
    m_imageLotView->Overlay_Show(SW_SHOW);

    FrameToString.Empty();
    //}}
}

LRESULT CDlgVisionInspectionBgaBallPixelQuality::OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/)
{
    m_pVisionInsp->m_visionUnit.RunInspection(
        m_pVisionInsp->m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_BGA_BALL_2D), false,
        m_pVisionInsp->GetCurVisionModule_Status());

    // Run을 해서 이제야 Raw가 아닌 검사영상을 보여줄수 있다
    m_imageLotView->ShowCurrentImage(false);
    m_imageLotView->ZoomImageFit();

    return 0L;
}
