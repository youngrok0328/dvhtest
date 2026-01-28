#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionProcessingPadAlign2D;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class ParaDB;
class DlgSelectPAD;
enum PAD_TYPE : long;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class DlgVisionProcessingPadAlign2D : public CDialog
{
    DECLARE_DYNAMIC(DlgVisionProcessingPadAlign2D)

public:
    DlgVisionProcessingPadAlign2D(const ProcessingDlgInfo& procDlgInfo, VisionProcessingPadAlign2D* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~DlgVisionProcessingPadAlign2D();

    void SetInitParaWindow();
    void OnBnClickedButtonInspect();
    void UpdatePropertyGrid();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    CString m_select_pad_name;

    PAD_TYPE GetPadType(LPCTSTR PAD_name);
    void CallSetPADAlgorithm();
    void CallImageCombine();
    void CallAlgorithmDB();

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    const ProcessingDlgInfo& m_procDlgInfo;
    ImageLotView* m_imageLotView;
    VisionProcessingPadAlign2D* m_pVisionInsp;
    ParaDB* m_paraDB;
    CXTPPropertyGrid* m_property_para;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;
    DlgSelectPAD* m_dialog_selectPAD;

public:
    CTabCtrl m_TabResult;

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewRoiChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};
