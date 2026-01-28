//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgChangeSpecName3D.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionComponent3DPara.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgChangeSpecName3D, CDialog)

CDlgChangeSpecName3D::CDlgChangeSpecName3D(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgChangeSpecName3D::IDD, pParent)
    , m_strReName(_T(""))
{
}

CDlgChangeSpecName3D::~CDlgChangeSpecName3D()
{
}

void CDlgChangeSpecName3D::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_CHANGE_SPEC, m_cmbSpec);
    DDX_Text(pDX, IDC_EDIT_CHANGE_SPEC, m_strReName);
}

BEGIN_MESSAGE_MAP(CDlgChangeSpecName3D, CDialog)
ON_BN_CLICKED(IDC_BUTTON_DEL, &CDlgChangeSpecName3D::OnBnClickedButtonDel)
ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CDlgChangeSpecName3D::OnBnClickedButtonClear)
ON_BN_CLICKED(IDOK, &CDlgChangeSpecName3D::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgChangeSpecName3D::OnBnClickedCancel)
ON_CBN_SELCHANGE(IDC_COMBO_CHANGE_SPEC, &CDlgChangeSpecName3D::OnCbnSelchangeComboChangeSpec)
END_MESSAGE_MAP()

// CDlgChangeSpecName3D 메시지 처리기입니다.

void CDlgChangeSpecName3D::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);

    OnOK();
}

void CDlgChangeSpecName3D::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnCancel();
}

BOOL CDlgChangeSpecName3D::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    UpdateSpecList();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

INT_PTR CDlgChangeSpecName3D::DoModal(VisionInspectionComponent3DPara* pPara, sPassive_InfoDB* i_pCurPassiveInfo)
{
    // TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
    m_pPara = pPara;
    m_pCurPassiveInfo = i_pCurPassiveInfo;
    return CDialog::DoModal();
}

void CDlgChangeSpecName3D::OnCbnSelchangeComboChangeSpec()
{
    long nSel = m_cmbSpec.GetCurSel();
    if (nSel < 0 || nSel >= (long)m_vecstrSpec.size())
        return;

    CString strSpec;
    m_cmbSpec.GetLBText(nSel, strSpec);

    if (strSpec.GetLength() > 0)
        m_strReName = strSpec;

    UpdateData(FALSE);

    strSpec.Empty();
}

void CDlgChangeSpecName3D::OnBnClickedButtonDel()
{
    if (IDYES == AfxMessageBox(_T("Are you sure you want to delete?"), MB_YESNO))
    {
        long nSel = m_cmbSpec.GetCurSel();
        if (nSel >= 0)
        {
            if (m_pCurPassiveInfo->eComponentType == eComponentTypeDefine::tyPASSIVE)
            {
                auto& vecPassiveAlgoSpec = m_pPara->m_vecComp3DAlgoSpec;
                if (vecPassiveAlgoSpec[nSel].strSpecName != "Default") // default는 남겨야함
                    vecPassiveAlgoSpec.erase(vecPassiveAlgoSpec.begin() + nSel);

                m_pCurPassiveInfo->strSpecName = vecPassiveAlgoSpec[0].strSpecName;
            }
        }
    }

    UpdateSpecList();
}

void CDlgChangeSpecName3D::OnBnClickedButtonClear()
{
    if (IDYES == AfxMessageBox(_T("All spec data will be removed.. Are you sure you want to clear?"), MB_YESNO))
    {
        if (m_pCurPassiveInfo->eComponentType == eComponentTypeDefine::tyPASSIVE)
        {
            auto& vecPassiveAlgoSpec = m_pPara->m_vecComp3DAlgoSpec;
            vecPassiveAlgoSpec.clear();

            SComp3DAlgorithmSpec DefaultSpec;
            vecPassiveAlgoSpec.push_back(DefaultSpec);
            m_pCurPassiveInfo->strSpecName = DefaultSpec.strSpecName;
        }
    }

    UpdateSpecList();

    m_strReName = "Default";
    UpdateData(FALSE);
}

void CDlgChangeSpecName3D::UpdateSpecList()
{
    if (m_pCurPassiveInfo->eComponentType == eComponentTypeDefine::tyPASSIVE)
    {
        auto& vecPassiveAlgoSpec = m_pPara->m_vecComp3DAlgoSpec;

        m_vecstrSpec.resize(vecPassiveAlgoSpec.size());
        for (long i = 0; i < m_vecstrSpec.size(); i++)
            m_vecstrSpec[i] = vecPassiveAlgoSpec[i].strSpecName;
    }

    long nSel = -1;
    m_cmbSpec.ResetContent();
    for (long i = 0; i < (long)m_vecstrSpec.size(); i++)
    {
        if (m_pCurPassiveInfo->strSpecName.Compare(m_vecstrSpec[i]) == 0)
            nSel = i;
        m_cmbSpec.AddString(m_vecstrSpec[i]);
    }
    if (nSel < 0)
    {
        AfxMessageBox(_T("?"));
    }

    m_cmbSpec.SetCurSel(nSel);
}