//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ProcCommonGridGroupDlg.h"

//CPP_2_________________________________ This project's headers
#include "ProcCommonGridBase.h"
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define UM_UPATE_GRID (WM_USER + 1024)

//CPP_7_________________________________ Implementation body
//
class ProcCommonGridGroup_ClientView : public CStatic
{
public:
    DECLARE_DYNAMIC(ProcCommonGridGroup_ClientView)
    ProcCommonGridGroup_ClientView()
        : m_control(nullptr)
        , m_controlIndex(-1)
    {
    }

    ProcCommonGridBase* m_control;
    long m_controlIndex;

    DECLARE_MESSAGE_MAP()

    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnUpdateGrid(WPARAM, LPARAM);
};

IMPLEMENT_DYNAMIC(ProcCommonGridGroup_ClientView, CStatic)

BEGIN_MESSAGE_MAP(ProcCommonGridGroup_ClientView, CStatic)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_MESSAGE(UM_UPATE_GRID, OnUpdateGrid)
END_MESSAGE_MAP()

LRESULT ProcCommonGridGroup_ClientView::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (m_control)
    {
        if (m_control->notify(m_controlIndex, wparam, lparam))
        {
            PostMessage(UM_UPATE_GRID);
        }
    }

    return 0;
}

LRESULT ProcCommonGridGroup_ClientView::OnUpdateGrid(WPARAM, LPARAM)
{
    m_control->active(m_controlIndex);
    return 0;
}

//=================================================================================================

IMPLEMENT_DYNAMIC(ProcCommonGridGroupDlg, CDialog)

ProcCommonGridGroupDlg::ProcCommonGridGroupDlg(CWnd* pParent, const CRect& rtPositionOnParent)
    : CDialog(IDD_DIALOG_GRID_GROUP, pParent)
    , m_rtInitPosition(rtPositionOnParent)
    , m_oldSelectGrid(-1)
    , m_tabClientWindow(new ProcCommonGridGroup_ClientView)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    VERIFY(Create(IDD_DIALOG_GRID_GROUP, CWnd::FromHandle(pParent->GetSafeHwnd())));
}

ProcCommonGridGroupDlg::~ProcCommonGridGroupDlg()
{
    for (auto& control : m_controls)
    {
        delete control;
    }

    delete m_tabClientWindow;
}

void ProcCommonGridGroupDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_GRID, m_tabGrid);
}

BEGIN_MESSAGE_MAP(ProcCommonGridGroupDlg, CDialog)
ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_GRID, &ProcCommonGridGroupDlg::OnTcnSelchangeTabGrid)
ON_WM_DESTROY()
END_MESSAGE_MAP()

// ProcCommonGridGroupDlg message handlers

BOOL ProcCommonGridGroupDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    MoveWindow(m_rtInitPosition, FALSE);

    CRect rect;
    GetClientRect(rect);

    m_tabGrid.MoveWindow(rect);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void ProcCommonGridGroupDlg::add(ProcCommonGridBase* control)
{
    for (long gridIndex = 0; gridIndex < control->getGridCount(); gridIndex++)
    {
        m_tabGrid.InsertItem(m_tabGrid.GetItemCount(), control->getName(gridIndex));
    }

    if (m_tabClientWindow->GetSafeHwnd() == NULL)
    {
        CRect rect;
        m_tabGrid.GetClientRect(rect);
        m_tabGrid.AdjustRect(FALSE, rect);
        m_tabClientWindow->Create(_T(""), WS_CHILD | WS_VISIBLE, rect, &m_tabGrid);
    }

    CRect rect;
    m_tabClientWindow->GetClientRect(rect);

    for (long gridIndex = 0; gridIndex < control->getGridCount(); gridIndex++)
    {
        control->create(gridIndex, rect, m_tabClientWindow->GetSafeHwnd());
        m_controls_view.emplace_back(control, gridIndex);
    }

    m_controls.push_back(control);

    OnTcnSelchangeTabGrid(nullptr, nullptr);
}

void ProcCommonGridGroupDlg::event_afterInspection()
{
    long nTab = m_tabGrid.GetCurSel();

    if (nTab >= 0)
    {
        auto& view = m_controls_view[nTab];
        view.first->event_afterInspection(view.second);
    }
}

void ProcCommonGridGroupDlg::event_changedRoi()
{
    long nTab = m_tabGrid.GetCurSel();

    if (nTab >= 0)
    {
        auto& view = m_controls_view[nTab];
        view.first->event_changedRoi(view.second);
    }
}

void ProcCommonGridGroupDlg::event_changedPane()
{
    long nTab = m_tabGrid.GetCurSel();

    if (nTab >= 0)
    {
        auto& view = m_controls_view[nTab];
        view.first->event_changedPane(view.second);
    }
}

void ProcCommonGridGroupDlg::event_changedRaw()
{
    long nTab = m_tabGrid.GetCurSel();

    if (nTab >= 0)
    {
        auto& view = m_controls_view[nTab];
        view.first->event_changedRaw(view.second);
    }
}

void ProcCommonGridGroupDlg::event_changedImage()
{
    long nTab = m_tabGrid.GetCurSel();

    if (nTab >= 0)
    {
        auto& view = m_controls_view[nTab];
        view.first->event_changedImage(view.second);
    }
}

void ProcCommonGridGroupDlg::SetGridTab(long i_nTabID)
{
    m_tabGrid.SetCurSel(i_nTabID);

    OnTcnSelchangeTabGrid(nullptr, nullptr);
}

long ProcCommonGridGroupDlg::GetGridTab()
{
    return m_tabGrid.GetCurSel();
}

void ProcCommonGridGroupDlg::OnTcnSelchangeTabGrid(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    long nTab = m_tabGrid.GetCurSel();

    if (m_oldSelectGrid >= 0 && m_oldSelectGrid != nTab)
    {
        auto& view = m_controls_view[m_oldSelectGrid];
        view.first->deactivate(view.second);
    }

    m_oldSelectGrid = nTab;

    if (nTab >= 0)
    {
        auto& view = m_controls_view[nTab];
        m_tabClientWindow->m_control = view.first;
        m_tabClientWindow->m_controlIndex = view.second;
        view.first->active(view.second);
    }
    else
    {
        m_tabClientWindow->m_control = nullptr;
        m_tabClientWindow->m_controlIndex = -1;
    }

    if (pResult)
    {
        *pResult = 0;
    }
}

void ProcCommonGridGroupDlg::OnDestroy()
{
    CDialog::OnDestroy();

    m_tabClientWindow->m_control = nullptr;
}
