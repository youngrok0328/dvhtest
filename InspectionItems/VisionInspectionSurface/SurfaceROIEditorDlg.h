#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionSurface;
class CSurfaceItem;
class CSurfaceAlgoPara;
class ImageViewEx;
class CSurfaceItemResult;
class Para_PolygonRects;
class SurfacePrepairResult;
class DlgSubThresholdAdaptive;
class DlgSubThresholdNormal;

//HDR_6_________________________________ Header body
//
enum PolygonType
{
    enum_Rectangle = 0,
    enum_Ellipse,
    enum_Polygon,
    enum_Detect_Rectangle,
};

class CSurfaceROIEditorDlg : public CDialog
{
    // Construction

public:
    int AddIgnoreData(CString& o_NewName);
    void Refresh_MaskList();

    CSurfaceROIEditorDlg(VisionInspectionSurface* pVisionInsp, BOOL bUseUserMask, CSurfaceItem& SurfaceItem,
        long nSurfaceID, long bUseVmapImage, CWnd* pParent = NULL); // standard constructor
    ~CSurfaceROIEditorDlg();

    CSurfaceAlgoPara& GetAlgoPara();

    VisionInspectionSurface* m_pVisionInsp;

    // Dialog Data
    //{{AFX_DATA(CSurfaceROIEditorDlg)
    enum
    {
        IDD = IDD_SURFACEROIEDITORDLG
    };
    CListBox m_lstCustomROI;
    int m_nCurSubArea_y;
    int m_nCurSubArea_x;
    float m_inspROIOffset_Left;
    float m_inspROIOffset_Top;
    float m_inspROIOffset_Right;
    float m_inspROIOffset_Bottom;
    BOOL m_bFirstMode;
    //}}AFX_DATA
    CGridCtrl m_gridIgnore;
    CGridCtrl m_gridMask;

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSurfaceROIEditorDlg)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    SurfacePrepairResult* m_prepairResult;
    DlgSubThresholdAdaptive* m_subThresholdAdaptive;
    DlgSubThresholdNormal* m_subThresholdNormal;

    float& GetThresholdDark();
    float& GetThresholdBright();
    BOOL& GetThresholdNormal();
    long& GetThresholdLow();
    long& GetThresholdHigh();

    void MakeGrayImage(bool applyCombine);

    BOOL m_bUseUserMask;

    void uOnUpdateThresholdScrollBar();
    void uOnUpdateROISize();

    void GetROI(Para_PolygonRects& ignorePara);
    void SetROI(const Para_PolygonRects& ignorePara);
    void GetParameter();
    long GetSelectedIgnoreType();
    long GetSelectedMask();

    Ipvm::Image8u m_grayImage;
    Ipvm::Image8u m_thresholdImage;

    long m_nSubAreaSize_x;
    long m_nSubAreaSize_y;

    long m_nSurfaceID;

    bool m_is3D;
    long m_Used3DImageType;

    CSurfaceItem* m_pSurfaceItem;
    CSurfaceAlgoPara* m_pAlgoPara;
    ImageViewEx* m_imageView;
    Ipvm::ProfileView* m_profileView;
    long m_profileThreshold;

    CComboBox m_cmbProfileType;
    CComboBox m_ThresholdType;

    // Generated message map functions
    //{{AFX_MSG(CSurfaceROIEditorDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnButtonIgnoreAdd();
    afx_msg void OnButtonIgnoreSub();
    virtual void OnOK();
    afx_msg void OnBtnAddAdd();
    afx_msg void OnBtnAddSub();
    afx_msg void OnBTNMaskOrderUP();
    afx_msg void OnBTNMaskOrderDOWN();
    afx_msg void OnBTNMaskOrderDel();
    afx_msg void OnChangeEDITInspROIOffsetLeft();
    afx_msg void OnChangeEDITInspROIOffsetRight();
    afx_msg void OnChangeEDITInspROIOffsetTop();
    afx_msg void OnChangeEDITInspROIOffsetBottom();

    afx_msg void OnGridClick(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnLbnSelchangeListCustomroi();

    //	DECLARE_EVENTSINK_MAP()
    //}}AFX_MSG
    afx_msg void OnGridMaskEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridModifyClick(NMHDR* pNotifyStruct, LRESULT* result); //kircheis_Hy
    DECLARE_MESSAGE_MAP()

public:
    void UpdateMainPrepairResult(bool updateThreshold = true);
    void UpdateMainThresholdImage();
    void UpdateProfileView();

    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedBtnViewInputImage();
    afx_msg void OnBnClickedCheckCombineImage();
    afx_msg void OnCbnSelchangeComboThresholdType();

    static void ROI_Changed(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength);
    void ROI_Changed(const int32_t id, const wchar_t* key, const size_t keyLength);
    afx_msg void OnCbnSelchangeComboProfiletype();
    afx_msg LRESULT OnChangedThreshold(WPARAM target, LPARAM);
};
