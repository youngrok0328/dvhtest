//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CDlgBall3DGroupParam.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionBgaBottom3D.h"
#include "VisionInspectionBgaBottom3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100

//CPP_7_________________________________ Implementation body
//
enum ParaID
{
    PID_BallSearchArea = 1,
    PID_BallHeight_MaxValidPoints = 2,
};

IMPLEMENT_DYNAMIC(CDlgBall3DGroupParam, CDialog)

CDlgBall3DGroupParam::CDlgBall3DGroupParam(CDlgVisionInspectionBgaBottom3D* i_pCParentDlg,
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

CDlgBall3DGroupParam::~CDlgBall3DGroupParam()
{
    delete m_propertyGrid;
}

void CDlgBall3DGroupParam::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgBall3DGroupParam, CDialog)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_BN_CLICKED(IDC_BUTTON_APPLY, &CDlgBall3DGroupParam::OnBnClickedButtonApply)
ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgBall3DGroupParam::OnBnClickedButtonCancel)
END_MESSAGE_MAP()

BOOL CDlgBall3DGroupParam::OnInitDialog()
{
    CRect rtParamArea(0, 0, 0, 0);
    GetDlgItem(IDC_STATIC_PARAM_AREA)->GetClientRect(rtParamArea);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtParamArea, this, IDC_FRAME_PROPERTY_GRID);

    SetPropertyGridParam();

    m_propertyGrid->SetViewDivider(0.70);
    m_propertyGrid->HighlightChangedItems(TRUE);

    return TRUE;
}

void CDlgBall3DGroupParam::SetPropertyGridParam()
{
    auto& TypeInfo = m_sCurBallType;
    auto& GroupPara = m_sUpdateGroupBallParam;

    CString str, strText; //kircheis_AIBC
    str.Format(_T("Diameter : %.2lf um / Height %.2lf um"), TypeInfo.m_diameter_um, TypeInfo.m_height_um);

    if (auto* category = m_propertyGrid->AddCategory(str))
    {
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(_T("Ball search area"), GroupPara.m_fBallSearchAreaPercent,
                _T("%.2lf %%"), &GroupPara.m_fBallSearchAreaPercent))
            ->SetID(PID_BallSearchArea);
        category->AddChildItem(new CXTPPropertyGridItemDouble(
            _T("Ball ignore area"), GroupPara.m_fBallIgnorePercent, _T("%.2lf %%"), &GroupPara.m_fBallIgnorePercent));
        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("Ball ignore expansion"),
            GroupPara.m_fBallIgnoreExpandPercent, _T("%.2lf %%"), &GroupPara.m_fBallIgnoreExpandPercent));
        category->AddChildItem(new CXTPPropertyGridItemDouble(_T("SR calculation area"), GroupPara.m_fSRCalcAreaPercent,
            _T("%.2lf %%"), &GroupPara.m_fSRCalcAreaPercent));

        for (long type = 0; type < long(enumGetherBallZType::END); type++)
        {
            auto& para_z = GroupPara.m_paraBallZ[type];
            CString typeName = GetBallZTypeString(enumGetherBallZType(type));

            category->AddChildItem(new CXTPPropertyGridItemDouble(typeName + _T(": Ball calculation area"),
                para_z.m_calcAreaPercent, _T("%.2lf %%"), &para_z.m_calcAreaPercent));
            category->AddChildItem(new CXTPPropertyGridItemDouble(typeName + _T(": Ball valid pixel height threshold"),
                para_z.m_validPixelHeightThreshold_Percent, _T("%.2lf %%"),
                &para_z.m_validPixelHeightThreshold_Percent));
            category->AddChildItem(new CXTPPropertyGridItemDouble(typeName + _T(": Ball Top Percent Average (%)"),
                para_z.m_topPercentAverage, _T("%.2lf %%"), &para_z.m_topPercentAverage));

            if (type == long(enumGetherBallZType::Height))
            {
                CString format = getHeight_MaxValidPointsFormat(TypeInfo, GroupPara);
                category
                    ->AddChildItem(new XTPPropertyGridItemCustomFloat<double>(
                        typeName + _T(": Max. Valid Points of spec radius (%)"),
                        para_z.m_maxValidPointsPercentOfSpecRadius, format, para_z.m_maxValidPointsPercentOfSpecRadius,
                        0., 100., 0.0001))
                    ->SetID(PID_BallHeight_MaxValidPoints);
                format.Empty();
            }

            strText.Format(_T(": Acceptable invalid ball count (Total group ball:%d)"),
                m_sCurBallType.m_groupBallNum); //kircheis_AIBC
            category->AddChildItem(
                new CXTPPropertyGridItemDouble(typeName + strText, para_z.m_dAcceptableInvalidBallCount, _T("%.0lf EA"),
                    &para_z.m_dAcceptableInvalidBallCount)); //kircheis_AIBC

            typeName.Empty();
        }

        category->Expand();
    }
    str.Empty();
    strText.Empty();
}

LRESULT CDlgBall3DGroupParam::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    switch (item->GetID())
    {
        case PID_BallSearchArea:
            if (wparam == XTP_PGN_ITEMVALUE_CHANGED || wparam == XTP_PGN_SELECTION_CHANGED)
            {
                m_pCParentDlg->ViewBallSearchROI(m_nBallGroupidx);
            }
            break;
        case PID_BallHeight_MaxValidPoints:
            if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
            {
                CString format = getHeight_MaxValidPointsFormat(m_sCurBallType, m_sUpdateGroupBallParam);
                auto& para = m_sUpdateGroupBallParam.m_paraBallZ[long(enumGetherBallZType::Height)];
                ((XTPPropertyGridItemCustomFloat<double>*)item)
                    ->SetValue(format, para.m_maxValidPointsPercentOfSpecRadius);
                format.Empty();
            }
            break;
    }

    return 0;
}

void CDlgBall3DGroupParam::OnBnClickedButtonApply()
{
    if (IDYES == AfxMessageBox(_T("Do you want apply parameter?"), MB_YESNO))
        UpdateGroupParameter(true);

    CDialog::OnOK();
}

void CDlgBall3DGroupParam::OnBnClickedButtonCancel()
{
    CDialog::OnCancel();
}

CString CDlgBall3DGroupParam::getHeight_MaxValidPointsFormat(
    Package::BallTypeDefinition i_sCurBallType, BallLevelParameter i_sGroupBallParam)
{
    const auto& scale = m_pCParentDlg->m_pVisionInsp->getScale();
    auto& typeInfo = i_sCurBallType;
    auto area = ITP_PI * (typeInfo.m_diameter_um * 0.5f) * (typeInfo.m_diameter_um * 0.5f);
    auto area_px = long(area * scale.umToPixel().m_x * scale.umToPixel().m_y);
    auto& para = i_sGroupBallParam.m_paraBallZ[long(enumGetherBallZType::Height)];

    auto& persent = para.m_maxValidPointsPercentOfSpecRadius;

    if (persent <= 0.)
        area_px = 0;
    else
        area_px = long(min(100., persent) / 100. * area_px);

    CString format;
    format.Format(_T("%%.3lf %%%% (%d pixels)"), area_px);

    return format;
}

void CDlgBall3DGroupParam::UpdateGroupParameter(bool i_bParamUpdate)
{
    if (i_bParamUpdate)
        m_sRetunGroupBallParam = m_sUpdateGroupBallParam;
    else
        return;
}

BallLevelParameter CDlgBall3DGroupParam::GetGroupParam()
{
    BallLevelParameter sUpdateBallGroupParam = m_sRetunGroupBallParam;

    return sUpdateBallGroupParam;
}