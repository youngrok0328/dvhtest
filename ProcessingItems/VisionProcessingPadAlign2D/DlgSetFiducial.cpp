//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSetFiducial.h"

//CPP_2_________________________________ This project's headers
#include "ParaDB.h"
#include "VisionProcessingPadAlign2DPara.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgSetFiducial, CDialog)

DlgSetFiducial::DlgSetFiducial(ParaDB* paraDB, VisionProcessingPadAlign2DPara* para, CWnd* pParent /*=NULL*/)
    : CDialog(IDD_DIALOG_SET_FIDUCIAL, pParent)
    , m_paraDB(paraDB)
    , m_para(para)
    , m_algorithm_name(_T(""))
    , m_category(0)
{
}

DlgSetFiducial::~DlgSetFiducial()
{
}

void DlgSetFiducial::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_CBString(pDX, IDC_COMBO_Algorithm, m_algorithm_name);
    DDX_Control(pDX, IDC_COMBO_Algorithm, m_cmbAlgorithm);
    DDX_Control(pDX, IDC_COMBO_Category, m_cmbCategory);
    DDX_CBIndex(pDX, IDC_COMBO_Category, m_category);
}

BEGIN_MESSAGE_MAP(DlgSetFiducial, CDialog)
ON_CBN_SELCHANGE(IDC_COMBO_Category, &DlgSetFiducial::OnCbnSelchangeComboCategory)
END_MESSAGE_MAP()

// DlgSetFiducial message handlers

BOOL DlgSetFiducial::OnInitDialog()
{
    CDialog::OnInitDialog();

    for (long n = 0; n < long(enumPAD_Category::END); n++)
    {
        m_cmbCategory.AddString(GetPAD_CategoryName(enumPAD_Category(n)));
        if (enumPAD_Category(m_category) == enumPAD_Category::PinIndex && n == m_category)
        {
            break;
        }
    }

    m_cmbCategory.SetCurSel(m_category);

    if (enumPAD_Category(m_category) == enumPAD_Category::PinIndex)
    {
        // PinIndex는 다른 알고리즘 못씀
        m_cmbCategory.EnableWindow(FALSE);
    }

    UpdateNameList();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgSetFiducial::OnCbnSelchangeComboCategory()
{
    m_category = m_cmbCategory.GetCurSel();
    UpdateNameList();
}

void DlgSetFiducial::UpdateNameList()
{
    std::map<CString, bool> algorithm_list;

    // 현재 세팅된 검사항목 이름 과 DB에 있는 이름 목록을 가져온다
    for (long index = 0; index < m_para->GetAlignPAD_Count(); index++)
    {
        if (m_para->GetAlignPAD_Category(index) != enumPAD_Category(m_category))
            continue;

        algorithm_list[m_para->GetAlignPAD_ParaName(index)] = true;
    }

    m_cmbAlgorithm.ResetContent();
    for (auto it_name : algorithm_list)
    {
        m_cmbAlgorithm.AddString(it_name.first);
    }
}

void DlgSetFiducial::OnOK()
{
    UpdateData(TRUE);

    m_algorithm_name.Trim();

    if (m_algorithm_name.IsEmpty())
    {
        MessageBox(_T("Algorithm's name empty."));
        return;
    }

    CDialog::OnOK();
}
