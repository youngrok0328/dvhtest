#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
#include <afxdialogex.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class StitchPara;
class Property_Stitch;

//HDR_6_________________________________ Header body
//
class DlgBodySizeROIPara : public CDialogEx
{
    DECLARE_DYNAMIC(DlgBodySizeROIPara)

public:
    DlgBodySizeROIPara(StitchPara& para, Property_Stitch* engine, CWnd* pParent = nullptr); // standard constructor
    virtual ~DlgBodySizeROIPara();

    void UpdateList();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_BODYSIZE_ROI_PARA
    };
#endif

protected:
    StitchPara& m_para;
    Property_Stitch* m_engine;
    CGridCtrl m_grid;

    virtual void OnOK()
    {
    }
    virtual void OnCancel()
    {
    }
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
};
