#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Common.h"
#include "SharedMemory.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
//namespace IpscTRVision
//{
class SharedMemForIODEF_API VisionInputOutput
{
public:
    VisionInputOutput(const int32_t name_idx);
    VisionInputOutput(const VisionInputOutput& rhs) = delete;
    VisionInputOutput(VisionInputOutput&& rhs) = delete;
    ~VisionInputOutput() = default;

    VisionInputOutput& operator=(const VisionInputOutput& rhs) = delete;
    VisionInputOutput& operator=(VisionInputOutput&& rhs) = delete;

    void Reset();
    void ClearResultData();

    // Input shared memory
    const Ipvm::SharedMemory<long, true> m_input_Signal_Strip_id;
    const Ipvm::SharedMemory<long, true> m_intput_Signal_Scan_id;
    const Ipvm::SharedMemory<long, true> m_intput_Signal_Unit_count_X;
    const Ipvm::SharedMemory<long, true> m_intput_Signal_Unit_count_Y;
    Ipvm::SharedMemory<FillPocketResult, false> m_intput_Signal_Fill_pocket;
    const Ipvm::SharedMemory<long, true> m_intput_Signal_Grab_number;
    const Ipvm::SharedMemory<BOOL, true> m_intput_Signal_Start;

    // Output shared memory
    Ipvm::SharedMemory<BOOL> m_output_Signal_Ready;
    Ipvm::SharedMemory<BOOL> m_output_Signal_Expose;
    Ipvm::SharedMemory<BOOL> m_output_Signal_Acquisition;
    Ipvm::SharedMemory<BOOL> m_output_Signal_Fault;
};
//} // namespace IpscTRVision
