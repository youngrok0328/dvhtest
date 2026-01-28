#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DlgSystemSetup : public CDialog
{
    DECLARE_DYNAMIC(DlgSystemSetup)

public:
    DlgSystemSetup(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~DlgSystemSetup();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_SYSTEM_SETUP
    };

    bool m_needReboot;

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedChkSaveReviewImage();
    afx_msg void OnBnClickedBtnSysteminiRefresh();
    CComboBox m_cmbImageSampling;

private:
    void Set_ReadOnly_CPU_Calc_Log_Param(const bool i_bReadOnly);

public:
    afx_msg void OnBnClickedChkEnableCpuCalcLog();
    afx_msg void OnBnClickedChkSaveMvRejectData();
    afx_msg void OnBnClickedBtnApplyCamGain();
};
