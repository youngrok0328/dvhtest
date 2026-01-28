#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <ipvmbasedef.h>
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
#include <Ipvm/Base/Point64r3.h>
#else
#include "Types/Point_64f_C3.h"
#endif

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
#if defined(IPVMSDK_MAJOR_VERSION) && IPVMSDK_MAJOR_VERSION < 9
namespace IntekPlus
{
namespace VisionMadang
{
using Point64r3 = Point_64f_C3;
}
} // namespace IntekPlus
namespace Ipvm = IntekPlus::VisionMadang;
#endif
struct ShapeCoefficients;

//HDR_6_________________________________ Header body
//
class __INTEKPLUS_SHARED_INTELSPECIFIC_API__ ShapeAlgorithm
{
public:
    ShapeAlgorithm(void);
    ~ShapeAlgorithm(void);

    bool GetShape(const std::vector<Ipvm::Point64r3>& points, CString& sign, CString& shape, double& r, double& b4_um,
        double& b5_um, double& angle_deg);
    double GetNormalizedB4() const;
    double GetNormalizedB5() const;

private:
    void RefineInputData(const std::vector<Ipvm::Point64r3>& input, std::vector<Ipvm::Point64r3>& output);

    void AdjustCoordinateXY(std::vector<Ipvm::Point64r3>& points);

    void FitC4Area(const std::vector<Ipvm::Point64r3>& points, ShapeCoefficients& result);

    void GetFittingSurface(const ShapeCoefficients& result, std::vector<Ipvm::Point64r3>& points);

    double GetNormalizedCorrelationCoeffR(
        const std::vector<Ipvm::Point64r3>& inputPoints, const std::vector<Ipvm::Point64r3>& fittedPoints);

    void FindB4B4Theta(const ShapeCoefficients& result, double& theta_deg, double& b4, double& b5);
    double FindNormalized(const std::vector<Ipvm::Point64r3>& points, const double b4, const double b5,
        const double theta_deg, double& normalizedB4, double& normalizedB5);

    //////////////////////////////////////////////////////////////////////////
    std::vector<Ipvm::Point64r3> m_inputPoints;
    std::vector<Ipvm::Point64r3> m_fittedPoints; //New Point Data
    double m_normalizedB4;
    double m_normalizedB5;
    double m_coeffThreshold;
    double m_insignificantThreshold;

    /*
	1.	both coeff_threshold, and insignificant_threshold should be leave to users to setup since this could be different for different applications.

	2.	output file should include CAW value, coefficient R, norminzed_b4, norminzed_b5,ø, shapes (concave in cylindrical/spherical,
	convex in cylindrical/spherical, saddle, non standard), sign for each single unit.
	*/

    // 아래 함수의 인자 이름은, 인텔 문서의 이름과 통일시키기 위해서 명명 표준을 준수하지 않도록 함.
    void Report(const double coeff_threshold, const double insignificant_threshold, const double R, const double b4,
        const double b5, const double K, CString& sign, CString& shape);
};
