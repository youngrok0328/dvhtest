//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionTapeSpec.h"

//CPP_2_________________________________ This project's headers
#include "VisionTrayScanSpec.h"

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
BOOL VisionTapeSpec::LinkDataBase(BOOL save, CiDataBase& db)
{
    if (!save)
    {
        *this = VisionTapeSpec(); // Reset to default values
    }

    db[L"{25E33443-925D-453B-9954-C45BF6296E36}"].Link(save, (int32_t&)m_sprocketHoleExistType);
    db[L"{18D8ACCF-2345-41FD-A9E7-200FB331EBA3}"].Link(save, m_w0);
    db[L"{0254A856-2159-4BEB-965F-FA773B250C63}"].Link(save, m_a0);
    db[L"{BC8FEDBA-0588-464C-9A48-AAA3428A4CDE}"].Link(save, m_b0);
    db[L"{87B47A57-3D87-4BF2-92ED-BA5437864A0B}"].Link(save, m_p0);
    db[L"{C70F5A7A-B850-47F2-AA58-36D24EB68B9C}"].Link(save, m_p1);
    db[L"{699267C2-353B-4FF2-A4AA-03B638E8E5D9}"].Link(save, m_d0);
    db[L"{62AB0A13-C92A-4AC3-B24C-6EB0AD7EA1B8}"].Link(save, m_d1);
    db[L"{FE2EFC71-19A5-45FD-A5A1-A75F5C23FE90}"].Link(save, m_s0);
    db[L"{39F639AD-6B9F-4A06-80D4-9381A1AD8F6F}"].Link(save, m_s1);
    db[L"{2F10309F-3A4F-4292-A319-43631DC6F597}"].Link(save, m_s2);
    db[L"{BD364A3D-B648-40BB-83E5-187DE302EAAA}"].Link(save, m_e1);
    db[L"{2BB7FA94-9899-491D-A804-E09B5B240096}"].Link(save, m_f0);
    db[L"{BCA8CD8D-713D-43EF-B6C5-772E54569F16}"].Link(save, m_f1);
    db[L"{9644739D-AF36-4A2B-9915-8C143B419848}"].Link(save, m_f2);

    return TRUE;
}

void VisionTapeSpec::ApplyToTrayScanSpec(VisionTrayScanSpec& trayScanSpec) const
{
    trayScanSpec.m_pocketPitchX_mm = m_w0 + 6; // Tape Width + Margin
    trayScanSpec.m_pocketPitchY_mm = m_p1 * 1.5f; // Pocket Pitch * 1.5 (여유있게)
}
