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
class VisionProcessingNGRV;
class VisionProcessingNGRVPara;
class DlgVisionProcessingNGRV_BTM2D;
class DlgVisionProcessingNGRV_TOP2D;
class ProcCommonGridGroupDlg;
struct ProcessingDlgInfo;

//HDR_6_________________________________ Header body
//
//CPP_7_________________________________ Implementation body
//
enum GridChildItemID
{
    ITEM_ADD,
    ITEM_DELETE,
    ITEM_APPLY,
};

class CDlgVisionProcessingNGRV : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionProcessingNGRV)

public:
    CDlgVisionProcessingNGRV(const ProcessingDlgInfo& procDlgInfo, VisionProcessingNGRV* pVisionInsp,
        CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~CDlgVisionProcessingNGRV();

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
    VisionProcessingNGRV* m_visionInsp;
    VisionProcessingNGRVPara* m_visionParaBTM;
    VisionProcessingNGRVPara* m_visionParaTOP;
    DlgVisionProcessingNGRV_BTM2D* m_bottom2Dtab;
    DlgVisionProcessingNGRV_TOP2D* m_top2Dtab;
    ProcCommonGridGroupDlg* m_procCommonGridGroup;
    CXTPPropertyGrid* m_propertyGrid;

    CRect m_imageRectBTM;
    CRect m_imageRectTOP;

    // Function
    void ShowImage(bool isTop);

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnDestroy();
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewRoiChanged(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewSelChangedRaw(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnImageLotViewSelChangedImage(WPARAM wparam, LPARAM lparam);
    CTabCtrl m_resultTab;
    CTabCtrl m_selectionTab;
    afx_msg void OnTcnSelchangeTabNgrvSideSelection(NMHDR* pNMHDR, LRESULT* pResult);
};
