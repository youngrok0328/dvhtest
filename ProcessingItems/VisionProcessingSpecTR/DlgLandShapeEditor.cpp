//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgLandShapeEditor.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingSpec.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgLandShapeEditor, CDialog)

DlgLandShapeEditor::DlgLandShapeEditor(VisionProcessingSpec* pVisionInsp, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_DlgLandShapeEditor, pParent)
    , m_VisionInsp(pVisionInsp)
    , m_PackageSpec(pVisionInsp->m_packageSpec)
{
}

DlgLandShapeEditor::~DlgLandShapeEditor()
{
}

void DlgLandShapeEditor::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_SHAPE_TYPE, m_cbShpaeType);
    DDX_Control(pDX, IDC_GROUP_INFO, m_cbGroupInfo);
}

BEGIN_MESSAGE_MAP(DlgLandShapeEditor, CDialog)
ON_BN_CLICKED(IDOK, &DlgLandShapeEditor::OnBnClickedOk)
ON_CBN_SELCHANGE(IDC_GROUP_INFO, &DlgLandShapeEditor::OnCbnSelchangeGroupInfo)
END_MESSAGE_MAP()

// DlgLandShapeEditor 메시지 처리기

void DlgLandShapeEditor::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long SelectIndex = m_cbGroupInfo.GetCurSel();
    long ChangeType = m_cbShpaeType.GetCurSel();

    if (m_vecnGroupShapeType[SelectIndex] != ChangeType)
    {
        for (auto& LandInfo : m_PackageSpec.m_LandMapConvertOrigin->vecLandData) //m_LandMapManager.vecLandData)
        {
            BOOL bCheckValue = m_group_id_list[SelectIndex] == LandInfo.m_groupID;

            if (bCheckValue)
            {
                LandInfo.nLandShapeType = ChangeType;
            }
        }
    }

    CDialog::OnOK();
}

BOOL DlgLandShapeEditor::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    if (m_PackageSpec.m_LandMapManager->vecLandData.size() <= 0)
    {
        ::SendMessage(this->m_hWnd, WM_CLOSE, NULL, NULL);
        return true;
    }

    m_group_id_list.clear();

    for (auto LandInfoGroupID : m_PackageSpec.m_LandMapManager->vecLandData)
    {
        BOOL bCheckValue = std::find(m_group_id_list.begin(), m_group_id_list.end(), LandInfoGroupID.m_groupID)
            != m_group_id_list.end();

        if (!bCheckValue)
        {
            m_group_id_list.push_back(LandInfoGroupID.m_groupID);
            m_vecnGroupShapeType.push_back(LandInfoGroupID.nLandShapeType);
        }
    }

    long GroupCount = (long)m_group_id_list.size();

    if (GroupCount <= 0)
    {
        ::SendMessage(this->m_hWnd, WM_CLOSE, NULL, NULL);
        return true;
    }

    for (int i = 0; i < GroupCount; i++)
    {
        CString str;
        str.Format(_T("Group %s"), LPCTSTR(m_group_id_list[i]));
        m_cbGroupInfo.AddString(str);

        str.Empty();
    }

    m_cbShpaeType.AddString(_T("Rect"));
    m_cbShpaeType.AddString(_T("Circle"));

    m_cbGroupInfo.SetCurSel(0);
    OnCbnSelchangeGroupInfo();

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgLandShapeEditor::OnCbnSelchangeGroupInfo()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long SelectIndex = m_cbGroupInfo.GetCurSel();

    m_cbShpaeType.SetCurSel(m_vecnGroupShapeType[SelectIndex]);
}
