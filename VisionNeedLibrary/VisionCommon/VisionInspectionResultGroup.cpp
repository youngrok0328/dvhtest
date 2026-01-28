//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionResultGroup.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
void VisionInspectionResultGroup::Set(const std::vector<VisionInspectionSpec>& visionInspectionSpecs)
{
    m_vecResult.clear();

    for (const auto& spec : visionInspectionSpecs)
    {
        m_vecResult.emplace_back(spec.m_moduleGuid, spec.m_specGuid, spec.m_inspName, spec.m_specName, spec.m_specName,
            spec.m_sortingName, spec.m_unit, spec.m_resultType, spec.m_hostReportCategory);
    }
}

void VisionInspectionResultGroup::Add(const std::vector<VisionInspectionSpec>& visionInspectionSpecs)
{
    for (const auto& spec : visionInspectionSpecs)
    {
        m_vecResult.emplace_back(spec.m_moduleGuid, spec.m_specGuid, spec.m_inspName, spec.m_specName, spec.m_specName,
            spec.m_sortingName, spec.m_unit, spec.m_resultType, spec.m_hostReportCategory);
    }
}

VisionInspectionResult* VisionInspectionResultGroup::GetResultByGuid(LPCTSTR guid)
{
    for (auto& result : m_vecResult)
    {
        if (result.m_specGuid == guid)
        {
            return &result;
        }
    }

    return nullptr;
}

VisionInspectionResult* VisionInspectionResultGroup::GetResultByName(LPCTSTR name)
{
    for (auto& result : m_vecResult)
    {
        if (result.m_resultName == name)
        {
            return &result;
        }
    }
    return nullptr;
}

VisionInspectionResult* VisionInspectionResultGroup::GetResultBySpec(const VisionInspectionSpec& spec)
{
    for (auto& result : m_vecResult)
    {
        if (result.m_specGuid == spec.m_specGuid)
        {
            return &result;
        }
    }
    return nullptr;
}
