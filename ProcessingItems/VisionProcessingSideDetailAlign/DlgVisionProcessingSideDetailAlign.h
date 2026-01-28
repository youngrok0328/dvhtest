#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedComponent/Label/NewLabel.h"
#include "../../UserInterfaceModules/ImageLotView/ImageLotView.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDebugInfoDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonDetailResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonLogDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonResultDlg.h"
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonSpecDlg.h"

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
class VisionProcessingSideDetailAlign;
class VisionProcessingSideDetailAlignPara;
enum PAD_TYPE : long;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
class CDlgVisionProcessingSideDetailAlign : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionProcessingSideDetailAlign)

public:
    CDlgVisionProcessingSideDetailAlign(const ProcessingDlgInfo& procDlgInfo,
        VisionProcessingSideDetailAlign* pVisionInsp, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionProcessingSideDetailAlign();

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
    ImageLotView* m_imageLotViewRearSide;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonLogDlg* m_procCommonLogDlgRear;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlgRear;

    CXTPPropertyGrid* m_propertyGrid;

    VisionProcessingSideDetailAlign* m_pVisionInsp;
    VisionProcessingSideDetailAlignPara* m_pVisionPara;

    BOOL m_bShowResultDetail;
    long m_nImageID;

    CString m_strMaxBlobSizeText;
    CString m_strBlobResultText;

    CTabCtrl m_TabResult;

    // Side 선택을 위한 Label - 2024.05.29_JHB
    CNewLabel m_Label_Side_Front_Status;
    CNewLabel m_Label_Side_Rear_Status;
    ////////////////////////////////////////////

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg void OnDestroy();
    afx_msg void AfterInspect();
    afx_msg void OnBnClickedButtonInspect();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    LRESULT OnImageLotViewRoiChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

    void ClickedButtonSubstrateImageCombine();
    void ClickedButtonGlassImageCombine();
    void UpdatePropertyGrid();

    void ShowImage(long nFrameIdx);

    // Side 선택을 위한 Label - 2024.05.29_JHB
    void SetInitialSideVisionSelector();

    afx_msg void OnStnClickedStaticSideFrontDetailalign();
    afx_msg void OnStnClickedStaticSideRearDetailalign();
    ///////////////////////////////////////////

private:
    long m_nCurCommonTab;
    void UpdateCommonTabShow();
};