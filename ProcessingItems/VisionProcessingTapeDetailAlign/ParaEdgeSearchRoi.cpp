//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ParaEdgeSearchRoi.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//

ParaEdgeSearchRoi::ParaEdgeSearchRoi()
{
}

BOOL ParaEdgeSearchRoi::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("Search Length")].Link(bSave, m_searchLength_um);
    db[_T("Percent Start")].Link(bSave, m_persent_start);
    db[_T("Percent End")].Link(bSave, m_persent_end);

    return TRUE;
}

Ipvm::Rect32s ParaEdgeSearchRoi::ToRect(const VisionScale& scale, long direction, const Ipvm::Point32r2& center_px,
    float rectSizeX_px, float rectSizeY_px) const
{
    Ipvm::Rect32s roi(0, 0, 0, 0);

    switch (direction)
    {
        case LEFT:
            if (true)
            {
                float midX = center_px.m_x - rectSizeX_px * 0.5f;
                float start = center_px.m_y - rectSizeY_px * 0.5f + rectSizeY_px * m_persent_start / 100.f;
                float end = center_px.m_y - rectSizeY_px * 0.5f + rectSizeY_px * m_persent_end / 100.f;

                roi.m_left = CAST_INT32T(midX - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_right = CAST_INT32T(midX + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_top = (long)min(start, end);
                roi.m_bottom = (long)max(start, end);
            }
            break;

        case RIGHT:
            if (true)
            {
                float midX = center_px.m_x + rectSizeX_px * 0.5f;
                float start = center_px.m_y - rectSizeY_px * 0.5f + rectSizeY_px * m_persent_start / 100.f;
                float end = center_px.m_y - rectSizeY_px * 0.5f + rectSizeY_px * m_persent_end / 100.f;

                roi.m_left = CAST_INT32T(midX - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_right = CAST_INT32T(midX + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_top = (long)min(start, end);
                roi.m_bottom = (long)max(start, end);
            }
            break;

        case UP:
            if (true)
            {
                float midY = center_px.m_y - rectSizeY_px * 0.5f;
                float start = center_px.m_x - rectSizeX_px * 0.5f + rectSizeX_px * m_persent_start / 100.f;
                float end = center_px.m_x - rectSizeX_px * 0.5f + rectSizeX_px * m_persent_end / 100.f;

                roi.m_top = CAST_INT32T(midY - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_bottom = CAST_INT32T(midY + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_left = (long)min(start, end);
                roi.m_right = (long)max(start, end);
            }
            break;

        case DOWN:
            if (true)
            {
                float midY = center_px.m_y + rectSizeY_px * 0.5f;
                float start = center_px.m_x - rectSizeX_px * 0.5f + rectSizeX_px * m_persent_start / 100.f;
                float end = center_px.m_x - rectSizeX_px * 0.5f + rectSizeX_px * m_persent_end / 100.f;

                roi.m_top = CAST_INT32T(midY - scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_bottom = CAST_INT32T(midY + scale.convert_umToPixelX(m_searchLength_um * 0.5f) + .5f);
                roi.m_left = (long)min(start, end);
                roi.m_right = (long)max(start, end);
            }
            break;
    }

    return roi;
}
