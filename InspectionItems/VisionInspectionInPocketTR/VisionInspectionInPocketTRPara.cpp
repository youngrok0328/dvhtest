//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionInPocketTRPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionInPocketTRPara::VisionInspectionInPocketTRPara(VisionProcessing& parent)
    : m_ImageProcManageParaMisPlace(parent)
    , m_calcFrameIndexMisPlace(parent, TRUE)
    , m_ImageProcManageParaOrientation(parent)
    , m_calcFrameIndexOrientation(parent, TRUE)
{
    Init();
}

VisionInspectionInPocketTRPara::~VisionInspectionInPocketTRPara(void)
{
}

BOOL VisionInspectionInPocketTRPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    UNREFERENCED_PARAMETER(db);

    if (!bSave)
    {
        Init();
    }

    return TRUE;
}

void VisionInspectionInPocketTRPara::Init()
{
}

std::vector<CString> VisionInspectionInPocketTRPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Module 특성 변수 //채울 필요 없음. 알고리즘 파라미터가 없음

    return vecstrAlgorithmParameters;
}