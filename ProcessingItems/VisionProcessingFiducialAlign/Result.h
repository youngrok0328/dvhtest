#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../InformationModule/dPI_DataBase/Enum.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class Result_Pad
{
public:
    bool m_enable; // 활성화된 PAD 인가
    CString m_name;
    PAD_TYPE m_type;
    FPI_RECT m_spec_px;
    FPI_RECT m_align_px;
};

class Result
{
public:
    Result();
    ~Result();

    void Init();

    Ipvm::Point32r2 m_bodyCenterBefore;
    std::vector<Result_Pad> m_pads;
    Ipvm::Point32r2 m_applyAlignTobodyCenter;
    FPI_RECT m_applyAlignTobodyRoi;

    // Debug Result 로 보여주는데 사용할 데이터
    std::vector<FPI_RECT> m_debugResult_spec_pad_rois;
    std::vector<Ipvm::Rect32s> m_debugResult_select_pad_rois;
    std::vector<Ipvm::Rect32s> m_debugResult_search_pad_rois;
    std::vector<Ipvm::Rect32s> m_debugResult_final_pad_rois;
    //std::vector<Ipvm::LineSeg32r>		m_debugResult_edgeDetectionLines;
    //std::vector<Ipvm::Point32r2>	m_debugResult_edgePoints;
    //std::vector<Ipvm::EllipseEq32r>	m_debugResult_align_Ellipses;
    //std::vector<Ipvm::Polygon32r>		m_debugResult_align_Polygons;
    //std::vector<FPI_RECT>			m_debugResult_align_Other;
    std::vector<Ipvm::Point32r2> m_debugResult_align_Center;
};
