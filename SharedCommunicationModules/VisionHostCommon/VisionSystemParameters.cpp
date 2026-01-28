//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionSystemParameters.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionSystemParam::VisionSystemParam()
{
    Init();
}

VisionSystemParam::~VisionSystemParam()
{
    Init();
}

void VisionSystemParam::Init()
{
    m_strFileName.Empty();
    m_strCategory.Empty();
    m_strKeyName.Empty();
    m_bIsGlobalParam = FALSE;
    m_strValue.Empty();
}

void VisionSystemParam::SetVisionSystemParam(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, long nValue)
{
    Init();
    m_strFileName = strFileName;
    m_strCategory = strCategory;
    m_strKeyName = strKeyName;
    m_bIsGlobalParam = bIsGlobalParam;
    m_strValue.Format(_T("%d"), nValue);
}

void VisionSystemParam::SetVisionSystemParam(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, float fValue)
{
    Init();
    m_strFileName = strFileName;
    m_strCategory = strCategory;
    m_strKeyName = strKeyName;
    m_bIsGlobalParam = bIsGlobalParam;
    m_strValue.Format(_T("%f"), fValue);
}

void VisionSystemParam::SetVisionSystemParam(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, double dValue)
{
    Init();
    m_strFileName = strFileName;
    m_strCategory = strCategory;
    m_strKeyName = strKeyName;
    m_bIsGlobalParam = bIsGlobalParam;
    m_strValue.Format(_T("%lf"), dValue);
}

void VisionSystemParam::SetVisionSystemParam(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, CString strValue)
{
    Init();
    m_strFileName = strFileName;
    m_strCategory = strCategory;
    m_strKeyName = strKeyName;
    m_bIsGlobalParam = bIsGlobalParam;
    m_strValue = strValue;
}

CArchive& operator<<(CArchive& ar, const VisionSystemParam& src)
{
    return ar << src.m_strFileName << src.m_strCategory << src.m_strKeyName << src.m_bIsGlobalParam << src.m_strValue;
}

CArchive& operator>>(CArchive& ar, VisionSystemParam& dst)
{
    return ar >> dst.m_strFileName >> dst.m_strCategory >> dst.m_strKeyName >> dst.m_bIsGlobalParam >> dst.m_strValue;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////   Spliter   ///////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

VisionSystemParameters::VisionSystemParameters()
{
    Init();
}

VisionSystemParameters::~VisionSystemParameters()
{
    Init();
}

void VisionSystemParameters::Init()
{
    m_vecVisionSysParam.clear();
}

void VisionSystemParameters::AddVisionSysParam(VisionSystemParam visionSysParam)
{
    m_vecVisionSysParam.push_back(visionSysParam);
}

void VisionSystemParameters::Add(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, long nValue)
{
    VisionSystemParam visionSysParam;
    visionSysParam.SetVisionSystemParam(strFileName, strCategory, strKeyName, bIsGlobalParam, nValue);
    AddVisionSysParam(visionSysParam);
}

void VisionSystemParameters::Add(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, float fValue)
{
    VisionSystemParam visionSysParam;
    visionSysParam.SetVisionSystemParam(strFileName, strCategory, strKeyName, bIsGlobalParam, fValue);
    AddVisionSysParam(visionSysParam);
}

void VisionSystemParameters::Add(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, double dValue)
{
    VisionSystemParam visionSysParam;
    visionSysParam.SetVisionSystemParam(strFileName, strCategory, strKeyName, bIsGlobalParam, dValue);
    AddVisionSysParam(visionSysParam);
}

void VisionSystemParameters::Add(
    CString strFileName, CString strCategory, CString strKeyName, bool bIsGlobalParam, CString strValue)
{
    VisionSystemParam visionSysParam;
    visionSysParam.SetVisionSystemParam(strFileName, strCategory, strKeyName, bIsGlobalParam, strValue);
    AddVisionSysParam(visionSysParam);
}

void VisionSystemParameters::GetKeyValue(
    CString strFileName, CString strCategory, CString strKeyName, bool& bIsGlobalParam, CString& strKeyValue)
{
    strKeyValue.Empty();
    for (auto& sysParam : m_vecVisionSysParam)
    {
        if (sysParam.m_strFileName == strFileName && sysParam.m_strCategory == strCategory
            && sysParam.m_strKeyName == strKeyName)
        {
            bIsGlobalParam = sysParam.m_bIsGlobalParam;
            strKeyValue = sysParam.m_strValue;
            return;
        }
    }
}

void VisionSystemParameters::SaveData(long nProbeID, CString strFileName)
{
    // 우선 아이템 수량을 써준다.
    CString strSection, strKey, strValue;
    strSection.Format(_T("VisionParameterCount"));
    strKey.Format(_T("Vision%d_ParameterCount"), nProbeID);
    strValue.Format(_T("%d"), m_vecVisionSysParam.size());

    ::WritePrivateProfileString(strSection, strKey, strValue, strFileName);

    int n = 0;
    for (auto& sysParam : m_vecVisionSysParam)
    {
        strSection.Format(_T("Vision%d_Parameter%d"), nProbeID, n);
        strKey.Format(_T("FileName"));
        ::WritePrivateProfileString(strSection, strKey, sysParam.m_strFileName, strFileName);
        strKey.Format(_T("Category"));
        ::WritePrivateProfileString(strSection, strKey, sysParam.m_strCategory, strFileName);
        strKey.Format(_T("KeyName"));
        ::WritePrivateProfileString(strSection, strKey, sysParam.m_strKeyName, strFileName);
        strKey.Format(_T("GlobalParam"));
        ::WritePrivateProfileString(strSection, strKey, sysParam.m_bIsGlobalParam ? _T("1") : _T("0"), strFileName);
        strKey.Format(_T("Value"));
        ::WritePrivateProfileString(strSection, strKey, sysParam.m_strValue, strFileName);
        n++;
    }
}

void VisionSystemParameters::LoadData(long nProbeID, CString strFileName)
{
    TCHAR szGet[1000];

    // 우선 아이템 수량을 써준다.
    CString strSection, strKey, strValue;
    strSection.Format(_T("VisionParameterCount"));
    strKey.Format(_T("Vision%d_ParameterCount"), nProbeID);

    ::GetPrivateProfileString(strSection, strKey, _T(""), szGet, 999, strFileName);
    strValue = CString(szGet);

    m_vecVisionSysParam.resize(::_ttoi(strValue));

    int n = 0;
    for (auto& sysParam : m_vecVisionSysParam)
    {
        strSection.Format(_T("Vision%d_Parameter%d"), nProbeID, n);
        strKey.Format(_T("FileName"));
        ::GetPrivateProfileString(strSection, strKey, _T(""), szGet, 999, strFileName);
        sysParam.m_strFileName = CString(szGet);
        strKey.Format(_T("Category"));
        ::GetPrivateProfileString(strSection, strKey, _T(""), szGet, 999, strFileName);
        sysParam.m_strCategory = CString(szGet);
        strKey.Format(_T("KeyName"));
        ::GetPrivateProfileString(strSection, strKey, _T(""), szGet, 999, strFileName);
        sysParam.m_strKeyName = CString(szGet);
        strKey.Format(_T("GlobalParam"));
        ::GetPrivateProfileString(strSection, strKey, _T(""), szGet, 999, strFileName);
        strValue = CString(szGet);
        sysParam.m_bIsGlobalParam = ::_ttoi(strValue);
        strKey.Format(_T("Value"));
        ::GetPrivateProfileString(strSection, strKey, _T(""), szGet, 999, strFileName);
        sysParam.m_strValue = CString(szGet);
        n++;
    }
}

CArchive& operator<<(CArchive& ar, const VisionSystemParameters& src)
{
    long nSize = (long)src.m_vecVisionSysParam.size();

    ar << nSize;
    for (auto& visionSysParam : src.m_vecVisionSysParam)
        ar << visionSysParam;

    return ar;
}

CArchive& operator>>(CArchive& ar, VisionSystemParameters& dst)
{
    long nSize = 0;
    ar >> nSize;
    dst.Init();
    dst.m_vecVisionSysParam.resize(nSize);
    for (auto& visionSysParam : dst.m_vecVisionSysParam)
        ar >> visionSysParam;

    return ar;
}
