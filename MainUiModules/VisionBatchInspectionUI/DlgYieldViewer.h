#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CDlgYieldViewer : public CDialog
{
    DECLARE_DYNAMIC(CDlgYieldViewer)

public:
    CDlgYieldViewer(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgYieldViewer();

    BOOL m_saveRNR;
    CGridCtrl m_GridList;
    std::vector<stBatchInspResult> m_vecstBatchInspResult;

public:
    void OnGridSetup();
    void SetVisionInspResult(const std::vector<stBatchInspResult>& vecstBatchInspResult);

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_YIELD_VIEWER
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnOK()
    {
    }
    afx_msg void OnCancel()
    {
    }
    afx_msg void OnBnClickedOk();
};
