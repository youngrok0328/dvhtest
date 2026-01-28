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
class VisionInspectionComponent2DPara;
struct sPassive_InfoDB;

//HDR_6_________________________________ Header body
//
class CDlgChangeSpecName : public CDialog
{
    DECLARE_DYNAMIC(CDlgChangeSpecName)

public:
    CDlgChangeSpecName(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgChangeSpecName();

    std::vector<CString> m_vecstrSpec;

    VisionInspectionComponent2DPara* m_pPara;
    sPassive_InfoDB* m_pCurPassiveInfo;

    void UpdateSpecList();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_CHANGE_SPEC_NAME
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DoModal(VisionInspectionComponent2DPara* pPara, sPassive_InfoDB* i_pCurPassiveInfo);

    CString m_strReName;
    CComboBox m_cmbSpec;

    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnCbnSelchangeComboSpec();
    afx_msg void OnBnClickedBnDel();
    afx_msg void OnBnClickedBnClear();
};
