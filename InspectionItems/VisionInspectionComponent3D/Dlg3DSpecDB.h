#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionComponent3DPara.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionComponent3D;
class VisionInspectionComponent3DPara;

//HDR_6_________________________________ Header body
//
class CDlg3DSpecDB : public CDialog
{
    DECLARE_DYNAMIC(CDlg3DSpecDB)

public:
    CDlg3DSpecDB(VisionInspectionComponent3D* pChipAlign, CWnd* pParent = NULL);
    virtual ~CDlg3DSpecDB();

    enum
    {
        IDD = IDD_DIALOG_3D_SPECDB
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnNMClickListChip(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedChipInfoExport();
    afx_msg void OnBnClickedChipDbImport();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnMenuDelete();

    VisionInspectionComponent3D* m_pChipVision;
    VisionInspectionComponent3DPara m_pChipAlignPara;

    CiDataBase m_ChipSpecDB;

    BOOL LoadChipSpecDB();
    BOOL SaveChipSpecDB();
    void ShowWidowList();

    long m_SpecID;
    long m_JobID;

    CListCtrl m_JobListChipInfo;
    CListCtrl m_SpecListChipInfo;
};