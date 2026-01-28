#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class _DA_BASE_MODULE_API SprocketHoleInfo //In-Pocket 과 OTI에서 모두 사용해야하므로 별도의 클래스로 만든다.
{
public:
    SprocketHoleInfo();
    ~SprocketHoleInfo();

    void Init();
    bool CollectSprocketHoleInfo(const Ipvm::EllipseEq32r* pEllipseEqAll, const long circleNum,
        const Ipvm::Point32r2* pEdgePointAll, const long pointNum, const Ipvm::Point32r2& centerPoint,
        const Ipvm::Point32r2& scale);

    std::vector<Ipvm::CircleEq32r> m_vecCircleEqLeftSprockeHole; //좌측  Sprocket Hole 정보
    std::vector<Ipvm::CircleEq32r> m_vecCircleEqRightSprockeHole; //우측 Sprocket Hole 정보

    std::vector<std::vector<Ipvm::Point32r2>> m_vec2PointLeftSprocketHole; //좌측 Sprocket Hole의 Edge Point 정보
    std::vector<std::vector<Ipvm::Point32r2>> m_vec2PointRightSprocketHole; //우측 Sprocket Hole의 Edge Point 정보

    bool m_isExistLeftSprocketHole; // 좌측 Sprocket Hole 존재 여부
    bool m_isExistRightSprocketHole; // 우측 Sprocket Hole 존재 여부
};
