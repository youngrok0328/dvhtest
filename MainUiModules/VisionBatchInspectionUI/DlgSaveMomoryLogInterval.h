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
class CDlgSaveMomoryLogInterval : public CDialog
{
    DECLARE_DYNAMIC(CDlgSaveMomoryLogInterval)

public:
    CDlgSaveMomoryLogInterval(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgSaveMomoryLogInterval();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_SAVE_MEMORY_LOG_TIME_INTERVAL_SETUP
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    CString m_strSaveMemoryLogInterval;
};
