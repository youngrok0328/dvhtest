//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingFOV.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DEF_RAD_TO_DEG 57.295779513082 // = (180 / ITP_PI), radian to degree
#define DEF_DEG_TO_RAD 0.017453292519943 // = (ITP_PI / 180), degree to radian

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingFOV::OnInspection()
{
    BOOL bResult = DoInspection(true);

    return bResult;
}

BOOL VisionProcessingFOV::DoInspection(const bool detailSetupMode, const enSideVisionModule i_ModuleStatus)
{
    UNREFERENCED_PARAMETER(detailSetupMode);

    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bMarginal = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    ResetResult();

    return TRUE;
}
