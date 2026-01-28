#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "TypeDefine.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
struct ResultPassiveItem
{
    //--------------------------------------------------------------------
    // 여기값은 Passive 결과 출력용이다
    // Passive Align 알고리즘에서 여기에 있는 값을 만드는 것이지
    // 외부에서 값이 세팅되어 있길 바라고 값을 이용하지는 않는다
    //
    // Teaching 등에서 빈객체를 넣고 출력값 확인용으로 사용하고 있으므로
    // 그러므로 이 객체에 Spec값을 넣고 Passive Align으로 전달해 주지는 말자.
    //--------------------------------------------------------------------

    long m_sPassive_Index; // sPassive_InfoDB 내에서 몇번째 Spec Index를 참조하는가
    bool m_bAlignSuccess; // mc_MED#3 Align Fail시, Mask에 Spec Position이 들어갈 수 있도록 하기 위함
    PI_RECT m_specROI; // Mask Image 만들때 사용.. Pad 때문에..
    Ipvm::Rect32s m_roughAlignROI_Hor; //kircheis_MED2Passive
    Ipvm::Rect32s m_roughAlignROI_Ver; //kircheis_MED2Passive
    FPI_RECT m_roughAlignResult; //kircheis_MED2Passive
    FPI_RECT m_detailSearchROI[4]; // 4방향 - Detail Search ROI Data_Debug
    EDGEPOINTS m_debugEdgePoints[4]; // Debug..
    FPI_RECT m_detailAlignResult; // Debug
    FPI_RECT m_detailAlignResult_Horizontal;
    FPI_RECT m_detailAlignResult_Vertical;
    FPI_RECT m_electAlignResult; //Debug (Final)

    BOOL m_use2ndInsp;
    CString m_str2ndInspMatchCode;

    std::vector<Ipvm::Rect32r> m_pad_specROIs;
    std::vector<Ipvm::Rect32s> m_shortenSearchROIs;
    std::vector<Ipvm::LineSeg32r> m_specLines;
};

class Result
{
public:
    Result();
    ~Result();

    void clear();
    ResultPassiveItem* findPassiveItem(long chipIndex);

    std::vector<ResultPassiveItem> m_passiveItems;
};
