//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/iDataType.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>

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
TEST_CLASS (tc_LineFitting)
{
public:
#pragma region LineFitting_RemoveNoise

    TEST_METHOD (LineFitting_RemoveNoise_vs)
    {
        // TODO: 테스트 코드를 여기에 입력합니다.
        //ax + by+ c = 0
        std::vector<Ipvm::Point32r2> vfpPoint;
        Ipvm::LineEq32r line(0.f, 0.f, 0.f);

        float fResulta = -0.0999949649f;
        float fResultb = -0.0999933705f;
        float fResultc = 1.0f;
        //===================================================================================================

        //===================================================================================================
        //-0.1x - 0.1y + 1 = 0;
        //x값 노이즈 있을시
        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fx = 65.0f;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        auto result = CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, line);

        CString msg;

        msg.Format(_T("Error_x_Result D : R = %d : %d"), result, 0L);
        Assert::AreEqual(true, result, msg);
        msg.Format(_T("Error_x_fDataa D : R = %0.10f : %0.10f"), line.m_a, fResulta);
        Assert::AreEqual(fResulta, line.m_a, msg);
        msg.Format(_T("Error_x_fDatab D : R = %0.10f : %0.10f"), line.m_b, fResultb);
        Assert::AreEqual(fResultb, line.m_b, msg);
        msg.Format(_T("Error_x_fDatac D : R = %0.10f : %0.10f"), line.m_c, fResultc);
        Assert::AreEqual(fResultc, line.m_c, msg);
        //===================================================================================================

        //===================================================================================================
        //y값 노이즈 있을 시
        vfpPoint.clear();

        fResulta = -0.0861174464f;
        fResultb = -0.0832529590f;
        fResultc = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fy = 40.0f;
            if (i == 70)
                fy = 65.0f;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        result = CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, line);

        msg.Format(_T("Error_y_Result D : R = %d : %d"), result, 0L);
        Assert::AreEqual(true, result, msg);
        msg.Format(_T("Error_y_fDataa D : R = %0.10f : %0.10f"), line.m_a, fResulta);
        Assert::AreEqual(fResulta, line.m_a, msg);
        msg.Format(_T("Error_y_fDatab D : R = %0.10f : %0.10f"), line.m_b, fResultb);
        Assert::AreEqual(fResultb, line.m_b, msg);
        msg.Format(_T("Error_y_fDatac D : R = %0.10f : %0.10f"), line.m_c, fResultc);
        Assert::AreEqual(fResultc, line.m_c, msg);
        //===================================================================================================

        //===================================================================================================
        //x, y값 노이즈 있을 시
        vfpPoint.clear();

        fResulta = -0.0918209702f;
        fResultb = -0.0901771560f;
        fResultc = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fy = 65.0f;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        result = CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, line);

        msg.Format(_T("Error_xy_Result D : R = %d : %d"), result, 0L);
        Assert::AreEqual(true, result, msg);
        msg.Format(_T("Error_xy_fDataa D : R = %0.10f : %0.10f"), line.m_a, fResulta);
        Assert::AreEqual(fResulta, line.m_a, msg);
        msg.Format(_T("Error_xy_fDatab D : R = %0.10f : %0.10f"), line.m_b, fResultb);
        Assert::AreEqual(fResultb, line.m_b, msg);
        msg.Format(_T("Error_xy_fDatac D : R = %0.10f : %0.10f"), line.m_c, fResultc);
        Assert::AreEqual(fResultc, line.m_c, msg);
        //===================================================================================================

        //===================================================================================================
        //저장소 값이 1개 있을 경우
        vfpPoint.clear();

        vfpPoint.push_back(Ipvm::Point32r2(10, 20));

        result = CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, line);

        msg.Format(_T("Error_1_Result D : R = %d : %d"), result, 0L);
        Assert::AreNotEqual(true, result, msg);
        //===================================================================================================
    }

    //VMSDK에 FitToLine_RN과 같음
    TEST_METHOD (LineFitting_RemoveNoise)
    {
        //ax + by+ c = 0
        std::vector<Ipvm::Point32r2> vfpPoint;
        float fSigma = 1.f;

        std::vector<Ipvm::Point32r2> vfpData;
        Ipvm::LineEq32r line(0.f, 0.f, 0.f);

        float fResulta = -0.0999949649f;
        float fResultb = -0.0999933705f;
        float fResultc = 1.0f;

        //===================================================================================================
        //-0.1x - 0.1y + 1 = 0;
        //x값 노이즈 있을시
        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fx = 65.0f;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        auto result = CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, vfpData, line, fSigma);

        vfpPoint.clear();

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                continue;
            if (i == 70)
                continue;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        CString msg;

        msg.Format(_T("Error_x_Result D : R = %d : %d"), result, TRUE);
        Assert::AreEqual(true, result, msg);
        msg.Format(_T("Error_x_fDataa D : R = %0.10f : %0.10f"), line.m_a, fResulta);
        Assert::AreEqual(fResulta, line.m_a, msg);
        msg.Format(_T("Error_x_fDatab D : R = %0.10f : %0.10f"), line.m_b, fResultb);
        Assert::AreEqual(fResultb, line.m_b, msg);
        msg.Format(_T("Error_x_fDatac D : R = %0.10f : %0.10f"), line.m_c, fResultc);
        Assert::AreEqual(fResultc, line.m_c, msg);
        msg.Format(_T("Error_x_Datasize D : R = %d : %d"), DATA_MAX - 2, vfpData.size());
        Assert::AreEqual(DATA_MAX - 2, (int)vfpData.size(), msg);

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(_T("Error_x_vfpData[%d].m_x D : R = %0.10f : %0.10f"), i, vfpData[i].m_x, vfpPoint[i].m_x);
            Assert::AreEqual(vfpPoint[i].m_x, vfpData[i].m_x, msg);
            msg.Format(_T("Error_x_vfpData[%d].m_y D : R = %0.10f : %0.10f"), i, vfpData[i].m_y, vfpPoint[i].m_y);
            Assert::AreEqual(vfpPoint[i].m_y, vfpData[i].m_y, msg);
        }
        //===================================================================================================

        //===================================================================================================
        //y값 노이즈 있을 시
        vfpPoint.clear();
        vfpData.clear();

        fResulta = -0.0861174464f;
        fResultb = -0.0832529590f;
        fResultc = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fy = 40.0f;
            if (i == 70)
                fy = 65.0f;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        Assert::AreEqual(true, CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, vfpData, line, fSigma));

        vfpPoint.clear();

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                continue;
            if (i == 70)
                continue;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        msg.Format(_T("Error_y_Result D : R = %d : %d"), result, TRUE);
        Assert::AreEqual(true, result, msg);
        msg.Format(_T("Error_y_fDataa D : R = %0.10f : %0.10f"), line.m_a, fResulta);
        Assert::AreEqual(fResulta, line.m_a, msg);
        msg.Format(_T("Error_y_fDatab D : R = %0.10f : %0.10f"), line.m_b, fResultb);
        Assert::AreEqual(fResultb, line.m_b, msg);
        msg.Format(_T("Error_y_fDatac D : R = %0.10f : %0.10f"), line.m_c, fResultc);
        Assert::AreEqual(fResultc, line.m_c, msg);
        msg.Format(_T("Error_y_Datasize D : R = %d : %d"), DATA_MAX - 2, vfpData.size());
        Assert::AreEqual(DATA_MAX - 2, (int)vfpData.size(), msg);

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(_T("Error_y_vfpData[%d].m_x D : R = %0.10f : %0.10f"), i, vfpData[i].m_x, vfpPoint[i].m_x);
            Assert::AreEqual(vfpPoint[i].m_x, vfpData[i].m_x, msg);
            msg.Format(_T("Error_y_vfpData[%d].m_y D : R = %0.10f : %0.10f"), i, vfpData[i].m_y, vfpPoint[i].m_y);
            Assert::AreEqual(vfpPoint[i].m_y, vfpData[i].m_y, msg);
        }
        //===================================================================================================

        //===================================================================================================
        //x, y값 노이즈 있을 시
        //왜 x값은 지워지지 않는가
        vfpPoint.clear();
        vfpData.clear();

        fResulta = -0.0918209702f;
        fResultb = -0.0901771560f;
        fResultc = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fy = 65.0f;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        Assert::AreEqual(true, CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, vfpData, line, fSigma));

        vfpPoint.clear();

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                continue;

            vfpPoint.push_back(Ipvm::Point32r2(fx, fy));
        }

        msg.Format(_T("Error_xy_Result D : R = %d : %d"), result, TRUE);
        Assert::AreEqual(true, result, msg);
        msg.Format(_T("Error_xy_fDataa D : R = %0.10f : %0.10f"), line.m_a, fResulta);
        Assert::AreEqual(fResulta, line.m_a, msg);
        msg.Format(_T("Error_xy_fDatab D : R = %0.10f : %0.10f"), line.m_b, fResultb);
        Assert::AreEqual(fResultb, line.m_b, msg);
        msg.Format(_T("Error_xy_fDatac D : R = %0.10f : %0.10f"), line.m_c, fResultc);
        Assert::AreEqual(fResultc, line.m_c, msg);
        msg.Format(_T("Error_xy_Datasize D : R = %d : %d"), DATA_MAX - 1 /*2*/, vfpData.size());
        Assert::AreEqual(DATA_MAX - 1 /*2*/, (int)vfpData.size(), msg);

        for (int i = 0; i < DATA_MAX - 1; i++)
        {
            msg.Format(_T("Error_y_vfpData[%d].m_x D : R = %0.10f : %0.10f"), i, vfpData[i].m_x, vfpPoint[i].m_x);
            Assert::AreEqual(vfpPoint[i].m_x, vfpData[i].m_x, msg);
            msg.Format(_T("Error_y_vfpData[%d].m_y D : R = %0.10f : %0.10f"), i, vfpData[i].m_y, vfpPoint[i].m_y);
            Assert::AreEqual(vfpPoint[i].m_y, vfpData[i].m_y, msg);
        }
        //===================================================================================================

        //===================================================================================================
        //저장소 값이 1개 있을 경우
        vfpPoint.clear();
        vfpData.clear();

        vfpPoint.push_back(Ipvm::Point32r2(10, 20));

        Assert::AreEqual(false, CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, vfpData, line, fSigma));

        //===================================================================================================

        //===================================================================================================
        //저장소 값이 0값만 있을 경우
        vfpPoint.clear();
        vfpData.clear();

        vfpPoint.push_back(Ipvm::Point32r2(0, 0));

        Assert::AreEqual(false, CPI_Geometry::LineFitting_RemoveNoise(vfpPoint, vfpData, line, fSigma));

        //===================================================================================================
    }

    //FitToLine_RN(long i_nSrcDataNum, const Point_32f_C2*	i_pPtSrcXY, float i_fSrcSigma, LineEq_32f &o_dstLineEq,
    //Point_32f_C2* o_pPtDstUsedXY = nullptr, long* o_pnDstUsedDataNum = nullptr);
    //FitToLine_RN(const LineEq_32f &i_srcLineEq, long i_nSrcDataNum, const Point_32f_C2* i_pPtSrcXY, float i_fSrcSigma,
    //LineEq_32f &o_dstLineEq, Point_32f_C2* o_pPtDstUsedXY = nullptr, long* o_pnDstUsedDataNum = nullptr);
    TEST_METHOD (FitToLine_RN)
    {
        //ax + by+ c = 0
        Ipvm::Point32r2* ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        float fSigma = 1.f;

        Ipvm::Point32r2* ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        Ipvm::LineEq32r lefData;
        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;
        int32_t nDatanum = 0;

        Ipvm::LineEq32r lefResult;
        lefResult.m_a = -0.1000000015f; //-0.0999949649f;
        lefResult.m_b = -0.1000000015f; //-0.0999933705f;
        lefResult.m_c = 1.0f;
        //===================================================================================================
        //-0.1x - 0.1y + 1 = 0;
        //x값 노이즈 있을시
        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fx = 65.0f;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;
        }

        auto result = Ipvm::DataFitting::FitToLineRn(DATA_MAX, ptfc2Point, fSigma, lefData, ptfc2Data, &nDatanum);

        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];

        /*for (int i = 0; i < DATA_MAX; i++)
			{
				float fx = (float)i;
				float fy = (float)DATA_MAX / 10.0f - (float)i;

				if (i == 35) i++;
				if (i == 70) i++;

				ptfc2Point[i].m_x = fx;
				ptfc2Point[i].m_y = fy;
			}*/

        int val = 0;

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            if (i == 35)
                val++;
            if (i == 69)
                val++;

            float fx = (float)val;
            float fy = (float)DATA_MAX / 10.0f - (float)val;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;

            val++;
        }

        CString msg;

        msg.Format(_T("Error_x_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_x_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_x_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_x_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);
        /*msg.Format(_T("Error_x_Datasize D : R = %d : %d"), DATA_MAX - 2, nDatanum);
			Assert::AreEqual(DATA_MAX - 2, (int)nDatanum, msg);*/

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(_T("Error_x_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Point[i].m_x);
            Assert::AreEqual(ptfc2Point[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(_T("Error_x_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Point[i].m_y);
            Assert::AreEqual(ptfc2Point[i].m_y, ptfc2Data[i].m_y, msg);
        }
        //===================================================================================================

        ////===================================================================================================
        ////y값 노이즈 있을 시
        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        delete ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = -0.1000000015f; //-0.0861174464f;
        lefResult.m_b = -0.1000000015f; //-0.0832529590f;
        lefResult.m_c = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fy = 40.0f;
            if (i == 70)
                fy = 65.0f;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;
        }

        result = Ipvm::DataFitting::FitToLineRn(DATA_MAX, ptfc2Point, fSigma, lefData, ptfc2Data, &nDatanum);

        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];

        /*for (int i = 0; i < DATA_MAX; i++)
			{
			float fx = (float)i;
			float fy = (float)DATA_MAX / 10.0f - (float)i;

			if (i == 35) i++;
			if (i == 70) i++;

			ptfc2Point[i].m_x = fx;
			ptfc2Point[i].m_y = fy;
			}*/

        val = 0;

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            if (i == 35)
                val++;
            if (i == 69)
                val++;

            float fx = (float)val;
            float fy = (float)DATA_MAX / 10.0f - (float)val;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;

            val++;
        }

        msg.Format(_T("Error_y_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_y_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_y_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_y_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);
        /*msg.Format(_T("Error_y_Datasize D : R = %d : %d"), DATA_MAX - 2, nDatanum);
			Assert::AreEqual(DATA_MAX - 2, (int)nDatanum, msg);*/

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(_T("Error_y_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Point[i].m_x);
            Assert::AreEqual(ptfc2Point[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(_T("Error_y_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Point[i].m_y);
            Assert::AreEqual(ptfc2Point[i].m_y, ptfc2Data[i].m_y, msg);
        }
        ////===================================================================================================

        ////===================================================================================================
        ////x, y값 노이즈 있을 시
        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        delete ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = -0.0991630629f; //-0.0918209702f;
        lefResult.m_b = -0.0990775824f; //-0.0901771560f;
        lefResult.m_c = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fy = 65.0f;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;
        }

        result = Ipvm::DataFitting::FitToLineRn(DATA_MAX, ptfc2Point, fSigma, lefData, ptfc2Data, &nDatanum);

        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];

        /*for (int i = 0; i < DATA_MAX; i++)
			{
			float fx = (float)i;
			float fy = (float)DATA_MAX / 10.0f - (float)i;

			if (i == 35) i++;
			if (i == 70) i++;

			ptfc2Point[i].m_x = fx;
			ptfc2Point[i].m_y = fy;
			}*/

        val = 0;

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            if (i == 70)
                val++;

            float fx = (float)val;
            float fy = (float)DATA_MAX / 10.0f - (float)val;

            if (i == 35)
                fx = 40;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;

            val++;
        }

        msg.Format(_T("Error_xy_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_xy_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_xy_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_xy_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);
        /*msg.Format(_T("Error_xy_Datasize D : R = %d : %d"), DATA_MAX - 2, nDatanum);
			Assert::AreEqual(DATA_MAX - 2, (int)nDatanum, msg);*/

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(
                _T("Error_xy_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Point[i].m_x);
            Assert::AreEqual(ptfc2Point[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(
                _T("Error_xy_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Point[i].m_y);
            Assert::AreEqual(ptfc2Point[i].m_y, ptfc2Data[i].m_y, msg);
        }
        ////===================================================================================================

        ////===================================================================================================
        ////저장소 값이 1개 있을 경우
        delete[] ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[1];
        delete[] ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[1];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = 0;
        lefResult.m_b = 0;
        lefResult.m_c = 0;

        ptfc2Point[0].m_x = 10;
        ptfc2Point[0].m_y = 20;

        result = Ipvm::DataFitting::FitToLineRn(1, ptfc2Point, fSigma, lefData, ptfc2Data, &nDatanum);

        msg.Format(_T("Error_1_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);
        ////===================================================================================================

        ////===================================================================================================
        ////저장소 값이 0값만 있을 경우
        delete[] ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        delete[] ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = 0;
        lefResult.m_b = 0;
        lefResult.m_c = 0;

        for (int i = 0; i < DATA_MAX; i++)
        {
            ptfc2Point[i].m_x = 0;
            ptfc2Point[i].m_y = 0;
        }

        result = Ipvm::DataFitting::FitToLineRn(1, ptfc2Point, fSigma, lefData, ptfc2Data, &nDatanum);

        msg.Format(_T("Error_0_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);
        ////===================================================================================================
    }

    TEST_METHOD (FitToLine_RN_LineEq_32f)
    {
        //ax + by+ c = 0
        Ipvm::Point32r2* ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        Ipvm::LineEq32r lefSrc;
        lefSrc.m_a = -0.1f;
        lefSrc.m_b = -0.1f;
        lefSrc.m_c = 1.0f;
        float fSigma = 1.f;

        Ipvm::Point32r2* ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        Ipvm::LineEq32r lefData;
        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;
        int32_t nDatanum = 0;

        Ipvm::LineEq32r lefResult;
        lefResult.m_a = -0.1000000015f; //-0.0999949649f;
        lefResult.m_b = -0.1000000015f; //-0.0999933705f;
        lefResult.m_c = 1.0f;
        //===================================================================================================
        //-0.1x - 0.1y + 1 = 0;
        //x값 노이즈 있을시
        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fx = 65.0f;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;
        }

        auto result
            = Ipvm::DataFitting::FitToLineRn(DATA_MAX, ptfc2Point, lefSrc, fSigma, lefData, ptfc2Data, &nDatanum);

        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];

        /*for (int i = 0; i < DATA_MAX; i++)
			{
			float fx = (float)i;
			float fy = (float)DATA_MAX / 10.0f - (float)i;

			if (i == 35) i++;
			if (i == 70) i++;

			ptfc2Point[i].m_x = fx;
			ptfc2Point[i].m_y = fy;
			}*/

        int val = 0;

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            if (i == 35)
                val++;
            if (i == 69)
                val++;

            float fx = (float)val;
            float fy = (float)DATA_MAX / 10.0f - (float)val;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;

            val++;
        }

        CString msg;

        msg.Format(_T("Error_x_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_x_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_x_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_x_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);
        /*msg.Format(_T("Error_x_Datasize D : R = %d : %d"), DATA_MAX - 2, nDatanum);
			Assert::AreEqual(DATA_MAX - 2, (int)nDatanum, msg);*/

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(_T("Error_x_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Point[i].m_x);
            Assert::AreEqual(ptfc2Point[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(_T("Error_x_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Point[i].m_y);
            Assert::AreEqual(ptfc2Point[i].m_y, ptfc2Data[i].m_y, msg);
        }
        //===================================================================================================

        ////===================================================================================================
        ////y값 노이즈 있을 시
        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        delete ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = -0.1000000015f; //-0.0861174464f;
        lefResult.m_b = -0.1000000015f; //-0.0832529590f;
        lefResult.m_c = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fy = 40.0f;
            if (i == 70)
                fy = 65.0f;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;
        }

        result = Ipvm::DataFitting::FitToLineRn(DATA_MAX, ptfc2Point, lefSrc, fSigma, lefData, ptfc2Data, &nDatanum);

        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];

        /*for (int i = 0; i < DATA_MAX; i++)
			{
			float fx = (float)i;
			float fy = (float)DATA_MAX / 10.0f - (float)i;

			if (i == 35) i++;
			if (i == 70) i++;

			ptfc2Point[i].m_x = fx;
			ptfc2Point[i].m_y = fy;
			}*/

        val = 0;

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            if (i == 35)
                val++;
            if (i == 69)
                val++;

            float fx = (float)val;
            float fy = (float)DATA_MAX / 10.0f - (float)val;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;

            val++;
        }

        msg.Format(_T("Error_y_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_y_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_y_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_y_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);
        /*msg.Format(_T("Error_y_Datasize D : R = %d : %d"), DATA_MAX - 2, nDatanum);
			Assert::AreEqual(DATA_MAX - 2, (int)nDatanum, msg);*/

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(_T("Error_y_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Point[i].m_x);
            Assert::AreEqual(ptfc2Point[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(_T("Error_y_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Point[i].m_y);
            Assert::AreEqual(ptfc2Point[i].m_y, ptfc2Data[i].m_y, msg);
        }
        ////===================================================================================================

        ////===================================================================================================
        ////x, y값 노이즈 있을 시
        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        delete ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = -0.0991630629f; //-0.0918209702f;
        lefResult.m_b = -0.0990775824f; //-0.0901771560f;
        lefResult.m_c = 1.0f;

        for (int i = 0; i < DATA_MAX; i++)
        {
            float fx = (float)i;
            float fy = (float)DATA_MAX / 10.0f - (float)i;

            if (i == 35)
                fx = 40.0f;
            if (i == 70)
                fy = 65.0f;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;
        }

        result = Ipvm::DataFitting::FitToLineRn(DATA_MAX, ptfc2Point, lefSrc, fSigma, lefData, ptfc2Data, &nDatanum);

        delete ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];

        /*for (int i = 0; i < DATA_MAX; i++)
			{
			float fx = (float)i;
			float fy = (float)DATA_MAX / 10.0f - (float)i;

			if (i == 35) i++;
			if (i == 70) i++;

			ptfc2Point[i].m_x = fx;
			ptfc2Point[i].m_y = fy;
			}*/

        val = 0;

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            if (i == 70)
                val++;

            float fx = (float)val;
            float fy = (float)DATA_MAX / 10.0f - (float)val;

            if (i == 35)
                fx = 40;

            ptfc2Point[i].m_x = fx;
            ptfc2Point[i].m_y = fy;

            val++;
        }

        msg.Format(_T("Error_xy_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_xy_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_xy_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_xy_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);
        /*msg.Format(_T("Error_xy_Datasize D : R = %d : %d"), DATA_MAX - 2, nDatanum);
			Assert::AreEqual(DATA_MAX - 2, (int)nDatanum, msg);*/

        for (int i = 0; i < DATA_MAX - 2; i++)
        {
            msg.Format(
                _T("Error_xy_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Point[i].m_x);
            Assert::AreEqual(ptfc2Point[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(
                _T("Error_xy_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Point[i].m_y);
            Assert::AreEqual(ptfc2Point[i].m_y, ptfc2Data[i].m_y, msg);
        }
        ////===================================================================================================

        ////===================================================================================================
        ////저장소 값이 1개 있을 경우
        delete[] ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[1];
        delete[] ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[1];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = 0;
        lefResult.m_b = 0;
        lefResult.m_c = 0;

        ptfc2Point[0].m_x = 10;
        ptfc2Point[0].m_y = 20;

        result = Ipvm::DataFitting::FitToLineRn(1, ptfc2Point, lefSrc, fSigma, lefData, ptfc2Data, &nDatanum);

        msg.Format(_T("Error_1_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        ////===================================================================================================

        ////===================================================================================================
        ////저장소 값이 0값만 있을 경우
        delete[] ptfc2Point;
        ptfc2Point = new Ipvm::Point32r2[DATA_MAX];
        delete[] ptfc2Data;
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0.f;
        lefData.m_b = 0.f;
        lefData.m_c = 0.f;

        lefResult.m_a = 0;
        lefResult.m_b = 0;
        lefResult.m_c = 0;

        for (int i = 0; i < DATA_MAX; i++)
        {
            ptfc2Point[i].m_x = 0;
            ptfc2Point[i].m_y = 0;
        }

        result = Ipvm::DataFitting::FitToLineRn(1, ptfc2Point, lefSrc, fSigma, lefData, ptfc2Data, &nDatanum);

        msg.Format(_T("Error_0_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        ////===================================================================================================
    }
#pragma endregion

#pragma region GetDistance
    //mGetDistance_PointToLine와 똑같음
    TEST_METHOD (GetDistance_PointToLine)
    {
        float fDatax0 = 20.0f;
        float fDatay0 = 40.0f;

        Ipvm::LineEq32r line(5.f, 4.f, 3.f);

        float fResult = 41.0736993767f;
        //===================================================================================================
        //정상적인 값이 대입 된 경우

        auto result = CPI_Geometry::GetDistance_PointToLine(fDatax0, fDatay0, line);

        CString msg;

        msg.Format(_T("Error_Result D : R = %0.10f : %0.10f"), result, fResult);
        Assert::AreEqual(fResult, result, msg);
        //===================================================================================================

        //===================================================================================================
        //비정상적인 값이 대입 된 경우
        line = Ipvm::LineEq32r(0.f, 0.f, 0.f);
        fResult = -1.f;

        result = CPI_Geometry::GetDistance_PointToLine(fDatax0, fDatay0, line);

        msg.Format(_T("Error_Result D : R = %0.10f : %0.10f"), result, fResult);
        Assert::AreEqual(fResult, result, msg);
        //===================================================================================================
    }

    //1.GetDistance(const LineEq_32f& LNSrc, const Point_32f_C2& PTSrcOrign, float &distance);
    //2.GetDistanceNoAbs(const LineEq_32f& LNSrc, const Point_32f_C2& ptSrcOrign, float &distance);
    //3.GetDistance(const LineSeg& i_srcSegment, const Point_32f_C2& i_ptSrcXY, float &distance); -> ?
    //4.GetDistance_AlongTheSegment(const LineSeg& SGSrc, const Point_32f_C2& ptSrcOrign, float &distance);
    TEST_METHOD (GetDistance_LNvPT_1)
    {
        Ipvm::LineEq32r lefData(5.f, 4.f, 3.f);
        Ipvm::Point32r2 ptfc2Data(20.f, 40.f);
        float fDist = 0;

        float fResult = 41.0736993767f;
        //===================================================================================================
        //정상적으로 값이 들어갈 때

        auto result = Ipvm::Geometry::GetDistance(lefData, ptfc2Data, fDist);

        CString msg;

        msg.Format(_T("Error_s_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_s_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================

        //===================================================================================================
        //0 값이 들어갈 때
        ptfc2Data.m_x = 0.f;
        ptfc2Data.m_y = 0.f;

        fResult = 0.4685212672f;

        result = Ipvm::Geometry::GetDistance(lefData, ptfc2Data, fDist);

        //예전 알고리즘으로는 실패해야 함
        msg.Format(_T("Error_0_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_0_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================
    }

    //? 1번과 먼 차이인지 모르겠습니다.
    TEST_METHOD (GetDistance_LNvPT_2)
    {
        Ipvm::LineEq32r lefData(5.f, 4.f, 3.f);
        Ipvm::Point32r2 ptfc2Data(20.f, 40.f);
        float fDist = 0;

        float fResult = 41.0736993767f;
        //===================================================================================================
        //정상적으로 값이 들어갈 때

        auto result = Ipvm::Geometry::GetDistanceNoAbs(lefData, ptfc2Data, fDist);

        CString msg;

        msg.Format(_T("Error_s_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_s_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================

        //===================================================================================================
        //0 값이 들어갈 때
        ptfc2Data.m_x = 0.f;
        ptfc2Data.m_y = 0.f;

        fResult = 0.4685212672f;

        result = Ipvm::Geometry::GetDistanceNoAbs(lefData, ptfc2Data, fDist);

        //예전 알고리즘으로는 실패해야 함
        msg.Format(_T("Error_0_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_0_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================
    }

    TEST_METHOD (GetDistance_LNvPT_3)
    {
        Ipvm::LineSeg32r lsData(-80.6f, 100.f, 79.4f, -100.f);
        Ipvm::Point32r2 ptfc2Data(20.f, 40.f);
        float fDist = 0;

        float fResult = 41.0736993767f;
        //===================================================================================================
        //정상적으로 값이 들어갈 때

        auto result = Ipvm::Geometry::GetDistance(lsData, ptfc2Data, fDist);

        CString msg;

        msg.Format(_T("Error_s_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_s_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================

        //===================================================================================================
        //0 값이 들어갈 때
        ptfc2Data.m_x = 0.f;
        ptfc2Data.m_y = 0.f;

        fResult = 0.4685212970f;

        result = Ipvm::Geometry::GetDistance(lsData, ptfc2Data, fDist);

        //예전 알고리즘으로는 실패해야 함
        msg.Format(_T("Error_0_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_0_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================
    }

    TEST_METHOD (GetDistance_LNvPT_4)
    {
        Ipvm::LineSeg32r lsData(-80.6f, 100.f, 79.4f, -100.f);
        Ipvm::Point32r2 ptfc2Data(20.f, 40.f);
        float fDist = 0;

        float fResult = 109.6964492798f;
        //===================================================================================================
        //정상적으로 값이 들어갈 때

        auto result = Ipvm::Geometry::GetDistance_AlongTheSegment(lsData, ptfc2Data, fDist);

        CString msg;

        msg.Format(_T("Error_s_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_s_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================

        //===================================================================================================
        //0 값이 들어갈 때
        ptfc2Data.m_x = 0.f;
        ptfc2Data.m_y = 0.f;

        fResult = 128.4373016357f;

        result = Ipvm::Geometry::GetDistance_AlongTheSegment(lsData, ptfc2Data, fDist);

        //예전 알고리즘으로는 실패해야 함
        msg.Format(_T("Error_0_fResult D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_0_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================
    }
#pragma endregion

    //FitToLine_RANSAC이랑 비슷함
    //FitToLine_RANSAC은 Fitting시 일정 거리 안에 있는 점의 갯수를 구함
    //RoughLineFitting은 Fitting시 거리를 점수로 매겨 특정 점수 안에 있는 것으로 구함
#pragma region RoughLineFitting
    TEST_METHOD (FitToLine_RANSAC)
    {
        Ipvm::Point32r2* ptfc2DataXY = new Ipvm::Point32r2[DATA_MAX];
        float nDistThreshold = 5.f;

        Ipvm::Point32r2* ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        Ipvm::LineEq32r lefData(0.f, 0.f, 0.f);
        int32_t fDatanum = 0;

        long nIdx = 0;
        Ipvm::Point32r2* ptfc2Result = new Ipvm::Point32r2[DATA_MAX];
        Ipvm::LineEq32r lefResult(-0.1000000015f, -0.1000000015f, 1.0f);
        //===================================================================================================
        //x값 노이즈 있을시 bUseVerity = 0;
        //-0.1x - 0.1y + 1 = 0;
        for (int i = 0; i < DATA_MAX; i++)
        {
            ptfc2DataXY[i].m_x = (float)i;
            ptfc2DataXY[i].m_y = (float)(DATA_MAX / 10 - i);

            if (i != 35 && i != 70)
            {
                ptfc2Result[nIdx].m_x = (float)i;
                ptfc2Result[nIdx++].m_y = (float)(DATA_MAX / 10 - i);
            }

            if (i == 35)
                ptfc2DataXY[i].m_x = 256;
            if (i == 70)
                ptfc2DataXY[i].m_x = -256;
        }

        auto result
            = Ipvm::DataFitting::FitToLineRansac(DATA_MAX, ptfc2DataXY, ptfc2Data, fDatanum, lefData, nDistThreshold);

        CString msg;

        msg.Format(_T("Error_x_Result_f D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_x_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_x_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_x_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);

        for (int i = 0; i < fDatanum; i++)
        {
            msg.Format(
                _T("Error_x_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Result[i].m_x);
            Assert::AreEqual(ptfc2Result[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(
                _T("Error_x_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Result[i].m_y);
            Assert::AreEqual(ptfc2Result[i].m_y, ptfc2Data[i].m_y, msg);
        }

        delete ptfc2Result;
        delete ptfc2DataXY;
        delete ptfc2Data;
        //===================================================================================================

        //===================================================================================================
        //y값 노이즈 있을시 bUseVerity = 0;
        //-0.1x - 0.1y + 1 = 0;
        ptfc2DataXY = new Ipvm::Point32r2[DATA_MAX];
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        ptfc2Result = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0;
        lefData.m_b = 0;
        lefData.m_c = 0;

        nIdx = 0;

        for (int i = 0; i < DATA_MAX; i++)
        {
            ptfc2DataXY[i].m_x = (float)i;
            ptfc2DataXY[i].m_y = (float)(DATA_MAX / 10 - i);

            if (i != 35 && i != 70)
            {
                ptfc2Result[nIdx].m_x = (float)i;
                ptfc2Result[nIdx++].m_y = (float)(DATA_MAX / 10 - i);
            }

            if (i == 35)
                ptfc2DataXY[i].m_y = 256;
            if (i == 70)
                ptfc2DataXY[i].m_y = -256;
        }

        result
            = Ipvm::DataFitting::FitToLineRansac(DATA_MAX, ptfc2DataXY, ptfc2Data, fDatanum, lefData, nDistThreshold);

        msg.Format(_T("Error_y_Result_f D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_y_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_y_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_y_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);

        for (int i = 0; i < fDatanum; i++)
        {
            msg.Format(
                _T("Error_x_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Result[i].m_x);
            Assert::AreEqual(ptfc2Result[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(
                _T("Error_x_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Result[i].m_y);
            Assert::AreEqual(ptfc2Result[i].m_y, ptfc2Data[i].m_y, msg);
        }

        delete[] ptfc2Result;
        delete[] ptfc2DataXY;
        delete[] ptfc2Data;
        //===================================================================================================

        //===================================================================================================
        //x, y값 노이즈 있을시 bUseVerity = 0;
        //-0.1x - 0.1y + 1 = 0;
        ptfc2DataXY = new Ipvm::Point32r2[DATA_MAX];
        ptfc2Data = new Ipvm::Point32r2[DATA_MAX];
        ptfc2Result = new Ipvm::Point32r2[DATA_MAX];

        lefData.m_a = 0;
        lefData.m_b = 0;
        lefData.m_c = 0;

        nIdx = 0;

        for (int i = 0; i < DATA_MAX; i++)
        {
            ptfc2DataXY[i].m_x = (float)i;
            ptfc2DataXY[i].m_y = (float)(DATA_MAX / 10 - i);

            if (i != 35 && i != 70)
            {
                ptfc2Result[nIdx].m_x = (float)i;
                ptfc2Result[nIdx++].m_y = (float)(DATA_MAX / 10 - i);
            }

            if (i == 35)
                ptfc2DataXY[i].m_x = 256;
            if (i == 70)
                ptfc2DataXY[i].m_y = -256;
        }

        result
            = Ipvm::DataFitting::FitToLineRansac(DATA_MAX, ptfc2DataXY, ptfc2Data, fDatanum, lefData, nDistThreshold);

        msg.Format(_T("Error_xy_Result_f D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_xy_lefData.m_a D : R = %0.10f : %0.10f"), lefData.m_a, lefResult.m_a);
        Assert::AreEqual(lefResult.m_a, lefData.m_a, msg);
        msg.Format(_T("Error_xy_lefData.m_b D : R = %0.10f : %0.10f"), lefData.m_b, lefResult.m_b);
        Assert::AreEqual(lefResult.m_b, lefData.m_b, msg);
        msg.Format(_T("Error_xy_lefData.m_c D : R = %0.10f : %0.10f"), lefData.m_c, lefResult.m_c);
        Assert::AreEqual(lefResult.m_c, lefData.m_c, msg);

        for (int i = 0; i < fDatanum; i++)
        {
            msg.Format(
                _T("Error_x_ptfc2Data[%d].m_x D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_x, ptfc2Result[i].m_x);
            Assert::AreEqual(ptfc2Result[i].m_x, ptfc2Data[i].m_x, msg);
            msg.Format(
                _T("Error_x_ptfc2Data[%d].m_y D : R = %0.10f : %0.10f"), i, ptfc2Data[i].m_y, ptfc2Result[i].m_y);
            Assert::AreEqual(ptfc2Result[i].m_y, ptfc2Data[i].m_y, msg);
        }

        delete[] ptfc2Result;
        delete[] ptfc2DataXY;
        delete[] ptfc2Data;
        //===================================================================================================
    }
#pragma endregion

    TEST_METHOD (GetCrossPointByTwoStraightLines_IPVM)
    {
        float fDataa1 = 2.f;
        float fDatab1 = 4.f;
        float fDatac1 = -20.f;
        float fDataa2 = -5.f;
        float fDatab2 = 4.f;
        float fDatac2 = 5.f;

        Ipvm::LineEq32r lefLine1(fDataa1, fDatab1, fDatac1);
        Ipvm::LineEq32r lefLine2(fDataa2, fDatab2, fDatac2);

        float fDatax = 0.0f;
        float fDatay = 0.0f;

        Ipvm::Point32r2 ptfc2Data(fDatax, fDatay);

        float fResultx = 3.5714285374f;
        float fResulty = 3.2142856121f;

        Ipvm::Point32r2 ptfc2Result(fResultx, fResulty);
        //===================================================================================================
        //교차되는 직선 두개를 이용할 때

        auto result = Ipvm::Geometry::GetCrossPoint(lefLine1, lefLine2, ptfc2Data);
        //CPI_Geometry::GetCrossPointByTwoStraightLines(fDataa1, fDatab1, fDatac1, fDataa2, fDatab2, fDatac2, fDatax, fDatay);

        CString msg;

        /*msg.Format(_T("Error_s_Result D : R = %d : %d"), result, 0L);
			Assert::AreEqual(0L, result, msg);
			msg.Format(_T("Error_s_fDatax D : R = %0.10f : %0.10f"), fDatax, fResultx);
			Assert::AreEqual(fDatax, fResultx, msg);
			msg.Format(_T("Error_s_fDatay D : R = %0.10f : %0.10f"), fDatay, fResulty);
			Assert::AreEqual(fDatay, fResulty, msg);*/
        msg.Format(_T("Error_c_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_c_ptfc2Data.m_x D : R = %0.10f : %0.10f"), ptfc2Data.m_x, ptfc2Result.m_x);
        Assert::AreEqual(ptfc2Data.m_x, ptfc2Result.m_x, msg);
        msg.Format(_T("Error_c_ptfc2Data.m_y D : R = %0.10f : %0.10f"), ptfc2Data.m_y, ptfc2Result.m_y);
        Assert::AreEqual(ptfc2Data.m_y, ptfc2Result.m_y, msg);
        //===================================================================================================

        //===================================================================================================
        //교차되지 않는 직선 두개를 이용할 때
        fDataa1 = 2.f;
        fDatab1 = 4.f;
        fDatac1 = -20.f;
        fDataa2 = 4.f;
        fDatab2 = 8.f;
        fDatac2 = -20.f;

        lefLine1.m_a = fDataa1;
        lefLine1.m_b = fDatab1;
        lefLine1.m_c = fDatac1;
        lefLine2.m_a = fDataa2;
        lefLine2.m_b = fDatab2;
        lefLine2.m_c = fDatac2;

        result = Ipvm::Geometry::GetCrossPoint(lefLine1, lefLine2, ptfc2Data);
        //CPI_Geometry::GetCrossPointByTwoStraightLines(fDataa1, fDatab1, fDatac1, fDataa2, fDatab2, fDatac2, fDatax, fDatay);

        /*msg.Format(_T("Error_f_Result_p D : R = %d : %d"), result, 0L);
			Assert::AreNotEqual(0L, result, msg);*/
        msg.Format(_T("Error_p_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);
        //===================================================================================================

        //===================================================================================================
        //정상적이지 않은 값이 들어갈 때
        fDataa1 = 0.f;
        fDatab1 = 0.f;
        fDataa2 = 0.f;
        fDatab2 = 0.f;

        lefLine1.m_a = fDataa1;
        lefLine1.m_b = fDatab1;
        lefLine2.m_a = fDataa2;
        lefLine2.m_b = fDatab2;

        result = Ipvm::Geometry::GetCrossPoint(lefLine1, lefLine2, ptfc2Data);
        //CPI_Geometry::GetCrossPointByTwoStraightLines(fDataa1, fDatab1, fDatac1, fDataa2, fDatab2, fDatac2, fDatax, fDatay);

        /*msg.Format(_T("Error_f_Result_z D : R = %d : %d"), result, 0L);
			Assert::AreNotEqual(0L, result, msg);*/
        msg.Format(_T("Error_0_Result D : R = %d : %d"), result, Ipvm::Status::determinantZero);
        Assert::AreEqual(Ipvm::Status::determinantZero, result, msg);
        //===================================================================================================
    }

    //mGetDistance_PointToLine는 해당 Point가 Line에서 얼마나 떨어져 있는지 연산하는 함수
    //GetDistPointOnLine는 해당 Line에서 해당 Distance만큼 떨어져 있는 Point의 좌표를 연산하는 함수
    TEST_METHOD (GetDistPointOnLine)
    {
        float fDatax0 = 20.0f;
        float fDatay0 = 20.0f;
        float fDataa = 2.f;
        float fDatab = 4.f;
        float fDistance = 5.f;
        BOOL bVer = TRUE;

        float fDatax = 0.0f;
        float fDatay = 0.0f;

        float fResultx = 15.5278644562f;
        float fResulty = 22.2360687256f;
        //===================================================================================================
        //bVer = TRUE

        auto result
            = CPI_Geometry::GetDistPointOnLine(fDatax0, fDatay0, fDataa, fDatab, fDistance, bVer, fDatax, fDatay);

        CString msg;

        msg.Format(_T("Error_t_Result D : R = %d : %d"), result, 0L);
        Assert::AreEqual(0L, result, msg);
        msg.Format(_T("Error_t_fDatax D : R = %0.10f : %0.10f"), fDatax, fResultx);
        Assert::AreEqual(fDatax, fResultx, msg);
        msg.Format(_T("Error_t_fDatay D : R = %0.10f : %0.10f"), fDatay, fResulty);
        Assert::AreEqual(fDatay, fResulty, msg);
        //===================================================================================================

        //===================================================================================================
        //bVer = FALSE
        bVer = FALSE;

        fResultx = 24.4721355438f;
        fResulty = 17.7639312744f;

        result = CPI_Geometry::GetDistPointOnLine(fDatax0, fDatay0, fDataa, fDatab, fDistance, bVer, fDatax, fDatay);

        msg.Format(_T("Error_f_Result D : R = %d : %d"), result, 0L);
        Assert::AreEqual(0L, result, msg);
        msg.Format(_T("Error_f_fDatax D : R = %0.10f : %0.10f"), fDatax, fResultx);
        Assert::AreEqual(fDatax, fResultx, msg);
        msg.Format(_T("Error_f_fDatay D : R = %0.10f : %0.10f"), fDatay, fResulty);
        Assert::AreEqual(fDatay, fResulty, msg);
        //===================================================================================================
    }

    TEST_METHOD (GetDistance_PointToPoint_IPVM)
    {
        float fDatax1 = 20.0f;
        float fDatay1 = 20.0f;
        float fDatax2 = 40.f;
        float fDatay2 = 40.f;

        Ipvm::Point32r2 ptfc2Data1(fDatax1, fDatay1);
        Ipvm::Point32r2 ptfc2Data2(fDatax2, fDatay2);
        float fDist = 0;

        float fResult = 28.2842712402f;
        //===================================================================================================
        //x1 != x2 && y1 != y2
        auto result = Ipvm::Geometry::GetDistance(ptfc2Data1, ptfc2Data2, fDist);
        //CPI_Geometry::GetDistance_PointToPoint(fDatax1, fDatay1, fDatax2, fDatay2);
        CString msg;

        /*msg.Format(_T("Error_ne_Result D : R = %0.10f : %0.10f"), result, fResult);
			Assert::AreEqual(fResult, result, msg);*/
        msg.Format(_T("Error_ne_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_ne_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================

        //===================================================================================================
        //x1 == x2 && y1 == y2
        fDatax1 = 20.0f;
        fDatay1 = 20.0f;
        fDatax2 = 20.f;
        fDatay2 = 20.f;

        ptfc2Data1.m_x = fDatax1;
        ptfc2Data1.m_y = fDatay1;
        ptfc2Data2.m_x = fDatax2;
        ptfc2Data2.m_y = fDatay2;

        fResult = 0.f;

        result = Ipvm::Geometry::GetDistance(ptfc2Data1, ptfc2Data2, fDist);
        //CPI_Geometry::GetDistance_PointToPoint(fDatax1, fDatay1, fDatax2, fDatay2);

        /*msg.Format(_T("Error_e_Result D : R = %0.10f : %0.10f"), result, fResult);
			Assert::AreEqual(fResult, result, msg);*/
        msg.Format(_T("Error_e_Result D : R = %d : %d"), result, Ipvm::Status::e_ok);
        Assert::AreEqual(Ipvm::Status::e_ok, result, msg);
        msg.Format(_T("Error_e_fDist D : R = %0.10f : %0.10f"), fDist, fResult);
        Assert::AreEqual(fResult, fDist, msg);
        //===================================================================================================
    }

    //2점 사이의 각 찾기
    TEST_METHOD (Get1LineAngle)
    {
        float fDatax1 = 20.f;
        float fDatay1 = 20.f;
        float fDatax2 = 40.f;
        float fDatay2 = 80.f;

        float fAngle = 0.f;

        float fResulta = 18.4349479675f;
        //===================================================================================================
        CPI_Geometry::Get1LineAngle(fDatax1, fDatay1, fDatax2, fDatay2, fAngle);

        CString msg;

        msg.Format(_T("Error_ne_fAngle D : R = %0.10f : %0.10f"), fAngle, fResulta);
        Assert::AreEqual(fAngle, fResulta, msg);
        //===================================================================================================
    }

    //두 선 사이의 각 찾기
    TEST_METHOD (GetAngleBetween2Lines)
    {
        Ipvm::Point32r2 pt1(20.f, 20.f);
        Ipvm::Point32r2 pt2(40.f, 80.f);
        Ipvm::Point32r2 pt3(60.f, 120.f);

        float fResult = 171.8699035645f;
        //===================================================================================================
        //정상값 들어갈 때
        auto result = CPI_Geometry::GetAngleBetween2Lines(pt1, pt2, pt3);

        CString msg;

        msg.Format(_T("Error_s_fresult D : R = %0.10f : %0.10f"), result, fResult);
        Assert::AreEqual(fResult, result, msg);
        //===================================================================================================

        //===================================================================================================
        //0값 들어갈 때
        pt1 = Ipvm::Point32r2(0.f, 0.f);
        pt2 = Ipvm::Point32r2(0.f, 0.f);
        pt3 = Ipvm::Point32r2(0.f, 0.f);

        fResult = -99999.f;

        result = CPI_Geometry::GetAngleBetween2Lines(pt1, pt2, pt3);

        msg.Format(_T("Error_f_fresult D : R = %0.10f : %0.10f"), result, fResult);
        Assert::AreEqual(fResult, result, msg);
        //===================================================================================================
    }
};
} // namespace UnitTestGeometry
