//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SpecLayerSingle.h"

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
SpecLayerSingle::SpecLayerSingle()
{
}

SpecLayerSingle::~SpecLayerSingle()
{
}

void SpecLayerSingle::Init()
{
    m_rois.clear();
}

BOOL SpecLayerSingle::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    long count = long(m_rois.size());

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    if (version == 0)
    {
        UserRoiType type = UserRoiType::Rect;
        long polygonPointCount = 3;
        if (!db[_T("type")].Link(bSave, (long&)type))
            type = UserRoiType::Rect;
        if (!db[_T("polygonPointCount")].Link(bSave, polygonPointCount))
            polygonPointCount = 3;

        if (!db[_T("count")].Link(bSave, count))
            count = 0;

        if (!bSave)
        {
            m_rois.resize(count);
        }

        for (long index = 0; index < count; index++)
        {
            auto& sub_db = db.GetSubDBFmt(_T("roi%d"), index + 1);
            m_rois[index].LinkDataBase(bSave, sub_db);
            m_rois[index].setType(type);
            m_rois[index].setPolygonPointCount(polygonPointCount);
        }
    }
    else
    {
        if (!db[_T("count")].Link(bSave, count))
            count = 0;

        if (!bSave)
        {
            m_rois.resize(count);
        }

        for (long index = 0; index < count; index++)
        {
            auto& sub_db = db.GetSubDBFmt(_T("roi%d"), index + 1);
            m_rois[index].LinkDataBase(bSave, sub_db);
        }
    }

    return TRUE;
}

const SpecRoi& SpecLayerSingle::operator[](long index) const
{
    return m_rois[index];
}

SpecRoi& SpecLayerSingle::operator[](long index)
{
    return m_rois[index];
}

long SpecLayerSingle::count() const
{
    return long(m_rois.size());
}

void SpecLayerSingle::add(UserRoiType type)
{
    long index = count();
    m_rois.resize(index + 1);
    m_rois[index].setType(type);
}

bool SpecLayerSingle::swap(long index1, long index2)
{
    if (index1 < 0 || index1 >= long(m_rois.size()))
        return false;
    if (index2 < 0 || index2 >= long(m_rois.size()))
        return false;

    auto temp = m_rois[index1];

    m_rois[index1] = m_rois[index2];
    m_rois[index2] = temp;

    return true;
}

void SpecLayerSingle::del(long index)
{
    m_rois.erase(m_rois.begin() + index);
}
