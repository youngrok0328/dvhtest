#pragma once
//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "HandlerInputOutput.h"

//CPP_2_________________________________ This project's headers
#include "Macro.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
//namespace IpscTRHandler
//{
HandlerInputOutput::HandlerInputOutput(const int32_t name_idx)
    : m_output_Signal_Strip_id(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_STRIP_ID, name_idx))
    , m_output_Signal_Scan_id(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_SCAN_AREA_ID, name_idx))
    , m_output_Signal_Unit_count_X(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_UNIT_COUNT_X, name_idx))
    , m_output_Signal_Unit_count_Y(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_UNIT_COUNT_Y, name_idx))
    , m_output_Signal_Fill_pocket(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_FILL_POCKET, name_idx))
    , m_output_Signal_Grab_number(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_GRAB_NUMBER, name_idx))
    , m_output_Signal_Start(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_START, name_idx))
    , m_input_Signal_Fault(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_FAULT, name_idx))
    , m_input_Signal_Ready(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_VISION_READY, name_idx))
    , m_input_Signal_Expose(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_VISION_EXPOSE, name_idx))
    , m_input_Signal_Acquisition(MarcoMakeKey(SHAREDMEMORY_INTERAFCE_VISION_ACQUISITION, name_idx))

{
    // 출력 데이터 초기화는 필요에 따라 Reset() 함수를 통해 수행하도록 함
}

void HandlerInputOutput::Reset()
{
    m_output_Signal_Strip_id.Reset();
    m_output_Signal_Scan_id.Reset();
    m_output_Signal_Unit_count_X.Reset();
    m_output_Signal_Unit_count_Y.Reset();
    m_output_Signal_Grab_number.Reset();
    m_output_Signal_Start.Reset();

    ClearResultData();
}

void HandlerInputOutput::ClearResultData()
{
    m_output_Signal_Fill_pocket.Reset();
}
//} // namespace IpscTRHandler
