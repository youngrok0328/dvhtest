//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "dPI_GeometryLib.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h" //kircheis_IllumCal

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Quadrangle32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define dPI_MIN 1e-10
#define dPI_MAX 1e10

//CPP_7_________________________________ Implementation body
//
static AFX_EXTENSION_MODULE DPI_GeometryDLL = {NULL, NULL};

extern "C" int APIENTRY DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    // Remove this if you use lpReserved
    UNREFERENCED_PARAMETER(lpReserved);

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        TRACE0("DPI_GEOMETRY.DLL Initializing!\n");

        // Extension DLL one-time initialization
        if (!AfxInitExtensionModule(DPI_GeometryDLL, hInstance))
            return 0;

        // Insert this DLL into the resource chain
        // NOTE: If this Extension DLL is being implicitly linked to by
        //  an MFC Regular DLL (such as an ActiveX Control)
        //  instead of an MFC application, then you will want to
        //  remove this line from DllMain and put it in a separate
        //  function exported from this Extension DLL.  The Regular DLL
        //  that uses this Extension DLL should then explicitly call that
        //  function to initialize this Extension DLL.  Otherwise,
        //  the CDynLinkLibrary object will not be attached to the
        //  Regular DLL's resource chain, and serious problems will
        //  result.

        new CDynLinkLibrary(DPI_GeometryDLL);
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
        TRACE0("DPI_GEOMETRY.DLL Terminating!\n");
        // Terminate the library before destructors are called
        AfxTermExtensionModule(DPI_GeometryDLL);
    }
    return 1; // ok
}

bool CPI_Geometry::LineFitting_RemoveNoise(const std::vector<Ipvm::Point32r2>& i_point, Ipvm::LineEq32r& o_line)
{
    if (i_point.size() <= 0)
        return false;

    if (Ipvm::DataFitting::FitToLineRn(long(i_point.size()), &i_point[0], 3.f, o_line) != Ipvm::Status::e_ok)
    {
        return false;
    }

    return true;
}

bool CPI_Geometry::LineFitting_RemoveNoise(const std::vector<Ipvm::Point32r2>& i_point,
    std::vector<Ipvm::Point32r2>& o_vecfptData, Ipvm::LineEq32r& o_line, float fSigma)
{
    if (i_point.size() == 0)
        return false;

    o_vecfptData.resize(i_point.size());

    int32_t useDataCount = 0;
    if (Ipvm::DataFitting::FitToLineRn(
            long(i_point.size()), &i_point[0], fSigma, o_line, &o_vecfptData[0], &useDataCount)
        != Ipvm::Status::e_ok)
    {
        o_vecfptData.clear();
        return false;
    }

    o_vecfptData.resize(useDataCount);

    return true;
}

// 영훈 : 기울기가 있는 직선에서 원하는 거리만큼 뺀 포인트를 반환시켜준다.
Ipvm::Point32r2 CPI_Geometry::GetLinePoint(Ipvm::Point32r2 fptStart, Ipvm::Point32r2 fptEnd, float fDist)
{
    Ipvm::Point32r2 fptData;
    fptData.m_x = fptStart.m_x - fDist;
    fptData.m_y = fptStart.m_y;

    float fAngle = GetAngleBetween2Lines(fptEnd, fptStart, fptData) * ITP_DEG_TO_RAD;

    if (fptStart.m_y < fptEnd.m_y)
        fAngle *= -1;

    RotatePoint(fptData, fAngle, fptStart);

    return fptData;
}

/////////////////////////////////////////////////////////////////////////////
// bVer == y방향으로  fdist 만큼 떨어진 Line 상의 좌표 구하기
// 기준점 : x0, y0
// Line 식 : a1, b1, c1
long CPI_Geometry::GetDistPointOnLine(
    float x0, float y0, float a1, float b1, float fdist, BOOL bVer, float& fx, float& fy)
{
    double det = sqrt(a1 * a1 + b1 * b1);
    double fcos = b1 / det;
    double fsin = -a1 / det;

    float fdelx = (float)(fdist * fcos);
    float fdely = (float)(fdist * fsin);

    float fsign;

    if (bVer)
        fsign = (float)fsin;
    else
        fsign = (float)fcos;

    if (fsign > 0.f)
    {
        fx = x0 + fdelx;
        fy = y0 + fdely;
    }
    else
    { // + -
        fx = x0 - fdelx;
        fy = y0 - fdely;
    }

    return 0;
}

float CPI_Geometry::GetDistance_PointToPoint(float x0, float y0, float x1, float y1)
{
    double dx = x0 - x1;
    double dy = y0 - y1;

    return float(sqrt(dx * dx + dy * dy));
}

/////////////////////////////////////////////////////////////////////////////////
float CPI_Geometry::GetDistance_PointToLine(float x0, float y0, const Ipvm::LineEq32r& line)
{
    double det = sqrt(line.m_a * line.m_a + line.m_b * line.m_b);

    if (det < dPI_MIN)
        return -1.f;

    return (float)((fabs(line.m_a * x0 + line.m_b * y0 + line.m_c)) / det);
}

#define PI 3.1415926535f

void CPI_Geometry::Get1LineAngle(float x0, float y0, float x1, float y1, float& o_angle)
{
    // 2점을 가지고 각도를 구한다
    //

    float a = (x1 - x0) / (y1 - y0);

    o_angle = (float)(atan(a) / PI * 180);
}

float CPI_Geometry::Get1LineAngleDeg(float fOriginX, float fOriginY, float fDestX, float fDestY)
{
    // fOrigin을 원점으로 fDest와의 각도를 구한다. 기준은 X축.
    float fDistX = fDestX - fOriginX;
    float fDistY = fDestY - fOriginY;

    float fDist = (float)sqrt(fDistX * fDistX + fDistY * fDistY);
    float a = CAST_FLOAT(fabs(fDistX) / fabs(fDist));
    float fAngle = (float)fabs(acos(a) / PI * 180);

    if (fDistX < 0.f && fDistY >= 0.f) //2사분면
        fAngle = 180.f - fAngle;
    else if (fDistX < 0.f && fDistY < 0.f) //3사분면
        fAngle -= 180.f;
    else if (fDistX >= 0.f && fDistY < 0.f) //4사분면
        fAngle *= -1.f;

    return fAngle;
}

float CPI_Geometry::Get1LineAngleRad(float fOriginX, float fOriginY, float fDestX, float fDestY)
{
    // fOrigin을 원점으로 fDest와의 각도를 구한다. 기준은 X축.
    float fDistX = fDestX - fOriginX;
    float fDistY = fDestY - fOriginY;

    float fDist = (float)sqrt(fDistX * fDistX + fDistY * fDistY);
    float a = CAST_FLOAT(fabs(fDistX) / fabs(fDist));
    float fAngle = (float)fabs(acos(a));

    if (fDistX < 0.f && fDistY >= 0.f) //2사분면
        fAngle = PI - fAngle;
    else if (fDistX < 0.f && fDistY < 0.f) //3사분면
        fAngle -= PI;
    else if (fDistX >= 0.f && fDistY < 0.f) //4사분면
        fAngle *= -1.f;

    return fAngle;
}

/////////////////////////////////////////////////////////////////////////////
// 2. Circle Fitting
//
//		Center coordinate of the circle : 	(Xc, Yc)
//		Radius							:	r
//
/////////////////////////////////////////////////////////////////////////////
long CPI_Geometry::mMakeLineByTwoPoints(Ipvm::LineEq32r& o_line, float x1, float y1, float x2, float y2)
{
    double det = x1 * y2 - x2 * y1;

    if (fabs(det) < dPI_MIN)
        return -1;

    o_line.m_a = (float)((y2 - y1) / det);
    o_line.m_b = (float)((x1 - x2) / det);
    o_line.m_c = -1.f;

    return 0;
}

float CPI_Geometry::CrossProductz(float x1, float y1, float x2, float y2)
{
    return x1 * y2 - x2 * y1;
}

float CPI_Geometry::InnerProduct(float x1, float y1, float x2, float y2)
{
    return x1 * x2 + y1 * y2;
}

float CPI_Geometry::Magnitude(float x, float y)
{
    return CAST_FLOAT(sqrt(x * x + y * y));
}

float CPI_Geometry::GetAngleBetween2Lines(
    const Ipvm::Point32r2& pt1, const Ipvm::Point32r2& pt2, const Ipvm::Point32r2& pt3)
{
    float fip = InnerProduct(pt1.m_x - pt2.m_x, pt1.m_y - pt2.m_y, pt3.m_x - pt2.m_x, pt3.m_y - pt2.m_y);
    float fMagnitude
        = (Magnitude(pt1.m_x - pt2.m_x, pt1.m_y - pt2.m_y) * Magnitude(pt3.m_x - pt2.m_x, pt3.m_y - pt2.m_y));

    if (fMagnitude == 0.f)
        return -99999.f;
    float fcos = fip / fMagnitude;
    return CAST_FLOAT(acos(fcos) * 180 / PI);
}

BOOL CPI_Geometry::bPtIsInsidePolygon(
    const Ipvm::Point32s2* pPt, const short nPolygonType, const Ipvm::Point32s2& ptCur)
{
    float dx1, dy1, dx2, dy2;
    float fz;
    for (int i = 0; i < nPolygonType; i++)
    {
        dx1 = (float)(pPt[i].m_x - ptCur.m_x);
        dy1 = (float)(pPt[i].m_y - ptCur.m_y);

        dx2 = (float)(pPt[(i + 1) % nPolygonType].m_x - ptCur.m_x);
        dy2 = (float)(pPt[(i + 1) % nPolygonType].m_y - ptCur.m_y);

        fz = CrossProductz(dx1, dy1, dx2, dy2);
        if (fz > 0.f)
            return FALSE;
    }

    return TRUE;
}

void CPI_Geometry::RotatePoint(Ipvm::Point32r2& io_xy, float fangle, const Ipvm::Point32s2& i_ptOrign)
{
    float fX1 = io_xy.m_x - i_ptOrign.m_x;
    float fY1 = io_xy.m_y - i_ptOrign.m_y;

    io_xy.m_x = CAST_FLOAT(cos(fangle) * fX1 - sin(fangle) * fY1 + i_ptOrign.m_x);
    io_xy.m_y = CAST_FLOAT(sin(fangle) * fX1 + cos(fangle) * fY1 + i_ptOrign.m_y);
}

void CPI_Geometry::RotatePoint(Ipvm::Point32r2& io_xy, float fangle, const Ipvm::Point32r2& i_ptOrign)
{
    float fX1 = io_xy.m_x - i_ptOrign.m_x;
    float fY1 = io_xy.m_y - i_ptOrign.m_y;

    io_xy.m_x = CAST_FLOAT(cos(fangle) * fX1 - sin(fangle) * fY1 + i_ptOrign.m_x);
    io_xy.m_y = CAST_FLOAT(sin(fangle) * fX1 + cos(fangle) * fY1 + i_ptOrign.m_y);
}

void CPI_Geometry::RotateRect(Ipvm::Rect32s& i_rtSrcDst, float i_fangle, const Ipvm::Point32s2& i_ptOrign)
{
    long fX1 = i_rtSrcDst.m_left - i_ptOrign.m_x;
    long fY1 = i_rtSrcDst.m_top - i_ptOrign.m_y;

    long fX2 = i_rtSrcDst.m_right - i_ptOrign.m_x;
    long fY2 = i_rtSrcDst.m_bottom - i_ptOrign.m_y;

    i_rtSrcDst.m_left = CAST_INT32T((cos(i_fangle) * fX1 - sin(i_fangle) * fY1 + i_ptOrign.m_x) + .5f);
    i_rtSrcDst.m_top = CAST_INT32T((sin(i_fangle) * fX1 + cos(i_fangle) * fY1 + i_ptOrign.m_y) + .5f);

    i_rtSrcDst.m_right = CAST_INT32T((cos(i_fangle) * fX2 - sin(i_fangle) * fY2 + i_ptOrign.m_x) + .5f);
    i_rtSrcDst.m_bottom = CAST_INT32T((sin(i_fangle) * fX2 + cos(i_fangle) * fY2 + i_ptOrign.m_y) + .5f);
}

void CPI_Geometry::RotateRectToPI_RECT(Ipvm::Rect32s i_rtSrc, float i_fAngle, const Ipvm::Point32r2& i_ptOrign,
    PI_RECT& o_pirtDest) //Rect를 돌린 사각형으로 출력
{
    float fPtX, fPtY;
    float fSin = (float)sin(i_fAngle);
    float fCos = (float)cos(i_fAngle);

    //LeftTop
    fPtX = (float)i_rtSrc.m_left - i_ptOrign.m_x;
    fPtY = (float)i_rtSrc.m_top - i_ptOrign.m_y;
    o_pirtDest.ltX = (long)(fCos * fPtX - fSin * fPtY + i_ptOrign.m_x + .5f);
    o_pirtDest.ltY = (long)(fSin * fPtX + fCos * fPtY + i_ptOrign.m_y + .5f);

    //LeftBottom
    fPtX = (float)i_rtSrc.m_left - i_ptOrign.m_x;
    fPtY = (float)i_rtSrc.m_bottom - i_ptOrign.m_y;
    o_pirtDest.blX = (long)(fCos * fPtX - fSin * fPtY + i_ptOrign.m_x + .5f);
    o_pirtDest.blY = (long)(fSin * fPtX + fCos * fPtY + i_ptOrign.m_y + .5f);

    //RightTop
    fPtX = (float)i_rtSrc.m_right - i_ptOrign.m_x;
    fPtY = (float)i_rtSrc.m_top - i_ptOrign.m_y;
    o_pirtDest.rtX = (long)(fCos * fPtX - fSin * fPtY + i_ptOrign.m_x + .5f);
    o_pirtDest.rtY = (long)(fSin * fPtX + fCos * fPtY + i_ptOrign.m_y + .5f);

    //RighBottom
    fPtX = (float)i_rtSrc.m_right - i_ptOrign.m_x;
    fPtY = (float)i_rtSrc.m_bottom - i_ptOrign.m_y;
    o_pirtDest.brX = (long)(fCos * fPtX - fSin * fPtY + i_ptOrign.m_x + .5f);
    o_pirtDest.brY = (long)(fSin * fPtX + fCos * fPtY + i_ptOrign.m_y + .5f);
}

void CPI_Geometry::RotateQuadRect(Ipvm::Quadrangle32r i_qrtSrc, float i_fAngle, const Ipvm::Point32r2& i_ptOrign,
    Ipvm::Quadrangle32r& o_qrtDest) //kircheis_MED3
{
    float fPtX, fPtY;
    const float fSin = (float)sin(i_fAngle);
    const float fCos = (float)cos(i_fAngle);

    //LeftTop
    fPtX = i_qrtSrc.m_ltX - i_ptOrign.m_x;
    fPtY = i_qrtSrc.m_ltY - i_ptOrign.m_y;
    o_qrtDest.m_ltX = (fCos * fPtX - fSin * fPtY + i_ptOrign.m_x);
    o_qrtDest.m_ltY = (fSin * fPtX + fCos * fPtY + i_ptOrign.m_y);

    //LeftBottom
    fPtX = i_qrtSrc.m_lbX - i_ptOrign.m_x;
    fPtY = i_qrtSrc.m_lbY - i_ptOrign.m_y;
    o_qrtDest.m_lbX = (fCos * fPtX - fSin * fPtY + i_ptOrign.m_x);
    o_qrtDest.m_lbY = (fSin * fPtX + fCos * fPtY + i_ptOrign.m_y);

    //RightTop
    fPtX = i_qrtSrc.m_rtX - i_ptOrign.m_x;
    fPtY = i_qrtSrc.m_rtY - i_ptOrign.m_y;
    o_qrtDest.m_rtX = (fCos * fPtX - fSin * fPtY + i_ptOrign.m_x);
    o_qrtDest.m_rtY = (fSin * fPtX + fCos * fPtY + i_ptOrign.m_y);

    //RighBottom
    fPtX = i_qrtSrc.m_rbX - i_ptOrign.m_x;
    fPtY = i_qrtSrc.m_rbY - i_ptOrign.m_y;
    o_qrtDest.m_rbX = (fCos * fPtX - fSin * fPtY + i_ptOrign.m_x);
    o_qrtDest.m_rbY = (fSin * fPtX + fCos * fPtY + i_ptOrign.m_y);
}
/////////////////////////////////////////////////////////////////////////////
// ax + by + c = 0 으로 Fitting 한다.
bool CPI_Geometry::RoughLineFitting(long i_datanum, const Ipvm::Point32r2* i_points, Ipvm::LineEq32r& o_line,
    long i_nModelSampleingNum, float i_fLimitDistance, std::vector<Ipvm::Point32r2>& o_vecfFilteredXY, BOOL bUseVerify)
{
    /// 최소 Modeling을 위해서는 2개 이상의 데이터 필요.
    if (i_nModelSampleingNum < 2)
        i_nModelSampleingNum = 2;

    if (i_datanum < 5 || i_datanum < i_nModelSampleingNum)
    {
        // 최소 5개이상의 데이터에서만 돌리자.
        // 뽑을 샘플수보다 데이터가 적으면 진행할 수 없다

        return false;
    }

    std::vector<Ipvm::Point32r2> vecfSampleXY(i_nModelSampleingNum);

    int nMaxInteration = (int)(1 + log(1. - 0.99) / log(1. - pow(0.5, i_nModelSampleingNum)));
    std::vector<double> vecdScore;
    std::vector<Ipvm::LineEq32r> vecModelLine;

    srand(i_datanum);

    for (int nIterCnt = 0; nIterCnt < nMaxInteration; nIterCnt++)
    {
        for (int i = 0; i < i_nModelSampleingNum; i++)
        {
            vecfSampleXY[i].m_x = -1.0f;
            vecfSampleXY[i].m_y = -1.0f;
        }

        /// 1) 임의의 Sample 취합::데이터에서 중복되지 않게 N개의 무작위 셈플을 채취한다
        int nSampleCnt = 0;

        do
        {
            // 임의데이터 선택.
            int nDataIndex = rand() % i_datanum;
            auto selected = i_points[nDataIndex];

            // Sample로 이미 선택된것이 있는지 확인.
            BOOL bSampleAlreadyExist = FALSE;
            for (int j = 0; j < nSampleCnt; j++)
            {
                if (vecfSampleXY[nSampleCnt] == selected)
                {
                    bSampleAlreadyExist = TRUE;
                }
            }

            // 등록된 Sample에 같은 것이 없으면 Sample로 등록
            if (!bSampleAlreadyExist)
            {
                vecfSampleXY[nSampleCnt] = selected;
                nSampleCnt++;
            }
        }
        while (nSampleCnt != i_nModelSampleingNum);

        ////===============Sample을 이용한 model을 구하고, Score를 맥이자.=====================//
        /// Samle을 이용하여, Line을 구한다.
        Ipvm::LineEq32r sLineTemp;
        Ipvm::DataFitting::FitToLine(i_nModelSampleingNum, &vecfSampleXY[0], sLineTemp);

        double dScoreSum = 0.0f;
        for (long i = 0; i < i_datanum; i++)
        {
            float distance = 0.f;
            if (Ipvm::Geometry::GetDistance(sLineTemp, i_points[i], distance) != Ipvm::Status::e_ok)
                continue;

            if (fabs(distance) < i_fLimitDistance)
            {
                dScoreSum = dScoreSum + 1;
            }
        }
        vecdScore.push_back(dScoreSum);
        vecModelLine.push_back(sLineTemp);
    }

    //// 최단거리를 찾는다.
    double dMaxScore = -100000000000.f;
    long nMaxIndex = 0;

    for (long i = 0; i < (long)vecdScore.size(); i++)
    {
        if (dMaxScore < vecdScore[i])
        {
            nMaxIndex = i;
            o_line = vecModelLine[i];
            dMaxScore = vecdScore[i];
        }
    }

    o_vecfFilteredXY.clear();
    for (long i = 0; i < i_datanum; i++)
    {
        float distance = 0.f;
        if (Ipvm::Geometry::GetDistance(o_line, i_points[i], distance) != Ipvm::Status::e_ok)
            continue;

        if (fabs(distance) < i_fLimitDistance)
        {
            o_vecfFilteredXY.emplace_back(i_points[i]);
        }
    }

    if (o_vecfFilteredXY.size() < 5)
    {
        return false;
    }

    if (bUseVerify)
    {
        if (Ipvm::DataFitting::FitToLineRn((long)o_vecfFilteredXY.size(), &o_vecfFilteredXY[0], 3.f, o_line)
            != Ipvm::Status::e_ok)
        {
            return false;
        }
    }

    return true;
}

BOOL CPI_Geometry::Get2DCurveFitting(
    const std::vector<Ipvm::Point32r2>& i_vecfPoint, Curve2DEq& o_Curve2DEq) //kircheis_Round
{
    long nDataNum = (long)i_vecfPoint.size();
    o_Curve2DEq.m_a = o_Curve2DEq.m_b = o_Curve2DEq.m_c = -1.;

    if (nDataNum < 3)
        return FALSE;

    double coef[3] = {0., 0., 0.};

    if (Ipvm::Status::e_ok != Ipvm::DataFitting::FitTo2ndOrderPolynomial(nDataNum, &i_vecfPoint[0], coef))
    {
        return FALSE;
    }

    o_Curve2DEq.m_a = coef[2];
    o_Curve2DEq.m_b = coef[1];
    o_Curve2DEq.m_c = coef[0];

    return TRUE;

    /*
	double AXM[3*3], BXM[3]; // matrix 변수
	memset(AXM, 0, sizeof(double)*9);
	memset(BXM, 0, sizeof(double)*3);

	for(int i=0; i<nDataNum; i++)
	{
		double dX= (double)i_vecfPoint[i].m_x;
		double dX2 = dX*dX;
		double dX3 = dX2*dX;
		double dX4 = dX3*dX;
		//double dX5 = dX4*dX;

		double dY = i_vecfPoint[i].m_y;
		//double dY2 = dY*dY;

		AXM[0*3+0] = nDataNum;	AXM[1*3+0] += dX;		AXM[2*3+0] += dX2;
		AXM[0*3+1] += dX ;		AXM[1*3+1] += dX2;		AXM[2*3+1] += dX3;
		AXM[0*3+2] += dX2;		AXM[1*3+2] += dX3;		AXM[2*3+2] += dX4;

		BXM[0] += dY;
		BXM[1] += dY*dX;
		BXM[2] += dY*dX2;
	}

	if(!inverse_matrix2(AXM,3))
		return FALSE;

	o_Curve2DEq.m_a = AXM[0*3+0]*BXM[0] + AXM[0*3+1]*BXM[1] + AXM[0*3+2]*BXM[2] ;
	o_Curve2DEq.m_b = AXM[1*3+0]*BXM[0] + AXM[1*3+1]*BXM[1] + AXM[1*3+2]*BXM[2] ;
	o_Curve2DEq.m_c = AXM[2*3+0]*BXM[0] + AXM[2*3+1]*BXM[1] + AXM[2*3+2]*BXM[2] ;

	return TRUE;
*/
}

/*
#define INVERSE_MAXDIMENSION 10
BOOL CPI_Geometry::inverse_matrix2(double *unknown, int dimension)
{
	int 	i , j , k , dd = 2*dimension ;
	double  pivot , arquement[INVERSE_MAXDIMENSION*INVERSE_MAXDIMENSION*2];
	double 	tolelence=0.000001f;

	for(i=0;i<dimension;i++)
	{
		for(j=0;j<dimension;j++)
		{
			arquement[i*dd+j] = unknown[i*dimension+j] ;
			arquement[i*dd+(j+dimension)] = (i==j) ? 1.0f : 0.0f ;	   	// 확대계수행렬을 구성한다.         
		}
	}

	for( k=0 ; k< dimension ; k++ )
	{
		for( i=k+1 ; i < dimension ; i++ )   							// Pivoting process
		{
			if( fabs(arquement[k*dd+k]/10000.0) < fabs(arquement[i*dd+k]/10000.0) )
			{
				for(j=0;j<dd;j++)
				{
					pivot = arquement[i*dd+j] ;
					arquement[i*dd+j] = arquement[k*dd+j] ;
					arquement[k*dd+j] = pivot ;
				}
			}
		}

		if(fabs(arquement[k*dd+k]) < tolelence ) return(FALSE) ;   		// matrix is singular.

		for(i=0;i<dimension;i++)                                        // Gauss elimination
		{
			if(i==k)
			{
				pivot = 1.0f/arquement[k*dd+k] ;
				for(j=0;j<dd;j++)  arquement[i*dd+j]=arquement[i*dd+j]*pivot ;
			}
			else
			{
				pivot = arquement[i*dd+k] / arquement[k*dd+k] ;
				for(j=0;j<dd;j++)  arquement[i*dd+j]=arquement[i*dd+j]-arquement[k*dd+j]*pivot ;
			}
		}
	}

	for(i=0;i<dimension;i++)
	{
		for(j=0;j<dimension;j++)
		{
			unknown[i*dimension+j] = arquement[i*dd+(j+dimension)] ;
		}
	}  
	return(TRUE);
}

*/

void CPI_Geometry::VerifyPolygonInnerAngle(
    float& io_fAngle_Deg) //kircheis_LandMissing //다각형의 내각은 0 < angle_deg <= 180의 범위안에 집에 넣어야 한다.
{
    float i_fAngle = (float)fabs(io_fAngle_Deg); //일단 양수화

    do
    {
        if (i_fAngle > 360.f)
            i_fAngle -= 360.f;
        else if (i_fAngle > 180.f)
            i_fAngle = 360.f - i_fAngle;
    }
    while (i_fAngle > 180.f);
    io_fAngle_Deg = i_fAngle;
}

void CPI_Geometry::GetRectSizeX(
    const PI_RECT& i_srtPackageBody, const bool& i_refLeft, float& o_minSizeX, float& o_maxSizeX, float& o_avrSizeX)
{
    std::vector<Ipvm::Point32r2> vecfptLine(2);
    std::vector<Ipvm::Point32r2> vecfptDst(2);
    Ipvm::LineEq32r lineEqRef;
    float distTop(0.f), distBottom(0.f);
    if (i_refLeft == true)
    {
        vecfptLine[0] = Ipvm::Point32r2((float)i_srtPackageBody.ltX, (float)i_srtPackageBody.ltY);
        vecfptLine[1] = Ipvm::Point32r2((float)i_srtPackageBody.blX, (float)i_srtPackageBody.blY);

        vecfptDst[0] = Ipvm::Point32r2((float)i_srtPackageBody.rtX, (float)i_srtPackageBody.rtY);
        vecfptDst[1] = Ipvm::Point32r2((float)i_srtPackageBody.brX, (float)i_srtPackageBody.brY);
    }
    else
    {
        vecfptLine[0] = Ipvm::Point32r2((float)i_srtPackageBody.rtX, (float)i_srtPackageBody.rtY);
        vecfptLine[1] = Ipvm::Point32r2((float)i_srtPackageBody.brX, (float)i_srtPackageBody.brY);

        vecfptDst[0] = Ipvm::Point32r2((float)i_srtPackageBody.ltX, (float)i_srtPackageBody.ltY);
        vecfptDst[1] = Ipvm::Point32r2((float)i_srtPackageBody.blX, (float)i_srtPackageBody.blY);
    }

    Ipvm::DataFitting::FitToLine(2, &vecfptLine[0], lineEqRef);
    Ipvm::Geometry::GetDistance(lineEqRef, vecfptDst[0], distTop);
    Ipvm::Geometry::GetDistance(lineEqRef, vecfptDst[1], distBottom);

    o_avrSizeX = (distTop + distBottom) * .5f;
    o_minSizeX = (float)min(distTop, distBottom);
    o_maxSizeX = (float)max(distTop, distBottom);
}

void CPI_Geometry::GetRectSizeY(
    const PI_RECT& i_srtPackageBody, const bool& i_refTop, float& o_minSizeY, float& o_maxSizeY, float& o_avrSizeY)
{
    std::vector<Ipvm::Point32r2> vecfptLine(2);
    std::vector<Ipvm::Point32r2> vecfptDst(2);
    Ipvm::LineEq32r lineEqRef;
    float distTop(0.f), distBottom(0.f);
    if (i_refTop == true)
    {
        vecfptLine[0] = Ipvm::Point32r2((float)i_srtPackageBody.ltX, (float)i_srtPackageBody.ltY);
        vecfptLine[1] = Ipvm::Point32r2((float)i_srtPackageBody.rtX, (float)i_srtPackageBody.rtY);

        vecfptDst[0] = Ipvm::Point32r2((float)i_srtPackageBody.blX, (float)i_srtPackageBody.blY);
        vecfptDst[1] = Ipvm::Point32r2((float)i_srtPackageBody.brX, (float)i_srtPackageBody.brY);
    }
    else
    {
        vecfptLine[0] = Ipvm::Point32r2((float)i_srtPackageBody.blX, (float)i_srtPackageBody.blY);
        vecfptLine[1] = Ipvm::Point32r2((float)i_srtPackageBody.brX, (float)i_srtPackageBody.brY);

        vecfptDst[0] = Ipvm::Point32r2((float)i_srtPackageBody.ltX, (float)i_srtPackageBody.ltY);
        vecfptDst[1] = Ipvm::Point32r2((float)i_srtPackageBody.rtX, (float)i_srtPackageBody.rtY);
    }

    Ipvm::DataFitting::FitToLine(2, &vecfptLine[0], lineEqRef);
    Ipvm::Geometry::GetDistance(lineEqRef, vecfptDst[0], distTop);
    Ipvm::Geometry::GetDistance(lineEqRef, vecfptDst[1], distBottom);

    o_avrSizeY = (distTop + distBottom) * .5f;
    o_minSizeY = (float)min(distTop, distBottom);
    o_maxSizeY = (float)max(distTop, distBottom);
}