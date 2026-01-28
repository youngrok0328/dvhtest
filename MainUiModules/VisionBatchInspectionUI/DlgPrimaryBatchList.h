#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class DlgVisionBatchInspectionUI;
class CDlgYieldViewer;
class CPackageSpec;
class VisionMainAgent;
class VisionUnit;
class VisionDeviceResult;
struct stBatchInspResult;

//HDR_6_________________________________ Header body
//
class CDlgPrimaryBatchList : public CDialog
{
    DECLARE_DYNAMIC(CDlgPrimaryBatchList)

public:
    CDlgPrimaryBatchList(VisionMainAgent& visionMain, VisionUnit& visionUnit, const CRect& rtPositionOnParent,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgPrimaryBatchList();

    BOOL isInspectionEnable();
    BOOL isImageOpenEnable();
    void call_inspection();
    void call_inspection(long nRunMode);
    void call_openImageFiles();
    void call_openImageFilesString(std::vector<CString> FileNames);
    void call_quit();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_BATCH_LIST
    };

    void Refresh();
    void SetCurNum();

    //변수 정의

public:
    VisionMainAgent& m_visionMainAgent;
    VisionUnit& m_visionUnit;
    const CRect m_rtPositionOnParent;
    DlgVisionBatchInspectionUI* m_pDlgVisionBatchInspectionUI;
    CGridCtrl* m_pgridBatch;

    CDlgYieldViewer* m_pDlgYieldViewer;

    long m_batchInspTime;

protected:
    enum class BatchState
    {
        Ready,
        Run,
        Stopping,
        Stop,
    };

    enum class BatchRunState
    {
        Idle,
        ThreadReadyCheck,
    };

    BOOL m_bSaveLogMemory;
    BatchState m_state;
    BatchRunState m_runState;
    bool DoInspection(long nGrid, long nPane, bool threadRun);
    BOOL Send2ndInspInfo(VisionDeviceResult& devResult,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // ksy for 2nd inspection
    BOOL CollectMVRejectData(VisionDeviceResult& devResult,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // ksy for collecting learning images (Insp item)
    BOOL SaveAllInspImage(VisionDeviceResult& devResult,
        const enSideVisionModule i_eSideVisionModule
        = enSideVisionModule::SIDE_VISIONMODULE_FRONT); // ksy for collecting learning images (All image)
    BOOL WriteRnRResult();
    BOOL WriteAutomationResult(); // SDY_Automation 자동화 레포트를 남긴다.
    void SetState(BatchState state);
    void ActiveVisionUnit(VisionUnit& visionUnit);
    BOOL SaveIntensityCheckerLog();

    BYTE* m_pbyDevResultSaveBuffer;
    void ConvertDeviceResultToBuffer(BYTE* pbyBuffer, VisionDeviceResult& deviceResult, BOOL bSave);

    static void callBack_InspectionEnd(void* userData, VisionUnit& visionUnit, long threadIndex);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    bool m_bIsRearModuleInspection;

    DECLARE_MESSAGE_MAP()

    // 함수 정의

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnDblclkGridBatchList(NMHDR* pNMHDR, LRESULT* pResult);

public:
    CButton m_buttonAutoInspect;
    CButton m_buttonClearAll;

    void SaveRnRData(long nScan, long nPane, long dataIndex, VisionUnit& visionUnit);

    void SaveMemorySize(); //kircheis_201711
    SIZE_T GetUsingMemorySize(CString strProcessName);

    afx_msg void OnBnClickedButtonAutoInspect();
    afx_msg void OnBnClickedButtonClearAll();
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnBnClickedButtonStop();
    afx_msg void OnBnClickedButtonResume();
    afx_msg void OnBnClickedButtonAsort();
    afx_msg void OnBnClickedButtonDsort();
    afx_msg void OnBnClickedButtonDelete();
    afx_msg void OnBnClickedButtonYield();
    afx_msg void OnBnClickedButtonInfiniteInsp();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnInspectionEnd(WPARAM wParam, LPARAM lParam);
    afx_msg void OnBnClickedButtonBarcode();
    CComboBox m_ctrlCmbSideModuleSelector;
    afx_msg void OnCbnSelchangeComboSideFRModule();
};
