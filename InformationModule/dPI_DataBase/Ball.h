#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
// 볼 위치는 픽셀 단위일 경우 유닛의 중앙을 원점으로 하는 왼손 좌표계를 사용한다.
// 볼 위치는 실 단위일 경우 유닛의 중앙을 원점으로 하는 오른손 좌표계를 사용한다.
namespace Package
{
class DPI_DATABASE_API Ball
{
public:
    Ball() = delete;
    Ball(const Ball& rhs) = default;
    Ball(LPCTSTR groupID, long typeIndex, LPCTSTR name, long index, double posX_um, double posY_um, double diameter_um,
        double height_um, double posX_px, double posY_px, double radiusX_px, double radiusY_px, bool ignored,
        std::vector<long>& vecLinkBallID);
    ~Ball() = default;

    Ball& operator=(const Ball& rhs) = default;

    CString m_groupID;
    long m_typeIndex;

    CString m_name;
    long m_index; // Ball map 상에서 몇 번째인지를 나타냄..순서.
    double m_posX_um; // 오른손 좌표계
    double m_posY_um; // 오른손 좌표계
    double m_diameter_um;
    double m_height_um;
    double m_posX_px; // 왼손 좌표계
    double m_posY_px; // 왼손 좌표계
    double m_radiusX_px;
    double m_radiusY_px;
    bool m_ignored; // true 면 검사에서 제외

    std::vector<long> m_vecLinkBallID;

    long m_ballID_PitchX; //해당 Ball과 Pitch X 검사 대상인 Ball의 ID
    long m_ballID_PitchY; //해당 Ball과 Pitch Y 검사 대상인 Ball의 ID
};

} // namespace Package
