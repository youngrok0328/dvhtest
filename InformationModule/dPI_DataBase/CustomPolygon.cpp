//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CustomPolygon.h"

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
namespace Package
{
CustomPolygon::CustomPolygon()
    : m_strCustomPolygonName(_T(""))
    , m_eCustomPolygonCategory(CustomPolygonCategory::CustomPolygonCategory_Polygon)
    , m_nPointNum(-1)
    , m_fSpecHeight_mm(-1.f)
    , m_bIgnore(TRUE)
{
    m_vecfptPointInfo_mm.clear();
}

CustomPolygon::~CustomPolygon()
{
}

BOOL CustomPolygon::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{7AC976EE-08EA-4C40-AEB6-0BB767E587C8}")].Link(bSave, m_strCustomPolygonName))
        m_strCustomPolygonName = _T("");
    if (!db[_T("{7BDDB159-1514-4368-9390-118F1B9F026E}")].Link(bSave, (long&)m_eCustomPolygonCategory))
        m_eCustomPolygonCategory = CustomPolygonCategory::CustomPolygonCategory_Polygon;
    if (!db[_T("{7D0D1F68-9D79-40EC-A77E-F7B4DA24FD3F}")].Link(bSave, m_nPointNum))
        m_nPointNum = -1;
    if (!db[_T("{BA44D025-E9DA-4EB5-8553-3153510EF1E1}")].Link(bSave, m_fSpecHeight_mm))
        m_fSpecHeight_mm = -1.f;
    if (!db[_T("{78C76000-97BB-4F91-AA6D-41FE22EBAD76}")].Link(bSave, m_bIgnore))
        m_bIgnore = TRUE;

    if (!bSave)
    {
        m_vecfptPointInfo_mm.clear();
        m_vecfptPointInfo_mm.resize(m_nPointNum);
    }
    else
    {
        if (m_vecfptPointInfo_mm.size() != m_nPointNum)
            ::AfxMessageBox(_T("WTF.."));
    }

    for (long nPointInfoidx = 0; nPointInfoidx < m_nPointNum; nPointInfoidx++)
    {
        CString strPointInfoKeyValX(""), strPointInfoKeyValY("");
        strPointInfoKeyValX.Format(_T("CustomPolygonLayer_Point_P%dX(mm)"), nPointInfoidx + 1); //0이아닌 1부터 시작
        strPointInfoKeyValY.Format(_T("CustomPolygonLayer_Point_P%dY(mm)"), nPointInfoidx + 1); //0이아닌 1부터 시작
        if (!db[strPointInfoKeyValX].Link(bSave, m_vecfptPointInfo_mm[nPointInfoidx].m_x))
            m_vecfptPointInfo_mm[nPointInfoidx].m_x = -1.f;
        if (!db[strPointInfoKeyValY].Link(bSave, m_vecfptPointInfo_mm[nPointInfoidx].m_y))
            m_vecfptPointInfo_mm[nPointInfoidx].m_y = -1.f;
    }

    return TRUE;
}

const CustomPolygonCategory CustomPolygon::GetCatrgory() const
{
    return m_eCustomPolygonCategory;
}

const CString CustomPolygon::GetID() const
{
    return m_strCustomPolygonName;
}

} // namespace Package