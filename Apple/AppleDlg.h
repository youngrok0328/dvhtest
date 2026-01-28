#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h" // 주 기호입니다.

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class SystemConfig;

//HDR_6_________________________________ Header body
//
class CAppleDlg : public CDialog
{
    // 생성입니다.

public:
    CAppleDlg(CWnd* pParent = NULL); // 표준 생성자입니다.
    ~CAppleDlg();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_Apple_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    // 구현입니다.

private:
    SystemConfig* m_pSysConfig;

protected:
    HICON m_hIcon;

    // 생성된 메시지 맵 함수
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()

public:
    void SetSystemConfig(void);
    void GetSystemConfig(void);
    void UpdateUI(const int& visionType);
    //{{ //kircheis_100M
    void UpdateUI_ControlCommon2D(BOOL bEnable, BOOL bCoaxialCal);
    void UpdateUI_Control2D(BOOL bEnable);
    void UpdateUI_Control3D(BOOL bEnable);
    //}}
    void UpdateUI_ControlNGRV(BOOL i_bEnable); // NGRV - 2021.01.21 : JHB
    void UpdateUI_ControlFrameGrabber(
        const int& nVisionType, const int frameGrabberType); //kircheis_FrameGrabber//kircheis_SWIR
    void UpdateUI_ControlNGRVOpticsType(const BOOL bEnable); 
    void UpdateUI_ControlSide(BOOL i_bEnable); 
    void UpdateUI_ControlDL(BOOL i_bEnable);

    BOOL m_bHardwareExist;
    int m_nVisionType;
    int m_frameGrabberType; //kircheis_FrameGrabber//kircheis_SWIR
    int m_n2DVisionCameraType;
    int m_n3DVisionCameraType;
    int m_n3DInspMethod;

    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedCheckUseAiInspection();
    afx_msg void OnEnChangeEditWaitTimeSendData();
    afx_msg void OnBnClickedCheckUseBayerPatternGPU();

    int m_nThreadNum;
    int m_nGrabBufferNum;
    int m_nLED_Duration;
    int m_nGRAB_Duration;
    int m_nCameraNumber;

    CComboBox m_cmbThreadNum;
    CComboBox m_cmbGrabBufferNum;
    CComboBox m_ctrlCmbHandlerType;

    long m_nWaitTImeToSendData;
    BOOL m_bUseLongfExposure;
    BOOL m_bUseBayerPattern;
    CComboBox m_cmbIR_Channel;
    CComboBox m_cmbUV_Channel;

    CEdit m_edit_Default_Channel_Gain;
    CEdit m_edit_IR_Channel_Gain;
    CEdit m_edit_UV_Channel_Gain;
    CComboBox m_ctrlCmbCoaxialCalibrationType;
    CComboBox m_ctrlCmbVisionType;
    CComboBox m_cmbSideVisionNumber;

    CComboBox m_cmb3DCameraType;
    CComboBox m_cmb3DCameraNumber;

    afx_msg void OnCbnSelchangeCmbVisionType();
    afx_msg void OnCbnSelchangeComSideVisionNumber();
    CComboBox m_cmbFrameGrabberType; //kircheis_SWIR
    afx_msg void OnCbnSelchangeCmbFrameGrabberType(); //kircheis_SWIR
    CComboBox m_cmbNgrvOpticsType;
    afx_msg void OnCbnSelchangeCmbNgrvOpticsType();
    CComboBox m_cmb2DCameraType;
    afx_msg void OnCbnSelchangeCmb2dvisionCameraType();
    afx_msg void OnCbnSelchangeCmb3dvisionCameraType();
};
