#pragma once
#include <vector>
#include "VisionInspectionLid3DPara.h"

class VisionScale;

class Result
{
public:
	Result();
	~Result();

	void Initialize(const VisionScale& scale, const VisionInspectionLid3DPara& para);

	// m_vecsLid3DGroupInfo_um로 계산한 값
	std::vector<SLid3DGroupInfo> m_vecsLid3DGroupInfo_pxl;

private:
	void ConvertionLid3DGroupInfo(const VisionScale& scale, const std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo_um);
};
