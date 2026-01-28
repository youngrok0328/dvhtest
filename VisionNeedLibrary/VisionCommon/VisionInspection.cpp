//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspection.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionDefaultSpec::VisionInspectionDefaultSpec(LPCTSTR specGuid, LPCTSTR specName, LPCTSTR shortName,
    LPCTSTR sortingName, LPCTSTR unit, HostReportCategory hostReportCategory, II_RESULT_TYPE resultType, BOOL bUseIn,
    float fMarginalMinIn, float fPassMinIn, float fPassMaxIn, float fMarginalMaxIn, BOOL bUseMinIn, BOOL bUseMaxIn,
    BOOL bUseMarginalIn)
    : m_specGuid(specGuid)
    , m_specName(specName)
    , m_shortName(shortName)
    , m_sortingName(sortingName)
    , m_unit(unit)
    , m_hostReportCategory(hostReportCategory)
    , m_resultType(resultType)
    , m_use(bUseIn)
    , m_marginalMin(fMarginalMinIn)
    , m_passMin(fPassMinIn)
    , m_passMax(fPassMaxIn)
    , m_marginalMax(fMarginalMaxIn)
    , m_useMin(bUseMinIn)
    , m_useMax(bUseMaxIn)
    , m_useMarginal(bUseMarginalIn)
{
}

VisionInspection::VisionInspection(
    LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec)
    : VisionProcessing(moduleGuid, moduleName, visionUnit, packageSpec)
{
}

VisionInspection::~VisionInspection()
{
}

VisionInspectionSpec* VisionInspection::GetSpecByGuid(LPCTSTR guid)
{
    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_specGuid == guid)
        {
            return &spec;
        }
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        if (spec.m_specGuid == guid)
        {
            return &spec;
        }
    }

    return nullptr;
}

VisionInspectionSpec* VisionInspection::GetSpecByName(LPCTSTR name)
{
    for (auto& spec : m_fixedInspectionSpecs)
    {
        if (spec.m_specName == name)
        {
            return &spec;
        }
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        if (spec.m_specName == name)
        {
            return &spec;
        }
    }

    return nullptr;
}

void VisionInspection::SetSpec(LPCTSTR specName, BOOL bUse, BOOL bUseMax, BOOL bUseMin, BOOL bUseMarginal,
    float fMarginalMax, float fPassMax, float fPassMin, float fMarginalMin)
{
    const long FixInsepctionSpecNum = (long)m_fixedInspectionSpecs.size();
    const long VariableInsepctionSpecNum = (long)m_variableInspectionSpecs.size();

    for (long i = 0; i < FixInsepctionSpecNum; i++)
    {
        if (m_fixedInspectionSpecs[i].m_specName == specName)
        {
            m_fixedInspectionSpecs[i].m_use = bUse;
            m_fixedInspectionSpecs[i].m_useMax = bUseMax;
            m_fixedInspectionSpecs[i].m_useMin = bUseMin;
            m_fixedInspectionSpecs[i].m_useMarginal = bUseMarginal;
            m_fixedInspectionSpecs[i].m_marginalMax = fMarginalMax;
            m_fixedInspectionSpecs[i].m_passMax = fPassMax;
            m_fixedInspectionSpecs[i].m_passMin = fPassMin;
            m_fixedInspectionSpecs[i].m_marginalMin = fMarginalMin;
        }
    }

    for (long i = 0; i < VariableInsepctionSpecNum; i++)
    {
        if (m_variableInspectionSpecs[i].m_specName == specName)
        {
            m_variableInspectionSpecs[i].m_use = bUse;
            m_variableInspectionSpecs[i].m_useMax = bUseMax;
            m_variableInspectionSpecs[i].m_useMin = bUseMin;
            m_variableInspectionSpecs[i].m_useMarginal = bUseMarginal;
            m_variableInspectionSpecs[i].m_marginalMax = fMarginalMax;
            m_variableInspectionSpecs[i].m_passMax = fPassMax;
            m_variableInspectionSpecs[i].m_passMin = fPassMin;
            m_variableInspectionSpecs[i].m_marginalMin = fMarginalMin;
        }
    }
}

void VisionInspection::ResetSpec()
{
    m_fixedInspectionSpecs.clear();
    for (auto& fixedSpec : m_defaultFixedInspectionSpecs)
    {
        m_fixedInspectionSpecs.emplace_back(m_moduleGuid, fixedSpec.m_specGuid, m_strModuleName, fixedSpec.m_specName,
            fixedSpec.m_shortName, fixedSpec.m_sortingName, fixedSpec.m_unit, fixedSpec.m_hostReportCategory,
            fixedSpec.m_resultType, fixedSpec.m_use, fixedSpec.m_marginalMin, fixedSpec.m_passMin, fixedSpec.m_passMax,
            fixedSpec.m_marginalMax, fixedSpec.m_useMin, fixedSpec.m_useMax, fixedSpec.m_useMarginal);
    }

    m_variableInspectionSpecs.clear();

    m_resultGroup.Set(m_fixedInspectionSpecs);
}

void VisionInspection::ResetResult()
{
    m_resultGroup.Set(m_fixedInspectionSpecs);
    m_resultGroup.Add(m_variableInspectionSpecs);
}

BOOL VisionInspection::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!__super::LinkDataBase(bSave, db))
    {
        return FALSE;
    }

    for (auto& spec : m_fixedInspectionSpecs)
    {
        spec.Link(bSave, db[spec.m_specGuid]);

        for (auto& fixedSpec : m_defaultFixedInspectionSpecs)
        {
            if (spec.m_specGuid == fixedSpec.m_specGuid)
            {
                spec.m_inspName = m_strModuleName;
                spec.m_specName = fixedSpec.m_specName;
                spec.m_shortName = fixedSpec.m_shortName;
                spec.m_sortingName = fixedSpec.m_sortingName;
                spec.MakeSpecFullName();
                break;
            }
        }
    }

    long variableInspectionSpecCount = long(m_variableInspectionSpecs.size());

    db[_T("VariableInspectionSpecCount")].Link(bSave, variableInspectionSpecCount);

    for (long specIndex = 0; specIndex < variableInspectionSpecCount; specIndex++)
    {
        CString strGuid;
        if (bSave)
            strGuid = m_variableInspectionSpecs[specIndex].m_specGuid;
        db.GetSubDBFmt(_T("VariableInspectionSpecGUID_%d"), specIndex).Link(bSave, strGuid);

        if (strGuid.IsEmpty())
        {
            continue;
        }

        if (!bSave)
        {
            m_variableInspectionSpecs.emplace_back(m_moduleGuid, strGuid);
        }

        strGuid.Empty();
    }

    for (auto& spec : m_variableInspectionSpecs)
    {
        spec.Link(bSave, db[spec.m_specGuid]);
    }

    if (!bSave)
    {
        ResetResult();
    }

    return TRUE;
}

std::vector<CString> VisionInspection::ExportInspectionSpecToText(CString strVisionName) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    const static CString strCategory = _T("Inspection Spec");
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)m_strModuleName);

    //개별 Parameter용 변수
    CString strGroup;

    //Output Parameter 생성
    std::vector<CString> vecstrInspectionSpec(0);
    CString strInspectionSpec;

    long nFixedNum = (long)m_fixedInspectionSpecs.size();

    for (long nInsp = 0; nInsp < nFixedNum; nInsp++)
    {
        auto& inspSpec = m_fixedInspectionSpecs[nInsp];
        strGroup = inspSpec.m_specName;

        //Use
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T(""), (bool)inspSpec.m_use));
        //Use Min
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T("Min."), (bool)inspSpec.m_useMin));
        //Use Max
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T("Max."), (bool)inspSpec.m_useMax));
        //Use Marginal
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T("Margin"), (bool)inspSpec.m_useMarginal));

        //Min. Margin
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Min."), _T("Margin"), inspSpec.m_marginalMin));
        //Min. Pass
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Min."), _T("Pass"), inspSpec.m_passMin));

        //Max. Pass
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Max."), _T("Pass"), inspSpec.m_passMax));
        //Max. Margin
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Max."), _T("Margin"), inspSpec.m_marginalMax));
    }

    long nVariableNum = (long)m_variableInspectionSpecs.size();

    for (long nInsp = 0; nInsp < nVariableNum; nInsp++)
    {
        auto& inspSpec = m_variableInspectionSpecs[nInsp];
        strGroup = inspSpec.m_specName;

        //Use
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T(""), (bool)inspSpec.m_use));
        //Use Min
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T("Min."), (bool)inspSpec.m_useMin));
        //Use Max
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T("Max."), (bool)inspSpec.m_useMax));
        //Use Marginal
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Use"), _T("Margin"), (bool)inspSpec.m_useMarginal));

        //Min. Margin
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Min."), _T("Margin"), inspSpec.m_marginalMin));
        //Min. Pass
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Min."), _T("Pass"), inspSpec.m_passMin));

        //Max. Pass
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Max."), _T("Pass"), inspSpec.m_passMax));
        //Max. Margin
        vecstrInspectionSpec.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Max."), _T("Margin"), inspSpec.m_marginalMax));
    }

    return vecstrInspectionSpec;
}

void VisionInspection::AppendDetailTextResult(CString& io_text, long specObjectCount)
{
    long nFixedInspNum = (long)m_fixedInspectionSpecs.size();
    long nVariableInspNum = (long)m_variableInspectionSpecs.size();
    long nInspNum = nFixedInspNum + nVariableInspNum;
    long nResultNum = (long)m_resultGroup.m_vecResult.size();
    if (nInspNum != nResultNum)
    {
        ASSERT(!(_T("Check the inspection item's number")));
        return;
    }

    std::vector<VisionInspectionResult*> vecpInspResult;
    std::vector<VisionInspectionSpec*> vecpInspSpec;
    std::vector<BOOL> vecbUseInsp;
    std::vector<CString> vecstrInspShortName;
    long nObjectNum = 0;

    for (long nID = 0; nID < nInspNum; nID++)
    {
        VisionInspectionResult* pInspResult;
        if (nID < nFixedInspNum)
            pInspResult = m_resultGroup.GetResultByName(m_fixedInspectionSpecs[nID].m_specName);
        else
            pInspResult = m_resultGroup.GetResultByName(m_variableInspectionSpecs[nID - nFixedInspNum].m_specName);
        if (pInspResult == nullptr)
        {
            ASSERT(!(_T("Check the inspection item's number")));
            return;
        }
        VisionInspectionSpec* pInspSpec = GetSpecByName(pInspResult->m_resultName);
        if (pInspSpec == nullptr)
        {
            ASSERT(!(_T("Check the inspection item's number")));
            return;
        }

        if ((specObjectCount >= 0 && pInspResult->m_objectResults.size() != specObjectCount) || !pInspSpec->m_use)
        {
            continue;
        }

        vecpInspResult.push_back(pInspResult);
        vecpInspSpec.push_back(pInspSpec);
        vecbUseInsp.push_back(pInspSpec->m_use);
        vecstrInspShortName.push_back(pInspSpec->m_shortName);

        nObjectNum = (long)max(pInspResult->m_objectResults.size(), nObjectNum);
    }

    long nItemNum = (long)vecpInspResult.size();
    if (nItemNum == 0)
        return;

    //mc_Mark예외처리 Test
    BOOL bMarkException(FALSE); //Mark는 Report를 다르게 쓴다
    CString strModuleName;
    strModuleName.Format(_T("%s"), (LPCTSTR)vecpInspResult[0]->m_inspName);
    if (strModuleName.Find(_T("Mark_")) > -1) //Inspection 항목이 Mark라면..
    {
        CString strMarkPositionGuid = _T("{EB3BEBFF-80E1-4593-A4D1-86FF84ED70AD}");
        CString strMarkAngleGuid = _T("{B1C1846C-6B20-4217-AAF0-24BA55315203}");

        //예외항목 판단(Mark Count검사항목만 존재할 수 있으니)
        for (auto InspectionType : vecpInspResult)
        {
            //하나라도 있다면 그만검색
            if (InspectionType->m_specGuid == strMarkPositionGuid || InspectionType->m_specGuid == strMarkAngleGuid)
            {
                bMarkException = TRUE;
                break;
            }
        }

        strMarkPositionGuid.Empty();
        strMarkAngleGuid.Empty();
    }

    if (bMarkException)
    {
        AppendDetailMarkTextResult(vecpInspResult, vecpInspSpec, nObjectNum, io_text);
    }
    else
    {
        io_text += _T("------------");
        for (long nID = 0; nID < nItemNum; nID++)
            io_text += _T("------------");
        io_text += _T("------------------------------------\r\n");

        io_text += _T("\tID ");
        for (long nID = 0; nID < nItemNum; nID++)
        {
            io_text.AppendFormat(_T("\t%s"), (LPCTSTR)vecpInspSpec[nID]->m_shortName);
        }

        io_text += _T("\tObject ROI [ L,T,R,B ]\r\n");

        io_text += _T("------------");
        for (long nID = 0; nID < nItemNum; nID++)
            io_text += _T("------------");
        io_text += _T("------------------------------------\r\n");

        long nRefID = -1;
        for (long nObj = 0; nObj < vecpInspResult.size(); nObj++)
        {
            if (vecpInspResult[nObj]->m_objectRects.size() == nObjectNum)
            {
                nRefID = nObj;
                break;
            }
        }

        if (nRefID >= 0)
        {
            for (long nObj = 0; nObj < nObjectNum; nObj++)
            {
                auto rtObj = vecpInspResult[nRefID]->m_objectRects[nObj];

                io_text.AppendFormat(_T("\t%s"), (LPCTSTR)vecpInspResult[nRefID]->m_objectNames[nObj]);

                for (long nID = 0; nID < nItemNum; nID++)
                {
                    if (vecpInspResult[nID]->m_objectErrorValues.size() > nObj)
                        io_text.AppendFormat(_T("\t%.2f"), vecpInspResult[nID]->m_objectErrorValues[nObj]);
                }
                io_text.AppendFormat(
                    _T("\t[ %d, %d, %d, %d ]\r\n"), rtObj.m_left, rtObj.m_top, rtObj.m_right, rtObj.m_bottom);
            }
        }

        io_text += _T("------------");
        for (long nID = 0; nID < nItemNum; nID++)
            io_text += _T("------------");
        io_text += _T("------------------------------------\r\n");
    }

    for (int nIdx = 0; nIdx < vecstrInspShortName.size(); nIdx++)
    {
        vecstrInspShortName[nIdx].Empty();
    }

    strModuleName.Empty();
}

void VisionInspection::AppendDetailMarkTextResult(std::vector<VisionInspectionResult*> i_vecpInspResult,
    std::vector<VisionInspectionSpec*> i_vecpInspSpec, long i_nObjectNum, CString& io_text)
{
    CString strMarkPositionGuid = _T("{EB3BEBFF-80E1-4593-A4D1-86FF84ED70AD}");
    CString strMarkAngleGuid = _T("{B1C1846C-6B20-4217-AAF0-24BA55315203}");
    long nExceptionInspResultNum(0);
    for (auto ExceptionInspResult : i_vecpInspResult)
    {
        if (ExceptionInspResult->m_specGuid == strMarkPositionGuid
            || ExceptionInspResult->m_specGuid == strMarkAngleGuid)
            nExceptionInspResultNum++;
    }

    io_text += _T("------------");
    for (long nID = 0; nID < nExceptionInspResultNum; nID++)
        io_text += _T("------------");
    io_text += _T("------------------------------------\r\n");

    io_text += _T("\tID ");
    for (long nID = 0; nID < nExceptionInspResultNum; nID++)
    {
        io_text.AppendFormat(_T("\t%s"), (LPCTSTR)i_vecpInspSpec[nID]->m_shortName);
    }

    io_text += _T("\tObject ROI [ L,T,R,B ]\r\n");

    io_text += _T("------------");
    for (long nID = 0; nID < nExceptionInspResultNum; nID++)
        io_text += _T("------------");
    io_text += _T("------------------------------------\r\n");

    //Mark Pos와 Angle의 값을 우선으로 기록한다. Rect의 Data의 갯수는 1개
    auto rtObj = i_vecpInspResult[0]->m_objectRects[0];

    io_text.AppendFormat(_T("\t%s"), (LPCTSTR)i_vecpInspResult[0]->m_objectNames[0]);

    for (long nID = 0; nID < nExceptionInspResultNum; nID++)
        io_text.AppendFormat(_T("\t%.2f"), i_vecpInspResult[nID]->m_objectErrorValues[0]);

    io_text.AppendFormat(_T("\t[ %d, %d, %d, %d ]\r\n"), rtObj.m_left, rtObj.m_top, rtObj.m_right, rtObj.m_bottom);

    io_text += _T("------------");
    for (long nID = 0; nID < nExceptionInspResultNum; nID++)
        io_text += _T("------------");
    io_text += _T("------------------------------------\r\n");

    if (i_vecpInspResult.size() <= nExceptionInspResultNum) //Mark Pos & Angle 만 검사 할 때 죽는 문제 수정
        return;

    //이후에 그외의 항목들을 다시 쓴다
    long nItemNum = (long)i_vecpInspResult.size();

    io_text += _T("------------");
    for (long nID = 0; nID < nItemNum; nID++)
        io_text += _T("------------");
    io_text += _T("------------------------------------\r\n");

    io_text += _T("\tID ");
    for (long nID = nExceptionInspResultNum; nID < nItemNum; nID++)
    {
        io_text.AppendFormat(_T("\t%s"), (LPCTSTR)i_vecpInspSpec[nID]->m_shortName);
    }

    io_text += _T("\tObject ROI [ L,T,R,B ]\r\n");

    io_text += _T("------------");
    for (long nID = 0; nID < nItemNum; nID++)
        io_text += _T("------------");
    io_text += _T("------------------------------------\r\n");

    for (long nObj = 0; nObj < i_nObjectNum; nObj++)
    {
        //예외처리한 이후의 검사항목부터 시작해야한다
        rtObj = i_vecpInspResult[nExceptionInspResultNum]->m_objectRects[nObj];

        io_text.AppendFormat(_T("\t%s"), (LPCTSTR)i_vecpInspResult[nExceptionInspResultNum]->m_objectNames[nObj]);

        for (long nID = nExceptionInspResultNum; nID < nItemNum; nID++)
        {
            io_text.AppendFormat(_T("\t%.2f"), i_vecpInspResult[nID]->m_objectErrorValues[nObj]);
        }
        io_text.AppendFormat(_T("\t[ %d, %d, %d, %d ]\r\n"), rtObj.m_left, rtObj.m_top, rtObj.m_right, rtObj.m_bottom);
    }

    io_text += _T("------------");
    for (long nID = 0; nID < nItemNum; nID++)
        io_text += _T("------------");
    io_text += _T("------------------------------------\r\n");

    strMarkPositionGuid.Empty();
    strMarkAngleGuid.Empty();
}