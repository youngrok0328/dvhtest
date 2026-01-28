//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SimpleLabel.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CSimpleLabel::CSimpleLabel(void)
    : m_pBrush(new CBrush(::GetSysColor(15)))
    , m_backgroundColor(0)
{
}

CSimpleLabel::~CSimpleLabel(void)
{
    if (m_pBrush->GetSafeHandle())
    {
        m_pBrush->DeleteObject();
    }

    delete m_pBrush;
}

BEGIN_MESSAGE_MAP(CSimpleLabel, CStatic)
ON_WM_PAINT()
END_MESSAGE_MAP()

void CSimpleLabel::PreSubclassWindow()
{
    CStatic::PreSubclassWindow();

    GetWindowText(m_strText);
}

// --------------------------------------------------------------------------

LRESULT CSimpleLabel::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_SETTEXT)
    {
        m_strText = (LPCTSTR)lParam;
    }
    else if (message == WM_GETTEXT)
    {
        int iCount = (int)wParam;

        memset((char*)lParam, 0, iCount + 1);
        memcpy((char*)lParam, m_strText, iCount);

        return TRUE;
    }

    return CStatic::WindowProc(message, wParam, lParam);
}

void CSimpleLabel::SetBkColor(COLORREF color)
{
    if (m_backgroundColor == color)
    {
        return;
    }

    if (m_pBrush->GetSafeHandle())
    {
        m_pBrush->DeleteObject();
    }

    m_pBrush->CreateSolidBrush(color);

    m_backgroundColor = color;

    Invalidate(FALSE);
}

void CSimpleLabel::SetText(LPCTSTR text)
{
    if (m_strText == text)
    {
        return;
    }

    m_strText = text;

    Invalidate(FALSE);
}

void CSimpleLabel::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    // 깜빡거림 때문에 더블 버퍼링으로 바꿈.
    CRect rtClient;
    GetClientRect(&rtClient);

    CBitmap bitmap;
    bitmap.CreateCompatibleBitmap(&dc, rtClient.Width(), rtClient.Height());

    CDC memdc;
    memdc.CreateCompatibleDC(&dc);
    auto* pOldbit = memdc.SelectObject(&bitmap);

    memdc.FillRect(rtClient, m_pBrush);

    int iOldMode = memdc.SetBkMode(TRANSPARENT);
    memdc.DrawText(m_strText, rtClient, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    memdc.SetBkMode(iOldMode);

    dc.BitBlt(0, 0, rtClient.Width(), rtClient.Height(), &memdc, 0, 0, SRCCOPY);

    memdc.SelectObject(pOldbit);
}
