#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ResultLayer.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionProcessingGlobalMaskMaker;
class CVisionProcessingGlobalMaskMakerPara;
class ImageLotView;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionAlignResult;
class DlgLayer;
class DlgPara;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgSetupUI : public CDialog
{
    DECLARE_DYNAMIC(CDlgSetupUI)

public:
    CDlgSetupUI(const ProcessingDlgInfo& procDlgInfo, VisionProcessingGlobalMaskMaker* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgSetupUI();

    void FreeOtherMaskImage();
    void SetROI();
    void SetDisplayImage(long imageIndex = -1);

    long GetCurrentLayer() const;
    void SetCurrentLayer(long layerIndex);
    bool isLayerValid(long layerIndex);
    CVisionProcessingGlobalMaskMakerPara& GetPara();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    long m_currentLayer;
    ResultLayer m_resultLayer;
    void CreateBackImage(long imageIndex);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    Ipvm::Image8u m_sourceImage;
    Ipvm::Image8u m_otherMaskImage;

    const ProcessingDlgInfo& m_procDlgInfo;
    VisionProcessingGlobalMaskMaker* m_pVisionInsp;
    CVisionProcessingGlobalMaskMakerPara* m_pVisionPara;
    ImageLotView* m_imageLotView;
    DlgLayer* m_layerDlg;
    DlgPara* m_paraDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    //-----------------------------
    //DLG 초기화를 위한 변수 & 함수
    //-----------------------------

public:
    CTabCtrl m_TabResult;

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    afx_msg void OnClose();
    void OnBnClickedButtonInspect();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewRoiChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewImageChanged(WPARAM imageIndex, LPARAM lparam);
    afx_msg LRESULT OnRefreshUI(WPARAM, LPARAM);

    bool GetROI();
};
