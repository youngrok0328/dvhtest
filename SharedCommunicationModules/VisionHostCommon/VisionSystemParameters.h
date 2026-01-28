#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ VisionSystemParam
{
public:
    VisionSystemParam();
    ~VisionSystemParam();

    void Init();

    void SetVisionSystemParam(
        CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, long nValue);
    void SetVisionSystemParam(
        CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, float fValue);
    void SetVisionSystemParam(
        CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, double dValue);
    void SetVisionSystemParam(
        CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, CString strValue);

    CString m_strFileName;
    CString m_strCategory;
    CString m_strKeyName;
    bool m_bIsGlobalParam;
    CString m_strValue;

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const VisionSystemParam& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, VisionSystemParam& dst);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////   Spliter   ///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class __VISION_HOST_COMMON_API__ VisionSystemParameters
{
public:
    VisionSystemParameters();
    ~VisionSystemParameters();

    void Init();
    void AddVisionSysParam(VisionSystemParam visionSysParam);
    void Add(CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, long nValue);
    void Add(CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, float fValue);
    void Add(CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, double dValue);
    void Add(CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, CString strValue);

    void GetKeyValue(
        CString strFileName, CString strCategory, CString strKeyName, bool& bIsGlobalParam, CString& strKeyValue);

    std::vector<VisionSystemParam> m_vecVisionSysParam;
    void SaveData(long nProbeID, CString strFileName);
    void LoadData(long nProbeID, CString strFileName);

private:
    friend __VISION_HOST_COMMON_API__ CArchive& operator<<(CArchive& ar, const VisionSystemParameters& src);
    friend __VISION_HOST_COMMON_API__ CArchive& operator>>(CArchive& ar, VisionSystemParameters& dst);
};
