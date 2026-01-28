//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Para_MaskOrder.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/Config.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
Para_MaskOrder::Para_MaskOrder()
{
}

Para_MaskOrder::~Para_MaskOrder()
{
}

void Para_MaskOrder::Init()
{
    m_maskInfos.clear();
}

bool Para_MaskOrder::IsValid(LPCTSTR maskName) const
{
    for (auto mask : m_maskInfos)
    {
        if (mask.m_strName == maskName)
        {
            return true;
        }
    }

    return false;
}

long Para_MaskOrder::FindIndex(LPCTSTR maskName) const
{
    for (long index = 0; index < long(m_maskInfos.size()); index++)
    {
        if (m_maskInfos[index].m_strName == maskName)
        {
            return index;
        }
    }

    return -1;
}

BOOL Para_MaskOrder::LinkDataBase(BOOL save, CiDataBase& db)
{
    long version = 2;
    long orderCount = (long)m_maskInfos.size();

    if (!db[_T("Version")].Link(save, version))
        version = 0;
    if (!db[_T("order Count")].Link(save, orderCount))
        orderCount = 0;

    if (!save)
    {
        m_maskInfos.resize(orderCount);
    }

    for (int index = 0; index < orderCount; index++)
    {
        auto& info = m_maskInfos[index];
        auto& sub_db = db.GetSubDBFmt(_T("order%d"), index);

        if (!sub_db[_T("Validate")].Link(save, info.m_validate))
            info.m_validate = FALSE;
        if (!sub_db[_T("MaskType")].Link(save, info.m_makeType))
            info.m_makeType = 0;
        if (!sub_db[_T("Name")].Link(save, info.m_strName))
            info.m_strName = _T("");
        if (!sub_db[_T("UseAutoThreshDR")].Link(save, info.m_useAutoThreshDR))
            info.m_useAutoThreshDR = FALSE;
        if (!sub_db[_T("ThreshDRX")].Link(save, info.m_threshDRX))
            info.m_threshDRX = 0;
        if (!sub_db[_T("ThreshDRY")].Link(save, info.m_threshDRX))
            info.m_threshDRY = 0;
        if (!sub_db[_T("UseBodyEdge")].Link(save, info.m_useBodyEdge))
            info.m_useBodyEdge = FALSE;

        long pointOffsetCount = (long)info.m_pointOffsets_um.size();
        if (!sub_db[_T("Point OffsetCount")].Link(save, pointOffsetCount))
            pointOffsetCount = 0;

        if (!save)
        {
            info.m_pointOffsets_um.resize(pointOffsetCount);
        }

        if (version < 2)
        {
            // Version2이후로 다음의 이름이 강제로 바뀌었다.
            if (info.m_strName == _T("LAND_Layer1"))
                info.m_strName = _T("Measured - Land Group1");
            if (info.m_strName == _T("LAND_Layer2"))
                info.m_strName = _T("Measured - Land Group2");
        }

        if (info.m_strName == _T("Component Origin_Mask Image"))
            info.m_strName = _T("Measured - Component with Pad");
        if (info.m_strName == _T("Component Mask Image"))
            info.m_strName = _T("Measured - Component");

        if (version < 1)
        {
            // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
            return FALSE;
        }
        else
        {
            if (!sub_db[_T("Dilate")].Link(save, info.m_dilateInUm))
                info.m_dilateInUm = 0.f;
            if (!sub_db[_T("OffsetX")].Link(save, info.m_offset_um.m_x))
                info.m_offset_um.m_x = 0.f;
            if (!sub_db[_T("OffsetY")].Link(save, info.m_offset_um.m_y))
                info.m_offset_um.m_y = 0.f;

            for (int pointIndex = 0; pointIndex < pointOffsetCount; pointIndex++)
            {
                auto& offset_dbX = sub_db.GetSubDBFmt(_T("PointOffsetX%d"), pointIndex);
                auto& offset_dbY = sub_db.GetSubDBFmt(_T("PointOffsetY%d"), pointIndex);

                if (!offset_dbX.Link(save, info.m_pointOffsets_um[pointIndex].m_x))
                {
                    info.m_pointOffsets_um[pointIndex].m_x = 0.f;
                }

                if (!offset_dbY.Link(save, info.m_pointOffsets_um[pointIndex].m_y))
                {
                    info.m_pointOffsets_um[pointIndex].m_y = 0.f;
                }
            }
        }
    }

    return TRUE;
}

void Para_MaskOrder::Add(const MaskInfo& maskInfo)
{
    m_maskInfos.push_back(maskInfo);
}

bool Para_MaskOrder::Del(long orderIndex)
{
    if (orderIndex < 0 || orderIndex >= long(m_maskInfos.size()))
    {
        return false;
    }

    m_maskInfos.erase(m_maskInfos.begin() + orderIndex);

    return true;
}

const Para_MaskOrder::MaskInfo& Para_MaskOrder::GetAt(long orderIndex) const
{
    return m_maskInfos[orderIndex];
}

Para_MaskOrder::MaskInfo& Para_MaskOrder::GetAt(long orderIndex)
{
    return m_maskInfos[orderIndex];
}

long Para_MaskOrder::GetCount() const
{
    return long(m_maskInfos.size());
}
