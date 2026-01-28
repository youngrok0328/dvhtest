//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgComponentAlign.h"

//CPP_2_________________________________ This project's headers
#include "DlgHistogramView.h"
#include "DlgVisionInspectionComponent3D.h"
#include "VisionInspectionComponent3D.h"
#include "VisionInspectionComponent3DPara.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomNumber.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum debugResult
{
    DEBUG_SPEC_CHIP_ROI,
    DEBUG_RES_ROUGH_ROI,
    DEBUG_RES_ROUGH_ELECTRODE_ROI,
    DEBUG_RES_ROUGH_BODY_ROI,
    DEBUG_COMPONENT_SEARCH_ROI,
    DEBUG_COMPONENT_EDGE_POINTS,
    /*DEBUG_ELECTRODE_INNER_SEARCH_ROI,
	DEBUG_ELECTRODE_INNER_EDGE_POINT,
	DEBUG_ELECTRODE_INNER_FINAL_EDGE_POINT,
	DEBUG_ELECTRODE_OUTER_SEARCH_ROI,
	DEBUG_ELECTRODE_OUTER_EDGE_POINT,
	DEBUG_ELECTRODE_OUTER_FINAL_EDGE_POINT,*/
    DEBUG_RES_VALID_COMPONENT_ROI,
    DEBUG_RES_VALID_ELECTRODE_ROI,
    DEBUG_RES_VALID_BODY_ROI,
    /*DEBUG_RES_CENTROID_ELECTRODE_POINTS,
	DEBUG_RES_MEASURE_ELECTRODE_RECT,*/
};

enum PropertyGridItemID
{
    ITEM_ID_ROUGH_ALIGN_SEARCH_ROI_EXTENSION_RATIO = 1,
    ITEM_ID_SPEC_ROI_SHIFT_X,
    ITEM_ID_SPEC_ROI_SHIFT_Y,
    ITEM_ID_RECALC_USE_SPEC_ROI,
    ITEM_ID_VALID_Z_RANGE,
    ITEM_ID_VISIBILITY_LOWER_BOUND,

    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_WIDTH_RATIO,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_LENGTH_UM,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_THRESHOLD,

    ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_WIDTH_RATIO,
    ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_LENGTH_UM,
    ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_THRESHOLD,

    ITEM_ID_MEASURE_COMPONENT,
    ITEM_ID_MEASURE_BODY,
    ITEM_ID_MEASURE_ELECTRODE,

    ITEM_ID_CALC_ELECTRODE_VAILD_ROI_WIDTH_RATIO,
    ITEM_ID_CALC_ELECTRODE_VAILD_ROI_LENGTH_RATIO,

    ITEM_ID_CALC_BODY_VAILD_ROI_WIDTH_RATIO,
    ITEM_ID_CALC_BODY_VAILD_ROI_LENGTH_RATIO,
    ITEM_ID_CALC_BODY_VAILD_ROI_DISTANCE_X_UM,
    ITEM_ID_CALC_BODY_VAILD_ROI_DISTANCE_Y_UM,

    ITEM_ID_CALC_SUBSTRATE_ROI_SIZE_UM,
    ITEM_ID_CALC_SUBSTRATE_ROI_X_OFFSET_FOR_BODY_UM,
    ITEM_ID_CALC_SUBSTRATE_ROI_Y_OFFSET_FOR_BODY_UM,
    ITEM_ID_CALC_SUBSTRATE_ROI_SHIFT_X_UM,
    ITEM_ID_CALC_SUBSTRATE_ROI_SHIFT_Y_UM,
};

IMPLEMENT_DYNAMIC(CDlgComponentAlign, CDialog)

CDlgComponentAlign::CDlgComponentAlign(CComponentAlign* pComponentAlign, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgComponentAlign::IDD, pParent)
    , m_propertyGrid(nullptr)
{
    m_pCompAlign = pComponentAlign;

    for (long n = 0; n < MAX_HISTOGRMA_VIEWER; n++)
    {
        m_pDlgHistoViewer[n] = new CDlgHistogramView;
        m_pDlgHistoViewer[n]->Create(CDlgHistogramView::IDD);
    }
}

CDlgComponentAlign::~CDlgComponentAlign()
{
    for (long n = 0; n < MAX_HISTOGRMA_VIEWER; n++)
    {
        delete m_pDlgHistoViewer[n];
        m_pDlgHistoViewer[n] = NULL;
    }

    delete m_propertyGrid;
}

void CDlgComponentAlign::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_DEBUG_RESULT, m_cmbDebugResult);
    DDX_Control(pDX, IDC_COMBO_INSPECTION, m_cmbInsp);
}

BEGIN_MESSAGE_MAP(CDlgComponentAlign, CDialog)
ON_BN_CLICKED(IDC_BTN_TEACH, &CDlgComponentAlign::OnBnClickedBtnTeach)
ON_CBN_SELCHANGE(IDC_COMBO_DEBUG_RESULT, &CDlgComponentAlign::OnCbnSelchangeComboDebugResult)
ON_WM_SHOWWINDOW()
ON_CBN_SELCHANGE(IDC_COMBO_INSPECTION, &CDlgComponentAlign::OnCbnSelchangeComboInspection)
ON_BN_CLICKED(IDC_BTN_VIEW_HISTOGRAM, &CDlgComponentAlign::OnBnClickedBtnViewHistogram)
ON_BN_CLICKED(IDC_BTN_VIEW_RESULT, &CDlgComponentAlign::OnBnClickedBtnViewResult)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// CDlgComponentAlign 메시지 처리기입니다.

CString CDlgComponentAlign::GetValueString(float value)
{
    CString data;
    if (value != Ipvm::k_noiseValue32r)
    {
        data.Format(_T("%.2f"), value);
    }
    else
    {
        data = _T("INV");
    }

    return data;
}

void CDlgComponentAlign::OnBnClickedBtnTeach()
{
    //BUG: 중복 코드임
    if (m_pCompAlign->m_pChipVision->m_pVisionInspDlg == nullptr
        || m_pCompAlign->m_pChipVision->m_pVisionInspDlg == nullptr)
        return;

    auto* VisionInspDlg = m_pCompAlign->m_pChipVision->m_pVisionInspDlg;
    auto* SelectPassiveInfoDB = VisionInspDlg->m_pCurPassiveInfoDB;

    VisionInspDlg->m_imageLotView->Overlay_RemoveAll();
    VisionInspDlg->m_imageLotView->ROI_RemoveAll();

    m_pCompAlign->m_result.Clear();

    const auto& scale = m_pCompAlign->m_pChipVision->getScale();

    const auto& ChipVision = m_pCompAlign->m_pChipVision;

    Ipvm::Image32r curZmap = ChipVision->getImageLotInsp().m_zmapImage;
    Ipvm::Image16u curWIntensity = ChipVision->getImageLotInsp().m_vmapImage;
    Ipvm::Image8u curHMap = ChipVision->getImageLotInsp().m_vecImages[ChipVision->GetCurVisionModule_Status()][0];
    Ipvm::Image8u curByInten = ChipVision->getImageLotInsp().m_vecImages[ChipVision->GetCurVisionModule_Status()][1];

    Ipvm::Image8u AlignimageBuffer;
    if (ChipVision->getReusableMemory().GetInspByteImage(AlignimageBuffer) == false)
        return;

    AlignimageBuffer.FillZero();

    if (m_pCompAlign->DoAlign(
            curZmap, curWIntensity, curHMap, curByInten, AlignimageBuffer, scale, SelectPassiveInfoDB, &m_debugResult))
    {
        ResultText(m_cmbInsp.GetCurSel(), FALSE);
    }

    VisionInspDlg->m_imageLotView->Overlay_Show(TRUE);
}

void CDlgComponentAlign::OnCbnSelchangeComboDebugResult()
{
    ImageLotView* pImaegDisp = m_pCompAlign->m_pChipVision->m_pVisionInspDlg->m_imageLotView;
    if (pImaegDisp == NULL)
        return;

    pImaegDisp->Overlay_RemoveAll();
    pImaegDisp->ROI_RemoveAll();

    COLORREF Color = RGB(0, 255, 0);
    long nSel = m_cmbDebugResult.GetCurSel();
    SComponentAlignResult result = m_pCompAlign->m_result;
    switch (nSel)
    {
        case DEBUG_SPEC_CHIP_ROI:
            pImaegDisp->Overlay_AddRectangle(result.rtSpecChip, Color); // Spec Chip ROI
            break;
        case DEBUG_RES_ROUGH_ROI: //
            pImaegDisp->Overlay_AddRectangle(result.rtResChip, Color); // Res Chip ROI
            break;
        case DEBUG_RES_ROUGH_ELECTRODE_ROI: //
            pImaegDisp->Overlay_AddRectangles(result.vecrtResElect, Color); // Res Electrode ROI
            break;
        case DEBUG_RES_ROUGH_BODY_ROI:
            pImaegDisp->Overlay_AddRectangle(result.rtResBody, Color); // Res Body ROI
            break;
        case DEBUG_COMPONENT_SEARCH_ROI: // Component Search ROI
            pImaegDisp->Overlay_AddRectangles(result.Debug_vecrtDetailPassiveSearchROI, Color);
            break;
        case DEBUG_COMPONENT_EDGE_POINTS: // Component Edge Points
            pImaegDisp->Overlay_AddPoints(result.Debug_vecfptPassiveEdge, Color);
            break;
            //pImaegDisp->Overlay_AddLine
            //case DEBUG_ELECTRODE_INNER_FINAL_EDGE_POINT: // Electrode Inner Final Edge Points
            //	pImaegDisp->Overlay_AddPoints(m_pCompAlign->m_result.vecfptFinalEdgeElectInner, Color);
            //	break;
        case DEBUG_RES_VALID_COMPONENT_ROI: // Res Valid Component ROI
            pImaegDisp->Overlay_AddRectangle(result.rtValidComponent, Color);
            break;
        case DEBUG_RES_VALID_ELECTRODE_ROI: // Res Valid Electrode ROI
            pImaegDisp->Overlay_AddRectangles(result.vecrtValidElect, Color);
            break;
        case DEBUG_RES_VALID_BODY_ROI: // Res Valid Body ROI
            pImaegDisp->Overlay_AddRectangle(result.rtValidBody, Color);
            break;
        default:
            break;
    }

    pImaegDisp->Overlay_Show(TRUE);
}

void CDlgComponentAlign::SetCurrentComboDebugResult(long nSetCurSelVal)
{
    ImageLotView* pImaegDisp = m_pCompAlign->m_pChipVision->m_pVisionInspDlg->m_imageLotView;
    if (pImaegDisp == NULL)
        return;

    pImaegDisp->Overlay_RemoveAll();
    pImaegDisp->ROI_RemoveAll();

    COLORREF Color = RGB(0, 255, 0);
    m_cmbDebugResult.SetCurSel(nSetCurSelVal);
    SComponentAlignResult result = m_pCompAlign->m_result;
    switch (nSetCurSelVal)
    {
        case 0:
            pImaegDisp->Overlay_AddRectangle(result.rtSpecChip, Color); // Spec Chip ROI
            break;
        case 1: //
            pImaegDisp->Overlay_AddRectangle(result.rtResChip, Color); // Res Chip ROI
            break;
        case 2: //
            pImaegDisp->Overlay_AddRectangles(result.vecrtResElect, Color); // Res Electrode ROI
            break;
        case 3:
            pImaegDisp->Overlay_AddRectangle(result.rtResBody, Color); // Res Body ROI
            break;

        case 4: // Component Search ROI
            pImaegDisp->Overlay_AddRectangles(result.Debug_vecrtDetailPassiveSearchROI, Color);
            break;
        case 5: // Component Edge Points
            pImaegDisp->Overlay_AddPoints(result.Debug_vecfptPassiveEdge, Color);
            break;
        case 12: // Res Valid Component ROI
            pImaegDisp->Overlay_AddRectangle(result.rtValidComponent, Color);
            break;
        case 13: // Res Valid Electrode ROI
            pImaegDisp->Overlay_AddRectangles(result.vecrtValidElect, Color);
            break;
        case 14: // Res Valid Body ROI
            pImaegDisp->Overlay_AddRectangle(result.rtValidBody, Color);
            break;

        default:
            break;
    }

    pImaegDisp->Overlay_Show(TRUE);
}

BOOL CDlgComponentAlign::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_cmbDebugResult.ResetContent();

    m_cmbDebugResult.AddString(_T("Spec Chip ROI")); // 0
    m_cmbDebugResult.AddString(_T("Res Rough Align ROI")); // 1
    m_cmbDebugResult.AddString(_T("Res Rough Electrode ROI")); // 2
    m_cmbDebugResult.AddString(_T("Res Rough Body ROI")); // 3

    m_cmbDebugResult.AddString(_T("Component Search ROI")); // 4
    m_cmbDebugResult.AddString(_T("Component Edge Points")); // 5

    m_cmbDebugResult.AddString(_T("Res Valid Component ROI")); //
    m_cmbDebugResult.AddString(_T("Res Valid Electrode ROI")); //
    m_cmbDebugResult.AddString(_T("Res Valid Body ROI")); //

    m_cmbInsp.SetCurSel(0);

    // Grid Control 추가
    CRect rtPropertyGrid;
    GetClientRect(rtPropertyGrid);
    rtPropertyGrid.bottom = rtPropertyGrid.top + 420; // Bottom Btn Size 추가

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtPropertyGrid, this, CAST_UINT(IDC_STATIC));
    m_propertyGrid->ShowHelp(FALSE);

    for (long n = 0; n < MAX_HISTOGRMA_VIEWER; n++)
        m_pDlgHistoViewer[n]->ShowWindow(SW_HIDE);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgComponentAlign::SetPropertyGrid(sPassive_InfoDB* i_pPassiveInfoDB)
{
    m_propertyGrid->ResetContent();

    SComponentAlignSpec* pSpec = m_pCompAlign->GetAlignSpec(i_pPassiveInfoDB->strSpecName);
    if (pSpec == NULL)
        return;

    if (auto* category = m_propertyGrid->AddCategory(_T("Search Ratio")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Extension Search Ratio"),
                pSpec->m_fRoughAlign_Search_ROI_Extension_Ratio, _T("%.0f %%"),
                &pSpec->m_fRoughAlign_Search_ROI_Extension_Ratio)))
            item->SetID(ITEM_ID_ROUGH_ALIGN_SEARCH_ROI_EXTENSION_RATIO);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Spec ROI Offset X"), pSpec->m_fSpecROI_ShiftXPos, _T("%.0f um"), &pSpec->m_fSpecROI_ShiftXPos)))
            item->SetID(ITEM_ID_SPEC_ROI_SHIFT_X);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Spec ROI Offset Y"), pSpec->m_fSpecROI_ShiftYPos, _T("%.0f um"), &pSpec->m_fSpecROI_ShiftYPos)))
            item->SetID(ITEM_ID_SPEC_ROI_SHIFT_Y);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Valid Z Range"), pSpec->m_fValidZRange, _T("%.0f um"), &pSpec->m_fValidZRange)))
            item->SetID(ITEM_ID_VALID_Z_RANGE);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemBool(
                _T("ReCalc. Use SpecROI"), pSpec->m_bReCalc_UseSpecROI, &pSpec->m_bReCalc_UseSpecROI)))
            item->SetID(ITEM_ID_RECALC_USE_SPEC_ROI);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Visibility Lower Bound (0~255)"),
                pSpec->m_fVisibilityLowerBound, _T("%.2lf"), &pSpec->m_fVisibilityLowerBound)))
            item->SetID(ITEM_ID_VISIBILITY_LOWER_BOUND);

        category->Expand();
    }

    if (i_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        if (auto* category = m_propertyGrid->AddCategory(_T("Outline Align Parameters for Electrode")))
        {
            if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
                    _T("Search Width (Passive Width Ration %)"), pSpec->m_nOutline_align_Electrode_Search_Width_Ratio,
                    _T("%d"), pSpec->m_nOutline_align_Electrode_Search_Width_Ratio, 10, 150)))
                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_WIDTH_RATIO);

            if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search Length (um)"),
                    pSpec->m_nOutline_align_Electrode_Search_Length_um,
                    &pSpec->m_nOutline_align_Electrode_Search_Length_um)))
                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_LENGTH_UM);

            if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Edge Threshold"),
                    pSpec->m_fOutline_align_Electrode_EdgeThreshold, _T(""),
                    &pSpec->m_fOutline_align_Electrode_EdgeThreshold)))
                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_THRESHOLD);

            category->Expand();
        }
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Outline Align Parameters for Body")))
    {
        if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
                _T("Search Width (Passive Width Ration %)"), pSpec->m_nOutline_align_Body_Search_Width_Ratio, _T("%d"),
                pSpec->m_nOutline_align_Body_Search_Width_Ratio, 10, 150)))
            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_WIDTH_RATIO);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search Length (um)"),
                pSpec->m_nOutline_align_Body_Search_Length_um, &pSpec->m_nOutline_align_Body_Search_Length_um)))
            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_LENGTH_UM);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Edge Threshold"),
                pSpec->m_fOutline_align_Body_EdgeThreshold, _T(""), &pSpec->m_fOutline_align_Body_EdgeThreshold)))
            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_THRESHOLD);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Measurement")))
    {
        BOOL bUseMesurementBodyAndElectrode
            = i_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA ? TRUE : FALSE;

        CXTPPropertyGridItemBool* pItemBool = (CXTPPropertyGridItemBool*)category->AddChildItem(
            new CXTPPropertyGridItemBool(_T("Component"), pSpec->bMeasureComp, &pSpec->bMeasureComp));
        pItemBool->SetID(ITEM_ID_MEASURE_COMPONENT);
        pItemBool->SetCheckBoxStyle();

        pItemBool = (CXTPPropertyGridItemBool*)category->AddChildItem(
            new CXTPPropertyGridItemBool(_T("Body"), pSpec->bMeasureBody, &pSpec->bMeasureBody));
        pItemBool->SetID(ITEM_ID_MEASURE_BODY);
        pItemBool->SetCheckBoxStyle();
        pItemBool->SetReadOnly(bUseMesurementBodyAndElectrode);

        pItemBool = (CXTPPropertyGridItemBool*)category->AddChildItem(
            new CXTPPropertyGridItemBool(_T("Electrode"), pSpec->bMeasureElect, &pSpec->bMeasureElect));
        pItemBool->SetID(ITEM_ID_MEASURE_ELECTRODE);
        pItemBool->SetCheckBoxStyle();
        pItemBool->SetReadOnly(bUseMesurementBodyAndElectrode);

        category->Expand();
    }

    if (i_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        if (auto* category = m_propertyGrid->AddCategory(_T("Electrode Cal. Param")))
        {
            if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
                    _T("Electrode Cal. Area Width Size (Ration %)"), pSpec->m_nCalc_Electrode_Area_Width_Ratio,
                    _T("%d"), pSpec->m_nCalc_Electrode_Area_Width_Ratio, 10, 100)))
                item->SetID(ITEM_ID_CALC_ELECTRODE_VAILD_ROI_WIDTH_RATIO);

            if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
                    _T("Electrode Cal. Area Length Size (Ration %)"), pSpec->m_nCalc_Electrode_Area_Length_Ratio,
                    _T("%d"), pSpec->m_nCalc_Electrode_Area_Length_Ratio, 10, 100)))
                item->SetID(ITEM_ID_CALC_ELECTRODE_VAILD_ROI_LENGTH_RATIO);

            category->Expand();
        }

        if (auto* category = m_propertyGrid->AddCategory(_T("Body Cal. Param")))
        {
            if (auto* item = category->AddChildItem(
                    new XTPPropertyGridItemCustomNumber<long>(_T("Body Cal. Area Width Size (Ration %)"),
                        pSpec->m_nCalc_Body_Area_Width_Ratio, _T("%d"), pSpec->m_nCalc_Body_Area_Width_Ratio, 10, 100)))
                item->SetID(ITEM_ID_CALC_BODY_VAILD_ROI_WIDTH_RATIO);

            if (auto* item = category->AddChildItem(new XTPPropertyGridItemCustomNumber<long>(
                    _T("Body Cal. Area Length Size (Ration %)"), pSpec->m_nCalc_Body_Area_Length_Ratio, _T("%d"),
                    pSpec->m_nCalc_Body_Area_Length_Ratio, 10, 100)))
                item->SetID(ITEM_ID_CALC_BODY_VAILD_ROI_LENGTH_RATIO);

            if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Body Cal. Area Offset X (um)"),
                    pSpec->m_nCalc_Body_Area_GapX_um, &pSpec->m_nCalc_Body_Area_GapX_um)))
                item->SetID(ITEM_ID_CALC_BODY_VAILD_ROI_DISTANCE_X_UM);

            if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Body Cal. Area Offset Y (um)"),
                    pSpec->m_nCalc_Body_Area_GapY_um, &pSpec->m_nCalc_Body_Area_GapY_um)))
                item->SetID(ITEM_ID_CALC_BODY_VAILD_ROI_DISTANCE_Y_UM);

            category->Expand();
        }
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Substrate Cal. Param")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Substrate Cal. Area Size (um)"),
                pSpec->m_nCalc_Substrate_Area_Size_um, &pSpec->m_nCalc_Substrate_Area_Size_um)))
            item->SetID(ITEM_ID_CALC_SUBSTRATE_ROI_SIZE_UM);

        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Substrate Cal. Area Offset X for Body (um)"),
                pSpec->m_nCalc_Substrate_Area_GapX_Body_um, &pSpec->m_nCalc_Substrate_Area_GapX_Body_um)))
            item->SetID(ITEM_ID_CALC_SUBSTRATE_ROI_X_OFFSET_FOR_BODY_UM);

        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Substrate Cal. Area Offset Y for Body (um)"),
                pSpec->m_nCalc_Substrate_Area_GapY_Body_um, &pSpec->m_nCalc_Substrate_Area_GapY_Body_um)))
            item->SetID(ITEM_ID_CALC_SUBSTRATE_ROI_Y_OFFSET_FOR_BODY_UM);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Substrate Cal. Area Shift X (um)"),
                pSpec->m_nCalc_Substrate_Area_GapX_um, &pSpec->m_nCalc_Substrate_Area_GapX_um)))
            item->SetID(ITEM_ID_CALC_SUBSTRATE_ROI_SHIFT_X_UM);

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(_T("Substrate Cal. Area Shift Y (um)"),
                pSpec->m_nCalc_Substrate_Area_GapY_um, &pSpec->m_nCalc_Substrate_Area_GapY_um)))
            item->SetID(ITEM_ID_CALC_SUBSTRATE_ROI_SHIFT_Y_UM);

        category->Expand();
    }

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

LRESULT CDlgComponentAlign::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    OnBnClickedBtnTeach();
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    bool bShowOverlay(true);
    if (auto* nValue = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_CALC_SUBSTRATE_ROI_SIZE_UM:
                OnBnClickedBtnViewResult();
                bShowOverlay = false;
                break;

            case ITEM_ID_CALC_BODY_VAILD_ROI_DISTANCE_X_UM:
            case ITEM_ID_CALC_BODY_VAILD_ROI_DISTANCE_Y_UM:
                SetCurrentComboInspection(2);
                bShowOverlay = false;
                break;
        }
    }

    if (auto* nValue = dynamic_cast<XTPPropertyGridItemCustomNumber<long>*>(item))
    {
        switch (item->GetID())
        {
            case ITEM_ID_CALC_ELECTRODE_VAILD_ROI_WIDTH_RATIO:
            case ITEM_ID_CALC_ELECTRODE_VAILD_ROI_LENGTH_RATIO:
                SetCurrentComboInspection(1);
                bShowOverlay = false;
                break;
            case ITEM_ID_CALC_BODY_VAILD_ROI_WIDTH_RATIO:
            case ITEM_ID_CALC_BODY_VAILD_ROI_LENGTH_RATIO:
                SetCurrentComboInspection(2);
                bShowOverlay = false;
                break;
        }
    }

    if (bShowOverlay)
        ShowOverlay(item->GetID());

    m_propertyGrid->Refresh();

    return 0;
}

void CDlgComponentAlign::ShowOverlay(long i_nCuridx)
{
    ImageLotView* pImageDisp = m_pCompAlign->m_pChipVision->m_pVisionInspDlg->m_imageLotView;
    if (pImageDisp == NULL)
        return;

    pImageDisp->Overlay_RemoveAll();
    pImageDisp->ROI_RemoveAll();

    auto* pAlignResult = &m_pCompAlign->m_result;
    switch (i_nCuridx)
    {
        case ITEM_ID_ROUGH_ALIGN_SEARCH_ROI_EXTENSION_RATIO:
            pImageDisp->Overlay_AddRectangle(pAlignResult->Debug_rtRoughAlign_SearchROI, RGB(0, 255, 0));
            break;
        case ITEM_ID_SPEC_ROI_SHIFT_X:
        case ITEM_ID_SPEC_ROI_SHIFT_Y:
            pImageDisp->Overlay_AddRectangle(pAlignResult->Debug_frtShiftSpecROI, RGB(0, 255, 0));
            break;
        case ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_WIDTH_RATIO:
        case ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_LENGTH_UM:
        case ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_WIDTH_RATIO:
        case ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_LENGTH_UM:
            pImageDisp->Overlay_AddRectangles(pAlignResult->Debug_vecrtDetailPassiveSearchROI, RGB(0, 255, 0));
            break;
        case ITEM_ID_CALC_SUBSTRATE_ROI_SIZE_UM:
        case ITEM_ID_CALC_SUBSTRATE_ROI_X_OFFSET_FOR_BODY_UM:
        case ITEM_ID_CALC_SUBSTRATE_ROI_Y_OFFSET_FOR_BODY_UM:
        case ITEM_ID_CALC_SUBSTRATE_ROI_SHIFT_X_UM:
        case ITEM_ID_CALC_SUBSTRATE_ROI_SHIFT_Y_UM:
            pImageDisp->Overlay_AddRectangles(pAlignResult->vecrtSubstrateArea, RGB(0, 255, 0));
            break;
    }

    pImageDisp->Overlay_Show(SW_SHOW);
}

void CDlgComponentAlign::OnShowWindow(BOOL bShow, UINT nStatus)
{
    if (m_pCompAlign->m_pChipVision->m_pVisionInspDlg == nullptr
        || m_pCompAlign->m_pChipVision->m_pVisionInspDlg->m_pCurPassiveInfoDB == nullptr)
        return;

    CDialog::OnShowWindow(bShow, nStatus);

    m_pCompAlign->m_result.Clear();
    if (bShow)
    {
        auto* VisionInspDlg = m_pCompAlign->m_pChipVision->m_pVisionInspDlg;
        auto* SelectPassiveInfoDB = VisionInspDlg->m_pCurPassiveInfoDB;

        SetPropertyGrid(SelectPassiveInfoDB);

        OnBnClickedBtnTeach();

        ImageLotView* m_imageLotView = m_pCompAlign->m_pChipVision->m_pVisionInspDlg->m_imageLotView;
        if (m_imageLotView == NULL)
            return;
        m_imageLotView->SetImage(m_debugResult.m_imgHmap, _T("Hmap"));
    }
}

void CDlgComponentAlign::OnCbnSelchangeComboInspection()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long nInsp = m_cmbInsp.GetCurSel();
    ResultText(nInsp, TRUE);
}

void CDlgComponentAlign::SetCurrentComboInspection(long nSetCurIdx)
{
    m_cmbInsp.SetCurSel(nSetCurIdx);
    ResultText(nSetCurIdx, TRUE);
}

void CDlgComponentAlign::ResultText(long nInsp, BOOL bOverlay)
{
    if (nInsp < 0)
        return;

    ImageLotView* pImageDisp = m_pCompAlign->m_pChipVision->m_pVisionInspDlg->m_imageLotView;
    if (pImageDisp == NULL)
        return;
    pImageDisp->Overlay_RemoveAll();
    pImageDisp->ROI_RemoveAll();

    CString strText = _T("");
    CString strTemp = _T("");

    long nFontSize = 2;
    COLORREF Color = RGB(0, 255, 0);
    COLORREF ColorObj = RGB(0, 255, 255);

    auto AlignResult = m_pCompAlign->m_result;

    if (nInsp == 0) // Component
    {
        // Disp
        if (bOverlay)
        {
            // Disp Component
            Ipvm::Point32s2 ptCompCenter = AlignResult.rtValidComponent.CenterPoint();
            pImageDisp->Overlay_AddRectangle(AlignResult.rtValidComponent, ColorObj);

            // Disp Land
            for (long nSubstrateAreaidx = 0; nSubstrateAreaidx < (long)AlignResult.vecrtSubstrateArea.size();
                nSubstrateAreaidx++)
            {
                Ipvm::Point32s2 ptLand = AlignResult.vecrtSubstrateArea[nSubstrateAreaidx].CenterPoint();
                pImageDisp->Overlay_AddRectangle(AlignResult.vecrtSubstrateArea[nSubstrateAreaidx], Color);

                strText.Format(_T("%d"), nSubstrateAreaidx);
                pImageDisp->Overlay_AddText(ptLand, strText, Color, nFontSize);
            }
        }

        // Text
        {
            strText = _T("");

            strTemp.Format(_T("Component Height = %s um\r\n"), (LPCTSTR)GetValueString(AlignResult.fCompHeight));
            strText += strTemp;
            strText += _T("\r\n");

            strTemp.Format(
                _T("Component Data = %s\r\n"), (LPCTSTR)GetValueString(AlignResult.Debug_fAlignAreaPassiveHeight));
            strText += strTemp;
            strText += _T("\r\n");

            float fHPosLand = 0.f;
            long nLandNum = (long)AlignResult.Debug_vecfPassive_Calc_SubstrateArea_Height.size();
            for (long nLand = 0; nLand < nLandNum; nLand++)
            {
                fHPosLand += AlignResult.Debug_vecfPassive_Calc_SubstrateArea_Height[nLand];
                strTemp.Format(_T("Substrate %d Data = %s(%s%%)\r\n"), nLand,
                    (LPCTSTR)GetValueString(AlignResult.Debug_vecfPassive_Calc_SubstrateArea_Height[nLand]),
                    (LPCTSTR)GetValueString(AlignResult.Debug_vecfPassive_Calc_SubstrateArea_Height_Ratio[nLand]));
                strText += strTemp;
            }
            strText += _T("\r\n");

            strTemp.Format(
                _T("Component Height Ratio = %s%%\r\n"), (LPCTSTR)GetValueString(AlignResult.fCompHeightRatio));
            strText += strTemp;

            strTemp.Format(
                _T("Component Valid Ratio = %s%%\r\n"), (LPCTSTR)GetValueString(AlignResult.fValidCompRatio));
            strText += strTemp;
        }
    }
    else if (nInsp == 1) // Electrode
    {
        // Disp
        if (bOverlay)
        {
            // Disp Electrode
            for (long nElect = 0; nElect < (long)AlignResult.vecrtValidElect.size(); nElect++)
            {
                pImageDisp->Overlay_AddRectangle(AlignResult.vecrtValidElect[nElect], ColorObj);

                strText.Format(_T("%d"), nElect);
                pImageDisp->Overlay_AddText(
                    AlignResult.vecrtValidElect[nElect].CenterPoint(), strText, ColorObj, nFontSize);
            }

            // Disp Land
            for (long nROICount = 0; nROICount < (long)AlignResult.vecrtSubstrateArea.size(); nROICount++)
            {
                pImageDisp->Overlay_AddRectangle(AlignResult.vecrtSubstrateArea[nROICount], Color);

                strText.Format(_T("%d"), nROICount);
                pImageDisp->Overlay_AddText(
                    AlignResult.vecrtSubstrateArea[nROICount].CenterPoint(), strText, Color, nFontSize);
            }
        }

        // Text
        {
            strText = _T("");

            for (long nElect = 0; nElect < AlignResult.vecfElectrodeHeight.size(); nElect++)
            {
                strTemp.Format(_T("Electrode %d Height = %s um\r\n"), nElect,
                    (LPCTSTR)GetValueString(AlignResult.vecfElectrodeHeight[nElect]));
                strText += strTemp;
            }
            strText += _T("\r\n");

            strTemp.Format(_T("Electrode_Tilt = %s um\r\n"), (LPCTSTR)GetValueString(AlignResult.fElectTiltHeight));
            strText += strTemp;
            strText += _T("\r\n");

            for (long nElect = 0; nElect < AlignResult.Debug_vecfElectrode_Area_Height.size(); nElect++)
            {
                strTemp.Format(_T("Electrode %d Data = %s( "), nElect,
                    (LPCTSTR)GetValueString(AlignResult.Debug_vecfElectrode_Area_Height[nElect]));
                strText += strTemp;

                strText += _T(")\r\n");
            }
            strText += _T("\r\n");

            strTemp.Format(_T("Substrate Average Data = %s\r\n"),
                (LPCTSTR)GetValueString(AlignResult.Debug_fSubstrate_Avg_Height_Electrode));
            strText += strTemp;

            for (long nData = 0; nData < AlignResult.Debug_vecfElectrode_Calc_SubstrateArea_Height.size(); nData++)
            {
                strTemp.Format(_T("Substrate %d Data = %s\r\n"), nData,
                    (LPCTSTR)GetValueString(AlignResult.Debug_vecfElectrode_Calc_SubstrateArea_Height[nData]));
                strText += strTemp;
            }
        }
    }
    else if (nInsp == 2) // Body
    {
        // Disp
        if (bOverlay)
        {
            // Disp Split Body
            for (long nBody = 0; nBody < (long)AlignResult.vecrtSplitBody.size(); nBody++)
            {
                Ipvm::Point32s2 ptSplitBodyCenter = AlignResult.vecrtSplitBody[nBody].CenterPoint();
                pImageDisp->Overlay_AddRectangle(AlignResult.vecrtSplitBody[nBody], ColorObj);

                strText.Format(_T("%d"), nBody);
                pImageDisp->Overlay_AddText(ptSplitBodyCenter, strText, ColorObj, nFontSize);
            }

            // Disp Land

            for (long nSubstrateAreaidx = 0; nSubstrateAreaidx < (long)AlignResult.vecrtSubstrateArea.size();
                nSubstrateAreaidx++)
            {
                Ipvm::Point32s2 ptLand = AlignResult.vecrtSubstrateArea[nSubstrateAreaidx].CenterPoint();
                pImageDisp->Overlay_AddRectangle(AlignResult.vecrtSubstrateArea[nSubstrateAreaidx], Color);

                strText.Format(_T("%d"), nSubstrateAreaidx);
                pImageDisp->Overlay_AddText(ptLand, strText, Color, nFontSize);
            }
        }

        // Text
        {
            strText = _T("");

            strTemp.Format(_T("Body_Height = %s um\r\n"), (LPCTSTR)GetValueString(AlignResult.fBodyHeight));
            strText += strTemp;
            strText += _T("\r\n");

            strTemp.Format(_T("Body_Tilt Height = %s um\r\n"), (LPCTSTR)GetValueString(AlignResult.fBodyTiltHeight));
            strText += strTemp;
            strText += _T("\r\n");

            for (long nPart = 0; nPart < (long)AlignResult.vecfSplitBody_Height.size(); nPart++)
            {
                strTemp.Format(_T("Body Part %d Data = %s(%s%%)\r\n"), nPart,
                    (LPCTSTR)GetValueString(AlignResult.vecfSplitBody_Height[nPart]),
                    (LPCTSTR)GetValueString(AlignResult.Debug_vecfValidSplitBodyRatio[nPart]));
                strText += strTemp;
            }
            strText += _T("\r\n");

            for (long nDataidx = 0; nDataidx < AlignResult.Debug_vecfBody_Calc_SubstrateArea_Height.size(); nDataidx++)
            {
                strTemp.Format(_T("Substrate %d Data = %s(%s%%)\r\n"), nDataidx,
                    (LPCTSTR)GetValueString(AlignResult.Debug_vecfBody_Calc_SubstrateArea_Height[nDataidx]),
                    (LPCTSTR)GetValueString(AlignResult.Debug_vecfBody_Calc_SubstrateArea_Height_Ratio[nDataidx]));
                strText += strTemp;
            }
        }
    }

    pImageDisp->Overlay_Show(TRUE);

    GetDlgItem(IDC_EDIT_RESULT_TEXT)->SetWindowText(strText);

    strText.Empty();
    strTemp.Empty();
}

void CDlgComponentAlign::OnBnClickedBtnViewHistogram()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long nInsp = m_cmbInsp.GetCurSel();
    if (nInsp < 0)
        return;

    for (long n = 0; n < MAX_HISTOGRMA_VIEWER; n++)
        m_pDlgHistoViewer[n]->ShowWindow(SW_HIDE);

    //long nIDHisto;
    Ipvm::Image8u byZMap;
    if (!m_pCompAlign->m_pChipVision->getReusableMemory().GetInspByteImage(byZMap))
        return;

    float scaleZ = m_pCompAlign->m_pChipVision->getScale().pixelToUmZ();

    auto AlignResult = m_pCompAlign->m_result;

    if (nInsp == 0)
    {
        CString str;
        str.Format(_T("Component"));
        Ipvm::Rect32s rtComponent = AlignResult.rtValidComponent;
        std::vector<long> vecHisto(101);
        m_pCompAlign->ConvertFloattoByteData(m_debugResult.m_zmap, 1.f / scaleZ, rtComponent, byZMap);
        MakeHistogram(byZMap.GetMem(), &vecHisto[0], byZMap.GetWidthBytes(), rtComponent);
        m_pDlgHistoViewer[0]->ShowWindow(SW_SHOW);
        m_pDlgHistoViewer[0]->DrawGraph(str, vecHisto);
        str.Empty();
    }
    else if (nInsp == 1)
    {
        for (long nElect = 0; nElect < (long)AlignResult.vecrtValidElect.size(); nElect++)
        {
            CString str;
            str.Format(_T("Electrode %d"), nElect);
            Ipvm::Rect32s rtElect = AlignResult.vecrtValidElect[nElect];
            std::vector<long> vecHisto(101);
            m_pCompAlign->ConvertFloattoByteData(m_debugResult.m_zmap, 1.f / scaleZ, rtElect, byZMap);
            MakeHistogram(byZMap.GetMem(), &vecHisto[0], byZMap.GetWidthBytes(), rtElect);
            m_pDlgHistoViewer[nElect]->ShowWindow(SW_SHOW);
            m_pDlgHistoViewer[nElect]->DrawGraph(str, vecHisto);
            str.Empty();
        }
    }
    else if (nInsp == 2)
    {
        CString str;
        std::vector<long> vecHisto(101);

        for (long nSplit = 0; nSplit < (long)AlignResult.vecrtSplitBody.size(); nSplit++)
        {
            Ipvm::Rect32s rtSplit = AlignResult.vecrtSplitBody[nSplit];
            m_pCompAlign->ConvertFloattoByteData(m_debugResult.m_zmap, 1.f / scaleZ, rtSplit, byZMap);
            MakeHistogram(byZMap.GetMem(), &vecHisto[0], byZMap.GetWidthBytes(), rtSplit);
            str.Format(_T("Split %d"), nSplit);
            m_pDlgHistoViewer[1 + nSplit]->ShowWindow(SW_SHOW);
            m_pDlgHistoViewer[1 + nSplit]->DrawGraph(str, vecHisto);
        }

        str.Format(_T("Body"));
        Ipvm::Rect32s rtBody = AlignResult.rtValidBody;
        m_pCompAlign->ConvertFloattoByteData(m_debugResult.m_zmap, 1.f / scaleZ, rtBody, byZMap);
        MakeHistogram(byZMap.GetMem(), &vecHisto[0], byZMap.GetWidthBytes(), rtBody);
        m_pDlgHistoViewer[0]->ShowWindow(SW_SHOW);
        m_pDlgHistoViewer[0]->DrawGraph(str, vecHisto);
        str.Empty();
    }
}

void CDlgComponentAlign::MakeHistogram(BYTE* pbySrc, long* pnHistogram, long nSizeX, const Ipvm::Rect32s& rtROI)
{
    memset(pnHistogram, 0, sizeof(long) * 101);

    Ipvm::Rect32s rtNewROI(rtROI);

    long nOffsetY = nSizeX * rtNewROI.m_top + rtNewROI.m_left;

    long nBeginY = rtNewROI.m_top;
    long nEndY = rtNewROI.m_bottom;
    long nBeginX = rtNewROI.m_left;
    long nEndX = rtNewROI.m_right;

    for (long y = nBeginY; y < nEndY; y++, nOffsetY += nSizeX)
    {
        long nOffset = nOffsetY;

        for (long x = nBeginX; x < nEndX; x++, nOffset++)
        {
            pnHistogram[pbySrc[nOffset]]++;
        }
    }
}

void CDlgComponentAlign::OnBnClickedBtnViewResult()
{
    long nInsp = m_cmbInsp.GetCurSel();
    ResultText(nInsp, TRUE);
}