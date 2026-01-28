//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CNGRVDioView.h"

//CPP_2_________________________________ This project's headers
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
enum enumTimerIndex
{
    REFRESH = 1,
    VISION_READY,
    VISION_ACQUISITION,
    VISION_ACQUISITION_FAST,
    VISION_RCV_PKG_ID,
    VISION_RCV_GRAB_ID,
    VISION_MOVE_IR_POS,
};

//IMPLEMENT_DYNAMIC(CNGRVDioView, CDialog)

CNGRVDioView::CNGRVDioView(SyncController_Base* pciSync)
    : CDialog(CNGRVDioView::IDD, NULL)
    , m_pSync(pciSync)

{
}

CNGRVDioView::~CNGRVDioView()
{
}

void CNGRVDioView::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CNGRVDioView)
    DDX_Control(pDX, IDC_STATIC_NGRV_VISION_READY, m_Label_NGRV_VisionReady);
    DDX_Control(pDX, IDC_STATIC_NGRV_VISION_ACQUISITION, m_Label_NGRV_VisionAcquisition);
    DDX_Control(pDX, IDC_STATIC_NGRV_ACQUISITION_FAST, m_Label_NGRV_VisionAcquisition_Fast);
    DDX_Control(pDX, IDC_STATIC_NGRV_VISION_RCV_PKG_ID_DONE, m_Label_NGRV_VisionRCV_PKG_ID);
    DDX_Control(pDX, IDC_STATIC_NGRV_VISION_RCV_DEFECT_ID_DONE, m_Label_NGRV_VisionRCV_Grab_ID);

    DDX_Control(pDX, IDC_STATIC_NGRV_HANDLER_START, m_Label_NGRV_Handler_Start);
    DDX_Control(pDX, IDC_STATIC_NGRV_START_IR_FRAME, m_Label_NGRV_Handler_Start_IR);
    DDX_Control(pDX, IDC_STATIC_NGRV_SEND_GRAB_ID, m_Label_NGRV_HandlerSend_Grab_ID);

    //	Output Bits
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_0, m_Label_NGRV_Output_Raw_Bits[0]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_1, m_Label_NGRV_Output_Raw_Bits[1]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_2, m_Label_NGRV_Output_Raw_Bits[2]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_3, m_Label_NGRV_Output_Raw_Bits[3]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_4, m_Label_NGRV_Output_Raw_Bits[4]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_5, m_Label_NGRV_Output_Raw_Bits[5]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_6, m_Label_NGRV_Output_Raw_Bits[6]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_7, m_Label_NGRV_Output_Raw_Bits[7]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_8, m_Label_NGRV_Output_Raw_Bits[8]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_9, m_Label_NGRV_Output_Raw_Bits[9]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_10, m_Label_NGRV_Output_Raw_Bits[10]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_11, m_Label_NGRV_Output_Raw_Bits[11]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_12, m_Label_NGRV_Output_Raw_Bits[12]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_13, m_Label_NGRV_Output_Raw_Bits[13]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_14, m_Label_NGRV_Output_Raw_Bits[14]);
    DDX_Control(pDX, IDC_STATIC_NGRV_OUTPUT_15, m_Label_NGRV_Output_Raw_Bits[15]);

    //	Input Bits
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_0, m_Label_NGRV_Input_Raw_Bits[0]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_1, m_Label_NGRV_Input_Raw_Bits[1]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_2, m_Label_NGRV_Input_Raw_Bits[2]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_3, m_Label_NGRV_Input_Raw_Bits[3]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_4, m_Label_NGRV_Input_Raw_Bits[4]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_5, m_Label_NGRV_Input_Raw_Bits[5]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_6, m_Label_NGRV_Input_Raw_Bits[6]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_7, m_Label_NGRV_Input_Raw_Bits[7]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_8, m_Label_NGRV_Input_Raw_Bits[8]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_9, m_Label_NGRV_Input_Raw_Bits[9]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_10, m_Label_NGRV_Input_Raw_Bits[10]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_11, m_Label_NGRV_Input_Raw_Bits[11]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_12, m_Label_NGRV_Input_Raw_Bits[12]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_13, m_Label_NGRV_Input_Raw_Bits[13]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_14, m_Label_NGRV_Input_Raw_Bits[14]);
    DDX_Control(pDX, IDC_STATIC_NGRV_INPUT_15, m_Label_NGRV_Input_Raw_Bits[15]);

    DDX_Control(pDX, IDC_EDIT_NGRV_NUMERICAL_ID, m_Edit_Numerical_ID);

    DDX_Control(pDX, IDC_STATIC_NGRV_MOVE_IR_POS, m_Label_NGRV_Move_IR_Pos);
}

BEGIN_MESSAGE_MAP(CNGRVDioView, CDialog)
ON_WM_TIMER()
ON_WM_CLOSE()
ON_BN_CLICKED(IDC_STATIC_NGRV_VISION_READY, &CNGRVDioView::OnStaticNGRV_VisionReady)
ON_BN_CLICKED(IDC_STATIC_NGRV_VISION_ACQUISITION, &CNGRVDioView::OnStaticNGRV_VisionAcquisition)
ON_BN_CLICKED(IDC_STATIC_NGRV_ACQUISITION_FAST, &CNGRVDioView::OnStaticNGRV_AcquisitionFast)
ON_BN_CLICKED(IDC_STATIC_NGRV_VISION_RCV_PKG_ID_DONE, &CNGRVDioView::OnStaticNGRV_VisionRCV_PGV_ID_Done)
ON_BN_CLICKED(IDC_STATIC_NGRV_VISION_RCV_DEFECT_ID_DONE, &CNGRVDioView::OnStaticNGRV_VisionRCV_Grab_ID_Done)
ON_STN_CLICKED(IDC_STATIC_NGRV_MOVE_IR_POS, &CNGRVDioView::OnStaticNGRV_Move_IR_Pos)
END_MESSAGE_MAP()

// CNGRVDioView 메시지 처리기

BOOL CNGRVDioView::OnInitDialog()
{
    CDialog::OnInitDialog();

    GetSyncVersion();

    SetTimer(enumTimerIndex::REFRESH, 10, NULL);

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CNGRVDioView::GetSyncVersion()
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

void CNGRVDioView::OnTimer(UINT_PTR nIDEvent)
{
    switch (nIDEvent)
    {
        case enumTimerIndex::REFRESH:
        {
            const auto defColor = ::GetSysColor(COLOR_3DFACE);

            m_Label_NGRV_Handler_Start.SetBkColor(m_pSync->GetStartSignal() ? RGB(0, 255, 0) : defColor);
            m_Label_NGRV_Handler_Start_IR.SetBkColor(m_pSync->GetSignalNGRV_StartIR() ? RGB(0, 255, 0) : defColor);

            m_Label_NGRV_HandlerSend_Grab_ID.SetBkColor(m_pSync->GetSignalNGRV_GrabID() ? RGB(0, 255, 0) : defColor);

            m_Label_NGRV_VisionReady.SetBkColor(m_pSync->GetReadySignal() ? RGB(0, 255, 0) : defColor);
            m_Label_NGRV_VisionAcquisition.SetBkColor(m_pSync->GetGrabExposure() ? RGB(0, 255, 0) : defColor);
            m_Label_NGRV_VisionAcquisition_Fast.SetBkColor(m_pSync->GetGrabAcquisition() ? RGB(0, 255, 0) : defColor);
            m_Label_NGRV_VisionRCV_PKG_ID.SetBkColor(
                m_pSync->GetSignalNGRV_VisionRecvPkgID() ? RGB(0, 255, 0) : defColor);
            m_Label_NGRV_HandlerSend_Grab_ID.SetBkColor(
                m_pSync->GetSignalNGRV_VisionRecvGrabID() ? RGB(0, 255, 0) : defColor);

            m_Label_NGRV_Move_IR_Pos.SetBkColor(m_pSync->GetSignalNGRV_VisionMoveIRPos() ? RGB(0, 255, 0) : defColor);

            // Refresh Numerical Number : PKG ID, GRAB ID
            CString strNumericalNumPKG, strNumericalNumGrab;

            if (m_pSync->GetReadySignal() == TRUE && m_pSync->GetGrabAcquisition() == FALSE
                && m_pSync->GetStartSignal() == TRUE) // NGRV Ready On 일 때 H->V PKG ID를 보내준다
            {
                long nPKG_ID = m_pSync->GetFovIndex();
                strNumericalNumPKG.Format(_T("P:%02d"), nPKG_ID);
                GetDlgItem(IDC_EDIT_NGRV_NUMERICAL_ID)->SetWindowTextW(strNumericalNumPKG);
            }

            if (m_pSync->GetReadySignal() == FALSE && m_pSync->GetSignalNGRV_GrabID() == TRUE)
            {
                long nGrab_ID = m_pSync->GetFovIndex();
                strNumericalNumGrab.Format(_T("G:%02d"), nGrab_ID);
                GetDlgItem(IDC_EDIT_NGRV_NUMERICAL_ID)->SetWindowTextW(strNumericalNumGrab);
            }
            ///////////////////////////////////////////////

            // Refresh Stitch ID
            CString strStitchID;

            if (m_pSync->GetReadySignal() == FALSE
                && m_pSync->GetGrabAcquisition() == FALSE) // NGRV Ready Off 일 때 H->V Grab ID를 보내준다
            {
                long nStitch_ID = m_pSync->GetIndexStitchIDNGRV();
                strStitchID.Format(_T("%02d"), nStitch_ID);
                GetDlgItem(IDC_EDIT_NGRV_NUMERICAL_ID)->SetWindowTextW(strStitchID);
            }
            ///////////////////////////////////////////////

            for (long n = 0; n < 16; n++)
            {
                BOOL bOutput = FALSE;
                BYTE byByteIndex_Out = BYTE(n / 8);
                BYTE byBitMask_Out = 0x01 << BYTE(n % 8);
                m_pSync->GetOutputBit(byByteIndex_Out, byBitMask_Out, bOutput);

                m_Label_NGRV_Output_Raw_Bits[n].SetBkColor(bOutput ? RGB(0, 255, 0) : ::GetSysColor(15));

                BOOL bInput = FALSE;
                BYTE byByteIndex_In = BYTE(n / 8);
                BYTE byBitMask_In = 0x01 << BYTE(n % 8);
                m_pSync->GetInputBit(byByteIndex_In, byBitMask_In, bInput);
                m_Label_NGRV_Input_Raw_Bits[n].SetBkColor(bInput ? RGB(0, 255, 0) : ::GetSysColor(15));
            }
        }
        break;

        case enumTimerIndex::VISION_READY:
            OnStaticNGRV_VisionReady();
            break;

        case enumTimerIndex::VISION_ACQUISITION:
            OnStaticNGRV_VisionAcquisition();
            break;

        case enumTimerIndex::VISION_ACQUISITION_FAST:
            OnStaticNGRV_AcquisitionFast();
            break;

        case enumTimerIndex::VISION_RCV_PKG_ID:
            OnStaticNGRV_VisionRCV_PGV_ID_Done(); //kircheis_NGRV_Sync
            break;

        case enumTimerIndex::VISION_RCV_GRAB_ID:
            OnStaticNGRV_VisionRCV_Grab_ID_Done(); //kircheis_NGRV_Sync
            break;

        case enumTimerIndex::VISION_MOVE_IR_POS:
            OnStaticNGRV_Move_IR_Pos(); //kircheis_NGRV_Sync
            break;
    }

    CDialog::OnTimer(nIDEvent);
}

void CNGRVDioView::OnClose()
{
    KillTimer(enumTimerIndex::REFRESH);
    KillTimer(enumTimerIndex::VISION_READY);
    KillTimer(enumTimerIndex::VISION_ACQUISITION);
    KillTimer(enumTimerIndex::VISION_ACQUISITION_FAST);
    KillTimer(enumTimerIndex::VISION_RCV_PKG_ID);
    KillTimer(enumTimerIndex::VISION_RCV_GRAB_ID);
    KillTimer(enumTimerIndex::VISION_MOVE_IR_POS);

    //CDialog::OnClose();
    OnOK();
}

void CNGRVDioView::OnStaticNGRV_VisionReady()
{
    m_pSync->SetReadySignal(!m_pSync->GetReadySignal()); //kircheis_NGRV_Sync
}

void CNGRVDioView::OnStaticNGRV_VisionAcquisition()
{
    m_pSync->SetGrabExposure(!m_pSync->GetGrabExposure()); //kircheis_NGRV_Sync
}

void CNGRVDioView::OnStaticNGRV_AcquisitionFast()
{
    m_pSync->SetGrabAcquisition(!m_pSync->GetGrabAcquisition()); //kircheis_NGRV_Sync
}

void CNGRVDioView::OnStaticNGRV_VisionRCV_PGV_ID_Done()
{
    m_pSync->SetSignalNGRV_VisionRecvPkgID(!m_pSync->GetSignalNGRV_VisionRecvPkgID()); //kircheis_NGRV_Sync
}

void CNGRVDioView::OnStaticNGRV_VisionRCV_Grab_ID_Done()
{
    m_pSync->SetSignalNGRV_VisionRecvGrabID(!m_pSync->GetSignalNGRV_VisionRecvGrabID()); //kircheis_NGRV_Sync
}

void CNGRVDioView::OnStaticNGRV_Move_IR_Pos()
{
    m_pSync->SetSignalNGRV_VisionMoveIRPos(!m_pSync->GetSignalNGRV_VisionMoveIRPos()); //kircheis_NGRV_Sync
}