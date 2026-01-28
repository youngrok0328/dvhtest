//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingSpec.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BOOL VisionProcessingSpec::OnInspection()
{
    return TRUE;
}
BOOL VisionProcessingSpec::DoInspection(const bool /*detailSetupMode*/, const enSideVisionModule i_ModuleStatus)
{
    //mc_Side Vision
    SetCurVisionModule_Status(i_ModuleStatus);
    //

    //{{ //kircheis_3DEmpty //검사 결과를 관리하는 민감한 변수니까 무조건 시작할 때 초기화 한다.
    m_bInvalid = FALSE;
    m_bEmpty = FALSE;
    m_bDoubleDevice = FALSE;
    //}}

    // 영훈 20141223_BodySize_Error : Body Size가 입력되지 않는 상태에서 다음 검사 진행 중 BodySize 참조 시 예상하지 못한 문제가 발생할 수 있으므로 예외처리 추가
    // bk LayoutEditor에 진입하지 않고 들어오면 m_pPackageSpec에는 쓰레기가 들어있어 수정.
    //if(m_packageSpec.m_bodyInfoMaster->fBodySizeX <= 0.f || m_packageSpec.m_bodyInfoMaster->fBodySizeY <= 0.f)
    if (m_packageSpec.m_bodyInfoMaster->fBodySizeX <= 0.f || m_packageSpec.m_bodyInfoMaster->fBodySizeY <= 0.f)
    {
        return FALSE;
    }

    return TRUE;
}
