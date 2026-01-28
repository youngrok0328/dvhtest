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
class VisionInspectionComponent3DPara;
struct sPassive_InfoDB;

//HDR_6_________________________________ Header body
//
class CDlgChangeSpecName3D : public CDialog
{
    DECLARE_DYNAMIC(CDlgChangeSpecName3D)

public:
    CDlgChangeSpecName3D(CWnd* pParent = NULL);
    virtual ~CDlgChangeSpecName3D();

    std::vector<CString> m_vecstrSpec;

    VisionInspectionComponent3DPara* m_pPara;
    sPassive_InfoDB* m_pCurPassiveInfo;

    void UpdateSpecList();

    enum
    {
        IDD = IDD_DIALOG_CHANGE_SPEC
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual INT_PTR DoModal(VisionInspectionComponent3DPara* pPara, sPassive_InfoDB* i_pCurPassiveInfo);

    CComboBox m_cmbSpec;
    CString m_strReName;

    afx_msg void OnBnClickedButtonDel();
    afx_msg void OnBnClickedButtonClear();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnCbnSelchangeComboChangeSpec();
};
