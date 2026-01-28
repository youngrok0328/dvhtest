#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DPI_PROCESSING_COMMON_DLG_API ProcCommonGridBase
{
public:
    ProcCommonGridBase();
    virtual ~ProcCommonGridBase();

    virtual long getGridCount() const;

    virtual bool create(long gridIndex, const RECT& rect, HWND parentWnd) = 0;
    virtual LPCTSTR getName(long gridIndex) const = 0;
    virtual void active(long gridIndex) = 0;
    virtual void deactivate(long gridIndex) = 0;
    virtual bool notify(long gridIndex, WPARAM wparam, LPARAM lparam) = 0;
    virtual void event_afterInspection(long gridIndex) = 0;
    virtual void event_changedRoi(long gridIndex) = 0;
    virtual void event_changedPane(long gridIndex) = 0;
    virtual void event_changedRaw(long gridIndex);
    virtual void event_changedImage(long gridIndex);
};
