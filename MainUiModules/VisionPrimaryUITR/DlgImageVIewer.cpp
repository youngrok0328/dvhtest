//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "DlgImageVIewer.h"

//CPP_2_________________________________ This project's headers
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
IMPLEMENT_DYNAMIC(CDlgImageVIewer, CDialog)

CDlgImageVIewer::CDlgImageVIewer(CWnd* pParent /*=NULL*/)
    : CDialog(CDlgImageVIewer::IDD, pParent)
{
    m_nImageSizeX = 0;
    m_nImageSizeY = 0;

    m_nScrollMoveX = 0;
    m_nScrollMoveY = 0;

    m_bSetImage = FALSE;
}

CDlgImageVIewer::~CDlgImageVIewer()
{
}

void CDlgImageVIewer::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDlgImageVIewer, CDialog)
ON_WM_HSCROLL()
ON_WM_VSCROLL()
ON_WM_PAINT()
END_MESSAGE_MAP()

// CDlgImageVIewer 메시지 처리기입니다.
void CDlgImageVIewer::SetImage(CString strJPGFullPath)
{
    m_strFilePath = strJPGFullPath;

    m_bSetImage = TRUE;

    Invalidate(FALSE);
}

void CDlgImageVIewer::SetupScroll(long nWidth, long nHeight)
{
    // 작업 가로/ 세로 길이
    int iWidth = nWidth; // 사용할 스크롤 의 가로 길이 ( 이미지 라면 이미지 가로 사이즈를 넣어주면 된다.)
    int iHeight = nHeight; // 사용할 스크롤 의 세로 길이 ( 이미지 라면 이미지 세로 사이즈를 넣어주면 된다.)

    // 윈도우 렉트
    CRect rt;
    GetClientRect(&rt);

    // 스크롤 정보
    SCROLLINFO si;
    si.cbSize = sizeof(si);
    si.fMask = SIF_ALL;
    ///////////////////////////////////////////////////////
    // 세로
    if (rt.Height() < iHeight)
    {
        si.nMin = 0;
        si.nMax = iHeight;
        si.nPage = rt.Height();
        si.nPos = GetScrollPos(SB_VERT);
        ;

        SetScrollInfo(SB_VERT, &si, TRUE);
        ShowScrollBar(SB_VERT, TRUE);
    }
    else
    {
        ShowScrollBar(SB_VERT, FALSE);
    }

    ///////////////////////////////////////////////////////
    // 가로
    if (rt.Width() < iWidth)
    {
        si.nMin = 0;
        si.nMax = iWidth;
        si.nPage = rt.Width();
        si.nPos = GetScrollPos(SB_HORZ);
        ;

        SetScrollInfo(SB_HORZ, &si, TRUE);
        ShowScrollBar(SB_HORZ, TRUE);
    }
    else
    {
        ShowScrollBar(SB_HORZ, FALSE);
    }
}

void CDlgImageVIewer::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

    long nPosTemp = (long)nPos;
    if (pScrollBar == NULL)
    {
        switch (nSBCode)
        {
            case SB_LINELEFT:
                nPosTemp = m_nScrollMoveX;
                nPosTemp -= 10;
                if (nPosTemp <= 0)
                    nPosTemp = 0;
                nPosTemp;
                break;
            case SB_LINERIGHT:
                nPosTemp = m_nScrollMoveX;
                nPosTemp += 10;
                if (nPosTemp >= m_nImageSizeX)
                    nPosTemp = m_nImageSizeX - 1;
                break;
            case SB_PAGELEFT:
                nPosTemp = m_nScrollMoveX;
                nPosTemp -= 50;
                if (nPosTemp <= 0)
                    nPosTemp = 0;
                break;
            case SB_PAGERIGHT:
                nPosTemp = m_nScrollMoveX;
                nPosTemp += 50;
                if (nPosTemp >= m_nImageSizeX)
                    nPosTemp = m_nImageSizeX - 1;
                break;
            case SB_LEFT:

                break;
            case SB_RIGHT:

                break;
            case SB_ENDSCROLL:
                return;
                break;
        };
    }

    nPos = (UINT)nPosTemp;

    SetScrollPos(SB_HORZ, nPos);

    m_nScrollMoveX = nPos;

    Invalidate(FALSE);

    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CDlgImageVIewer::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
    long nScrollPos = (long)nPos;
    if (pScrollBar == NULL)
    {
        switch (nSBCode)
        {
            case SB_LINEUP:
                nScrollPos = m_nScrollMoveY;
                nScrollPos -= 10;
                if (nScrollPos <= 0)
                    nScrollPos = 0;
                break;
            case SB_LINEDOWN:
                nScrollPos = m_nScrollMoveY;
                nScrollPos += 10;
                if (nScrollPos >= m_nImageSizeY)
                    nScrollPos = m_nImageSizeY - 1;
                break;
            case SB_PAGEUP:
                nScrollPos = m_nScrollMoveY;
                nScrollPos -= 50;
                if (nScrollPos <= 0)
                    nScrollPos = 0;
                break;
            case SB_PAGEDOWN:
                nScrollPos = m_nScrollMoveY;
                nScrollPos += 50;
                if (nScrollPos >= m_nImageSizeY)
                    nScrollPos = m_nImageSizeY - 1;
                break;
            case SB_ENDSCROLL:
                return; // 처리 해주지 않으면 계속 리셋...
                break;
        };
    }

    SetScrollPos(SB_VERT, nScrollPos);

    m_nScrollMoveY = nScrollPos;

    Invalidate(FALSE);

    CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CDlgImageVIewer::OnPaint()
{
    CPaintDC dc(this); // device context for painting

    if (m_bSetImage)
    {
        CImage Image;
        Image.Load(m_strFilePath);

        m_nImageSizeX = Image.GetWidth();
        m_nImageSizeY = Image.GetHeight();

        CRect rtClient;
        GetClientRect(&rtClient);

        CBitmap bitmap, *pOldbit = NULL;
        bitmap.CreateCompatibleBitmap(&dc, m_nImageSizeX, m_nImageSizeY);

        SetupScroll(m_nImageSizeX, m_nImageSizeY);

        CDC MemDC;
        MemDC.CreateCompatibleDC(&dc);
        pOldbit = MemDC.SelectObject(&bitmap);
        MemDC.PatBlt(0, 0, m_nImageSizeX, m_nImageSizeY, WHITENESS);

        Image.BitBlt(MemDC.GetSafeHdc(), 0, 0);

        long nOldMode = dc.SetStretchBltMode(COLORONCOLOR);
        dc.SetStretchBltMode(nOldMode);
        dc.SelectStockObject(WHITE_BRUSH);
        dc.Rectangle(0, 0, 1400, 1024);
        dc.StretchBlt(0, 0, m_nImageSizeX, m_nImageSizeY, &MemDC, m_nScrollMoveX, m_nScrollMoveY, m_nImageSizeX,
            m_nImageSizeY, SRCCOPY);
        dc.SelectObject(pOldbit);
    }
}
