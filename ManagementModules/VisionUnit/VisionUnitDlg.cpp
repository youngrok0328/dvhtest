//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionUnitDlg.h"

//CPP_2_________________________________ This project's headers
#include "VisionUnit.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMark.h"
#include "../../InspectionItems/VisionInspectionMark/VisionInspectionMarkPara.h"
#include "../../ProcessingItems/VisionProcessingFOV/VisionProcessingFOV.h"
#include "../../ProcessingItems/VisionProcessingSpec/VisionProcessingSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Widget/AsyncProgress.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_LIST 100
#define IDC_BUTTON_ROLLBACK 120
#define IDC_TabCtrl 130

#define UM_SELECTION_CHANGED_PROCESS (WM_USER + 999)

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(VisionUnitDlg, CDialog)

VisionUnitDlg::VisionUnitDlg(VisionUnit& visionUnit, CWnd* pParent /*=NULL*/)
    : CDialog(VisionUnitDlg::IDD, pParent)
    , m_visionUnit(visionUnit)
    , m_procDlgInfo(new ProcessingDlgInfo)
{
}

VisionUnitDlg::~VisionUnitDlg()
{
    delete m_procDlgInfo;
}

void VisionUnitDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(VisionUnitDlg, CDialog)
ON_WM_DESTROY()
ON_MESSAGE(UM_SELECTION_CHANGED_PROCESS, OnSelectionChangedProcess)
ON_BN_CLICKED(IDC_BUTTON_ROLLBACK, OnClickedButtonRollback)
ON_NOTIFY(TCN_SELCHANGE, IDC_TabCtrl, OnTcnSelchangeSwitchAceesMode)
END_MESSAGE_MAP()

// VisionUnitDlg 메시지 처리기입니다.
LRESULT VisionUnitDlg::OnSelectionChangedProcess(WPARAM, LPARAM)
{
    auto processIndex = m_processing.GetActiveItemID();
    if (processIndex < 0)
        return 0;

    VisionProcessing* newProc = (VisionProcessing*)(m_visionUnit.m_vecVisionModules[processIndex]);

    if (m_visionUnit.m_currentProcessingModule == newProc)
    {
        return 0;
    }

    loadProcessor(newProc);
    updateProcessorResult();

    return 0;
}

void VisionUnitDlg::OnClickedButtonRollback()
{
    if (m_visionUnit.m_currentProcessingModule == nullptr)
        return;

    if (MessageBox(_T("Do you want to restore your previous setting?"), _T("Message"), MB_YESNO) != IDYES)
        return;

    if (m_visionUnit.m_currentProcessingModule)
    {
        m_visionUnit.m_currentProcessingModule->CloseDlg();
    }

    m_visionUnit.m_currentProcessingModule->LinkDataBase(FALSE, m_dbProcessBackup);
    loadProcessor(m_visionUnit.m_currentProcessingModule);
    updateProcessorResult();
}

void VisionUnitDlg::OnTcnSelchangeSwitchAceesMode(NMHDR* pNMHDR, LRESULT* pResult)
{
    if (pNMHDR == NULL)
        return;

    //mc_여기를 진입할 수 있다는건 Mark Inspection에서 EngineerMode 상태에서 선택하였다는거 Evnet는 무조건 받아준다
    auto* pMarkproc = dynamic_cast<VisionInspectionMark*>(m_visionUnit.m_currentProcessingModule);
    if (pMarkproc != nullptr)
        pMarkproc->SetSwitchMarkParameterView(m_CtrlTabSwitchAceesMode.GetCurSel());

    *pResult = 0;
}

BOOL VisionUnitDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtParentClient;
    GetParent()->GetClientRect(rtParentClient);

    MoveWindow(0, 0, rtParentClient.Width(), rtParentClient.Height(), FALSE);

    GetClientRect(m_clientWindow);
    m_clientWindow.DeflateRect(5, 5);

    m_btnRollback.Create(_T("Rollback"), WS_CHILD | WS_VISIBLE,
        ::GetContentSubArea(m_clientWindow, 20, 20 + 28, 100, 0, 3, 100), this, IDC_BUTTON_ROLLBACK);
    m_CtrlTabSwitchAceesMode.Create(TCS_TABS | WS_CHILD | WS_VISIBLE,
        ::GetContentSubArea(m_clientWindow, 20, 20 + 28, 100, 3, 6, 100), this, IDC_TabCtrl);
    ////////////////////////////////////////////////////////////////////////////////////////
    m_processing.Create(_T(""), WS_CHILD | WS_VISIBLE, ::GetContentSubArea(m_clientWindow, 0, 20, 100, 0, 65, 100),
        this, IDC_GRID_LIST);

    CString strFontName = _T("Arial");
    CFont fontGrid;
    fontGrid.CreateFont(20, 0, 0, 0, FW_BOLD, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS,
        DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, strFontName);

    m_processing.SetFont(&fontGrid);
    m_processing.SetTitle(_T("[Processing sequence]"));
    m_processing.SetSelectionChangedEvent(GetSafeHwnd(), UM_SELECTION_CHANGED_PROCESS);

    bool firstProc = true;
    bool itemEnables = false;

    for (int procIndex = 0; procIndex < long(m_visionUnit.m_vecVisionModules.size()); procIndex++)
    {
        itemEnables = true;
        auto* proc = m_visionUnit.m_vecVisionModules[procIndex];
        if (m_visionUnit.m_systemConfig.m_nCurrentAccessMode == _OPERATOR)
        {
            itemEnables = false;
            if (proc->m_strModuleName.Find(_T("Mark_")) == 0)
            {
                if (auto* markProc = dynamic_cast<VisionInspectionMark*>(proc))
                {
                    if (markProc->m_VisionPara->m_nMarkInspMode == 0)
                    {
                        itemEnables = true;
                    }
                }
            }
        }

        if (proc != nullptr)
            m_processing.AddItem(procIndex, proc->m_strModuleName, itemEnables);

        if (firstProc && itemEnables == true)
        {
            m_processing.SetActiveItem(procIndex);
            firstProc = false;
        }
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////

    if (SystemConfig::GetInstance().GetVisionType() != VISIONTYPE_SIDE_INSP)
    {
        m_procDlgInfo->m_hwndParent = GetSafeHwnd();
        m_procDlgInfo->m_rtSelectionArea = ::GetContentSubArea(m_clientWindow, 0, 20, 100, 0, 65, 100);
        m_procDlgInfo->m_rtDataArea = ::GetContentSubArea(m_clientWindow, 0, 20 + 28, 100, 65, 100, 100);
        m_procDlgInfo->m_rtImageArea = ::GetContentSubArea(m_clientWindow, 20 + 28, 100, 100, 0, 100, 100);
    }
    else
    {
        m_procDlgInfo->m_hwndParent = GetSafeHwnd();
        m_procDlgInfo->m_rtSelectionArea = ::GetContentSubArea(m_clientWindow, 0, 20, 100, 0, 65, 100);
        m_procDlgInfo->m_rtDataArea = ::GetContentSubArea(m_clientWindow, 0, 20 + 28, 100, 68, 100, 100);
        m_procDlgInfo->m_rtImageArea = ::GetContentSubArea(m_clientWindow, 20 + 28, 100, 100, 0, 100, 100);

        m_procDlgInfo->m_rtSideFrontStatusArea = ::GetContentSubArea(m_clientWindow, 0, 10, 100, 65, 68, 100);
        m_procDlgInfo->m_rtSideRearStatusArea = ::GetContentSubArea(m_clientWindow, 10, 20, 100, 65, 68, 100);
    }

    auto startProcessor = m_visionUnit.m_currentProcessingModule;
    if (startProcessor == nullptr && m_processing.GetActiveItemID() >= 0)
    {
        startProcessor = m_visionUnit.m_vecVisionModules[m_processing.GetActiveItemID()];
    }

    loadProcessor(startProcessor);
    updateProcessorResult();

    return TRUE; // return TRUE  unless you set the focus to a control
}

void VisionUnitDlg::OnDestroy()
{
    if (m_visionUnit.m_currentProcessingModule)
    {
        m_visionUnit.m_currentProcessingModule->CloseDlg();

        m_visionUnit.m_currentProcessingModule = nullptr;
    }

    CDialog::OnDestroy();
}

void VisionUnitDlg::updateProcessorResult()
{
    for (int procIndex = 0; procIndex < long(m_visionUnit.m_vecVisionModules.size()); procIndex++)
    {
        auto* proc = m_visionUnit.m_vecVisionModules[procIndex];

        if (!m_processing.IsExistItem(procIndex))
            continue;
        auto& state = proc->GetResultState();
        m_processing.SetItemStateNum(procIndex, state.Count());

        for (long stateIndex = 0; stateIndex < state.Count(); stateIndex++)
        {
            m_processing.SetItemState(procIndex, stateIndex, state.GetSate(stateIndex));
        }
    }
}

void VisionUnitDlg::loadProcessor(VisionProcessing* proc)
{
    static const bool hwExist = (bool)SystemConfig::GetInstance().m_bHardwareExist;

    if (proc == nullptr)
        return;

    proc->SetCurVisionModule_Status(enSideVisionModule(m_visionUnit.GetSideVisionSection()));

    Ipvm::AsyncProgress* pProgress
        = nullptr; //디버그할 때 프로그레스 바때문에 락되는 일이 많아 일단 이렇게 처리는 하는데 장비에서는 문제가 없는지 확인 필요

    if (hwExist == true)
        pProgress = new Ipvm::AsyncProgress(_T("Loading..."));

    if (m_visionUnit.m_currentProcessingModule)
    {
        if (hwExist)
            pProgress->Post(_T("Closing..."));
        m_visionUnit.m_currentProcessingModule->CloseDlg();
    }

    if (hwExist)
        pProgress->Post(_T("Loading..."));

    m_visionUnit.m_currentProcessingModule = proc;

    m_dbProcessBackup.Reset();
    if (m_visionUnit.m_currentProcessingModule)
    {
        m_visionUnit.m_currentProcessingModule->LinkDataBase(TRUE, m_dbProcessBackup);
    }

    if (dynamic_cast<VisionProcessingSpec*>(proc) != nullptr || dynamic_cast<VisionProcessingFOV*>(proc) != nullptr)
    {
        // 다음의 모듈은 자기자신의 spec을 바꾸는 것이 아니라 전역에서 쓰일 정보를
        // 수정하는 모듈이라 롤백이 불가능하다

        m_btnRollback.EnableWindow(FALSE);
        m_btnRollback.ShowWindow(SW_HIDE);

        m_procDlgInfo->m_rtParaArea = ::GetContentSubArea(m_clientWindow, 20, 20 + 28, 100, 0, 65, 100);
    }
    else
    {
        m_btnRollback.EnableWindow(TRUE);
        m_btnRollback.ShowWindow(SW_SHOW);

        m_procDlgInfo->m_rtParaArea = ::GetContentSubArea(m_clientWindow, 20, 20 + 28, 100, 3, 65, 100);
    }

    if (dynamic_cast<VisionInspectionMark*>(proc)
        != nullptr) //mc_Mark Dll일경우 Parameter창을 약간 줄인다. Tab Ctrl이 들어가야하기 때문에..
    {
        // Tab Control 의 크기를 얻어서 그 아래만큼 크기를 줄인다
        CRect accessModeTab;
        m_CtrlTabSwitchAceesMode.GetWindowRect(accessModeTab);
        ScreenToClient(accessModeTab);

        long nLeft = m_procDlgInfo->m_rtParaArea.left;
        long nTop = accessModeTab.bottom;
        long nRight = m_procDlgInfo->m_rtParaArea.right;
        long nBottom = m_procDlgInfo->m_rtParaArea.bottom;

        m_procDlgInfo->m_rtParaArea.SetRect(nLeft, nTop, nRight, nBottom);

        m_CtrlTabSwitchAceesMode.DeleteAllItems();
        m_CtrlTabSwitchAceesMode.InsertItem(_OPERATOR, _T("Operator View"));
        m_CtrlTabSwitchAceesMode.InsertItem(_ENGINEER, _T("Engineer View"));

        if (m_visionUnit.m_systemConfig.m_nCurrentAccessMode
            == _OPERATOR) //Operator면 View는 하게 해줘도 선택 못하게 한다
        {
            m_CtrlTabSwitchAceesMode.SetCurSel(_OPERATOR);
            m_CtrlTabSwitchAceesMode.EnableWindow(FALSE);
        }
        else
        {
            m_CtrlTabSwitchAceesMode.SetCurSel(_ENGINEER);
            m_CtrlTabSwitchAceesMode.EnableWindow(TRUE);
        }

        m_CtrlTabSwitchAceesMode.ShowWindow(SW_SHOW);
    }
    else
    {
        //이외는 보여줄 필요없다
        m_CtrlTabSwitchAceesMode.EnableWindow(FALSE);
        m_CtrlTabSwitchAceesMode.ShowWindow(SW_HIDE);
    }

    if (m_visionUnit.m_currentProcessingModule)
    {
        m_visionUnit.m_currentProcessingModule->ShowDlg(*m_procDlgInfo);
    }

    if (pProgress != nullptr)
        delete pProgress;
}
