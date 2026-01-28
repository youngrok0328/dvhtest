#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
struct Curve2DEq; //a + bX + cX^2 = Y
struct PI_RECT;
struct FPI_RECT; //kircheis_EdgePeeling

//HDR_6_________________________________ Header body
//
class __DPI_GEOMETRY_API__ CPI_Geometry
{
public:
    // Line Fitting
    static bool LineFitting_RemoveNoise(const std::vector<Ipvm::Point32r2>& i_point, Ipvm::LineEq32r& o_line);
    static bool LineFitting_RemoveNoise(const std::vector<Ipvm::Point32r2>& i_point,
        std::vector<Ipvm::Point32r2>& o_vecfptData, Ipvm::LineEq32r& o_line, float fSigma = 1.f);
    static long mMakeLineByTwoPoints(Ipvm::LineEq32r& o_line, float x1, float y1, float x2, float y2);

    /// 이태용 :: RANSAC_LineFitting
    static bool RoughLineFitting(long i_datanum, const Ipvm::Point32r2* i_points, Ipvm::LineEq32r& o_line,
        long i_nModelSampleingNum, float i_fLimitDistance, std::vector<Ipvm::Point32r2>& o_vecfFilteredXY,
        BOOL bUseVerify = FALSE);

    static long GetDistPointOnLine(
        float x0, float y0, float a1, float b1, float fdist, BOOL bVer, float& fx, float& fy);
    static float GetDistance_PointToPoint(float x0, float y0, float x1, float y1);
    static float GetDistance_PointToLine(float x0, float y0, const Ipvm::LineEq32r& line);

    static void Get1LineAngle(
        float x0, float y0, float x1, float y1, float& o_angle); // 2점을 가지고 그사이의 각도를 구한다
    static float Get1LineAngleDeg(float fOriginX, float fOriginY, float fDestX,
        float fDestY); // fOrigin을 원점으로 fDest와의 각도를 구한다. 기준은 X축.
    static float Get1LineAngleRad(float fOriginX, float fOriginY, float fDestX,
        float fDestY); // fOrigin을 원점으로 fDest와의 각도를 구한다. 기준은 X축.
    static float GetAngleBetween2Lines(
        const Ipvm::Point32r2& pt1, const Ipvm::Point32r2& pt2, const Ipvm::Point32r2& pt3);
    static BOOL bPtIsInsidePolygon(const Ipvm::Point32s2* pPt, const short nPolygonType, const Ipvm::Point32s2& ptCur);
    static void RotateRect(Ipvm::Rect32s& i_rtSrcDst, float i_fAngle, const Ipvm::Point32s2& i_ptOrign);
    static void RotateRectToPI_RECT(Ipvm::Rect32s i_rtSrc, float i_fAngle, const Ipvm::Point32r2& i_ptOrign,
        PI_RECT& o_pirtDest); //Rect를 돌린 사각형으로 출력
    static void RotateQuadRect(Ipvm::Quadrangle32r i_qrtSrc, float i_fAngle, const Ipvm::Point32r2& i_ptOrign,
        Ipvm::Quadrangle32r& o_qrtDest); //kircheis_MED3//Rect를 돌린 사각형으로 출력
    static void RotatePoint(Ipvm::Point32r2& io_xy, float fangle, const Ipvm::Point32s2& i_ptOrign);
    static void RotatePoint(Ipvm::Point32r2& io_xy, float fangle, const Ipvm::Point32r2& i_ptOrign);
    static BOOL Get2DCurveFitting(
        const std::vector<Ipvm::Point32r2>& i_vecfPoint, Curve2DEq& o_Curve2DEq); //kircheis_Round
    static void VerifyPolygonInnerAngle(float&
            io_fAngle_Deg); //kircheis_LandMissing //다각형의 내각은 0 < angle_deg <= 180의 범위안에 집에 넣어야 한다.

    static void GetRectSizeX(const PI_RECT& i_srtPackageBody, const bool& i_refLeft, float& o_minSizeX,
        float& o_maxSizeX, float& o_avrSizeX);
    static void GetRectSizeY(
        const PI_RECT& i_srtPackageBody, const bool& i_refTop, float& o_minSizeY, float& o_maxSizeY, float& o_avrSizeY);

private:
    static float CrossProductz(float x1, float y1, float x2, float y2);
    static float InnerProduct(float x1, float y1, float x2, float y2);
    static float Magnitude(float x, float y);
    //	static BOOL inverse_matrix2(double *unknown, int dimension);

public:
    // 영훈 : 기울기가 있는 직선에서 원하는 거리만큼 뺀 포인트를 반환시켜준다.
    static Ipvm::Point32r2 GetLinePoint(Ipvm::Point32r2 fptStart, Ipvm::Point32r2 fptEnd, float fDist);
};
