#include "stdafx.h"
#include "Result.h"
#include "../../DefineModules/dA_Base/VisionScale.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

Result::Result()
{
}


Result::~Result()
{
}

void Result::Initialize(const VisionScale& scale, const VisionInspectionLid3DPara& para)
{
	ConvertionLid3DGroupInfo(scale, para.m_vecsLid3DGroupInfo_um);
}

void Result::ConvertionLid3DGroupInfo(const VisionScale& scale, const std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo_um)
{
	long nGroupNum = (long)vecsLid3DGroupInfo_um.size();
	if (nGroupNum <= 0)
		return;

	const auto& um2px = scale.umToPixel();

	long nROINum = 0;
	VisionInspectionLid3DPara::SetLid3DGroupInfo(nGroupNum, m_vecsLid3DGroupInfo_pxl);

	for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
	{
		auto& lid3DGroupInfo_um = vecsLid3DGroupInfo_um[nGroup];
		auto& lid3DGroupInfo_pxl = m_vecsLid3DGroupInfo_pxl[nGroup];

		nROINum = (long)lid3DGroupInfo_um.vecLidROI_Info.size();
		if (nROINum <= 0) continue;

		VisionInspectionLid3DPara::SetLid3DROI_Info(nGroup, nROINum, m_vecsLid3DGroupInfo_pxl);
		lid3DGroupInfo_pxl.strID = lid3DGroupInfo_um.strID;

		for (long nROI = 0; nROI < nROINum; nROI++)
		{
			auto& lid3DROIInfo_um = lid3DGroupInfo_um.vecLidROI_Info[nROI];
			auto& lid3DROIInfo_pxl = lid3DGroupInfo_pxl.vecLidROI_Info[nROI];

			lid3DROIInfo_pxl.strID = lid3DROIInfo_um.strID;
			lid3DROIInfo_pxl.fOffsetX = lid3DROIInfo_um.fOffsetX * um2px.m_x;
			lid3DROIInfo_pxl.fOffsetY = lid3DROIInfo_um.fOffsetY * um2px.m_y;
			lid3DROIInfo_pxl.fWidth = lid3DROIInfo_um.fWidth * um2px.m_x;
			lid3DROIInfo_pxl.fLength = lid3DROIInfo_um.fLength * um2px.m_y;
		}
	}
}