#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
enum class enDeviceType : long;
enum class enPackageType : long;

//HDR_6_________________________________ Header body
//
class CDeviceType
{
public:
    CDeviceType();
    virtual ~CDeviceType();

    BOOL DoModal();

    enDeviceType m_nDeviceType;
    enPackageType m_nPackageType;
    BOOL m_bDeadBug;
    CString m_strJobName;
};