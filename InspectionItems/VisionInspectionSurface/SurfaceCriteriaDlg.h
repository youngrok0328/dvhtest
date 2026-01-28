#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageViewEx;
class CSurfaceItem;
class CSurfaceItemResult;
class VisionInspectionSurface;
struct SItemSpec;

//HDR_6_________________________________ Header body
//
class CSurfaceCriteriaDlg : public CDialog
{
    // Construction

public:
    CSurfaceCriteriaDlg(VisionInspectionSurface* pVisionInsp, const CSurfaceItem& SurfaceItem, long nCurrentUnit,
        long nSurfaceID, BOOL bMaskGenMode, long bUseVmap, CWnd* pParent = NULL); // standard constructor
    virtual ~CSurfaceCriteriaDlg();

    VisionInspectionSurface* m_pVisionInsp;

    int GetColSPos();
    int GetCriterialNum();
    void SetROI();
    void GetROI();
    void RefreshROIList();

    CSurfaceItem& GetSurfaceItem();

    // Dialog Data
    //{{AFX_DATA(CSurfaceCriteriaDlg)
    enum
    {
        IDD = IDD_SURFACECRITERIADLG
    };
    CListCtrl m_lstROI;
    CComboBox m_cboUnit;
    int m_nPolygonNum;
    CString m_strROIName;
    int m_nROIType;
    //}}AFX_DATA
    CGridCtrl m_gridCriteria;
    CGridCtrl m_gridObjects;

    std::vector<Ipvm::Point32r2> m_vecfCen;
    std::vector<Ipvm::Rect32s> m_vecrtObj; // bk : 좀 더 명시적으로 확인하기 위해 추가.
    std::vector<long> m_objects_label;

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSurfaceCriteriaDlg)

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    //}}AFX_VIRTUAL

    // Implementation

protected:
    Ipvm::Image8u m_image;
    Ipvm::Image32s m_labelImage;

    bool m_is3D;
    void InitializeObjectsGrid();
    void InitializeCriteriaGrid();

    long m_nCurrentUnit;
    void ConvertUnit(float fFactor);
    void ConvertUnit(long nCurrentUnit, long nNewUnit);
    void ConvertUnit(CGridCtrl& GridCtrl, long nRow, long nCol, float fFactor);
    float GetConvertUnitFactor(long nCurrentUnit, long nNewUnit);

    void GetCriteriaMin(long nRow, long nCol, SItemSpec& ItemSpec);
    void GetCriteriaMax(long nRow, long nCol, SItemSpec& ItemSpec);

    void SetCriteriaMin(long nRow, long nCol, const SItemSpec& ItemSpec);
    void SetCriteriaMax(long nRow, long nCol, const SItemSpec& ItemSpec);

    void SetCellColor(int nRow, int nCol);
    void SetCellType(int nRow, int nCol);

    void UpdateSpec(BOOL bSaveAndValidate);
    void UpdateDefectOverlay();

    long m_nSurfaceID;
    BOOL m_bMaskGenMode;
    CSurfaceItem* m_pSurfaceItem;

    ImageViewEx* m_imageView;

    // Generated message map functions
    //{{AFX_MSG(CSurfaceCriteriaDlg)
    virtual BOOL OnInitDialog();
    //	virtual void OnCancel() {};

    afx_msg void OnSelchangeComboUnit();
    afx_msg void OnButtonSurfaceInsertCriteria();
    afx_msg void OnButtonSurfaceRemoveCriteria();
    virtual void OnOK();
    afx_msg void OnBtnAddRoi();
    afx_msg void OnBtnDelRoi();
    //}}AFX_MSG
    afx_msg void OnGridObjectSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnGridCriteriaEditEnd(NMHDR* pNMHDR, LRESULT* pResult);
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedOk();
    static int CompareFunction(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    afx_msg void OnBnClickedButtonInspection();

    static void ROI_Changed(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength);
    void ROI_Changed(const int32_t id, const wchar_t* key, const size_t keyLength);
    afx_msg void OnBnClickedCheckShowSelectedDefectsOnly();
    afx_msg void OnBnClickedCheckShowSelectedDefectsLabel();
};
