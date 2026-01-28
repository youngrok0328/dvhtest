/*********************************************************************

   CPolyBtn

   Copyright (C) 2003 Smaller Animals Software, Inc.

   This software is provided 'as-is', without any express or implied
   warranty.  In no event will the authors be held liable for any damages
   arising from the use of this software.

   Permission is granted to anyone to use this software for any purpose,
   including commercial applications, and to alter it and redistribute it
   freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.

   3. This notice may not be removed or altered from any source distribution.

   http://www.smalleranimals.com
   smallest@smalleranimals.com

**********************************************************************/

//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PolyBtn.h"

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
#pragma comment(lib, "msimg32.lib") // for TransparentBlt

CPolyBtn::CPolyBtn()
{
    m_edges.clear();
    m_pPoints = NULL;

    m_bmpOffset = CPoint(0, 0);
    m_textOffset = CPoint(0, 0);

    m_ToolTip.m_hWnd = NULL;
    m_bMouseOnButton = false;

    m_bAllowHighlightOnGtrNeg45 = false;

    m_bmpID = m_bmpIDD = 0;

    m_bAllowRollover = false;

    m_clrHighlight = ::GetSysColor(COLOR_3DHIGHLIGHT);
    m_clrShadow = ::GetSysColor(COLOR_3DSHADOW);
    m_clrRollover = ::GetSysColor(COLOR_3DHIGHLIGHT);
    m_clrFocus = ::GetSysColor(COLOR_3DHIGHLIGHT);
    m_clrDisabled = ::GetSysColor(COLOR_3DSHADOW);
    m_clrFill = ::GetSysColor(COLOR_BTNFACE);
}

/////////////////////////////////////////////////////////////////////////////

CPolyBtn::~CPolyBtn()
{
    if (m_bmp.GetSafeHandle())
    {
        m_bmp.DeleteObject();
    }

    if (m_bmpD.GetSafeHandle())
    {
        m_bmpD.DeleteObject();
    }

    if (m_pPoints)
    {
        delete[] m_pPoints;
    }

    m_rgn.DeleteObject();
    m_rgnWnd.DeleteObject();
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::SetEdges(const std::vector<CEdge>& edges)
{
    m_edges = edges;
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::SetPoints(const std::vector<CPoint>& points)
{
    if (points.size() < 3)
    {
        ASSERT(0);
        return;
    }

    m_edges.clear();

    // we'll reuse this edge object
    CEdge t;
    t.rolloverclr = m_clrRollover;
    t.disabledclr = m_clrDisabled;
    t.focusclr = m_clrFocus;

    // build our edge vector from the vector of input points
    for (size_t i = 0; i < points.size(); i++)
    {
        t.start = points.at(i);
        if (i < points.size() - 1)
        {
            t.end = points.at(i + 1);
        }
        else
        {
            t.end = points.at(0);
        }

        // line slope
        double dy = t.end.y - t.start.y;
        double dx = t.end.x - t.start.x;

        // if the next point is due-north, northeast or due-east of the
        // current point, we'll assume we're facing the light
        if (dx >= 0 && dy <= 0)
        {
            t.clr = m_clrHighlight;
            t.downclr = m_clrShadow;
        }
        else if (dx >= 0 && dy >= 0 && dx >= dy && m_bAllowHighlightOnGtrNeg45)
        {
            // optionally set a highlight if the line is sloping at less than 0, but more than 45 degrees
            t.clr = m_clrHighlight;
            t.downclr = m_clrShadow;
        }
        else
        {
            // facing away from the light
            t.clr = m_clrShadow;
            t.downclr = m_clrHighlight;
        }

        m_edges.push_back(t);
    }
}

/////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CPolyBtn, CButton)
//{{AFX_MSG_MAP(CPolyBtn)
ON_WM_ERASEBKGND()
ON_WM_MOUSEMOVE()
//}}AFX_MSG_MAP
ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPolyBtn message handlers

void CPolyBtn::PreSubclassWindow()
{
    ModifyStyle(0, BS_OWNERDRAW);

    if (m_edges.size() == 0)
    {
        ASSERT(0);
    }
    else
    {
        m_rgn.DeleteObject();
        SetWindowRgn(NULL, FALSE);

        m_pPoints = new POINT[m_edges.size()];
        for (size_t i = 0; i < m_edges.size(); i++)
        {
            m_pPoints[i] = m_edges.at(i).start;

            if (i > 0)
            {
                ASSERT(m_edges.at(i - 1).end == m_edges.at(i).start);
            }
        }

        m_rgnWnd.CreateRectRgn(0, 0, 0, 0);

        // couldn't find a way to expand a region. so, we'll
        // offset the region in 8 directions and combine the
        // results. yes, this is a hack.

        CRgn temp;
        temp.CreatePolygonRgn(m_pPoints, static_cast<int>(m_edges.size()), ALTERNATE);

        temp.OffsetRgn(-1, 0);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);
        temp.OffsetRgn(0, 1);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);
        temp.OffsetRgn(1, 0);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);
        temp.OffsetRgn(1, 0);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);
        temp.OffsetRgn(0, -1);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);
        temp.OffsetRgn(0, -1);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);
        temp.OffsetRgn(-1, 0);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);
        temp.OffsetRgn(-1, 0);
        m_rgnWnd.CombineRgn(&temp, &m_rgnWnd, SIMPLEREGION);

        // once you use a region in SetWindowRgn, you're not allowed to do *anything* else with it
        // so, we'll make a copy.
        m_rgn.CreateRectRgn(0, 0, 0, 0);
        m_rgn.CopyRgn(&m_rgnWnd);

        SetWindowRgn(m_rgnWnd, TRUE);

        if (m_bmpID != 0)
        {
            if (!m_bmp.LoadBitmap(m_bmpID))
            {
                ASSERT(0);
            }
        }

        if (m_bmpIDD != 0)
        {
            if (!m_bmpD.LoadBitmap(m_bmpIDD))
            {
                ASSERT(0);
            }
        }
    }

    CButton::PreSubclassWindow();
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
    CRect rect = lpDrawItemStruct->rcItem;
    CDC* pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
    UINT state = lpDrawItemStruct->itemState;
    //	UINT nStyle = GetStyle();

    int nSavedDC = pDC->SaveDC();

    CBrush br;
    br.CreateSolidBrush(m_clrFill);

    // fill interior
    pDC->SetPolyFillMode(ALTERNATE);
    CBrush* ob = pDC->SelectObject(&br);
    pDC->Polygon(m_pPoints, static_cast<int>(m_edges.size()));
    pDC->SelectObject(ob);

    pDC->SelectClipRgn(&m_rgn);

    // draw bitmap
    if (m_bmpID != 0)
    {
        CPoint pos = m_bmpOffset;
        if ((state & ODS_SELECTED))
        {
            pos.x++;
            pos.y++;
        }

        _TransparentBlt(*pDC, m_bmp, pos.x, pos.y, m_clrTrans);
    }

    if ((state & ODS_DISABLED) && m_bmpIDD != 0)
    {
        _TransparentBlt(*pDC, m_bmpD, m_bmpOffset.x, m_bmpOffset.y, m_clrTransD);
    }

    // draw text
    CString txt;
    GetWindowText(txt);
    if (!txt.IsEmpty())
    {
        pDC->SetBkMode(TRANSPARENT);

        //YR
        //      if ((state & ODS_DISABLED))
        //      {
        //         CSize cz = pDC->GetTextExtent(txt);
        //         pDC->DrawState(m_textOffset, cz, txt, DSS_DISABLED, TRUE, 0, (HBRUSH)NULL);
        //      }
        //      else
        {
            CPoint pos = m_textOffset;
            if ((state & ODS_SELECTED))
            {
                pos.x++;
                pos.y++;
            }

            //YR pDC->TextOut(pos.x, pos.y, txt);
            {
                CRect rect0;
                GetClientRect(&rect0);
                int h0 = rect0.Height();
                CRect rect1 = rect0;
                int h1 = pDC->DrawText(txt, &rect1, DT_WORDBREAK | DT_CENTER | DT_CALCRECT);
                int dh = (h0 - h1) / 2;
                rect0.top += dh;
                rect0.bottom -= dh;
                pDC->DrawText(txt, &rect0, DT_WORDBREAK | DT_CENTER);
            }
        }
    }

    // paint the edges
    for (size_t i = 0; i < m_edges.size(); i++)
    {
        int iPenStyle = PS_SOLID;

        COLORREF clr = m_edges.at(i).clr;

        if ((state & ODS_SELECTED))
        {
            clr = m_edges.at(i).downclr;
        }
        else if ((state & ODS_FOCUS))
        {
            if (m_bMouseOnButton)
            {
                clr = m_edges.at(i).rolloverclr;
            }
            else
            {
                iPenStyle = PS_DOT;
            }
        }
        else if ((state & ODS_DISABLED))
        {
            clr = m_edges.at(i).disabledclr;
        }
        else if (m_bMouseOnButton)
        {
            clr = m_edges.at(i).rolloverclr;
        }

        //YR CPen pen(iPenStyle, 1, clr);
        CPen pen(iPenStyle, 2, clr);

        CPen* opp = pDC->SelectObject(&pen);

        pDC->MoveTo(m_edges.at(i).start);
        pDC->LineTo(m_edges.at(i).end);

        pDC->SelectObject(opp);
    }

    pDC->SelectClipRgn(NULL);
    pDC->RestoreDC(nSavedDC);
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::_TransparentBlt(CDC& dc, CBitmap& bmp, int xpos, int ypos, COLORREF clrTrans)
{
    CDC memDC;
    if (memDC.CreateCompatibleDC(&dc))
    {
        BITMAP bm;
        bmp.GetObject(sizeof(bm), &bm);

        CBitmap* ob = memDC.SelectObject(&bmp);

        TransparentBlt(
            dc.m_hDC, xpos, ypos, bm.bmWidth, bm.bmHeight, memDC.m_hDC, 0, 0, bm.bmWidth, bm.bmHeight, clrTrans);

        memDC.SelectObject(ob);
    }
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPolyBtn::OnEraseBkgnd(CDC* pDC)
{
    UNREFERENCED_PARAMETER(pDC);

    //CBrush br;
    //br.CreateSolidBrush(::GetSysColor(COLOR_BTNFACE));
    //pDC->FillRgn(&m_rgn, &br);
    return TRUE;

    //	return CButton::OnEraseBkgnd(pDC);
}

/////////////////////////////////////////////////////////////////////////////

BOOL CPolyBtn::PreTranslateMessage(MSG* pMsg)
{
    InitToolTip();
    m_ToolTip.RelayEvent(pMsg);

    if (WM_KEYDOWN == pMsg->message)
    {
        switch (pMsg->wParam)
        {
            case VK_ESCAPE:
            case VK_RETURN:
                return TRUE;
        }
    }

    return CButton::PreTranslateMessage(pMsg);
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::InitToolTip()
{
    if (m_ToolTip.m_hWnd == NULL)
    {
        m_ToolTip.Create(this);
        m_ToolTip.Activate(FALSE);
        m_ToolTip.SendMessage(TTM_SETMAXTIPWIDTH, 0, 400);
    }
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::SetTooltipText(LPCTSTR lpszText, BOOL bActivate)
{
    if (lpszText == NULL)
        return;

    InitToolTip();

    if (m_ToolTip.GetToolCount() == 0)
    {
        CRect rectBtn;
        GetClientRect(rectBtn);
        m_ToolTip.AddTool(this, lpszText, rectBtn, 1);
    }

    m_ToolTip.UpdateTipText(lpszText, this, 1);
    m_ToolTip.Activate(bActivate);
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::ActivateTooltip(BOOL bActivate)
{
    if (m_ToolTip.GetToolCount() == 0)
        return;

    m_ToolTip.Activate(bActivate);
}

/////////////////////////////////////////////////////////////////////////////

void CPolyBtn::OnMouseMove(UINT nFlags, CPoint point)
{
    CButton::OnMouseMove(nFlags, point);

    if (m_bAllowRollover)
    {
        ClientToScreen(&point);
        CWnd* wndUnderMouse = WindowFromPoint(point);
        if (wndUnderMouse == this)
        {
            if (!m_bMouseOnButton)
            {
                m_bMouseOnButton = true;
                Invalidate();

                TRACKMOUSEEVENT csTME;
                csTME.cbSize = sizeof(csTME);
                csTME.dwFlags = TME_LEAVE;
                csTME.hwndTrack = m_hWnd;
                ::_TrackMouseEvent(&csTME);
            }
        }
        else
        {
            m_bMouseOnButton = false;
            Invalidate();
        }
    }
    else
    {
        m_bMouseOnButton = false;
    }
}

/////////////////////////////////////////////////////////////////////////////

LRESULT CPolyBtn::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(wParam);
    UNREFERENCED_PARAMETER(lParam);

    m_bMouseOnButton = false;
    Invalidate();
    return 0;
}

//YR
void CPolyBtn::SetFillColor(COLORREF fillClr)
{
    if (m_clrFill == fillClr)
        return;
    else
    {
        m_clrFill = fillClr;
        Invalidate(FALSE);
    }
}

void CPolyBtn::MakeArrowButton(CWnd* pWnd, int iDepth)
{
    CRect rect;
    pWnd->GetClientRect(&rect);

    int iL = rect.Width();
    int iH = rect.Height();
    int iD = iDepth;

    std::vector<CPoint> pts1;
    pts1.push_back(CPoint(0, 0));
    pts1.push_back(CPoint(iD, iH / 2));
    pts1.push_back(CPoint(0, iH));
    pts1.push_back(CPoint(iL - iD, iH));
    pts1.push_back(CPoint(iL, iH / 2));
    pts1.push_back(CPoint(iL - iD, 0));

    SetPoints(pts1);
    //SetTextOffset(iD+2, iH/2-8);
}

void CPolyBtn::MakeHexagonButton(CWnd* pWnd, int iDepth)
{
    CRect rect;
    pWnd->GetClientRect(&rect);

    int iL = rect.Width();
    int iH = rect.Height();
    int iD = iDepth;

    std::vector<CPoint> pts1;
    pts1.push_back(CPoint(iD, 0));
    pts1.push_back(CPoint(0, iH / 2));
    pts1.push_back(CPoint(iD, iH));
    pts1.push_back(CPoint(iL - iD, iH));
    pts1.push_back(CPoint(iL, iH / 2));
    pts1.push_back(CPoint(iL - iD, 0));

    SetPoints(pts1);
    //SetTextOffset(iD+2, iH/2-8);
}
