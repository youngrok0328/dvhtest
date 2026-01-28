//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Ball.h"

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
namespace Package
{
Ball::Ball(LPCTSTR groupID, long typeIndex, LPCTSTR name, long index, double posX_um, double posY_um,
    double diameter_um, double height_um, double posX_px, double posY_px, double radiusX_px, double radiusY_px,
    bool ignored, std::vector<long>& vecLinkBallID)
    : m_groupID(groupID)
    , m_typeIndex(typeIndex)
    , m_name(name)
    , m_index(index)
    , m_posX_um(posX_um)
    , m_posY_um(posY_um)
    , m_diameter_um(diameter_um)
    , m_height_um(height_um)
    , m_posX_px(posX_px)
    , m_posY_px(posY_px)
    , m_radiusX_px(radiusX_px)
    , m_radiusY_px(radiusY_px)
    , m_ignored(ignored)
    , m_ballID_PitchX(-1)
    , m_ballID_PitchY(-1) //-1이면 Pitch 검사 대상이 없다.
{
    m_vecLinkBallID.clear();
    long nLinkNum = (long)vecLinkBallID.size();
    m_vecLinkBallID.resize(nLinkNum);
    for (long nLink = 0; nLink < nLinkNum; nLink++)
        m_vecLinkBallID[nLink] = vecLinkBallID[nLink];
}

} // namespace Package
