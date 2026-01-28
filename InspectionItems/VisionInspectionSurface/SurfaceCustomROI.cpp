//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceCustomROI.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceCustomROIItem.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CSurfaceCustomROI::CSurfaceCustomROI()
    : m_pMapMaskShape(new std::map<CString, SSurfaceCustomROIItem>)
    , m_mapMaskShape(*m_pMapMaskShape)
{
}

CSurfaceCustomROI::~CSurfaceCustomROI()
{
    delete m_pMapMaskShape;
}

void CSurfaceCustomROI::PrepareCustomROI()
{
    CustomROI_DeleteAll();
}

void CSurfaceCustomROI::CustomROI_Delete(LPCTSTR name)
{
    m_mapMaskShape.erase(name);
}

void CSurfaceCustomROI::CustomROI_DeleteAll()
{
    m_mapMaskShape.clear();
}

int CSurfaceCustomROI::CustomROI_GetCount() const
{
    return (long)m_mapMaskShape.size();
}

SSurfaceCustomROIItem* CSurfaceCustomROI::CustomROI_Search(LPCTSTR name)
{
    auto it = m_mapMaskShape.find(name);
    if (it == m_mapMaskShape.end())
        return nullptr;

    return &it->second;
}

const SSurfaceCustomROIItem* CSurfaceCustomROI::CustomROI_Search(LPCTSTR name) const
{
    auto it = m_mapMaskShape.find(name);
    if (it == m_mapMaskShape.end())
        return nullptr;

    return &it->second;
}

CString CSurfaceCustomROI::CustomROI_GetName(int nID) const
{
    if (nID >= CustomROI_GetCount())
    {
        return CString("");
    }

    std::map<CString, SSurfaceCustomROIItem>::const_iterator itr = m_mapMaskShape.begin();

    for (int n = 0; n < nID; n++)
        itr++;

    return itr->first;
}

BOOL CSurfaceCustomROI::CustomROI_AddRect(LPCTSTR name, Ipvm::Rect32s* pRectArray, long nShapeNum)
{
    SSurfaceCustomROIItem& CustomROI = m_mapMaskShape[name];
    CustomROI.nType = 0;
    CustomROI.strGroupName = name;
    CustomROI.Resize(nShapeNum);

    for (long i = 0; i < nShapeNum; i++)
    {
        CustomROI.SetRect(i, pRectArray[i]);
    }

    return TRUE;
}

BOOL CSurfaceCustomROI::CustomROI_AddEllipse(LPCTSTR name, Ipvm::Rect32s* pRectArray, long nShapeNum)
{
    SSurfaceCustomROIItem& CustomROI = m_mapMaskShape[name];
    CustomROI.nType = 1;
    CustomROI.strGroupName = name;
    CustomROI.Resize(nShapeNum);

    for (long i = 0; i < nShapeNum; i++)
    {
        CustomROI.SetRect(i, pRectArray[i]);
    }

    return TRUE;
}

BOOL CSurfaceCustomROI::CustomROI_SetImage(LPCTSTR name, const Ipvm::Image8u& image)
{
    SSurfaceCustomROIItem& CustomROI = m_mapMaskShape[name];
    CustomROI.nType = 2;
    CustomROI.SetImage(image);

    return TRUE;
}

BOOL CSurfaceCustomROI::CustomROI_AddPolygon(LPCTSTR name, Ipvm::Point32s2* pPolygonArray, long nShapeNum, long nType)
{
    //=============================================
    // 3점 이상을 가지는 Polygon_32f 만 세팅할 수 있다
    //
    // 인자로 넘어오는 pPolygonArray는
    //  nShapeNum * nPtNum 만한 크기를 가진다
    //=============================================
    ASSERT(nType >= 3);
    if (nType < 3)
        return FALSE;

    SSurfaceCustomROIItem& CustomROI = m_mapMaskShape[name];

    CustomROI.nType = nType;
    CustomROI.strGroupName = name;
    CustomROI.Resize(nShapeNum);

    for (long i = 0; i < nShapeNum; i++)
    {
        for (int nPT = 0; nPT < nType; nPT++)
        {
            CustomROI.vecShape[i].vecptPos[nPT] = pPolygonArray[i * nType + nPT];
        }
    }

    return TRUE;
}

BOOL CSurfaceCustomROI::CustomROI_AddPolygon(
    LPCTSTR name, std::vector<std::vector<Ipvm::Point32s2>> vecPolygonArray, long nShapeNum, long nType)
{
    //=============================================
    // 3점 이상을 가지는 Polygon_32f 만 세팅할 수 있다
    //
    // 인자로 넘어오는 pPolygonArray는
    //  nShapeNum * nPtNum 만한 크기를 가진다
    //=============================================
    ASSERT(nType >= 3);
    if (nType < 3)
        return FALSE;

    SSurfaceCustomROIItem& CustomROI = m_mapMaskShape[name];

    CustomROI.nType = nType;
    CustomROI.strGroupName = name;
    CustomROI.Resize(nShapeNum);

    for (long i = 0; i < nShapeNum; i++)
    {
        for (int nPT = 0; nPT < nType; nPT++)
        {
            CustomROI.vecShape[i].vecptPos[nPT] = vecPolygonArray[i][nPT];
        }
    }

    return TRUE;
}

void CSurfaceCustomROI::Init()
{
    m_mapMaskShape.clear();
}