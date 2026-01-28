//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SprocketHoleInfo.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <algorithm>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
SprocketHoleInfo::SprocketHoleInfo()
{
    Init();
}

SprocketHoleInfo::~SprocketHoleInfo()
{

}

void SprocketHoleInfo::Init()
{
    //Circle 정보 초기화
    m_vecCircleEqLeftSprockeHole.clear();
    m_vecCircleEqRightSprockeHole.clear();

    // Edge Point 정보 초기화
    long nLeftSize = static_cast<long>(m_vec2PointLeftSprocketHole.size());
    long nRightSize = static_cast<long>(m_vec2PointRightSprocketHole.size());

    for (long i = 0; i < nLeftSize; i++)
    {
        m_vec2PointLeftSprocketHole[i].clear();
    }
    for (long i = 0; i < nRightSize; i++)
    {
        m_vec2PointRightSprocketHole[i].clear();
    }
    m_vec2PointLeftSprocketHole.clear();
    m_vec2PointRightSprocketHole.clear();

    //각 Sprocket Hole 존재 여부 초기화
    m_isExistLeftSprocketHole = false; // Left Sprocket Hole is exist
    m_isExistRightSprocketHole = false; // Right Sprocket Hole is exist
}

bool SprocketHoleInfo::CollectSprocketHoleInfo(const Ipvm::EllipseEq32r* pEllipseEqAll, const long circleNum,
    const Ipvm::Point32r2* pEdgePointAll, const long pointNum, const Ipvm::Point32r2& centerPoint,
    const Ipvm::Point32r2& scale)
{
    //모든 정보 초기화
    Init();

    //입력 값이 유효한지 검사
    if (circleNum <= 0 || pointNum <= 0 || pEllipseEqAll == nullptr || pEdgePointAll == nullptr)
        return false;

    //Edge Point 정보 수집에 사용할 vector Rect 변수 선언
    std::vector<Ipvm::Rect32s> vecLeftSprocketHole(0);
    std::vector<Ipvm::Rect32s> vecRightSprocketHole(0);
    Ipvm::Rect32s rtSprocketHole;
    long n500um_Pxl = static_cast<long>(500.f / scale.m_x + .5f); // 500um를 Pixel로 변환

    //Sprocket Hole의 좌우를 구분하기 위해 CircleEq 정보를 좌측과 우측으로 분류
    Ipvm::CircleEq32r circleEq;
    for (long i = 0; i < circleNum; i++)
    {
        circleEq.m_x = pEllipseEqAll[i].m_x;
        circleEq.m_y = pEllipseEqAll[i].m_y;
        circleEq.m_radius = pEllipseEqAll[i].m_xradius;
        if (pEllipseEqAll[i].m_x < centerPoint.m_x)
        {
            m_vecCircleEqLeftSprockeHole.push_back(circleEq);
        }
        else
        {
            m_vecCircleEqRightSprockeHole.push_back(circleEq);
        }
    }

    const long nLeftSize = static_cast<long>(m_vecCircleEqLeftSprockeHole.size());
    const long nRightSize = static_cast<long>(m_vecCircleEqRightSprockeHole.size());

    //Sprocket Hole의 좌우 개수가 0인 경우는 존재하지 않음
    //분류한 CircleEq를 높이 순으로 정렬
    if (nLeftSize > 0)
    {
        m_isExistLeftSprocketHole = true;

        //m_vecCircleEqLeftSprockeHole의 Y좌표 기준으로 오름차순으로 정렬
        std::sort(m_vecCircleEqLeftSprockeHole.begin(), m_vecCircleEqLeftSprockeHole.end(),
            [](const Ipvm::CircleEq32r& a, const Ipvm::CircleEq32r& b)
            {
                return a.m_y < b.m_y;
            });

        //Sorting된  CircleEq 정보로 vecLeftSprocketHole 정보 수집
        for (const auto& circle : m_vecCircleEqLeftSprockeHole)
        {
            rtSprocketHole.m_left = static_cast<long>(circle.m_x - circle.m_radius + .5f);
            rtSprocketHole.m_top = static_cast<long>(circle.m_y - circle.m_radius + .5f);
            rtSprocketHole.m_right = static_cast<long>(circle.m_x + circle.m_radius + .5f);
            rtSprocketHole.m_bottom = static_cast<long>(circle.m_y + circle.m_radius + .5f);
            rtSprocketHole.InflateRect(n500um_Pxl, n500um_Pxl); // 500um를 Pixel로 변환한 값을 InflateRect에 사용

            vecLeftSprocketHole.push_back(rtSprocketHole);
        }
    }
    if (m_vecCircleEqRightSprockeHole.size() > 0)
    {
        m_isExistRightSprocketHole = true;

        //m_vecCircleEqRightSprockeHole의 Y좌표 기준으로 오름차순으로 정렬
        std::sort(m_vecCircleEqRightSprockeHole.begin(), m_vecCircleEqRightSprockeHole.end(),
            [](const Ipvm::CircleEq32r& a, const Ipvm::CircleEq32r& b)
            {
                return a.m_y < b.m_y;
            });

        //Sorting된  CircleEq 정보로 vecRightSprocketHole 정보 수집
        for (const auto& circle : m_vecCircleEqRightSprockeHole)
        {
            rtSprocketHole.m_left = static_cast<long>(circle.m_x - circle.m_radius + .5f);
            rtSprocketHole.m_top = static_cast<long>(circle.m_y - circle.m_radius + .5f);
            rtSprocketHole.m_right = static_cast<long>(circle.m_x + circle.m_radius + .5f);
            rtSprocketHole.m_bottom = static_cast<long>(circle.m_y + circle.m_radius + .5f);
            rtSprocketHole.InflateRect(n500um_Pxl, n500um_Pxl); // 500um를 Pixel로 변환한 값을 InflateRect에 사용
            vecRightSprocketHole.push_back(rtSprocketHole);
        }
    }

    //Sprocket Hole 정보의 유효성 판단
    if (m_isExistLeftSprocketHole == false && m_isExistRightSprocketHole == false)
    {
        Init();
        return false; // Sprocket Hole is not exist
    }

    if (m_isExistLeftSprocketHole == true && m_isExistRightSprocketHole == true)
    {
        if (m_vecCircleEqLeftSprockeHole.size() != m_vecCircleEqRightSprockeHole.size())
        {
            Init();
            return false; // Sprocket Hole is not exist
        }
    }

    //pEdgePointAll의 정보를 각 Sprocket Hole에 맞게 분류하여 저장
    m_vec2PointLeftSprocketHole.resize(vecLeftSprocketHole.size());
    m_vec2PointRightSprocketHole.resize(vecRightSprocketHole.size());

    for (long i = 0; i < pointNum; i++)
    {
        if (pEdgePointAll[i].m_x < centerPoint.m_x)
        {
            //좌측 Sprocket Hole Edge Point 정보 수집
            for (long nLeft = 0; nLeft < nLeftSize; nLeft++)
            {
                if (vecLeftSprocketHole[nLeft].PtInRect(pEdgePointAll[i]))
                {
                    m_vec2PointLeftSprocketHole[nLeft].push_back(pEdgePointAll[i]);
                    break;
                }
            }
        }
        else
        {
            //우측 Sprocket Hole Edge Point 정보 수집
            for (long nRight = 0; nRight < nRightSize; nRight++)
            {
                if (vecRightSprocketHole[nRight].PtInRect(pEdgePointAll[i]))
                {
                    m_vec2PointRightSprocketHole[nRight].push_back(pEdgePointAll[i]);
                    break;
                }
            }
        }
    }
    return true;
}