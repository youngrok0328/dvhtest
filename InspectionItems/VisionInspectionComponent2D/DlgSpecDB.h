#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionComponent2DPara.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionComponent2D;
class VisionInspectionComponent2DPara;

//HDR_6_________________________________ Header body
//
class CDlgSpecDB : public CDialog
{
    DECLARE_DYNAMIC(CDlgSpecDB)

public:
    CDlgSpecDB(VisionInspectionComponent2D* pChipAlign, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgSpecDB();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_SPECDB
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnBnClickedChipInfoExport();
    afx_msg void OnBnClickedChipDbImport();
    virtual BOOL OnInitDialog();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnNMClickListChip(NMHDR* pNMHDR, LRESULT* pResult);

    VisionInspectionComponent2D* m_pChipVision;
    VisionInspectionComponent2DPara m_ChipAlgoDBPara;

    CListCtrl m_JobListChipInfo;
    CListCtrl m_SpecListChipInfo;

    CiDataBase m_ChipSpecDB;

    BOOL LoadChipSpecDB();
    BOOL SaveChipSpecDB();
    void ShowWidowList();

    long m_SpecID;
    long m_JobID;
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedBtnDelJob();
    afx_msg void OnBnClickedBtnDelDb();
};
