#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "DlgSubThresholdBase.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DlgSubThresholdNormal : public DlgSubThresholdBase
{
    DECLARE_DYNAMIC(DlgSubThresholdNormal)

public:
    DlgSubThresholdNormal(
        int nUse3DImageType, CWnd* pParent = nullptr); // standard constructor //SDY_3D Surface VMap UI 오류 수정
    virtual ~DlgSubThresholdNormal();

    void Set(long& low, long& high);

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_SUB_THRESHOLD_NORMAL
    };
#endif

protected:
    bool m_sbarInitialize;

    long* m_low;
    long* m_high;

    int m_nUse3DImageType; //SDY_3D Surface VMap UI 오류 수정

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    virtual long OnChangedScroll(CScrollBar* pScrollBar) override;

    DECLARE_MESSAGE_MAP()

public:
    CScrollBar m_sbarDark;
    CScrollBar m_sbarBright;
    virtual BOOL OnInitDialog();

    void OnEnChangeThreshold(long target);

    afx_msg void OnEnChangeThresholdDark();
    afx_msg void OnEnChangeThresholdBright();
};
