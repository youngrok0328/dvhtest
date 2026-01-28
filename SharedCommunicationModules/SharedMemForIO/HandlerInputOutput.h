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
//namespace IpscTRHandler
//{
class SharedMemForIODEF_API HandlerInputOutput
{
public:
    HandlerInputOutput(const int32_t name_idx);
    HandlerInputOutput(const HandlerInputOutput& rhs) = delete;
    HandlerInputOutput(HandlerInputOutput&& rhs) = delete;
    ~HandlerInputOutput() = default;

    HandlerInputOutput& operator=(const HandlerInputOutput& rhs) = delete;
    HandlerInputOutput& operator=(HandlerInputOutput&& rhs) = delete;

    void Reset();
    void ClearResultData();

    // Input shared memory
    const Ipvm::SharedMemory<BOOL, true> m_input_Signal_Ready;
    const Ipvm::SharedMemory<BOOL, true> m_input_Signal_Expose;
    const Ipvm::SharedMemory<BOOL, true> m_input_Signal_Acquisition;
    const Ipvm::SharedMemory<BOOL, true> m_input_Signal_Fault;

    // Output shared memory
    Ipvm::SharedMemory<BOOL> m_output_Signal_Start;
    Ipvm::SharedMemory<long> m_output_Signal_Strip_id;
    Ipvm::SharedMemory<long> m_output_Signal_Scan_id;
    Ipvm::SharedMemory<long> m_output_Signal_Unit_count_X;
    Ipvm::SharedMemory<long> m_output_Signal_Unit_count_Y;
    Ipvm::SharedMemory<FillPocketResult> m_output_Signal_Fill_pocket;
    Ipvm::SharedMemory<long> m_output_Signal_Grab_number;
};
//} // namespace IpscTRHandler
