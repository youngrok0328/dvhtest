//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DioView.h"

//CPP_2_________________________________ This project's headers
#include "SyncController.h"
#include "SyncController_Base.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
enum TimerIndex
{
    Refresh = 1,
    Repeat_Ready,
    Repeat_Acquisition,
    Repeat_Exposure,
};

CDioView::CDioView(SyncController_Base* pciSync)
    : CDialog(CDioView::IDD, NULL)
    , m_pSync(pciSync)
{
}

CDioView::~CDioView()
{
}

void CDioView::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDioView)
    DDX_Control(pDX, IDC_STATIC_VISION_READY, m_Label_VisionReady);
    DDX_Control(pDX, IDC_STATIC_VISION_ACQUISITION, m_Label_VisionAcquisition);
    DDX_Control(pDX, IDC_STATIC_VISION_EXPOSURE, m_Label_VisionExposure);
    DDX_Control(pDX, IDC_STATIC_HANDLER_START, m_Label_HandlerStart);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_0, m_Label_HandlerBit[0]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_1, m_Label_HandlerBit[1]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_2, m_Label_HandlerBit[2]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_3, m_Label_HandlerBit[3]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_4, m_Label_HandlerBit[4]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_5, m_Label_HandlerBit[5]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_6, m_Label_HandlerBit[6]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_7, m_Label_HandlerBit[7]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_8, m_Label_HandlerBit[8]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_BIT_9, m_Label_HandlerBit[9]);
    DDX_Control(pDX, IDC_STATIC_HANDLER_POCKET_ID, m_Label_HandlerPocketID);
    DDX_Control(pDX, IDC_STATIC_HANDLER_ENCODER_COUNT, m_Label_HandlerEncoderCnt);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_0, m_Label_OutputRawBits[0]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_1, m_Label_OutputRawBits[1]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_2, m_Label_OutputRawBits[2]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_3, m_Label_OutputRawBits[3]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_4, m_Label_OutputRawBits[4]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_5, m_Label_OutputRawBits[5]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_6, m_Label_OutputRawBits[6]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_7, m_Label_OutputRawBits[7]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_8, m_Label_OutputRawBits[8]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_9, m_Label_OutputRawBits[9]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_10, m_Label_OutputRawBits[10]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_11, m_Label_OutputRawBits[11]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_12, m_Label_OutputRawBits[12]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_13, m_Label_OutputRawBits[13]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_14, m_Label_OutputRawBits[14]);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_15, m_Label_OutputRawBits[15]);
    DDX_Control(pDX, IDC_STATIC_INPUT_0, m_Label_InputRawBits[0]);
    DDX_Control(pDX, IDC_STATIC_INPUT_1, m_Label_InputRawBits[1]);
    DDX_Control(pDX, IDC_STATIC_INPUT_2, m_Label_InputRawBits[2]);
    DDX_Control(pDX, IDC_STATIC_INPUT_3, m_Label_InputRawBits[3]);
    DDX_Control(pDX, IDC_STATIC_INPUT_4, m_Label_InputRawBits[4]);
    DDX_Control(pDX, IDC_STATIC_INPUT_5, m_Label_InputRawBits[5]);
    DDX_Control(pDX, IDC_STATIC_INPUT_6, m_Label_InputRawBits[6]);
    DDX_Control(pDX, IDC_STATIC_INPUT_7, m_Label_InputRawBits[7]);
    DDX_Control(pDX, IDC_STATIC_INPUT_8, m_Label_InputRawBits[8]);
    DDX_Control(pDX, IDC_STATIC_INPUT_9, m_Label_InputRawBits[9]);
    DDX_Control(pDX, IDC_STATIC_INPUT_10, m_Label_InputRawBits[10]);
    DDX_Control(pDX, IDC_STATIC_INPUT_11, m_Label_InputRawBits[11]);
    DDX_Control(pDX, IDC_STATIC_INPUT_12, m_Label_InputRawBits[12]);
    DDX_Control(pDX, IDC_STATIC_INPUT_13, m_Label_InputRawBits[13]);
    DDX_Control(pDX, IDC_STATIC_INPUT_14, m_Label_InputRawBits[14]);
    DDX_Control(pDX, IDC_STATIC_INPUT_15, m_Label_InputRawBits[15]);
    DDX_Control(pDX, IDC_CHECK_REPEAT_READY, m_checkRepeatReady);
    DDX_Control(pDX, IDC_CHECK_REPEAT_ACQUISITION, m_checkRepeatAcquisition);
    DDX_Control(pDX, IDC_CHECK_REPEAT_EXPOSURE, m_checkRepeatExposure);
}

BEGIN_MESSAGE_MAP(CDioView, CDialog)
//{{AFX_MSG_MAP(CDioView)
ON_BN_CLICKED(IDC_STATIC_VISION_ACQUISITION, OnStaticVisionAcquisition)
ON_BN_CLICKED(IDC_STATIC_VISION_EXPOSURE, OnStaticVisionExposure)
ON_BN_CLICKED(IDC_STATIC_VISION_READY, OnStaticVisionReady)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_0, OnStaticOutput_0)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_1, OnStaticOutput_1)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_2, OnStaticOutput_2)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_3, OnStaticOutput_3)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_4, OnStaticOutput_4)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_5, OnStaticOutput_5)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_6, OnStaticOutput_6)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_7, OnStaticOutput_7)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_8, OnStaticOutput_8)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_9, OnStaticOutput_9)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_10, OnStaticOutput_A)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_11, OnStaticOutput_B)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_12, OnStaticOutput_C)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_13, OnStaticOutput_D)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_14, OnStaticOutput_E)
ON_BN_CLICKED(IDC_STATIC_OUTPUT_15, OnStaticOutput_F)
ON_WM_TIMER()
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_CHECK_REPEAT_READY, &CDioView::OnBnClickedCheckRepeatReady)
ON_BN_CLICKED(IDC_CHECK_REPEAT_ACQUISITION, &CDioView::OnBnClickedCheckRepeatAcquisition)
ON_BN_CLICKED(IDC_CHECK_REPEAT_EXPOSURE, &CDioView::OnBnClickedCheckRepeatExposure)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDioView message handlers
BOOL CDioView::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetSyncVersion();

    SetTimer(TimerIndex::Refresh, 10, NULL);

    return TRUE; // return TRUE unless you set the focus to a control
        // EXCEPTION: OCX Property Pages should return FALSE
}

void CDioView::OnTimer(UINT_PTR nIDEvent)
{
    static const bool bIsSideVision = m_pSync->GetVisionType() == VISIONTYPE_SIDE_INSP;

    switch (nIDEvent)
    {
        case TimerIndex::Refresh:
        {
            const auto defColor = ::GetSysColor(COLOR_3DFACE);

            m_Label_HandlerStart.SetBkColor(m_pSync->GetStartSignal() ? RGB(0, 255, 0) : defColor);
            m_Label_VisionReady.SetBkColor(m_pSync->GetReadySignal() ? RGB(0, 255, 0) : defColor);
            m_Label_VisionAcquisition.SetBkColor(m_pSync->GetGrabAcquisition() ? RGB(0, 255, 0) : defColor);
            m_Label_VisionExposure.SetBkColor(m_pSync->GetGrabExposure() ? RGB(0, 255, 0) : defColor);

            UINT fullBit = m_pSync->GetFovIndex();
            UINT fovIndex = bIsSideVision ? m_pSync->GetScanIDforSide() : fullBit;
            UINT stitchIndex = m_pSync->GetStitchingIndexForSide();

            for (long nBit = 0; nBit < 10; nBit++)
            {
                m_Label_HandlerBit[nBit].SetBkColor((fullBit & (0x01 << nBit)) ? RGB(0, 255, 0) : defColor);
            }

            CString str;

            if (bIsSideVision == true)
            {
                str.Format(_T("FOV[%02d], ST[%02d]"), fovIndex, stitchIndex);
            }
            else
            {
                str.Format(_T("FOV[%02d]"), fovIndex);
            }

            m_Label_HandlerPocketID.SetText(str);

            for (long n = 0; n < 16; n++)
            {
                BOOL bOutput = FALSE;
                BYTE byByteIndex_Out = BYTE(n / 8);
                BYTE byBitMask_Out = 0x01 << BYTE(n % 8);
                m_pSync->GetOutputBit(byByteIndex_Out, byBitMask_Out, bOutput);

                m_Label_OutputRawBits[n].SetBkColor(bOutput ? RGB(0, 255, 0) : ::GetSysColor(15));

                BOOL bInput = FALSE;
                //BYTE byByteIndex_In = BYTE(n / 8);
                BYTE byBitMask_In = 0x01 << BYTE(n % 8);
                m_pSync->GetInputBit(byBitMask_In, byBitMask_In, bInput);
                m_Label_InputRawBits[n].SetBkColor(bInput ? RGB(0, 255, 0) : ::GetSysColor(15));
            }

            //{{//kircheis_GrabDir
            int nEncoderCnt(0), nEncoderCnt_New(0);
            nEncoderCnt = m_pSync->ReadCount(nEncoderCnt_New);
            str.Format(_T("%d , %d"), nEncoderCnt, nEncoderCnt_New);
            //}}

            m_Label_HandlerEncoderCnt.SetText(str);
        }
        break;
        case TimerIndex::Repeat_Ready:
            OnStaticVisionReady();
            break;
        case TimerIndex::Repeat_Acquisition:
            OnStaticVisionAcquisition();
            break;
        case TimerIndex::Repeat_Exposure:
            OnStaticVisionExposure();
            break;
    }

    CDialog::OnTimer(nIDEvent);
}

void CDioView::OnClose()
{
    KillTimer(TimerIndex::Refresh);
    KillTimer(TimerIndex::Repeat_Ready);
    KillTimer(TimerIndex::Repeat_Acquisition);
    KillTimer(TimerIndex::Repeat_Exposure);

    OnOK();
}

void CDioView::OnStaticVisionAcquisition()
{
    m_pSync->SetGrabAcquisition(!m_pSync->GetGrabAcquisition());
}

void CDioView::OnStaticVisionExposure()
{
    m_pSync->SetGrabExposure(!m_pSync->GetGrabExposure());
}

void CDioView::OnStaticVisionReady()
{
    m_pSync->SetReadySignal(!m_pSync->GetReadySignal());
}

void CDioView::InvertOutput(long nBitOrder)
{
    BOOL bOutput = FALSE;

    SyncController::GetInstance().GetOutputBit(nBitOrder, bOutput);
    SyncController::GetInstance().SetOutputBit(nBitOrder, !bOutput);

    //m_pSync->GetOutputBit(nBitOrder, bOutput);
    //m_pSync->SetOutputBit(nBitOrder, !bOutput);
}

void CDioView::OnStaticOutput_0()
{
    InvertOutput(0);
}
void CDioView::OnStaticOutput_1()
{
    InvertOutput(1);
}
void CDioView::OnStaticOutput_2()
{
    InvertOutput(2);
}
void CDioView::OnStaticOutput_3()
{
    InvertOutput(3);
}
void CDioView::OnStaticOutput_4()
{
    InvertOutput(4);
}
void CDioView::OnStaticOutput_5()
{
    InvertOutput(5);
}
void CDioView::OnStaticOutput_6()
{
    InvertOutput(6);
}
void CDioView::OnStaticOutput_7()
{
    InvertOutput(7);
}
void CDioView::OnStaticOutput_8()
{
    InvertOutput(8);
}
void CDioView::OnStaticOutput_9()
{
    InvertOutput(9);
}
void CDioView::OnStaticOutput_A()
{
    InvertOutput(10);
}
void CDioView::OnStaticOutput_B()
{
    InvertOutput(11);
}
void CDioView::OnStaticOutput_C()
{
    InvertOutput(12);
}
void CDioView::OnStaticOutput_D()
{
    InvertOutput(13);
}
void CDioView::OnStaticOutput_E()
{
    InvertOutput(14);
}
void CDioView::OnStaticOutput_F()
{
    InvertOutput(15);
}

void CDioView::GetSyncVersion()
{
    CString strWindowText;

    BYTE byCPLDReadHighVal;
    BYTE byCPLDReadLowVal;
    BYTE byReadHighVal;
    BYTE byReadLowVal;

    m_pSync->ReadCPLDVersion(byCPLDReadHighVal, byCPLDReadLowVal);

    m_pSync->ReadAVRVersion(byReadHighVal, byReadLowVal);

    CString strCPLDReadHighVal;
    CString strCPLDReadLowVal;
    CString strReadHighVal;
    CString strReadLowVal;

    if (byCPLDReadHighVal == 0)
    {
        strCPLDReadHighVal = _T("00");
    }
    else
    {
        strCPLDReadHighVal.Format(_T("%02x"), byCPLDReadHighVal);
    }

    if (byCPLDReadLowVal == 0)
    {
        strCPLDReadLowVal = _T("00");
    }
    else
    {
        strCPLDReadLowVal.Format(_T("%02x"), byCPLDReadLowVal);
    }

    if (byReadHighVal == 0)
    {
        strReadHighVal = _T("00");
    }
    else
    {
        strReadHighVal.Format(_T("%02x"), byReadHighVal);
    }

    if (byReadLowVal == 0)
    {
        strReadLowVal = _T("00");
    }
    else
    {
        strReadLowVal.Format(_T("%02x"), byReadLowVal);
    }

    strWindowText.Format(_T("CPLD Version : %s.%s,    AVR Version : %s.%s"), (LPCTSTR)strCPLDReadLowVal,
        (LPCTSTR)strCPLDReadHighVal, (LPCTSTR)strReadLowVal, (LPCTSTR)strReadHighVal);

    strWindowText.MakeUpper();
    SetWindowText(strWindowText);
}

void CDioView::OnBnClickedCheckRepeatReady()
{
    if (m_checkRepeatReady.GetCheck() == BST_CHECKED)
    {
        SetTimer(TimerIndex::Repeat_Ready, 1000, nullptr);
    }
    else
    {
        KillTimer(TimerIndex::Repeat_Ready);
    }
}

void CDioView::OnBnClickedCheckRepeatAcquisition()
{
    if (m_checkRepeatAcquisition.GetCheck() == BST_CHECKED)
    {
        SetTimer(TimerIndex::Repeat_Acquisition, 1000, nullptr);
    }
    else
    {
        KillTimer(TimerIndex::Repeat_Acquisition);
    }
}

void CDioView::OnBnClickedCheckRepeatExposure()
{
    if (m_checkRepeatExposure.GetCheck() == BST_CHECKED)
    {
        SetTimer(TimerIndex::Repeat_Exposure, 1000, nullptr);
    }
    else
    {
        KillTimer(TimerIndex::Repeat_Exposure);
    }
}
