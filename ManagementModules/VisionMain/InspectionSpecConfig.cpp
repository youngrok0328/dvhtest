//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "InspectionSpecConfig.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../SharedComponent/Persistence/IniHelper.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CInspectionSpecConfig::CInspectionSpecConfig(void)
{
}

CInspectionSpecConfig::~CInspectionSpecConfig(void)
{
}

void CInspectionSpecConfig::SaveIni(std::vector<VisionInspectionSpec> vecSpecData)
{
    CString strIniPath = DynamicSystemPath::get(DefineFolder::System) + _T("InspectionSpec.ini");

    CString strModuleName;
    CString strSpecName;

    long nSize = (long)vecSpecData.size();
    for (long n = 0; n < nSize; n++)
    {
        strModuleName.Format(_T("%s"), (LPCTSTR)vecSpecData[n].m_inspName);

        if (strModuleName.Find(_T("Mark_")) > -1)
        {
            strModuleName = _T("Mark");
        }

        strSpecName.Format(_T("%s_Use"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveINT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_use);

        strSpecName.Format(_T("%s_UseMin"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveINT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_useMin);

        strSpecName.Format(_T("%s_UseMax"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveINT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_useMax);

        strSpecName.Format(_T("%s_UseMarginal"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveINT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_useMarginal);

        strSpecName.Format(_T("%s_MarginalMinValue"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveFLOAT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_marginalMin);

        strSpecName.Format(_T("%s_PassMinValue"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveFLOAT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_passMin);

        strSpecName.Format(_T("%s_PassMaxValue"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveFLOAT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_passMax);

        strSpecName.Format(_T("%s_MarginalMaxValue"), (LPCTSTR)vecSpecData[n].m_specName);
        IniHelper::SaveFLOAT(strIniPath, strModuleName, strSpecName, vecSpecData[n].m_marginalMax);
    }

    strIniPath.Empty();
    strModuleName.Empty();
    strSpecName.Empty();
}

void CInspectionSpecConfig::LoadIni(std::vector<VisionInspectionSpec>& vecSpecData)
{
    CString strIniPath = DynamicSystemPath::get(DefineFolder::System) + _T("InspectionSpec.ini");
    CString strModuleName;
    CString strSpecName;

    long nSize = (long)vecSpecData.size();
    for (long n = 0; n < nSize; n++)
    {
        strModuleName.Format(_T("%s"), (LPCTSTR)vecSpecData[n].m_inspName);

        if (strModuleName.Find(_T("Mark_")) > -1)
        {
            strModuleName = _T("Mark");
        }

        strSpecName.Format(_T("%s_Use"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_use = IniHelper::LoadINT(strIniPath, strModuleName, strSpecName, FALSE);

        strSpecName.Format(_T("%s_UseMin"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_useMin = IniHelper::LoadINT(strIniPath, strModuleName, strSpecName, FALSE);

        strSpecName.Format(_T("%s_UseMax"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_useMax = IniHelper::LoadINT(strIniPath, strModuleName, strSpecName, FALSE);

        strSpecName.Format(_T("%s_UseMarginal"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_useMarginal = IniHelper::LoadINT(strIniPath, strModuleName, strSpecName, FALSE);

        strSpecName.Format(_T("%s_MarginalMinValue"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_marginalMin = IniHelper::LoadFLOAT(strIniPath, strModuleName, strSpecName, 0.f);

        strSpecName.Format(_T("%s_PassMinValue"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_passMin = IniHelper::LoadFLOAT(strIniPath, strModuleName, strSpecName, 0.f);

        strSpecName.Format(_T("%s_PassMaxValue"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_passMax = IniHelper::LoadFLOAT(strIniPath, strModuleName, strSpecName, 0.f);

        strSpecName.Format(_T("%s_MarginalMaxValue"), (LPCTSTR)vecSpecData[n].m_specName);
        vecSpecData[n].m_marginalMax = IniHelper::LoadFLOAT(strIniPath, strModuleName, strSpecName, 0.f);
    }

    strIniPath.Empty();
    strModuleName.Empty();
    strSpecName.Empty();
}