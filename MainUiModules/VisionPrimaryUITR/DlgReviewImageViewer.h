#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CDlgImageVIewer;

//HDR_6_________________________________ Header body
//
class CDlgReviewImageViewer : public CDialog
{
    DECLARE_DYNAMIC(CDlgReviewImageViewer)

public:
    CDlgReviewImageViewer(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgReviewImageViewer();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_REVIEWIMAGE_VIEWER
    };

public:
    CDlgImageVIewer* m_pDlgImageViewer;

public:
    CTabCtrl m_ctrlTab;
    CRichEditCtrl m_reTextResult;
    CComboBox m_cmbDateList;
    CComboBox m_cmbJobNameList;
    CComboBox m_cmbLotIDList;
    CComboBox m_cmbFileNameList;

private:
    std::vector<CString> m_vecstrDate;
    std::vector<CString> m_vecstrJobName;
    std::vector<CString> m_vecstrLotID;
    std::vector<CString> m_vecstrFileName;
    std::vector<CString> m_vecstrFileName_Backup;

    long m_nSelectDate;
    long m_nSelectJobName;
    long m_nSelectLotID;
    long m_nSelectFileName;

    BOOL m_bInitCheck;

public:
    void Init(long nLevel);
    void SetupComboBox(long nLevel);
    BOOL VerifyVectorSize(long nLevel);

    void FileSearch(CString strFolderPath, long nLevel);
    void ImageOpen(CString strFilePath);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtnRefresh();
    afx_msg void OnCbnSelchangeCmbDate();
    afx_msg void OnCbnSelchangeCmbJobName();
    afx_msg void OnCbnSelchangeCmbLotid();
    afx_msg void OnCbnSelchangeCmbFileName();
    afx_msg void OnBnClickedBtnSearchFile();
    afx_msg void OnTcnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
};
