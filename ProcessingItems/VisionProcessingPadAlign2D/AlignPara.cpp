//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "AlignPara.h"

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
AlignPara::AlignPara(VisionProcessing& parent)
    : m_imageProc(parent)
{
    Init();
}

AlignPara::~AlignPara()
{
}

void AlignPara::Init()
{
    m_imageProc.Init();

    m_edgeSearchOffset_um = 0.f;
    m_edgeSearchLength_um = 100.f;
    m_edgeSearchDirection = 1;
    m_edgeType = 0;
    m_edgeThreshold = 1.f;
    m_findFirstEdge = FALSE;
    m_edgeSearchCount = 10;
    m_edgeSearchWidth = 2;

    m_blobThreshold = 200;
    m_blobSearchOffset_um = 100.f;
}

BOOL AlignPara::LinkDataBase(BOOL save, CiDataBase& db)
{
    if (!save)
    {
        Init();
    }

    if (!m_imageProc.LinkDataBase(save, db[_T("image proc")]))
    {
        m_imageProc.Init();
    }

    if (!db[_T("{C10FE1E9-46C5-4940-B967-124EED6E84E5}")].Link(save, m_edgeSearchOffset_um))
        m_edgeSearchOffset_um = 0.f;
    if (!db[_T("edgeSearchOffset_um")].Link(save, m_edgeSearchLength_um))
        m_edgeSearchLength_um = 100.f;
    if (!db[_T("edgeSearchDirection")].Link(save, m_edgeSearchDirection))
        m_edgeSearchDirection = 1;
    if (!db[_T("edgeType")].Link(save, m_edgeType))
        m_edgeType = 0;
    if (!db[_T("edgeThreshold")].Link(save, m_edgeThreshold))
        m_edgeThreshold = 1.f;
    if (!db[_T("findFirstEdge")].Link(save, m_findFirstEdge))
        m_findFirstEdge = FALSE;
    if (!db[_T("edgeSearchCount")].Link(save, m_edgeSearchCount))
        m_edgeSearchCount = 10;
    if (!db[_T("edgeSearchWidth")].Link(save, m_edgeSearchWidth))
        m_edgeSearchWidth = 2;

    if (!db[_T("{C3338A2F-67A4-4C36-AC85-2C4ECD7D8F97}")].Link(save, m_blobThreshold))
        m_blobThreshold = 200;
    if (!db[_T("Teach Object")][_T("SearchOffset_um")].Link(save, m_blobSearchOffset_um))
        m_blobSearchOffset_um = 100.f;

    return TRUE;
}

AlignPara& AlignPara::operator=(const AlignPara& object)
{
    m_imageProc = object.m_imageProc;
    m_edgeSearchOffset_um = object.m_edgeSearchOffset_um;
    m_edgeSearchLength_um = object.m_edgeSearchLength_um;
    m_edgeSearchDirection = object.m_edgeSearchDirection;
    m_edgeType = object.m_edgeType;
    m_edgeThreshold = object.m_edgeThreshold;
    m_findFirstEdge = object.m_findFirstEdge;
    m_edgeSearchCount = object.m_edgeSearchCount;
    m_edgeSearchWidth = object.m_edgeSearchWidth;
    m_blobThreshold = object.m_blobThreshold;
    m_blobSearchOffset_um = object.m_blobSearchOffset_um;

    return *this;
}
