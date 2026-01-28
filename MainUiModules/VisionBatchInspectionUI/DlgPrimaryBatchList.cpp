//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgPrimaryBatchList.h"

//CPP_2_________________________________ This project's headers
#include "BatchManager.h"
#include "DlgSaveMomoryLogInterval.h"
#include "DlgSelectVision.h"
#include "DlgVisionBatchInspectionUI.h"
#include "DlgYieldViewer.h"
#include "ImageFileParser.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"
#include "../../InformationModule/dPI_SystemIni/PersonalConfig.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../InspectionItems/VisionInspectionSurface/InspBase.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurface.h"
#include "../../InspectionItems/VisionInspectionSurface/VisionInspectionSurfacePara.h"
#include "../../ManagementModules/VisionUnit/VisionUnit.h"
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"
#include "../../SharedCommunicationModules/VisionHostCommon/ArchiveAllType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionDeviceResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../SharedComponent/Persistence/IniHelper.h"
#include "../../UserInterfaceModules/CommonControlExtension/AutoHidePopup.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonResultDlg.h"
#include "../../UserInterfaceModules/VisionCommonDialog/VisionCommonTextResultDlg.h"
#include "../../UtilityMacroFunction.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLot.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionImageLotInsp.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspectionOverlayResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionMainAgent.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/ImageFile.h>
#include <Ipvm/Base/Rect32s.h>
#include <Ipvm/Gadget/Miscellaneous.h>
#include <Ipvm/Widget/AsyncProgress.h>
#include <afxmt.h>
#include <io.h>
#include <psapi.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_GRID_BATCH_LIST 100
#define TIMER_MEM_LOG 10
#define TIMER_MEM_LOG_20MIN 9999
#define UM_INSPECTION_END (WM_USER + 100)

//CPP_7_________________________________ Implementation body
//
#pragma comment(lib, "Psapi.lib")

static long snInspCount = 0;
static long snReportidx = 0;

BatchManager g_batchManager;

IMPLEMENT_DYNAMIC(CDlgPrimaryBatchList, CDialog)

CDlgPrimaryBatchList::CDlgPrimaryBatchList(
    VisionMainAgent& visionMainAgent, VisionUnit& visionUnit, const CRect& rtPositionOnParent, CWnd* pParent /*=NULL*/)
    : CDialog(CDlgPrimaryBatchList::IDD, pParent)
    , m_visionMainAgent(visionMainAgent)
    , m_visionUnit(visionUnit)
    , m_rtPositionOnParent(rtPositionOnParent)
    , m_state(BatchState::Ready)
    , m_runState(BatchRunState::Idle)
    , m_batchInspTime(0)
    , m_bIsRearModuleInspection(false)
{
    m_pDlgVisionBatchInspectionUI = (DlgVisionBatchInspectionUI*)pParent;
    m_pgridBatch = NULL;
    m_bSaveLogMemory = FALSE;
    m_pbyDevResultSaveBuffer = NULL;
    m_pbyDevResultSaveBuffer = new BYTE[MAX_SEND_RESULT_SIZE];
}

CDlgPrimaryBatchList::~CDlgPrimaryBatchList()
{
    m_visionMainAgent.UnRegisterCallBack_InspectionEnd(callBack_InspectionEnd);
    delete m_pgridBatch;
    if (m_pbyDevResultSaveBuffer != NULL)
    {
        delete[] m_pbyDevResultSaveBuffer;
        m_pbyDevResultSaveBuffer = NULL;
    }
}

void CDlgPrimaryBatchList::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_CLEAR_ALL, m_buttonClearAll);
    DDX_Control(pDX, IDC_BUTTON_AUTO_INSPECT, m_buttonAutoInspect);
    DDX_Control(pDX, IDC_COMBO_SIDE_F_R_MODULE, m_ctrlCmbSideModuleSelector);
}

BEGIN_MESSAGE_MAP(CDlgPrimaryBatchList, CDialog)
ON_BN_CLICKED(IDC_BUTTON_CLEAR_ALL, &CDlgPrimaryBatchList::OnBnClickedButtonClearAll)
ON_NOTIFY(NM_DBLCLK, IDC_GRID_BATCH_LIST, OnDblclkGridBatchList)
ON_BN_CLICKED(IDC_BUTTON_AUTO_INSPECT, &CDlgPrimaryBatchList::OnBnClickedButtonAutoInspect)
ON_WM_TIMER()
ON_BN_CLICKED(IDC_BUTTON_STOP, &CDlgPrimaryBatchList::OnBnClickedButtonStop)
ON_BN_CLICKED(IDC_BUTTON_RESUME, &CDlgPrimaryBatchList::OnBnClickedButtonResume)
ON_BN_CLICKED(IDC_BUTTON_ASORT, &CDlgPrimaryBatchList::OnBnClickedButtonAsort)
ON_BN_CLICKED(IDC_BUTTON_DSORT, &CDlgPrimaryBatchList::OnBnClickedButtonDsort)
ON_BN_CLICKED(IDC_BUTTON_DELETE, &CDlgPrimaryBatchList::OnBnClickedButtonDelete)
ON_BN_CLICKED(IDC_BUTTON_YIELD, &CDlgPrimaryBatchList::OnBnClickedButtonYield)
ON_BN_CLICKED(IDC_BUTTON_INFINITE_INSP, &CDlgPrimaryBatchList::OnBnClickedButtonInfiniteInsp)
ON_WM_DESTROY()
ON_MESSAGE(UM_INSPECTION_END, OnInspectionEnd)
ON_BN_CLICKED(IDC_BUTTON_BARCODE, &CDlgPrimaryBatchList::OnBnClickedButtonBarcode)
ON_CBN_SELCHANGE(IDC_COMBO_SIDE_F_R_MODULE, &CDlgPrimaryBatchList::OnCbnSelchangeComboSideFRModule)
END_MESSAGE_MAP()

// CDlgPrimaryBatchList 메시지 처리기입니다.
BOOL CDlgPrimaryBatchList::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_visionMainAgent.RegisterCallBack_InspectionEnd(this, callBack_InspectionEnd);

    m_pDlgVisionBatchInspectionUI->m_imageLotView->ShowImage(g_batchManager.m_lastFrameIndex);

    MoveWindow(m_rtPositionOnParent);

    m_pgridBatch = new CGridCtrl;
    m_pgridBatch->EnableDragAndDrop(TRUE);

    CRect rtClient;
    GetClientRect(rtClient);

    CRect rtAutoButton;
    GetDlgItem(IDC_BUTTON_AUTO_INSPECT)->GetWindowRect(rtAutoButton);
    ScreenToClient(rtAutoButton);

    rtClient.top = rtAutoButton.bottom + 3;

    m_pgridBatch->Create(rtClient, this, IDC_GRID_BATCH_LIST);
    m_pgridBatch->SetEditable(TRUE);
    m_pgridBatch->EnableDragAndDrop(FALSE);

    // 임시로 가로 5줄, 세로 3줄 설정
    m_pgridBatch->SetColumnCount(BatchPreColumn_InspItemStart);
    m_pgridBatch->SetRowCount(2);
    m_pgridBatch->SetFixedRowCount(2);
    m_pgridBatch->SetFixedColumnCount(2);
    m_pgridBatch->SetEditable(FALSE);

    m_pgridBatch->SetColumnWidth(BatchPreColumn_Index, 40);
    m_pgridBatch->SetColumnWidth(BatchPreColumn_FileName, 300);
    m_pgridBatch->SetColumnWidth(BatchPreColumn_Barcode, 0);
    m_pgridBatch->SetColumnWidth(BatchPreColumn_ScanID, 40);
    m_pgridBatch->SetColumnWidth(BatchPreColumn_PaneID, 40);
    m_pgridBatch->SetColumnWidth(BatchPreColumn_TotalResult, 80);

    // 0 번째 Row 에 각각의 Title 을 정의
    m_pgridBatch->SetItemText(0, BatchPreColumn_Index, _T("No"));
    m_pgridBatch->SetItemText(0, BatchPreColumn_FileName, _T("File Name"));
    m_pgridBatch->SetItemText(0, BatchPreColumn_Barcode, _T("Barcode"));
    m_pgridBatch->SetItemText(0, BatchPreColumn_ScanID, _T("Scan"));
    m_pgridBatch->SetItemText(0, BatchPreColumn_PaneID, _T("Pane"));
    m_pgridBatch->SetItemText(0, BatchPreColumn_TotalResult, _T("Total Result"));

    for (long columnIndex = 0; columnIndex < BatchPreColumn_InspItemStart; columnIndex++)
    {
        m_pgridBatch->SetItemBkColour(0, columnIndex, RGB(200, 200, 250));
        m_pgridBatch->MergeCells(0, columnIndex, 1, BatchPreColumn_Index);
    }

    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);
    if (bIsSideVision)
    {
        long nSideModuleSelection = (long)m_visionUnit.GetSideVisionSection();
        m_ctrlCmbSideModuleSelector.SetCurSel(nSideModuleSelection);
        m_bIsRearModuleInspection = (bool)(nSideModuleSelection == (long)(enSideVisionModule::SIDE_VISIONMODULE_REAR));

        m_pDlgVisionBatchInspectionUI->m_imageLotView->SetSideVisionModule((enSideVisionModule)nSideModuleSelection);
        m_visionUnit.SetSideVisionSection((enSideVisionModule)nSideModuleSelection);
        m_visionMainAgent.SetSideVisionSection((enSideVisionModule)nSideModuleSelection);
    }
    else
        m_ctrlCmbSideModuleSelector.ShowWindow(SW_HIDE);

    CString intervalTime;
    intervalTime.Format(_T("%d"), PersonalConfig::getInstance().getSequenceIntervalTimeInBatchInspection());
    GetDlgItem(IDC_EDIT_SequenceInterrvalTime)->SetWindowText(intervalTime);

    // Grid 설정
    Refresh();
    g_batchManager.UpdateGridCtrl_List(*m_pgridBatch);
    g_batchManager.UpdateGridCtrl_Batch(*m_pgridBatch);

    SetState(BatchState::Ready);

    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CDlgPrimaryBatchList::Refresh()
{
    // Batch inspection을 진행하기 전에 recipe open 하며 받은 파라미터를 메모리에 올렸다가 다시 받아서 사용한다.
    CiDataBase db;
    m_visionMainAgent.LinkDataBase(TRUE, db);
    m_visionMainAgent.LinkDataBase(FALSE, db);

    auto visionInspectionSpecs = m_visionUnit.GetVisionInspectionSpecs();
    auto visionInspectionResults = m_visionUnit.GetVisionInspectionResults();

    // 임시로 가로 5줄, 세로 3줄 설정
    m_pgridBatch->SetColumnCount(BatchPreColumn_InspItemStart + long(visionInspectionResults.size()));

    if ((long)visionInspectionResults.size() == (long)visionInspectionSpecs.size())
    {
        long i;
        long nNameCount = 0;
        long nCellWidth = 0;
        CString strSpecName;
        for (i = 0; i < (long)(visionInspectionResults.size()); i++)
        {
            strSpecName = visionInspectionResults[i]->m_resultName;
            nNameCount = strSpecName.GetLength();
            nCellWidth = ((nNameCount * 7) > 100) ? nNameCount * 7 : 100;

            long itemColumnIndex = BatchPreColumn_InspItemStart + i;
            m_pgridBatch->SetColumnWidth(itemColumnIndex, nCellWidth);
            m_pgridBatch->SetItemText(0, itemColumnIndex, visionInspectionResults[i]->m_inspName);
            m_pgridBatch->SetItemText(1, itemColumnIndex, visionInspectionResults[i]->m_resultName);

            m_pgridBatch->SetItemBkColour(0, itemColumnIndex, RGB(200, 200, 250));

            if (visionInspectionSpecs[i]->m_use)
            {
                m_pgridBatch->SetColumnWidth(itemColumnIndex, nCellWidth);
            }
            else
            {
                m_pgridBatch->SetColumnWidth(itemColumnIndex, 0);
            }
        }
    }

    // 모든 Cell 을 가운데 정렬
    for (long i = 0; i < m_pgridBatch->GetRowCount(); i++)
    {
        for (long j = 0; j < m_pgridBatch->GetColumnCount(); j++)
        {
            m_pgridBatch->SetItemFormat(i, j, DT_CENTER);
        }
    }

    m_pgridBatch->Refresh();
}

void CDlgPrimaryBatchList::OnBnClickedButtonClearAll()
{
    // Batch List 변수 Update
    g_batchManager.Reset();

    // Batch List UI Update
    m_pgridBatch->DeleteNonFixedRows();
    m_pgridBatch->Refresh();
    m_pgridBatch->SetColumnWidth(BatchPreColumn_Barcode, 0);

    SetCurNum();

    SetState(BatchState::Ready);
}

void CDlgPrimaryBatchList::OnDblclkGridBatchList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
    if (m_state == BatchState::Run)
        return;

    CCellID CellID = m_pgridBatch->GetFocusCell();
    long nGridNum = (long)(m_pgridBatch->GetRowCount());
    CString strFile;
    CString strDir;
    long nScanID = 0;
    long nPaneID = 0;
    if (CellID.row >= 2 && CellID.row <= nGridNum - 1)
    {
        long nImage = _ttoi(m_pgridBatch->GetItemText(CellID.row, BatchPreColumn_Index)) - 1;
        strDir = g_batchManager.m_vecBatchList[nImage].strFileDir;
        strFile = m_pgridBatch->GetItemText(CellID.row, BatchPreColumn_FileName);
        CString strFullFilename = strDir + _T("\\") + strFile;

        nScanID = _ttoi(m_pgridBatch->GetItemText(CellID.row, BatchPreColumn_ScanID)) - 1;
        nPaneID = _ttoi(m_pgridBatch->GetItemText(CellID.row, BatchPreColumn_PaneID)) - 1;

        if (m_visionMainAgent.OpenImageFileAgent(strFullFilename))
        {
            // Primary Vision Unit을 Actvie
            ActiveVisionUnit(m_visionUnit);

            // Image Display Overlay 제거
            m_pDlgVisionBatchInspectionUI->m_imageLotView->Overlay_RemoveAll();
            m_pDlgVisionBatchInspectionUI->m_imageLotView->ShowCurrentImage();
            m_pDlgVisionBatchInspectionUI->m_imageLotView->ShowPane(nPaneID);
        }
    }

    SetCurNum();
    *pResult = 0;
}

void CDlgPrimaryBatchList::OnBnClickedButtonAutoInspect()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision)
    {
        enSideVisionModule sideVisionModule = (m_bIsRearModuleInspection == true)
            ? enSideVisionModule::SIDE_VISIONMODULE_REAR
            : enSideVisionModule::SIDE_VISIONMODULE_FRONT;

        m_pDlgVisionBatchInspectionUI->m_imageLotView->SetSideVisionModule(sideVisionModule);
        m_visionUnit.SetSideVisionSection(sideVisionModule);
    }

    g_batchManager.Inspect_StartReady(m_visionUnit);

    // 결과 클리어
    for (long row = 2; row < m_pgridBatch->GetRowCount(); row++)
    {
        m_pgridBatch->SetItemText(row, BatchPreColumn_TotalResult, _T(""));
        m_pgridBatch->SetItemBkColour(row, BatchPreColumn_TotalResult);

        for (long col = BatchPreColumn_InspItemStart; col < m_pgridBatch->GetColumnCount(); col++)
        {
            m_pgridBatch->SetItemText(row, col, _T(""));
            m_pgridBatch->SetItemBkColour(row, col);
        }
    }

    m_pgridBatch->Refresh();

    long nGridNum = (long)(m_pgridBatch->GetRowCount());

    if (nGridNum > 1)
    {
        g_batchManager.m_nGridNum = 2;
        g_batchManager.m_nImageNum = 0;

        SetState(BatchState::Run);
    }
}

bool CDlgPrimaryBatchList::DoInspection(long nGrid, long nPane, bool threadRun)
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    long dataIndex = _ttoi(m_pgridBatch->GetItemText(nGrid, BatchPreColumn_Index)) - 1;

    CString strDir = g_batchManager.m_vecBatchList[dataIndex].strFileDir;
    CString strFilename = g_batchManager.m_vecBatchList[dataIndex].strFileName;
    CString strBarcode = g_batchManager.m_barcodeData.GetBarcode(nPane, strFilename);
    CString strFullFilename = strDir + _T("\\") + strFilename;

    if (threadRun)
    {
        switch (m_visionMainAgent.PushBatchImage(nGrid, nPane, strFullFilename, strBarcode))
        {
            case -1:
                return false;

            case 0:
                return true;
        }
    }

    // Thread가 하나밖에 없거나 요청에 의해 Thread로 계산할 수 없다
    // Thread로 돌지 말자
    m_visionUnit.SetCurrentPaneID(nPane);
    m_visionUnit.getTrayScanSpec().SetBatchKey(nGrid, g_batchManager.m_barcodeData.GetBarcode(nPane, strFullFilename));
    m_visionUnit.OpenBmpImageFile(strFullFilename, true);

    unsigned __int64 nInspTime = GetTickCount64();
    if (bIsSideVision == true && m_bIsRearModuleInspection == true)
    {
        m_visionUnit.RunInspection(m_visionUnit.GetVisionProcessingByGuid(_VISION_INSP_GUID_ALIGN_2D),
            true); //이유는 모르겠는데 Front 없이 Rear만 검사하면 Surface Mask 생성 버퍼가 Raw 기준으로 만들어 진다.
        // Front에서 Align을 하고 Rear 검사하면 문제가 없다. Align이나 Surface 또는 GMM에 뭔가 키가 있을거 같긴한데...
        //Batch에서 Rear로 돌린 후 Surface Mask를 보면 Mask가 Shift 되어 있고 GMM에 갔다가 다시 Surface로 오면 정상화 된다...
        m_visionUnit.RunInspection(nullptr, false, enSideVisionModule::SIDE_VISIONMODULE_REAR);
    }
    else
        m_visionUnit.RunInspection();

    long nSingleInspTime = CAST_LONG(GetTickCount64() - nInspTime);
    m_batchInspTime += nSingleInspTime;

    VisionDeviceResult deviceResult;
    m_visionMainAgent.PostExecutionProcessing(true, m_visionUnit, deviceResult);

    // ksy collect reject information & image 09.28.20 KSY
    if (m_pDlgVisionBatchInspectionUI->m_visionUnit.m_systemConfig.m_bUseSaveCollectMVRejectData)
        CollectMVRejectData(deviceResult);

    // Send Deviceresult to shared memory
    BOOL b2ndInsp = FALSE;

    if (m_pDlgVisionBatchInspectionUI->m_visionUnit.m_systemConfig.m_bStateiDL)
        b2ndInsp = Send2ndInspInfo(deviceResult);

    callBack_InspectionEnd(this, m_visionUnit, -1);

    return true;
}

BOOL CDlgPrimaryBatchList::SaveAllInspImage(VisionDeviceResult& devResult, const enSideVisionModule i_eSideVisionModule)
{
    VisionImageLotInsp& lotInspImage = m_visionUnit.getImageLotInsp();
    //long nFrmNum = lotInspImage.GetImageFrameCount();

    CString strFolderPath = _T("D:\\intekfiles\\LearningImage\\");

    ::CreateDirectory(strFolderPath, NULL);

    strFolderPath = strFolderPath + _T("AllImages\\");
    ::CreateDirectory(strFolderPath, NULL);

    CString strFilePath = strFolderPath;
    CString strFileName, strFrameFolder, strAdd;

    long i{};

    if ((devResult.m_nTotalResult != NOT_MEASURED) && (devResult.m_nTotalResult != EMPTY))
    {
        long nImageSizeX, nImageSizeY, nDepth;

        for (i = 0; i < lotInspImage.m_vecImages[i_eSideVisionModule].size(); i++)
        {
            strFrameFolder.Format(_T("Frame[%d]"), i);
            ::CreateDirectory(strFolderPath + strFrameFolder, NULL);
            strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + _T(".BMP");
            const char* file;
            file = (CStringA)strFileName;

            if (!::_access(file, 0))
            {
                long cnt = 0;
                strAdd.Format(_T("_%d"), cnt);
                strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + strAdd + _T(".BMP");
                file = (CStringA)strFileName;
                while (!::_access(file, 0))
                {
                    cnt++;
                    strAdd.Format(_T("_%d"), cnt);
                    strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + strAdd + _T(".BMP");
                    file = (CStringA)strFileName;
                }
            }
            else
            {
                strFileName = strFolderPath + strFrameFolder + _T("\\") + strFrameFolder + _T(".BMP");
            }

            nImageSizeX = lotInspImage.GetImageSizeX();
            nImageSizeY = lotInspImage.GetImageSizeY();

            if (nImageSizeX == 0 || nImageSizeY == 0)
                return FALSE;

            nDepth = (lotInspImage.m_vecImages[i_eSideVisionModule][i].GetWidthBytes() / nImageSizeX) * 8;
            BYTE* pbyImage = lotInspImage.m_vecImages[i_eSideVisionModule][i].GetMem();

            const Ipvm::Image8u image(nImageSizeX, nImageSizeY, pbyImage, nImageSizeX);
            Ipvm::ImageFile::SaveAsBmp(image, strFileName);
        }
    }

    return TRUE;
}

BOOL CDlgPrimaryBatchList::CollectMVRejectData(
    VisionDeviceResult& devResult, const enSideVisionModule i_eSideVisionModule)
{
    int nRejectNum{}, i{}, frm{}, roi{};
    //CString strFilePath = _T("");
    //IniHelper::SaveINT(strFilePath, _T("Vision_Setting"), _T("Send_Host_Frame_Num"), 0);

    VisionImageLotInsp& lotInspImage = m_visionUnit.getImageLotInsp();
    long nFrmNum = lotInspImage.GetImageFrameCount();

    CString strFolderPath = _T("D:\\intekfiles\\");

    ::CreateDirectory(strFolderPath, NULL);

    strFolderPath = _T("D:\\intekfiles\\LearningImage\\");
    ::CreateDirectory(strFolderPath, NULL);

    strFolderPath = strFolderPath + _T("CollectedData\\");
    ::CreateDirectory(strFolderPath, NULL);

    CString strFilePath = strFolderPath;
    CString strFileName, strFrameFolder, strAdd, strLotStartDate, strLotStartTime;
    CString strSection, strKey, strValue;
    CString strLotID = m_visionMainAgent.m_lotID;
    CString strLotID_time;
    CString strJobName = m_pDlgVisionBatchInspectionUI->m_visionUnit.GetJobFileName();
    BOOL bDeadbug = m_pDlgVisionBatchInspectionUI->m_visionUnit.m_pPackageSpec->m_deadBug;

    CTime curTime = CTime::GetCurrentTime();

    long nYear = curTime.GetYear();
    long nMonth = curTime.GetMonth();
    long nDay = curTime.GetDay();
    long nHour = curTime.GetHour();
    long nMinute = curTime.GetMinute();
    long nSecond = curTime.GetSecond();

    strLotStartDate.Format(_T("%04d_%02d_%02d"), nYear, nMonth, nDay);
    strLotStartTime.Format(_T("%02d_%02d"), nHour, 0);

    if (strLotID.GetLength() <= 0)
    {
        strLotID = _T("BatchInsp_") + strLotStartTime;
        strLotID_time = strLotID;
    }
    else
    {
        strLotID_time = strLotID + _T("_") + strLotStartTime;
    }
    strFolderPath = strFolderPath + strLotStartDate;
    ::CreateDirectory(strFolderPath, NULL);
    CString strLotPath = strFolderPath + _T("\\") + strLotID_time + _T("\\");
    ::CreateDirectory(strLotPath, NULL);

    long nNumInsp = CAST_LONG(devResult.m_vecVisionInspResult.size());

    CString strInspName, strInspFolderPath, strInspIniFilePath, strFramePath;
    CString strImageFile, strFile, strText;

    long nImageWidth(0), nImageHeight(0), nImageDepth(8);

    const char* file;

    for (i = 0; i < nNumInsp; i++)
    {
        if (devResult.m_vecVisionInspResult[i]->m_totalResult == REJECT
            && (devResult.m_vecVisionInspResult[i]->m_hostReportCategory == HostReportCategory::SURFACE
                || devResult.m_vecVisionInspResult[i]->m_resultName == _T("Ball Quality")
                || devResult.m_vecVisionInspResult[i]->m_resultName == _T("Ball Bridge")))
        {
            nRejectNum = 0;
            strInspName = (*(devResult.m_vecVisionInspResult)[i]).m_inspName;
            strInspFolderPath = strLotPath + strInspName + _T("\\");
            strInspIniFilePath = strInspFolderPath + strInspName + (_T(".ini"));
            file = (CStringA)strInspFolderPath;

            strSection = _T("Public");

            if (::_access(file, 0))
            {
                ::CreateDirectory(strInspFolderPath, NULL);

                for (frm = 0; frm < nFrmNum; frm++)
                {
                    strFramePath.Format(strInspFolderPath + _T("\\Frame[%d]"), frm);
                    ::CreateDirectory(strFramePath, NULL);
                }

                // ini 파일 쓰기..
                strKey.Format(_T("TotalRejectNum"));
                IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, 0);
            }

            strKey.Format(_T("TotalRejectNum"));
            nRejectNum = IniHelper::LoadINT(strInspIniFilePath, strSection, strKey, 0);

            strSection.Format(_T("InspProcess_%d"), nRejectNum);

            strKey.Format(_T("LotStartTime"));
            strValue.Format(_T("%04d-%02d-%02dT%02d:%02d:%02d"), nYear, nMonth, nDay, nHour, nMinute, 0);
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strValue);
            strKey.Format(_T("LotID"));
            strValue = strLotID;
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strValue);
            strKey.Format(_T("JobName"));
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strJobName);

            strKey.Format(_T("NumFrame"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, nFrmNum);
            strKey.Format(_T("ImageWidth"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, lotInspImage.GetImageSizeX());
            strKey.Format(_T("ImageHeight"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, lotInspImage.GetImageSizeY());
            strKey.Format(_T("ImageDepth"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, nImageDepth);
            strKey.Format(_T("Reject_Item_Name"));
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strInspName);

            strKey.Format(_T("VisionType"));
            IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, bDeadbug == TRUE ? _T("BOTTOM") : _T("TOP"));

            for (frm = 0; frm < nFrmNum; frm++)
            {
                BYTE* pbyImage = lotInspImage.m_vecImages[i_eSideVisionModule][frm].GetMem();
                nImageWidth = lotInspImage.GetImageSizeX();
                nImageHeight = lotInspImage.GetImageSizeY();
                nImageDepth = 8;

                strFramePath.Format(strInspFolderPath + _T("\\Frame[%d]"), frm);
                strFile.Format(strFramePath + _T("\\") + strInspName + _T("_Frame%d") + _T("_[%d.%d.%d]"), frm, nHour,
                    nMinute, nSecond);
                strImageFile = strFile + _T(".BMP");
                file = (CStringA)strImageFile;
                if (!::_access(file, 0))
                {
                    long cnt = 0;
                    strText.Format(_T("%d"), cnt);
                    strImageFile = strFile + _T("_") + strText + _T(".BMP");

                    file = (CStringA)strImageFile;
                    while (!::_access(file, 0))
                    {
                        cnt++;
                        strText.Format(_T("%d"), cnt);
                        strImageFile = strFile + _T("_") + strText + _T(".BMP");
                    }
                }

                const Ipvm::Image8u image(nImageWidth, nImageHeight, pbyImage, nImageWidth);
                Ipvm::ImageFile::SaveAsBmp(image, strFileName);

                strKey.Format(_T("Frame%d_ImagePath"), frm);
                IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strImageFile);
            }

            // Reject Information 쓰기
            long nRoiNum = CAST_LONG((*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI.size());
            strKey.Format(_T("FAIL_Item_%d_Roi"), nRejectNum);
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, nRoiNum);
            for (roi = 0; roi < nRoiNum; roi++)
            {
                strKey.Format(_T("FAIL_Item_%d_Roi%d"), nRejectNum, roi);
                CRect rtROI = CRect((*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].TopLeft().m_x,
                    (*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].TopLeft().m_y,
                    (*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].BottomRight().m_x,
                    (*(devResult.m_vecVisionInspResult)[i]).vecrtRejectROI[roi].BottomRight().m_y);

                strValue.Format(_T("%d,%d,%d,%d"), rtROI.left, rtROI.top, rtROI.right, rtROI.bottom);
                IniHelper::SaveSTRING(strInspIniFilePath, strSection, strKey, strValue);
            }

            // TotalRejectNum에 1 더하기
            strSection = _T("Public");
            strKey.Format(_T("TotalRejectNum"));
            IniHelper::SaveINT(strInspIniFilePath, strSection, strKey, ++nRejectNum);
        }
    }

    return TRUE;
}

BOOL CDlgPrimaryBatchList::Send2ndInspInfo(VisionDeviceResult& devResult, const enSideVisionModule i_eSideVisionModule)
{
    if (devResult.m_nTotalResult != REJECT)
        return FALSE;

    long nNumInspVsModule = CAST_LONG(m_visionUnit.m_vecVisionModules.size());

    long nTrayID = devResult.m_nTrayID;
    long nPocketID = devResult.m_nPocketID;
    long nPaneID = devResult.m_nPane;

    CString strLotID = m_visionMainAgent.m_lotID;
    CString strJobName = m_pDlgVisionBatchInspectionUI->m_visionUnit.GetJobFileName();

    //
    if (strLotID.GetLength() <= 0)
    {
        strLotID = _T("BatchInsp");
    }

    CTime curTime = CTime::GetCurrentTime();

    long nYear = curTime.GetYear();
    long nMonth = curTime.GetMonth();
    long nDay = curTime.GetDay();
    long nHour = curTime.GetHour();
    //long nMinute = curTime.GetMinute();

    CString strLotStartDate, strLotStartTime;

    strLotStartDate.Format(_T("[%04d%02d%02d]_"), nYear, nMonth, nDay);
    strLotStartTime.Format(_T("_%02d_%02d"), nHour, 0);
    //strLotID = strLotID + strLotStartTime;
    strLotID = strLotID;

    long nInspItemNum = CAST_LONG(devResult.m_vecVisionInspResult.size());

    VisionImageLotInsp& lotInspImage = m_visionUnit.getImageLotInsp();
    long nFrmNum = lotInspImage.GetImageFrameCount();

    CString strInspModuleName;
    std::vector<CString> vecstrRejInspName;
    std::vector<CString> vecstr2ndInspCode;
    std::vector<BOOL> vecbUseDLOnly;
    std::vector<BOOL> vecbUseCompInsp;

    long i, j, idx, k;
    long nTimeCnt;

    BOOL bPASS = TRUE;
    BOOL bOverTime;
    BOOL bComponentInsp = FALSE;

    std::vector<BOOL> vecbCompUse2ndInsp;
    std::vector<CString> vecstrComp2ndInspCode;
    std::vector<CString> vecstrRejComp2ndInspCode; // Reject인 component의 2nd insp code => reject불량 ROI개수와 같다.

    std::vector<int> vecnRejCompIndx;

    for (i = 0; i < nNumInspVsModule; i++)
    {
        if (m_visionUnit.m_vecVisionModules[i]->m_bUse2ndInspection)
        {
            strInspModuleName = m_visionUnit.m_vecVisionModules[i]->m_strModuleName;

            for (j = 0; j < nInspItemNum; j++)
            {
                if (strInspModuleName == (*(devResult.m_vecVisionInspResult)[j]).m_inspName)
                {
                    if (!m_visionUnit.m_vecVisionModules[i]
                            ->m_bIsComponentInsp) // Component 검사가 아닌경우 08.20.20 KSY Component D/L Inspection
                    {
                        if ((*(devResult.m_vecVisionInspResult)[j]).m_totalResult == REJECT
                            || m_visionUnit.m_vecVisionModules[i]->m_bUseOnlyDLInsp == TRUE)
                        {
                            vecstrRejInspName.push_back(strInspModuleName);
                            vecstr2ndInspCode.push_back(m_visionUnit.m_vecVisionModules[i]->m_str2ndInspCode);
                            vecbUseDLOnly.push_back(m_visionUnit.m_vecVisionModules[i]->m_bUseOnlyDLInsp);
                            vecbUseCompInsp.push_back(FALSE);

                            //if()

                            bPASS = FALSE;
                        }
                    }
                    else // Component 검사 인 경우
                    {
                        if ((*(devResult.m_vecVisionInspResult)[j]).m_totalResult == REJECT
                            && (((*(devResult.m_vecVisionInspResult)[j])).m_resultName == _T("PASSIVE_MISSING")
                                || ((*(devResult.m_vecVisionInspResult)[j])).m_resultName == _T("PASSIVE_ROTATE")
                                || ((*(devResult.m_vecVisionInspResult)[j])).m_resultName == _T("PASSIVE_LENGTH")))
                        {
                            if ((*(devResult.m_vecVisionInspResult)[j]).m_objectResults.size()
                                != m_visionUnit.m_vecVisionModules[i]->m_vecbUseComp2ndInsp.size())
                            {
                                continue;
                            }

                            long nNumComp = CAST_LONG((*(devResult.m_vecVisionInspResult)[j]).m_objectResults.size());

                            if (vecbCompUse2ndInsp.size() == 0)
                            {
                                vecbCompUse2ndInsp.resize(nNumComp, FALSE);
                                vecstrComp2ndInspCode.resize(nNumComp);
                            }

                            for (long comp = 0; comp < nNumComp; comp++)
                            {
                                if ((*(devResult.m_vecVisionInspResult)[j]).m_objectResults[comp] == REJECT)
                                {
                                    // 중복체크
                                    BOOL bDup = FALSE;
                                    for (long d = 0; d < vecnRejCompIndx.size(); d++)
                                    {
                                        if (comp == vecnRejCompIndx[d])
                                            bDup = TRUE;
                                    }

                                    if (!bDup)
                                        vecnRejCompIndx.push_back(comp);
                                    // Component에 하나의 불량이라도 있고 D/L 검사를 한다면 TRUE
                                    if (m_visionUnit.m_vecVisionModules[i]->m_vecbUseComp2ndInsp[comp])
                                    {
                                        bComponentInsp = TRUE;
                                        vecbCompUse2ndInsp[comp]
                                            = m_visionUnit.m_vecVisionModules[i]->m_vecbUseComp2ndInsp[comp];
                                        vecstrComp2ndInspCode[comp]
                                            = m_visionUnit.m_vecVisionModules[i]->m_vecstrComp2ndMatchCode[comp];
                                    }
                                    else
                                    {
                                        vecbCompUse2ndInsp[comp] = FALSE;
                                        vecstrComp2ndInspCode[comp] = _T("");
                                    }
                                }
                            }
                        }
                    }
                }

                if (bComponentInsp)
                {
                    bComponentInsp = FALSE;
                    BOOL bDuplicate = FALSE;
                    for (long a = 0; a < vecstrRejInspName.size(); a++)
                    {
                        if (vecstrRejInspName[a] == strInspModuleName)
                        {
                            bDuplicate = TRUE;
                            break;
                        }
                    }
                    if (bDuplicate)
                        continue;

                    vecstrRejInspName.push_back(strInspModuleName);
                    vecstr2ndInspCode.push_back(
                        _T("CompInsp")); // <- 임시로 넣어주는 코드, 실제로는 Component 유형별로 각기 다른 Code를 사용할 수 있기 때문에 따로 정보를 보내줌
                    vecbUseDLOnly.push_back(m_visionUnit.m_vecVisionModules[i]->m_bUseOnlyDLInsp);
                    vecbUseCompInsp.push_back(TRUE);

                    bPASS = FALSE;
                }
            }
        }
    }

    //08.21.20 KSY Component관련 정보 저장

    VisionAlignResult* sEdgeAlignResult = nullptr;

    // Body align information
    long nDataNum;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        m_visionUnit.GetVisionAlignProcessingModuleGUID(), _T("EDGE Align Result"), nDataNum);
    if (pData != nullptr && nDataNum > 0)
        sEdgeAlignResult = (VisionAlignResult*)pData;

    if (sEdgeAlignResult == nullptr)
        return FALSE;

    CRect rtArea = CRect((int)sEdgeAlignResult->fptLT.m_x, (int)(int)sEdgeAlignResult->fptLT.m_y,
        (int)sEdgeAlignResult->fptRB.m_x, (int)sEdgeAlignResult->fptRB.m_y);
    ::Ipvm::Rect32s Area(0, 0, lotInspImage.GetImageSizeX(), lotInspImage.GetImageSizeY());

    // 덤프 이미지 테스트 코드
    //CString strFileName;
    //std::string str;
    //for (i = 0; i < nFrmNum; i++)
    //{
    //	strFileName.Format(_T("C:\\test\\Frame%d.BMP"), i);
    //	cv::Mat img(cvSize(lotInspImage.GetImageSizeX(), lotInspImage.GetImageSizeY()), CV_8UC1, lotInspImage.m_vecImages[i].GetMem(), cv::Mat::AUTO_STEP);
    //	str = CT2CA(strFileName);
    //	cv::imwrite(str, img);
    //}

    // 불량이 있으면.. (또는 불량이 없어도 Only DL Insp 옵션에 의해 딥러닝 검사가 필요한 경우)
    if (bPASS == FALSE && m_visionUnit.m_ptrSharedMem != NULL)
    {
        long nImageSizeX{}, nImageSizeY{}, nDepth{};
        m_visionUnit.m_Mutex.Lock();

        long nCount = 0;
        while (m_visionUnit.m_ptrSharedMem->bAvailable)
        {
            if (nCount > m_visionUnit.m_systemConfig.m_nDLWaitTimeSendRejectData)
            {
                m_visionUnit.m_Mutex.Unlock();
                return FALSE;
            }

            Sleep(1);
            nCount++;
        }

        long lotImageNum = CAST_LONG(lotInspImage.m_vecImages[i_eSideVisionModule].size());
        if (lotImageNum == 0)
        {
            m_visionUnit.m_Mutex.Unlock();
            return FALSE;
        }

        for (i = 0; i < lotImageNum; i++)
        {
            nImageSizeX = lotInspImage.GetImageSizeX();
            nImageSizeY = lotInspImage.GetImageSizeY();

            if (nImageSizeX == 0 || nImageSizeY == 0)
                return FALSE;

            nDepth = (lotInspImage.m_vecImages[i_eSideVisionModule][i].GetWidthBytes() / nImageSizeX) * 8;
            BYTE* pbyImage = lotInspImage.m_vecImages[i_eSideVisionModule][i].GetMem();

            memcpy(m_visionUnit.m_ptrSharedImageMem + (MAX_DL_IMAGE_SIZE_X * MAX_DL_IMAGE_SIZE_Y * i), pbyImage,
                (nImageSizeX * (nDepth / 8)) * nImageSizeY);
        }

        m_visionUnit.m_ptrSharedMem->cTime = curTime;
        m_visionUnit.m_ptrSharedMem->nTrayID = nTrayID;
        m_visionUnit.m_ptrSharedMem->nPocketID = nPocketID;
        m_visionUnit.m_ptrSharedMem->nPaneID = nPaneID;
        ::strcpy_s(m_visionUnit.m_ptrSharedMem->lotID, (CStringA)strLotID);
        ::strcpy_s(m_visionUnit.m_ptrSharedMem->jobName, (CStringA)strJobName);
        m_visionUnit.m_ptrSharedMem->nNum2ndInsp = CAST_LONG(vecstrRejInspName.size());
        m_visionUnit.m_ptrSharedMem->nNumFrame = nFrmNum;
        m_visionUnit.m_ptrSharedMem->nWidth = nImageSizeX;
        m_visionUnit.m_ptrSharedMem->nHeight = nImageSizeY;
        m_visionUnit.m_ptrSharedMem->nDepth = nDepth;
        m_visionUnit.m_ptrSharedMem->ptAlign[0]
            = CPoint((int)sEdgeAlignResult->fptLT.m_x, (int)sEdgeAlignResult->fptLT.m_y);
        m_visionUnit.m_ptrSharedMem->ptAlign[1]
            = CPoint((int)sEdgeAlignResult->fptRT.m_x, (int)sEdgeAlignResult->fptRT.m_y);
        m_visionUnit.m_ptrSharedMem->ptAlign[2]
            = CPoint((int)sEdgeAlignResult->fptLB.m_x, (int)sEdgeAlignResult->fptLB.m_y);
        m_visionUnit.m_ptrSharedMem->ptAlign[3]
            = CPoint((int)sEdgeAlignResult->fptRB.m_x, (int)sEdgeAlignResult->fptRB.m_y);
        //m_visionUnit.m_ptrSharedMem->ptPaneLT = rtPane.TopLeft(); // Insp 영상이 Pane기준으로 자른 영상이므로 Pane ROI가 필요없음 (Intel 500 iPack)

        for (k = 0; k < vecbUseDLOnly.size(); k++)
        {
            m_visionUnit.m_ptrSharedMem->bDLInspOnly[k] = vecbUseDLOnly[k];
        }

        // 08.24.20 KSY Component Inspection -----------------------------------------
        for (k = 0; k < vecbUseCompInsp.size(); k++)
            m_visionUnit.m_ptrSharedMem->bIsCompInsp[k] = vecbUseCompInsp[k];

        for (k = 0; k < vecbCompUse2ndInsp.size(); k++)
        {
            m_visionUnit.m_ptrSharedMem->bCompUse2ndInsp[k] = vecbCompUse2ndInsp[k];
            ::strcpy_s(m_visionUnit.m_ptrSharedMem->cComp2ndInspCode[k], (CStringA)vecstrComp2ndInspCode[k]);
        }

        m_visionUnit.m_ptrSharedMem->nNumTotRejectComponent = CAST_LONG(vecnRejCompIndx.size());
        for (k = 0; k < vecnRejCompIndx.size(); k++)
        {
            m_visionUnit.m_ptrSharedMem->nRejCompIndex[k] = vecnRejCompIndx[k];
        }
        // ---------------------------------------------------------------------------

        for (i = 0; i < vecstrRejInspName.size(); i++)
        {
            ::strcpy_s(m_visionUnit.m_ptrSharedMem->c2ndInspName[i], (CStringA)vecstrRejInspName[i]);
            ::strcpy_s(m_visionUnit.m_ptrSharedMem->c2ndInspCode[i], (CStringA)vecstr2ndInspCode[i]);

            // Criteria 데이터 //----------------------------------------------------------------------------------------
            for (idx = 0; idx < m_visionUnit.m_visionInspectionSurfaces.size(); idx++)
            {
                std::vector<CSurfaceCriteria>& vecCriteria
                    = m_visionUnit.m_visionInspectionSurfaces[idx]->m_surfacePara->m_SurfaceItem.vecCriteria;
                SItemSpec sCriteriaSpec;

                if (vecstr2ndInspCode[i]
                    == m_visionUnit.m_visionInspectionSurfaces[idx]
                        ->m_surfacePara->m_SurfaceItem.AlgoPara.m_str2ndInspCode)
                {
                    if (vecCriteria.size() == 0)
                        continue;

                    sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Length);
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].length.fMin = sCriteriaSpec.GetPassMax();
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].length.fMax = sCriteriaSpec.GetPassMin();
                    sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Area);
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].area.fMin = sCriteriaSpec.GetPassMax();
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].area.fMax = sCriteriaSpec.GetPassMin();
                    sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::LengthX);
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthX.fMin = sCriteriaSpec.GetPassMax();
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthX.fMax = sCriteriaSpec.GetPassMin();
                    sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::LengthY);
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthY.fMin = sCriteriaSpec.GetPassMax();
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].lengthY.fMax = sCriteriaSpec.GetPassMin();
                    sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Thickness);
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].thickness.fMin = sCriteriaSpec.GetPassMax();
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].thickness.fMax = sCriteriaSpec.GetPassMin();
                    sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Locus);
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].locus.fMin = sCriteriaSpec.GetPassMax();
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].locus.fMax = sCriteriaSpec.GetPassMin();
                    sCriteriaSpec = vecCriteria[0].GetSpecMin(VisionSurfaceCriteria_Column::Width);
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].width.fMin = sCriteriaSpec.GetPassMax();
                    m_visionUnit.m_ptrSharedMem->sCriteriaSpec[idx].width.fMax = sCriteriaSpec.GetPassMin();
                }
            }
            // --------------------------------------------------------------------------------------------------------------
        }

        if (m_visionUnit.m_pbySharedDevResult != NULL)
        {
            ConvertDeviceResultToBuffer(m_visionUnit.m_pbySharedDevResult, devResult, TRUE);
        }

        nTimeCnt = 0;
        bOverTime = FALSE;

        //SendMessage Test Code
        HWND hWnd = ::FindWindow(NULL, _T("iEasyAI"));
        if (hWnd == 0)
        {
            //AfxMessageBox(_T("윈도우를 찾지 못하였습니다."));
            m_visionUnit.m_Mutex.Unlock();
            return FALSE;
        }
        else
        {
            DWORD pid = ::GetCurrentProcessId();
            m_visionUnit.m_ptrSharedMem->bAvailable
                = TRUE; // AI Runtime이 데이터가 Shared Memory에 들어왔는지 알아챈다.
            ::SendMessage(hWnd, MsgSendInspDataVsToDL, NULL, pid);

            m_visionUnit.m_Mutex.Unlock();
            return TRUE;
        }

        //::WaitForSingleObject(m_visionMainAgent.m_Control_SendConfirm.m_Signal_SendResult, INFINITE);
        //::ResetEvent(m_visionMainAgent.m_Control_SendConfirm.m_Signal_SendResult);
    }
    else
    {
        return FALSE;
    }
}

void CDlgPrimaryBatchList::OnTimer(UINT_PTR nIDEvent)
{
    if (nIDEvent == 0)
    {
        switch (m_runState)
        {
            case BatchRunState::Idle:
                if (1)
                {
                    // 영훈 20150211_infinite_Inspection : 현재 모드가 활성화 되면 BatchInspection을 무한으로 검사하도록 한다.
                    if (g_batchManager.m_bInfiniteInspectionMode)
                    {
                        if (g_batchManager.m_nGridNum >= (long)(m_pgridBatch->GetRowCount()))
                        {
                            g_batchManager.m_nGridNum = 2;

                            if (SystemConfig::GetInstance().Get_Enable_CPU_CALC_SAVE_LOG() == true)
                            {
                                if (SystemConfig::GetInstance().Get_CPU_SavebyInspectionCount() > 0
                                    && snInspCount >= SystemConfig::GetInstance().Get_CPU_SavebyInspectionCount())
                                {
                                    snReportidx++;
                                    SystemConfig::GetInstance().SaveVisionInspTime(snReportidx);
                                    SystemConfig::GetInstance().ResetVisionInspTime();
                                    snInspCount = 0;
                                }

                                snInspCount++;
                            }
                        }

                        if (SystemConfig::GetInstance().Get_Enable_CPU_CALC_SAVE_LOG() == true
                            && snReportidx > SystemConfig::GetInstance().Get_CPU_SaveReportMaximumCount())
                            g_batchManager.m_nGridNum = (long)(m_pgridBatch->GetRowCount()) + 1;
                    }

                    if (g_batchManager.m_nGridNum < (long)(m_pgridBatch->GetRowCount())
                        && g_batchManager.m_nGridNum > 0)
                    {
                        //m_pgridBatch->SetFocusCell(g_batchManager.m_nGridNum, 1);
                        //m_pgridBatch->EnsureVisible(g_batchManager.m_nGridNum, 1);

                        m_runState = BatchRunState::ThreadReadyCheck;
                    }
                    else
                    {
                        if (m_visionMainAgent.IsAllBatchInspectionThreadsIdle())
                        {
                            SetState(BatchState::Ready);
                            m_visionMainAgent.SetInlineStop(true);

                            CDlgYieldViewer Dlg;
                            Dlg.SetVisionInspResult(g_batchManager.m_vecstBatchInspResult);

                            if (SystemConfig::GetInstance().m_nAutomationMode == 1)
                            {
                                m_visionMainAgent.SendToAutomationBatchEnd(m_batchInspTime);
                                BOOL bRes = WriteRnRResult();
                                if (!bRes)
                                    return;
                            }
                            else
                            {
                                Dlg.DoModal();
                                BOOL bSaveRNR = Dlg.m_saveRNR;

                                if (bSaveRNR)
                                {
                                    BOOL bRes = WriteRnRResult();
                                    if (!bRes)
                                        return;
                                }
                            }
                        }
                    }
                }
                break;

            case BatchRunState::ThreadReadyCheck:
                if (1)
                {
                    long nPane = _ttoi(m_pgridBatch->GetItemText(g_batchManager.m_nGridNum, BatchPreColumn_PaneID)) - 1;

                    if (DoInspection(g_batchManager.m_nGridNum, nPane, true))
                    {
                        m_runState = BatchRunState::Idle;
                        g_batchManager.m_nGridNum++;

                        // SDY_IntensityChecker 로그 작성
                        SaveIntensityCheckerLog();

                        if (m_bSaveLogMemory) //mc_임시방편..
                            SaveMemorySize();
                    }
                }
                break;
        }
    }
    else if (nIDEvent == TIMER_MEM_LOG_20MIN)
        SystemConfig::GetInstance().logger_memcheck();
    //else if (nIDEvent == TIMER_MEM_LOG)//kircheis_201711
    //{
    //	SaveMemorySize();
    //}

    CDialog::OnTimer(nIDEvent);
}

BOOL CDlgPrimaryBatchList::WriteRnRResult()
{
    //{{ //kircheis_KillProc
    long nBatchListSize = (long)g_batchManager.m_vecBatchList.size();
    if (nBatchListSize <= 0)
    {
        return FALSE;
    }
    //}}

    BOOL bSaveInfo = TRUE;

    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));

    CString strFolderPath_RnR;
    CString strFolderPath_Surface;

    if (SystemConfig::GetInstance().m_nAutomationMode == 1) // Automation이 활성화 될 경우 Automation 폴더로 이동한다.
    {
        CString strAutomationFolder;
        CString strAutomationResultFolder;

        strAutomationFolder.Format(_T("%sAutomationTemp\\"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)));
        Ipvm::CreateDirectories(LPCTSTR(strAutomationFolder)); // Automation 폴더 생성

        strAutomationResultFolder.Format(_T("%sAutomationTemp\\%s\\"),
            LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)),
            (LPCTSTR)REMOVE_EXT_FROM_FILENAME((CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName));
        Ipvm::CreateDirectories(LPCTSTR(strAutomationResultFolder)); // 결과 폴더 생성

        strFolderPath_RnR.Format(_T("%s\\RnRData\\"), LPCTSTR(strAutomationResultFolder));
        strFolderPath_Surface.Format(_T("%s\\Surface\\"), LPCTSTR(strAutomationResultFolder));
    }
    else
    {
        strFolderPath_RnR.Format(
            _T("%sRnRData\\%s\\"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)), LPCTSTR(strTime));
        strFolderPath_Surface.Format(
            _T("%sSurface\\%s\\"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)), LPCTSTR(strTime));
    }
    Ipvm::CreateDirectories(LPCTSTR(strFolderPath_RnR));
    Ipvm::CreateDirectories(LPCTSTR(strFolderPath_Surface));

    struct SInfo
    {
        CString m_keyName;
        float m_minSigma;
        float m_maxSigma;
    };

    std::map<CString, std::vector<SInfo>> collectInsp_PaneInfo;

    for (auto& rnrDataSet : g_batchManager.m_mapRnRData)
    {
        auto& objLevelData = rnrDataSet.second;

        if (objLevelData.size() == 0)
        {
            continue;
        }

        const long repeatCount = (long)objLevelData[0].second.size();

        if (repeatCount == 0)
        {
            bSaveInfo &= FALSE;
            continue;
        }

        auto& collect_paneInfo = collectInsp_PaneInfo[rnrDataSet.first.first];

        CString key = rnrDataSet.first.second;
        CString strTotalFilePath = strFolderPath_RnR + rnrDataSet.first.first + _T(" __ ") + key + _T(".csv");

        FILE* fp = nullptr;
        _tfopen_s(&fp, strTotalFilePath, _T("w"));

        if (fp == nullptr)
        {
            bSaveInfo &= FALSE;
            continue;
        }

        // Write header

        fprintf(fp, "PIN,");
        for (long repeat = 0; repeat < repeatCount; repeat++)
        {
            fprintf(fp, "R_%d,", repeat + 1);
        }

        fprintf(fp, "AVG,");
        fprintf(fp, "3SIGMA\n");

        float minSigma = FLT_MAX;
        float maxSigma = -FLT_MAX;

        // Write data
        for (auto& objData : objLevelData)
        {
            double sum = 0.;
            long sumCount = 0;

            fprintf(fp, "%s,", (const char*)CStringA(objData.first));

            for (auto value : objData.second)
            {
                if (value == Ipvm::k_noiseValue32r)
                {
                    fprintf(fp, ",");
                }
                else
                {
                    fprintf(fp, "%.2f,", value);
                    sum += value;
                    sumCount++;
                }
            }

            for (long repeat = sumCount; repeat < repeatCount; repeat++)
            {
                fprintf(fp, ",");
            }

            float average = sumCount > 0 ? float(sum / sumCount) : 0.f;

            double sqrSum = 0.;

            for (auto value : objData.second)
            {
                if (value != Ipvm::k_noiseValue32r)
                {
                    sqrSum += (value - average) * (value - average);
                }
            }

            float sigma = sumCount > 1 ? float(sqrt(sqrSum / (sumCount))) : 0.f;

            minSigma = min(minSigma, sigma);
            maxSigma = max(maxSigma, sigma);

            fprintf(fp, "%.2f,%.2f\n", average, 3 * sigma);
        }

        SInfo info;
        info.m_minSigma = minSigma;
        info.m_maxSigma = maxSigma;
        info.m_keyName = key;
        collect_paneInfo.push_back(info);

        for (long repeat = 0; repeat < repeatCount; repeat++)
            fprintf(fp, ",");
        fprintf(fp, ",MIN,%.2f\n", 3 * minSigma);

        for (long repeat = 0; repeat < repeatCount; repeat++)
            fprintf(fp, ",");
        fprintf(fp, ",MAX,%.2f\n", 3 * maxSigma);

        fclose(fp);
    }

    if (bSaveInfo)
    {
        CString summaryPath = strFolderPath_RnR + _T("RnR Summary.csv");

        FILE* fp = nullptr;
        _tfopen_s(&fp, summaryPath, _T("w"));

        if (fp != nullptr)
        {
            // Write header

            long maxRowCount = 0;

            for (auto& itPane : collectInsp_PaneInfo)
            {
                fprintf(fp, (const char*)CStringA(itPane.first));
                fprintf(fp, ",3S MIN, 3S MAX,,");

                maxRowCount = max(maxRowCount, long(itPane.second.size()));
            }

            fprintf(fp, "\n");

            for (long row = 0; row < maxRowCount; row++)
            {
                for (auto& itPane : collectInsp_PaneInfo)
                {
                    if (row >= long(itPane.second.size()))
                    {
                        fprintf(fp, ",,,,");
                    }
                    else
                    {
                        fprintf(fp, (const char*)CStringA(itPane.second[row].m_keyName));
                        fprintf(
                            fp, ",%.2f, %.2f,,", 3 * itPane.second[row].m_minSigma, 3 * itPane.second[row].m_maxSigma);
                    }
                }

                fprintf(fp, "\n");
            }

            fclose(fp);
        }

        AutoHidePopup::add(AutoHidePopupType::Normal, _T("Save Complete!"));
    }
    else
    {
        AutoHidePopup::add(AutoHidePopupType::Error, _T("Save Error!"));
    }

    g_batchManager.m_reportData.Save(g_batchManager.m_barcodeData.IsEnabled(), strFolderPath_RnR);
    g_batchManager.m_reportWarpageData.Save(g_batchManager.m_barcodeData.IsEnabled(), strFolderPath_RnR);
    g_batchManager.m_reportSurfaceData.Save(g_batchManager.m_barcodeData.IsEnabled(), strFolderPath_Surface);
    g_batchManager.m_reportBodyData.Save(g_batchManager.m_barcodeData.IsEnabled(), strFolderPath_RnR);

    WriteAutomationResult();

    return TRUE;
}

BOOL CDlgPrimaryBatchList::WriteAutomationResult()
{
    CString strAutomationFolder;
    CString strAutomationResultFolder;

    strAutomationFolder.Format(_T("%sAutomationTemp\\%s\\"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)),
        (LPCTSTR)REMOVE_EXT_FROM_FILENAME(
            (CString)SystemConfig::GetInstance().m_strAutomationRecipeFileName)); // Automation 폴더 생성
    //m_visionUnit.m_visionInspectionSurfaces.size();
    g_batchManager.m_reportData.SaveAutomationInfo(g_batchManager.m_barcodeData.IsEnabled(), strAutomationFolder);
    g_batchManager.m_reportSurfaceData.SaveAutomationInfo(
        g_batchManager.m_barcodeData.IsEnabled(), strAutomationFolder, m_visionUnit);
    g_batchManager.m_reportBodyData.SaveAutomationInfo(g_batchManager.m_barcodeData.IsEnabled(), strAutomationFolder);

    return TRUE;
}
void CDlgPrimaryBatchList::SetState(BatchState state)
{
    m_state = state;

    bool thread = true;

    switch (state)
    {
        case BatchState::Ready:
            KillTimer(0);
            KillTimer(TIMER_MEM_LOG);
            KillTimer(TIMER_MEM_LOG_20MIN);

            thread = false;

            m_buttonAutoInspect.EnableWindow(TRUE);
            m_buttonClearAll.EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_RESUME)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_ASORT)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_DSORT)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_YIELD)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_INFINITE_INSP)->EnableWindow(TRUE);
            GetDlgItem(IDC_EDIT_SequenceInterrvalTime)->EnableWindow(TRUE);
            GetDlgItem(IDC_CHECK_SCANID_AND_PANE)->EnableWindow(TRUE);

            m_pDlgVisionBatchInspectionUI->m_imageLotView->SetPaneWindowEnabled(true);
            break;
        case BatchState::Run:
            if (1)
            {
                m_runState = BatchRunState::Idle;
                CString intervalTimeText;
                GetDlgItem(IDC_EDIT_SequenceInterrvalTime)->GetWindowText(intervalTimeText);

                // 2 Step 으로 검사하므로 절반의 시간의 타이머를 세팅하자
                m_visionMainAgent.SetInlineStart(true);

                long itervalTime = _ttoi(intervalTimeText);
                SetTimer(0, max(1, itervalTime / 2), NULL);

                PersonalConfig::getInstance().setSequenceIntervalTimeInBatchInspection(itervalTime);

                m_buttonAutoInspect.EnableWindow(FALSE);
                m_buttonClearAll.EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);
                GetDlgItem(IDC_BUTTON_RESUME)->EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_ASORT)->EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_DSORT)->EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_YIELD)->EnableWindow(FALSE);
                GetDlgItem(IDC_BUTTON_INFINITE_INSP)->EnableWindow(FALSE);
                GetDlgItem(IDC_EDIT_SequenceInterrvalTime)->EnableWindow(FALSE);
                GetDlgItem(IDC_CHECK_SCANID_AND_PANE)->EnableWindow(FALSE);

                m_pDlgVisionBatchInspectionUI->m_imageLotView->SetPaneWindowEnabled(false);
            }
            break;
        case BatchState::Stopping:
            KillTimer(0);
            KillTimer(TIMER_MEM_LOG);
            KillTimer(TIMER_MEM_LOG_20MIN);

            m_buttonAutoInspect.EnableWindow(FALSE);
            m_buttonClearAll.EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_RESUME)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_ASORT)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_DSORT)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_YIELD)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_INFINITE_INSP)->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_SequenceInterrvalTime)->EnableWindow(FALSE);
            GetDlgItem(IDC_CHECK_SCANID_AND_PANE)->EnableWindow(FALSE);
            break;

        case BatchState::Stop:
            thread = false;
            m_buttonAutoInspect.EnableWindow(TRUE);
            m_buttonClearAll.EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);
            GetDlgItem(IDC_BUTTON_RESUME)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_ASORT)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_DSORT)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_DELETE)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_YIELD)->EnableWindow(TRUE);
            GetDlgItem(IDC_BUTTON_INFINITE_INSP)->EnableWindow(FALSE);
            GetDlgItem(IDC_EDIT_SequenceInterrvalTime)->EnableWindow(TRUE);
            GetDlgItem(IDC_CHECK_SCANID_AND_PANE)->EnableWindow(TRUE);

            m_pDlgVisionBatchInspectionUI->m_imageLotView->SetPaneWindowEnabled(true);
            break;
    }

    if (m_pDlgVisionBatchInspectionUI->m_visionCommonResultDlg)
    {
        m_pDlgVisionBatchInspectionUI->m_visionCommonResultDlg->EnableWindow(thread ? FALSE : TRUE);
        m_pDlgVisionBatchInspectionUI->m_visionCommonDetailResultDlg->EnableWindow(thread ? FALSE : TRUE);
        m_pDlgVisionBatchInspectionUI->m_visionCommonTextResultDlg->EnableWindow(thread ? FALSE : TRUE);
        m_pDlgVisionBatchInspectionUI->m_visionCommonDebugInfoDlg->EnableWindow(thread ? FALSE : TRUE);
    }
}

// 영훈 [Batch Insp GRNR Accuracy] 20130807 : Vision에서 Save된 Image를 가지고 정보를 뽑을 수 있도록 하자.
void CDlgPrimaryBatchList::SaveRnRData(long nScan, long nPane, long dataIndex, VisionUnit& visionUnit)
{
    if (!ReportData::isUnitValid(visionUnit))
    {
        return;
    }

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    long realDataIndex = 0;

    for (long rowIndex = 2; rowIndex < dataIndex + 2; rowIndex++)
    {
        long rowPane = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_PaneID)) - 1;

        if (rowPane == nPane)
        {
            realDataIndex++;
        }
    }

    CString key;
    if (((CButton*)GetDlgItem(IDC_CHECK_SCANID_AND_PANE))->GetCheck())
    {
        key.Format(_T("%02d_%02d"), nScan + 1, nPane + 1);
    }
    else
    {
        key.Format(_T("Pane%d"), nPane + 1);
    }

    for (auto* result : visionInspectionResults)
    {
        auto& data = g_batchManager.m_mapRnRData[std::make_pair(result->m_resultName, key)];

        const auto objCount = result->m_objectErrorValues.size();

        data.resize(max(data.size(), objCount));

        for (size_t obj = 0; obj < objCount; obj++)
        {
            data[obj].first = result->m_objectNames[obj];
            data[obj].second.resize(max(data[obj].second.size(), realDataIndex + 1), Ipvm::k_noiseValue32r);
            data[obj].second[realDataIndex] = result->m_objectErrorValues[obj];
        }
    }
}

void DoEvents(DWORD iWaitTimeMS)
{
    MSG msg;

    const DWORD dwStartTime = GetTickCount();
    DWORD dwCurrentTime = 0;

    do
    {
        while (::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            ::TranslateMessage(&msg);

            TRACE(_T("TranslateMessage %x %d\n"), msg.hwnd, msg.message);
            ::DispatchMessage(&msg);
        }

        Sleep(1);

        dwCurrentTime = GetTickCount();
    }
    while (dwCurrentTime - dwStartTime < iWaitTimeMS);
}

void CDlgPrimaryBatchList::OnBnClickedButtonStop()
{
    m_bSaveLogMemory = FALSE;

    SetState(BatchState::Stopping);

    Ipvm::AsyncProgress progress(_T("Stopping ..."));

    EnableWindow(FALSE);

    while (!m_visionMainAgent.IsAllBatchInspectionThreadsIdle())
    {
        // 검사 결과가 오면 Callback에 의해 화면갱신이 일어나므로
        // Message 처리를 할 수 있어야 한다. 다른 UI는 건들지 못하게
        // Enable Window을 전후로 수행시켰다

        DoEvents(10);
        Sleep(10);
    }

    m_visionMainAgent.SetInlineStop(true);
    EnableWindow(TRUE);

    SetState(BatchState::Stop);
}

void CDlgPrimaryBatchList::OnBnClickedButtonResume()
{
    SetState(BatchState::Run);
}

void CDlgPrimaryBatchList::OnBnClickedButtonAsort()
{
    CCellID cell = m_pgridBatch->GetFocusCell();

    if (!cell.IsValid())
    {
        return;
    }

    m_pgridBatch->SortItems(cell.col, TRUE);
    m_pgridBatch->Refresh();
    SetCurNum();
    SetState(BatchState::Ready);
}

void CDlgPrimaryBatchList::OnBnClickedButtonDsort()
{
    CCellID cell = m_pgridBatch->GetFocusCell();

    if (!cell.IsValid())
    {
        return;
    }

    m_pgridBatch->SortItems(cell.col, FALSE);
    m_pgridBatch->Refresh();
    SetCurNum();
    SetState(BatchState::Ready);
}

void CDlgPrimaryBatchList::SetCurNum()
{
    long nGridMaxNum = m_pgridBatch->GetRowCount();
    CString str;
    str.Format(_T("%d/%d"), g_batchManager.m_nGridNum - 2, nGridMaxNum - 2);

    GetDlgItem(IDC_STATIC_CURNUM)->SetWindowText(str);
}

void CDlgPrimaryBatchList::OnBnClickedButtonDelete()
{
    CCellRange range = m_pgridBatch->GetSelectedCellRange();

    if (!range.IsValid())
        return;

    for (long rowIndex = range.GetMaxRow(); rowIndex >= range.GetMinRow(); rowIndex--)
    {
        long index = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_Index)) - 1;

        g_batchManager.m_vecBatchList.erase(g_batchManager.m_vecBatchList.begin() + index);
        m_pgridBatch->DeleteRow(rowIndex);

        // 삭제하면서 Index가 갱신되었으므로 index보다 큰 index는 1씩 빼주자
        for (long check = 2; check < m_pgridBatch->GetRowCount(); check++)
        {
            long curIndex = _ttoi(m_pgridBatch->GetItemText(check, BatchPreColumn_Index)) - 1;
            if (curIndex > index)
            {
                curIndex--;
                CString textIndex;
                textIndex.Format(_T("%d"), curIndex + 1);
                m_pgridBatch->SetItemText(check, BatchPreColumn_Index, textIndex);
            }
        }
    }

    m_pgridBatch->Refresh();
    SetCurNum();

    SetState(BatchState::Ready);
}

void CDlgPrimaryBatchList::OnBnClickedButtonYield()
{
    CDlgYieldViewer Dlg;
    Dlg.SetVisionInspResult(g_batchManager.m_vecstBatchInspResult);

    Dlg.DoModal();
}

void CDlgPrimaryBatchList::OnBnClickedButtonInfiniteInsp()
{
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision)
    {
        enSideVisionModule sideVisionModule = (m_bIsRearModuleInspection == true)
            ? enSideVisionModule::SIDE_VISIONMODULE_REAR
            : enSideVisionModule::SIDE_VISIONMODULE_FRONT;

        m_pDlgVisionBatchInspectionUI->m_imageLotView->SetSideVisionModule(sideVisionModule);
        m_visionUnit.SetSideVisionSection(sideVisionModule);
    }

    long nGridNum = (long)(m_pgridBatch->GetRowCount());

    CString strFullFilename;
    CString strFilename;
    CString strDir;

    g_batchManager.Inspect_StartReady(m_visionUnit);
    g_batchManager.m_bInfiniteInspectionMode = TRUE;
    if (nGridNum > 1)
    {
        //{{//kircheis_201711
        BOOL bSaveMemmoryLog = FALSE;
        UINT nSaveMemoryLogTime = 60000;
        if (GetAsyncKeyState(VK_CONTROL) < 0 && GetAsyncKeyState(VK_SHIFT) < 0)
        {
            CDlgSaveMomoryLogInterval dlg;
            if (dlg.DoModal() == IDOK)
            {
                m_bSaveLogMemory = TRUE;
                bSaveMemmoryLog = TRUE;
                float fSaveIntervalTime = (float)_wtof(dlg.m_strSaveMemoryLogInterval);
                nSaveMemoryLogTime = (UINT)((float)nSaveMemoryLogTime * fSaveIntervalTime + .5f);
            }
        }
        //}}

        g_batchManager.m_nGridNum = 2;
        g_batchManager.m_nImageNum = 0;

        UINT nTimeGap = 1200000;
        if (GetAsyncKeyState(VK_SHIFT))
            nTimeGap = 120000;

        SetTimer(TIMER_MEM_LOG_20MIN, nTimeGap, NULL);
        SystemConfig::GetInstance().logger_memcheck();

        SetState(BatchState::Run); //여기에서 Timer Event를 0으로 초기화를 하기 때문에 Memory Save가 정상동작 하지않음..

        if (bSaveMemmoryLog && nSaveMemoryLogTime >= 5000) //kircheis_201711
        {
            SetTimer(TIMER_MEM_LOG, nSaveMemoryLogTime, NULL);
            SaveMemorySize();
        }
    }
}

void CDlgPrimaryBatchList::SaveMemorySize() //kircheis_201711
{
    SYSTEMTIME time;
    ::GetLocalTime(&time);
    auto nMemSize = GetUsingMemorySize(_T("iPack.exe"));
    CString strFileName;
    strFileName.Format(_T("%s\\UseMemoryTestLog.csv"), LPCTSTR(DynamicSystemPath::get(DefineFolder::Log)));
    FILE* fp = NULL;
    _tfopen_s(&fp, strFileName, _T("a"));

    nMemSize /= 1024;
    if (fp != NULL)
    {
        CString strTemp;
        strTemp.Format(_T("%d"), nMemSize);
        fprintf(fp, "%04d.%02d.%02d,%02d:%02d:%02d,%s,Kb\n", time.wYear, time.wMonth, time.wDay, time.wHour,
            time.wMinute, time.wSecond, (LPCSTR)CStringA(strTemp));
        fclose(fp);
    }
}

SIZE_T CDlgPrimaryBatchList::GetUsingMemorySize(CString strProcessName)
{
    DWORD pid[1024];
    DWORD cb;
    EnumProcesses(pid, sizeof(DWORD) * 1024, &cb);
    HMODULE hMo;
    CString strAll;
    for (int i = 0; i < 1024; ++i)
    {
        TCHAR szFileName[1024];
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid[i]);
        if (!hProcess)
            continue;
        BOOL bb = EnumProcessModules(hProcess, &hMo, sizeof(hMo), &cb);
        if (!bb)
        {
            DWORD error = GetLastError();
            LPVOID lpMsgBuf;
            FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, error, 0, (LPTSTR)&lpMsgBuf, 0, NULL);
            CString strLastErrMsg = static_cast<LPCTSTR>(lpMsgBuf);
            LocalFree(lpMsgBuf);
            CloseHandle(hProcess);
            continue;
        }
        GetModuleFileNameEx(hProcess, hMo, szFileName, 1024);
        CString strProcName(szFileName);
        if (strProcName.Find(strProcessName) != -1)
        {
            PROCESS_MEMORY_COUNTERS mem;
            GetProcessMemoryInfo(hProcess, &mem, sizeof(PROCESS_MEMORY_COUNTERS));
            CloseHandle(hProcess);
            return mem.WorkingSetSize;
        }
        CloseHandle(hProcess);
    }
    return NULL;
}

void CDlgPrimaryBatchList::call_openImageFiles()
{
    // BMP File 을 선택할 File Dialog 팝업
    CString strFileType;
    CString strFileExtension;

    strFileType = _T("bmp file format(*.bmp)|*.bmp|all files(*.*)|*.*|");
    strFileExtension = _T("*.bmp");

    CFileDialog Dlg(TRUE, strFileExtension, NULL,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, strFileType);

    // TODO : 이현민 - 힙 사용하기
    std::vector<TCHAR> filenamebuff(480000, 0);

    Dlg.m_ofn.lpstrFile = &filenamebuff[0];
    Dlg.m_ofn.nMaxFile = DWORD(filenamebuff.size());

    if (Dlg.DoModal() != IDOK)
        return;

    CString dirname;
    TCHAR dirbuff[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, dirbuff);
    dirname = dirbuff;

    POSITION pos = Dlg.GetStartPosition();

    while (pos)
    {
        CString strFullFilename = Dlg.GetNextPathName(pos);
        CString strFilename = ImageFileParser::GetFileName(strFullFilename);
        CString strDir = ImageFileParser::GetFolder(strFullFilename);

        VisionImageLotInfo info;
        if (!info.LoadInfo(strFullFilename, true))
        {
            // 로드할수 없는 이미지 파일임
            continue;
        }

        if (info.m_scanAreaIndex < 0 || info.m_scanAreaIndex >= m_visionUnit.getTrayScanSpec().GetInspectionItemCount())
        {
            ASSERT(!_T("FOV 수가 맞지 않는 이상한 이미지다"));
            continue;
        }
        auto& inspScanInfo = m_visionUnit.getTrayScanSpec().GetInspectionItem(info.m_scanAreaIndex);

        long paneCount = (long)inspScanInfo.m_unitIndexList.size();

        for (long paneIndex = 0; paneIndex < paneCount; paneIndex++)
        {
            SBatchUnitInfo BatchUnitInfo;
            BatchUnitInfo.strFileName = strFilename;
            BatchUnitInfo.strFileDir = strDir;
            BatchUnitInfo.m_scanIndex = info.m_scanAreaIndex;
            BatchUnitInfo.m_paneIndex = paneIndex;

            // Batch List 변수 Update
            g_batchManager.m_vecBatchList.push_back(BatchUnitInfo);
        }
    }

    g_batchManager.UpdateGridCtrl_List(*m_pgridBatch);

    SetCurNum();
}

void CDlgPrimaryBatchList::call_openImageFilesString(std::vector<CString> FileNames)
{
    // image들을 초기화 한다.
    OnBnClickedButtonClearAll();

    // File의 이름을 지정한다.
    for (size_t idx = 0; idx < FileNames.size(); idx++)
    {
        CString strFullFilename = FileNames[idx];
        CString strFilename = ImageFileParser::GetFileName(strFullFilename);
        CString strDir = ImageFileParser::GetFolder(strFullFilename);

        VisionImageLotInfo info;
        if (!info.LoadInfo(strFullFilename, true))
        {
            // 로드할수 없는 이미지 파일임
            continue;
        }

        if (info.m_scanAreaIndex < 0 || info.m_scanAreaIndex >= m_visionUnit.getTrayScanSpec().GetInspectionItemCount())
        {
            ASSERT(!_T("FOV 수가 맞지 않는 이상한 이미지다"));
            continue;
        }
        auto& inspScanInfo = m_visionUnit.getTrayScanSpec().GetInspectionItem(info.m_scanAreaIndex);

        long paneCount = (long)inspScanInfo.m_unitIndexList.size();

        for (long paneIndex = 0; paneIndex < paneCount; paneIndex++)
        {
            SBatchUnitInfo BatchUnitInfo;
            BatchUnitInfo.strFileName = strFilename;
            BatchUnitInfo.strFileDir = strDir;
            BatchUnitInfo.m_scanIndex = info.m_scanAreaIndex;
            BatchUnitInfo.m_paneIndex = paneIndex;

            // Batch List 변수 Update
            g_batchManager.m_vecBatchList.push_back(BatchUnitInfo);
        }
    }

    g_batchManager.UpdateGridCtrl_List(*m_pgridBatch);

    SetCurNum();
}

void CDlgPrimaryBatchList::call_quit()
{
    OnBnClickedButtonStop();
}

BOOL CDlgPrimaryBatchList::isInspectionEnable()
{
    if (m_state == BatchState::Run || m_state == BatchState::Stopping)
        return false;
    return true;
}

BOOL CDlgPrimaryBatchList::isImageOpenEnable()
{
    if (m_state == BatchState::Run || m_state == BatchState::Stopping)
        return false;
    return true;
}

void CDlgPrimaryBatchList::call_inspection()
{
    CCellID cell = m_pgridBatch->GetFocusCell();

    if (!cell.IsValid())
    {
        return;
    }

    auto visionInspectionResults = m_visionUnit.GetVisionInspectionResults();

    long ninspNum = (long)visionInspectionResults.size();
    g_batchManager.m_vecstBatchInspResult.clear();
    g_batchManager.m_vecstBatchInspResult.resize(ninspNum);

    long nGridNum = cell.row;
    if (nGridNum < (long)(m_pgridBatch->GetRowCount()) && nGridNum > 0)
    {
        long nPane = _ttoi(m_pgridBatch->GetItemText(nGridNum, BatchPreColumn_PaneID)) - 1;
        m_pgridBatch->SetFocusCell(nGridNum, BatchPreColumn_FileName);
        m_pgridBatch->EnsureVisible(nGridNum, BatchPreColumn_FileName);
        DoInspection(nGridNum, nPane, false);
    }

    SetCurNum();
}

void CDlgPrimaryBatchList::call_inspection(long nRunMode)
{
    if (nRunMode == 0) //TODO:: 해당 내용 enum 으로 재작업 필요
    {
        OnBnClickedButtonAutoInspect();
    }
    else
    {
        OnBnClickedButtonInfiniteInsp();
    }
}

void CDlgPrimaryBatchList::OnDestroy()
{
    CDialog::OnDestroy();

    g_batchManager.m_lastFrameIndex = m_pDlgVisionBatchInspectionUI->m_imageLotView->GetCurrentImageFrame();
}

void CDlgPrimaryBatchList::callBack_InspectionEnd(void* userData, VisionUnit& visionUnit, long threadIndex)
{
    ((CDlgPrimaryBatchList*)userData)->SendMessage(UM_INSPECTION_END, (WPARAM)&visionUnit, (LPARAM)threadIndex);
}

LRESULT CDlgPrimaryBatchList::OnInspectionEnd(WPARAM wParam, LPARAM lParam)
{
    long threadIndex = (long)lParam;

    VisionUnit& visionUnit = *((VisionUnit*)wParam);

    long rowIndex = visionUnit.getTrayScanSpec().GetBatchKey();

    long dataIndex = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_Index)) - 1;
    long scanIndex = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_ScanID)) - 1;
    long paneIndex = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_PaneID)) - 1;
    CString barcode = m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_Barcode);
    CString filePath = m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_FileName);

    m_pDlgVisionBatchInspectionUI->m_imageLotView->Overlay_RemoveAll();

    g_batchManager.Inspect_DataCollect(visionUnit, dataIndex);
    g_batchManager.UpdateGridCtrl_Batch(*m_pgridBatch, rowIndex);
    m_pgridBatch->Refresh();

    //m_visionMainAgent.SetPrimaryVisionUnit(threadIndex);

    if (threadIndex >= 0)
    {
        // 마지막 보여주고 있는 이미지를 Primary Vision Unit으로 가져온다
        m_visionMainAgent.CopyImage_From_OtherVisionUnit_Into_PrimaryVisionUnit(threadIndex);
    }

    ActiveVisionUnit(visionUnit);

    visionUnit.GetInspectionOverlayResult().Apply(m_pDlgVisionBatchInspectionUI->m_imageLotView->GetCoreView());

    m_pDlgVisionBatchInspectionUI->m_imageLotView->Overlay_Show(TRUE);
    m_pDlgVisionBatchInspectionUI->m_imageLotView->ShowCurrentImage();
    m_pDlgVisionBatchInspectionUI->m_imageLotView->ShowPane(paneIndex);

    if (g_batchManager.m_bInfiniteInspectionMode == false)
    {
        SaveRnRData(scanIndex, paneIndex, dataIndex, visionUnit);

        long paneCount = (long)visionUnit.getInspectionAreaInfo().m_unitIndexList.size();

        if (paneCount > 1)
        {
            filePath.AppendFormat(_T("_Pane%02d"), paneIndex + 1);
        }

        g_batchManager.m_reportData.Add(dataIndex, filePath, barcode, visionUnit);
        g_batchManager.m_reportWarpageData.Add(dataIndex, filePath, barcode, visionUnit);
        g_batchManager.m_reportSurfaceData.Add(dataIndex, filePath, barcode, visionUnit);
        g_batchManager.m_reportBodyData.Add(dataIndex, filePath, barcode, visionUnit);
    }
    SetCurNum();

    return 0;
}

void CDlgPrimaryBatchList::ActiveVisionUnit(VisionUnit& visionUnit)
{
    // Image는 Primary UI로 복사해 왔다
    m_pDlgVisionBatchInspectionUI->m_imageLotView->SetTarget(
        m_visionUnit.getImageLot(), m_visionUnit.getImageLotInsp());

    //
    m_pDlgVisionBatchInspectionUI->m_visionCommonResultDlg->Refresh(visionUnit);
    m_pDlgVisionBatchInspectionUI->m_visionCommonDebugInfoDlg->Refresh(visionUnit);
    m_pDlgVisionBatchInspectionUI->m_visionCommonDetailResultDlg->SetTarget(visionUnit);
}

void CDlgPrimaryBatchList::OnBnClickedButtonBarcode()
{
    DlgSelectVision selectVisionDlg;
    if (selectVisionDlg.DoModal() != IDOK)
        return;

    CString strFileType;
    CString strFileExtension;

    strFileType = _T("ini file format(*.ini)|*.ini|all files(*.*)|*.*|");
    strFileExtension = _T("*.ini");

    CFileDialog Dlg(TRUE, strFileExtension, NULL,
        OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_ALLOWMULTISELECT, strFileType);

    std::vector<TCHAR> filenamebuff(480000, 0);

    Dlg.m_ofn.lpstrFile = &filenamebuff[0];
    Dlg.m_ofn.nMaxFile = DWORD(filenamebuff.size());

    if (Dlg.DoModal() != IDOK)
        return;

    CString dirname;
    TCHAR dirbuff[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, dirbuff);
    dirname = dirbuff;

    POSITION pos;
    pos = Dlg.GetStartPosition();

    g_batchManager.m_barcodeData.Reset();
    while (pos)
    {
        g_batchManager.m_barcodeData.AddFile(selectVisionDlg.m_visionIndex, Dlg.GetNextPathName(pos));
    }

    g_batchManager.UpdateGridCtrl_List(*m_pgridBatch);

    SetCurNum();
}

void CDlgPrimaryBatchList::ConvertDeviceResultToBuffer(BYTE* pbyBuffer, VisionDeviceResult& deviceResult, BOOL bSave)
{
    if (pbyBuffer == NULL)
        return;

    if (bSave)
    {
        memset(m_pbyDevResultSaveBuffer, 0, MAX_SEND_RESULT_SIZE);
        CMemFile memFile(m_pbyDevResultSaveBuffer, MAX_SEND_RESULT_SIZE, MAX_SEND_RESULT_SIZE);
        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::store);
        deviceResult.Serialize(ar);
        ar.Close();

        memFile.SeekToBegin();
        BYTE* pByte = memFile.Detach();
        memcpy(pbyBuffer, pByte, MAX_SEND_RESULT_SIZE);

        if (pByte != m_pbyDevResultSaveBuffer)
        {
            ASSERT(!"A new memory allocation occurs.");
            ::free(pByte);
        }
    }
    else
    {
        CMemFile memFile;
        memFile.Attach((BYTE*)pbyBuffer, MAX_SEND_RESULT_SIZE);
        ArchiveAllType ar((CFile*)&memFile, ArchiveAllType::load);
        deviceResult.Serialize(ar);
        ar.Close();
    }
}

BOOL CDlgPrimaryBatchList::SaveIntensityCheckerLog()
{
    // Intensity Checker 2D 에서 로그 생성

    // Intensity checker 2D 사용 여부 확인
    auto visionInspectionSpecs = m_visionUnit.GetVisionInspectionSpecs();
    BOOL IntensityChekerUsage = FALSE;

    for (int nIdx = 0; nIdx < visionInspectionSpecs.size(); nIdx++)
    {
        if (visionInspectionSpecs[nIdx]->m_moduleGuid == _VISION_INSP_GUID_INTENSITYCHECKER_2D)
        {
            if (visionInspectionSpecs[nIdx]->m_use == TRUE)
            {
                IntensityChekerUsage = TRUE;
                break;
            }
        }
    }

    if (IntensityChekerUsage != TRUE)
    {
        return FALSE;
    }

    // SDY_IntensityChecker2D 데이터 추가 항목
    std::vector<bool> vecbIntensityCheckerUsage(0);
    std::vector<double> vecdIntensityAVG(0);
    std::vector<long> vecdIntensityMax(0);
    std::vector<long> vecdIntensityMin(0);

    long nDataNum = 0;
    void* pData = m_visionUnit.GetVisionDebugInfo(
        _VISION_INSP_GUID_INTENSITYCHECKER_2D, _T("Intensity Checker Usage"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        vecbIntensityCheckerUsage.resize(nDataNum);
        long* pnIntensityCheckerUsage = (long*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
            vecbIntensityCheckerUsage[nID] = pnIntensityCheckerUsage[nID];
    }

    nDataNum = 0;
    pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_INTENSITYCHECKER_2D, _T("Intensity AVG"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        vecdIntensityAVG.resize(nDataNum);
        double* pdIntensityAVG = (double*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
            vecdIntensityAVG[nID] = pdIntensityAVG[nID];
    }

    nDataNum = 0;
    pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_INTENSITYCHECKER_2D, _T("Intensity Max"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        vecdIntensityMax.resize(nDataNum);
        long* pdIntensityMax = (long*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
            vecdIntensityMax[nID] = pdIntensityMax[nID];
    }

    nDataNum = 0;
    pData = m_visionUnit.GetVisionDebugInfo(_VISION_INSP_GUID_INTENSITYCHECKER_2D, _T("Intensity Min"), nDataNum);
    if (nDataNum > 0 && pData != nullptr)
    {
        vecdIntensityMin.resize(nDataNum);
        long* pdIntensityMin = (long*)pData;
        for (long nID = 0; nID < nDataNum; nID++)
            vecdIntensityMin[nID] = pdIntensityMin[nID];
    }

    //조명 시간에 대한 로그를 위해 파일 생성
    CFile File;
    CString strFileName("");
    CString strFileSaveFolder = DynamicSystemPath::get(DefineFolder::Log) + _T("IntensityLog\\");

    Ipvm::CreateDirectories(strFileSaveFolder);

    // 파일을 생성한다. 혹은 기존에 있으면 추가 한다.
    strFileName.Format(_T("%sIntensityChecker2D(%s)_%s.CSV"), (LPCTSTR)strFileSaveFolder,
        (LPCTSTR)SystemConfig::GetInstance().m_strVisionInfo, (LPCTSTR)CTime::GetCurrentTime().Format(_T("%Y%m%d")));

    if (!File.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite))
    {
    }
    else
    {
        CArchive ar(&File, CArchive::store);

        // 파일의 맨 마지막을 찾는다.
        long nFileEnd = CAST_LONG(File.SeekToEnd());

        CString strIntensityLog;
        strIntensityLog.Empty();

        // 파일이 없을 경우에는 맨 윗 라인을 새로 만들어준다.
        if (nFileEnd == 0)
        {
            CString strIntensityLogTitle;
            strIntensityLogTitle.Empty();
            strIntensityLogTitle.Format(_T("No,FileName,Scan,Pane,"));

            CString strIntensityLogTitleAdder;
            strIntensityLogTitleAdder.Empty();

            for (long nIdx = 0; nIdx < vecbIntensityCheckerUsage.size(); nIdx++)
            {
                long nRoiNum = nIdx / 3;
                long nFrameNum = nIdx % 3;
                strIntensityLogTitleAdder.Format(_T("ROI%d Frame%d AVG,ROI%d Frame%d Min,ROI%d Frame%d Max,"), nRoiNum,
                    nFrameNum, nRoiNum, nFrameNum, nRoiNum, nFrameNum);
                strIntensityLogTitle += strIntensityLogTitleAdder;
            }
            strIntensityLogTitleAdder.Empty();
            strIntensityLogTitleAdder.Format(_T("\r\n"));
            strIntensityLogTitle += strIntensityLogTitleAdder;

            ar.WriteString(strIntensityLogTitle);
        }

        // 검사 결과를 형식에 맞추어 추가한다.
        CString strIntensityLogAdder;
        strIntensityLogAdder.Empty();

        //auto data = g_batchManager.m_reportData;

        long rowIndex
            = m_visionUnit.getTrayScanSpec().GetBatchKey(); //m_visionUnit.m_pScanAreaImageInfo->GetBatchKey();
        long dataIndex = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_Index));
        long scanIndex = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_ScanID));
        long paneIndex = _ttoi(m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_PaneID));
        CString filePath = m_pgridBatch->GetItemText(rowIndex, BatchPreColumn_FileName);

        strIntensityLogAdder.Format(_T("%d,%s,%d,%d,"), dataIndex, (LPCTSTR)filePath, scanIndex, paneIndex);
        strIntensityLog += strIntensityLogAdder;

        for (int nIdx = 0; nIdx < vecbIntensityCheckerUsage.size(); nIdx++)
        {
            strIntensityLogAdder.Empty();

            if (vecbIntensityCheckerUsage[nIdx] == FALSE)
            {
                strIntensityLogAdder.Format(_T("-,-,-,"));

                strIntensityLog += strIntensityLogAdder;
            }
            else
            {
                strIntensityLogAdder.Format(_T("%0.2f,%0.2f,%0.2f,"), vecdIntensityAVG[nIdx],
                    (float)vecdIntensityMin[nIdx], (float)vecdIntensityMax[nIdx]);

                strIntensityLog += strIntensityLogAdder;
            }
        }

        // 파일에 추가
        strIntensityLogAdder.Empty();
        strIntensityLogAdder.Format(_T("\r\n"));
        strIntensityLog += strIntensityLogAdder;

        ar.WriteString(strIntensityLog);

        ar.Close();
        File.Close();
    }
    return TRUE;
}

void CDlgPrimaryBatchList::OnCbnSelchangeComboSideFRModule()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    static const bool bIsSideVision = (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_SIDE_INSP);

    if (bIsSideVision == false)
        return;

    m_bIsRearModuleInspection
        = (bool)(m_ctrlCmbSideModuleSelector.GetCurSel() == (long)(enSideVisionModule::SIDE_VISIONMODULE_REAR));

    m_bIsRearModuleInspection &= bIsSideVision;

    enSideVisionModule sideVisionModule = (m_bIsRearModuleInspection == true)
        ? enSideVisionModule::SIDE_VISIONMODULE_REAR
        : enSideVisionModule::SIDE_VISIONMODULE_FRONT;

    m_pDlgVisionBatchInspectionUI->m_imageLotView->SetSideVisionModule(sideVisionModule);
    m_visionUnit.SetSideVisionSection(sideVisionModule);
    m_visionMainAgent.SetSideVisionSection(sideVisionModule);
}
