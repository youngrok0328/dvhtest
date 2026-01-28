//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgSubThresholdBase.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <afxdialogex.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(DlgSubThresholdBase, CDialogEx)

DlgSubThresholdBase::DlgSubThresholdBase(UINT resourceID, CWnd* pParent)
    : CDialogEx(resourceID, pParent)
{
}

DlgSubThresholdBase::~DlgSubThresholdBase()
{
}

void DlgSubThresholdBase::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(DlgSubThresholdBase, CDialogEx)
ON_WM_HSCROLL()
END_MESSAGE_MAP()

// DlgSubThresholdBase message handlers

void DlgSubThresholdBase::sendMessage_ThresholdChanged(long target)
{
    GetParent()->SendMessage(UM_THRESHOLD_CHANGED, target);
}

void DlgSubThresholdBase::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    int step = 1;
    int Mstep = 5;
    int nNext, nmin, nmax;

    pScrollBar->GetScrollRange(&nmin, &nmax);

    switch (nSBCode)
    {
        case SB_THUMBPOSITION:
        case SB_THUMBTRACK:
            nNext = nPos;
            if (nNext > nmax)
                nNext = nmax;
            else if (nNext < nmin)
                nNext = nmin;
            pScrollBar->SetScrollPos(nNext);
            break;
        case SB_LINEDOWN:
            nNext = pScrollBar->GetScrollPos();
            if (nNext + step <= nmax)
            {
                pScrollBar->SetScrollPos(nNext + step);
                nNext += step;
            }
            break;
        case SB_LINEUP:
            nNext = pScrollBar->GetScrollPos();
            if (nNext - step >= nmin)
            {
                pScrollBar->SetScrollPos(nNext - step);
                nNext -= step;
            }
            break;
        case SB_PAGEDOWN:
            nNext = pScrollBar->GetScrollPos();
            if (nNext + Mstep <= nmax)
            {
                pScrollBar->SetScrollPos(nNext + Mstep);
                nNext += Mstep;
            }
            else
            {
                nNext = nmax;
                pScrollBar->SetScrollPos(nNext);
            }
            break;
        case SB_PAGEUP:
            nNext = pScrollBar->GetScrollPos();
            if (nNext - Mstep >= nmin)
            {
                pScrollBar->SetScrollPos(nNext - Mstep);
                nNext -= Mstep;
            }
            else
            {
                nNext = nmin;
                pScrollBar->SetScrollPos(nNext);
            }
            break;
        default:
            return;
            break;
    }

    auto retValue = OnChangedScroll(pScrollBar);
    sendMessage_ThresholdChanged(retValue);

    CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}
