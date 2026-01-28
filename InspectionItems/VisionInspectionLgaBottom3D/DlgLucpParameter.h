#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionInspectionLgaBottom3DPara;

//HDR_6_________________________________ Header body
//
class DlgLucpParameter : public CDialog
{
    DECLARE_DYNAMIC(DlgLucpParameter)

public:
    DlgLucpParameter(VisionInspectionLgaBottom3DPara* VisionPara, std::vector<CString>& group_id_lst, long GroupIndex,
        CWnd* pParent = nullptr);
    virtual ~DlgLucpParameter();

#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DLG_LUCP_PARAMETER
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

private:
    VisionInspectionLgaBottom3DPara* m_pVisionPara;
    CXTPPropertyGrid* m_propertyGrid;
    long m_nGroupIndex;
    std::vector<CString> m_group_id_lst;
    std::vector<BOOL> m_vecParameterInfo;

public:
    void UpdatePropertyGrid();
    BOOL MakeParameterInfo();
    BOOL MakeUseGroupInfo();

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
};
