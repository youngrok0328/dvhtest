//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgAlgorithmDB.h"

//CPP_2_________________________________ This project's headers
#include "VisionProcessingPadAlign2D.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgAlgorithmDB, CDialog)

DlgAlgorithmDB::DlgAlgorithmDB(
    VisionProcessing& proc, VisionProcessingPadAlign2DPara& para, ParaDB& db, CWnd* pParent /*=NULL*/)
    : CDialog(IDD_DIALOG_ALGORITHM_DB, pParent)
    , m_para(proc, proc.m_packageSpec)
    , m_db(proc)
{
    CiDataBase dbObject1;
    CiDataBase dbObject2;

    para.LinkDataBase(TRUE, dbObject1);
    db.LinkDataBase(TRUE, dbObject2);

    m_para.LinkDataBase(FALSE, dbObject1);
    m_db.LinkDataBase(FALSE, dbObject2);
}

DlgAlgorithmDB::~DlgAlgorithmDB()
{
}

void DlgAlgorithmDB::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_JOB, m_lstJob);
    DDX_Control(pDX, IDC_LIST_DB, m_lstDB);
}

BEGIN_MESSAGE_MAP(DlgAlgorithmDB, CDialog)
ON_LBN_SELCHANGE(IDC_LIST_JOB, &DlgAlgorithmDB::OnLbnSelchangeListJob)
ON_LBN_SELCHANGE(IDC_LIST_DB, &DlgAlgorithmDB::OnLbnSelchangeListDb)
ON_BN_CLICKED(IDC_CHIP_INFO_EXPORT, &DlgAlgorithmDB::OnBnClickedChipInfoExport)
ON_BN_CLICKED(IDC_CHIP_DB_IMPORT, &DlgAlgorithmDB::OnBnClickedChipDbImport)
ON_BN_CLICKED(IDC_BTN_DEL_DB, &DlgAlgorithmDB::OnBnClickedBtnDelDb)
END_MESSAGE_MAP()

// DlgAlgorithmDB message handlers

BOOL DlgAlgorithmDB::OnInitDialog()
{
    CDialog::OnInitDialog();

    UpdateJobList();
    UpdateDBList();

    m_lstJob.SetCurSel(0);
    m_lstDB.SetCurSel(0);

    OnLbnSelchangeListJob();
    OnLbnSelchangeListDb();

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void DlgAlgorithmDB::OnLbnSelchangeListJob()
{
    long select_job = m_lstJob.GetCurSel();
    GetDlgItem(IDC_CHIP_INFO_EXPORT)->EnableWindow(select_job >= 0);
}

void DlgAlgorithmDB::OnLbnSelchangeListDb()
{
    CString selectDBName;

    long select_db = m_lstDB.GetCurSel();
    if (select_db >= 0)
        m_lstDB.GetText(select_db, selectDBName);

    bool find_job = false;

    for (long index = 0; index < m_para.GetParaCount(); index++)
    {
        if (selectDBName == m_para.GetParaName(index))
        {
            find_job = true;
        }
    }

    GetDlgItem(IDC_CHIP_DB_IMPORT)->EnableWindow(find_job);
    GetDlgItem(IDC_BTN_DEL_DB)->EnableWindow(select_db >= 0);

    selectDBName.Empty();
}

void DlgAlgorithmDB::OnBnClickedChipInfoExport()
{
    CString select_JobName = GetSelectJobName();

    m_db.add(select_JobName, *m_para.GetPara(select_JobName));
    UpdateDBList();

    select_JobName.Empty();
}

void DlgAlgorithmDB::OnBnClickedChipDbImport()
{
    CString select_dbName = GetSelectDBName();

    m_para.SetPara(select_dbName, *m_db.getAlgorithmPara(select_dbName));
    UpdateDBList();

    select_dbName.Empty();
}

void DlgAlgorithmDB::OnBnClickedBtnDelDb()
{
    long index = m_lstDB.GetCurSel();
    m_db.del(index);

    UpdateDBList();
}

CString DlgAlgorithmDB::GetSelectJobName()
{
    CString returnValue;
    long index = m_lstJob.GetCurSel();
    if (index >= 0)
        m_lstJob.GetText(index, returnValue);

    return returnValue;
}

CString DlgAlgorithmDB::GetSelectDBName()
{
    CString returnValue;
    long index = m_lstDB.GetCurSel();
    if (index >= 0)
        m_lstDB.GetText(index, returnValue);

    return returnValue;
}

void DlgAlgorithmDB::UpdateJobList()
{
    long selectIndex = m_lstJob.GetCurSel();

    m_lstJob.ResetContent();
    for (long index = 0; index < m_para.GetParaCount(); index++)
    {
        m_lstJob.AddString(m_para.GetParaName(index));
    }

    m_lstJob.SetCurSel(selectIndex);
    OnLbnSelchangeListJob();
}

void DlgAlgorithmDB::UpdateDBList()
{
    long selectIndex = m_lstDB.GetCurSel();

    m_lstDB.ResetContent();
    for (long index = 0; index < m_db.count(); index++)
    {
        m_lstDB.AddString(m_db.getAlgoritmName(index));
    }

    m_lstDB.SetCurSel(selectIndex);
    OnLbnSelchangeListDb();
}
