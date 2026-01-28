//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ShapeAlgorithm.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
#include <Ipvm/Algorithm/DataFitting.h>
#else
#include "Algorithm/DataFitting.h"
#endif

//CPP_5_________________________________ Standard library headers
#include <cmath>
#include <numeric>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
// Z = A + Bx + Cy + Dxy + Ex^2 + Fy^2
struct ShapeCoefficients
{
    double m_a0;
    double m_a1;
    double m_a2;
    double m_a3;
    double m_a4;
    double m_a5;
};

const double _PI = 3.141592653589793;

ShapeAlgorithm::ShapeAlgorithm(void)
    : m_normalizedB4(0.)
    , m_normalizedB5(0.)
    , m_coeffThreshold(0.8)
    , m_insignificantThreshold(10.)
{
}

ShapeAlgorithm::~ShapeAlgorithm(void)
{
}

bool ShapeAlgorithm::GetShape(const std::vector<Ipvm::Point64r3>& points, CString& sign, CString& shape, double& r,
    double& b4_um, double& b5_um, double& angle_deg)
{
    m_normalizedB4 = 0.;
    m_normalizedB5 = 0.;
    sign.Empty();
    shape.Empty();
    r = 0.;
    b4_um = 0.;
    b5_um = 0.;
    angle_deg = 0.;

    if (points.size() < 6)
    {
        return false;
    }

    // 적절한 데이터가 없는 녀석들을 솎아내야 함.
    RefineInputData(points, m_inputPoints);

    if (m_inputPoints.size() < 6)
    {
        return false;
    }

    // 이현민 : 좌표계는 Die Center 기준으로 되어 있어야 함..
    AdjustCoordinateXY(m_inputPoints);

    m_fittedPoints = m_inputPoints;

    //Fitting
    ShapeCoefficients shapeCoef;

    FitC4Area(m_inputPoints, shapeCoef);

    GetFittingSurface(shapeCoef, m_fittedPoints);

    r = GetNormalizedCorrelationCoeffR(m_inputPoints, m_fittedPoints);

    FindB4B4Theta(shapeCoef, angle_deg, b4_um, b5_um);

    const double calculateK = FindNormalized(m_inputPoints, b4_um, b5_um, angle_deg, m_normalizedB4, m_normalizedB5);

    Report(m_coeffThreshold, m_insignificantThreshold, r, m_normalizedB4, m_normalizedB5, calculateK, sign, shape);

    return true;
}

double ShapeAlgorithm::GetNormalizedB4() const
{
    return m_normalizedB4;
}

double ShapeAlgorithm::GetNormalizedB5() const
{
    return m_normalizedB5;
}

void ShapeAlgorithm::RefineInputData(const std::vector<Ipvm::Point64r3>& input, std::vector<Ipvm::Point64r3>& output)
{
    output.clear();
    output.reserve(input.size());

    for (const auto& pt : input)
    {
        if (pt.m_x == 0. && pt.m_y == 0. && pt.m_z == 0.)
        {
            continue;
        }

        output.push_back(pt);
    }

    return;
}

void ShapeAlgorithm::AdjustCoordinateXY(std::vector<Ipvm::Point64r3>& points)
{
    double xMax = std::numeric_limits<double>::lowest();
    double xMin = -xMax;
    double yMax = xMax;
    double yMin = xMin;

    const size_t length = points.size();
    auto* pointsMem = points.data();

    for (size_t idx = 0; idx < length; idx++)
    {
        if (pointsMem[idx].m_x < xMin)
            xMin = pointsMem[idx].m_x;
        if (pointsMem[idx].m_x > xMax)
            xMax = pointsMem[idx].m_x;
        if (pointsMem[idx].m_y < yMin)
            yMin = pointsMem[idx].m_y;
        if (pointsMem[idx].m_y > yMax)
            yMax = pointsMem[idx].m_y;
    }

    const double xCenter = (xMin + xMax) * 0.5;
    const double yCenter = (yMin + yMax) * 0.5;

    for (size_t idx = 0; idx < length; idx++)
    {
        pointsMem[idx].m_x -= xCenter;
        pointsMem[idx].m_y -= yCenter;
    }

    return;
}

void ShapeAlgorithm::FitC4Area(const std::vector<Ipvm::Point64r3>& points, ShapeCoefficients& result)
{
    // Z = Ax^2 + Bxy + Cy^2 + Dx + Ey + F
    double surfaceCoef[6] = {
        0.,
    };

    Ipvm::DataFitting::FitTo2ndOrderPolynomial3DSurface(static_cast<int32_t>(points.size()), &points[0], surfaceCoef);

    // Z = A + Bx + Cy + Dxy + Ex^2 + Fy^2
    result.m_a0 = surfaceCoef[5];
    result.m_a1 = surfaceCoef[3];
    result.m_a2 = surfaceCoef[4];
    result.m_a3 = surfaceCoef[1];
    result.m_a4 = surfaceCoef[0];
    result.m_a5 = surfaceCoef[2];

    return;
}

void ShapeAlgorithm::GetFittingSurface(const ShapeCoefficients& result, std::vector<Ipvm::Point64r3>& points)
{
    const size_t length = points.size();
    auto* pointsMem = points.data();

    for (size_t i = 0; i < length; i++)
    {
        const double x = pointsMem[i].m_x;
        const double y = pointsMem[i].m_y;

        pointsMem[i].m_z = result.m_a0 + (result.m_a1 * x) + (result.m_a2 * y) + (result.m_a3 * x * y)
            + (result.m_a4 * x * x) + (result.m_a5 * y * y);
    }
}

double ShapeAlgorithm::GetNormalizedCorrelationCoeffR(
    const std::vector<Ipvm::Point64r3>& inputPoints, const std::vector<Ipvm::Point64r3>& fittedPoints)
{
    const size_t length = inputPoints.size();
    const auto* inputPointsMem = inputPoints.data();
    const auto* fittedPointsMem = fittedPoints.data();

    double sumInput = 0.0;
    double sumFitted = 0.0;

    for (size_t i = 0; i < length; i++)
    {
        sumInput += inputPointsMem[i].m_z;
        sumFitted += fittedPointsMem[i].m_z;
    }

    const double avgInput = sumInput / length;
    const double avgFitted = sumFitted / length;

    sumInput = sumFitted = 0.0;
    double mult = 0.0;

    for (size_t i = 0; i < length; i++)
    {
        const double diffInput = inputPointsMem[i].m_z - avgInput;
        const double diffFitted = fittedPointsMem[i].m_z - avgFitted;

        mult += (diffInput * diffFitted);
        sumInput += diffInput * diffInput;
        sumFitted += diffFitted * diffFitted;
    }

    double result = mult / (std::sqrt(sumInput) * std::sqrt(sumFitted));
    return result;
}

void ShapeAlgorithm::FindB4B4Theta(const ShapeCoefficients& result, double& theta_deg, double& b4, double& b5)
{
    const double a4 = result.m_a4;
    const double a5 = result.m_a5;
    const double a3 = result.m_a3;

    const double a4a5_Sum_Half = (a4 + a5) / 2;

    const double a4a5_diffSquare = (a4 - a5) * (a4 - a5);
    const double a3_Square = a3 * a3;
    const double Insqrt = a3_Square + a4a5_diffSquare;
    const double dTheta_rad = 0.5 * std::asin(a3 / std::sqrt(Insqrt));
    const double NotEqualValue = a4 * std::cos(dTheta_rad) * std::cos(dTheta_rad)
        + a5 * std::sin(dTheta_rad) * std::sin(dTheta_rad) + a3 * std::cos(dTheta_rad) * std::sin(dTheta_rad);

    theta_deg = dTheta_rad * 180 / _PI;
    b4 = a4a5_Sum_Half + std::sqrt(Insqrt) / 2;
    b5 = a4a5_Sum_Half - std::sqrt(Insqrt) / 2;

    const double error = std::fabs(NotEqualValue - b4);

    if (error > 1e-20) // 이현민 : 이게 적당한 기준인지 불명확함..
        // 기준 설비와 비교시 아래 수식을 평가해야 제대로 된 값이 나오는 경우가 생긴다면
        // 이 조건을 완화할 필요가 있음..
        // 반대로 아래 수식을 평가하지 않아야 하는 경우가 발생하면, 이 조건을 강화해야 함.
    {
        theta_deg = 90 - theta_deg;
    }
}

double ShapeAlgorithm::FindNormalized(const std::vector<Ipvm::Point64r3>& points, const double b4, const double b5,
    const double theta_deg, double& normalizedB4, double& normalizedB5)
{
    const size_t length = points.size();
    const auto* pointsMem = points.data();

    double xMax = 0.;
    double xMin = 0.;
    double yMax = 0.;
    double yMin = 0.;

    const double theta_rad = theta_deg * _PI / 180;

    const double cosVal = std::cos(theta_rad);
    const double sinVal = std::sin(theta_rad);

    for (size_t idx = 0; idx < length; idx++)
    {
        const double x = pointsMem[idx].m_x;
        const double y = pointsMem[idx].m_y;

        const double xx = cosVal * x + sinVal * y;
        const double yy = -sinVal * x + cosVal * y;

        if (idx == 0)
        {
            xMin = xMax = xx;
            yMin = yMax = yy;
        }
        else
        {
            xMax = max(xMax, xx);
            xMin = min(xMin, xx);
            yMax = max(yMax, yy);
            yMin = min(yMin, yy);
        }
    }

    normalizedB4 = b4 * (xMax - xMin) * (xMax - xMin) / 4;
    normalizedB5 = b5 * (yMax - yMin) * (yMax - yMin) / 4;

    if (std::fabs(normalizedB5) > DBL_EPSILON)
    {
        return std::fabs(normalizedB4) / std::fabs(normalizedB5); //Calculate K
    }
    else
    {
        return 0.;
    }
}

/*
6). If R≥coeff_threshold (default is 0.8), then do the following
	6.1
		if  K≥insignificant_threshold (default is 10, but should be adjustable),
			If b4>0, “concave shape in cylindrical’, sign is “-”
			If b4< or equal 0, “convex shape in cylindrical’, sign is “+”
	6.2  if K≤1/ insignificant_threshold
		6.2.1 If b5>0, “concave shape in cylindrical’, sign is “-”
		6.2.2 If b5≤0, “convex shape in cylindrical’, sign is “+”
	6.3    when insignificant_threshold >K>1
		6.3.1 if both b4 and b5 >0, “concave shape in spherical”, sign is “-”
		6.3.2  if both b4 and b5 ≤0, “convex shape in spherical”, sign is “+”
		6.3.3  if b4*b5<0,
			If b4>0, “saddle shape’, sign is “-”
			If b4 ≤0, “saddle shape’, sign is “+”
	6.4  when 1≥K≥(1/insignificant_threshold)
		6.4.1 if both b4 and b5 >0, “concave shape in spherical”, sign is “-”
		6.4.2  if both b4 and b5 ≤0, “convex shape in spherical”, sign is “+”
		6.4.3  if b4*b5<0,
			If b5>0, “saddle shape’, sign is “-”
			If b5 ≤0, “saddle shape’, sign is “+”
7) if R< coeff_threshold,
	If K>1 and b4>0, “non-standard shape”, sign is “-”
	If K>1 and b4≤ 0, “non-standard shape”, sign is “+”
	If K≤1 and b5>0, “non-standard shape”, sign is “-”
	If K≤1 and b5≤0, “non-standard shape”, sign is “+”
*/
void ShapeAlgorithm::Report(const double coeff_threshold, const double insignificant_threshold, const double R,
    const double b4, const double b5, const double K, CString& sign, CString& shape)
{
    if (R >= coeff_threshold)
    {
        //6.1
        if (K >= insignificant_threshold)
        {
            if (b4 > 0)
            {
                sign = _T("-");
                shape = _T("Cylindrical");
            }
            else
            {
                sign = _T("+");
                shape = _T("Cylindrical");
            }
        }
        //6.2
        if (K <= (1 / insignificant_threshold))
        {
            if (b5 > 0)
            {
                sign = _T("-");
                shape = _T("Cylindrical");
            }
            else
            {
                sign = _T("+");
                shape = _T("Cylindrical");
            }
        }
        //6.3
        if (insignificant_threshold > K && K > 1)
        {
            if (b4 > 0 && b5 > 0)
            {
                sign = _T("-");
                shape = _T("Spherical");
            }
            else if (b4 <= 0 && b5 <= 0)
            {
                sign = _T("+");
                shape = _T("Spherical");
            }
            else //6.3.3
            {
                if (b4 > 0)
                {
                    sign = _T("-");
                    shape = _T("Saddle");
                }
                else
                {
                    sign = _T("+");
                    shape = _T("Saddle");
                }
            }
        }
        //6.4
        if ((1 >= K) && (K >= (1 / insignificant_threshold)))
        {
            if (b4 > 0 && b5 > 0)
            {
                sign = _T("-");
                shape = _T("Spherical");
            }
            else if (b4 <= 0 && b5 <= 0)
            {
                sign = _T("+");
                shape = _T("Spherical");
            }
            else
            {
                if (b5 > 0)
                {
                    sign = _T("-");
                    shape = _T("Saddle");
                }
                else
                {
                    sign = _T("+");
                    shape = _T("Saddle");
                }
            }
        }
    }
    else //R<coeff_threshold
    {
        if (K > 1 && b4 > 0)
        {
            sign = _T("-");
            shape = _T("Others");
        }
        if (K > 1 && b4 <= 0)
        {
            sign = _T("+");
            shape = _T("Others");
        }
        if (K <= 1 && b5 > 0)
        {
            sign = _T("-");
            shape = _T("Others");
        }
        if (K <= 1 && b5 <= 0)
        {
            sign = _T("+");
            shape = _T("Others");
        }
    }
}
