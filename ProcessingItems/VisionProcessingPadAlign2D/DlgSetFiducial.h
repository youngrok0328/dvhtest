#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ParaDB;
class VisionProcessingPadAlign2DPara;

//HDR_6_________________________________ Header body
//
class DlgSetFiducial : public CDialog
{
    DECLARE_DYNAMIC(DlgSetFiducial)

public:
    DlgSetFiducial(ParaDB* paraDB, VisionProcessingPadAlign2DPara* para, CWnd* pParent = NULL); // standard constructor
    virtual ~DlgSetFiducial();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_SET_FIDUCIAL
    };
#endif

protected:
    ParaDB* m_paraDB;
    VisionProcessingPadAlign2DPara* m_para;

    void UpdateNameList();
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CString m_algorithm_name;
    CComboBox m_cmbAlgorithm;
    CComboBox m_cmbCategory;
    int m_category;

    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelchangeComboCategory();
};
