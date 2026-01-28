#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionMarkPara.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionScale;

//HDR_6_________________________________ Header body
//
void ConvertionMarkMapInfo(
    const VisionScale& scale, const sMarkMapInfo& i_RealData, BOOL i_isUMtoPX, sMarkMapInfo& o_ConvertData);
void ConvertionMarkMapInfo(
    const VisionScale& scale, const sReadMarkMapInfo& i_DataInfo_um, sReadMarkMapInfo& o_ConvertData_px);

class InspResult
{
public:
    InspResult();
    ~InspResult();

    void Alloc(long charectorNum);
    void InitializeMarkInfoforPixel(const VisionScale& scale, const VisionInspectionMarkPara& para);

    std::vector<Ipvm::Rect32s> m_vecrtCharPositionforCalcImage; // 검사영상에 대한 문자 좌표
    std::vector<Ipvm::Rect32s> m_vecrtCharPositionforImage; // 실제 원본 영상에 대한 문자 좌표
    std::vector<Ipvm::Rect32s> m_vecroiCharSearch; // Charector 별 Search ROI

    std::vector<float> m_vecfFirstTrialUnderPrint;
    std::vector<float> m_vecfFirstTrialOverPrint;

    sMarkMapInfo m_sMarkMapInfo_px; //계산용
};
