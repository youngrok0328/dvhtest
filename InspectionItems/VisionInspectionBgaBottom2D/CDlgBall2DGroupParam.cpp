//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CDlgBall2DGroupParam.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBottom2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomItems.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100

//CPP_7_________________________________ Implementation body
//
enum enumGroupItemID
{
    ITEM_ID_BALL_EDGE_SEARCH_MODE,
    ITEM_ID_BALL_WIDTH_CALCULATION_METOHD,
    ITEM_ID_BALL_EDGE_ROUGH_SEARCH_LENGTH_RATIO,
    ITEM_ID_BALL_EDGE_DETAIL_SEARCH_LENGTH_RATIO,
    ITEM_ID_BALL_EDGE_SEARCH_IGNORE_LENGTH_RATIO,
    ITEM_ID_BALL_RE_SEARCH_MODE,
    ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_INNER,
    ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_OUTER,
    ITEM_ID_BALL_RE_SEARCH_NOISE_FILTER_THRESHOLD_RATIO, //kircheis_BallFluxModify
    ITEM_ID_BALL_ROUGH_ALIGN_METHOD, //kircheis_LKF
    ITEM_ID_QUALITY_THRESHOLD_VALUE,
    ITEM_ID_QUALITY_CHECK_METHOD,
    ITEM_ID_QUALITY_CHECK_INNER,
    ITEM_ID_QUALITY_CHECK_OUTER,
    ITEM_ID_QUALITY_CHECK_SAMPLINGINTERVAL,
    ITEM_ID_QUALITY_RANGE_BEGIN,
    ITEM_ID_QUALITY_RANGE_END,
    ITEM_ID_QUALITY_RANGE_RATIO,
    ITEM_ID_TAUGHT_RING_THICKNESS,
    ITEM_ID_TEACH_BUTTON,
};

IMPLEMENT_DYNAMIC(CDlgBall2DGroupParam, CDialog)

CDlgBall2DGroupParam::CDlgBall2DGroupParam(CDlgVisionInspectionBgaBottom2D* i_pCParentDlg,
    BallLevelParameter i_sGroupBallParam, Package::BallTypeDefinition i_sBallTypeDefinition, long i_nBallGroupidx,
    CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DIALOG_PARAMETER, pParent)
    , m_pCParentDlg(i_pCParentDlg)
    , m_propertyGrid(nullptr)
    , m_sUpdateGroupBallParam(i_sGroupBallParam)
    , m_sRetunGroupBallParam(i_sGroupBallParam)
    , m_sCurBallType(i_sBallTypeDefinition)
    , m_nBallGroupidx(i_nBallGroupidx)
{
}

CDlgBall2DGroupParam::~CDlgBall2DGroupParam()
{
}

void CDlgBall2DGroupParam::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgBall2DGroupParam, CDialog)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDlgBall2DGroupParam::OnBnClickedButtonApply)
ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgBall2DGroupParam::OnBnClickedButtonCancel)
END_MESSAGE_MAP()

BOOL CDlgBall2DGroupParam::OnInitDialog()
{
    CRect rtParamArea(0, 0, 0, 0);
    GetDlgItem(IDC_STATIC_PARAM_AREA)->GetClientRect(rtParamArea);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtParamArea, this, IDC_FRAME_PROPERTY_GRID);

    SetPropertyGridParam();

    m_propertyGrid->SetViewDivider(0.55); //kircheis_LKF
    m_propertyGrid->HighlightChangedItems(TRUE);

    return TRUE;
}

void CDlgBall2DGroupParam::SetPropertyGridParam()
{
    auto& TypeInfo = m_sCurBallType;
    auto& GroupPara = m_sUpdateGroupBallParam;

    CString str;
    str.Format(_T("GroupID : G%d Diameter : %.2lf um / Height %.2lf um"), m_nBallGroupidx + 1, TypeInfo.m_diameter_um,
        TypeInfo.m_height_um); //Group ID Start Value 1
    this->SetWindowTextW(str);

    str.Empty();

    if (auto* category = m_propertyGrid->AddCategory(_T("Ball width calculation parameters")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Ball edge search mode"),
                GroupPara.m_nBallEdgeCountMode, (int*)&GroupPara.m_nBallEdgeCountMode))) //kircheis_BallFlux
        {
            item->GetConstraints()->AddConstraint(_T("Fast Mode"), 0);
            item->GetConstraints()->AddConstraint(_T("Detail Mode"), 1);
            item->SetID(ITEM_ID_BALL_EDGE_SEARCH_MODE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Inspection method"), GroupPara.m_nBallWidthMethod, (int*)&GroupPara.m_nBallWidthMethod)))
        {
            item->GetConstraints()->AddConstraint(_T("Normal fitting"), 0);
            item->GetConstraints()->AddConstraint(_T("Maximum value"), 1);
            item->SetID(ITEM_ID_BALL_WIDTH_CALCULATION_METOHD);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Ball edge rough search length ratio"), GroupPara.m_ballWidthRoughSearchLengthRatio,
                _T("%.2lf x ball diameter"), &GroupPara.m_ballWidthRoughSearchLengthRatio)))
        {
            item->SetID(ITEM_ID_BALL_EDGE_ROUGH_SEARCH_LENGTH_RATIO);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Ball edge detail search length ratio"), GroupPara.m_ballWidthDetailSearchLengthRatio,
                _T("%.2lf x ball diameter"), &GroupPara.m_ballWidthDetailSearchLengthRatio))) //kircheis_BallFlux
        {
            item->SetID(ITEM_ID_BALL_EDGE_DETAIL_SEARCH_LENGTH_RATIO);
        }

        if (auto* item
            = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Ball edge search ignore length ratio"),
                GroupPara.m_ballWidthSearchIgnoreLengthRatio, _T("%.2lf x ball diameter")))) //kk
        {
            item->SetID(ITEM_ID_BALL_EDGE_SEARCH_IGNORE_LENGTH_RATIO);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Edge re-search mode by oblique"),
                GroupPara.m_nBallEdgeReSearchMode, (int*)&GroupPara.m_nBallEdgeReSearchMode))) //kircheis_BallFlux
        {
            item->GetConstraints()->AddConstraint(_T("Not use"), 0);
            item->GetConstraints()->AddConstraint(_T("Use"), 1);
            item->SetID(ITEM_ID_BALL_RE_SEARCH_MODE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Ball edge re-search length ratio (Inner)"), GroupPara.m_ballReSearchLengthRatioInner,
                _T("%.2lf x ball diameter"), &GroupPara.m_ballReSearchLengthRatioInner))) //kircheis_BallFlux
        {
            item->SetID(ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_INNER);
            item->SetReadOnly(!GroupPara.m_nBallEdgeReSearchMode);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Ball edge re-search length ratio (Outer)"), GroupPara.m_ballReSearchLengthRatioOuter,
                _T("%.2lf x ball diameter"), &GroupPara.m_ballReSearchLengthRatioOuter))) //kircheis_BallFlux
        {
            item->SetID(ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_OUTER);
            item->SetReadOnly(!GroupPara.m_nBallEdgeReSearchMode);
        }
        if (auto* item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Ball edge re-search noise filter threshold ratio"),
                    GroupPara.m_ballReSearchNoiseFilterThresholdRatio, _T("%.2lf x ball radius"),
                    &GroupPara.m_ballReSearchNoiseFilterThresholdRatio))) //kircheis_BallFluxModify
        {
            item->SetID(ITEM_ID_BALL_RE_SEARCH_NOISE_FILTER_THRESHOLD_RATIO);
            item->SetReadOnly(!GroupPara.m_nBallEdgeReSearchMode);
        }
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Ball rough align method"),
                GroupPara.m_nRoughAlignMethod, (int*)&GroupPara.m_nRoughAlignMethod))) //kircheis_LKF
        {
            item->GetConstraints()->AddConstraint(_T("Default-Circle fitting"), 0);
            item->GetConstraints()->AddConstraint(_T("Use peak of ball in coax. image"), 1);
            item->SetID(ITEM_ID_BALL_ROUGH_ALIGN_METHOD);
        }
        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Ball quality inspection parameters")))
    {
        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemNumber(
                _T("Threshold"), GroupPara.m_qualityThresholdValue, &GroupPara.m_qualityThresholdValue)))
        {
            if (auto* slider = item->AddSliderControl())
            {
                slider->SetMin(0);
                slider->SetMax(256);
            }

            if (auto* spin = item->AddSpinButton())
            {
                spin->SetMin(0);
                spin->SetMax(256);
            }

            item->SetID(ITEM_ID_QUALITY_THRESHOLD_VALUE);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemEnum(
                _T("Inspection method"), GroupPara.m_qualityCheckMethod, (int*)&GroupPara.m_qualityCheckMethod)))
        {
            item->SetID(ITEM_ID_QUALITY_CHECK_METHOD);
            item->GetConstraints()->AddConstraint(_T("Normal"), 0);
            item->GetConstraints()->AddConstraint(_T("Ring thickness"), 1);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Void checking position in"),
                GroupPara.m_qualityCheckInnerVoid, _T("%.2lf x ball radius"), &GroupPara.m_qualityCheckInnerVoid)))
        {
            item->SetID(ITEM_ID_QUALITY_CHECK_INNER);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Void checking position out"),
                GroupPara.m_qualityCheckOuterVoid, _T("%.2lf x ball radius"), &GroupPara.m_qualityCheckOuterVoid)))
        {
            item->SetID(ITEM_ID_QUALITY_CHECK_OUTER);
        }

        if (auto* item = category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Sampling interval"),
                GroupPara.m_qualityCheckSamplingInterval, _T("%.1lf px"), &GroupPara.m_qualityCheckSamplingInterval)))
        {
            item->SetID(ITEM_ID_QUALITY_CHECK_SAMPLINGINTERVAL);
        }

        if (auto* item = m_propertyGrid->AddCategory(_T("Normal method")))
        {
            if (auto* subItem = item->AddChildItem(new CXTPPropertyGridItemDouble(_T("Checking range begin"),
                    GroupPara.m_qualityCheckRangeMin, _T("%.2lf x ball radius"), &GroupPara.m_qualityCheckRangeMin)))
            {
                subItem->SetID(ITEM_ID_QUALITY_RANGE_BEGIN);
            }

            if (auto* subItem = item->AddChildItem(new CXTPPropertyGridItemDouble(_T("Checking range end"),
                    GroupPara.m_qualityCheckRangeMax, _T("%.2lf x ball radius"), &GroupPara.m_qualityCheckRangeMax)))
            {
                subItem->SetID(ITEM_ID_QUALITY_RANGE_END);
            }

            item->Expand();
        }

        if (auto* item = m_propertyGrid->AddCategory(_T("Ring thickness method")))
        {
            if (auto* subItem = item->AddChildItem(
                    new CXTPPropertyGridItemDouble(_T("Checking range ratio"), GroupPara.m_qualityRingThicknessRate,
                        _T("%.2lf x taught ball radius"), &GroupPara.m_qualityRingThicknessRate)))
            {
                subItem->SetID(ITEM_ID_QUALITY_RANGE_RATIO);
            }

            if (auto* subItem = item->AddChildItem(new CXTPPropertyGridItemDouble(_T("Taught ring radius"),
                    GroupPara.m_qualityRingRadius_px, _T("%.1lf px"), &GroupPara.m_qualityRingRadius_px)))
            {
                subItem->SetReadOnly(TRUE);
                subItem->SetID(ITEM_ID_TAUGHT_RING_THICKNESS);
            }

            if (auto* subItem = item->AddChildItem(new CCustomItemButton(_T("Teach ring radius !!!"), TRUE, FALSE)))
            {
                subItem->SetID(ITEM_ID_TEACH_BUTTON);
                //subItem->SetReadOnly(GroupPara.m_qualityCheckMethod != VisionInspectionBgaBottom2DPara::enumRingThickness);
            }

            item->Expand();
        }

        category->Expand();
    }

    SetReadyOnlyParam(m_sUpdateGroupBallParam.m_qualityCheckMethod);
}

LRESULT CDlgBall2DGroupParam::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    auto& GroupPara = m_sUpdateGroupBallParam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case ITEM_ID_BALL_EDGE_SEARCH_MODE:
                if (auto* data = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                    GroupPara.m_nBallEdgeCountMode = data->GetEnum();
                break;
            case ITEM_ID_BALL_EDGE_ROUGH_SEARCH_LENGTH_RATIO:
                if (auto* Ball_Edge_Rough_Search_Length_Ratio = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_ballWidthRoughSearchLengthRatio = Ball_Edge_Rough_Search_Length_Ratio->GetDouble();
                break;
            case ITEM_ID_BALL_EDGE_DETAIL_SEARCH_LENGTH_RATIO:
                if (auto* Ball_Edge_Detail_Search_Length_Ratio = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_ballWidthDetailSearchLengthRatio = Ball_Edge_Detail_Search_Length_Ratio->GetDouble();
                break;
            case ITEM_ID_BALL_EDGE_SEARCH_IGNORE_LENGTH_RATIO:
                if (auto* Ball_Edge_Search_Ignore_Length_Ratio = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    GroupPara.m_ballWidthSearchIgnoreLengthRatio = Ball_Edge_Search_Ignore_Length_Ratio->GetDouble();

                    if (GroupPara.m_ballWidthSearchIgnoreLengthRatio < 0.1f
                        || GroupPara.m_ballWidthSearchIgnoreLengthRatio > 0.4f)
                    {
                        GroupPara.m_ballWidthSearchIgnoreLengthRatio = 0.3f;
                        Ball_Edge_Search_Ignore_Length_Ratio->SetDouble(GroupPara.m_ballWidthSearchIgnoreLengthRatio);
                    }
                }
                break;
            case ITEM_ID_BALL_RE_SEARCH_MODE:
                if (auto* data = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                {
                    GroupPara.m_nBallEdgeReSearchMode = data->GetEnum();
                    m_propertyGrid->FindItem(ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_INNER)
                        ->SetReadOnly(!GroupPara.m_nBallEdgeReSearchMode);
                    m_propertyGrid->FindItem(ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_OUTER)
                        ->SetReadOnly(!GroupPara.m_nBallEdgeReSearchMode);
                    m_propertyGrid->FindItem(ITEM_ID_BALL_RE_SEARCH_NOISE_FILTER_THRESHOLD_RATIO)
                        ->SetReadOnly(!GroupPara.m_nBallEdgeReSearchMode); //kircheis_BallFluxModify
                }
                break;
            case ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_INNER:
                if (auto* data = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_ballReSearchLengthRatioInner = data->GetDouble();
                break;
            case ITEM_ID_BALL_RE_SEARCH_LENGTH_RATIO_OUTER:
                if (auto* data = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_ballReSearchLengthRatioOuter = data->GetDouble();
                break;
            case ITEM_ID_BALL_RE_SEARCH_NOISE_FILTER_THRESHOLD_RATIO: //kircheis_BallFluxModify
                if (auto* data = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_ballReSearchNoiseFilterThresholdRatio = data->GetDouble();
                break;
            case ITEM_ID_BALL_ROUGH_ALIGN_METHOD: //kircheis_LKF
                if (auto* data = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
                {
                    GroupPara.m_nRoughAlignMethod = data->GetEnum();
                }
                break;
            case ITEM_ID_QUALITY_THRESHOLD_VALUE:
                m_pCParentDlg->ShowImage_Quality(TRUE, GroupPara.m_qualityThresholdValue);
                break;
            case ITEM_ID_QUALITY_CHECK_METHOD:
                SetReadyOnlyParam(m_sUpdateGroupBallParam.m_qualityCheckMethod);
                break;
            case ITEM_ID_QUALITY_CHECK_INNER:
                if (auto* Qulity_check_inner = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_qualityCheckInnerVoid = Qulity_check_inner->GetDouble();
                break;
            case ITEM_ID_QUALITY_CHECK_OUTER:
                if (auto* Qulity_check_outer = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_qualityCheckOuterVoid = Qulity_check_outer->GetDouble();
                break;
            case ITEM_ID_QUALITY_CHECK_SAMPLINGINTERVAL:
                if (auto* Qulity_check_samplinginterval = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_qualityCheckSamplingInterval = Qulity_check_samplinginterval->GetDouble();
                break;
            case ITEM_ID_QUALITY_RANGE_BEGIN:
                if (auto* Qulity_check_Range_Min = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_qualityCheckRangeMin = Qulity_check_Range_Min->GetDouble();
                break;
            case ITEM_ID_QUALITY_RANGE_END:
                if (auto* Qulity_check_Range_Max = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_qualityCheckRangeMax = Qulity_check_Range_Max->GetDouble();
                break;
            case ITEM_ID_QUALITY_RANGE_RATIO:
                if (auto* Qulity_check_Range_Ratio = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    GroupPara.m_qualityRingThicknessRate = Qulity_check_Range_Ratio->GetDouble();
                break;
            case ITEM_ID_TEACH_BUTTON:
                long OriginalData = m_pCParentDlg->m_pVisionPara->m_parameters[m_nBallGroupidx].m_qualityCheckMethod;
                double OriginalData_RingRadius
                    = m_pCParentDlg->m_pVisionPara->m_parameters[m_nBallGroupidx].m_qualityRingRadius_px;

                m_pCParentDlg->m_pVisionPara->m_parameters[m_nBallGroupidx].m_qualityCheckMethod
                    = m_sUpdateGroupBallParam.m_qualityCheckMethod;

                if (m_pCParentDlg->TeachRingRadius(false, m_nBallGroupidx))
                {
                    if (CXTPPropertyGridItemDouble* value = dynamic_cast<CXTPPropertyGridItemDouble*>(
                            m_propertyGrid->FindItem(ITEM_ID_TAUGHT_RING_THICKNESS)))
                    {
                        m_sUpdateGroupBallParam.m_qualityRingRadius_px
                            = m_pCParentDlg->m_pVisionPara->m_parameters[m_nBallGroupidx].m_qualityRingRadius_px;

                        value->SetDouble(m_sUpdateGroupBallParam.m_qualityRingRadius_px);
                    }
                }

                m_pCParentDlg->m_pVisionPara->m_parameters[m_nBallGroupidx].m_qualityCheckMethod = OriginalData;
                m_pCParentDlg->m_pVisionPara->m_parameters[m_nBallGroupidx].m_qualityRingRadius_px
                    = OriginalData_RingRadius;

                break;
        }
    }

    if (wparam == XTP_PGN_SELECTION_CHANGED)
    {
        switch (item->GetID())
        {
            case ITEM_ID_QUALITY_THRESHOLD_VALUE:
                if (auto* control = item->GetInplaceControls()->GetAt(0))
                {
                    auto* slider = dynamic_cast<CXTPPropertyGridInplaceSlider*>(control);

                    if (slider)
                    {
                        slider->GetSliderCtrl()->SetPageSize(8);
                    }
                }
                m_pCParentDlg->ShowImage_Quality(TRUE, GroupPara.m_qualityThresholdValue);
                break;
        }
    }

    return 0;
}

void CDlgBall2DGroupParam::SetReadyOnlyParam(long i_qualityCheckMethod)
{
    switch (i_qualityCheckMethod)
    {
        case VisionInspectionBgaBottom2DPara::enumNormalMethod:
        {
            m_propertyGrid->FindItem(ITEM_ID_TEACH_BUTTON)->SetReadOnly(TRUE);
            m_propertyGrid->FindItem(ITEM_ID_QUALITY_RANGE_BEGIN)->SetReadOnly(FALSE);
            m_propertyGrid->FindItem(ITEM_ID_QUALITY_RANGE_END)->SetReadOnly(FALSE);
            m_propertyGrid->FindItem(ITEM_ID_QUALITY_RANGE_RATIO)->SetReadOnly(TRUE);
        }
        break;
        case VisionInspectionBgaBottom2DPara::enumRingThickness:
        {
            m_propertyGrid->FindItem(ITEM_ID_TEACH_BUTTON)->SetReadOnly(FALSE);
            m_propertyGrid->FindItem(ITEM_ID_QUALITY_RANGE_BEGIN)->SetReadOnly(TRUE);
            m_propertyGrid->FindItem(ITEM_ID_QUALITY_RANGE_END)->SetReadOnly(TRUE);
            m_propertyGrid->FindItem(ITEM_ID_QUALITY_RANGE_RATIO)->SetReadOnly(FALSE);
        }
        break;

        default:
            break;
    }
}

void CDlgBall2DGroupParam::OnBnClickedButtonApply()
{
    if (IDYES == AfxMessageBox(_T("Do you Want Apply Parameter?"), MB_YESNO))
        UpdateGroupParameter(true);

    CDialog::OnOK();
}

void CDlgBall2DGroupParam::OnBnClickedButtonCancel()
{
    CDialog::OnCancel();
}

void CDlgBall2DGroupParam::UpdateGroupParameter(bool i_bParamUpdate)
{
    if (i_bParamUpdate)
        m_sRetunGroupBallParam.Copy(m_sUpdateGroupBallParam);
    else
        return;
}

BallLevelParameter CDlgBall2DGroupParam::GetGroupParam()
{
    return m_sRetunGroupBallParam;
}
