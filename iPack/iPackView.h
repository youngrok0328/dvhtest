#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiPackDoc;

//HDR_6_________________________________ Header body
//
class CiPackView : public CFormView
{
protected: // serialization에서만 만들어집니다.
    CiPackView();
    DECLARE_DYNCREATE(CiPackView)

public:
    enum
    {
        IDD = IDD_IPACK_FORM
    };

    // 특성입니다.

public:
    CiPackDoc* GetDocument() const;

    CString m_strAppVersion;
    CString m_strAppDetailVersion;

    // 작업입니다.

public:
    static void UserExceptionHandlerPre(void* callbackData);
    static void UserExceptionHandlerPost(
        void* callbackData, bool success, const wchar_t* dumpPathName, const wchar_t* message);

    // 재정의입니다.

public:
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual void OnInitialUpdate(); // 생성 후 처음 호출되었습니다.

    // 구현입니다.

public:
    virtual ~CiPackView();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    // 생성된 메시지 맵 함수

protected:
    DECLARE_MESSAGE_MAP()
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnSize(UINT nType, int cx, int cy);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#ifndef _DEBUG // iPackView.cpp의 디버그 버전
inline CiPackDoc* CiPackView::GetDocument() const
{
    return reinterpret_cast<CiPackDoc*>(m_pDocument);
}
#endif
