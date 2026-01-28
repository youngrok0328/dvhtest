#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxtempl.h>
#include <windows.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
#ifndef DECLARE_USER_MESSAGE
#define DECLARE_USER_MESSAGE(name) static const UINT name = ::RegisterWindowMessage(name##_MSG);
#endif

#define WMU_HITCAPBUTTON_MSG _T("WMU_HITCAPBUTTON--{46CB2AA1-EB38-48ab-A4EC-D7C1D5D15648}")

//#define WMU_HITCAPBUTTON WM_APP + 15

class __INTEKPLUS_SHARED_BUTTON_API__ CCaptionButton : public CWnd
{
public:
    // *** C'tor + Deconstructor:
    CCaptionButton();
    ~CCaptionButton() override;
    // *** static functions:
    // Initialize the class with the Handle of the window/dialogue
    static BOOL InitCapBtn(HWND hWnd);
    // ***member functions:
    // set the bitmap ID's and the No.(order) of the button(s)
    void SetBmpID(HINSTANCE hContainerModule, UINT iBtnNo, UINT ID_Bmp_up, UINT ID_Bmp_down = 0);
    // enable/disable button TRUE/FALSE
    BOOL EnableButton(BOOL bEnable);

private:
    // *** static variables:
    static HWND m_hWndCap; // Handle of the window that receives the button
    static BOOL m_bActive; // state of the window/dialogue
    static CMap<UINT, UINT&, CCaptionButton*, CCaptionButton*&>
        m_mapBtn; // map of: button no.<->CCaptionButton instance
    // *** member variables:
    HINSTANCE m_hContainerModule;
    CRect m_rc; // caption/button rectangle
    UINT m_idBmpUp; // bitmap ID
    UINT m_idBmpDown; // bitmap ID
    UINT m_uiBtnCount; // No. of buttons
    BOOL m_bBtnChecked; // button is checked TRUE/FALSE
    BOOL m_bEnable; // button is enabled TRUE/FALSE
    COLORADJUSTMENT* m_CAdj; // structure to adjust the color of the bitmap
    TRACKMOUSEEVENT m_TME;
    // *** member functions:
    // calculate the caption-rectangle
    void CalcCapRect(CRect* pCapRect);
    // paint the frame of the button (highlighted/diabled or not)
    void PaintBtnFrame(CDC* dc);
    // change the COLORADJUSTMENT structure dependent on the window-state
    void PaintBmpActive(CDC* dc);
    // the main paint procedure
    BOOL PaintBmpCheckBtn(UINT uiBtnCount = 1);
    // find out if a button was hit or not:
    BOOL ButtonHitTest(CPoint point);

    void ChangeButtonState();
    // *** static functions:
    // is the window/dialogue active or inactive:
    static BOOL isWindowActive(BOOL& bActive);
    // change the caption text of the window/dialogue:
    static void OnMsgSetText(LPCSTR lpText);
    // *** static functions to Subclass the dialogue:
    // Subclass a dialogue (change the name of it's WindowProc):
    static BOOL HookDlg();
    // the "new" WindowProc:
    static LRESULT CALLBACK HookedWindowProc(HWND, UINT, WPARAM, LPARAM);
};
