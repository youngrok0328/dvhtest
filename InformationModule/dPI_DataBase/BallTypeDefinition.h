#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "BallOriginal.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <afxstr.h>
#include <minwindef.h>
#include <winnt.h>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
// 볼 위치는 유닛의 중앙을 원점으로 하는 오른손 좌표계를 사용한다.
namespace Package
{
struct DPI_DATABASE_API BallTypeDefinition
{
public:
    BallTypeDefinition() = delete;
    BallTypeDefinition(const BallTypeDefinition& rhs) = default;
    BallTypeDefinition(
        LPCTSTR groupID, const double diameter_um, const double height_um, const long groupBallNum); //kircheis_AIBC
    ~BallTypeDefinition() = default;

    BallTypeDefinition& operator=(const BallTypeDefinition& rhs) = default;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

public:
    CString m_groupID;
    double m_diameter_um;
    double m_height_um;
    long m_groupBallNum; //kircheis_AIBC
};

} // namespace Package
