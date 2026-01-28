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
class CNGRVDioView : public CDialog
{
    //DECLARE_DYNAMIC(CNGRVDioView)

public:
    CNGRVDioView(SyncController_Base* pciSync); // 표준 생성자입니다.
    virtual ~CNGRVDioView();

    // 대화 상자 데이터입니다.
    //#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_NGRV_DIO
    };
    //#endif

    // Dialog Data
    //{{AFX_DATA(CNGRVDioView)
    CSimpleLabel m_Label_NGRV_VisionReady;
    CSimpleLabel m_Label_NGRV_VisionAcquisition;
    CSimpleLabel m_Label_NGRV_VisionAcquisition_Fast;
    CSimpleLabel m_Label_NGRV_VisionRCV_PKG_ID;
    CSimpleLabel m_Label_NGRV_VisionRCV_Grab_ID;

    CSimpleLabel m_Label_NGRV_Handler_Start;
    CSimpleLabel m_Label_NGRV_Handler_Bit[10];
    CSimpleLabel m_Label_NGRV_HandlerSend_Grab_ID;

    CSimpleLabel m_Label_NGRV_Output_Raw_Bits[16];
    CSimpleLabel m_Label_NGRV_Input_Raw_Bits[16];

    CSimpleLabel m_Label_NGRV_Handler_Start_IR;
    CSimpleLabel m_Label_NGRV_Move_IR_Pos;

    CEdit m_Edit_Numerical_ID;
    //}}AFX_DATA

    SyncController_Base* m_pSync;

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CDioView)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    //}}AFX_VIRTUAL

protected:
    void GetSyncVersion();

    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnClose();
    afx_msg void OnStaticNGRV_VisionReady();
    afx_msg void OnStaticNGRV_VisionAcquisition();
    afx_msg void OnStaticNGRV_AcquisitionFast();
    afx_msg void OnStaticNGRV_VisionRCV_PGV_ID_Done();
    afx_msg void OnStaticNGRV_VisionRCV_Grab_ID_Done();
    afx_msg void OnStaticNGRV_Move_IR_Pos();

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
};
