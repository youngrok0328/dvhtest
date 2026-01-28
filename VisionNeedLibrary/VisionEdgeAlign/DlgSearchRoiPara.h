#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
class PropertyGrid_Impl;
} // namespace VisionEdgeAlign

//HDR_6_________________________________ Header body
//
class DlgSearchRoiPara : public CDialogEx
{
    DECLARE_DYNAMIC(DlgSearchRoiPara)

public:
    DlgSearchRoiPara(VisionEdgeAlign::Para& para, VisionEdgeAlign::PropertyGrid_Impl* gridEngine,
        CWnd* pParent); // standard constructor
    virtual ~DlgSearchRoiPara();

    void UpdateList();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_SEARCH_ROI_PARA
    };
#endif

protected:
    VisionEdgeAlign::PropertyGrid_Impl* m_gridEngine;
    VisionEdgeAlign::Para& m_para;
    CGridCtrl m_grid;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnOK()
    {
    }
    afx_msg void OnCancel()
    {
    }

    afx_msg void OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
