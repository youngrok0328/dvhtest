//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ImageProcPara.h"

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
ImageProcPara::ImageProcPara(VisionProcessing& parent)
    : m_parent(parent)
    , m_aux1FrameID(parent, TRUE)
    , m_aux2FrameID(parent, TRUE)
{
    Init();
}

ImageProcPara::~ImageProcPara()
{
}

bool ImageProcPara::isCombine()
{
    if (nFirstCombineType != enumImageCombine_NotThing)
        return true;
    if (nSecondCombineType != enumImageCombine_NotThing)
        return true;

    return false;
}

void ImageProcPara::Init()
{
    m_aux1FrameID.setFrameIndex(0);
    m_aux2FrameID.setFrameIndex(0);
    nFirstCombineType = 0;
    nSecondCombineType = 0;

    vecnCurProc.clear();
    vecnCurProcParams.clear();

    vecnAux1Proc.clear();
    vecnAux1ProcParams.clear();

    vecnAux2Proc.clear();
    vecnAux2ProcParams.clear();

    vecnFirstCombineProc.clear();
    vecnFirstCombineProcParams.clear();

    vecnSecondCombineProc.clear();
    vecnSecondCombineProcParams.clear();
}

BOOL ImageProcPara::LinkDataBase(BOOL bSave, CiDataBase& procParaDB)
{
    int nSize;

    if (!m_aux1FrameID.LinkDataBase(bSave, procParaDB[_T("nAux1FrameID")]))
        m_aux1FrameID.setFrameIndex(0);
    if (!m_aux2FrameID.LinkDataBase(bSave, procParaDB[_T("nAux2FrameID")]))
        m_aux2FrameID.setFrameIndex(0);

    if (!procParaDB[_T("nFirstCombineType")].Link(bSave, nFirstCombineType))
        nFirstCombineType = 0;

    if (!procParaDB[_T("nSecondCombineType")].Link(bSave, nSecondCombineType))
        nSecondCombineType = 0;

    nSize = (long)vecnCurProc.size();
    if (!procParaDB[_T("vecnCurProcSize")].Link(bSave, nSize))
        nSize = 0;

    vecnCurProc.resize(nSize);
    vecnCurProcParams.resize(nSize);
    for (long nIdx = 0; nIdx < nSize; nIdx++)
    {
        CString str;
        str.Format(_T("vecnCurProc[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnCurProc[nIdx]))
            vecnCurProc[nIdx] = 0;

        str.Format(_T("vecnCurProcParams[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnCurProcParams[nIdx]))
            vecnCurProcParams[nIdx] = 0;

        str.Empty();
    }

    nSize = (long)vecnAux1Proc.size();
    if (!procParaDB[_T("vecnAux1ProcSize")].Link(bSave, nSize))
        nSize = 0;

    vecnAux1Proc.resize(nSize);
    vecnAux1ProcParams.resize(nSize);
    for (long nIdx = 0; nIdx < nSize; nIdx++)
    {
        CString str;
        str.Format(_T("vecnAux1Proc[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnAux1Proc[nIdx]))
            vecnAux1Proc[nIdx] = 0;

        str.Format(_T("vecnAux1ProcParams[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnAux1ProcParams[nIdx]))
            vecnAux1ProcParams[nIdx] = 0;

        str.Empty();
    }

    nSize = (long)vecnAux2Proc.size();
    if (!procParaDB[_T("vecnAux2ProcSize")].Link(bSave, nSize))
        nSize = 0;

    vecnAux2Proc.resize(nSize);
    vecnAux2ProcParams.resize(nSize);
    for (long nIdx = 0; nIdx < nSize; nIdx++)
    {
        CString str;
        str.Format(_T("vecnAux2Proc[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnAux2Proc[nIdx]))
            vecnAux2Proc[nIdx] = 0;

        str.Format(_T("vecnAux2ProcParams[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnAux2ProcParams[nIdx]))
            vecnAux2ProcParams[nIdx] = 0;

        str.Empty();
    }

    nSize = (long)vecnFirstCombineProc.size();
    if (!procParaDB[_T("vecnFirstCombineProcSize")].Link(bSave, nSize))
        nSize = 0;

    vecnFirstCombineProc.resize(nSize);
    vecnFirstCombineProcParams.resize(nSize);
    for (long nIdx = 0; nIdx < nSize; nIdx++)
    {
        CString str;
        str.Format(_T("vecnFirstCombineProc[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnFirstCombineProc[nIdx]))
            vecnFirstCombineProc[nIdx] = 0;

        str.Format(_T("vecnFirstCombineProcParams[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnFirstCombineProcParams[nIdx]))
            vecnFirstCombineProcParams[nIdx] = 0;

        str.Empty();
    }

    nSize = (long)vecnSecondCombineProc.size();
    if (!procParaDB[_T("vecnSecondCombineProcSize")].Link(bSave, nSize))
        nSize = 0;

    vecnSecondCombineProc.resize(nSize);
    vecnSecondCombineProcParams.resize(nSize);
    for (long nIdx = 0; nIdx < nSize; nIdx++)
    {
        CString str;
        str.Format(_T("vecnSecondCombineProc[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnSecondCombineProc[nIdx]))
            vecnSecondCombineProc[nIdx] = 0;

        str.Format(_T("vecnSecondCombineProcParams[%d]"), nIdx);
        if (!procParaDB[str].Link(bSave, vecnSecondCombineProcParams[nIdx]))
            vecnSecondCombineProcParams[nIdx] = 0;

        str.Empty();
    }

    return TRUE;
}

ImageProcPara& ImageProcPara::operator=(const ImageProcPara& srcObj)
{
    m_aux1FrameID.setFrameIndex(srcObj.m_aux1FrameID.getFrameIndex());
    m_aux2FrameID.setFrameIndex(srcObj.m_aux2FrameID.getFrameIndex());
    nFirstCombineType = srcObj.nFirstCombineType;
    nSecondCombineType = srcObj.nSecondCombineType;

    vecnCurProc = srcObj.vecnCurProc;
    vecnCurProcParams = srcObj.vecnCurProcParams;

    vecnAux1Proc = srcObj.vecnAux1Proc;
    vecnAux1ProcParams = srcObj.vecnAux1ProcParams;

    vecnAux2Proc = srcObj.vecnAux2Proc;
    vecnAux2ProcParams = srcObj.vecnAux2ProcParams;

    vecnFirstCombineProc = srcObj.vecnFirstCombineProc;
    vecnFirstCombineProcParams = srcObj.vecnFirstCombineProcParams;

    vecnSecondCombineProc = srcObj.vecnSecondCombineProc;
    vecnSecondCombineProcParams = srcObj.vecnSecondCombineProcParams;

    return *this;
}

std::vector<CString> ImageProcPara::ExportImageCombineParaToText(
    const CString strVisionName, const CString strModuleName, const CString strCategory) //kircheis_TxtRecipe
{
    CString strHeader;
    CString strGroup;
    CString strParaNameAux;
    CString strValue;

    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strModuleName);

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);
    CString strAlgorithmParameter;

    //First Combine Start/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strGroup.Format(_T("First Combine"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Image Combine Mode"), _T(""), nFirstCombineType));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Combine Aux1 Frame"), _T(""), m_aux1FrameID.getFrameIndex()));

    long nProcessType(0);

    //1st Combine Cur Frame Algo
    long nProcessNum = (long)vecnCurProc.size();
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Current Frame"), _T("Algorithm Num"), nProcessNum));

    strParaNameAux.Format(_T("Apply List"));
    for (long nProc = 0; nProc < nProcessNum; nProc++)
    {
        nProcessType = vecnCurProc[nProc];
        if (nProcessType == 9 /*enumImageProc_Offset*/ || nProcessType == 10 /*enumImageProc_Contrast*/)
            strValue.Format(_T("[%d]%d"), nProcessType, vecnCurProcParams[nProc]);
        else
            strValue.Format(_T("%d"), nProcessType);

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Current Frame"), strParaNameAux, strValue));
    }

    //1st Combine Aux Frame Algo
    nProcessNum = (long)vecnAux1Proc.size();
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Aux 1 Frame"), _T("Algorithm Num"), nProcessNum));

    strParaNameAux.Format(_T("Apply List"));
    for (long nProc = 0; nProc < nProcessNum; nProc++)
    {
        nProcessType = vecnAux1Proc[nProc];
        if (nProcessType == 9 /*enumImageProc_Offset*/ || nProcessType == 10 /*enumImageProc_Contrast*/)
            strValue.Format(_T("[%d]%d"), nProcessType, vecnAux1ProcParams[nProc]);
        else
            strValue.Format(_T("%d"), nProcessType);

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Aux 1 Frame"), strParaNameAux, strValue));
    }

    //1st Combine Result Frame Algo
    nProcessNum = (long)vecnFirstCombineProc.size();
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Result Frame"), _T("Algorithm Num"), nProcessNum));

    strParaNameAux.Format(_T("Apply List"));
    for (long nProc = 0; nProc < nProcessNum; nProc++)
    {
        nProcessType = vecnFirstCombineProc[nProc];
        if (nProcessType == 9 /*enumImageProc_Offset*/ || nProcessType == 10 /*enumImageProc_Contrast*/)
            strValue.Format(_T("[%d]%d"), nProcessType, vecnFirstCombineProcParams[nProc]);
        else
            strValue.Format(_T("%d"), nProcessType);

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Result Frame"), strParaNameAux, strValue));
    }
    //First Combine End/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    //Second Combine Start/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    strGroup.Format(_T("Second Combine"));

    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Image Combine Mode"), _T(""), nSecondCombineType));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Combine Aux2 Frame"), _T(""), m_aux2FrameID.getFrameIndex()));

    //Second Combine Aux Frame Algo
    nProcessNum = (long)vecnAux2Proc.size();
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Aux 2 Frame"), _T("Algorithm Num"), nProcessNum));

    strParaNameAux.Format(_T("Apply List"));
    for (long nProc = 0; nProc < nProcessNum; nProc++)
    {
        nProcessType = vecnAux2Proc[nProc];
        if (nProcessType == 9 /*enumImageProc_Offset*/ || nProcessType == 10 /*enumImageProc_Contrast*/)
            strValue.Format(_T("[%d]%d"), nProcessType, vecnAux2ProcParams[nProc]);
        else
            strValue.Format(_T("%d"), nProcessType);

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Aux 2 Frame"), strParaNameAux, strValue));
    }

    //Second Combine Result Frame Algo
    nProcessNum = (long)vecnSecondCombineProc.size();
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Result Frame"), _T("Algorithm Num"), nProcessNum));

    strParaNameAux.Format(_T("Apply List"));
    for (long nProc = 0; nProc < nProcessNum; nProc++)
    {
        nProcessType = vecnSecondCombineProc[nProc];
        if (nProcessType == 9 /*enumImageProc_Offset*/ || nProcessType == 10 /*enumImageProc_Contrast*/)
            strValue.Format(_T("[%d]%d"), nProcessType, vecnSecondCombineProcParams[nProc]);
        else
            strValue.Format(_T("%d"), nProcessType);

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Result Frame"), strParaNameAux, strValue));
    }
    //Second Combine End/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    return vecstrAlgorithmParameters;
}
