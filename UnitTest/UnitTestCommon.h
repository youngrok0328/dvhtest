#include "stdafx.h"
#include "CppUnitTest.h"

#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework
		{
			template<> inline std::wstring ToString<CPoint>(const CPoint& t)
			{
				static CStringW error;
				error.Format(_T("CPoint (%d, %d)"), t.x, t.y);
				return error.GetBuffer();
			}

			template<> inline std::wstring ToString<Ipvm::Point32s2>(const Ipvm::Point32s2& t)
			{
				static CStringW error;
				error.Format(_T("Ipvm::Point32s2 (%d, %d)"), t.m_x, t.m_y);
				return error.GetBuffer();
			}

			template<> inline std::wstring ToString<CRect>(const CRect& t)
			{
				static CStringW error;
				error.Format(_T("CRect (%d, %d, %d, %d)"), t.left, t.top, t.right, t.bottom);
				return error.GetBuffer();
			}

			template<> inline std::wstring ToString<Ipvm::Rect32s>(const Ipvm::Rect32s& t)
			{
				static CStringW error;
				error.Format(_T("CRect (%d, %d, %d, %d)"), t.m_left, t.m_top, t.m_right, t.m_bottom);
				return error.GetBuffer();
			}
		}
	}
}

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
