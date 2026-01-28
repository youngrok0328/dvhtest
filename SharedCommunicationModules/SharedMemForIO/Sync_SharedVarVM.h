#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Common.h"
#include "SharedMemory.h"

#include "HandlerInputOutput.h"
#include "VisionInputOutput.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations

enum VisionName
{
    Inpocket_Vision,
    OTI_Vision,
    VISION_NAME_END,
};

class SharedMemForIODEF_API CSync_SharedVarVM
{
public: // Ctor & Dtor
    CSync_SharedVarVM(long id);
    ~CSync_SharedVarVM(void);

public: // Interfaces
    // DIO Read & Write
    void SetSignal_VisionReady(BOOL bActive);
    BOOL GetSignal_VisionReady();
    void SetSignal_VisionAcquisition(BOOL bActive);
    BOOL GetSignal_VisionAcquisition();
    BOOL GetSignal_HandlerStart();
    long GetHandlerStripID();
    void SetHandlerStripID(long nStripID);
    long GetHandlerScanAreaID();
    void SetHandlerScanAreaID(long nAreaID);
    long GetHandlerUnitCountX();
    long GetHandlerUnitCountY();
    void SetHandlerUnitCountX(long nUnitX);
    void SetHandlerUnitCountY(long nUnitY);
    void SetGrabNumber(long nCount);
    long GetGrabNumber();
    void SetFault(BOOL bExposer);
    long GetFault();
    void SetExposer(BOOL bExposer);
    long GetExposer();

    void SetSignal_HandlerStart(BOOL bActive);

    FillPocketResult GetEmptyPocketCheck();
    void SetEmptyPocketCheck(FillPocketResult arrFillPocketResult);

private:
    HandlerInputOutput* m_HandlerIO;
    VisionInputOutput* m_visionIO;

private: // Do Not Use
    CSync_SharedVarVM(void);
};
