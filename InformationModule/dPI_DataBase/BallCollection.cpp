//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BallCollection.h"

//CPP_2_________________________________ This project's headers
#include "BallCollectionOriginal.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
namespace Package
{
BallCollection::BallCollection()
    : m_minX_px(0.f)
    , m_maxX_px(0.f)
    , m_minY_px(0.f)
    , m_maxY_px(0.f)
    , m_avrMinimumDist_um(0.f)
{
}

BallCollection::~BallCollection()
{
}

bool BallCollection::Setup(
    const BallCollectionOriginal& ballInfoMaster, float um2px_x, float um2px_y, const bool removeIgnoredBall)
{
    m_balls.clear();

    m_minX_px = 0.f;
    m_maxX_px = 0.f;
    m_minY_px = 0.f;
    m_maxY_px = 0.f;
    m_avrMinimumDist_um = 0.f;

    const auto ballCount = ballInfoMaster.m_balls.size();
    const static float fDefaultGain = 1.5f;

    if (ballCount <= 0)
    {
        return true;
    }

    m_balls.reserve(ballCount);

    // 직관적으로 Device 를 뒤집을 후 회전을 생각하게 되므로
    // 이에 맞게 자세 변경 순서를 적용한다.
    const long mul_x = ballInfoMaster.m_ballMapFlipX ? -1 : 1;
    const long mul_y = ballInfoMaster.m_ballMapFlipY ? -1 : 1;

    long cos_val = 1;
    long sin_val = 0;

    const long bumpRotation = long(ballInfoMaster.m_ballMapRotation) % 4;

    switch (bumpRotation)
    {
        case 0: // 0 Deg
            cos_val = 1;
            sin_val = 0;
            break;
        case 1: // 90 Deg
            cos_val = 0;
            sin_val = 1;
            break;
        case 2: // 180 Deg
            cos_val = -1;
            sin_val = 0;
            break;
        case 3: // 270 Deg
            cos_val = 0;
            sin_val = -1;
            break;
    }

    const float um2px_y_abs = CAST_FLOAT(fabs(um2px_y));

    m_minX_px = FLT_MAX;
    m_maxX_px = -FLT_MAX;
    m_minY_px = FLT_MAX;
    m_maxY_px = -FLT_MAX;

    long ballIndex = 0;

    std::vector<long> vecBallLinkTemp(0); //어차피 아래의 PackageSpec_BallMap::SetBallLink() 함수에서 계산해준다.

    for (const auto& ball : ballInfoMaster.m_balls)
    {
        if (removeIgnoredBall && ball.m_ignored)
            continue;

        const float x_um = CAST_FLOAT(mul_x * cos_val * ball.m_posX_um - mul_y * sin_val * ball.m_posY_um);
        const float y_um = CAST_FLOAT(mul_x * sin_val * ball.m_posX_um + mul_y * cos_val * ball.m_posY_um);

        const float x_px = x_um * um2px_x;
        const float y_px = y_um * um2px_y;

        m_balls.emplace_back(ball.m_groupID, ball.m_typeIndex, ball.m_name, ballIndex, x_um, y_um, ball.m_diameter_um,
            ball.m_height_um, x_px, y_px, 0.5 * ball.m_diameter_um * um2px_x, 0.5 * ball.m_diameter_um * um2px_y_abs,
            ball.m_ignored, vecBallLinkTemp);

        if (x_px < m_minX_px)
            m_minX_px = x_px;
        if (x_px > m_maxX_px)
            m_maxX_px = x_px;
        if (y_px < m_minY_px)
            m_minY_px = y_px;
        if (y_px < m_maxY_px)
            m_maxY_px = y_px;

        ballIndex++;
    }

    if (removeIgnoredBall == true
        && (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_2D_INSP
            || SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_TR))
    {
        CalcAvrBallDist(m_avrMinimumDist_um);
        SetBallLink(m_avrMinimumDist_um, fDefaultGain);
    }

    return true;
}

float BallCollection::GetAvrBallDist()
{
    return m_avrMinimumDist_um;
}

void BallCollection::CalcAvrBallDist(float& o_fAvrDist)
{
    o_fAvrDist = 0.f;

    long nBallNum = (long)m_balls.size();
    if (nBallNum < 5)
        return;

    std::vector<long> vecnLinkBallID(nBallNum, -1); //가장 까까운 Ball의 Index
    std::vector<double> vecfLinkBallMinDistSqr(nBallNum, 0.f); //가장 가까운 Ball까지의 거리

    long* pnLinkBallID = nBallNum ? &vecnLinkBallID[0] : nullptr;
    double* pfLinkBallMinDistSqr = nBallNum ? &vecfLinkBallMinDistSqr[0] : nullptr;

    double distBallX{};
    double distBallY{};

    static const double maxLimitDist = 4000.; //4mm
    static const double minLimitDist = -4000.; //4mm

    for (auto& srcBall : m_balls)
    {
        const double fSrcBallPosX = srcBall.m_posX_um;
        const double fSrcBallPosY = srcBall.m_posY_um;

        for (auto& dstBall : m_balls)
        {
            //비교 대상이 자기 자신이거나 Dest Ball의 LinkBall이 자기 자신이면 해당 Ball을 Passing
            if (srcBall.m_index == dstBall.m_index || srcBall.m_index == pnLinkBallID[dstBall.m_index])
                continue;

            const double fDstBallPosX = dstBall.m_posX_um;
            const double fDstBallPosY = dstBall.m_posY_um;

            distBallX = fDstBallPosX - fSrcBallPosX;
            distBallY = fDstBallPosY - fSrcBallPosY;

            if (distBallX > maxLimitDist || distBallY > maxLimitDist || distBallX < minLimitDist
                || distBallY < minLimitDist) //4mm 이상은 제외
                continue;

            const double fDistSqr = (distBallX * distBallX) + (distBallY * distBallY);

            if (pfLinkBallMinDistSqr[srcBall.m_index] > fDistSqr
                || pfLinkBallMinDistSqr[srcBall.m_index]
                    == 0.f) //vecfLinkBallMinDist이 초기값이 0이니까 0이면 fDist를 적용해야한다
            {
                pfLinkBallMinDistSqr[srcBall.m_index] = fDistSqr;
                pnLinkBallID[srcBall.m_index] = dstBall.m_index;
            }
        }
    }

    //버블 소팅한다.
    std::sort(vecfLinkBallMinDistSqr.begin(), vecfLinkBallMinDistSqr.end());

    //상하위 30%를 제외한 나머지의 평균을 구한다.
    long nStart = (long)((float)nBallNum * .3f + .5f);
    long nEnd = (long)((float)nBallNum * .7f);
    double dSum = 0.;
    long nDataNum = 0;
    for (long nID = nStart; nID < nEnd; nID++)
    {
        if (pfLinkBallMinDistSqr[nID] > 0.f)
        {
            dSum += sqrt(pfLinkBallMinDistSqr[nID]);
            nDataNum++;
        }
    }

    o_fAvrDist = (float)(dSum / (double)nDataNum);
}

void BallCollection::SetBallLink(float fAvrDist_um, float fSearchLengthGain)
{
    long nBallNum = (long)m_balls.size();
    if (nBallNum < 5 || fAvrDist_um < 0.f || fSearchLengthGain < 0.5f)
        return;

    const double fSearchRangeR_um = fAvrDist_um * fSearchLengthGain;
    const double fSearchRangeRminus_um = -fSearchRangeR_um;
    const double fSearchRangeRSqr_um = fSearchRangeR_um * fSearchRangeR_um;

    double distBallX{};
    double distBallY{};
    double distMinX{};
    double distMinY{};

    for (auto& srcBall : m_balls)
    {
        srcBall.m_vecLinkBallID.clear();

        const double fSrcBallPosX = srcBall.m_posX_um;
        const double fSrcBallPosY = srcBall.m_posY_um;

        distMinX = FLT_MAX;
        distMinY = FLT_MAX;

        for (auto& dstBall : m_balls)
        {
            //비교 대상이 자기 자신이거나 Dest Ball의 LinkBall이 자기 자신이면 해당 Ball을 Passing
            if (srcBall.m_index == dstBall.m_index)
                continue;

            //현재 Ball과 Dest Ball간 거리를 계산한 후 그 거리가 정해진 거리보다 작은 지 확인하고
            const double fDstBallPosX = dstBall.m_posX_um;
            const double fDstBallPosY = dstBall.m_posY_um;

            distBallX = fDstBallPosX - fSrcBallPosX;
            distBallY = fDstBallPosY - fSrcBallPosY;

            //{{//Pitch 검사를 위한 Ball ID 탐색 구간
            //distBallY가 0이면, distBallX가 가장 작은 BallID를 저장한다.
            if (distBallY == 0 && distBallX > 0. && distBallX < distMinX)
            {
                distMinX = distBallX;
                srcBall.m_ballID_PitchX = dstBall.m_index;
            }
            if (distBallX == 0 && distBallY > 0. && distBallY < distMinY)
            {
                distMinY = distBallY;
                srcBall.m_ballID_PitchY = dstBall.m_index;
            }
            //}}

            if (distBallX > fSearchRangeR_um || distBallY > fSearchRangeR_um || distBallX < fSearchRangeRminus_um
                || distBallY < fSearchRangeRminus_um) //4mm 이상은 제외
                continue;

            const double fDistSqr = (distBallX * distBallX) + (distBallY * distBallY);

            if (fDistSqr > fSearchRangeRSqr_um)
                continue;

            bool bIsSameLink = false;

            for (long linkBallID : dstBall.m_vecLinkBallID)
            {
                if (srcBall.m_index == linkBallID)
                {
                    bIsSameLink = true;
                    break;
                }
            }

            if (bIsSameLink)
            {
                continue;
            }

            //거리를 만족하면 각도가 45도에서 -135도 사이인지 확인
            const float fAngle = CPI_Geometry::Get1LineAngleDeg(
                CAST_FLOAT(fSrcBallPosX), CAST_FLOAT(fSrcBallPosY), CAST_FLOAT(fDstBallPosX), CAST_FLOAT(fDstBallPosY));

            if (fAngle >= 45.f || fAngle < -135.f)
                continue;

            srcBall.m_vecLinkBallID.push_back(dstBall.m_index);
        }
    }
}

} // namespace Package
