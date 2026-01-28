//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SystemConfigForTR.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../SharedComponent/Persistence/IniHelper.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define NGRV_SAVE_IMAGE_FORMAT 1

//CPP_7_________________________________ Implementation body
//

void SystemConfigForTR::SetPrevJob(int32_t mainType, CString strPrevJob)
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);
    m_strPrevJob[mainType] = strPrevJob;

    switch (mainType)
    {
        case TR_EXECUTE_IN_POCKET:
            IniHelper::SaveSTRING(systemFilePath, _T("TR - In Pocket"), _T("Prev Job Name"), m_strPrevJob[mainType]);
            break;

        case TR_EXECUTE_OTI:
            IniHelper::SaveSTRING(systemFilePath, _T("TR - OTI"), _T("Prev Job Name"), m_strPrevJob[mainType]);
            break;
    }
}

CString SystemConfigForTR::GetPrevJob(int32_t mainType)
{
    CString systemFilePath = DynamicSystemPath::get(DefineFile::System);

    switch (mainType)
    {
        case TR_EXECUTE_IN_POCKET:
            m_strPrevJob[mainType]
                = IniHelper::LoadSTRING(systemFilePath, _T("TR - In Pocket"), _T("Prev Job Name"), _T(""));
            break;
        case TR_EXECUTE_OTI:
            m_strPrevJob[mainType] = IniHelper::LoadSTRING(systemFilePath, _T("TR - OTI"), _T("Prev Job Name"), _T(""));
            break;
    }

    return m_strPrevJob[mainType];
}

void SystemConfigForTR::LoadIni(CString strIniPath)
{
    m_executeMode = IniHelper::LoadINT(strIniPath, _T("TR"), _T("Execute Mode"), -1);
}

void SystemConfigForTR::SaveIni(CString strIniPath)
{
    IniHelper::SaveINT(strIniPath, _T("TR"), _T("Execute Mode"), m_executeMode);
}
