#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class DlgVisionProcessingPadAlign2D;
class VisionProcessingPadAlign2D;
class ImageLotView;
class ParaDB;

//HDR_6_________________________________ Header body
//
class DlgSelectPAD : public CDialog
{
    DECLARE_DYNAMIC(DlgSelectPAD)

public:
    DlgSelectPAD(VisionProcessingPadAlign2D* visionInsp, ImageLotView* imageLotView, ParaDB* paraDB,
        DlgVisionProcessingPadAlign2D* parent); // standard constructor
    virtual ~DlgSelectPAD();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_SELECT_PAD
    };
#endif

protected:
    ImageLotView* m_imageLotView;
    ParaDB* m_paraDB;
    DlgVisionProcessingPadAlign2D* m_parent;

    Ipvm::Rect32s m_selectBoundary;
    bool m_padListUpdateOK;
    std::vector<long> m_select_padIndexList_for_menu;

    void UpdatePadList();
    void UpdateFixTheBoundary();
    bool GetSelectPadIndexList(std::vector<long>& o_list);

    VisionProcessingPadAlign2D* m_visionInsp;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CListCtrl m_lstPAD;
    CButton m_chkFixTheBoundary;
    virtual BOOL OnInitDialog();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnLvnItemchangedListPad(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnMenuSetAlgo();
    afx_msg void OnMenuDeleteAlgo();
    afx_msg void OnBnClickedCheckFixtheboundary();
};
