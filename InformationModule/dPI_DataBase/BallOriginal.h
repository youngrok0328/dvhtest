#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
// 볼 위치는 유닛의 중앙을 원점으로 하는 오른손 좌표계를 사용한다.
namespace Package
{
class DPI_DATABASE_API BallOriginal
{
public:
    BallOriginal() = delete;
    BallOriginal(const BallOriginal& rhs) = default;
    BallOriginal(LPCTSTR groupID, LPCTSTR name, long index, double posX_um, double posY_um, double diameter_um,
        double height_um, bool bIgnore);
    ~BallOriginal() = default;

    BallOriginal& operator=(const BallOriginal& rhs) = default;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    CString m_name;
    long m_index; // Ball map 상에서 몇 번째인지를 나타냄..순서.
    double m_posX_um;
    double m_posY_um;
    double m_diameter_um;
    double m_height_um;
    bool m_ignored; // true 면 검사에서 제외
    CString m_groupID;
    long m_typeIndex;
};

} // namespace Package