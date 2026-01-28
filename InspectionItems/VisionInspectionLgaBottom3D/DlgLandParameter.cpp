//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgLandParameter.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionLgaBottom3D.h"
#include "VisionInspectionLgaBottom3DPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../UserInterfaceModules/CommonControlExtension/XTPPropertyGridItemCustomFloat.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 102

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID_LandPara
{
    ITEM_ID_START = 0,
    ITEM_ID_SEARCH_OBJECT_X = ITEM_ID_START,
    ITEM_ID_SEARCH_OBJECT_Y,
    ITEM_ID_LAND_CAL_AREAR_SIZE,
    ITEM_ID_LAND_IGNORE_AREA_SIZE,
    ITEM_ID_SR_CAL_AREA_SIZE,
    ITEM_ID_END,
};

IMPLEMENT_DYNAMIC(DlgLandParameter, CDialog)

DlgLandParameter::DlgLandParameter(
    VisionInspectionLgaBottom3DPara* VisionPara, long GroupIndex, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DLG_LAND_PARAMETER, pParent)
    , m_pVisionPara(VisionPara)
    , m_nGroupIndex(GroupIndex)
{
}

DlgLandParameter::~DlgLandParameter()
{
}

void DlgLandParameter::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgLandParameter, CDialog)
ON_BN_CLICKED(IDOK, &DlgLandParameter::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &DlgLandParameter::OnBnClickedCancel)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &DlgLandParameter::OnGridNotify)
END_MESSAGE_MAP()

// DlgLandParameter 메시지 처리기

BOOL DlgLandParameter::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    CRect rtParamArea(0, 0, 0, 0);
    GetDlgItem(IDC_STATIC_PARAM_AREA)->GetClientRect(rtParamArea);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtParamArea, this, IDC_FRAME_PROPERTY_GRID);

    m_LandParameter = new Land3DPara;
    m_LandParameter->Copy(m_pVisionPara->m_vecLandParameter[m_nGroupIndex]);

    UpdatePropertyGrid();

    m_propertyGrid->SetViewDivider(0.70);
    m_propertyGrid->HighlightChangedItems(TRUE);

    CString str;
    str.Format(_T("GroupID : G%d"), m_nGroupIndex + 1);
    this->SetWindowTextW(str);

    str.Empty();

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgLandParameter::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Land Search Parameter")))
    {
        if (m_LandParameter->LandType == LandType_Rect)
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Edge Search Length X (mm)"), m_LandParameter->m_fObjSearchX, _T("%.2f")))
                ->SetID(ITEM_ID_SEARCH_OBJECT_X);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Edge Search Length Y (mm)"), m_LandParameter->m_fObjSearchY, _T("%.2f")))
                ->SetID(ITEM_ID_SEARCH_OBJECT_Y);
        }
        else if (m_LandParameter->LandType == LandType_Circle)
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Edge Search Length (mm)"), m_LandParameter->m_fObjSearchX, _T("%.2f")))
                ->SetID(ITEM_ID_SEARCH_OBJECT_X);
        }

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("Land Mask Parameter")))
    {
        category
            ->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Land Cal. Area from Edge (Erode Size: um)"), m_LandParameter->m_fLandMaskErodeSize, _T("%.2f")))
            ->SetID(ITEM_ID_LAND_CAL_AREAR_SIZE);

        category->Expand();
    }

    if (auto* category = m_propertyGrid->AddCategory(_T("SR Mask Parameter")))
    {
        category
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("Land Ignore Area from Edge (Dilate Size: um)"),
                m_LandParameter->m_SR_landIgnoreSize, _T("%.2f"), m_LandParameter->m_SR_landIgnoreSize, 0.f, 500.f,
                0.1f))
            ->SetID(ITEM_ID_LAND_IGNORE_AREA_SIZE);
        category
            ->AddChildItem(new XTPPropertyGridItemCustomFloat<float>(_T("SR Cal. Area from Land Edge (um)"),
                m_LandParameter->m_SR_landDilateSize, _T("%.2f"), m_LandParameter->m_SR_landDilateSize, 0.f, 5000.f,
                0.1f))
            ->SetID(ITEM_ID_SR_CAL_AREA_SIZE);

        category->Expand();
    }
}

LRESULT DlgLandParameter::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case ITEM_ID_SEARCH_OBJECT_X:
                if (auto* dValue = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    double data = dValue->GetDouble();

                    if (m_LandParameter->LandType == LandType_Rect)
                    {
                        m_LandParameter->m_fObjSearchX = (float)data;
                    }
                    else if (m_LandParameter->LandType == LandType_Circle)
                    {
                        m_LandParameter->m_fObjSearchX = (float)data;
                        m_LandParameter->m_fObjSearchY = m_LandParameter->m_fObjSearchX;
                    }
                }
                break;

            case ITEM_ID_SEARCH_OBJECT_Y:
                if (auto* dValue = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    double data = dValue->GetDouble();
                    m_LandParameter->m_fObjSearchY = (float)data;
                }
                break;

            case ITEM_ID_LAND_CAL_AREAR_SIZE:
                if (auto* dValue = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                {
                    double data = dValue->GetDouble();
                    m_LandParameter->m_fLandMaskErodeSize = (float)data;
                }
                break;

            default:
                break;
        }
    }

    return 0;
}

void DlgLandParameter::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    if (IDYES == AfxMessageBox(_T("Do you want apply parameter?"), MB_YESNO))
        m_pVisionPara->m_vecLandParameter[m_nGroupIndex].Copy(*m_LandParameter);

    CDialog::OnOK();
}

void DlgLandParameter::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    CDialog::OnCancel();
}
