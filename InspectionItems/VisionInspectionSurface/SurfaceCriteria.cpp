//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceCriteria.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceItemResult.h"

//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CSurfaceCriteria::CSurfaceCriteria()
    : bInsp(FALSE)
    , strName(_T("Unknown"))
    , m_color(1) // Default : Bright
    , m_type(2) // Default : Reject Criteria
    , vecbUseDecisionArea(1)
{
    for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
    {
        long specType = GetCriteriaColumnSpecType(VisionSurfaceCriteria_Column(item));

        m_specTable_min[item]
            = new SItemSpec(FALSE, SItemSpec::fLimitMin, SItemSpec::fLimitMin, 0.f, 0.f, 0.f, specType, FALSE, FALSE);
        m_specTable_max[item]
            = new SItemSpec(FALSE, 0.f, 0.f, 0.f, SItemSpec::fLimitMax, SItemSpec::fLimitMax, specType, FALSE, FALSE);
    }

    vecbUseDecisionArea[0] = TRUE;
}

CSurfaceCriteria::CSurfaceCriteria(const CSurfaceCriteria& Src)
    : bInsp(Src.bInsp)
    , strName(Src.strName)
    , m_color(Src.m_color)
    , m_type(Src.m_type)
    , vecbUseDecisionArea(Src.vecbUseDecisionArea)
{
    for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
    {
        m_specTable_min[item] = new SItemSpec(*Src.m_specTable_min[item]);
        m_specTable_max[item] = new SItemSpec(*Src.m_specTable_max[item]);
    }
}

CSurfaceCriteria::~CSurfaceCriteria()
{
    for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
    {
        delete m_specTable_min[item];
        delete m_specTable_max[item];
    }
}

CSurfaceCriteria& CSurfaceCriteria::operator=(const CSurfaceCriteria& Src)
{
    bInsp = Src.bInsp;
    strName = Src.strName;
    m_color = Src.m_color;
    m_type = Src.m_type;

    for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
    {
        *m_specTable_min[item] = *Src.m_specTable_min[item];
        *m_specTable_max[item] = *Src.m_specTable_max[item];
    }

    vecbUseDecisionArea = Src.vecbUseDecisionArea;

    return *this;
}

void CSurfaceCriteria::Init()
{
    bInsp = FALSE;
    strName = _T("Unknown");
    m_color = 1;
    m_type = 2;

    for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
    {
        m_specTable_min[item]->Set(
            FALSE, SItemSpec::fLimitMin, SItemSpec::fLimitMin, 0.f, 0.f, 0.f, SItemSpec::MIL, FALSE);
        m_specTable_max[item]->Set(
            FALSE, 0.f, 0.f, 0.f, SItemSpec::fLimitMax, SItemSpec::fLimitMax, SItemSpec::MIL, FALSE);
    }

    vecbUseDecisionArea[0] = TRUE;
}

long CSurfaceCriteria::Judge(bool is3D, SDetectedSurfaceObject& DSOValue) const
{
    // 색깔이 틀리면 Pass
    if (m_color != 2 && m_color != DSOValue.m_color)
        return PASS;

    std::vector<long> result_min;
    std::vector<long> result_max;

    long columnCount = GetCriteriaColumnCount(is3D);
    result_min.resize(columnCount, NOT_MEASURED);
    result_max.resize(columnCount, NOT_MEASURED);

    BOOL bAllNotMeasured = TRUE;

    for (long item = 0; item < columnCount; item++)
    {
        auto columnID = GetCriteriaColumnID(is3D, item);

        if (columnID == VisionSurfaceCriteria_Column::Defect_num)
        {
            // Defect Num은 여기서 판단 할 수 없다
            continue;
        }

        long specIndex = long(columnID);

        if (m_specTable_min[specIndex]->GetInsp())
        {
            result_min[item] = m_specTable_min[specIndex]->Judge(DSOValue.m_inspValue[specIndex]);
            if (result_min[item] == PASS)
                return PASS;
        }

        if (m_specTable_max[specIndex]->GetInsp())
        {
            result_max[item] = m_specTable_max[specIndex]->Judge(DSOValue.m_inspValue[specIndex]);
            if (result_max[item] == PASS)
                return PASS;
        }

        bAllNotMeasured &= (result_min[item] == NOT_MEASURED);
        bAllNotMeasured &= (result_max[item] == NOT_MEASURED);
    }

    if (bAllNotMeasured)
    {
        return PASS;
    }

    return m_type == 1 ? MARGINAL : REJECT;
}

SItemSpec& CSurfaceCriteria::GetSpecMin(VisionSurfaceCriteria_Column column)
{
    return *m_specTable_min[long(column)];
}

SItemSpec& CSurfaceCriteria::GetSpecMax(VisionSurfaceCriteria_Column column)
{
    return *m_specTable_max[long(column)];
}

const SItemSpec& CSurfaceCriteria::GetSpecMin(VisionSurfaceCriteria_Column column) const
{
    return *m_specTable_min[long(column)];
}

const SItemSpec& CSurfaceCriteria::GetSpecMax(VisionSurfaceCriteria_Column column) const
{
    return *m_specTable_max[long(column)];
}

// 영훈 20150609_JobEditor_Criteria : Criteria는 추후 추가하도록 한다.
BOOL CSurfaceCriteria::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        Init();
    }

    long version = 20180511;

    if (!db[_T("version")].Link(bSave, version))
        version = 0;
    if (!db[_T("bInsp")].Link(bSave, bInsp))
        bInsp = FALSE;
    if (!db[_T("strName")].Link(bSave, strName))
        strName = _T("Unknown");
    if (!db[_T("nColor")].Link(bSave, m_color))
        m_color = 1;
    if (!db[_T("nType")].Link(bSave, m_type))
        m_type = 2;

    long nSize = (long)vecbUseDecisionArea.size();
    if (!db[_T("vecbUseDecisionAreaSize")].Link(bSave, nSize))
        nSize = 0;
    vecbUseDecisionArea.resize(nSize);

    CString str;
    for (long n = 0; n < nSize; n++)
    {
        str.Format(_T("vecbUseDecisionArea%d"), n);
        if (!db[str].Link(bSave, vecbUseDecisionArea[n]))
            vecbUseDecisionArea[n] = FALSE;
    }

    if (version == 0)
    {
        for (long item = 0; item < 27; item++)
        {
            VisionSurfaceCriteria_Column columnIndex = VisionSurfaceCriteria_Column::Area;
            switch (item)
            {
                case 0:
                    columnIndex = VisionSurfaceCriteria_Column::Area;
                    break;
                case 1:
                    columnIndex = VisionSurfaceCriteria_Column::Defect_num;
                    break;
                case 2:
                    columnIndex = VisionSurfaceCriteria_Column::Dist_X_to_BodyCenter;
                    break;
                case 3:
                    columnIndex = VisionSurfaceCriteria_Column::Dist_Y_to_BodyCenter;
                    break;
                case 4:
                    columnIndex = VisionSurfaceCriteria_Column::Dist_to_Body;
                    break;
                case 5:
                    columnIndex = VisionSurfaceCriteria_Column::Width;
                    break;
                case 6:
                    columnIndex = VisionSurfaceCriteria_Column::Length;
                    break;
                case 7:
                    columnIndex = VisionSurfaceCriteria_Column::LengthX;
                    break;
                case 8:
                    columnIndex = VisionSurfaceCriteria_Column::LengthY;
                    break;
                case 9:
                    columnIndex = VisionSurfaceCriteria_Column::TotalLength;
                    break;
                case 10:
                    columnIndex = VisionSurfaceCriteria_Column::LocalArea_Percent_all;
                    break;
                case 11:
                    columnIndex = VisionSurfaceCriteria_Column::LocalArea_Percent;
                    break;
                case 12:
                    columnIndex = VisionSurfaceCriteria_Column::DefectROI_ratio;
                    break;
                case 13:
                    columnIndex = VisionSurfaceCriteria_Column::Thickness;
                    break;
                case 14:
                    columnIndex = VisionSurfaceCriteria_Column::Locus;
                    break;
                case 15:
                    columnIndex = VisionSurfaceCriteria_Column::Aspect_ratio;
                    break;
                case 16:
                    columnIndex = VisionSurfaceCriteria_Column::Binary_density;
                    break;
                case 17:
                    columnIndex = VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight;
                    break;
                case 18:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_KeyContrast;
                    break;
                case 19:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_AvgContrast;
                    break;
                case 20:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_LocalContrast;
                    break;
                case 21:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_AuxLocalContrast;
                    break;
                case 22:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_Deviation;
                    break;
                case 23:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_AuxDeviation;
                    break;
                case 24:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_EdgeEnegry;
                    break;
                case 25:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_MergedCount;
                    break;
                case 26:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_BlobGV_AVR;
                    break;
                case 27:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_BlobGV_Min;
                    break;
                case 28:
                    columnIndex = VisionSurfaceCriteria_Column::t2D_BlobGV_Max;
                    break;
            }

            auto& criteria_db = db.GetSubDBFmt(_T("criteria%d"), item);

            m_specTable_min[long(columnIndex)]->LinkDataBase(bSave, criteria_db[_T("Min")]);
            m_specTable_max[long(columnIndex)]->LinkDataBase(bSave, criteria_db[_T("Max")]);
        }
    }
    else
    {
        long itemCount = long(VisionSurfaceCriteria_Column::Surface_Criteria_END);
        if (!db[_T("itemCount")].Link(bSave, itemCount))
            itemCount = 0;

        for (long item = 0; item < itemCount; item++)
        {
            auto& criteria_db = db.GetSubDBFmt(_T("criteria%d"), item);

            CString criteriaKey;
            if (bSave)
            {
                criteriaKey = GetCriteriaColumnKey(VisionSurfaceCriteria_Column(item));
            }

            criteria_db[_T("Key")].Link(bSave, criteriaKey);

            long specIndex = item;
            if (!bSave)
            {
                specIndex = -1;

                for (long findIndex = 0; findIndex < long(VisionSurfaceCriteria_Column::Surface_Criteria_END);
                    findIndex++)
                {
                    if (GetCriteriaColumnKey(VisionSurfaceCriteria_Column(findIndex)) == criteriaKey)
                    {
                        specIndex = findIndex;
                        break;
                    }
                }

                if (specIndex < 0)
                    continue;
            }

            m_specTable_min[specIndex]->LinkDataBase(bSave, criteria_db[_T("Min")]);
            m_specTable_max[specIndex]->LinkDataBase(bSave, criteria_db[_T("Max")]);

            criteriaKey.Empty();
        }
    }

    if (!bSave)
    {
        // 예전에 잘못 저장된 스펙 타입이 있어서 강제로 타입을 다시 세팅해 주게 하였다
        for (long item = 0; item < long(VisionSurfaceCriteria_Column::Surface_Criteria_END); item++)
        {
            long specType = GetCriteriaColumnSpecType(VisionSurfaceCriteria_Column(item));

            m_specTable_min[item]->SetSpecType(specType);
            m_specTable_max[item]->SetSpecType(specType);
        }
    }

    str.Empty();

    return TRUE;
}
