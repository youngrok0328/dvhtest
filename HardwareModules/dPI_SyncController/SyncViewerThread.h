#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class SyncViewerThread : public CWinThread
{
    DECLARE_DYNCREATE(SyncViewerThread)

protected:
    SyncViewerThread(); // protected constructor used by dynamic creation

public:
    virtual BOOL InitInstance();
    virtual int ExitInstance();

protected:
    virtual ~SyncViewerThread();

    DECLARE_MESSAGE_MAP()
};
