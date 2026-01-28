//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "GridCellImage.h"

//CPP_2_________________________________ This project's headers
#include "GridCell.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <atlimage.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNCREATE(CGridCellImage, CGridCell)

CGridCellImage::CGridCellImage()
    : CGridCell()
{
    //m_bChecked = FALSE;
    //m_Rect.IsRectNull();
}
//
// CSize CGridCellImage::GetCellExtent(CDC* pDC)
// {
//     // Using SM_CXHSCROLL as a guide to the size of the checkbox
// 	int nWidth = GetSystemMetrics(SM_CXHSCROLL) + 2*GetMargin();
// 	return CGridCell::GetCellExtent(pDC) + CSize(nWidth, nWidth);
// }
//
// // i/o:  i=dims of cell rect; o=dims of text rect
// BOOL CGridCellImage::GetTextRect( LPRECT pRect)
// {
//     BOOL bResult = CGridCell::GetTextRect(pRect);
//     if (bResult)
//     {
//         int nWidth = GetSystemMetrics(SM_CXHSCROLL) + 2*GetMargin();
//         pRect->left += nWidth;
//         if (pRect->left > pRect->right)
//             pRect->left = pRect->right;
//     }
//     return bResult;
// }

// Override draw so that when the cell is selected, a drop arrow is shown in the RHS.
BOOL CGridCellImage::Draw(CDC* pDC, int nRow, int nCol, CRect rect, BOOL bEraseBkgnd /*=TRUE*/)
{
    BOOL bResult = CGridCell::Draw(pDC, nRow, nCol, rect, bEraseBkgnd);

#ifndef _WIN32_WCE
    // Store the cell's dimensions for later
    CImage image;
    image.Load(m_Image); //이미지 경로를 넣어 이미지를 불러온다 (BMP, JPG 모두 가능)
    if (image)
    {
        /*CPaintDC dc(this);*/
        pDC->SetStretchBltMode(HALFTONE); //이미지 확대 축소시 깨짐방지
        image.StretchBlt(pDC->GetSafeHdc(), rect, SRCCOPY); //이미지를 클라이언트 크기에 맞도록 화면에 출력한다.
    }
    // }
#endif
    return bResult;
}
void CGridCellImage::SetCellImage(CString image)
{
    m_Image = image;
}
