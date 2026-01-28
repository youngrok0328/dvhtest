//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSpecDB.h"

//CPP_2_________________________________ This project's headers
#include "DlgVisionInspectionComponent2D.h"
#include "PassiveAlign.h"
#include "VisionInspectionComponent2D.h"
#include "VisionInspectionComponent2DPara.h" // job 저장

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlgSpecDB, CDialog)

CDlgSpecDB::CDlgSpecDB(VisionInspectionComponent2D* pChipAlign, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgSpecDB::IDD, pParent)
    , m_ChipAlgoDBPara(*pChipAlign)
{
    m_pChipVision = pChipAlign;
    m_SpecID = -1;
    m_JobID = -1;
}

CDlgSpecDB::~CDlgSpecDB()
{
}

void CDlgSpecDB::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_JOB_LISTCONTROL, m_JobListChipInfo);
    DDX_Control(pDX, IDC_SPEC_LISTCONTROL, m_SpecListChipInfo);
}

BEGIN_MESSAGE_MAP(CDlgSpecDB, CDialog)
ON_BN_CLICKED(IDC_CHIP_INFO_EXPORT, &CDlgSpecDB::OnBnClickedChipInfoExport)
ON_BN_CLICKED(IDC_CHIP_DB_IMPORT, &CDlgSpecDB::OnBnClickedChipDbImport)
ON_WM_SHOWWINDOW()
ON_NOTIFY(NM_CLICK, IDC_JOB_LISTCONTROL, &CDlgSpecDB::OnNMClickListChip)
ON_NOTIFY(NM_CLICK, IDC_SPEC_LISTCONTROL, &CDlgSpecDB::OnNMClickListChip)
ON_BN_CLICKED(IDOK, &CDlgSpecDB::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CDlgSpecDB::OnBnClickedCancel)
ON_BN_CLICKED(IDC_BTN_DEL_JOB, &CDlgSpecDB::OnBnClickedBtnDelJob)
ON_BN_CLICKED(IDC_BTN_DEL_DB, &CDlgSpecDB::OnBnClickedBtnDelDb)
END_MESSAGE_MAP()

// CDlgSpecDB 메시지 처리기입니다.
BOOL CDlgSpecDB::LoadChipSpecDB()
{
    CString strFilePath
        = DynamicSystemPath::get(DefineFolder::Job_ChipAlgo) + _T("ChipAlgoDB.db"); // 절대경로로 넣어 준다.
    // Job File Open 한다.
    if (!m_ChipSpecDB.Load(strFilePath))
        return FALSE;

    m_ChipAlgoDBPara.m_bAlgoDB = TRUE;
    m_ChipAlgoDBPara.LinkDataBase(FALSE, m_ChipSpecDB);
    m_ChipAlgoDBPara.m_bAlgoDB = FALSE;

    strFilePath.Empty();

    return TRUE;
}

BOOL CDlgSpecDB::SaveChipSpecDB()
{
    m_ChipAlgoDBPara.LinkDataBase(TRUE, m_ChipSpecDB);

    CString strFilePath = DynamicSystemPath::get(DefineFolder::Job_ChipAlgo) + _T("ChipAlgoDB.db");
    if (!m_ChipSpecDB.Save(strFilePath))
    {
        strFilePath.Empty();
        return FALSE;
    }

    strFilePath.Empty();

    return TRUE;
}

BOOL CDlgSpecDB::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtTemp;

    m_JobListChipInfo.SetExtendedStyle(m_JobListChipInfo.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_JobListChipInfo.GetClientRect(&rtTemp);
    m_JobListChipInfo.InsertColumn(0, _T("No."), LVCFMT_LEFT, 30);
    m_JobListChipInfo.InsertColumn(1, _T("Chip"), LVCFMT_LEFT, 50);
    m_JobListChipInfo.InsertColumn(2, _T("Algorithm"), LVCFMT_LEFT, 100);

    m_SpecListChipInfo.SetExtendedStyle(
        m_SpecListChipInfo.GetExtendedStyle() | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
    m_SpecListChipInfo.GetClientRect(&rtTemp);
    m_SpecListChipInfo.InsertColumn(0, _T("No."), LVCFMT_LEFT, 30);
    m_SpecListChipInfo.InsertColumn(1, _T("Chip"), LVCFMT_LEFT, 50);
    m_SpecListChipInfo.InsertColumn(2, _T("Algorithm"), LVCFMT_LEFT, 100);

    LoadChipSpecDB();

    return TRUE;
}

void CDlgSpecDB::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    if (bShow)
    {
        ShowWidowList();
    }
    else
    {
    }
}

void CDlgSpecDB::ShowWidowList()
{
    CString strTemp;
    m_JobListChipInfo.DeleteAllItems();
    m_SpecListChipInfo.DeleteAllItems();

    long nItemNum = 0;

    auto& vecJobPassiveSpec = m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec;

    auto& vecDBPassiveSpec = m_ChipAlgoDBPara.m_vecPassiveAlgoSpec;

    for (long nItem = 0; nItem < (long)vecJobPassiveSpec.size(); nItem++)
    {
        // No.
        strTemp.Format(_T("%d"), nItemNum);
        m_JobListChipInfo.InsertItem(nItemNum, strTemp);

        // Chip Type
        m_JobListChipInfo.SetItemText(nItemNum, 1, _T("Passive"));

        // Spec Algorithm
        m_JobListChipInfo.SetItemText(nItemNum, 2, vecJobPassiveSpec[nItem].strSpecName);
        nItemNum++;
    }

    nItemNum = 0;
    for (long nItem = 0; nItem < (long)vecDBPassiveSpec.size(); nItem++)
    {
        // No.
        strTemp.Format(_T("%d"), nItemNum);
        m_SpecListChipInfo.InsertItem(nItemNum, strTemp);

        // Chip Type
        m_SpecListChipInfo.SetItemText(nItemNum, 1, _T("Passive"));

        // Spec Algorithm
        m_SpecListChipInfo.SetItemText(nItemNum, 2, vecDBPassiveSpec[nItem].strSpecName);
        nItemNum++;
    }

    strTemp.Empty();
}

void CDlgSpecDB::OnNMClickListChip(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    m_JobID = m_JobListChipInfo.GetSelectionMark();
    m_SpecID = m_SpecListChipInfo.GetSelectionMark();

    *pResult = 0;
}

void CDlgSpecDB::OnBnClickedChipInfoExport()
{
    if (m_JobID == -1)
    {
        ::SimpleMessage(_T("Please, Select Item."));
        return;
    }

    CString strType = m_JobListChipInfo.GetItemText(m_JobID, 1);
    CString strAlgorithm = m_JobListChipInfo.GetItemText(m_JobID, 2);

    long nType = -1;
    if (strType == _T("Chip"))
        nType = 0;
    else if (strType == _T("Passive"))
        nType = 1;
    else if (strType == _T("Array"))
        nType = 2;
    else if (strType == _T("Heatsink"))
        nType = 3;
    else
        return;

    if (strType.IsEmpty() == true || nType < 0)
        return;

    std::map<long, std::vector<AlgorithmSpec*>> mapJobSpec;
    for (long i = 0; i < (long)m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec.size(); i++)
    {
        mapJobSpec[nType].push_back((AlgorithmSpec*)&(m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec[i]));
    }

    std::map<long, std::vector<AlgorithmSpec*>> mapDBSpec;
    for (long i = 0; i < (long)m_ChipAlgoDBPara.m_vecPassiveAlgoSpec.size(); i++)
    {
        mapDBSpec[nType].push_back((AlgorithmSpec*)&(m_ChipAlgoDBPara.m_vecPassiveAlgoSpec[i]));
    }

    // Job에서 알고리즘 선택
    AlgorithmSpec* pSelChipSpec = NULL;
    for (long nJob = 0; nJob < (long)mapJobSpec[nType].size(); nJob++)
    {
        if (strAlgorithm.Compare(mapJobSpec[nType][nJob]->strSpecName) == 0)
        {
            pSelChipSpec = mapJobSpec[nType][nJob];
            break;
        }
    }

    if (pSelChipSpec == NULL)
    {
        ::SimpleMessage(_T("Please, Select Item."));
        return;
    }

    // 선택한 알고리즘을 DB에 적용.
    BOOL bSameName = FALSE;
    for (long nDB = 0; nDB < (long)mapDBSpec[nType].size(); nDB++)
    {
        if (pSelChipSpec->strSpecName.Compare(mapDBSpec[nType][nDB]->strSpecName) == 0)
        {
            bSameName = TRUE;

            CString strMessage;
            strMessage.Format(_T("[%s] Already exists. \n Do you want Copy?"), (LPCTSTR)pSelChipSpec->strSpecName);
            if (SimpleMessage(strMessage, MB_YESNO) != IDYES)
                return;

            switch (nType)
            {
                case 1:
                    m_ChipAlgoDBPara.m_vecPassiveAlgoSpec[nDB].CopyFrom(*(PassiveAlgorithmSpec*)pSelChipSpec);
                    break;
            }

            strMessage.Empty();
        }
    }

    if (!bSameName)
    {
        switch (nType)
        {
            case 1:
                m_ChipAlgoDBPara.m_vecPassiveAlgoSpec.push_back(*(PassiveAlgorithmSpec*)pSelChipSpec);
                break;
        }
    }

    ShowWidowList();

    strType.Empty();
    strAlgorithm.Empty();
}

void CDlgSpecDB::OnBnClickedChipDbImport() //mc_여기쪽 기능확인필요
{
    if (m_SpecID == -1)
    {
        ::SimpleMessage(_T("Please, Select Item."));
        return;
    }

    CString strType = m_SpecListChipInfo.GetItemText(m_SpecID, 1);
    CString strAlgorithm = m_SpecListChipInfo.GetItemText(m_SpecID, 2);

    long nType = -1;
    if (strType == _T("Chip"))
        nType = 0;
    else if (strType == _T("Passive"))
        nType = 1;
    else if (strType == _T("Array"))
        nType = 2;
    else if (strType == _T("Heatsink"))
        nType = 3;
    else
        return;

    if (strType.IsEmpty() == true || nType < 0)
        return;

    std::map<long, std::vector<AlgorithmSpec*>> mapJobSpec;
    for (long i = 0; i < (long)m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec.size(); i++)
        mapJobSpec[nType].push_back((AlgorithmSpec*)&(m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec[i]));

    std::map<long, std::vector<AlgorithmSpec*>> mapDBSpec;
    for (long i = 0; i < (long)m_ChipAlgoDBPara.m_vecPassiveAlgoSpec.size(); i++)
        mapDBSpec[nType].push_back((AlgorithmSpec*)&(m_ChipAlgoDBPara.m_vecPassiveAlgoSpec[i]));

    // DB에서 알고리즘 선택
    AlgorithmSpec* pSelChipSpec = NULL;
    for (long nDB = 0; nDB < (long)mapDBSpec[nType].size(); nDB++)
    {
        if (strAlgorithm.Compare(mapDBSpec[nType][nDB]->strSpecName) == 0)
        {
            pSelChipSpec = mapDBSpec[nType][nDB];
            break;
        }
    }

    if (pSelChipSpec == NULL)
    {
        ::SimpleMessage(_T("Please, Select Item."));
        return;
    }

    // 선택한 알고리즘을 Job에 적용.
    BOOL bSameName = FALSE;
    for (long nJob = 0; nJob < (long)mapJobSpec[nType].size(); nJob++)
    {
        if (pSelChipSpec->strSpecName.Compare(mapJobSpec[nType][nJob]->strSpecName) == 0)
        {
            bSameName = TRUE;

            CString strMessage;
            strMessage.Format(_T("[%s] Already exists. \n Do you want Copy?"), (LPCTSTR)pSelChipSpec->strSpecName);
            if (SimpleMessage(strMessage, MB_YESNO) != IDYES)
                return;

            switch (nType)
            {
                case 1:
                    m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec[nJob].CopyFrom(
                        *(PassiveAlgorithmSpec*)pSelChipSpec);
                    break;
            }

            strMessage.Empty();
        }
    }

    if (!bSameName)
    {
        switch (nType)
        {
            case 1:
                m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec.push_back(*(PassiveAlgorithmSpec*)pSelChipSpec);
                break;
        }
    }

    ShowWidowList();

    strType.Empty();
    strAlgorithm.Empty();
}

void CDlgSpecDB::OnBnClickedOk()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    SaveChipSpecDB();
    m_pChipVision->m_pVisionInspDlg->UpdateChipList(); // 갱신

    OnOK();
}

void CDlgSpecDB::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    LoadChipSpecDB();

    OnCancel();
}

void CDlgSpecDB::OnBnClickedBtnDelJob()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long nCurSelJob = m_JobListChipInfo.GetSelectionMark();
    if (nCurSelJob < 0)
        return;

    CString strType = m_JobListChipInfo.GetItemText(nCurSelJob, 1);
    CString strAlgorithm = m_JobListChipInfo.GetItemText(nCurSelJob, 2);

    long nType = -1;
    if (strType == _T("Chip"))
        nType = 0;
    else if (strType == _T("Passive"))
        nType = 1;
    else if (strType == _T("Array"))
        nType = 2;
    else if (strType = _T("Heatsink"))
        nType = 3;

    auto& vecJobPassiveSpec = m_pChipVision->m_VisionPara->m_vecPassiveAlgoSpec;

    switch (nType)
    {
        case 1: // Passive
            for (long i = 0; i < (long)vecJobPassiveSpec.size(); i++)
            {
                if (vecJobPassiveSpec[i].strSpecName == strAlgorithm)
                {
                    vecJobPassiveSpec.erase(i);
                    break;
                }
            }
            break;

        default:
            break;
    }

    ShowWidowList();

    strType.Empty();
    strAlgorithm.Empty();
}

void CDlgSpecDB::OnBnClickedBtnDelDb()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    long nCurSelDB = m_SpecListChipInfo.GetSelectionMark();
    if (nCurSelDB < 0)
        return;

    CString strType = m_SpecListChipInfo.GetItemText(nCurSelDB, 1);
    CString strAlgorithm = m_SpecListChipInfo.GetItemText(m_SpecID, 2);

    long nType = -1;
    if (strType == _T("Chip"))
        nType = 0;
    else if (strType == _T("Passive"))
        nType = 1;
    else if (strType == _T("Array"))
        nType = 2;
    else if (strType = _T("Heatsink"))
        nType = 3;

    auto& vecDBPassiveSpec = m_ChipAlgoDBPara.m_vecPassiveAlgoSpec;

    switch (nType)
    {
        case 1: // Passive
            for (long i = 0; i < (long)vecDBPassiveSpec.size(); i++)
            {
                if (vecDBPassiveSpec[i].strSpecName == strAlgorithm)
                {
                    vecDBPassiveSpec.erase(i);
                    break;
                }
            }
            break;

        default:
            break;
    }

    ShowWidowList();

    strType.Empty();
    strAlgorithm.Empty();
}
