#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/VisionTapeSpec.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DrawViewTape : public CStatic
{
public:
    bool Create(CWnd* parentWnd, const CRect& rect);
    void Set(const VisionTapeSpec& spec);

private:
    VisionTapeSpec m_spec;
    virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
    void DrawHoles(CDC& dc, float holeCenterX_px, float firstPocketY_px, float scale);

    DECLARE_MESSAGE_MAP()
};
