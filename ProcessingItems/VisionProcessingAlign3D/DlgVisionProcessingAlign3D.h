#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class ImageLotView;
class VisionProcessingAlign3D;
class CVisionProcessingAlign3DPara;
class CPackageSpec;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class ProcCommonGridGroupDlg;
struct ProcessingDlgInfo;
struct S3DMergeResult;

//HDR_6_________________________________ Header body
//
class CDlgVisionProcessingAlign3D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionProcessingAlign3D)

public:
    CDlgVisionProcessingAlign3D(const ProcessingDlgInfo& procDlgInfo, VisionProcessingAlign3D* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionProcessingAlign3D();

    void OnBnClickedButtonInspect();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

private:
    const ProcessingDlgInfo& m_procDlgInfo;
    VisionProcessingAlign3D* m_pVisionInsp;
    CPackageSpec& m_packageSpec;
    CVisionProcessingAlign3DPara& m_visionPara;
    S3DMergeResult& m_3DMergeResult;

    ImageLotView* m_imageLotView;
    ProcCommonGridGroupDlg* m_procCommonGridGroup;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    CTabCtrl m_tabResult;

    virtual BOOL OnInitDialog();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);
    afx_msg LRESULT OnImageLotViewSelChangedRaw(WPARAM /*wparam*/, LPARAM /*lparam*/);
    afx_msg LRESULT OnImageLotViewSelChangedImage(WPARAM /*wparam*/, LPARAM /*lparam*/);
};
