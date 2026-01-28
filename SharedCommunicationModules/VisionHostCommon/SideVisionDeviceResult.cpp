//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SideVisionDeviceResult.h"

//CPP_2_________________________________ This project's headers
#include "ArchiveAllType.h"
#include "VisionDeviceResult.h"
#include "VisionHostBaseDef.h"
#include "VisionInspectionResult.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SideVisionDeviceResult::SideVisionDeviceResult()
{
    m_vecVisionDeviceResult.resize(Common_SIDE_VISIONMODULE_END);
    m_vecnSideEachModuleResult.resize(Common_SIDE_VISIONMODULE_END);
    m_vec2nSideEachModuleInspectionResult.resize(Common_SIDE_VISIONMODULE_END);

    Init();
}

SideVisionDeviceResult::~SideVisionDeviceResult()
{
#if defined(USE_BY_HANDLER)

#endif
    m_vecVisionDeviceResult.clear();
    m_vecnSideEachModuleResult.clear();
    for (long nIdx = Common_SIDE_VISIONMODULE_START; nIdx < Common_SIDE_VISIONMODULE_END; nIdx++)
    {
        m_vec2nSideEachModuleInspectionResult[nIdx].clear();
    }
    m_vec2nSideEachModuleInspectionResult.clear();
}

void SideVisionDeviceResult::Init()
{
    m_nThreadID = -1;
    m_nProbeID = -1;
    m_nTrayID = 0;
    m_nInspectionRepeatIndex = 0; //mc_KMAT Repeat Crash
    m_nPocketID = 0;
    m_nPane = 0;
    m_nTotalResult = NOT_MEASURED;

    if (m_vecVisionDeviceResult.size() == Common_SIDE_VISIONMODULE_END)
    {
        for (long nIdx = Common_SIDE_VISIONMODULE_START; nIdx < Common_SIDE_VISIONMODULE_END; nIdx++)
        {
            m_vecnSideEachModuleResult[nIdx] = NOT_MEASURED;
            m_vecVisionDeviceResult[nIdx].Init();
            m_vec2nSideEachModuleInspectionResult[nIdx].clear();
        }
    }
}

void SideVisionDeviceResult::Serialize(ArchiveAllType& ar)
{
    long nDateVersion = 20240219;

    ar.Serialize_Element(nDateVersion);

    if (nDateVersion != 20240219)
    {
        ASSERT(!_T("Fuck the DeviceResult!"));
    }
    else
    {
        ar.Serialize_Element(m_nProbeID);
        ar.Serialize_Element(m_nThreadID);
        ar.Serialize_Element(m_nTrayID);
        ar.Serialize_Element(m_nInspectionRepeatIndex);
        ar.Serialize_Element(m_nPocketID);
        ar.Serialize_Element(m_nScanID);
        ar.Serialize_Element(m_nPane);
        ar.Serialize_Element(m_nTotalResult);

        if (m_vecnSideEachModuleResult.size() == Common_SIDE_VISIONMODULE_END)
        {
            for (long nIdx = Common_SIDE_VISIONMODULE_START; nIdx < Common_SIDE_VISIONMODULE_END; nIdx++)
                ar.Serialize_Element(m_vecnSideEachModuleResult[nIdx]);
        }
        if (m_vecVisionDeviceResult.size() == Common_SIDE_VISIONMODULE_END)
        {
            for (long nIdx = Common_SIDE_VISIONMODULE_START; nIdx < Common_SIDE_VISIONMODULE_END; nIdx++)
            {
                m_vecVisionDeviceResult[nIdx].Serialize(ar);
            }
        }

        long nEachInspectionNum = 0;
        if (ar.IsStoring() == TRUE)
        {
            for (long nIdx = Common_SIDE_VISIONMODULE_START; nIdx < Common_SIDE_VISIONMODULE_END; nIdx++)
            {
                nEachInspectionNum = (long)m_vec2nSideEachModuleInspectionResult[nIdx].size();
                ar << nEachInspectionNum;
                for (long nInsp = 0; nInsp < nEachInspectionNum; nInsp++)
                    ar << m_vec2nSideEachModuleInspectionResult[nIdx][nInsp];
            }
        }
        else
        {
            for (long nIdx = Common_SIDE_VISIONMODULE_START; nIdx < Common_SIDE_VISIONMODULE_END; nIdx++)
            {
                m_vec2nSideEachModuleInspectionResult[nIdx].clear();
                ar >> nEachInspectionNum;
                m_vec2nSideEachModuleInspectionResult[nIdx].resize(nEachInspectionNum);
                for (long nInsp = 0; nInsp < nEachInspectionNum; nInsp++)
                    ar >> m_vec2nSideEachModuleInspectionResult[nIdx][nInsp];
            }
        }
    }
}

BOOL SideVisionDeviceResult::UpdateCommonInfo()
{
    if (m_vecVisionDeviceResult.size() != Common_SIDE_VISIONMODULE_END)
        return FALSE;

    long nEachInspectionNum = 0;
    BOOL bExistNotMeasured = FALSE;
    BOOL bUpdateCommonID = FALSE;

    for (long nIdx = Common_SIDE_VISIONMODULE_START; nIdx < Common_SIDE_VISIONMODULE_END; nIdx++)
    {
        m_vec2nSideEachModuleInspectionResult[nIdx].clear();

        if (m_vecVisionDeviceResult[nIdx].m_nTotalResult > NOT_MEASURED)
        {
            if (bUpdateCommonID == FALSE)
            {
                m_nThreadID = m_vecVisionDeviceResult[nIdx].m_nThreadID;
                m_nProbeID = m_vecVisionDeviceResult[nIdx].m_nProbeID;
                m_nTrayID = m_vecVisionDeviceResult[nIdx].m_nTrayID;
                m_nInspectionRepeatIndex
                    = m_vecVisionDeviceResult[nIdx].m_nInspectionRepeatIndex; //mc_KMAT Repeat Crash
                m_nPocketID = m_vecVisionDeviceResult[nIdx].m_nPocketID;
                m_nScanID = m_vecVisionDeviceResult[nIdx].m_nScanID;
                m_nPane = m_vecVisionDeviceResult[nIdx].m_nPane;
                bUpdateCommonID = TRUE;
            }
            m_vecnSideEachModuleResult[nIdx] = m_vecVisionDeviceResult[nIdx].m_nTotalResult;
            m_nTotalResult = (long)max(m_nTotalResult, m_vecVisionDeviceResult[nIdx].m_nTotalResult);

            nEachInspectionNum = (long)m_vecVisionDeviceResult[nIdx].m_vecVisionInspResult.size();
            m_vec2nSideEachModuleInspectionResult[nIdx].resize(nEachInspectionNum);
            for (long nInsp = 0; nInsp < nEachInspectionNum; nInsp++)
                m_vec2nSideEachModuleInspectionResult[nIdx][nInsp]
                    = m_vecVisionDeviceResult[nIdx].m_vecVisionInspResult[nInsp]->m_totalResult;
        }
        else
            bExistNotMeasured = TRUE;
    }

    return (bExistNotMeasured == FALSE);
}
