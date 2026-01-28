#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionInspectionBgaBottom3DPara;

//HDR_6_________________________________ Header body
//
class CDlgBucpParameter : public CDialog
{
    DECLARE_DYNAMIC(CDlgBucpParameter)

public:
    CDlgBucpParameter(VisionInspectionBgaBottom3DPara* VisionPara, std::vector<CString>& group_id_lst, long GroupIndex,
        CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~CDlgBucpParameter();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DLG_BUCP_PARAMETER
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

private:
    VisionInspectionBgaBottom3DPara* m_pVisionPara;
    CXTPPropertyGrid* m_propertyGrid;
    long m_nGroupIndex;
    std::vector<CString> m_group_id_lst;
    std::vector<BOOL> m_vecParameterInfo;

public:
    void UpdatePropertyGrid();
    BOOL MakeParameterInfo();
    BOOL MakeUseGroupInfo();

public:
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    virtual BOOL OnInitDialog();
};
