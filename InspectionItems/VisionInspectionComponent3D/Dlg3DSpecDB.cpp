//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Dlg3DSpecDB.h"

//CPP_2_________________________________ This project's headers
#include "ComponentAlign.h"
#include "DlgVisionInspectionComponent3D.h"
#include "VisionInspectionComponent3D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_DataBase/ChipInfoCollection.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CDlg3DSpecDB, CDialog)

CDlg3DSpecDB::CDlg3DSpecDB(VisionInspectionComponent3D* pChipAlign, CWnd* pParent /*=NULL*/)
    : CDialog(CDlg3DSpecDB::IDD, pParent)
{
    m_pChipVision = pChipAlign;
    m_SpecID = -1;
    m_JobID = -1;
}

CDlg3DSpecDB::~CDlg3DSpecDB()
{
}

void CDlg3DSpecDB::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_3DJOB_LISTCONTROL, m_JobListChipInfo);
    DDX_Control(pDX, IDC_3DSPEC_LISTCONTROL, m_SpecListChipInfo);
}

BEGIN_MESSAGE_MAP(CDlg3DSpecDB, CDialog)
ON_WM_SHOWWINDOW()
ON_BN_CLICKED(IDC_CHIP_INFO_EXPORT, &CDlg3DSpecDB::OnBnClickedChipInfoExport)
ON_BN_CLICKED(IDC_CHIP_DB_IMPORT, &CDlg3DSpecDB::OnBnClickedChipDbImport)
ON_WM_CONTEXTMENU()
ON_COMMAND(ID_MENU_DELET_3DSPECDB, &CDlg3DSpecDB::OnMenuDelete)
ON_NOTIFY(NM_CLICK, IDC_3DJOB_LISTCONTROL, &CDlg3DSpecDB::OnNMClickListChip)
ON_NOTIFY(NM_CLICK, IDC_3DSPEC_LISTCONTROL, &CDlg3DSpecDB::OnNMClickListChip)
END_MESSAGE_MAP()

// CDlg3DSpecDB 메시지 처리기입니다.
BOOL CDlg3DSpecDB::LoadChipSpecDB()
{
    CString strFilePath = DynamicSystemPath::get(DefineFolder::Job_ChipAlgo) + _T("ChipAlgoDB_3D.db");

    // Job File Open 한다.
    if (!m_ChipSpecDB.Load(strFilePath))
    {
        strFilePath.Empty();
        return FALSE;
    }

    strFilePath.Empty();

    m_pChipAlignPara.LinkDataBase(FALSE, m_ChipSpecDB);

    // 	for(long nNum=0; nNum < m_pChipAlignPara.m_vecComp3DAlgoSpec.size(); nNum++) // 자동으로 생성되는 "Default" spec은 쓰지 않음
    // 	{
    // 		if(m_pChipAlignPara.m_vecComp3DAlgoSpec[nNum].m_specName == "Default")
    // 			m_pChipAlignPara.m_vecComp3DAlgoSpec.pop_back();
    // 	}
    strFilePath.Empty();

    return TRUE;
}

BOOL CDlg3DSpecDB::SaveChipSpecDB()
{
    m_pChipAlignPara.LinkDataBase(TRUE, m_ChipSpecDB);

    CString strFilePath = DynamicSystemPath::get(DefineFolder::Job_ChipAlgo) + _T("ChipAlgoDB_3D.db");
    if (!m_ChipSpecDB.Save(strFilePath))
    {
        strFilePath.Empty();
        return FALSE;
    }

    strFilePath.Empty();
    return TRUE;
}

BOOL CDlg3DSpecDB::OnInitDialog()
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

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlg3DSpecDB::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialog::OnShowWindow(bShow, nStatus);

    ShowWidowList();
    if (bShow)
    {
    }
    else
    {
    }
}

void CDlg3DSpecDB::ShowWidowList()
{
    LoadChipSpecDB();

    CString strTemp;
    m_JobListChipInfo.DeleteAllItems();
    m_SpecListChipInfo.DeleteAllItems();

    long nItemNum = 0;
    long nJobChipAlgorithmSize = (long)m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.size();
    long nSpecChipAlgorithmSize = (long)m_pChipAlignPara.m_vecComp3DAlgoSpec.size();

    for (long nItem = 0; nItem < nJobChipAlgorithmSize; nItem++)
    {
        // No.
        strTemp.Format(_T("%d"), nItemNum);
        m_JobListChipInfo.InsertItem(nItemNum, strTemp);

        // Chip Type
        m_JobListChipInfo.SetItemText(nItemNum, 1, _T("Comp"));

        // Spec Algorithm
        m_JobListChipInfo.SetItemText(nItemNum, 2, m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nItem].strSpecName);
        nItemNum++;
    }
    nItemNum = 0;
    for (long nItem = 0; nItem < nSpecChipAlgorithmSize; nItem++)
    {
        // No.
        strTemp.Format(_T("%d"), nItemNum);
        m_SpecListChipInfo.InsertItem(nItemNum, strTemp);

        // Chip Type
        m_SpecListChipInfo.SetItemText(nItemNum, 1, _T("Comp"));

        // Spec Algorithm
        m_SpecListChipInfo.SetItemText(nItemNum, 2, m_pChipAlignPara.m_vecComp3DAlgoSpec[nItem].strSpecName);
        nItemNum++;
    }
    strTemp.Empty();
}
void CDlg3DSpecDB::OnBnClickedChipInfoExport()
{
    if (m_JobID == -1)
    {
        ::SimpleMessage(_T("Please, Select Item."));
    }
    else
    {
        CString strJobType = m_JobListChipInfo.GetItemText(m_JobID, 1);
        CString strJobAlgorithm = m_JobListChipInfo.GetItemText(m_JobID, 2);
        long nJobChipSize = (long)m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.size();
        long nSpecChipSize = (long)m_pChipAlignPara.m_vecComp3DAlgoSpec.size();
        CString strMessage;

        if (strJobAlgorithm == "Default") // 저장시 자동으로 생성되는 default로 인한 예외처리..
        {
            for (long nSpecNum = 0; nSpecNum < nSpecChipSize; nSpecNum++)
            {
                if (m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum].strSpecName == "default")
                {
                    strMessage.Format(_T("[%s] Already exists. \n Do you want Copy?"), (LPCTSTR)strJobAlgorithm);
                    if (SimpleMessage(strMessage, MB_YESNO) == IDYES)
                    {
                        for (long nJobNum = 0; nJobNum < nJobChipSize; nJobNum++)
                        {
                            if (m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum].strSpecName == "Default")
                            {
                                m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum]
                                    = m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum];
                                m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum].strSpecName = "default";

                                SaveChipSpecDB();
                                return;
                            }
                        }
                    }
                    else
                        return;
                }
            }
        }
        else
        {
            for (long nSpecNum = 0; nSpecNum < nSpecChipSize;
                nSpecNum++) // 만약 같은 이름의 spec이 이미 있으면 덮어쓰기
            {
                if (strJobAlgorithm == m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum].strSpecName)
                {
                    strMessage.Format(_T("[%s] Already exists. \n Do you want Copy?"), (LPCTSTR)strJobAlgorithm);
                    if (SimpleMessage(strMessage, MB_YESNO) == IDYES)
                    {
                        for (long nJobNum = 0; nJobNum < nJobChipSize; nJobNum++)
                        {
                            if (strJobAlgorithm == m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum].strSpecName
                                && strJobAlgorithm
                                    == m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum].strSpecName)
                            {
                                m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum]
                                    = m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum];
                                SaveChipSpecDB();

                                return;
                            }
                        }
                    }
                    else
                        return;
                }
            }
        }

        for (long nJobNum = 0; nJobNum < nJobChipSize;
            nJobNum++) // 위에서 (복사 및 예외처리) return되지 않았으면 추가등록
        {
            if (strJobAlgorithm == m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum].strSpecName)
            {
                m_pChipAlignPara.m_vecComp3DAlgoSpec.push_back(
                    m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum]);
                if (strJobAlgorithm == "Default") // default도 spec에 등록되도록 하기위한 예외처리
                {
                    long nAlgoSize = (long)m_pChipAlignPara.m_vecComp3DAlgoSpec.size();
                    m_pChipAlignPara.m_vecComp3DAlgoSpec[nAlgoSize - 1].strSpecName = "default";
                }

                SaveChipSpecDB();
                break;
            }
        }

        strJobType.Empty();
        strJobAlgorithm.Empty();
        strMessage.Empty();
    }
    ShowWidowList();
}

void CDlg3DSpecDB::OnBnClickedChipDbImport()
{
    if (m_SpecID == -1)
    {
        ::SimpleMessage(_T("Please, Select Item."));
    }
    else
    {
        CString strSpecType = m_SpecListChipInfo.GetItemText(m_SpecID, 1);
        CString strSpecAlgorithm = m_SpecListChipInfo.GetItemText(m_SpecID, 2);
        long nJobChipSize = (long)m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.size();
        long nSpecChipSize = (long)m_pChipAlignPara.m_vecComp3DAlgoSpec.size();

        CString strMessage;

        if (strSpecAlgorithm == "default")
        {
            strMessage.Format(_T("[%s] Already exists. \n Do you want Copy?"), (LPCTSTR)strSpecAlgorithm);
            if (SimpleMessage(strMessage, MB_YESNO) == IDYES)
            {
                for (long nJobNum = 0; nJobNum < nJobChipSize; nJobNum++)
                {
                    for (long nSpecNum = 0; nSpecNum < nSpecChipSize; nSpecNum++)
                    {
                        if (m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum].strSpecName == "default"
                            && m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum].strSpecName == "Default")
                        {
                            m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum]
                                = m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum];
                            m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum].strSpecName = "Default";
                            return;
                        }
                    }
                }
            }
            else
                return;
        }
        else
        {
            for (long nJobNum = 0; nJobNum < nJobChipSize; nJobNum++)
            {
                if (strSpecAlgorithm == m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum].strSpecName)
                {
                    strMessage.Format(_T("[%s] Already exists. \n Do you want Copy?"), (LPCTSTR)strSpecAlgorithm);
                    if (SimpleMessage(strMessage, MB_YESNO) == IDYES)
                    {
                        for (long nSpecNum = 0; nSpecNum < nSpecChipSize; nSpecNum++)
                        {
                            if (strSpecAlgorithm == m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum].strSpecName
                                && strSpecAlgorithm
                                    == m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum].strSpecName)
                            {
                                m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nJobNum]
                                    = m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum];
                                return;
                            }
                        }
                    }
                    else
                        return;
                }
            }
        }

        for (long nSpecNum = 0; nSpecNum < nSpecChipSize; nSpecNum++)
        {
            if (strSpecAlgorithm == m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum].strSpecName)
            {
                m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.push_back(
                    m_pChipAlignPara.m_vecComp3DAlgoSpec[nSpecNum]);

                break;
            }
        }

        strSpecType.Empty();
        strSpecAlgorithm.Empty();
        strMessage.Empty();
    }
    ShowWidowList();
}

void CDlg3DSpecDB::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    m_JobID = -1;
    m_SpecID = -1; // 초기화

    m_JobID = m_JobListChipInfo.GetSelectionMark();
    m_SpecID = m_SpecListChipInfo.GetSelectionMark();

    CMenu MenuItem;
    if (!MenuItem.LoadMenu(IDR_MENU_3DSPECDB))
        return;
    CMenu* pMenuSub = MenuItem.GetSubMenu(0);
    pMenuSub->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, this);
}
void CDlg3DSpecDB::OnNMClickListChip(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    m_JobID = -1;
    m_SpecID = -1; // 초기화

    m_JobID = m_JobListChipInfo.GetSelectionMark();
    m_SpecID = m_SpecListChipInfo.GetSelectionMark();

    *pResult = 0;
}
void CDlg3DSpecDB::OnMenuDelete()
{
    if (m_SpecID >= 0)
    {
        CString strSpecType = m_SpecListChipInfo.GetItemText(m_SpecID, 1);
        CString strSpecAlgorithm = m_SpecListChipInfo.GetItemText(m_SpecID, 2);

        long nSpecChipSize = (long)m_pChipAlignPara.m_vecComp3DAlgoSpec.size();

        for (long nNum = 0; nNum < nSpecChipSize; nNum++)
        {
            if (m_pChipAlignPara.m_vecComp3DAlgoSpec[nNum].strSpecName == strSpecAlgorithm)
            {
                m_pChipAlignPara.m_vecComp3DAlgoSpec.erase(m_pChipAlignPara.m_vecComp3DAlgoSpec.begin() + nNum);
                break;
            }
        }

        SaveChipSpecDB();
        ShowWidowList();

        strSpecType.Empty();
        strSpecAlgorithm.Empty();
    }
    else if (m_JobID >= 0) //job parameter algorithm을 지워야함.
    {
        CString strJobType = m_JobListChipInfo.GetItemText(m_JobID, 1);
        CString strJobAlgorithm = m_JobListChipInfo.GetItemText(m_JobID, 2);

        if (strJobAlgorithm == "Default")
            return;

        long nJobChipAlgorithmSize = (long)m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.size();

        for (long nNum = 0; nNum < nJobChipAlgorithmSize; nNum++)
        {
            if (m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nNum].strSpecName == strJobAlgorithm)
            {
                m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.erase(
                    m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.begin() + nNum);
                break;
            }
        }

        ShowWidowList();

        long nChipSize = (long)(m_pChipVision->m_PackageInfo->vecChips.size());
        nJobChipAlgorithmSize = (long)m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec.size();

        CString strSpecName;
        BOOL bOverlap = FALSE;
        for (long nNum = 0; nNum < nChipSize; nNum++) // 삭제한 specname을 갖고 있는 comp은 'default'로 갱신
        {
            bOverlap = FALSE;

            strSpecName = m_pChipVision->m_PackageInfo->vecChips[nNum].strSpecName;
            for (long nChip = 0; nChip < nJobChipAlgorithmSize; nChip++)
            {
                if (strSpecName == m_pChipVision->m_VisionPara->m_vecComp3DAlgoSpec[nChip].strSpecName)
                {
                    bOverlap = TRUE;
                    break;
                }
            }
            if (bOverlap == FALSE) // 중복되지않는 이름(지운이름) default로.
                m_pChipVision->m_PackageInfo->vecChips[nNum].strSpecName = "Default";
        }

        //m_pChipVision->m_pVisionInspDlg->m_pVisionInspParaDlg->UpdateChipList(); // 갱신
        m_pChipVision->m_pVisionInspDlg->UpdateChipList(); // 갱신

        strJobType.Empty();
        strJobAlgorithm.Empty();
        strSpecName.Empty();
    }
}