#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DlgSubThresholdBase : public CDialogEx
{
    DECLARE_DYNAMIC(DlgSubThresholdBase)

public:
    DlgSubThresholdBase(UINT resourceID, CWnd* pParent); // standard constructor
    virtual ~DlgSubThresholdBase();

protected:
    void sendMessage_ThresholdChanged(long target);
    virtual long OnChangedScroll(CScrollBar* pScrollBar) = 0;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    virtual void OnOK()
    {
    }
    virtual void OnCancel()
    {
    }
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
