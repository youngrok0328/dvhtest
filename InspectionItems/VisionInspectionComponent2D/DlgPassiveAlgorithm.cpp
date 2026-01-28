//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgPassiveAlgorithm.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionComponent2D.h"
#include "FunctionResult.h"
#include "PassiveAlign.h"
#include "Result.h"
#include "VisionInspectionComponent2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/IllumInfo2D.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../MemoryModules/VisionReusableMemory/VisionReusableMemory.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionUnitAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID_PASSIVEALIGN
{
    ITEM_ID_ROUGH_ALIGN_IMAGE,
    ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_X,
    ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_Y,
    ITEM_ID_ROUGH_ALIGN_ELECTRODE_COLOR,
    ITEM_ID_ROUGH_ALIGN_ONLY_USE,

    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_IMAGE,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_WIDTH_RATIO,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_LENGTH_UM,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_ELECTRODE_COLOR,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_SEARCH_DIRECTION,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_THRESHOLD,
    ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_DETECT_MODE,

    ITEM_ID_OUTLINE_ALIGN_BODY_IMAGE,
    ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_WIDTH_RATIO,
    ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_LENGTH_UM,
    ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_COUNT,
    ITEM_ID_OUTLINE_ALIGN_BODY_ELECTRODE_COLOR,
    ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_SEARCH_DIRECTION,
    ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_THRESHOLD,
    ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_DETECT_MODE,

    ITEM_ID_2ND_INSPECTION_USE,
    ITEM_ID_2ND_INSPECTION_MATCH_CODE,

    ITEM_ID_PAD_USE_ALIGN,
    ITEM_ID_PAD_ALIGN_IMAGE,
    ITEM_ID_PAD_ALIGN_EDGE_SEARCH_ROI_RATIO
};

IMPLEMENT_DYNAMIC(CDlgPassiveAlgorithm, CDialog)

CDlgPassiveAlgorithm::CDlgPassiveAlgorithm(VisionInspectionComponent2D* pChipAlign, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgPassiveAlgorithm::IDD, pParent)
    , m_pItem(pChipAlign->m_pPassiveAlign)
    , m_pChipVision(pChipAlign)
{
}

CDlgPassiveAlgorithm::~CDlgPassiveAlgorithm()
{
}

void CDlgPassiveAlgorithm::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_DEBUGVIEW, m_strName);
    DDX_Control(pDX, IDC_CMB_DEBUG, m_cmbDebug);
    DDX_Control(pDX, IDC_GRID_PASSIVEINFO, m_gridPassive); //나중에 삭제
}

BEGIN_MESSAGE_MAP(CDlgPassiveAlgorithm, CDialog)
ON_WM_SHOWWINDOW()
ON_BN_CLICKED(IDC_BUTTON_TEACH, &CDlgPassiveAlgorithm::OnBnClickedButtonTeach)
ON_CBN_SELCHANGE(IDC_CMB_DEBUG, &CDlgPassiveAlgorithm::OnCbnSelchangeCmbDebug)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
END_MESSAGE_MAP()

// CDlgPassiveAlgorithm 메시지 처리기입니다.

BOOL CDlgPassiveAlgorithm::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Grid Control 추가
    CRect rtPropertyGrid;
    GetClientRect(rtPropertyGrid);

    CRect rtTeachButton;
    GetDlgItem(IDC_BUTTON_TEACH)->GetWindowRect(rtTeachButton);
    ScreenToClient(rtTeachButton);

    rtPropertyGrid.bottom = rtTeachButton.top - 5;

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtPropertyGrid, this, CAST_UINT(IDC_STATIC));
    m_propertyGrid->ShowHelp(FALSE);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgPassiveAlgorithm::SetPropertyGrid(sPassive_InfoDB* i_pPassiveInfoDB)
{
    m_propertyGrid->ResetContent();

    PassiveAlignSpec::CapAlignSpec* pSpec = m_pItem->GetCapAlignSpec(i_pPassiveInfoDB->strSpecName);
    if (pSpec == NULL)
        return;

    if (auto* category_Rough_Align_Param = m_propertyGrid->AddCategory(_T("Rough Align Parameters")))
    {
        pSpec->m_Rough_align_ImageFrameIndex.makePropertyGridItem(
            category_Rough_Align_Param, _T("Rough Align Image"), ITEM_ID_ROUGH_ALIGN_IMAGE);

        if (auto* item
            = category_Rough_Align_Param->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search Offset X (um)"),
                pSpec->m_nRougAlign_SearchOffsetX_um, &pSpec->m_nRougAlign_SearchOffsetX_um)))
            item->SetID(ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_X);

        if (auto* item
            = category_Rough_Align_Param->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search Offset Y (um)"),
                pSpec->m_nRougAlign_SearchOffsetY_um, &pSpec->m_nRougAlign_SearchOffsetY_um)))
            item->SetID(ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_Y);

        if (auto* item = category_Rough_Align_Param->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Electorde Color (contrast to substrate)"),
                    pSpec->m_bRoughAlign_ElectrodeColor, &pSpec->m_bRoughAlign_ElectrodeColor)))
        {
            item->GetConstraints()->AddConstraint(_T("Dark"), DARK);
            item->GetConstraints()->AddConstraint(_T("Bright"), BRIGHT);

            item->SetID(ITEM_ID_ROUGH_ALIGN_ELECTRODE_COLOR);
        }

        if (auto* item = category_Rough_Align_Param->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Only Use Rough Align"), pSpec->m_bRoughAlign_OnlyUse, &pSpec->m_bRoughAlign_OnlyUse)))
        {
            item->GetConstraints()->AddConstraint(_T("Not Use"), 0);
            item->GetConstraints()->AddConstraint(_T("Use"), 1);

            item->SetID(ITEM_ID_ROUGH_ALIGN_ONLY_USE);
        }

        category_Rough_Align_Param->Expand();
    }

    if (m_pChipVision->m_pVisionInspDlg->m_pCurPassiveInfoDB->ePassiveType
        != enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        if (auto* category_Outline_Align_Electrode
            = m_propertyGrid->AddCategory(_T("Outline Align Parameters for Electrode")))
        {
            pSpec->m_Outline_align_Electrode_ImageFrameIndex.makePropertyGridItem(category_Outline_Align_Electrode,
                _T("Outline Align(Electrode) Image"), ITEM_ID_OUTLINE_ALIGN_ELECTRODE_IMAGE);

            if (auto* item = category_Outline_Align_Electrode->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Search Width (Passive Width Ration %)"), pSpec->m_nOutline_align_Electrode_Search_Width_Ratio,
                    &pSpec->m_nOutline_align_Electrode_Search_Width_Ratio)))
                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_WIDTH_RATIO);

            if (auto* item = category_Outline_Align_Electrode->AddChildItem(new CXTPPropertyGridItemNumber(
                    _T("Search Length (um)"), pSpec->m_nOutline_align_Electrode_Search_Length_um,
                    &pSpec->m_nOutline_align_Electrode_Search_Length_um)))
                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_LENGTH_UM);

            if (auto* item = category_Outline_Align_Electrode->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Electorde Color (contrast to substrate)"), pSpec->m_nOutline_align_Electrode_ElectrodeColor,
                    &pSpec->m_nOutline_align_Electrode_ElectrodeColor)))
            {
                item->GetConstraints()->AddConstraint(_T("Should be Dark"), DARK);
                item->GetConstraints()->AddConstraint(_T("Should be Bright"), BRIGHT);

                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_ELECTRODE_COLOR);
                item->SetReadOnly(TRUE);
            }

            if (auto* item = category_Outline_Align_Electrode->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Edge Search Direction"), pSpec->m_nOutline_align_Electrode_EdgeSearch_Direction,
                    &pSpec->m_nOutline_align_Electrode_EdgeSearch_Direction)))
            {
                item->GetConstraints()->AddConstraint(_T("In -> Out"), PI_ED_DIR_OUTER);
                item->GetConstraints()->AddConstraint(_T("Out -> In"), PI_ED_DIR_INNER);

                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_SEARCH_DIRECTION);
            }

            if (auto* item = category_Outline_Align_Electrode->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Edge Threshold"), pSpec->m_fOutline_align_Electrode_EdgeThreshold, _T(""),
                    &pSpec->m_fOutline_align_Electrode_EdgeThreshold)))
                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_THRESHOLD);

            if (auto* item = category_Outline_Align_Electrode->AddChildItem(new CXTPPropertyGridItemEnum(
                    _T("Electrode Edge Detect Mode"), pSpec->m_nOutline_align_Electrode_Edge_Detect_Mode,
                    &pSpec->m_nOutline_align_Electrode_Edge_Detect_Mode)))
            {
                item->GetConstraints()->AddConstraint(_T("Use Best Edge"), PI_ED_MAX_EDGE);
                item->GetConstraints()->AddConstraint(_T("Use First Edge"), PI_ED_FIRST_EDGE);

                item->SetID(ITEM_ID_OUTLINE_ALIGN_ELECTRODE_EDGE_DETECT_MODE);
            }

            category_Outline_Align_Electrode->Expand();
        }
    }

    if (auto* category_Outline_Align_Body = m_propertyGrid->AddCategory(_T("Outline Align Parameters for Body")))
    {
        pSpec->m_Outline_align_Body_ImageFrameIndex.makePropertyGridItem(
            category_Outline_Align_Body, _T("Outline Align(Body) Image"), ITEM_ID_OUTLINE_ALIGN_BODY_IMAGE);

        if (auto* item = category_Outline_Align_Body->AddChildItem(
                new CXTPPropertyGridItemNumber(_T("Search Width (Passive Width Ration %)"),
                    pSpec->m_nOutline_align_Body_Search_Width_Ratio, &pSpec->m_nOutline_align_Body_Search_Width_Ratio)))
            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_WIDTH_RATIO);

        if (auto* item
            = category_Outline_Align_Body->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search Length (um)"),
                pSpec->m_nOutline_align_Body_Search_Length_um, &pSpec->m_nOutline_align_Body_Search_Length_um)))
            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_LENGTH_UM);

        if (auto* item = category_Outline_Align_Body->AddChildItem(new CXTPPropertyGridItemNumber(_T("Search Count"),
                pSpec->m_nOutline_align_Body_Search_Count_Gap, &pSpec->m_nOutline_align_Body_Search_Count_Gap)))
            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_COUNT);

        if (auto* item = category_Outline_Align_Body->AddChildItem(
                new CXTPPropertyGridItemEnum(_T("Body Color (contrast to substrate)"),
                    pSpec->m_nOutline_align_Body_ElectrodeColor, &pSpec->m_nOutline_align_Body_ElectrodeColor)))
        {
            item->GetConstraints()->AddConstraint(_T("Dark"), DARK);
            item->GetConstraints()->AddConstraint(_T("Bright"), BRIGHT);
            item->GetConstraints()->AddConstraint(_T("Both"), BOTH);

            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_ELECTRODE_COLOR);
        }

        if (auto* item
            = category_Outline_Align_Body->AddChildItem(new CXTPPropertyGridItemEnum(_T("Edge Search Direction"),
                pSpec->m_nOutline_align_Body_EdgeSearch_Direction, &pSpec->m_nOutline_align_Body_EdgeSearch_Direction)))
        {
            item->GetConstraints()->AddConstraint(_T("In -> Out"), PI_ED_DIR_OUTER);
            item->GetConstraints()->AddConstraint(_T("Out -> In"), PI_ED_DIR_INNER);

            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_SEARCH_DIRECTION);
        }

        if (auto* item = category_Outline_Align_Body->AddChildItem(new CXTPPropertyGridItemDouble(_T("Edge Threshold"),
                pSpec->m_fOutline_align_Body_EdgeThreshold, _T(""), &pSpec->m_fOutline_align_Body_EdgeThreshold)))
            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_THRESHOLD);

        if (auto* item
            = category_Outline_Align_Body->AddChildItem(new CXTPPropertyGridItemEnum(_T("Body Edge Detect Mode"),
                pSpec->m_nOutline_align_Body_Edge_Detect_Mode, &pSpec->m_nOutline_align_Body_Edge_Detect_Mode)))
        {
            item->GetConstraints()->AddConstraint(_T("Use Best Edge"), PI_ED_MAX_EDGE);
            item->GetConstraints()->AddConstraint(_T("Use First Edge"), PI_ED_FIRST_EDGE);

            item->SetID(ITEM_ID_OUTLINE_ALIGN_BODY_EDGE_DETECT_MODE);
        }

        category_Outline_Align_Body->Expand();
    }

    if (m_pChipVision->m_visionUnit.m_systemConfig.m_bUseAiInspection)
    {
        if (auto* category_2ndInspection = m_propertyGrid->AddCategory(_T("2nd Inspection Parameter (Deep Learning)")))
        {
            if (auto* Item = category_2ndInspection->AddChildItem(new CXTPPropertyGridItemBool(
                    _T("Use Inspection"), pSpec->m_bUse2ndInspection, &pSpec->m_bUse2ndInspection)))
            {
                Item->SetID(ITEM_ID_2ND_INSPECTION_USE);
            }

            if (auto* Item = category_2ndInspection->AddChildItem(new CXTPPropertyGridItem(
                    _T("DL Model Match Code"), pSpec->m_str2ndInspCode, &pSpec->m_str2ndInspCode)))
            {
                Item->SetID(ITEM_ID_2ND_INSPECTION_MATCH_CODE);

                //Read2ndInspCodeInfo(_T("C:\\intekplus\\iDL\\Vision_x64\\System\\DLVisionSystem.ini"));
            }
            category_2ndInspection->Expand();
        }
    }

    /*if (auto *category = m_propertyGrid->AddCategory(_T("Pad Align")))
	{
		if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Pad Use Align"), pSpec->m_padUseAlign)))
		{
			item->GetConstraints()->AddConstraint(_T("X"), 0);
			item->GetConstraints()->AddConstraint(_T("O"), 1);

			item->SetID(ITEM_ID_PAD_USE_ALIGN);
		}

		pSpec->m_padAlignImageFrameIndex.makePropertyGridItem(category, _T("Pad Align Image"), ITEM_ID_PAD_ALIGN_IMAGE);

		CXTPPropertyGridItem* pItemWidthLength = category->AddChildItem(new CCustomItemWidthLength(_T("Edge SearchROI Ratio (Width, Length)"), _T("Width (%)"), _T("Length (%)"), _T("%"), (double)pSpec->m_fPadSearchROI_Width_Ratio, (double)pSpec->m_fPadSearchROI_Width_Ratio));
		pItemWidthLength->SetID(ITEM_ID_PAD_ALIGN_EDGE_SEARCH_ROI_RATIO);
		pItemWidthLength->Expand();
	}*/

    m_propertyGrid->SetViewDivider(0.60);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

LRESULT CDlgPassiveAlgorithm::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    sPassive_InfoDB* pCurSelectPassiveInfoDB = m_pItem->m_pChipVision->m_pVisionInspDlg->m_pCurPassiveInfoDB;
    if (pCurSelectPassiveInfoDB == nullptr)
        return 0;
    PassiveAlignSpec::CapAlignSpec* pSpec
        = m_pItem->GetCapAlignSpec(pCurSelectPassiveInfoDB->strSpecName); //모르겠다 주소값 펑이다...

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;
    if (auto* NumberVale = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
    {
        bool bShowEdgeSearchROI(false);
        switch (item->GetID())
        {
            case ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_COUNT:
            {
                long nMinimumValue = max(5, pSpec->m_nOutline_align_Body_Search_Count_Gap);
                pSpec->m_nOutline_align_Body_Search_Count_Gap = nMinimumValue;
                break;
            }
            case ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_X:
                bShowEdgeSearchROI = true;
                break;
            case ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_Y:
                bShowEdgeSearchROI = true;
                break;
            case ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_WIDTH_RATIO:
                bShowEdgeSearchROI = true;
                break;
            case ITEM_ID_OUTLINE_ALIGN_ELECTRODE_SEARCH_LENGTH_UM:
                bShowEdgeSearchROI = true;
                break;
            case ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_WIDTH_RATIO:
                bShowEdgeSearchROI = true;
                break;
            case ITEM_ID_OUTLINE_ALIGN_BODY_SEARCH_LENGTH_UM:
                bShowEdgeSearchROI = true;
                break;
        }

        if (bShowEdgeSearchROI)
            ShowEdgeSearchROI(pCurSelectPassiveInfoDB, item->GetID());
    }

    if (auto* ComboValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = ComboValue->GetEnum();

        switch (item->GetID())
        {
            case ITEM_ID_PAD_USE_ALIGN:
                pSpec->m_padUseAlign = data ? TRUE : FALSE;
                break;

            case ITEM_ID_ROUGH_ALIGN_IMAGE:
            {
                switch (wparam)
                {
                    case XTP_PGN_ITEMVALUE_CHANGED:
                    case XTP_PGN_SELECTION_CHANGED:
                        ShowImage((long)data, pCurSelectPassiveInfoDB);
                        break;
                }
                break;
            }

            case ITEM_ID_OUTLINE_ALIGN_ELECTRODE_IMAGE:
            {
                switch (wparam)
                {
                    case XTP_PGN_ITEMVALUE_CHANGED:
                    case XTP_PGN_SELECTION_CHANGED:
                        ShowImage((long)data, pCurSelectPassiveInfoDB);
                        break;
                }
                break;
            }

            case ITEM_ID_OUTLINE_ALIGN_BODY_IMAGE:
            {
                switch (wparam)
                {
                    case XTP_PGN_ITEMVALUE_CHANGED:
                    case XTP_PGN_SELECTION_CHANGED:
                        ShowImage((long)data, pCurSelectPassiveInfoDB);
                        break;
                }
                break;
            }

            case ITEM_ID_PAD_ALIGN_IMAGE:
                switch (wparam)
                {
                    case XTP_PGN_ITEMVALUE_CHANGED:
                    case XTP_PGN_SELECTION_CHANGED:
                        ShowImage((long)data, pCurSelectPassiveInfoDB);
                        break;
                }
                break;
        }
    }
    m_propertyGrid->Refresh();

    return 0;
}

BOOL CDlgPassiveAlgorithm::ShowEdgeSearchROI(sPassive_InfoDB* i_pPassiveInfoDB, int nType)
{
    if (i_pPassiveInfoDB == NULL)
        return FALSE;

    ImageLotView* pImageDisp = m_pItem->m_pChipVision->m_pVisionInspDlg->m_imageLotView;
    if (pImageDisp == NULL)
        return FALSE;

    pImageDisp->Overlay_RemoveAll();

    const auto& scale = m_pItem->m_pChipVision->getScale();

    ResultPassiveItem passiveItem;
    m_pItem->DoAlign(scale, i_pPassiveInfoDB, &passiveItem);

    if (nType == ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_X || nType == ITEM_ID_ROUGH_ALIGN_SEARCH_OFFSET_Y)
    {
        pImageDisp->Overlay_AddRectangle(passiveItem.m_roughAlignROI_Hor, RGB(0, 255, 0));
        pImageDisp->Overlay_AddRectangle(passiveItem.m_roughAlignROI_Ver, RGB(0, 255, 0));
        pImageDisp->Overlay_Show(SW_SHOW);
    }
    else
    {
        long nDataSize = (long)m_pItem->m_result.m_vecfrtDebug_DetailSearchROI.size();
        if (nDataSize > 0)
        {
            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_vecfrtDebug_DetailSearchROI, RGB(0, 255, 0));
            pImageDisp->Overlay_Show(SW_SHOW);
        }
    }

    return TRUE;
}

void CDlgPassiveAlgorithm::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    if (bShow)
    {
        m_cmbDebug.ResetContent();

        for (long nDebugViewidx = DebugView_Start; nDebugViewidx < DebugView_End; nDebugViewidx++)
            m_cmbDebug.AddString(g_szPassiveDeubgViewName[nDebugViewidx]);

        TeachRun();
    }

    Invalidate(FALSE);
}

void CDlgPassiveAlgorithm::OnBnClickedButtonTeach()
{
    TeachRun();
}

void CDlgPassiveAlgorithm::TeachRun(FunctionResult* result)
{
    UNREFERENCED_PARAMETER(result);
    if (m_pChipVision->m_pVisionInspDlg == nullptr || m_pChipVision->m_pVisionInspDlg->m_pCurPassiveInfoDB == nullptr)
        return;

    auto* VisionInspDlg = m_pChipVision->m_pVisionInspDlg;
    auto* SelectPassiveInfoDB = VisionInspDlg->m_pCurPassiveInfoDB;

    VisionInspDlg->m_imageLotView->Overlay_RemoveAll();

    PassiveAlignSpec::CapAlignSpec* pSpec = m_pItem->GetCapAlignSpec(SelectPassiveInfoDB->strSpecName);
    if (pSpec == NULL)
        return;

    const auto& scale = m_pItem->m_pChipVision->getScale();

    ResultPassiveItem passiveItem;
    m_pItem->DoAlign(scale, SelectPassiveInfoDB, &passiveItem);

    SetPropertyGrid(SelectPassiveInfoDB);

    CString strTemp("");
    SetDlgItemText(IDC_STATIC_RESULT, strTemp);

    Invalidate(FALSE);

    strTemp.Empty();
}

void CDlgPassiveAlgorithm::ShowImage(long nFrame, sPassive_InfoDB* i_pPassiveInfoDB)
{
    VisionInspectionComponent2D* pChipAlign = m_pChipVision;

    const auto orgimage = pChipAlign->GetInspectionFrameImage(TRUE, nFrame);
    CString FrameToString = pChipAlign->getIllumInfo2D().getIllumFullName(pChipAlign->GetImageFrameIndex(nFrame));

    if (orgimage.GetMem() == nullptr)
        return;

    Ipvm::Image8u viewimage;
    BOOL bRotateImage = !(i_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Horizontal
        || i_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Horizontal_reverse
        || i_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Vertical
        || i_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Vertical_reverse);
    if (bRotateImage)
    {
        if (pChipAlign->getReusableMemory().GetInspByteImage(viewimage))
        {
            Ipvm::ImageProcessing::Copy(orgimage, Ipvm::Rect32s(orgimage), viewimage);
            pChipAlign->m_pPassiveAlign->RotateImage(orgimage, viewimage, i_pPassiveInfoDB);
        }
    }
    else
    {
        viewimage = orgimage;
    }

    pChipAlign->m_pVisionInspDlg->m_imageLotView->SetImage(viewimage, FrameToString);

    FrameToString.Empty();
}

void CDlgPassiveAlgorithm::OnCbnSelchangeCmbDebug()
{
    ImageLotView* pImageDisp = m_pItem->m_pChipVision->m_pVisionInspDlg->m_imageLotView;
    if (pImageDisp == NULL)
        return;

    sPassive_InfoDB* pCurSelectPassiveInfoDB = m_pItem->m_pChipVision->m_pVisionInspDlg->m_pCurPassiveInfoDB;
    if (pCurSelectPassiveInfoDB == nullptr)
        return;

    const auto& scale = m_pItem->m_pChipVision->getScale();

    ResultPassiveItem passiveItem;
    m_pItem->DoAlign(scale, pCurSelectPassiveInfoDB, &passiveItem);

    pImageDisp->Overlay_RemoveAll();
    pImageDisp->ROI_RemoveAll();

    long nSel = m_cmbDebug.GetCurSel();
    COLORREF Color = RGB(0, 255, 0);

    long nDebugIndex(0);
    switch (nSel)
    {
        case DebugView_SpecPassiveROI:
            pImageDisp->Overlay_AddRectangle(m_pItem->m_result.m_chip_spec, RGB(0, 255, 0));
            nDebugIndex = DebugView_SpecPassiveROI;
            break;
        case DebugView_SpecElectROI:
            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_elect_spec, RGB(0, 255, 0));
            nDebugIndex = DebugView_SpecElectROI;
            break;
        case DebugView_SpecBodyROI:
            pImageDisp->Overlay_AddRectangle(m_pItem->m_result.rtSpecBody, RGB(0, 255, 0));
            nDebugIndex = DebugView_SpecBodyROI;
            break;
        case DebugView_SpecPADROI:
            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_pad_spec, Color);
            nDebugIndex = DebugView_SpecPADROI;
            break;
        case DebugView_ResPADROI:
            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_pad_finalRes, Color);
            nDebugIndex = DebugView_ResPADROI;
            break;
        case DebugView_ResPADBaseEdge:
            for (long n = 0;; n++)
            {
                std::vector<Ipvm::LineEq32r>* lines = nullptr;
                switch (n)
                {
                    case 0:
                        lines = &m_pItem->m_result.m_pad_baseLine;
                        break;
                    case 1:
                        lines = &m_pItem->m_result.m_pad_sideLine;
                        break;
                }

                if (lines == nullptr)
                    break;

                for (long index = 0; index < long(lines->size()); index++)
                {
                    auto& line = (*lines)[index];
                    if (line.m_a > line.m_b)
                    {
                        float maxY = (float)m_pChipVision->getReusableMemory().GetInspImageSizeY();
                        pImageDisp->Overlay_AddLine(Ipvm::Point32r2(line.GetPositionX(0), 0.f),
                            Ipvm::Point32r2(line.GetPositionX(maxY), maxY), RGB(255, 0, 0));
                    }
                    else
                    {
                        float maxX = (float)m_pChipVision->getReusableMemory().GetInspImageSizeX();
                        pImageDisp->Overlay_AddLine(Ipvm::Point32r2(0.f, line.GetPositionY(0)),
                            Ipvm::Point32r2(maxX, line.GetPositionY(maxX)), RGB(255, 0, 0));
                    }
                }
            }

            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_pad_baseline_searchROI, RGB(0, 0, 255));
            pImageDisp->Overlay_AddPoints(m_pItem->m_result.m_pad_baseLine_edges, Color);
            nDebugIndex = DebugView_ResPADBaseEdge;
            break;
        case DebugView_ResPADTopBottomEdge:
            for (long n = 0;; n++)
            {
                std::vector<Ipvm::LineEq32r>* lines = nullptr;
                switch (n)
                {
                    case 0:
                        lines = &m_pItem->m_result.m_pad_topLine;
                        break;
                    case 1:
                        lines = &m_pItem->m_result.m_pad_btmLine;
                        break;
                }

                if (lines == nullptr)
                    break;

                for (long index = 0; index < long(lines->size()); index++)
                {
                    auto& line = (*lines)[index];
                    if (line.m_a > line.m_b)
                    {
                        float maxY = (float)m_pChipVision->getReusableMemory().GetInspImageSizeY();
                        pImageDisp->Overlay_AddLine(Ipvm::Point32r2(line.GetPositionX(0), 0.f),
                            Ipvm::Point32r2(line.GetPositionX(maxY), maxY), RGB(255, 0, 0));
                    }
                    else
                    {
                        float maxX = (float)m_pChipVision->getReusableMemory().GetInspImageSizeX();
                        pImageDisp->Overlay_AddLine(Ipvm::Point32r2(0.f, line.GetPositionY(0)),
                            Ipvm::Point32r2(maxX, line.GetPositionY(maxX)), RGB(255, 0, 0));
                    }
                }
            }

            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_pad_top_bottom_line_searchROI, RGB(0, 0, 255));
            pImageDisp->Overlay_AddPoints(m_pItem->m_result.m_pad_top_bottom_edges, Color);
            nDebugIndex = DebugView_ResPADTopBottomEdge;
            break;
        case DebugView_RoughAlignSearchROI:
            pImageDisp->Overlay_AddRectangle(passiveItem.m_roughAlignROI_Hor, Color);
            pImageDisp->Overlay_AddRectangle(passiveItem.m_roughAlignROI_Ver, Color);
            break;
        case DebugView_RoughAlignResultROI:
            pImageDisp->Overlay_AddRectangle(m_pItem->m_result.m_rtPassiveRough_Align_Result_ROI, Color);
            nDebugIndex = DebugView_RoughAlignResultROI;
            break;
        case DebugView_DetailSearchROI:
        {
            long nDataSize = (long)m_pItem->m_result.m_vecfrtDebug_DetailSearchROI.size();
            std::vector<Ipvm::Rect32r> vecfrtDebug_SearchROI;
            for (long nCount = 0; nCount < nDataSize; nCount++)
                vecfrtDebug_SearchROI.push_back(m_pItem->m_result.m_vecfrtDebug_DetailSearchROI[nCount].GetFRect());
            pImageDisp->Overlay_AddRectangles(vecfrtDebug_SearchROI, Color);
            nDebugIndex = DebugView_DetailSearchROI;
            break;
        }
        case DebugView_DetailShortenROI:
        {
            if (m_pItem->m_result.m_vecrtShortenROI.size() <= 0)
                return;

            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_vecrtShortenROI, Color);
            nDebugIndex = DebugView_DetailShortenROI;
            break;
        }
        case DebugView_DetailShortenEdgePoints:
        {
            if (m_pItem->m_result.m_vecfptMinoraxisEdgePoint.size() <= 0)
                return;

            pImageDisp->Overlay_AddPoints(m_pItem->m_result.m_vecfptMinoraxisEdgePoint, Color);
            nDebugIndex = DebugView_DetailShortenEdgePoints;
            break;
        }
        case DebugView_DetailBodyEdgePoints:
            pImageDisp->Overlay_AddPoints(m_pItem->m_result.m_vecfptDebug_DetailSearchEdgePoints, Color);
            nDebugIndex = DebugView_DetailBodyEdgePoints;
            break;

        case DebugView_DetailAlignResult:
            pImageDisp->Overlay_AddRectangle(m_pItem->m_result.m_chip_resDetailAlign, Color);
            nDebugIndex = DebugView_DetailAlignResult;
            break;

        case DebugView_DetailAlignElect: //얘는 무조건 2개
            pImageDisp->Overlay_AddRectangles(m_pItem->m_result.m_elect_result, Color);
            nDebugIndex = DebugView_DetailAlignElect;
            break;

        case DebugView_DetailAlignBody:
            pImageDisp->Overlay_AddRectangle(m_pItem->m_result.m_chip_resDetailBody, Color);
            nDebugIndex = DebugView_DetailAlignBody;
            break;

        default:
            break;
    }

    SetDebugText(pCurSelectPassiveInfoDB->strCompType, nDebugIndex);

    pImageDisp->Overlay_Show(TRUE);
}

void CDlgPassiveAlgorithm::SetDebugText(LPCTSTR i_strCompType, long i_nDebugIndex)
{
    CString strNormalFormat;
    CString strDataFormat;

    //혹시 모르니까 초기화
    strNormalFormat.Empty();
    strDataFormat.Empty();

    float fLeftPos(0), fTopPos(0), fRightPos(0), fBottomPos(0);
    float fCenPosX(0.f), fCenPosY(0.f);
    Ipvm::Point32r2 fLTPos(0.f, 0.f), fRTPos(0.f, 0.f), fLBPos(0.f, 0.f), fRBPos(0.f, 0.f);
    std::vector<FPI_RECT> vecfrtROIData(2);
    std::vector<Ipvm::Rect32s> vecrtROIData(2);
    std::vector<Ipvm::Rect32r> vec32frtROIData(2);
    EDGEPOINTS vecfptROIData;
    switch (i_nDebugIndex) //Index에 따라 Data쓰는걸 달리한다..
    {
            //{{Data Count 1 고정 항목
        case DebugView_SpecPassiveROI:
            fCenPosX = (float)m_pItem->m_result.m_chip_spec.CenterPoint().m_x;
            fCenPosY = (float)m_pItem->m_result.m_chip_spec.CenterPoint().m_y;
            fLeftPos = m_pItem->m_result.m_chip_spec.m_left;
            fTopPos = m_pItem->m_result.m_chip_spec.m_top;
            fRightPos = m_pItem->m_result.m_chip_spec.m_right;
            fBottomPos = m_pItem->m_result.m_chip_spec.m_bottom;
            break;
        case DebugView_SpecBodyROI:
            fCenPosX = (float)m_pItem->m_result.rtSpecBody.CenterPoint().m_x;
            fCenPosY = (float)m_pItem->m_result.rtSpecBody.CenterPoint().m_y;
            fLeftPos = (float)m_pItem->m_result.rtSpecBody.m_left;
            fTopPos = (float)m_pItem->m_result.rtSpecBody.m_top;
            fRightPos = (float)m_pItem->m_result.rtSpecBody.m_right;
            fBottomPos = (float)m_pItem->m_result.rtSpecBody.m_bottom;
            break;
        case DebugView_DetailAlignResult:
            fCenPosX = m_pItem->m_result.m_chip_resDetailAlign.GetCenter().m_x;
            fCenPosY = m_pItem->m_result.m_chip_resDetailAlign.GetCenter().m_y;
            fLTPos = m_pItem->m_result.m_chip_resDetailAlign.fptLT;
            fRTPos = m_pItem->m_result.m_chip_resDetailAlign.fptRT;
            fLBPos = m_pItem->m_result.m_chip_resDetailAlign.fptLB;
            fRBPos = m_pItem->m_result.m_chip_resDetailAlign.fptRB;
            break;
        case DebugView_DetailAlignBody:
            fCenPosX = m_pItem->m_result.m_chip_resDetailBody.GetCenter().m_x;
            fCenPosY = m_pItem->m_result.m_chip_resDetailBody.GetCenter().m_y;
            fLTPos = m_pItem->m_result.m_chip_resDetailBody.fptLT;
            fRTPos = m_pItem->m_result.m_chip_resDetailBody.fptRT;
            fLBPos = m_pItem->m_result.m_chip_resDetailBody.fptLB;
            fRBPos = m_pItem->m_result.m_chip_resDetailBody.fptRB;
            break;
            //}}
            //{{Data Count 2 고정인 항목
        case DebugView_DetailAlignElect:
            vecfrtROIData = m_pItem->m_result.m_elect_result;
            break;
        case DebugView_SpecElectROI:
            vecrtROIData = m_pItem->m_result.m_elect_spec;
            break;
        case DebugView_SpecPADROI:
            vec32frtROIData = m_pItem->m_result.m_pad_spec;
            break;
            //}}

            //{{Data Count 2 초과인 항목
        case DebugView_DetailBodyEdgePoints:
            vecfptROIData.resize(m_pItem->m_result.m_vecfptDebug_DetailSearchEdgePoints.size());
            vecfptROIData = m_pItem->m_result.m_vecfptDebug_DetailSearchEdgePoints;
            break;
        case DebugView_DetailSearchROI:
            vecfrtROIData.resize(m_pItem->m_result.m_vecfrtDebug_DetailSearchROI.size());
            vecfrtROIData = m_pItem->m_result.m_vecfrtDebug_DetailSearchROI;
            break;
        case DebugView_DetailShortenROI:
            vecrtROIData.resize(m_pItem->m_result.m_vecrtShortenROI.size());
            vecrtROIData = m_pItem->m_result.m_vecrtShortenROI;
            break;
        case DebugView_DetailShortenEdgePoints:
            vecfptROIData.resize(m_pItem->m_result.m_vecfptMinoraxisEdgePoint.size());
            vecfptROIData = m_pItem->m_result.m_vecfptMinoraxisEdgePoint;
            break;
            //}}
        default:
            break;
    }

    CString strTemp;

    if (i_nDebugIndex == DebugView_SpecPassiveROI || i_nDebugIndex == DebugView_SpecBodyROI)
    {
        strNormalFormat.Format(_T("\n Passive Name : %s \n Cneter Pos : %.2f, %.2f \n Data Count : %d"), i_strCompType,
            fCenPosX, fCenPosY, 1);
        strDataFormat.Format(_T("\n      Left         Top        Right      Bottom \n %.2f, %.2f, %.2f, %.2f"),
            fLeftPos, fTopPos, fRightPos, fBottomPos);

        strTemp = strNormalFormat + strDataFormat;
    }
    else if (i_nDebugIndex == DebugView_DetailAlignResult || i_nDebugIndex == DebugView_DetailAlignBody)
    {
        strNormalFormat.Format(_T("\n Passive Name : %s \n Cneter Pos : %.2f, %.2f \n Data Count : %d"), i_strCompType,
            fCenPosX, fCenPosY, 1);
        strDataFormat.Format(_T("\n               LT                           RT                          LB         ")
                             _T("                  RB \n %.2f,%.2f | %.2f,%.2f | %.2f,%.2f | %.2f,%.2f |"),
            fLTPos.m_x, fLTPos.m_y, fRTPos.m_x, fRTPos.m_y, fLBPos.m_x, fLBPos.m_y, fRBPos.m_x, fRBPos.m_y);

        strTemp = strNormalFormat + strDataFormat;
    }

    else if (i_nDebugIndex == DebugView_DetailAlignElect)
    {
        if (vecfrtROIData.size() <= 0)
            return;

        strNormalFormat.Format(_T("\n Passive Name : %s \n Data Count : %d"), i_strCompType, 2);

        strTemp = strNormalFormat;
        long nDataIndex(0);
        for (long n = 0; n < 2; n++)
        {
            strDataFormat.Format(_T("\n               LT                           RT                          LB     ")
                                 _T("                      RB \n %.2f,%.2f | %.2f,%.2f | %.2f,%.2f | %.2f,%.2f |"),
                vecfrtROIData[nDataIndex].fptLT.m_x, vecfrtROIData[nDataIndex].fptLT.m_y,
                vecfrtROIData[nDataIndex].fptRT.m_x, vecfrtROIData[nDataIndex].fptRT.m_y,
                vecfrtROIData[nDataIndex].fptLB.m_x, vecfrtROIData[nDataIndex].fptLB.m_y,
                vecfrtROIData[nDataIndex].fptRB.m_x, vecfrtROIData[nDataIndex].fptRB.m_y);

            strTemp += strDataFormat;
            nDataIndex++;
        }
    }
    else if (i_nDebugIndex == DebugView_SpecElectROI)
    {
        if (vecrtROIData.size() <= 0)
            return;

        strNormalFormat.Format(_T("\n Passive Name : %s \n Data Count : %d"), i_strCompType, 2);
        strTemp = strNormalFormat;
        long nDataIndex(0);
        for (long n = 0; n < 2; n++)
        {
            strDataFormat.Format(_T("\n  Left    Top   Right Bottom \n %d, %d, %d, %d"),
                vecrtROIData[nDataIndex].m_left, vecrtROIData[nDataIndex].m_top, vecrtROIData[nDataIndex].m_right,
                vecrtROIData[nDataIndex].m_bottom);

            strTemp += strDataFormat;

            nDataIndex++;
        }
    }
    else if (i_nDebugIndex == DebugView_SpecPADROI)
    {
        if (vec32frtROIData.size() <= 0)
            return;

        strNormalFormat.Format(_T("\n Passive Name : %s \n Data Count : %d"), i_strCompType, 2);

        strTemp = strNormalFormat;

        long nDataIndex(0);
        for (long n = 0; n < 2; n++)
        {
            strDataFormat.Format(_T("\n      Left         Top        Right      Bottom \n %.2f, %.2f, %.2f, %.2f"),
                vec32frtROIData[nDataIndex].m_bottom, vec32frtROIData[nDataIndex].m_top,
                vec32frtROIData[nDataIndex].m_right, vec32frtROIData[nDataIndex].m_bottom);

            strTemp += strDataFormat;

            nDataIndex++;
        }
    }

    else if (i_nDebugIndex == DebugView_DetailBodyEdgePoints)
    {
        if (vecfptROIData.size() <= 0)
            return;

        strNormalFormat.Format(_T("\n Passive Name : %s \n Data Count : %d \n"), i_strCompType, vecfptROIData.size());

        strTemp = strNormalFormat;
        long nDataIndex(0);
        for (long n = 0; n < vecfptROIData.size(); n++)
        {
            if (n <= 3)
            {
                strDataFormat.Format(_T(" %.2f,%.2f"), vecfptROIData[nDataIndex].m_x, vecfptROIData[nDataIndex].m_y);
            }
            else
            {
                if (n % 4 == 0)
                {
                    strDataFormat.Format(_T("\n"));
                    strTemp += strDataFormat;
                }

                strDataFormat.Format(_T(" %.2f,%.2f"), vecfptROIData[nDataIndex].m_x, vecfptROIData[nDataIndex].m_y);
            }

            strTemp += strDataFormat;
            nDataIndex++;
        }
    }
    else if (i_nDebugIndex == DebugView_DetailSearchROI)
    {
        if (vecfrtROIData.size() <= 0)
            return;

        strNormalFormat.Format(_T("\n Passive Name : %s \n Data Count : %d \n"), i_strCompType, vecfrtROIData.size());

        strTemp = strNormalFormat;

        long nDataIndex(0);
        for (long n = 0; n < vecfrtROIData.size(); n++)
        {
            switch (n)
            {
                case LEFT:
                    strDataFormat.Format(_T("                                                           [LEFT]"));
                    strTemp += strDataFormat;
                    break;
                case UP:
                    strDataFormat.Format(_T("                                                         [Top]"));
                    strTemp += strDataFormat;
                    break;
                case RIGHT:
                    strDataFormat.Format(_T("                                                           [Right]"));
                    strTemp += strDataFormat;
                    break;
                case DOWN:
                    strDataFormat.Format(_T("                                                         [Bottom]"));
                    strTemp += strDataFormat;
                    break;
            }
            strDataFormat.Format(_T("\n               LT                           RT                          LB     ")
                                 _T("                      RB \n %.2f,%.2f | %.2f,%.2f | %.2f,%.2f | %.2f,%.2f |"),
                vecfrtROIData[nDataIndex].fptLT.m_x, vecfrtROIData[nDataIndex].fptLT.m_y,
                vecfrtROIData[nDataIndex].fptRT.m_x, vecfrtROIData[nDataIndex].fptRT.m_y,
                vecfrtROIData[nDataIndex].fptLB.m_x, vecfrtROIData[nDataIndex].fptLB.m_y,
                vecfrtROIData[nDataIndex].fptRB.m_x, vecfrtROIData[nDataIndex].fptRB.m_y);

            strTemp += strDataFormat;
            nDataIndex++;
        }
    }
    else if (i_nDebugIndex == DebugView_DetailShortenROI)
    {
        if (vecrtROIData.size() <= 0)
            return;

        strNormalFormat.Format(_T("\n Passive Name : %s \n Data Count : %d \n"), i_strCompType, vecrtROIData.size());

        strTemp = strNormalFormat;

        for (long nIndex = 0; nIndex < vecrtROIData.size(); nIndex++)
        {
            strDataFormat.Format(_T("\n  Left    Top   Right  Bottom \n %d, %d, %d, %d"), vecrtROIData[nIndex].m_left,
                vecrtROIData[nIndex].m_top, vecrtROIData[nIndex].m_right, vecrtROIData[nIndex].m_bottom);

            strTemp += strDataFormat;
        }
    }
    else if (i_nDebugIndex == DebugView_DetailShortenEdgePoints)
    {
        if (vecfptROIData.size() <= 0)
            return;

        strNormalFormat.Format(_T("\n Passive Name : %s \n Data Count : %d \n"), i_strCompType, vecfptROIData.size());

        strTemp = strNormalFormat;

        for (long n = 0; n < vecfptROIData.size(); n++)
        {
            strDataFormat.Format(_T("\n"));
            strTemp += strDataFormat;

            strDataFormat.Format(_T(" %.2f,%.2f"), vecfptROIData[n].m_x, vecfptROIData[n].m_y);

            strTemp += strDataFormat;
        }
    }

    SetDlgItemText(IDC_STATIC_RESULT, strTemp);
    strNormalFormat.Empty();
    strDataFormat.Empty();
    strTemp.Empty();
}