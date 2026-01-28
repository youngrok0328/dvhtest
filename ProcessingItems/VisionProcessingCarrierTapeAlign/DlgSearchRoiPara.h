#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class Para;
class PropertyGrid;

//HDR_6_________________________________ Header body
//
class DlgSearchRoiPara : public CDialogEx
{
    DECLARE_DYNAMIC(DlgSearchRoiPara)

public:
    DlgSearchRoiPara(Para& para, PropertyGrid* gridEngine, int32_t gridIndex,
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
    Para& m_para;
    PropertyGrid* m_gridEngine;
    int32_t m_gridIndex;

    CGridCtrl m_grid;
    void UpdateCarrierTapeInfo();
    void UpdatePocketInfo();
    bool CheckChangedCarrierTapeValues();
    bool CheckChangedPocketValues();

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
