//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SpecLayer.h"

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
SpecLayer::SpecLayer()
    : m_name(_T("Unknown"))
{
}

SpecLayer::~SpecLayer()
{
}

void SpecLayer::Init()
{
    m_single.Init();
}

BOOL SpecLayer::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 3;
    if (!db[_T("version")].Link(bSave, version))
        version = 0;

    if (!db[_T("name")].Link(bSave, m_name))
        m_name = _T("Unknown");

    if (version == 0)
    {
        m_single.LinkDataBase(bSave, db);
    }
    else
    {
        m_single.LinkDataBase(bSave, db[_T("{94E9E748-B880-4BAA-AC30-8C337C10DCAD}")]);
    }

    if (version < 2)
    {
        enum class LayerType : long
        {
            Single,
            Group
        };

        LayerType layerType;

        if (!db[_T("layer_type")].Link(bSave, (long&)layerType))
            layerType = LayerType::Single;
        if (layerType == LayerType::Group)
        {
            m_single.Init();

            auto& gdb = db[_T("group Info")];

            long sequenceCount = 0;
            if (!gdb[_T("{8988E89A-1838-4364-8935-18310FB245F2}")].Link(bSave, sequenceCount))
                sequenceCount = 0;

            for (long n = 0; n < sequenceCount; n++)
            {
                auto& sub_db = gdb.GetSubDBFmt(_T("%d"), n + 1);

                CString name;
                OperationType operation;

                sub_db[_T("X")].Link(bSave, name);
                sub_db[_T("Y")].Link(bSave, (long&)operation);

                m_single.add(UserRoiType::PreparedMask);

                auto& layer = m_single[m_single.count() - 1];
                layer.m_preparedObjectName = name;
                layer.m_operation = operation;

                name.Empty();
            }
        }
    }

    if (version < 3)
    {
        // Version3이후로 다음의 이름이 강제로 바뀌었다.
        for (long index = 0; index < m_single.count(); index++)
        {
            auto& name = m_single[index].m_preparedObjectName;

            if (name == _T("LAND_Layer1"))
                name = _T("Measured - Land Group1");
            if (name == _T("LAND_Layer2"))
                name = _T("Measured - Land Group2");
        }
    }

    for (long index = 0; index < m_single.count(); index++)
    {
        auto& name = m_single[index].m_preparedObjectName;

        if (name == _T("Component Origin_Mask Image"))
            name = _T("Measured - Component with Pad");
        if (name == _T("Component Mask Image"))
            name = _T("Measured - Component");
    }

    return TRUE;
}
