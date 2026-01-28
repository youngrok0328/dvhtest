//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgLandParameter.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionLgaBottom2DPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 101

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID_LandPara
{
    ITEM_ID_START = 0,
    ITEM_ID_EDGE_SEARCH_LENGTH = ITEM_ID_START,
    ITEM_ID_TOP_SEARCH_START,
    ITEM_ID_TOP_SEARCH_END,
    ITEM_ID_BOTTOM_SEARCH_START,
    ITEM_ID_BOTTOM_SEARCH_END,
    ITEM_ID_LEFT_SEARCH_START,
    ITEM_ID_LEFT_SEARCH_END,
    ITEM_ID_RIGHT_SEARCH_START,
    ITEM_ID_RIGHT_SEARCH_END,
    ITEM_ID_EDGE_THRESHOLD,
    ITEM_ID_EDGE_SEARCH_COUNT,
    ITEM_ID_END,
};

// DlgLandParameter 대화 상자

IMPLEMENT_DYNAMIC(DlgLandParameter, CDialog)

DlgLandParameter::DlgLandParameter(
    VisionInspectionLgaBottom2DPara* VisionPara, long GroupIndex, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DLG_LAND_PARAMETER, pParent)
    , m_VisionPara(VisionPara)
    , m_nGroupIndex(GroupIndex)
{
}

DlgLandParameter::~DlgLandParameter()
{
    delete m_propertyGrid;
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

    m_LandParameter = new LandPara;
    m_LandParameter->Copy(m_VisionPara->m_vecLandParameter[m_nGroupIndex]);

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
    if (m_LandParameter->LandType == LandType_Rect)
    {
        if (auto* category = m_propertyGrid->AddCategory(_T("Land Search Parameter")))
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Edge Search Length (mm)"), m_LandParameter->m_fEdgeSearchLength, _T("%.2f")))
                ->SetID(ITEM_ID_EDGE_SEARCH_LENGTH);
            category->Expand();
        }

        if (auto* category = m_propertyGrid->AddCategory(_T("Land Top Search Range")))
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Top Search Start (%)"), m_LandParameter->m_vec2fSearchRangeStart[0], _T("%.2f")))
                ->SetID(ITEM_ID_TOP_SEARCH_START);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Top Search End (%)"), m_LandParameter->m_vec2fSearchRangeEnd[0], _T("%.2f")))
                ->SetID(ITEM_ID_TOP_SEARCH_END);
            category->Expand();
        }

        if (auto* category = m_propertyGrid->AddCategory(_T("Land Bottom Search Range")))
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Bottom Search Start (%)"), m_LandParameter->m_vec2fSearchRangeStart[1], _T("%.2f")))
                ->SetID(ITEM_ID_BOTTOM_SEARCH_START);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Bottom Search End (%)"), m_LandParameter->m_vec2fSearchRangeEnd[1], _T("%.2f")))
                ->SetID(ITEM_ID_BOTTOM_SEARCH_END);
            category->Expand();
        }

        if (auto* category = m_propertyGrid->AddCategory(_T("Land Left Search Range")))
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Left Search Start (%)"), m_LandParameter->m_vec2fSearchRangeStart[2], _T("%.2f")))
                ->SetID(ITEM_ID_LEFT_SEARCH_START);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Left Search End (%)"), m_LandParameter->m_vec2fSearchRangeEnd[2], _T("%.2f")))
                ->SetID(ITEM_ID_LEFT_SEARCH_END);
            category->Expand();
        }

        if (auto* category = m_propertyGrid->AddCategory(_T("Land Rigth Search Range")))
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Rigth Search Start (%)"), m_LandParameter->m_vec2fSearchRangeStart[3], _T("%.2f")))
                ->SetID(ITEM_ID_RIGHT_SEARCH_START);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Rigth Search End (%)"), m_LandParameter->m_vec2fSearchRangeEnd[3], _T("%.2f")))
                ->SetID(ITEM_ID_RIGHT_SEARCH_END);
            category->Expand();
        }
    }
    else if (m_LandParameter->LandType == LandType_Circle)
    {
        if (auto* category = m_propertyGrid->AddCategory(_T("Land Search Parameter")))
        {
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Edge Search Length Ratio"), m_LandParameter->m_fEdgeSearchLength, _T("%.2f")))
                ->SetID(ITEM_ID_EDGE_SEARCH_LENGTH);
            category
                ->AddChildItem(new CXTPPropertyGridItemDouble(
                    _T("Edge Search Threshold"), m_LandParameter->m_fThresholdValue, _T("%.2f")))
                ->SetID(ITEM_ID_EDGE_THRESHOLD);
            category
                ->AddChildItem(
                    new CXTPPropertyGridItemNumber(_T("Edge Search Count"), m_LandParameter->m_nEdgeSearchCount))
                ->SetID(ITEM_ID_EDGE_SEARCH_COUNT);
            category->Expand();
        }
    }
}

LRESULT DlgLandParameter::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
        switch (item->GetID())
        {
            case ITEM_ID_EDGE_SEARCH_LENGTH:
                if (auto* SearchLength = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_fEdgeSearchLength = static_cast<float>(SearchLength->GetDouble());
                break;
            case ITEM_ID_TOP_SEARCH_START:
                if (auto* TopSearchStart = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeStart[0] = static_cast<float>(TopSearchStart->GetDouble());
                break;
            case ITEM_ID_TOP_SEARCH_END:
                if (auto* TopSearchEnd = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeEnd[0] = static_cast<float>(TopSearchEnd->GetDouble());
                break;
            case ITEM_ID_BOTTOM_SEARCH_START:
                if (auto* BottomSearchStart = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeStart[1] = static_cast<float>(BottomSearchStart->GetDouble());
                break;
            case ITEM_ID_BOTTOM_SEARCH_END:
                if (auto* BottomSearchEnd = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeEnd[1] = static_cast<float>(BottomSearchEnd->GetDouble());
                break;
            case ITEM_ID_LEFT_SEARCH_START:
                if (auto* LeftSearchStart = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeStart[2] = static_cast<float>(LeftSearchStart->GetDouble());
                break;
            case ITEM_ID_LEFT_SEARCH_END:
                if (auto* LeftSearchEnd = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeEnd[2] = static_cast<float>(LeftSearchEnd->GetDouble());
                break;
            case ITEM_ID_RIGHT_SEARCH_START:
                if (auto* RightSearchStart = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeStart[3] = static_cast<float>(RightSearchStart->GetDouble());
                break;
            case ITEM_ID_RIGHT_SEARCH_END:
                if (auto* RightSearchEnd = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_vec2fSearchRangeEnd[3] = static_cast<float>(RightSearchEnd->GetDouble());
                break;
            case ITEM_ID_EDGE_THRESHOLD:
                if (auto* EdgeThreshold = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
                    m_LandParameter->m_fThresholdValue = static_cast<float>(EdgeThreshold->GetDouble());
                break;
            case ITEM_ID_EDGE_SEARCH_COUNT:
                if (auto* EdgeSearchCount = dynamic_cast<CXTPPropertyGridItemNumber*>(item))
                    m_LandParameter->m_nEdgeSearchCount = static_cast<long>(EdgeSearchCount->GetNumber());
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
    m_VisionPara->m_vecLandParameter[m_nGroupIndex].Copy(*m_LandParameter);

    CDialog::OnOK();
}

void DlgLandParameter::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    CDialog::OnCancel();
}
