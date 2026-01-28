#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ParaDB.h"
#include "VisionProcessingPadAlign2DPara.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DlgAlgorithmDB : public CDialog
{
    DECLARE_DYNAMIC(DlgAlgorithmDB)

public:
    DlgAlgorithmDB(VisionProcessing& proc, VisionProcessingPadAlign2DPara& para, ParaDB& db,
        CWnd* pParent = NULL); // standard constructor
    virtual ~DlgAlgorithmDB();

    VisionProcessingPadAlign2DPara m_para;
    ParaDB m_db;

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_ALGORITHM_DB
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

    DECLARE_MESSAGE_MAP()

public:
    CString GetSelectJobName();
    CString GetSelectDBName();
    void UpdateJobList();
    void UpdateDBList();

    CListBox m_lstJob;
    CListBox m_lstDB;

    virtual BOOL OnInitDialog();
    afx_msg void OnLbnSelchangeListJob();
    afx_msg void OnLbnSelchangeListDb();
    afx_msg void OnBnClickedChipInfoExport();
    afx_msg void OnBnClickedChipDbImport();
    afx_msg void OnBnClickedBtnDelDb();
};
