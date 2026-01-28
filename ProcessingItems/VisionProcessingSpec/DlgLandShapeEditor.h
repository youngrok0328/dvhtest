#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class VisionProcessingSpec;

//HDR_6_________________________________ Header body
//
class DlgLandShapeEditor : public CDialog
{
    DECLARE_DYNAMIC(DlgLandShapeEditor)

public:
    DlgLandShapeEditor(VisionProcessingSpec* pVisionInsp, CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~DlgLandShapeEditor();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DlgLandShapeEditor
    };
#endif

public:
    VisionProcessingSpec* m_VisionInsp;
    CPackageSpec& m_PackageSpec;
    std::vector<CString> m_group_id_list;
    std::vector<long> m_vecnGroupShapeType;

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    CComboBox m_cbShpaeType;
    CComboBox m_cbGroupInfo;
    afx_msg void OnBnClickedOk();
    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelchangeGroupInfo();
};
