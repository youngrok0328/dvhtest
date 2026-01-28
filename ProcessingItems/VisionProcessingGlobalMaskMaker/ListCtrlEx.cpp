//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ListCtrlEx.h"

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
IMPLEMENT_DYNAMIC(ListCtrlEx, CListCtrl)

ListCtrlEx::ListCtrlEx()
{
}

ListCtrlEx::~ListCtrlEx()
{
}

BEGIN_MESSAGE_MAP(ListCtrlEx, CListCtrl)
ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &ListCtrlEx::OnNMCustomdraw)
END_MESSAGE_MAP()

// ListCtrlEx message handlers

BOOL ListCtrlEx::DeleteAllItems()
{
    m_items_color.clear();
    return CListCtrl::DeleteAllItems();
}

void ListCtrlEx::SetItemColor(int nItem, int nSubItem, COLORREF color)
{
    m_items_color[nItem][nSubItem] = color;
}

void ListCtrlEx::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);

    if (pNMCD->dwDrawStage == CDDS_PREPAINT)
    {
        *pResult = (LRESULT)CDRF_NOTIFYITEMDRAW;
        return;
    }

    if (pNMCD->dwDrawStage == CDDS_ITEMPREPAINT)
    {
        NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
        pDraw->clrText = 0xffffff;
        pDraw->clrTextBk = 0x0;
        *pResult = (LRESULT)CDRF_NOTIFYSUBITEMDRAW;
        return;
    }

    if (pNMCD->dwDrawStage == (CDDS_SUBITEM | CDDS_ITEMPREPAINT))
    {
        NMLVCUSTOMDRAW* pDraw = (NMLVCUSTOMDRAW*)(pNMHDR);
        auto itRowColor = m_items_color.find(static_cast<int>(pNMCD->dwItemSpec));

        pDraw->clrText = 0x0;
        pDraw->clrTextBk = 0xffffff;

        if (itRowColor != m_items_color.end())
        {
            auto itSubColor = itRowColor->second.find(pDraw->iSubItem);
            if (itSubColor != itRowColor->second.end())
            {
                pDraw->clrText = itSubColor->second;
            }
        }

        *pResult = (LRESULT)CDRF_NEWFONT; // 이렇게 해야 설정한 대로 그려진다.
        return;
    }

    *pResult = 0;
}
