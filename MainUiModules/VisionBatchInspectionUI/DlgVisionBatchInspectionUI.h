#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CDlgPrimaryBatchList;
class ImageLotView;
class VisionCommonSpecDlg;
class VisionCommonResultDlg;
class VisionCommonDetailResultDlg;
class VisionCommonTextResultDlg;
class VisionCommonDebugInfoDlg;
class VisionMainAgent;
class VisionUnit;

//HDR_6_________________________________ Header body
//
class DlgVisionBatchInspectionUI : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionBatchInspectionUI)

public:
    DlgVisionBatchInspectionUI(
        VisionMainAgent& visionMainAgent, VisionUnit& visionUnit, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~DlgVisionBatchInspectionUI();

    void OnJobChanged();
    BOOL isInspectionEnable();
    BOOL isImageOpenEnable();
    void call_inspection();
    void call_inspection(long nRunMode);
    void call_openImageFiles();
    void call_openImageFiles(std::vector<CString> FileNames);
    void call_quit();

    VisionMainAgent& m_visionMainAgent;
    VisionUnit& m_visionUnit;

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_PRIMARY
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnDestroy();

    // 함수 정의

public:
    // 변수 정의

public:
    CDlgPrimaryBatchList* m_batchList;
    ImageLotView* m_imageLotView;
    VisionCommonSpecDlg* m_visionCommonSpecDlg;
    VisionCommonResultDlg* m_visionCommonResultDlg;
    VisionCommonDetailResultDlg* m_visionCommonDetailResultDlg;
    VisionCommonTextResultDlg* m_visionCommonTextResultDlg;
    VisionCommonDebugInfoDlg* m_visionCommonDebugInfoDlg;

public:
    CTabCtrl m_tabResult;
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);
};
