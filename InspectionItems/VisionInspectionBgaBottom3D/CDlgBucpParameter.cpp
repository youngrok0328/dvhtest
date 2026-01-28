//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CDlgBucpParameter.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionBgaBottom3DPara.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 104

//CPP_7_________________________________ Implementation body
//
enum PropertyGridItemID_BallPara
{
    ITEM_ID_START = 0,
    ITEM_ID_GROUP_ID = ITEM_ID_START,
    ITEM_ID_END,
};

IMPLEMENT_DYNAMIC(CDlgBucpParameter, CDialog)

CDlgBucpParameter::CDlgBucpParameter(VisionInspectionBgaBottom3DPara* VisionPara, std::vector<CString>& group_id_lst,
    long GroupIndex, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DLG_BUCP_PARAMETER, pParent)
    , m_pVisionPara(VisionPara)
    , m_nGroupIndex(GroupIndex)
    , m_group_id_lst(group_id_lst)
{
}

CDlgBucpParameter::~CDlgBucpParameter()
{
}

void CDlgBucpParameter::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgBucpParameter, CDialog)
ON_BN_CLICKED(IDOK, &CDlgBucpParameter::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgBucpParameter::OnBnClickedCancel)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, &CDlgBucpParameter::OnGridNotify)
END_MESSAGE_MAP()

// CDlgBucpParameter 메시지 처리기

void CDlgBucpParameter::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long CheckCount = 0;

    for (int i = 0; i < long(m_group_id_lst.size()); i++)
    {
        if (m_vecParameterInfo[i] == TRUE)
        {
            CheckCount++;
        }
    }

    if (CheckCount < 2)
    {
        AfxMessageBox(_T("Please select at least 2 groups."), MB_OK);
        return;
    }

    if (IDYES == AfxMessageBox(_T("Do you want apply parameter?"), MB_YESNO))
        MakeUseGroupInfo();

    CDialog::OnOK();
}

void CDlgBucpParameter::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    CDialog::OnCancel();
}

BOOL CDlgBucpParameter::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    CRect rtParamArea(0, 0, 0, 0);
    GetDlgItem(IDC_STATIC_PARAM_AREA)->GetClientRect(rtParamArea);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtParamArea, this, IDC_FRAME_PROPERTY_GRID);

    MakeParameterInfo();

    UpdatePropertyGrid();

    m_propertyGrid->SetViewDivider(0.70);
    m_propertyGrid->HighlightChangedItems(TRUE);

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CDlgBucpParameter::MakeParameterInfo()
{
    m_vecParameterInfo.clear();
    m_vecParameterInfo.resize(m_group_id_lst.size(), FALSE);

    long MaxCount = (long)m_pVisionPara->m_vec2UnitCoplGroup[m_nGroupIndex].size();

    for (int j = 0; j < MaxCount; j++)
    {
        long selectGroup = m_pVisionPara->m_vec2UnitCoplGroup[m_nGroupIndex][j];

        if (selectGroup >= m_vecParameterInfo.size())
        {
            continue;
        }

        m_vecParameterInfo[selectGroup] = TRUE;
    }

    return true;
}

BOOL CDlgBucpParameter::MakeUseGroupInfo()
{
    m_pVisionPara->m_vec2UnitCoplGroup[m_nGroupIndex].clear();

    for (int i = 0; i < long(m_group_id_lst.size()); i++)
    {
        if (m_vecParameterInfo[i] == TRUE)
        {
            m_pVisionPara->m_vec2UnitCoplGroup[m_nGroupIndex].push_back(i);
        }
    }

    return true;
}

void CDlgBucpParameter::UpdatePropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("Data to be used for inspection")))
    {
        for (int i = 0; i < long(m_group_id_lst.size()); i++)
        {
            CString str;

            str.Format(_T("Group %d (%s)"), i + 1, LPCTSTR(m_group_id_lst[i]));

            if (auto* item = category->AddChildItem(
                    new CXTPPropertyGridItemBool(str, m_vecParameterInfo[i], &m_vecParameterInfo[i])))
            {
                (dynamic_cast<CXTPPropertyGridItemBool*>(item))->SetCheckBoxStyle();

                item->SetID(ITEM_ID_GROUP_ID + i);
            }

            str.Empty();
        }

        category->Expand();
    }
}

LRESULT CDlgBucpParameter::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    UNREFERENCED_PARAMETER(item);

    if (wparam == XTP_PGN_ITEMVALUE_CHANGED)
    {
    }

    return 0;
}
