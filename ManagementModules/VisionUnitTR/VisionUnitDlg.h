#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "MenuBar.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class VisionUnit;
class VisionProcessing;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class VisionUnitDlg : public CDialog
{
    DECLARE_DYNAMIC(VisionUnitDlg)

public:
    VisionUnitDlg(VisionUnit& visionUnit, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~VisionUnitDlg();

    void updateProcessorResult();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_VISION_UNIT
    };

protected:
    MenuBar m_processing;
    CButton m_btnRollback;
    VisionUnit& m_visionUnit;
    ProcessingDlgInfo* m_procDlgInfo;
    CiDataBase m_dbProcessBackup;
    CRect m_clientWindow;

    void loadProcessor(VisionProcessing* proc);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnDestroy();
    afx_msg LRESULT OnSelectionChangedProcess(WPARAM, LPARAM);
    afx_msg void OnClickedButtonRollback();

    CTabCtrl m_CtrlTabSwitchAceesMode;
    afx_msg void OnTcnSelchangeSwitchAceesMode(NMHDR* pNMHDR, LRESULT* pResult);
};
