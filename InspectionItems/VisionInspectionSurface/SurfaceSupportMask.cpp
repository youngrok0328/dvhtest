//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceSupportMask.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SurfaceSupportMask::SurfaceSupportMask()
{
}

SurfaceSupportMask::~SurfaceSupportMask()
{
}

const Ipvm::Image8u& SurfaceSupportMask::Get(int n) const
{
    return m_maskList[n].m_image;
}

const Ipvm::Image8u* SurfaceSupportMask::Find(LPCTSTR name) const
{
    for (auto& mask : m_maskList)
    {
        if (mask.m_name == name)
        {
            return &mask.m_image;
        }
    }

    return nullptr;
}

int SurfaceSupportMask::count() const
{
    return long(m_maskList.size());
}

LPCTSTR SurfaceSupportMask::GetBitmapMaskName(int n) const
{
    return m_maskList[n].m_name;
}

void SurfaceSupportMask::Set(LPCTSTR maskName, const Ipvm::Image8u& image)
{
    for (auto& mask : m_maskList)
    {
        if (mask.m_name == maskName)
        {
            mask.m_image = image;
            return;
        }
    }

    Item item;
    item.m_name = maskName;
    item.m_image = image;

    m_maskList.push_back(item);
}
