#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CDlgHistogramView : public CDialog
{
    DECLARE_DYNAMIC(CDlgHistogramView)

public:
    CDlgHistogramView(CWnd* pParent = NULL);
    virtual ~CDlgHistogramView();

    void DrawGraph(CString strTitle, std::vector<long> vecnHisto);

    enum
    {
        IDD = IDD_DIALOG_HISTOGRAM
    };

protected:
    Ipvm::ProfileView* m_profileView;
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
