//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSystemSetup.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../HardwareModules/dPI_Framegrabber/Framegrabber.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgSystemSetup, CDialog)

DlgSystemSetup::DlgSystemSetup(CWnd* pParent /*=NULL*/)
    : CDialog(DlgSystemSetup::IDD, pParent)
    , m_needReboot(false)
{
}

DlgSystemSetup::~DlgSystemSetup()
{
}

void DlgSystemSetup::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_IMAGESAMPLING, m_cmbImageSampling);
}

BEGIN_MESSAGE_MAP(DlgSystemSetup, CDialog)
ON_BN_CLICKED(IDOK, &DlgSystemSetup::OnBnClickedOk)
ON_BN_CLICKED(IDC_CHK_SAVE_REVIEW_IMAGE, &DlgSystemSetup::OnBnClickedChkSaveReviewImage)
ON_BN_CLICKED(IDC_BTN_SYSTEMINI_REFRESH, &DlgSystemSetup::OnBnClickedBtnSysteminiRefresh)
ON_BN_CLICKED(IDC_CHK_ENABLE_CPU_CALC_LOG, &DlgSystemSetup::OnBnClickedChkEnableCpuCalcLog)
ON_BN_CLICKED(IDC_CHK_SAVE_MV_REJECT_DATA, &DlgSystemSetup::OnBnClickedChkSaveMvRejectData)
ON_BN_CLICKED(IDC_BTN_APPLY_CAM_GAIN, &DlgSystemSetup::OnBnClickedBtnApplyCamGain)
END_MESSAGE_MAP()

// DlgSystemSetup 메시지 처리기입니다.

BOOL DlgSystemSetup::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    auto& information = SystemConfig::GetInstance();

    CString strTemp;
    if (information.GetHandlerType() != HANDLER_TYPE_380BRIDGE)
    {
        ((CButton*)GetDlgItem(IDC_CHK_SAVE_INLINE_RAW_IMAGES))->EnableWindow(FALSE);
    }

    m_cmbImageSampling.SetCurSel(PersonalConfig::getInstance().getImageSampling() - 1);
    ((CButton*)GetDlgItem(IDC_CHECK_SOCKET_CONNECTION))
        ->SetCheck(PersonalConfig::getInstance().isSocketConnectionEnabled());
    if (information.m_bHardwareExist)
    {
        m_cmbImageSampling.EnableWindow(FALSE);
        GetDlgItem(IDC_CHECK_SOCKET_CONNECTION)->EnableWindow(FALSE);
    }

    ((CButton*)GetDlgItem(IDC_CHK_SAVE_INLINE_RAW_IMAGES))
        ->SetCheck(information.m_saveInlineRawImage ? BST_CHECKED : BST_UNCHECKED);
    ((CButton*)GetDlgItem(IDC_CHK_SAVE_SLITBEAM_ORIGINAL_IMAGES))
        ->SetCheck(information.m_saveSlitbeamOriginalImage ? BST_CHECKED : BST_UNCHECKED);

    SetDlgItemInt(IDC_EDIT_SEND_FRAME, information.m_nSendHostFrameNumber);
    ((CButton*)GetDlgItem(IDC_CHK_USE_MULTI_TEACHING))->SetCheck(information.m_bUseMarkMultiTeaching);

    strTemp.Format(_T("%d"), information.m_nSurfaceRejectReportPatchCount_X);
    ((CEdit*)GetDlgItem(IDC_EDIT_REJECT_REPORT_X))->SetWindowText(strTemp);

    strTemp.Format(_T("%d"), information.m_nSurfaceRejectReportPatchCount_Y);
    ((CEdit*)GetDlgItem(IDC_EDIT_REJECT_REPORT_Y))->SetWindowText(strTemp);

    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->ResetContent();
    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->AddString(_T("C:\\"));
    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->AddString(_T("D:\\"));
    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->AddString(_T("E:\\"));
    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->AddString(_T("F:\\"));
    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->AddString(_T("G:\\"));
    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->AddString(_T("H:\\"));
    ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->SetCurSel(information.m_nReviewImageSaveDrive);

    const long nLength = sizeof(information.m_strGrabVerifyMatchingCount) / sizeof(CString);
    ;

    ((CComboBox*)GetDlgItem(IDC_CMB_GPU_CALC_RETRY))->ResetContent();

    for (int i = 0; i < nLength; i++)
    {
        ((CComboBox*)GetDlgItem(IDC_CMB_GPU_CALC_RETRY))->AddString(information.m_strGrabVerifyMatchingCount[i]);
    }

    ((CComboBox*)GetDlgItem(IDC_CMB_GPU_CALC_RETRY))->SetCurSel(information.m_nGrabVerifyMatchingCount);
    ((CButton*)GetDlgItem(IDC_CHK_USE_GRAB_RETRY))->SetCheck(information.m_bUseGrabRetry); //kircheis_3DCalc

    SetDlgItemText(IDC_EDIT_RECIPE_VERSION, information.m_strRecipeVersion);

    ((CButton*)GetDlgItem(IDC_CHK_ENABLE_CPU_CALC_LOG))
        ->SetCheck(information.Get_Enable_CPU_CALC_SAVE_LOG()); //mc_CPU_Test
    ((CButton*)GetDlgItem(IDC_CHK_ENABLE_GPU_CALC_LOG))
        ->SetCheck(information.Get_Enable_GPU_CALC_SAVE_LOG()); //mc_GPU_Test

    strTemp.Format(_T("%d"), information.Get_CPU_SavebyInspectionCount());
    ((CEdit*)GetDlgItem(IDC_EDIT_CPU_SAVE_INSP_COUNT))->SetWindowText(strTemp);

    strTemp.Format(_T("%d"), information.Get_CPU_SaveReportMaximumCount());
    ((CEdit*)GetDlgItem(IDC_EDIT_CPU_SAVE_REPORT_MAXIMUM_COUNT))->SetWindowText(strTemp);

    Set_ReadOnly_CPU_Calc_Log_Param(information.Get_Enable_CPU_CALC_SAVE_LOG());

    CheckDlgButton(
        IDC_CHK_SAVE_MV_REJECT_DATA, SystemConfig::GetInstance().m_bUseSaveCollectMVRejectData); // 09.28.20 KSY

    ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))->ResetContent();
    ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))->AddString(_T("Use [Gain]"));
    ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))->AddString(_T("Use only linear"));
    ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))->AddString(_T("Use curve + linear"));
    ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))->AddString(_T("Use linear+"));
    if (SystemConfig::GetInstance().m_bLockIllumCalLinearPlus == TRUE)
        SystemConfig::GetInstance().m_nIlluminationCalType = IllumCalType_Linear_Plus;

    ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))
        ->SetCurSel(SystemConfig::GetInstance().m_nIlluminationCalType); //kircheis_IllumCalType
    ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))
        ->EnableWindow(!(SystemConfig::GetInstance().m_bLockIllumCalLinearPlus));

    ((CComboBox*)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))->EnableWindow(FALSE);
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_NGRV_INSP) // 2021.01.20 - JHB_NGRV image type
    {
        ((CComboBox*)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))->EnableWindow(TRUE);
        ((CComboBox*)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))->ResetContent();
        ((CComboBox*)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))->AddString(_T("BMP"));
        ((CComboBox*)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))->AddString(_T("JPG"));
        ((CComboBox*)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))->AddString(_T("PNG"));
        ((CComboBox*)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))
            ->SetCurSel(SystemConfig::GetInstance().m_nSaveImageTypeForNGRV);
    }

    ((CButton*)GetDlgItem(IDC_CHK_USE_3D_NOISECHECK))
        ->SetCheck(information.GetUse3DCameraNoiseTest()); //SDY 3D Noise check

    ((CButton*)GetDlgItem(IDC_CHK_ENABLE_SEQUENCE_INSPECTION_TIME_LOG))
        ->SetCheck(information.Get_SaveSequenceAndInspectionTimeLog()); //mc_Seq&Inspection_Log_Test

    ((CButton*)GetDlgItem(IDC_CHK_ENABLE_IGRAB_BD_TEMPERATURE_LOG))
        ->SetCheck(information.GetiGrab_Board_Temperature_Log_save_Vision()); //mc_iGrab B/D Temperature Log
    ((CButton*)GetDlgItem(IDC_CHK_ENABLE_INSPECTION_RESULT_TIMEOUT_RAW_IMAGE_SAVE))
        ->SetCheck(information.GetInspectionResultTimeoutSaveRawimage()); //mc_Inspection Result Timeout Save Raw image

    strTemp.Format(_T("%.3f"), information.m_fInterpolationStart);
    SetDlgItemText(IDC_EDIT_INTERPOLATION_START, strTemp);

    ((CButton*)GetDlgItem(IDC_CHK_ENABLE_INSPECTION_RESULT_SEND_TIMELOG))
        ->SetCheck(information.Get_SaveDeviceSendResultTimeLog()); //mc_Inspection Result Timeout Save Raw image

    {
        const long nimage_Zero_Limit_Count_Length
            = sizeof(information.m_strGrabFailforimageZeroLimitCount) / sizeof(CString);
        ;

        ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_IMAGE_ZERO_LIMIT_COUNT))->ResetContent();

        for (int i = 0; i < nimage_Zero_Limit_Count_Length; i++)
        {
            ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_IMAGE_ZERO_LIMIT_COUNT))
                ->AddString(information.m_strGrabFailforimageZeroLimitCount[i]);
        }
    }

    {
        const long nCable_Error_Limit_Count_Length
            = sizeof(information.m_strGrabFailforCableErrorLimitCount) / sizeof(CString);
        ;

        ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_CABLE_ERROR_LIMIT_COUNT))->ResetContent();

        for (int i = 0; i < nCable_Error_Limit_Count_Length; i++)
        {
            ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_CABLE_ERROR_LIMIT_COUNT))
                ->AddString(information.m_strGrabFailforCableErrorLimitCount[i]);
        }
    }

    ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_IMAGE_ZERO_LIMIT_COUNT))
        ->SetCurSel(information.m_nGrabFailforimageZeroLimitCount_idx); //mc_Grab Fail Limit Count
    ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_CABLE_ERROR_LIMIT_COUNT))
        ->SetCurSel(information.m_nGrabFailforCableErrorLimitCount_idx); //mc_Grab Fail Limit Count

    ((CComboBox*)GetDlgItem(IDC_COMBO_2DID_CROPPING_IMAGE_SAVE_OPTION))
        ->SetCurSel((long)information.Get2DIDCroppingimageSaveOption()); //mc_2DID Cropping image Save Option

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void DlgSystemSetup::OnBnClickedOk()
{
    UpdateData(TRUE);

    CString strTemp;
    bool socketConnectionEnabled = ((CButton*)GetDlgItem(IDC_CHECK_SOCKET_CONNECTION))->GetCheck() ? true : false;

    if (PersonalConfig::getInstance().getImageSampling() != m_cmbImageSampling.GetCurSel() + 1
        || PersonalConfig::getInstance().isSocketConnectionEnabled() != socketConnectionEnabled)
    {
        if (MessageBox(
                _T("A restart is required for the application.\r\nDo you want to continue?"), _T("Message"), MB_YESNO)
            != IDYES)
        {
            return;
        }
        PersonalConfig::getInstance().setImageSampling(m_cmbImageSampling.GetCurSel() + 1);
        PersonalConfig::getInstance().setSocketConnectionEnabled(socketConnectionEnabled);
        m_needReboot = true;
    }

    auto& information = SystemConfig::GetInstance();

    information.m_saveInlineRawImage
        = ((CButton*)GetDlgItem(IDC_CHK_SAVE_INLINE_RAW_IMAGES))->GetCheck() == BST_CHECKED;
    information.m_saveSlitbeamOriginalImage
        = ((CButton*)GetDlgItem(IDC_CHK_SAVE_SLITBEAM_ORIGINAL_IMAGES))->GetCheck() == BST_CHECKED;

    ((CEdit*)GetDlgItem(IDC_EDIT_SEND_FRAME))->GetWindowText(strTemp);
    information.m_nSendHostFrameNumber = _ttoi(strTemp);

    ((CEdit*)GetDlgItem(IDC_EDIT_REJECT_REPORT_X))->GetWindowText(strTemp);
    information.m_nSurfaceRejectReportPatchCount_X = _ttoi(strTemp);

    ((CEdit*)GetDlgItem(IDC_EDIT_REJECT_REPORT_Y))->GetWindowText(strTemp);
    information.m_nSurfaceRejectReportPatchCount_Y = _ttoi(strTemp);

    information.m_nReviewImageSaveDrive = ((CComboBox*)GetDlgItem(IDC_CMB_DRIVE))->GetCurSel();
    information.SetReviewImageSaveDriveString();

    information.m_bUseMarkMultiTeaching = ((CButton*)GetDlgItem(IDC_CHK_USE_MULTI_TEACHING))->GetCheck();

    information.m_nGrabVerifyMatchingCount = ((CComboBox*)GetDlgItem(IDC_CMB_GPU_CALC_RETRY))->GetCurSel();
    information.m_bUseGrabRetry = ((CButton*)GetDlgItem(IDC_CHK_USE_GRAB_RETRY))->GetCheck(); //kircheis_3DCalcRetry

    information.Set_CPU_CALC_SAVE_LOG(((CButton*)GetDlgItem(IDC_CHK_ENABLE_CPU_CALC_LOG))->GetCheck());
    information.Set_GPU_CALC_SAVE_LOG(((CButton*)GetDlgItem(IDC_CHK_ENABLE_GPU_CALC_LOG))->GetCheck());

    information.m_nIlluminationCalType
        = ((CComboBox*)GetDlgItem(IDC_CMB_ILLUM_CAL_TYPE))->GetCurSel(); //kircheis_IllumCalType
    information.SaveIniIllumCalType();

    if (information.Get_Enable_CPU_CALC_SAVE_LOG() == true)
    {
        ((CEdit*)GetDlgItem(IDC_EDIT_CPU_SAVE_INSP_COUNT))->GetWindowText(strTemp);
        information.Set_CPU_SavebyInspectionCount(_ttoi(strTemp));

        ((CEdit*)GetDlgItem(IDC_EDIT_CPU_SAVE_REPORT_MAXIMUM_COUNT))->GetWindowText(strTemp);
        information.Set_CPU_SaveReportMaximumCount(_ttoi(strTemp));
    }

    //SystemConfig::GetInstance().m_nSaveImageTypeForNGRV = ((CComboBox *)GetDlgItem(IDC_CMB_NGRV_IMAGE_SAVE_TYPE))->GetCurSel();	// 2021.01.20 - JHB_NGRV image type option save

    information.SetUse3DCameraNoiseTest(((CButton*)GetDlgItem(IDC_CHK_USE_3D_NOISECHECK))->GetCheck());

    information.Set_SaveSequenceAndInspectionTimeLog(
        ((CButton*)GetDlgItem(IDC_CHK_ENABLE_SEQUENCE_INSPECTION_TIME_LOG))->GetCheck());

    ((CEdit*)GetDlgItem(IDC_EDIT_INTERPOLATION_START))->GetWindowText(strTemp);
    information.m_fInterpolationStart = CAST_FLOAT(_ttof(strTemp));

    information.SetiGrab_Board_Temperature_Log_save_Vision(
        ((CButton*)GetDlgItem(IDC_CHK_ENABLE_IGRAB_BD_TEMPERATURE_LOG))->GetCheck()); //mc_iGrab B/D Temperature Log
    information.SetInspectionResultTimeoutSaveRawimage(
        ((CButton*)GetDlgItem(IDC_CHK_ENABLE_INSPECTION_RESULT_TIMEOUT_RAW_IMAGE_SAVE))
            ->GetCheck()); //mc_Inspection Result Timeout Save Raw image

    information.Set_SaveDeviceSendResultTimeLog(
        ((CButton*)GetDlgItem(IDC_CHK_ENABLE_INSPECTION_RESULT_SEND_TIMELOG))->GetCheck());

    information.m_nGrabFailforimageZeroLimitCount_idx
        = ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_IMAGE_ZERO_LIMIT_COUNT))
              ->GetCurSel(); //mc_Grab Fail Limit Count
    information.m_nGrabFailforCableErrorLimitCount_idx
        = ((CComboBox*)GetDlgItem(IDC_COMBO_GRAB_FAIL_FOR_CABLE_ERROR_LIMIT_COUNT))
              ->GetCurSel(); //mc_Grab Fail Limit Count

    information.Set2DIDCroppingimageSaveOption(
        ((CComboBox*)GetDlgItem(IDC_COMBO_2DID_CROPPING_IMAGE_SAVE_OPTION))->GetCurSel());

    OnOK();
}

void DlgSystemSetup::OnBnClickedChkSaveReviewImage()
{
    UpdateData(TRUE);

    GetDlgItem(IDC_EDIT_REVIEW_SAVE_FRAME)->EnableWindow(((CButton*)GetDlgItem(IDC_CHK_SAVE_REVIEW_IMAGE))->GetCheck());
}

void DlgSystemSetup::OnBnClickedBtnSysteminiRefresh()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

    ::DeleteFile(DynamicSystemPath::get(DefineFile::System));

    SystemConfig::GetInstance().SaveIni(DynamicSystemPath::get(DefineFile::System));

    SystemConfig::GetInstance().SaveCurrentScaleXY();
}

void DlgSystemSetup::Set_ReadOnly_CPU_Calc_Log_Param(const bool i_bReadOnly)
{
    bool bReadyOnly = i_bReadOnly ? FALSE : TRUE;

    ((CEdit*)GetDlgItem(IDC_EDIT_CPU_SAVE_INSP_COUNT))->SetReadOnly(bReadyOnly);
    ((CEdit*)GetDlgItem(IDC_EDIT_CPU_SAVE_REPORT_MAXIMUM_COUNT))->SetReadOnly(bReadyOnly);
}

void DlgSystemSetup::OnBnClickedChkEnableCpuCalcLog()
{
    Set_ReadOnly_CPU_Calc_Log_Param(((CButton*)GetDlgItem(IDC_CHK_ENABLE_CPU_CALC_LOG))->GetCheck());
}

void DlgSystemSetup::OnBnClickedChkSaveMvRejectData()
{
    SystemConfig::GetInstance().m_bUseSaveCollectMVRejectData = IsDlgButtonChecked(IDC_CHK_SAVE_MV_REJECT_DATA);
}

void DlgSystemSetup::OnBnClickedBtnApplyCamGain()
{
    CString strTemp;
    float fInputGain(0.f);

    ((CEdit*)GetDlgItem(IDC_EDIT_CAM_GAIN))->GetWindowText(strTemp);
    fInputGain = CAST_FLOAT(_ttof(strTemp));

    FrameGrabber::GetInstance().SetDigitalGainValue(fInputGain, FALSE);
}
