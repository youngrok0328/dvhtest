#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "MaskListCtrl.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CDlgSetupUI;

//HDR_6_________________________________ Header body
//
class DlgLayer : public CDialogEx
{
    DECLARE_DYNAMIC(DlgLayer)

public:
    DlgLayer(CDlgSetupUI* parent); // standard constructor
    virtual ~DlgLayer();

    void UpdateLayerList();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_LAYER
    };
#endif

protected:
    CDlgSetupUI* m_parent;
    long m_popupSelectLayerIndex;
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    MaskListCtrl m_listLayer;

    virtual void OnOK()
    {
    }
    virtual void OnCancel()
    {
    }
    virtual BOOL OnInitDialog();

    afx_msg void OnLmAddlayer();
    afx_msg void OnLmDeletelayer();
    afx_msg void OnLmRenamelayer();
    afx_msg void OnLmEmptylayer();
    afx_msg LRESULT OnLayerItemRClick(WPARAM wParam, LPARAM);
    afx_msg LRESULT OnLayerItemSelected(WPARAM wParam, LPARAM);
    afx_msg LRESULT OnLayerItemMoved(WPARAM wParam, LPARAM lParam);

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLmExportalllayer();
    afx_msg void OnLmImportalllayer();
};
