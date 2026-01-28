//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Analysis.h"

//CPP_2_________________________________ This project's headers
#include "ArchiveAllType.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
static long g_histogramMaxSize = 10000;
static long g_histogramCenter = g_histogramMaxSize / 2;

Analysis::Analysis()
    : m_use(false)
    , m_packageOffsetX(0.f)
    , m_packageOffsetY(0.f)
    , m_histogramZ(nullptr)
{
    Init();
}

Analysis::Analysis(const Analysis& object)
    : m_histogramZ(nullptr)
{
    *this = object;
}

Analysis::~Analysis()
{
    delete[] m_histogramZ;
}

void Analysis::Init()
{
    m_use = false;
    m_packageOffsetX = 0.f;
    m_packageOffsetY = 0.f;

    delete[] m_histogramZ;
    m_histogramZ = nullptr;
}

void Analysis::SetUse(bool use)
{
    m_use = use;

    delete[] m_histogramZ;
    m_histogramZ = nullptr;

    if (m_use)
    {
        m_histogramZ = new long[g_histogramMaxSize];
        memset(m_histogramZ, 0, sizeof(long) * g_histogramMaxSize);
    }
}

bool Analysis::isUsed() const
{
    return m_use;
}

bool Analysis::AccumulateZ(float z, long count)
{
    long index = long(z) + g_histogramCenter;
    if (index < 0 || index >= g_histogramMaxSize)
        return false;
    if (m_histogramZ == nullptr)
        return false;

    m_histogramZ[index] += count;

    return true;
}

std::pair<float, long> Analysis::GetZC(long index) const
{
    if (m_histogramZ == nullptr || index < 0 || index >= g_histogramMaxSize)
    {
        return std::pair<float, long>(0.f, 0);
    }

    float z = (float)(index - g_histogramCenter);

    return std::pair<float, long>(z, m_histogramZ[index]);
}

long Analysis::GetZCCount() const
{
    if (m_histogramZ == nullptr)
        return 0;

    return g_histogramMaxSize;
}

void Analysis::Serialize(ArchiveAllType& ar)
{
    bool use = m_use;
    ar.Serialize_Element(use);

    if (ar.IsLoading())
    {
        SetUse(use);
    }

    if (!use)
        return;

    ar.Serialize_Element(m_packageOffsetX);
    ar.Serialize_Element(m_packageOffsetY);

    if (ar.IsStoring())
    {
        ar.Write(m_histogramZ, g_histogramMaxSize * sizeof(long));
    }
    else
    {
        ar.Read(m_histogramZ, g_histogramMaxSize * sizeof(long));
    }
}

Analysis& Analysis::operator=(const Analysis& object)
{
    m_use = object.m_use;
    m_packageOffsetX = object.m_packageOffsetX;
    m_packageOffsetY = object.m_packageOffsetY;

    delete[] m_histogramZ;
    m_histogramZ = nullptr;
    if (m_use)
    {
        m_histogramZ = new long[g_histogramMaxSize];
        memcpy(m_histogramZ, object.m_histogramZ, g_histogramMaxSize * sizeof(long));
    }

    return *this;
}
