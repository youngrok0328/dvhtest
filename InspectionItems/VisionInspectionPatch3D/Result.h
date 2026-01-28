#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionPatch3DPara.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionScale;

//HDR_6_________________________________ Header body
//
class Result
{
public:
    Result();
    ~Result();

    void Initialize(const VisionScale& scale, const VisionInspectionPatch3DPara& para);

    // m_vecsPatch3DGroupInfo_um로 계산한 값
    std::vector<SInterposerRefROI_Info> m_vecsInterposerRefROI_Info_pxl;

private:
    void ConvertionInterposerROIInfoUmToPxl(
        const VisionScale& scale, const std::vector<SInterposerRefROI_Info>& vecsPatch3DGroupInfo_um);
};
