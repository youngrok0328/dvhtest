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
class VisionProcessingCoupon2D;
class VisionProcessingCoupon2DPara;
enum PAD_TYPE : long;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionProcessingCoupon2D : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionProcessingCoupon2D)

public:
    CDlgVisionProcessingCoupon2D(const ProcessingDlgInfo& procDlgInfo, VisionProcessingCoupon2D* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionProcessingCoupon2D();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    const ProcessingDlgInfo& m_procDlgInfo;
    ImageLotView* m_imageLotView;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    CXTPPropertyGrid* m_propertyGrid;

    VisionProcessingCoupon2D* m_pVisionInsp;
    VisionProcessingCoupon2DPara* m_pVisionPara;

    BOOL m_bShowResultDetail;
    long m_nImageID;

    CString m_strMaxBlobSizeText;
    CString m_strBlobResultText;

    CTabCtrl m_TabResult;

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    afx_msg void AfterInspect();
    LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    LRESULT OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);
    void ShowThresholdImage(bool thresholdDisplay);

    void GetROI();
    void SetROI();

    void ClickedButtonImageCombine();
    void UpdatePropertyGrid();
};