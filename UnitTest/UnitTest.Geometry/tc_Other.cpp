//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Base/CircleEq32r.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Polygon32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define DATA_MAX 100

//CPP_7_________________________________ Implementation body
//
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestGeometry
{
TEST_CLASS (tc_Other)
{
#pragma region CircleFitting
    TEST_METHOD (CircleFitting_pf_IPVM)
    {
        // TODO: 테스트 코드를 여기에 입력합니다.
        float* fDatax = new float[DATA_MAX];
        float* fDatay = new float[DATA_MAX];
        Ipvm::Point32r2* ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        long nIdx = 0;

        Ipvm::CircleEq32r clData;

        clData.m_radius = 0;
        clData.m_x = 0;
        clData.m_y = 0;

        //해답
        Ipvm::CircleEq32r clResult;

        clResult.m_radius = 10;
        clResult.m_x = 30;
        clResult.m_y = 30;

        //===================================================================================================
        //원
        for (int y = 0; y < DATA_MAX; y++)
        {
            for (int x = 0; x < DATA_MAX; x++)
            {
                if (pow(x - clResult.m_x, 2) + pow(y - clResult.m_y, 2) == pow(clResult.m_radius, 2))
                {
                    fDatax[nIdx] = (float)x;
                    fDatay[nIdx] = (float)y;

                    ptfc2Data[nIdx].m_x = fDatax[nIdx];
                    ptfc2Data[nIdx].m_y = fDatay[nIdx++];
                }
            }
        }

        auto result = Ipvm::DataFitting::FitToCircle(nIdx, ptfc2Data, clData);
        //cgGeometry.CircleFitting(clData, nIdx, fDatax, fDatay);

        CString msg;

        msg.Format(_T("Error_s_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_s_clData.m_radius D : R = %0.10f : %0.10f"), clData.m_radius, clResult.m_radius);
        Assert::AreEqual(clResult.m_radius, clData.m_radius, msg);
        msg.Format(_T("Error_s_clData.m_x D : R = %0.10f : %0.10f"), clData.m_x, clResult.m_x);
        Assert::AreEqual(clResult.m_x, clData.m_x, msg);
        msg.Format(_T("Error_s_clData.m_y D : R = %0.10f : %0.10f"), clData.m_y, clResult.m_y);
        Assert::AreEqual(clResult.m_y, clData.m_y, msg);

        delete[] ptfc2Data;
        delete[] fDatax;
        delete[] fDatay;
        //===================================================================================================

        //===================================================================================================
        //저장소에 값들이 0값만 있을 경우
        fDatax = new float[nIdx];
        fDatay = new float[nIdx];
        ptfc2Data = new Ipvm::Point32r2[nIdx];

        for (int i = 0; i < nIdx; i++)
        {
            fDatax[i] = 0;
            fDatay[i] = 0;

            ptfc2Data[i].m_x = fDatax[i];
            ptfc2Data[i].m_y = fDatay[i];
        }

        result = Ipvm::DataFitting::FitToCircle(nIdx, ptfc2Data, clData);
        //cgGeometry.CircleFitting(clResult, nIdx, fDatax, fDatay);

        msg.Format(_T("Error_f1_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);

        delete[] ptfc2Data;
        delete[] fDatax;
        delete[] fDatay;
        //===================================================================================================

        //===================================================================================================
        //저장소에 값이 1개 있을 경우
        fDatax = new float[1];
        fDatay = new float[1];
        ptfc2Data = new Ipvm::Point32r2[1];

        fDatax[0] = 0;
        fDatay[0] = 0;

        ptfc2Data[0].m_x = fDatax[0];
        ptfc2Data[0].m_y = fDatay[0];

        result = Ipvm::DataFitting::FitToCircle(1, ptfc2Data, clData);
        //cgGeometry.CircleFitting(clResult, 1, fDatax, fDatay);

        msg.Format(_T("Error_f1_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);

        delete[] ptfc2Data;
        delete[] fDatax;
        delete[] fDatay;
        //===================================================================================================
    }

    TEST_METHOD (CircleFitting_vs_IPVM)
    {
        // TODO: 테스트 코드를 여기에 입력합니다.
        std::vector<Ipvm::Point32r2> vsfData;
        Ipvm::Point32r2* ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        long nIdx = 0;

        Ipvm::CircleEq32r clData;

        clData.m_radius = 0;
        clData.m_x = 0;
        clData.m_y = 0;

        //해답
        Ipvm::CircleEq32r clResult;

        clResult.m_radius = 10;
        clResult.m_x = 30;
        clResult.m_y = 30;

        //===================================================================================================
        //원
        for (int y = 0; y < DATA_MAX; y++)
        {
            for (int x = 0; x < DATA_MAX; x++)
            {
                if (pow(x - clResult.m_x, 2) + pow(y - clResult.m_y, 2) == pow(clResult.m_radius, 2))
                {
                    vsfData.push_back(Ipvm::Point32r2((float)x, (float)y));

                    ptfc2Data[nIdx].m_x = (float)x;
                    ptfc2Data[nIdx++].m_y = (float)y;
                }
            }
        }

        auto result = Ipvm::DataFitting::FitToCircle(nIdx, ptfc2Data, clData);
        //cgGeometry.CircleFitting(clData, (long)vsfData.size(), vsfData);

        CString msg;

        msg.Format(_T("Error_s_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_s_clData.m_radius D : R = %0.10f : %0.10f"), clData.m_radius, clResult.m_radius);
        Assert::AreEqual(clResult.m_radius, clData.m_radius, msg);
        msg.Format(_T("Error_s_clData.m_x D : R = %0.10f : %0.10f"), clData.m_x, clResult.m_x);
        Assert::AreEqual(clResult.m_x, clData.m_x, msg);
        msg.Format(_T("Error_s_clData.m_y D : R = %0.10f : %0.10f"), clData.m_y, clResult.m_y);
        Assert::AreEqual(clResult.m_y, clData.m_y, msg);

        delete[] ptfc2Data;
        //===================================================================================================

        //===================================================================================================
        //저장소에 값들이 0값만 있을 경우
        long size = (long)vsfData.size();
        ptfc2Data = new Ipvm::Point32r2[nIdx];

        vsfData.clear();

        for (int i = 0; i < size; i++)
        {
            vsfData.push_back(Ipvm::Point32r2(0, 0));
            ptfc2Data[i].m_x = 0;
            ptfc2Data[i].m_y = 0;
        }

        result = Ipvm::DataFitting::FitToCircle(1, ptfc2Data, clData);
        //cgGeometry.CircleFitting(clData, (long)vsfData.size(), vsfData);

        msg.Format(_T("Error_f1_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);

        delete[] ptfc2Data;
        //===================================================================================================

        //===================================================================================================
        //저장소에 값이 1개 있을 경우
        vsfData.clear();
        ptfc2Data = new Ipvm::Point32r2[1];

        vsfData.push_back(Ipvm::Point32r2(0, 0));

        ptfc2Data[0].m_x = 0;
        ptfc2Data[0].m_y = 0;

        result = Ipvm::DataFitting::FitToCircle(1, ptfc2Data, clData);
        //cgGeometry.CircleFitting(clData, (long)vsfData.size(), vsfData);

        msg.Format(_T("Error_f1_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);

        delete[] ptfc2Data;
        //===================================================================================================
    }
#pragma endregion

    TEST_METHOD (bPtIsInsidePolygon_IPVM)
    {
        // TODO: 테스트 코드를 여기에 입력합니다.
        short nPolygonType = 0;
        CPoint* cpData = new CPoint[DATA_MAX];
        Ipvm::Point32r2* ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        Ipvm::Point32r2 cpCur(10, 20);
        //===================================================================================================
        //오각형, 선분이 이어지는 순서대로 있어야 성공
        cpData[0] = {10, 10};
        cpData[1] = {5, 20};
        cpData[2] = {8, 30};
        cpData[3] = {12, 30};
        cpData[4] = {15, 20};

        ptfc2Data[0] = Ipvm::Point32r2(10, 10);
        ptfc2Data[1] = Ipvm::Point32r2(5, 20);
        ptfc2Data[2] = Ipvm::Point32r2(8, 30);
        ptfc2Data[3] = Ipvm::Point32r2(12, 30);
        ptfc2Data[4] = Ipvm::Point32r2(15, 20);

        nPolygonType = 5;

        Ipvm::Polygon32r plData;
        plData = Ipvm::Polygon32r(ptfc2Data, nPolygonType);

        auto result = plData.ptInPolygon(cpCur);
        //cgGeometry.bPtIsInsidePolygon(cpData, nPolygonType, cpCur);

        CString msg;

        msg.Format(_T("Error_s_Result D : R = %d : %d"), result, true);
        Assert::AreEqual(true, result, msg);
        //===================================================================================================

        //===================================================================================================
        cpCur = Ipvm::Point32r2(100, 100);

        result = plData.ptInPolygon(cpCur);
        //cgGeometry.bPtIsInsidePolygon(cpData, nPolygonType, cpCur);

        msg.Format(_T("Error_f_Result D : R = %d : %d"), result, false);
        Assert::AreEqual(false, result, msg);
        //===================================================================================================
    }

#pragma region Rotate
    //Rotate가 있긴한데 함수 각각이 무슨 차이인지 모르겠습니다..
    TEST_METHOD (RotateRect)
    {
        // TODO: 테스트 코드를 여기에 입력합니다.
        CPI_Geometry cgGeometry;

        Ipvm::Rect32s rData(20, 20, 60, 60);
        float fRadian = 3.1415926535f / 180.f;
        float fAngle = 45.f * fRadian;
        Ipvm::Point32s2 ptOrigin(40, 40);

        Ipvm::Rect32s rResult(40, 11, 40, 68);
        //===================================================================================================
        cgGeometry.RotateRect(rData, fAngle, ptOrigin);

        CString msg;

        msg.Format(_T("Error_s_rData.Top D : R = %d : %d"), rData.m_top, rResult.m_top);
        Assert::AreEqual(rData.m_top, rResult.m_top, msg);
        msg.Format(_T("Error_s_rData.Bottom D : R = %d : %d"), rData.m_bottom, rResult.m_bottom);
        Assert::AreEqual(rData.m_bottom, rResult.m_bottom, msg);
        msg.Format(_T("Error_s_rData.Left D : R = %d : %d"), rData.m_left, rResult.m_left);
        Assert::AreEqual(rData.m_left, rResult.m_left, msg);
        msg.Format(_T("Error_s_rData.Right D : R = %d : %d"), rData.m_right, rResult.m_right);
        Assert::AreEqual(rData.m_right, rResult.m_right, msg);
        //===================================================================================================

        //===================================================================================================
        rData = Ipvm::Rect32s{0, 0, 0, 0};
        fAngle = 45 * fRadian;
        ptOrigin = {0, 0};

        rResult = Ipvm::Rect32s{0, 0, 0, 0};

        cgGeometry.RotateRect(rData, fAngle, ptOrigin);

        msg.Format(_T("Error_0_rData.Top D : R = %d : %d"), rData.m_top, rResult.m_top);
        Assert::AreEqual(rData.m_top, rResult.m_top, msg);
        msg.Format(_T("Error_0_rData.Bottom D : R = %d : %d"), rData.m_bottom, rResult.m_bottom);
        Assert::AreEqual(rData.m_bottom, rResult.m_bottom, msg);
        msg.Format(_T("Error_0_rData.Left D : R = %d : %d"), rData.m_left, rResult.m_left);
        Assert::AreEqual(rData.m_left, rResult.m_left, msg);
        msg.Format(_T("Error_0_rData.Right D : R = %d : %d"), rData.m_right, rResult.m_right);
        Assert::AreEqual(rData.m_right, rResult.m_right, msg);
        //===================================================================================================
    }

    TEST_METHOD (RotatePoint_cp)
    {
        Ipvm::Point32r2 dataXY(20.f, 20.f);
        float fRadian = 3.1415926535f / 180.f;
        float fAngle = 45.f * fRadian;
        Ipvm::Point32s2 ptOrigin(40, 40);

        float fResultx = 40.f;
        float fResulty = 11.7157287598f;
        //===================================================================================================
        CPI_Geometry::RotatePoint(dataXY, fAngle, ptOrigin);

        CString msg;

        msg.Format(_T("Error_s_fDatax D : R = %0.10f : %0.10f"), dataXY.m_x, fResultx);
        Assert::AreEqual(dataXY.m_x, fResultx, msg);
        msg.Format(_T("Error_s_fDatay D : R = %0.10f : %0.10f"), dataXY.m_y, fResulty);
        Assert::AreEqual(dataXY.m_y, fResulty, msg);
        //===================================================================================================

        //===================================================================================================

        dataXY = Ipvm::Point32r2(0.f, 0.f);
        fAngle = 45 * fRadian;
        ptOrigin = {0, 0};

        fResultx = 0.f;
        fResulty = 0.f;

        CPI_Geometry::RotatePoint(dataXY, fAngle, ptOrigin);

        msg.Format(_T("Error_0_fDatax D : R = %0.10f : %0.10f"), dataXY.m_x, fResultx);
        Assert::AreEqual(dataXY.m_x, fResultx, msg);
        msg.Format(_T("Error_0_fDatay D : R = %0.10f : %0.10f"), dataXY.m_y, fResulty);
        Assert::AreEqual(dataXY.m_y, fResulty, msg);
        //===================================================================================================
    }

    TEST_METHOD (RotatePoint_f)
    {
        Ipvm::Point32r2 dataXY(20.f, 20.f);
        Ipvm::Point32r2 ptOrigin(40.f, 40.f);
        float fRadian = 3.1415926535f / 180.f;
        float fAngle = 45.f * fRadian;

        float fResultx = 40.f;
        float fResulty = 11.7157287598f;
        //===================================================================================================
        CPI_Geometry::RotatePoint(dataXY, fAngle, ptOrigin);

        CString msg;

        msg.Format(_T("Error_s_fDatax D : R = %0.10f : %0.10f"), dataXY.m_x, fResultx);
        Assert::AreEqual(dataXY.m_x, fResultx, msg);
        msg.Format(_T("Error_s_fDatay D : R = %0.10f : %0.10f"), dataXY.m_y, fResulty);
        Assert::AreEqual(dataXY.m_y, fResulty, msg);
        //===================================================================================================

        //===================================================================================================
        dataXY = Ipvm::Point32r2(0.f, 0.f);
        fAngle = 45 * fRadian;
        ptOrigin = Ipvm::Point32r2(0.f, 0.f);

        fResultx = 0.f;
        fResulty = 0.f;

        CPI_Geometry::RotatePoint(dataXY, fAngle, ptOrigin);

        msg.Format(_T("Error_0_fDatax D : R = %0.10f : %0.10f"), dataXY.m_x, fResultx);
        Assert::AreEqual(dataXY.m_x, fResultx, msg);
        msg.Format(_T("Error_0_fDatay D : R = %0.10f : %0.10f"), dataXY.m_y, fResulty);
        Assert::AreEqual(dataXY.m_y, fResulty, msg);
        //===================================================================================================
    }
#pragma endregion

    TEST_METHOD (Get2DCurveFitting)
    {
        // TODO: 테스트 코드를 여기에 입력합니다.
        CPI_Geometry cgGeometry;

        std::vector<Ipvm::Point32r2> vsfData;

        const Curve2DEq c2dOper{2, 5, 10};

        //===================================================================================================
        //y = 10x^2 + 5x + 2
        for (int i = 0; i < DATA_MAX; i++)
        {
            vsfData.push_back(
                Ipvm::Point32r2((float)i, (float)(c2dOper.m_c * pow(i, 2) + c2dOper.m_b * i + c2dOper.m_a)));
        }

        Curve2DEq c2dCurve{0., 0., 0.};

        auto result = cgGeometry.Get2DCurveFitting(vsfData, c2dCurve);

        CString msg;

        msg.Format(_T("Error_s_Result D : R = %d : %d"), result, TRUE);
        Assert::AreEqual(TRUE, result, msg);
        msg.Format(_T("Error_s_c2dCurve.a D : R = %0.30f : %0.30f"), c2dCurve.m_a, c2dOper.m_a);
        Assert::AreEqual(c2dOper.m_a, c2dCurve.m_a, 0.000000001, msg);
        msg.Format(_T("Error_s_c2dCurve.b D : R = %0.30f : %0.30f"), c2dCurve.m_b, c2dOper.m_b);
        Assert::AreEqual(c2dOper.m_b, c2dCurve.m_b, 0.000000001, msg);
        msg.Format(_T("Error_s_c2dCurve.c D : R = %0.30f : %0.30f"), c2dCurve.m_c, c2dOper.m_c);
        Assert::AreEqual(c2dOper.m_c, c2dCurve.m_c, 0.000000001, msg);
        //===================================================================================================

        //===================================================================================================
        //0값을 DATA_MAX만큼 넣었을 때
        vsfData.clear();

        for (int i = 0; i < DATA_MAX; i++)
        {
            vsfData.push_back(Ipvm::Point32r2((float)0, (float)0));
        }

        result = cgGeometry.Get2DCurveFitting(vsfData, c2dCurve);

        msg.Format(_T("Error_0_Result D : R = %d : %d"), result, TRUE);
        Assert::AreNotEqual(TRUE, result, msg);
        //===================================================================================================

        //===================================================================================================
        //0값을 1개만 넣었을 때
        vsfData.clear();

        vsfData.push_back(Ipvm::Point32r2((float)0, (float)0));

        result = cgGeometry.Get2DCurveFitting(vsfData, c2dCurve);

        msg.Format(_T("Error_01_Result D : R = %d : %d"), result, TRUE);
        Assert::AreNotEqual(TRUE, result, msg);
        //===================================================================================================
    }

    //직선상에서 주어진 거리만큼 떨어진 위치의 좌표를 반환한다
    TEST_METHOD (GetLinePoint)
    {
        CPI_Geometry cgGeometry;

        Ipvm::Point32r2 sfpStart(10.f, 20.f);
        Ipvm::Point32r2 sfpEnd(30.f, 30.f);
        float fDist = 5.0f;

        Ipvm::Point32r2 sfpResult(14.4880466461f, 22.2039604187f);
        //===================================================================================================
        auto result = cgGeometry.GetLinePoint(sfpStart, sfpEnd, fDist);

        CString msg;

        msg.Format(_T("Error_s_sfpResult.m_x D : R = %0.10f : %0.10f"), result.m_x, sfpResult.m_x);
        Assert::AreEqual(sfpResult.m_x, result.m_x, msg);
        msg.Format(_T("Error_s_sfpResult.m_y D : R = %0.10f : %0.10f"), result.m_y, sfpResult.m_y);
        Assert::AreEqual(sfpResult.m_y, result.m_y, msg);
        //===================================================================================================

        //===================================================================================================
        sfpStart.Set(10.f, 10.f);
        sfpEnd.Set(10.f, 10.f);
        fDist = 10.f;

        sfpResult.Set(19.9334468842f, 8.8482027054f);

        result = cgGeometry.GetLinePoint(sfpStart, sfpEnd, fDist);

        msg.Format(_T("Error_s_sfpResult.m_x D : R = %0.10f : %0.10f"), result.m_x, sfpResult.m_x);
        Assert::AreEqual(sfpResult.m_x, result.m_x, msg);
        msg.Format(_T("Error_s_sfpResult.m_y D : R = %0.10f : %0.10f"), result.m_y, sfpResult.m_y);
        Assert::AreEqual(sfpResult.m_y, result.m_y, msg);
        //===================================================================================================
    }

    //직선상에서 주어진 거리만큼 떨어진 위치의 좌표를 반환한다
    TEST_METHOD (FitToEllipse_IPVM)
    {
        Ipvm::Point32r2 points[10];
        points[0] = Ipvm::Point32r2(8548.12305f, 8720.00000f);
        points[1] = Ipvm::Point32r2(8542.31836f, 8730.36426f);
        points[2] = Ipvm::Point32r2(8533.57617f, 8737.14746f);
        points[3] = Ipvm::Point32r2(8521.54688f, 8739.84473f);
        points[4] = Ipvm::Point32r2(8506.81641f, 8735.33301f);
        points[5] = Ipvm::Point32r2(8496.41699f, 8720.00000f);
        points[6] = Ipvm::Point32r2(8438.42188f, 8655.16309f);
        points[7] = Ipvm::Point32r2(8517.24805f, 8686.93750f);
        points[8] = Ipvm::Point32r2(8537.65918f, 8690.29785f);
        points[9] = Ipvm::Point32r2(8548.84961f, 8704.90820f);

        Ipvm::EllipseEq32r result;
        Assert::AreNotEqual(Ipvm::Status::e_ok, Ipvm::DataFitting::FitToEllipse(10, points, result));
    }
};
} // namespace UnitTestGeometry