#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SimpleLabel.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class SyncController_Base;

//HDR_6_________________________________ Header body
//
class CDioView : public CDialog
{
    // Construction

public:
    CDioView(SyncController_Base* pciSync); // standard constructor
    virtual ~CDioView();

    // Dialog Data
    //{{AFX_DATA(CDioView)
    enum
    {
        IDD = IDD_DIALOG_DIO
    };
    CSimpleLabel m_Label_VisionReady;
    CSimpleLabel m_Label_VisionAcquisition;
    CSimpleLabel m_Label_VisionExposure;
    CSimpleLabel m_Label_HandlerStart;
    CSimpleLabel m_Label_HandlerBit[10];
    CSimpleLabel m_Label_HandlerPocketID;
    CSimpleLabel m_Label_HandlerEncoderCnt;
    CSimpleLabel m_Label_OutputRawBits[16];
    CSimpleLabel m_Label_InputRawBits[16];
    CButton m_checkRepeatReady;
    CButton m_checkRepeatAcquisition;
    CButton m_checkRepeatExposure;

    //}}AFX_DATA

    SyncController_Base* m_pSync;

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDioView)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    void InvertOutput(long nBitOrder);
    void GetSyncVersion();

    // Generated message map functions
    //{{AFX_MSG(CDioView)
    afx_msg void OnStaticVisionAcquisition();
    afx_msg void OnStaticVisionExposure();
    afx_msg void OnStaticVisionReady();
    afx_msg void OnStaticOutput_0();
    afx_msg void OnStaticOutput_1();
    afx_msg void OnStaticOutput_2();
    afx_msg void OnStaticOutput_3();
    afx_msg void OnStaticOutput_4();
    afx_msg void OnStaticOutput_5();
    afx_msg void OnStaticOutput_6();
    afx_msg void OnStaticOutput_7();
    afx_msg void OnStaticOutput_8();
    afx_msg void OnStaticOutput_9();
    afx_msg void OnStaticOutput_A();
    afx_msg void OnStaticOutput_B();
    afx_msg void OnStaticOutput_C();
    afx_msg void OnStaticOutput_D();
    afx_msg void OnStaticOutput_E();
    afx_msg void OnStaticOutput_F();

    virtual BOOL OnInitDialog();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedCheckRepeatReady();
    afx_msg void OnBnClickedCheckRepeatAcquisition();
    afx_msg void OnBnClickedCheckRepeatExposure();
};