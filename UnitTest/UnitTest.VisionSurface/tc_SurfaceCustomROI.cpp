//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
#include "../../InspectionItems/VisionInspectionSurface/SurfaceCustomROI.h"
#include "../../InspectionItems/VisionInspectionSurface/SurfaceCustomROIItem.h"
#include "../UnitTestCommon.h"

#include <Ipvm/Base/Point32s2.h>

namespace UnitTestVisionSurface
{
TEST_CLASS (tc_SurfaceCustomROI)
{
public:
    TEST_METHOD (CustomROI_AddEllipse)
    {
        CSurfaceCustomROI data;

        Ipvm::Rect32s* original_data = new Ipvm::Rect32s[100];
        Ipvm::Rect32s* copy_data = new Ipvm::Rect32s[100];
        for (int n = 0; n < 100; n++)
        {
            original_data[n] = Ipvm::Rect32s(n, n, n + 10, n + 10);
            copy_data[n] = original_data[n];
        }

        // 1. Test1이라는 Ellipse을 100개 넣음
        //    데이터는 copy_data의 0~99 번 데이터를 넣었다
        Assert::AreEqual(TRUE, data.CustomROI_AddEllipse(_T("Test1"), copy_data, 100));

        // 객체가 하나인지 Test1 객체의 내부 수가 100개인지 확인함
        Assert::AreEqual(1, data.CustomROI_GetCount());
        Assert::IsNotNull(data.CustomROI_Search(_T("Test1")));
        Assert::AreEqual((size_t)100, data.CustomROI_Search(_T("Test1"))->vecShape.size());

        // 2. Test1이라는 Ellipse을 10개 넣음
        //    기본 동작은 같은 이름의 객체가 있을 경우 덮어 쓰게 되어 있음
        //    데이터는 copy_data의 0~9 번 데이터를 넣었다
        Assert::AreEqual(TRUE, data.CustomROI_AddEllipse(_T("Test1"), copy_data, 10));

        // 여전히 객체가 하나인지 Test1 객체의 내부 수가 10개로 바뀌었는지 확인함
        Assert::AreEqual(1, data.CustomROI_GetCount());
        Assert::IsNotNull(data.CustomROI_Search(_T("Test1")));
        Assert::AreEqual((size_t)10, data.CustomROI_Search(_T("Test1"))->vecShape.size());

        // 3. Test2이라는 Ellipse을 15개 넣음. 새로운 이름의 객체이므로 추가로 만들어 져야 함
        //    데이터는 copy_data의 10~14 번 데이터를 넣었다
        Assert::AreEqual(TRUE, data.CustomROI_AddEllipse(_T("Test2"), copy_data + 10, 5));

        // 객체가 두개인지 Test1, Test2 객체의 내부 수가 맞는지 확인함
        Assert::AreEqual(2, data.CustomROI_GetCount());
        Assert::IsNotNull(data.CustomROI_Search(_T("Test1")));
        Assert::IsNotNull(data.CustomROI_Search(_T("Test2")));
        Assert::AreEqual((size_t)10, data.CustomROI_Search(_T("Test1"))->vecShape.size());
        Assert::AreEqual((size_t)5, data.CustomROI_Search(_T("Test2"))->vecShape.size());

        delete[] copy_data;

        // 4. 입력으로 넣었던 데이터를 삭제하여도 SurfaceCustomROI에는 문제가 없는지와
        //    ROI가 문제 없는지를 같이 체크한다
        for (long object_idnex = 0; object_idnex < 2; object_idnex++)
        {
            CString name;
            name.Format(_T("Test%d"), object_idnex + 1);

            // ROI 가 Ellipse 타입으로 들어갔는지 체크
            Assert::IsNotNull(data.CustomROI_Search(name));
            Assert::AreEqual(1L, data.CustomROI_Search(name)->nType);

            for (long n = 0; n < long(data.CustomROI_Search(name)->vecShape.size()); n++)
            {
                auto& correct_answer = original_data[n + (object_idnex ? 10 : 0)];
                auto& shape_points = data.CustomROI_Search(name)->vecShape[n].vecptPos;

                Assert::AreEqual((size_t)2, shape_points.size());

                // Ellipse일 경우 Rect영역의 Left-Top, Right-Bottom의 두점이 각각 Shape에 들어간다
                Assert::AreEqual(correct_answer.TopLeft(), shape_points[0]);
                Assert::AreEqual(correct_answer.BottomRight(), shape_points[1]);
            }
        }

        delete[] original_data;
    }
};
} // namespace UnitTestVisionSurface