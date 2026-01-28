#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageLotView;
class ProcCommonGridBase;
class ProcCommonGridGroup_ClientView;

//HDR_6_________________________________ Header body
//
class DPI_PROCESSING_COMMON_DLG_API ProcCommonGridGroupDlg : public CDialog
{
    DECLARE_DYNAMIC(ProcCommonGridGroupDlg)

public:
    ProcCommonGridGroupDlg() = delete;
    ProcCommonGridGroupDlg(CWnd* pParent, const CRect& rtPositionOnParent); // standard constructor
    virtual ~ProcCommonGridGroupDlg();

    void add(ProcCommonGridBase* control);
    void event_afterInspection();
    void event_changedRoi();
    void event_changedPane();
    void event_changedRaw();
    void event_changedImage();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    DECLARE_MESSAGE_MAP()

private:
    const CRect m_rtInitPosition;
    CTabCtrl m_tabGrid;
    ProcCommonGridGroup_ClientView* m_tabClientWindow;
    long m_oldSelectGrid;

    std::vector<ProcCommonGridBase*> m_controls;
    std::vector<std::pair<ProcCommonGridBase*, long>> m_controls_view;

    afx_msg void OnTcnSelchangeTabGrid(NMHDR* pNMHDR, LRESULT* pResult);

public:
    afx_msg void OnDestroy();

    // Side Align2D에서 필요한 TabID Get & Set Function - 2024.06.11_JHB
    void SetGridTab(long i_nTabID);
    long GetGridTab();
    ////////
};
