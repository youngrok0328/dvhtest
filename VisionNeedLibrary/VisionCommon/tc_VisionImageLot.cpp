#include "stdafx.h"
#include "../UnitTest/UnitTestCommon.h"
#include "VisionImageLot.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace UnitTestVisionCommon
{		
	TEST_CLASS(tc_VisionImageLot)
	{
	public:
		TEST_METHOD(ConvertHandlerStitchIndexToVisionStitchIndex)
		{
			VisionImageLot imageLot;

			const long handlerStitchCountX = 3;
			const long handlerStitchCountY = 2;

			const long visionStitchCountX = handlerStitchCountY;
			const long visionStitchCountY = handlerStitchCountX;

			imageLot.Setup2D(100, 100, visionStitchCountX, visionStitchCountY, 2);

			// Handler stitch indices
			// 0 1 2 
			// 3 4 5

			// Vision stitch indices
			// 0 1
			// 2 3
			// 4 5

			Assert::AreEqual(4L, imageLot.ConvertHandlerStitchIndexToVisionStitchIndex(0));
			Assert::AreEqual(2L, imageLot.ConvertHandlerStitchIndexToVisionStitchIndex(1));
			Assert::AreEqual(0L, imageLot.ConvertHandlerStitchIndexToVisionStitchIndex(2));
			Assert::AreEqual(5L, imageLot.ConvertHandlerStitchIndexToVisionStitchIndex(3));
			Assert::AreEqual(3L, imageLot.ConvertHandlerStitchIndexToVisionStitchIndex(4));
			Assert::AreEqual(1L, imageLot.ConvertHandlerStitchIndexToVisionStitchIndex(5));
		}
	};
}