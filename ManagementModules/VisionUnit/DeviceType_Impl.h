#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
enum class enDeviceType : long;
enum class enPackageType : long;

//HDR_6_________________________________ Header body
//
class CDeviceType_Impl : public CDialog
{
    DECLARE_DYNAMIC(CDeviceType_Impl)

public:
    CDeviceType_Impl(CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDeviceType_Impl();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_DEVICE_TYPE
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    enDeviceType m_nDeviceType;
    enPackageType m_nPackageType;

    CString m_strJobName;

    BOOL m_bDeadBug;

    enDeviceType GetDeviceType();
    enPackageType GetPackageType();
    BOOL GetDeadBug();
    CString GetJobName();

    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    int m_nRadioPackageType;
};
