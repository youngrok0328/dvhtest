//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceItem.h"

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
CSurfaceItem::CSurfaceItem()
    : strName(_T("Unknown Item"))
    , bInsp(FALSE)
    , b2ndInsp(FALSE)
    , str2ndCode(_T(""))
    , vecCriteria(1)
    , m_bSurfaceMaskMode(FALSE)
    , m_bSurfaceMask_GenerateAlways(TRUE)
{
}

CSurfaceItem::CSurfaceItem(const CSurfaceItem& Src)
    : strName(Src.strName)
    , bInsp(Src.bInsp)
    , vecCriteria(Src.vecCriteria)
    , AlgoPara(Src.AlgoPara)
    , m_decisionArea(Src.m_decisionArea)
{
}

CSurfaceItem& CSurfaceItem::operator=(const CSurfaceItem& Src)
{
    strName = Src.strName;
    bInsp = Src.bInsp;

    vecCriteria = Src.vecCriteria;

    AlgoPara = Src.AlgoPara;

    m_decisionArea = Src.m_decisionArea;

    return *this;
}

void CSurfaceItem::Init()
{
    strName = _T("Unknown Item");
    bInsp = FALSE;
    b2ndInsp = FALSE;
    str2ndCode = _T("");

    vecCriteria.resize(1);
    vecCriteria.front().Init();

    AlgoPara.Init();
    m_decisionArea.Init();
}

BOOL CSurfaceItem::IsNeed_Spec(VisionSurfaceCriteria_Column column) const
{
    if (!bInsp)
        return FALSE;

    std::vector<CSurfaceCriteria>::const_iterator itr = vecCriteria.begin();
    std::vector<CSurfaceCriteria>::const_iterator itrEnd = vecCriteria.end();

    while (itr != itrEnd)
    {
        if (itr->bInsp)
        {
            if (itr->GetSpecMin(column).GetInsp() || itr->GetSpecMax(column).GetInsp())
            {
                return TRUE;
            }
        }

        itr++;
    }

    return FALSE;
}

BOOL CSurfaceItem::IsNeed_BrightObject() const
{
    if (!bInsp)
        return FALSE;

    std::vector<CSurfaceCriteria>::const_iterator itr = vecCriteria.begin();
    std::vector<CSurfaceCriteria>::const_iterator itrEnd = vecCriteria.end();

    while (itr != itrEnd)
    {
        if (itr->bInsp)
        {
            if (itr->m_color != 0) // Not "DARK"
            {
                return TRUE;
            }
        }

        itr++;
    }

    return FALSE;
}

BOOL CSurfaceItem::IsNeed_DarkObject() const
{
    if (!bInsp)
        return FALSE;

    std::vector<CSurfaceCriteria>::const_iterator itr = vecCriteria.begin();
    std::vector<CSurfaceCriteria>::const_iterator itrEnd = vecCriteria.end();

    while (itr != itrEnd)
    {
        if (itr->bInsp)
        {
            if (itr->m_color != 1) // Not "BRIGHT"
            {
                return TRUE;
            }
        }

        itr++;
    }

    return FALSE;
}

BOOL CSurfaceItem::IsNeed_KeyContrastAverageContrast() const
{
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_KeyContrast))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::t2D_AvgContrast))
        return TRUE;

    return FALSE;
}

BOOL CSurfaceItem::IsNeed_WidthLengthAspectRatioBinaryDensity() const
{
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::Width))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::Length))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::LengthX))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::LengthY))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::Aspect_ratio))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::Binary_density))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::TotalLength))
        return TRUE;

    return FALSE;
}

BOOL CSurfaceItem::IsNeed_LengthXY() const
{
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::LengthX))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::LengthY))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::DefectROI_ratio))
        return TRUE;

    return FALSE;
}

BOOL CSurfaceItem::IsNeed_Local_Area() const
{
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::LocalArea_Percent_all))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::LocalArea_Percent))
        return TRUE;

    return FALSE;
}

BOOL CSurfaceItem::IsNeed_ThicknessLocus() const
{
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::Thickness))
        return TRUE;
    if (IsNeed_Spec(VisionSurfaceCriteria_Column::Locus))
        return TRUE;

    return FALSE;
}

BOOL CSurfaceItem::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;
    if (!db[_T("SurfaceMaskMode")].Link(bSave, m_bSurfaceMaskMode))
        m_bSurfaceMaskMode = FALSE;
    if (!db[_T("strName")].Link(bSave, strName))
        strName = _T("Unknown Item");
    if (!db[_T("bInsp")].Link(bSave, bInsp))
        bInsp = FALSE;
    if (!db[_T("b2ndInsp")].Link(bSave, b2ndInsp))
        b2ndInsp = FALSE;
    if (!db[_T("str2ndCode")].Link(bSave, str2ndCode))
        str2ndCode = _T("");

    AlgoPara.LinkDataBase(bSave, db[_T("AlgoPara")]);

    // 영훈 20150609_JobEditor_Criteria : Criteria 저장 하는 곳
    long nCriteriaSize = (long)vecCriteria.size();
    if (!db[_T("nvecCriteria")].Link(bSave, nCriteriaSize))
        nCriteriaSize = 0;

    vecCriteria.resize(nCriteriaSize);

    for (int n = 0; n < nCriteriaSize; n++)
    {
        vecCriteria[n].LinkDataBase(bSave, db.GetSubDBFmt(_T("Criteria%d"), n));
    }

    if (!bSave && version == 0)
    {
        // Criteria 에서 Type을 Reject으로 설정하면 그냥 Job에 저장된 Mask를 사용
        // Igonore일 경우 매 검사하여 Mask 생성모드 였다

        m_bSurfaceMask_GenerateAlways = TRUE;
        for (long nCriteriaID = 0; nCriteriaID < long(vecCriteria.size()); nCriteriaID++)
        {
            if (vecCriteria[nCriteriaID].m_type == 2) // Reject이 1개라도 있을 경우
            {
                m_bSurfaceMask_GenerateAlways = FALSE;
                break;
            }
        }
    }
    else
    {
        if (!db[_T("SurfaceMask Generate Always")].Link(bSave, m_bSurfaceMask_GenerateAlways))
            m_bSurfaceMask_GenerateAlways = FALSE;
    }

    if (!m_decisionArea.LinkDataBase(bSave, db[_T("DecisionArea")]))
    {
        m_decisionArea.Init();
    }

    return TRUE;
}
