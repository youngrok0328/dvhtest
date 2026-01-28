//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceCirteriaColumn.h"

//CPP_2_________________________________ This project's headers
#include "InspBase.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <vector>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
LPCTSTR GetCriteriaColumnKey(VisionSurfaceCriteria_Column column)
{
    switch (column)
    {
        case VisionSurfaceCriteria_Column::Area:
            return _T("{E9F3EA54-97F3-4661-9E91-44D0DF2D16D6}");
        case VisionSurfaceCriteria_Column::Defect_num:
            return _T("{3A98BE61-C18E-4382-A058-E81A3DB18F45}");
        case VisionSurfaceCriteria_Column::Dist_X_to_BodyCenter:
            return _T("{D0B17D2F-E00A-40A9-91AE-F0F58F274AD9}");
        case VisionSurfaceCriteria_Column::Dist_Y_to_BodyCenter:
            return _T("{5CBA3967-3882-4229-8F20-99A3532AE6BE}");
        case VisionSurfaceCriteria_Column::Dist_to_Body:
            return _T("{8AEA7C78-8FD4-4D42-B40E-007FCF243CFC}");
        case VisionSurfaceCriteria_Column::Width:
            return _T("{D2706BC6-46AE-4CFA-A73A-2D96E1775EAD}");
        case VisionSurfaceCriteria_Column::Length:
            return _T("{C0C7EDE4-4F7E-40F2-83C5-A0D7AF85D552}");
        case VisionSurfaceCriteria_Column::LengthX:
            return _T("{E55D24B7-0415-4FFE-9965-313AC4E4CD24}");
        case VisionSurfaceCriteria_Column::LengthY:
            return _T("{A6369310-CBAF-49B1-B7BE-2617F5E94C7F}");
        case VisionSurfaceCriteria_Column::TotalLength:
            return _T("{C541AA04-817B-4548-A98E-2A7F7FEF0E2C}");
        case VisionSurfaceCriteria_Column::LocalArea_Percent_all:
            return _T("{263E2CEA-C6F9-4209-83A6-22537FAF04DE}");
        case VisionSurfaceCriteria_Column::LocalArea_Percent:
            return _T("{B47564AE-6A44-481A-A7ED-2FB92E5EEF5F}");
        case VisionSurfaceCriteria_Column::DefectROI_ratio:
            return _T("{61051F46-8B08-4F44-BC3C-0A204487B97F}");
        case VisionSurfaceCriteria_Column::Thickness:
            return _T("{A0CCBDE7-31ED-461A-B91B-5BE275A71911}");
        case VisionSurfaceCriteria_Column::Locus:
            return _T("{124395EB-C84A-4BE5-BA86-5D005F3FE8F0}");
        case VisionSurfaceCriteria_Column::Aspect_ratio:
            return _T("{E51A5A3F-EE05-44D9-A6ED-7052B70239E3}");
        case VisionSurfaceCriteria_Column::Binary_density:
            return _T("{4D4A7597-6F95-434A-B4A7-620F8BD39FD5}");
        case VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight:
            return _T("{49F7F0F2-E3CA-4028-9E38-3B345C51F6F6}");
        case VisionSurfaceCriteria_Column::t3D_KeyDeltaHeight:
            return _T("{24AAFA8C-AD82-446B-8985-3609BF88A963}");
        case VisionSurfaceCriteria_Column::t2D_KeyContrast:
            return _T("{0282BAD6-8AF2-4769-9E44-C577974EA4CC}");
        case VisionSurfaceCriteria_Column::t2D_AvgContrast:
            return _T("{A594257B-D7D6-440B-8DEE-F93DFC704D65}");
        case VisionSurfaceCriteria_Column::t2D_LocalContrast:
            return _T("{150B133C-0675-426A-A2AD-B3FC1048504C}");
        case VisionSurfaceCriteria_Column::t2D_AuxLocalContrast:
            return _T("{B3056AE9-4231-4AD7-B939-1E2075DD1648}");
        case VisionSurfaceCriteria_Column::t2D_Deviation:
            return _T("{DD299019-4CC4-4ACF-A483-FC9C8E8F83AB}");
        case VisionSurfaceCriteria_Column::t2D_AuxDeviation:
            return _T("{C6697ADE-40EB-42B2-9054-AE4776A7953B}");
        case VisionSurfaceCriteria_Column::t2D_EdgeEnegry:
            return _T("{525FBAB5-A5B4-4760-BD86-EA3A3A4D1D33}");
        case VisionSurfaceCriteria_Column::t2D_MergedCount:
            return _T("{85A36658-3375-4933-A39C-E58B27CC295D}");
        case VisionSurfaceCriteria_Column::t2D_BlobGV_AVR:
            return _T("{F782AC9C-E55B-4F0F-9908-3AACA19C6432}");
        case VisionSurfaceCriteria_Column::t2D_BlobGV_Min:
            return _T("{F21DB3DA-5174-40A9-AC30-BD946E778D66}");
        case VisionSurfaceCriteria_Column::t2D_BlobGV_Max:
            return _T("{BF013683-BC64-413B-A8EC-748540661085}");
    }

    ASSERT(!_T("???"));
    return _T("");
}

LPCTSTR GetCriteriaColumnName(VisionSurfaceCriteria_Column column)
{
    return g_strVisionSurfCriteria_Col[long(column)];
}

long GetCriteriaColumnSpecType(VisionSurfaceCriteria_Column column)
{
    long specType = SItemSpec::NUMBER;

    switch (column)
    {
        case VisionSurfaceCriteria_Column::Area:
            specType = SItemSpec::MEASURE2;
            break;

        case VisionSurfaceCriteria_Column::LocalArea_Percent_all:
        case VisionSurfaceCriteria_Column::LocalArea_Percent:
        case VisionSurfaceCriteria_Column::DefectROI_ratio:

            specType = SItemSpec::PERCENT;
            break;

        case VisionSurfaceCriteria_Column::Width:
        case VisionSurfaceCriteria_Column::Length:
        case VisionSurfaceCriteria_Column::LengthX:
        case VisionSurfaceCriteria_Column::LengthY:
        case VisionSurfaceCriteria_Column::TotalLength:
        case VisionSurfaceCriteria_Column::Thickness:
        case VisionSurfaceCriteria_Column::Locus:
        case VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight:
        case VisionSurfaceCriteria_Column::t3D_KeyDeltaHeight:
        case VisionSurfaceCriteria_Column::Dist_X_to_BodyCenter:
        case VisionSurfaceCriteria_Column::Dist_Y_to_BodyCenter:
        case VisionSurfaceCriteria_Column::Dist_to_Body:
            specType = SItemSpec::MEASURE;
            break;
    }

    return specType;
}

bool GetCriteriaColumnUse(VisionSurfaceCriteria_Column column, bool is3D)
{
    switch (column)
    {
        case VisionSurfaceCriteria_Column::Area:
        case VisionSurfaceCriteria_Column::Defect_num:
        case VisionSurfaceCriteria_Column::Dist_X_to_BodyCenter:
        case VisionSurfaceCriteria_Column::Dist_Y_to_BodyCenter:
        case VisionSurfaceCriteria_Column::Dist_to_Body:
        case VisionSurfaceCriteria_Column::Width:
        case VisionSurfaceCriteria_Column::Length:
        case VisionSurfaceCriteria_Column::LengthX:
        case VisionSurfaceCriteria_Column::LengthY:
        case VisionSurfaceCriteria_Column::TotalLength:
        case VisionSurfaceCriteria_Column::LocalArea_Percent_all:
        case VisionSurfaceCriteria_Column::LocalArea_Percent:
        case VisionSurfaceCriteria_Column::DefectROI_ratio:
        case VisionSurfaceCriteria_Column::Thickness:
        case VisionSurfaceCriteria_Column::Locus:
        case VisionSurfaceCriteria_Column::Aspect_ratio:
            // 2D, 3D 모두 사용함
            return true;

        case VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight:
        case VisionSurfaceCriteria_Column::t3D_KeyDeltaHeight:
            // 3D에서만 사용가능한 항목
            return is3D;
    }

    // 2D 에서만 사용가능한 항목
    return !is3D;
}

class ColumnCollect
{
public:
    ColumnCollect(bool is3D)
    {
        for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
        {
            if (GetCriteriaColumnUse(VisionSurfaceCriteria_Column(item), is3D))
            {
                m_collectColumns.push_back(item);
            }
        }
    }

    std::vector<long> m_collectColumns;
};
ColumnCollect g_count2D(false);
ColumnCollect g_count3D(true);

long GetCriteriaColumnCount(bool is3D)
{
    if (is3D)
    {
        return long(g_count3D.m_collectColumns.size());
    }

    return long(g_count2D.m_collectColumns.size());
}

VisionSurfaceCriteria_Column GetCriteriaColumnID(bool is3D, long index)
{
    if (is3D)
    {
        return VisionSurfaceCriteria_Column(g_count3D.m_collectColumns[index]);
    }

    return VisionSurfaceCriteria_Column(g_count2D.m_collectColumns[index]);
}
