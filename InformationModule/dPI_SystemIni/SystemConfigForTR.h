#pragma once
//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DPI_SYSTEMINISLITDEF_API SystemConfigForTR
{
public:
    int32_t m_executeMode = TR_EXECUTE_ALL;

    void SetPrevJob(int32_t mainType, CString strPrevJob);
    CString GetPrevJob(int32_t mainType);

    void LoadIni(CString strIniPath);
    void SaveIni(CString strIniPath);

private:
    CString m_strPrevJob[2];
};
