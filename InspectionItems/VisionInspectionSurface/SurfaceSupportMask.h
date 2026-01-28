#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SurfaceSupportMask
{
public:
    SurfaceSupportMask();
    ~SurfaceSupportMask();

    const Ipvm::Image8u& Get(int n) const;
    const Ipvm::Image8u* Find(LPCTSTR name) const;

    int count() const;
    LPCTSTR GetBitmapMaskName(int n) const;

    void Set(LPCTSTR maskName, const Ipvm::Image8u& image);

private:
    struct Item
    {
        CString m_name;
        Ipvm::Image8u m_image;
    };

    std::vector<Item> m_maskList;
};
