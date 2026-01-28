#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/gridctrl.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageViewEx;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
class CDlgImageGrayProcManager : public CDialog
{
    DECLARE_DYNAMIC(CDlgImageGrayProcManager)

public:
    CDlgImageGrayProcManager(
        VisionProcessing& proc, bool isRaw, long baseFrameIndex, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgImageGrayProcManager();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_IMAGE_COMBINE
    };

    //영훈_ImageCombine_20101221

public:
    void SetParameter(ImageProcPara* pProcManagePara, Ipvm::Rect32s rtPaneROI);

    VisionProcessing& m_proc;
    bool m_isRaw;
    long m_imageFrameCount;
    ImageProcPara* m_pProcManagePara_;
    ImageProcPara m_ProcManagePara;
    ImageViewEx* m_imageView;

    int m_baseFrameIndex;

    CComboBox m_cmbCombineMode;
    CComboBox m_cmbAuxFrame;
    CTabCtrl m_tabSubFrame;
    CTabCtrl m_tabMainFrame;
    CListBox m_lstAlgorithm;
    CGridCtrl m_gridApply;
    CStatic m_staticAuxCmbName;

    int m_nImageSizeX;
    int m_nImageSizeY;
    int m_nFrame;
    Ipvm::Rect32s m_rtPaneROI;
    CRect m_rtROI;
    BOOL m_bImagePaneView;

    void DrawPreViewImage(long nFrame, std::vector<short> vecnApplyProc, std::vector<short> vecnApplyParams);
    void UpdateApplyList(int nFrame);
    long GetSelectedGridCellRow();

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual void OnOK();

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    BOOL Initialize();
    afx_msg void OnBnClickedButtonUp();
    afx_msg void OnBnClickedButtonDown();
    afx_msg void OnCbnSelchangeComboCombineMode();
    afx_msg void OnCbnSelchangeComboCombineAuxFrame();
    afx_msg void OnLbnDblclkListAlgorithm();
    afx_msg void OnTcnSelchangeTabAlgorithm(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTcnSelchangeTabFrame(NMHDR* pNMHDR, LRESULT* pResult);

    afx_msg void OnGridDbClick(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
};
