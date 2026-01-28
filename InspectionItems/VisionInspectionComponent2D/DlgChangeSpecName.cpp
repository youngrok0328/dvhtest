//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgChangeSpecName.h"

//CPP_2_________________________________ This project's headers
#include "PassiveAlign.h"
#include "VisionInspectionComponent2DPara.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgChangeSpecName, CDialog)

CDlgChangeSpecName::CDlgChangeSpecName(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgChangeSpecName::IDD, pParent)
    , m_strReName(_T(""))
{
}

CDlgChangeSpecName::~CDlgChangeSpecName()
{
}

void CDlgChangeSpecName::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_SPEC_RENAME, m_strReName);
    DDX_Control(pDX, IDC_COMBO_SPEC, m_cmbSpec);
}

BEGIN_MESSAGE_MAP(CDlgChangeSpecName, CDialog)
ON_BN_CLICKED(IDOK, &CDlgChangeSpecName::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgChangeSpecName::OnBnClickedCancel)
ON_CBN_SELCHANGE(IDC_COMBO_SPEC, &CDlgChangeSpecName::OnCbnSelchangeComboSpec)
ON_BN_CLICKED(IDC_BN_DEL, &CDlgChangeSpecName::OnBnClickedBnDel)
ON_BN_CLICKED(IDC_BN_CLEAR, &CDlgChangeSpecName::OnBnClickedBnClear)
END_MESSAGE_MAP()

// CDlgChangeSpecName 메시지 처리기입니다.

void CDlgChangeSpecName::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    UpdateData(TRUE);

    OnOK();
}

void CDlgChangeSpecName::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    OnCancel();
}

BOOL CDlgChangeSpecName::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    UpdateSpecList();

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

INT_PTR CDlgChangeSpecName::DoModal(VisionInspectionComponent2DPara* pPara, sPassive_InfoDB* i_pCurPassiveInfo)
{
    // TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
    m_pPara = pPara;
    m_pCurPassiveInfo = i_pCurPassiveInfo;

    return CDialog::DoModal();
}

void CDlgChangeSpecName::OnCbnSelchangeComboSpec()
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

void CDlgChangeSpecName::OnBnClickedBnDel()
{
    if (IDYES == AfxMessageBox(_T("Are you sure you want to delete?"), MB_YESNO))
    {
        long nSel = m_cmbSpec.GetCurSel();
        if (nSel >= 0)
        {
            if (m_pCurPassiveInfo->eComponentType == eComponentTypeDefine::tyPASSIVE)
            {
                auto& vecPassiveAlgoSpec = m_pPara->m_vecPassiveAlgoSpec;
                vecPassiveAlgoSpec.erase(nSel);
            }
        }
    }

    UpdateSpecList();
}

void CDlgChangeSpecName::OnBnClickedBnClear()
{
    if (IDYES == AfxMessageBox(_T("All spec data will be removed.. Are you sure you want to clear?"), MB_YESNO))
    {
        if (m_pCurPassiveInfo->eComponentType == eComponentTypeDefine::tyPASSIVE)
        {
            auto& vecPassiveAlgoSpec = m_pPara->m_vecPassiveAlgoSpec;
            vecPassiveAlgoSpec.clear();

            auto& DefaultSpec = vecPassiveAlgoSpec.add();
            m_pCurPassiveInfo->strSpecName = DefaultSpec.strSpecName;
        }
    }

    UpdateSpecList();
}

void CDlgChangeSpecName::UpdateSpecList()
{
    if (m_pCurPassiveInfo->eComponentType == eComponentTypeDefine::tyPASSIVE)
    {
        auto& vecPassiveAlgoSpec = m_pPara->m_vecPassiveAlgoSpec;

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