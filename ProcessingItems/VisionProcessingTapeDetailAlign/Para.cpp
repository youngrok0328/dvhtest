//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Para.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Para::Para(VisionProcessing& parent)
    : m_sprocketHoleAlign(parent)
    , m_pocketAlign(parent)
    , m_deviceAlign(parent)
{
    Init();
}

Para::~Para()
{
}

BOOL Para::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        Init();
    }

    m_sprocketHoleAlign.LinkDataBase(bSave, db[_T("SprocketHole")]);
    m_pocketAlign.LinkDataBase(bSave, db[_T("Pocket")]);
    m_deviceAlign.LinkDataBase(bSave, db[_T("Device")]);

    return TRUE;
}

void Para::Init()
{
    m_sprocketHoleAlign.Init();
    m_pocketAlign.Init();
    m_deviceAlign.Init();
}

std::vector<CString> Para::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    return vecstrAlgorithmParameters;
}
