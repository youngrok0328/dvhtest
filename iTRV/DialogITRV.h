
#pragma once
//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ViewStatus.h"
#include "ViewVision.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class DialogITRV : public CDialogEx
{
    // 생성입니다.

public:
    DialogITRV(CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~DialogITRV();
    void ReturnToPrimaryUI();

    void Crashed(const wchar_t* dumpPathName);
    void AccessModeChanged();
    void ShowAccessModeDialog();
    void ShowIllumSetup(int32_t visionIndex);
    void ShowTeach(int32_t visionIndex);
    void ShowBatchInspection(int32_t visionIndex);

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_ITRV
    };
#endif

protected:
    HICON m_hIcon;
    std::vector<std::shared_ptr<ViewVision>> m_visions;
    std::unique_ptr<ViewStatus> m_viewStatus;

    void SetCurrentFrameString();
    void UpdateLayout();
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    DECLARE_MESSAGE_MAP()

private:
    static UINT ThreadSyncStartFunc(LPVOID pParam);
    UINT ThreadSyncStartFunc();

private:
    //Thread 내에서 호출 이후, Reset시켜준다
    HANDLE m_Flag_thread_SyncStart_INPOCKET;
    HANDLE m_Flag_thread_SyncStart_OTI;

    //Inline Start가 들어오면 Set, Inline Stop일때 Reset
    HANDLE m_Flag_thread_Inline_INPOCKET;
    HANDLE m_Flag_thread_Inline_OTI;

    HANDLE m_Handle_thread; // 쓰레드의 핸들을 가지고 있는다. 종료 대기시 사용.
    HANDLE m_Handle_thread_Kill; // 쓰레드의 핸들을 가지고 있는다. 종료 대기시 사용.
};
