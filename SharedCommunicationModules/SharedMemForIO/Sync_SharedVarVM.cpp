#include "StdAfx.h"
#include "Sync_SharedVarVM.h"
#include "Macro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 주의 : PCI-Sync 의 DIO 는 Active Low 임.

namespace Ipvm
{
void InitializeStaticResourceSharedMemoryResourceHandleMap();
void TerminateStaticResourceSharedMemoryResourceHandleMap();
} // namespace Ipvm

CSync_SharedVarVM::CSync_SharedVarVM(long id)
:m_HandlerIO(new HandlerInputOutput(id))
,m_visionIO(new VisionInputOutput(id))
{
}

CSync_SharedVarVM::~CSync_SharedVarVM(void)
{
}

void CSync_SharedVarVM::SetSignal_VisionReady(BOOL bActive)
{
    *m_visionIO->m_output_Signal_Ready = bActive;
}

BOOL CSync_SharedVarVM::GetSignal_VisionReady()
{
    return *m_HandlerIO->m_input_Signal_Ready;
}

void CSync_SharedVarVM::SetSignal_VisionAcquisition(BOOL bActive)
{
    *m_visionIO->m_output_Signal_Acquisition = bActive;
}

BOOL CSync_SharedVarVM::GetSignal_VisionAcquisition()
{
    return *m_HandlerIO->m_input_Signal_Acquisition;
}

BOOL CSync_SharedVarVM::GetSignal_HandlerStart()
{
    return *m_visionIO->m_intput_Signal_Start;
}

void CSync_SharedVarVM::SetSignal_HandlerStart(BOOL bActive)
{
    *m_HandlerIO->m_output_Signal_Start = bActive;
}

long CSync_SharedVarVM::GetHandlerStripID()
{
    return *m_visionIO->m_input_Signal_Strip_id;
}
void CSync_SharedVarVM::SetHandlerStripID(long nStripID)
{
    *m_HandlerIO->m_output_Signal_Strip_id = nStripID;
}
long CSync_SharedVarVM::GetHandlerScanAreaID()
{
    return *m_visionIO->m_intput_Signal_Scan_id;
}
void CSync_SharedVarVM::SetHandlerScanAreaID(long nAreaID)
{
    *m_HandlerIO->m_output_Signal_Scan_id = nAreaID;
}
long CSync_SharedVarVM::GetHandlerUnitCountX()
{
    return *m_visionIO->m_intput_Signal_Unit_count_X;
}
void CSync_SharedVarVM::SetHandlerUnitCountX(long nUnitX)
{
    *m_HandlerIO->m_output_Signal_Unit_count_X = nUnitX;
}
long CSync_SharedVarVM::GetHandlerUnitCountY()
{
    return *m_visionIO->m_intput_Signal_Unit_count_Y;
}
void CSync_SharedVarVM::SetHandlerUnitCountY(long nUnitY)
{
    *m_HandlerIO->m_output_Signal_Unit_count_Y = nUnitY;
}

//OTI Fill Pocket 체크용
FillPocketResult CSync_SharedVarVM::GetEmptyPocketCheck()
{
	return m_visionIO->m_intput_Signal_Fill_pocket[0];
}

void CSync_SharedVarVM::SetEmptyPocketCheck(FillPocketResult arrFillPocketResult)
{
    m_HandlerIO->m_output_Signal_Fill_pocket[0] = arrFillPocketResult;
}

void CSync_SharedVarVM::SetGrabNumber(long nCount)
{
    *m_HandlerIO->m_output_Signal_Grab_number = nCount;
}

long CSync_SharedVarVM::GetGrabNumber()
{
    return *m_visionIO->m_intput_Signal_Grab_number;
}

void CSync_SharedVarVM::SetFault(BOOL bFault)
{
    *m_visionIO->m_output_Signal_Fault = bFault;
}

long CSync_SharedVarVM::GetFault()
{
    return *m_HandlerIO->m_input_Signal_Fault;
}

void CSync_SharedVarVM::SetExposer(BOOL bExposer)
{
    *m_visionIO->m_output_Signal_Expose = bExposer;
}

long CSync_SharedVarVM::GetExposer()
{
    return *m_HandlerIO->m_input_Signal_Expose;
}
