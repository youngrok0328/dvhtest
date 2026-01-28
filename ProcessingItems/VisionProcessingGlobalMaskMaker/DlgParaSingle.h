#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "MaskListCtrl.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class DlgPara;
class VisionProcessingGlobalMaskMaker;
class CVisionProcessingGlobalMaskMakerPara;

//HDR_6_________________________________ Header body
//
class DlgParaSingle : public CDialogEx
{
    DECLARE_DYNAMIC(DlgParaSingle)

public:
    DlgParaSingle(
        VisionProcessingGlobalMaskMaker* pVisionInsp, DlgPara& parent, CWnd* parentWnd); // standard constructor
    virtual ~DlgParaSingle();

    void UpdateLayerMaskList();
    void UpdateProperty();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_PARA_SINGLE
    };
#endif

protected:
    std::vector<std::pair<CString, long>> m_preparedMasks; // long (type) : 0/ROI, 1/Mask
    std::vector<CString> m_userROITypes;
    void SetROIType(long index, long userType);

    DlgPara& m_parent;
    VisionProcessingGlobalMaskMaker* m_pVisionInsp;
    CVisionProcessingGlobalMaskMakerPara* m_pVisionPara;
    CXTPPropertyGrid* m_property;
    long m_selectMenuRoiIndex;

    void UpdateProperty_ROIInfo();
    void UpdateRefreshMaskInfo(long maskIndex);
    void UpdateProperty_AuthorityAndRefreshMaskInfo();
    CString GetCategortyName(long roiIndex);
    bool IsPreparedROI(LPCTSTR name);

    void menuFunction_deleteRoiIndex(long roiIndex);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    MaskListCtrl m_maskList;
    CListBox m_lstPreparedImage;
    CListBox m_lstUserRoi;

    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnDestroy();
    afx_msg void OnLbnDblclkListUserRoi();
    afx_msg void OnLbnDblclkListPreparedImage();
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);

    void OnGridNotify_itemValueChanged(CXTPPropertyGridItem* item);
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

    afx_msg LRESULT OnMaskSelected(WPARAM, LPARAM);
    afx_msg LRESULT OnMaskMoved(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMaskDBClicked(WPARAM wParam, LPARAM);
    afx_msg LRESULT OnMaskRClicked(WPARAM wParam, LPARAM);
};
