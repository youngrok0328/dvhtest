#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Ball.h"
#include "BallCollectionOriginal.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class BallCollectionOriginal;

//HDR_6_________________________________ Header body
//
namespace Package
{
class DPI_DATABASE_API BallCollection
{
public:
    BallCollection();
    ~BallCollection();

    bool Setup(
        const BallCollectionOriginal& ballInfoMaster, float um2px_x, float um2px_y, const bool removeIgnoredBall);

    void CalcAvrBallDist(float& o_fAvrDist); //얘는 계산
    float GetAvrBallDist(); //애는 m_avrMinimumDist_um 리턴
    void SetBallLink(float fAvrDist_um,
        float fSearchLengthGain); //외부에서 호출해야 한다. AvrDist와 Gain 모두 Parametar로 받아야 한다.

public:
    std::vector<Ball> m_balls;

    float m_minX_px;
    float m_maxX_px;
    float m_minY_px;
    float m_maxY_px;

private:
    float m_avrMinimumDist_um;
};

} // namespace Package