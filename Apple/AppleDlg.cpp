//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "AppleDlg.h"

//CPP_2_________________________________ This project's headers
#include "dlgPassword.h"

//CPP_3_________________________________ Other projects' headers
#include "../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../DefineModules/dA_Base/semiinfo.h"
#include "../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CAppleDlg::CAppleDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CAppleDlg::IDD, pParent)
    , m_pSysConfig(new SystemConfig())
    , m_bHardwareExist(FALSE)
    , m_nThreadNum(0)
    , m_nGrabBufferNum(0)
    , m_nLED_Duration(4)
    , m_nGRAB_Duration(10)
    , m_nWaitTImeToSendData(2000)
    , m_bUseLongfExposure(TRUE)
    , m_bUseBayerPattern(TRUE)
    , m_frameGrabberType(enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES) //kircheis_SWIR
    , m_nCameraNumber(1)
    , m_n2DVisionCameraType(CAMERATYPE_LAON_PEOPLE_CXP25M)
    , m_n3DVisionCameraType(CAMERATYPE_VIEWWORKS_12MX)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CAppleDlg::~CAppleDlg()
{
    delete m_pSysConfig;
}

void CAppleDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_HARDWARE_EXIST, m_bHardwareExist);
    DDX_Text(pDX, IDC_EDIT_LED_DURATION, m_nLED_Duration);
    DDX_Text(pDX, IDC_EDIT_GRAB_DURATION, m_nGRAB_Duration);
    DDX_Control(pDX, IDC_CMB_THREAD_NUMBER, m_cmbThreadNum);
    DDX_Control(pDX, IDC_CMB_GRAB_BUFFER_NUMBER, m_cmbGrabBufferNum);

    DDX_Control(pDX, IDC_CMB_HANDLER_TYPE, m_ctrlCmbHandlerType);
    DDX_Text(pDX, IDC_EDIT_WAIT_TIME_SEND_DATA, m_nWaitTImeToSendData);
    DDX_Check(pDX, IDC_CHECK_USE_LONG_EXPOSURE, m_bUseLongfExposure);
    DDX_Check(pDX, IDC_CHECK_USE_BAYER_PATTERN_GPU, m_bUseBayerPattern);
    DDX_Control(pDX, IDC_COMBO_IR_CHANNEL_ID, m_cmbIR_Channel);
    DDX_Control(pDX, IDC_COMBO_UV_CHANNEL_ID, m_cmbUV_Channel);
    DDX_Control(pDX, IDC_EDIT_DEFAULT_CHANNEL_GAIN_VALUE, m_edit_Default_Channel_Gain);
    DDX_Control(pDX, IDC_EDIT_IR_CHANNEL_GAIN_VALUE, m_edit_IR_Channel_Gain);
    DDX_Control(pDX, IDC_EDIT_UV_CHANNEL_GAIN_VALUE, m_edit_UV_Channel_Gain);
    DDX_Control(pDX, IDC_COMBO_COAXIAL_ILLUMINATION_CALIBRATION_TYPE, m_ctrlCmbCoaxialCalibrationType);
    DDX_Control(pDX, IDC_CMB_VISION_TYPE, m_ctrlCmbVisionType);
    DDX_Control(pDX, IDC_CMB_SIDE_VISION_NUMBER, m_cmbSideVisionNumber);
    DDX_Control(pDX, IDC_CMB_FRAME_GRABBER_TYPE, m_cmbFrameGrabberType); //kircheis_SWIR
    DDX_Control(pDX, IDC_CMB_NGRV_OPTICS_TYPE, m_cmbNgrvOpticsType);
    DDX_Control(pDX, IDC_CMB_2DVISION_CAMERA_TYPE, m_cmb2DCameraType);
    DDX_Control(pDX, IDC_CMB_3DVISION_CAMERA_TYPE, m_cmb3DCameraType);
    DDX_Control(pDX, IDC_CMB_3DVISION_CAMERA_NUMBER, m_cmb3DCameraNumber);
}

BEGIN_MESSAGE_MAP(CAppleDlg, CDialog)
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
//}}AFX_MSG_MAP
ON_BN_CLICKED(IDOK, &CAppleDlg::OnBnClickedOk)
ON_BN_CLICKED(IDCANCEL, &CAppleDlg::OnBnClickedCancel)
ON_BN_CLICKED(IDC_CHECK_USE_AI_INSPECTION, &CAppleDlg::OnBnClickedCheckUseAiInspection)
ON_EN_CHANGE(IDC_EDIT_WAIT_TIME_SEND_DATA, &CAppleDlg::OnEnChangeEditWaitTimeSendData)
ON_BN_CLICKED(IDC_CHECK_USE_BAYER_PATTERN_GPU, &CAppleDlg::OnBnClickedCheckUseBayerPatternGPU)
ON_CBN_SELCHANGE(IDC_CMB_VISION_TYPE, &CAppleDlg::OnCbnSelchangeCmbVisionType)
ON_CBN_SELCHANGE(IDC_CMB_SIDE_VISION_NUMBER, &CAppleDlg::OnCbnSelchangeComSideVisionNumber)
ON_CBN_SELCHANGE(IDC_CMB_FRAME_GRABBER_TYPE, &CAppleDlg::OnCbnSelchangeCmbFrameGrabberType) //kircheis_SWIR
ON_CBN_SELCHANGE(IDC_CMB_NGRV_OPTICS_TYPE, &CAppleDlg::OnCbnSelchangeCmbNgrvOpticsType)
ON_CBN_SELCHANGE(IDC_CMB_2DVISION_CAMERA_TYPE, &CAppleDlg::OnCbnSelchangeCmb2dvisionCameraType)
ON_CBN_SELCHANGE(IDC_CMB_3DVISION_CAMERA_TYPE, &CAppleDlg::OnCbnSelchangeCmb3dvisionCameraType)
END_MESSAGE_MAP()

// CAppleDlg 메시지 처리기

BOOL CAppleDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
    //  프레임워크가 이 작업을 자동으로 수행합니다.
    SetIcon(m_hIcon, TRUE); // 큰 아이콘을 설정합니다.
    SetIcon(m_hIcon, FALSE); // 작은 아이콘을 설정합니다.

    CdlgPassword Dlg;
    BOOL bRetry = TRUE;

    while (bRetry)
    {
        if (Dlg.DoModal() == IDOK)
        {
            if (Dlg.m_strPassword == _T("admin"))
            {
                break;
            }
            else
            {
                ::AfxMessageBox(_T("Please check Passwork."), MB_OK);
            }
        }
        else
        {
            OnBnClickedCancel();
            break;
        }
    }

    SetSystemConfig();

    return TRUE; // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CAppleDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // 아이콘을 그립니다.
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CAppleDlg::OnQueryDragIcon()
{
    return (HCURSOR)(m_hIcon);
}

void CAppleDlg::UpdateUI_ControlFrameGrabber(const int& nVisionType, const int frameGrabberType) //kircheis_SWIR
{
    switch (nVisionType)
    {
        case VISIONTYPE_2D_INSP:
            m_cmbFrameGrabberType.EnableWindow(TRUE); // 2D Vision은 Frame Grabber Type을 변경할 수 있다.
            if (frameGrabberType == enFrameGrabberType::FG_TYPE_IGRAB_XE)
            {
                m_cmbFrameGrabberType.SetCurSel(
                    (int)enFrameGrabberType::FG_TYPE_IGRAB_XQ); // 20250710 현재 2D Vision에서 IGRAB XE는 지원하지 않음
            }
            else
            {
                m_cmbFrameGrabberType.SetCurSel(frameGrabberType);
            }
            break;
        case VISIONTYPE_3D_INSP:
            m_cmbFrameGrabberType.EnableWindow(TRUE);
            if (frameGrabberType == enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES)
            {
                m_cmbFrameGrabberType.SetCurSel(
                    (int)enFrameGrabberType::FG_TYPE_IGRAB_XE);
            }
            else
            {
                m_cmbFrameGrabberType.SetCurSel(frameGrabberType);
            }
            break;
        case VISIONTYPE_NGRV_INSP:
            m_cmbFrameGrabberType.SetCurSel(enFrameGrabberType::FG_TYPE_IGRAB_XQ);
            m_cmbFrameGrabberType.EnableWindow(FALSE); // NGRV Vision은 Frame Grabber Type을 변경할 수 없다.
            break;
        case VISIONTYPE_SIDE_INSP:
            m_cmbFrameGrabberType.SetCurSel(enFrameGrabberType::FG_TYPE_IGRAB_XQ);
            m_cmbFrameGrabberType.EnableWindow(FALSE); // Side Vision은 Frame Grabber Type을 변경할 수 없다.
            break;
        case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
            m_cmbFrameGrabberType.SetCurSel(enFrameGrabberType::FG_TYPE_IGRAB_XQ);
            m_cmbFrameGrabberType.EnableWindow(FALSE); // SWIR Vision은 Frame Grabber Type을 변경할 수 없다.
            break;
        case VISIONTYPE_TR:
            m_cmbFrameGrabberType.SetCurSel(enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES);
            m_cmbFrameGrabberType.EnableWindow(FALSE); // TR Vision은 Frame Grabber Type을 변경할 수 없다.
            break;
        default:
            m_cmbFrameGrabberType.SetCurSel(-1);
            m_cmbFrameGrabberType.EnableWindow(FALSE); // Vision Type이 잘못된 경우 Frame Grabber Type을 변경할 수 없다.
            break;
    }
}

void CAppleDlg::SetSystemConfig(void)
{
    UpdateData(TRUE);

    m_bHardwareExist = m_pSysConfig->IsHardwareExist();
    m_nVisionType = m_pSysConfig->GetVisionType();
    m_n2DVisionCameraType = m_pSysConfig->Get2DVisionCameraType();
    m_n3DVisionCameraType = m_pSysConfig->Get3DVisionCameraType();
    m_nCameraNumber = m_pSysConfig->Get3DVisionCameraNumber();
    m_frameGrabberType = m_pSysConfig->GetFrameGrabberType(); //kircheis_SWIR
    UpdateUI_ControlFrameGrabber(m_nVisionType, m_frameGrabberType); //kircheis_SWIR

    UpdateUI_ControlNGRVOpticsType(m_nVisionType == VISIONTYPE_NGRV_INSP); 
    m_cmbNgrvOpticsType.SetCurSel(m_pSysConfig->GetNGRVOpticsType()); //kircheis_NGRV_Type
    const BOOL bEnableNGRVColorOpticsParams = SystemConfig::GetInstance().IsNgrvColorOptics() ? TRUE : FALSE;
    UpdateUI_ControlNGRV(bEnableNGRVColorOpticsParams);

    m_ctrlCmbHandlerType.SetCurSel(m_pSysConfig->GetHandlerType());
    m_ctrlCmbVisionType.SetCurSel(m_nVisionType);
    m_cmbSideVisionNumber.SetCurSel(m_pSysConfig->GetSideVisionNumber());
    m_cmb2DCameraType.SetCurSel(m_n2DVisionCameraType);
    m_cmb3DCameraType.SetCurSel(m_n3DVisionCameraType);
    m_cmb3DCameraNumber.SetCurSel(m_nCameraNumber - 1);

    m_nGRAB_Duration = m_pSysConfig->GetGrabDuration();

    m_nThreadNum = m_pSysConfig->GetThreadNum();
    m_nGrabBufferNum = m_pSysConfig->GetGrabBufferNum(false);

    switch (m_nVisionType)
    {
        case VISIONTYPE_2D_INSP:
            m_ctrlCmbCoaxialCalibrationType.EnableWindow(TRUE);
            ((CButton*)GetDlgItem(IDC_CHECK_USE_RING_ILLUMINATION))
                ->SetCheck(m_pSysConfig->GetExistRingillumination()); //mc_Exist Ring illum.
            break;

        case VISIONTYPE_3D_INSP:
            m_ctrlCmbCoaxialCalibrationType.EnableWindow(FALSE);
            ((CButton*)GetDlgItem(IDC_CHECK_USE_RING_ILLUMINATION))->SetCheck(FALSE);
            break;

        case VISIONTYPE_NGRV_INSP:
            m_ctrlCmbCoaxialCalibrationType.EnableWindow(TRUE);
            ((CButton*)GetDlgItem(IDC_CHECK_USE_RING_ILLUMINATION))->SetCheck(FALSE);
            break;

        case VISIONTYPE_SIDE_INSP:
        case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
            ((CButton*)GetDlgItem(IDC_CHECK_USE_RING_ILLUMINATION))->SetCheck(FALSE);
            break;

        case VISIONTYPE_TR:
            m_ctrlCmbCoaxialCalibrationType.EnableWindow(TRUE);
            ((CButton*)GetDlgItem(IDC_CHECK_USE_RING_ILLUMINATION))
                ->SetCheck(m_pSysConfig->GetExistRingillumination()); //mc_Exist Ring illum.
            break;
    }

    m_cmbThreadNum.SetCurSel(m_nThreadNum - 1);
    m_cmbGrabBufferNum.SetCurSel(m_nGrabBufferNum);

    CheckDlgButton(IDC_CHECK_USE_AI_INSPECTION, m_pSysConfig->m_bUseAiInspection);
    UpdateUI_ControlDL(m_pSysConfig->m_bUseAiInspection);

    m_nWaitTImeToSendData = m_pSysConfig->m_nDLWaitTimeSendRejectData;

    // NGRV용 Check Box는 항상 TRUE 일 것, ASNC에 포함되어 Disable 설정해놓음 - JHB_2024.02.14
    m_bUseLongfExposure = m_pSysConfig->m_bUseLongExposureNGRV;
    m_bUseBayerPattern = m_pSysConfig->m_bUseBayerPatternGPU;

    m_cmbIR_Channel.SetCurSel(m_pSysConfig->m_nNgrvIRchID);
    m_cmbUV_Channel.SetCurSel(m_pSysConfig->m_nNgrvUVchID);

    m_ctrlCmbCoaxialCalibrationType.SetCurSel(m_pSysConfig->m_nCoaxialIllumMirrorCalType);

    //{{ Set Camera Channel Gain Values
    CString strTemp;

    strTemp.Format(_T("%.01f"), m_pSysConfig->m_fDefault_Camera_Gain);
    m_edit_Default_Channel_Gain.SetWindowText(strTemp);

    strTemp.Format(_T("%.01f"), m_pSysConfig->m_fIR_Camera_Gain);
    m_edit_IR_Channel_Gain.SetWindowText(strTemp);

    strTemp.Format(_T("%.01f"), m_pSysConfig->m_fUV_Camera_Gain);
    m_edit_UV_Channel_Gain.SetWindowText(strTemp);
    //}}

    UpdateUI(m_nVisionType);

    UpdateData(FALSE);
}

void CAppleDlg::GetSystemConfig(void)
{
    UpdateData(TRUE);

    m_nVisionType = m_ctrlCmbVisionType.GetCurSel();
    m_nCameraNumber = m_cmb3DCameraNumber.GetCurSel() + 1;

    if (m_nVisionType == VISIONTYPE_2D_INSP || m_nVisionType == VISIONTYPE_3D_INSP) //kircheis_SWIR
    {
        int nFrameGrabberType = m_cmbFrameGrabberType.GetCurSel();
        m_pSysConfig->SetFrameGrabberType(nFrameGrabberType); // 2D & 3D Vision은 Frame Grabber Type을 변경할 수 있다.
    }

    m_pSysConfig->SetNGRVOpticsType(m_cmbNgrvOpticsType.GetCurSel()); //kircheis_NGRV_Type

    m_pSysConfig->SetSystemConfig(m_bHardwareExist, m_nVisionType, 1, m_nLED_Duration, m_nGRAB_Duration,
        m_ctrlCmbHandlerType.GetCurSel(), m_cmb2DCameraType.GetCurSel(), m_cmb3DCameraType.GetCurSel(),
        m_nCameraNumber);

    m_pSysConfig->SetThreadNum(m_cmbThreadNum.GetCurSel() + 1);
    m_pSysConfig->SetGrabBufferNum(m_cmbGrabBufferNum.GetCurSel());
    m_pSysConfig->m_nCoaxialIllumMirrorCalType = m_ctrlCmbCoaxialCalibrationType.GetCurSel();

    if (m_nVisionType == VISIONTYPE_2D_INSP || m_nVisionType == VISIONTYPE_TR)
        m_pSysConfig->SetExistRingillumination(
            ((CButton*)GetDlgItem(IDC_CHECK_USE_RING_ILLUMINATION))->GetCheck()); //mc_Exist Ring illum.

    if (m_pSysConfig->IsVisionTypeNGRV() == TRUE)
    {
        m_pSysConfig->m_bUseLongExposureNGRV = m_bUseLongfExposure;
        m_pSysConfig->m_bUseBayerPatternGPU = m_bUseBayerPattern;
        m_pSysConfig->m_nNgrvIRchID = m_cmbIR_Channel.GetCurSel();
        m_pSysConfig->m_nNgrvUVchID = m_cmbUV_Channel.GetCurSel();

        //{{ Set Camera Channel Gain Values
        CString strTemp;

        m_edit_Default_Channel_Gain.GetWindowText(strTemp);
        m_pSysConfig->m_fDefault_Camera_Gain = (float)_ttof(strTemp);

        m_edit_IR_Channel_Gain.GetWindowText(strTemp);
        m_pSysConfig->m_fIR_Camera_Gain = (float)_ttof(strTemp);

        m_edit_UV_Channel_Gain.GetWindowText(strTemp);
        m_pSysConfig->m_fUV_Camera_Gain = (float)_ttof(strTemp);
        //}}
    }

    if (m_nVisionType == VISIONTYPE_SIDE_INSP)
    {
        long nSideVisionNumber = m_cmbSideVisionNumber.GetCurSel();
        m_pSysConfig->SetSideVisionNumber(nSideVisionNumber);
    }
    /*else	해당 Parameter는 무조건 TRUE - JHB_2024.02.14
		m_pSysConfig->m_bUseLongExposureNGRV = FALSE;*/
}

void CAppleDlg::UpdateUI(const int& visionType)
{
    BOOL enableCommon2D = FALSE;
    BOOL enableCoaxialCal = FALSE;
    BOOL enable2D = FALSE;
    BOOL enable3D = FALSE;
    BOOL enableNGRV = FALSE;
    BOOL enableNGRVOpticsType = FALSE;
    BOOL enableSide = FALSE;
    BOOL enableDL = FALSE;

    if (VISIONTYPE_START <= visionType && visionType < VISIONTYPE_END)
    {
        m_pSysConfig->m_nVisionType = m_nVisionType;

        switch (visionType)
        {
            case VISIONTYPE_2D_INSP:
            {
                enableCommon2D = TRUE;
                enableCoaxialCal = TRUE;
                enable2D = TRUE;
                enableDL = TRUE;
                break;
            }
            case VISIONTYPE_3D_INSP:
            {
                enable3D = TRUE;
                break;
            }
            case VISIONTYPE_NGRV_INSP:
            {
                enableCommon2D = TRUE;
                enableNGRVOpticsType = TRUE;
                enableNGRV = (m_cmbNgrvOpticsType.GetCurSel() == NGRV_VISION_OPTICS_TYPE_COLOR);                
                break;
            }
            case VISIONTYPE_SIDE_INSP:
            {
                enableCommon2D = TRUE;
                enableCoaxialCal = TRUE;
                enableSide = TRUE;
                break;
            }
            case VISIONTYPE_SWIR_INSP: //kircheis_SWIR
            {
                enableCommon2D = TRUE;
                enableCoaxialCal = TRUE;
                enableDL = TRUE;
                break;
            }
            case VISIONTYPE_TR:
            {
                enableCommon2D = TRUE;
                break;
            }
        }
    }

    UpdateUI_ControlCommon2D(enableCommon2D, enableCoaxialCal);
    UpdateUI_Control2D(enable2D);
    UpdateUI_Control3D(enable3D);
    UpdateUI_ControlNGRV(enableNGRV);
    UpdateUI_ControlNGRVOpticsType(enableNGRVOpticsType);
    UpdateUI_ControlSide(enableSide);
    UpdateUI_ControlDL(enableDL);
}

void CAppleDlg::UpdateUI_ControlCommon2D(BOOL bEnable, BOOL bCoaxialCal)
{
    GetDlgItem(IDC_STATIC_LED_DURATION)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_LED_DURATION)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_GRAB_DURATION)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_GRAB_DURATION)->EnableWindow(bEnable);
    
    const BOOL bFinalCoaxialCalEnable = bEnable && bCoaxialCal;
    GetDlgItem(IDC_STATIC_COAXIAL_ILLUMINATION_CALIBRATION_TYPE)->EnableWindow(bFinalCoaxialCalEnable);
    GetDlgItem(IDC_COMBO_COAXIAL_ILLUMINATION_CALIBRATION_TYPE)->EnableWindow(bFinalCoaxialCalEnable);
}

void CAppleDlg::UpdateUI_Control2D(BOOL bEnable) //kircheis_100M
{    
    GetDlgItem(IDC_CHECK_USE_RING_ILLUMINATION)->EnableWindow(bEnable);
    GetDlgItem(IDC_CMB_2DVISION_CAMERA_TYPE)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_2DVISION_CAMERA_TYPE)->EnableWindow(bEnable);
}

void CAppleDlg::UpdateUI_Control3D(BOOL bEnable) //kircheis_100M
{
    GetDlgItem(IDC_STATIC_3DVISION_CAMERA_TYPE)->EnableWindow(bEnable);
    GetDlgItem(IDC_CMB_3DVISION_CAMERA_TYPE)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_CAMERA_NUMBER)->EnableWindow(bEnable);
    GetDlgItem(IDC_CMB_3DVISION_CAMERA_NUMBER)->EnableWindow(bEnable);
}

void CAppleDlg::UpdateUI_ControlNGRV(BOOL bEnable) //JHB
{
    GetDlgItem(IDC_STATIC_NGRV_COLOR_PARAM_BOX)->EnableWindow(bEnable);
    
    GetDlgItem(IDC_CHECK_USE_LONG_EXPOSURE)->EnableWindow(bEnable);

    GetDlgItem(IDC_STATIC_IR_CHANNEL_ID)->EnableWindow(bEnable);
    GetDlgItem(IDC_COMBO_IR_CHANNEL_ID)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_UV_CHANNEL_ID)->EnableWindow(bEnable);
    GetDlgItem(IDC_COMBO_UV_CHANNEL_ID)->EnableWindow(bEnable);

    GetDlgItem(IDC_STATIC_DEFAULT_CAM_GAIN)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_DEFAULT_CHANNEL_GAIN_VALUE)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_IR_CHANNEL_GAIN)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_IR_CHANNEL_GAIN_VALUE)->EnableWindow(bEnable);
    GetDlgItem(IDC_STATIC_UV_CHANNEL_GAIN)->EnableWindow(bEnable);
    GetDlgItem(IDC_EDIT_UV_CHANNEL_GAIN_VALUE)->EnableWindow(bEnable);
}

void CAppleDlg::UpdateUI_ControlNGRVOpticsType(const BOOL bEnable)
{
    GetDlgItem(IDC_STATIC_NGRV_OPTICS_TYPE)->EnableWindow(bEnable);
    GetDlgItem(IDC_CMB_NGRV_OPTICS_TYPE)->EnableWindow(bEnable);
}

void CAppleDlg::UpdateUI_ControlSide(BOOL i_bEnable)
{
    GetDlgItem(IDC_STATIC_SIDE_VISION_NUMBER)->EnableWindow(i_bEnable);
    GetDlgItem(IDC_CMB_SIDE_VISION_NUMBER)->EnableWindow(i_bEnable);
}

void CAppleDlg::UpdateUI_ControlDL(BOOL i_bEnable)
{
    GetDlgItem(IDC_CHECK_USE_AI_INSPECTION)->EnableWindow(i_bEnable);

    BOOL bUseAIInspection = i_bEnable && IsDlgButtonChecked(IDC_CHECK_USE_AI_INSPECTION);
    GetDlgItem(IDC_STATIC_WAIT_TIME_SEND_DATA)->EnableWindow(bUseAIInspection);
    GetDlgItem(IDC_EDIT_WAIT_TIME_SEND_DATA)->EnableWindow(bUseAIInspection);
    GetDlgItem(IDC_STATIC_WAIT_TIME_SEC)->EnableWindow(bUseAIInspection);
}

void CAppleDlg::OnBnClickedOk()
{
    GetSystemConfig();

    m_pSysConfig->SaveIni(DynamicSystemPath::get(DefineFile::System));

    OnOK();

    AfxMessageBox(_T("Application is Complete!"));
}

void CAppleDlg::OnBnClickedCancel()
{
    OnCancel();
}

void CAppleDlg::OnBnClickedCheckUseAiInspection()
{
    m_pSysConfig->m_bUseAiInspection = IsDlgButtonChecked(IDC_CHECK_USE_AI_INSPECTION);

    UpdateUI_ControlDL(TRUE);
}

void CAppleDlg::OnEnChangeEditWaitTimeSendData()
{
    UpdateData(TRUE);
    m_pSysConfig->m_nDLWaitTimeSendRejectData = m_nWaitTImeToSendData;
}

void CAppleDlg::OnBnClickedCheckUseBayerPatternGPU()
{
    m_pSysConfig->m_bUseBayerPatternGPU = IsDlgButtonChecked(IDC_CHECK_USE_BAYER_PATTERN_GPU);
}

void CAppleDlg::OnCbnSelchangeCmbVisionType()
{
    long nVisionType = m_ctrlCmbVisionType.GetCurSel();

    UpdateUI_ControlFrameGrabber(nVisionType, m_frameGrabberType); //kircheis_SWIR

    UpdateUI(nVisionType);
}

void CAppleDlg::OnCbnSelchangeComSideVisionNumber()
{
    long nSideVisionNumber = m_cmbSideVisionNumber.GetCurSel();

    switch (nSideVisionNumber)
    {
        case SIDE_VISIONNUMBER_1:
            m_pSysConfig->SetSideVisionNumber(nSideVisionNumber);
            break;
        case SIDE_VISIONNUMBER_2:
            m_pSysConfig->SetSideVisionNumber(nSideVisionNumber);
            break;
        default:
            break;
    }
}

void CAppleDlg::OnCbnSelchangeCmbFrameGrabberType() //kircheis_SWIR
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    int nVisionType = m_ctrlCmbVisionType.GetCurSel();
    if (nVisionType != VISIONTYPE_2D_INSP
        && nVisionType != VISIONTYPE_3D_INSP) // 2D & 3D Vision이 아닌 경우 Frame Grabber Type을 변경할 수 없다.
    {
        return;
    }

    m_frameGrabberType = m_cmbFrameGrabberType.GetCurSel();

    if (nVisionType == VISIONTYPE_3D_INSP && m_frameGrabberType == enFrameGrabberType::FG_TYPE_IGRAB_G2_SERIES)
    {
        m_cmbFrameGrabberType.SetCurSel((int)enFrameGrabberType::FG_TYPE_IGRAB_XE);
        m_cmb3DCameraType.SetCurSel((int)en3DVisionCameraType::CAMERATYPE_VIEWWORKS_12MX);
        m_cmb3DCameraNumber.SetCurSel(0);
    }
    else if (nVisionType == VISIONTYPE_3D_INSP && m_frameGrabberType == enFrameGrabberType::FG_TYPE_IGRAB_XQ)
    {
        m_cmb3DCameraType.SetCurSel((int)en3DVisionCameraType::CAMERATYPE_MIKROTRON_EOSENS_21CXP2);
        m_nCameraNumber = m_pSysConfig->Get3DVisionCameraNumber();
        m_cmb3DCameraNumber.SetCurSel(m_nCameraNumber-1);
    }
    else
    {
        if (nVisionType == VISIONTYPE_3D_INSP)
        {
            m_cmb3DCameraType.SetCurSel((int)en3DVisionCameraType::CAMERATYPE_VIEWWORKS_12MX);
            m_cmb3DCameraNumber.SetCurSel(0);
        }
    }

    if (nVisionType == VISIONTYPE_2D_INSP && m_frameGrabberType == enFrameGrabberType:: FG_TYPE_IGRAB_XE) //2D Vision은 아직 iGrabXE를 지원하지 않기에 XQ로 바꾸고, 2D Camera Type도 DALSA로 변경
    {
        m_cmbFrameGrabberType.SetCurSel((int)enFrameGrabberType::FG_TYPE_IGRAB_XQ);
        m_cmb2DCameraType.SetCurSel((int)en2DVisionCameraType::CAMERATYPE_DALSA_GENIE_NANO_CXP_67M);

    }
    else if (nVisionType == VISIONTYPE_2D_INSP && m_frameGrabberType == enFrameGrabberType::FG_TYPE_IGRAB_XQ) // 2D Vision & iGrabXQ를 사용하는 Camera는 DALSA밖에 없음.
    {
        m_cmb2DCameraType.SetCurSel((int)en2DVisionCameraType::CAMERATYPE_DALSA_GENIE_NANO_CXP_67M);
    }
    else // 나머지 Grabber는 Laon People Camera사용
    {
        if (nVisionType == VISIONTYPE_2D_INSP)
        {
            m_cmb2DCameraType.SetCurSel((int)en2DVisionCameraType::CAMERATYPE_LAON_PEOPLE_CXP25M);
        }
    }
}

void CAppleDlg::OnCbnSelchangeCmbNgrvOpticsType()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    const int nOpticsType = m_cmbNgrvOpticsType.GetCurSel();

    const BOOL opticsTypeIsColorOptics = (nOpticsType == NGRV_VISION_OPTICS_TYPE_COLOR) ? TRUE : FALSE;

    UpdateUI_ControlNGRV(opticsTypeIsColorOptics);

}

void CAppleDlg::OnCbnSelchangeCmb2dvisionCameraType()
{
    int nVisionType = m_ctrlCmbVisionType.GetCurSel();
    if (nVisionType != VISIONTYPE_2D_INSP) // 2D Vision이 아닌 경우 2D Camera Type을 변경할 수 없다.
    {
        return;
    }
}

void CAppleDlg::OnCbnSelchangeCmb3dvisionCameraType()
{
    int nVisionType = m_ctrlCmbVisionType.GetCurSel();
    if (nVisionType != VISIONTYPE_3D_INSP)
    {
        return;
    }
}
