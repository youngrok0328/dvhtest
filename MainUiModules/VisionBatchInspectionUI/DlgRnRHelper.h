#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CGridCtrl;
class VisionUnit;
class VisionInspectionResult;

//HDR_6_________________________________ Header body
//
class CDlgRnRHelper : public CDialog
{
    DECLARE_DYNAMIC(CDlgRnRHelper)

public:
    CDlgRnRHelper(VisionUnit& visionUnit, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgRnRHelper();

    VisionUnit& m_visionUnit;
    CGridCtrl* m_pgridSpec;

    std::vector<BOOL> m_vecbUserRnRData;
    long m_nSeletedPaneNum;
    long m_nRepeatCount;
    long m_nPackageCount;
    long m_nCurrentPackageCnt;

    std::vector<std::vector<std::vector<VisionInspectionResult>>> m_pvecvecvecResult;

    void SetResultData(long nRepeatNum, long nPackageNum, std::vector<VisionInspectionResult*>* pResult);

    long m_nCurrentResultCnt;

    // 함수

public:
    void SetInitDialog();
    void SetInitGrid();
    void UpdateSpecData();
    void SetInitRnRGrid();
    void RefreshRnRGrid();
    long m_nSelectResult;

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DLGRNRHELPER
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnBnClickedButtonStart();
    afx_msg void OnBnClickedButtonEnd();
    CGridCtrl* m_Grid_RnR;
    afx_msg void OnBnClickedButtonReport();
    afx_msg void OnGridEditEnd(NMHDR* pNotifyStruct, LRESULT* result);
};
