//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceDetectRectThreshDlg.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceROIEditorDlg.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CSurfaceDetectRectThreshDlg, CDialog)

CSurfaceDetectRectThreshDlg::CSurfaceDetectRectThreshDlg(const VisionScale& scale, CWnd* pParent /*=NULL*/)
    : CDialog(CSurfaceDetectRectThreshDlg::IDD, pParent)
    , m_scale(scale)
{
    m_pMaskOrder = NULL;
    m_bDrawGraph = FALSE;
}

CSurfaceDetectRectThreshDlg::~CSurfaceDetectRectThreshDlg()
{
}

void CSurfaceDetectRectThreshDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSurfaceDetectRectThreshDlg, CDialog)
ON_WM_HSCROLL()
ON_BN_CLICKED(IDCANCEL, &CSurfaceDetectRectThreshDlg::OnBnClickedCancel)
ON_WM_VSCROLL()
ON_WM_PAINT()
ON_EN_CHANGE(IDC_EDIT_MIN_WIDTH, &CSurfaceDetectRectThreshDlg::OnEnChangeEditMinWidth)
END_MESSAGE_MAP()

// CSurfaceDetectRectThreshDlg 메시지 처리기입니다.

void CSurfaceDetectRectThreshDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
    int nID = pScrollBar->GetDlgCtrlID();
    int nPosition;

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

    if (nID == IDC_SCROLLBAR_THRESHOLD_Y)
    {
        CScrollBar* pScrollBarY = (CScrollBar*)GetDlgItem(IDC_SCROLLBAR_THRESHOLD_Y);
        nPosition = 255 - pScrollBarY->GetScrollPos();
        CString str;
        str.Format(_T("%d"), nPosition);
        SetDlgItemText(IDC_EDIT_THRESH_Y, str);
        if (m_pMaskOrder != NULL)
            m_pMaskOrder->m_threshDRY = nPosition;

        str.Empty();
    }
    UpdateData(FALSE);

    Invalidate(FALSE);
    uUpdateData();

    CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CSurfaceDetectRectThreshDlg::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
    // TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
    int nID = pScrollBar->GetDlgCtrlID();
    int nPosition;

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

    if (nID == IDC_SCROLLBAR_THRESHOLD_X)
    {
        CScrollBar* pScrollBarX = (CScrollBar*)GetDlgItem(IDC_SCROLLBAR_THRESHOLD_X);
        nPosition = 255 - pScrollBarX->GetScrollPos();
        CString str;
        str.Format(_T("%d"), nPosition);
        SetDlgItemText(IDC_EDIT_THRESH_X, str);
        if (m_pMaskOrder != NULL)
            m_pMaskOrder->m_threshDRX = nPosition;

        str.Empty();
    }

    UpdateData(FALSE);

    Invalidate(FALSE);
    uUpdateData();

    CDialog::OnVScroll(nSBCode, nPos, pScrollBar);
}

BOOL CSurfaceDetectRectThreshDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // TODO:  여기에 추가 초기화 작업을 추가합니다.
    CScrollBar* pScrollBarX = (CScrollBar*)GetDlgItem(IDC_SCROLLBAR_THRESHOLD_X);
    CScrollBar* pScrollBarY = (CScrollBar*)GetDlgItem(IDC_SCROLLBAR_THRESHOLD_Y);

    pScrollBarX->SetScrollRange(0, 255);
    pScrollBarY->SetScrollRange(0, 255);

    if (m_pMaskOrder != NULL)
    {
        pScrollBarX->SetScrollPos(255 - m_pMaskOrder->m_threshDRX);
        pScrollBarY->SetScrollPos(255 - m_pMaskOrder->m_threshDRY);
        CString str;
        str.Format(_T("%d"), m_pMaskOrder->m_threshDRX);
        SetDlgItemText(IDC_EDIT_THRESH_X, str);
        str.Format(_T("%d"), m_pMaskOrder->m_threshDRY);
        SetDlgItemText(IDC_EDIT_THRESH_Y, str);
        str.Format(_T("%d"), m_pMaskOrder->m_cuttingWidth);
        SetDlgItemText(IDC_EDIT_MIN_WIDTH, str);

        str.Empty();
    }
    return TRUE; // return TRUE unless you set the focus to a control
    // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CSurfaceDetectRectThreshDlg::SetMaskOrder(Para_MaskOrder::MaskInfo* pMaskOder)
{
    m_pMaskOrder = pMaskOder;
    m_MaskOrderOld = *pMaskOder;
}

void CSurfaceDetectRectThreshDlg::OnBnClickedCancel()
{
    // TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
    *m_pMaskOrder = m_MaskOrderOld;
    OnCancel();
}

void CSurfaceDetectRectThreshDlg::uUpdateData(void)
{
    CSurfaceROIEditorDlg* pParent = (CSurfaceROIEditorDlg*)GetParent();

    pParent->UpdateMainPrepairResult();
}

// 그래프를 그리기위한 Image를 전달한다 //kircheis_Hy
void CSurfaceDetectRectThreshDlg::SetImageData(const Ipvm::Image8u& image, const Ipvm::Point32r2* pptRect_BCU)
{
    Ipvm::Point32r2 imageCenter(image.GetSizeX() * 0.5f, image.GetSizeY() * 0.5f);
    auto roi = m_scale.convert_BCUToPixel(Ipvm::Rect32r(pptRect_BCU[0], pptRect_BCU[1]), imageCenter);

    int nWidth = roi.Width();
    int nHeight = roi.Height();

    m_veclProfileX.resize(nWidth);
    m_veclProfileY.resize(nHeight);
    long* plProfileX = &m_veclProfileX[0];

    long* plProfileY = &m_veclProfileY[0];
    memset(plProfileX, 0, nWidth * sizeof(long));
    memset(plProfileY, 0, nHeight * sizeof(long));

    for (long y = roi.m_top, lCntY = 0; y < roi.m_bottom; y++, lCntY++)
    {
        auto* image_y = image.GetMem(0, y);
        for (long x = roi.m_left, lCntX = 0; x < roi.m_right; x++, lCntX++)
        {
            plProfileX[lCntX] += (long)image_y[x];
            plProfileY[lCntY] += (long)image_y[x];
        }
        plProfileY[lCntY] /= (long)nWidth;
    }
    for (long lCntX = 0; lCntX < nWidth; lCntX++)
    {
        plProfileX[lCntX] /= (long)nHeight;
    }

    m_bDrawGraph = TRUE;
}

void CSurfaceDetectRectThreshDlg::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // TODO: 여기에 메시지 처리기 코드를 추가합니다.
    // 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.

    if (m_bDrawGraph)
    {
        DrawProfileGraphX(dc);
        DrawProfileGraphY(dc);
        DrawValidOption(dc);
    }
}

void CSurfaceDetectRectThreshDlg::DrawProfileGraphX(CPaintDC& dc)
{
    long lWidth = (long)m_veclProfileX.size();
    CDC memDC;
    CBitmap memBitmap;

    memDC.CreateCompatibleDC(&dc);
    memBitmap.CreateCompatibleBitmap(&dc, lWidth, 256);
    memDC.SelectObject(&memBitmap);

    CPen penLine(PS_SOLID, 2, RGB(0, 0, 255));
    CPen penValid(PS_SOLID, 1, RGB(0, 255, 0));
    CPen penInvaild(PS_SOLID, 1, RGB(255, 0, 0));
    CPen* pOldPen;
    ;

    CBrush* pOldBrush = (CBrush*)memDC.SelectStockObject(WHITE_BRUSH);

    memDC.FillRect(CRect(0, 0, lWidth, 256), pOldBrush);

    memDC.MoveTo(0, 256 - m_pMaskOrder->m_threshDRX);
    memDC.LineTo(lWidth, 256 - m_pMaskOrder->m_threshDRX);

    long lStart = -1, lEnd = -1, i;
    for (i = 0; i < lWidth; i++)
    {
        if (m_veclProfileX[i] > m_pMaskOrder->m_threshDRX)
        {
            lStart = i;
            break;
        }
    }
    for (i = lWidth - 1; i >= 0; i--)
    {
        if (m_veclProfileX[i] > m_pMaskOrder->m_threshDRX)
        {
            lEnd = i;
            break;
        }
    }

    for (i = 0; i < lWidth; i++)
    {
        if (i >= lStart && i <= lEnd && lStart != -1 && lEnd != -1)
            pOldPen = memDC.SelectObject(&penValid);
        else
            pOldPen = memDC.SelectObject(&penInvaild);

        memDC.MoveTo(i, 256);
        memDC.LineTo(i, 256 - m_veclProfileX[i]);
        memDC.SelectObject(pOldPen);
    }

    long nOldMode = dc.SetStretchBltMode(HALFTONE);
    CRect rtProfileGraph;
    GetDlgItem(IDC_STATIC_GRAPH_X)->GetWindowRect(&rtProfileGraph);
    ScreenToClient(rtProfileGraph);
    rtProfileGraph.DeflateRect(1, 1, 1, 1);
    dc.StretchBlt(rtProfileGraph.left, rtProfileGraph.top, rtProfileGraph.Width(), rtProfileGraph.Height(), &memDC, 0,
        0, (int)lWidth, 256, SRCCOPY);

    dc.SetStretchBltMode(nOldMode);

    memDC.DeleteDC();
}

void CSurfaceDetectRectThreshDlg::DrawProfileGraphY(CPaintDC& dc)
{
    long lHeight = (long)m_veclProfileY.size();
    CDC memDC;
    CBitmap memBitmap;

    memDC.CreateCompatibleDC(&dc);
    memBitmap.CreateCompatibleBitmap(&dc, 256, lHeight);
    memDC.SelectObject(&memBitmap);

    CPen penLine(PS_SOLID, 2, RGB(0, 0, 255));
    CPen penValid(PS_SOLID, 1, RGB(0, 255, 0));
    CPen penInvaild(PS_SOLID, 1, RGB(255, 0, 0));
    CPen* pOldPen;

    CBrush* pOldBrush = (CBrush*)memDC.SelectStockObject(WHITE_BRUSH);

    memDC.FillRect(CRect(0, 0, 256, lHeight), pOldBrush);

    memDC.MoveTo(256 - m_pMaskOrder->m_threshDRY, 0);
    memDC.LineTo(256 - m_pMaskOrder->m_threshDRY, lHeight);

    long lStart = -1, lEnd = -1, i;
    for (i = 0; i < lHeight; i++)
    {
        if (m_veclProfileY[i] > m_pMaskOrder->m_threshDRY)
        {
            lStart = i;
            break;
        }
    }
    for (i = lHeight - 1; i >= 0; i--)
    {
        if (m_veclProfileY[i] > m_pMaskOrder->m_threshDRY)
        {
            lEnd = i;
            break;
        }
    }

    for (i = 0; i < lHeight; i++)
    {
        if (i >= lStart && i <= lEnd && lStart != -1 && lEnd != -1)
            pOldPen = memDC.SelectObject(&penValid);
        else
            pOldPen = memDC.SelectObject(&penInvaild);

        memDC.MoveTo(256, i);
        memDC.LineTo(256 - m_veclProfileY[i], i);
        memDC.SelectObject(pOldPen);
    }

    long nOldMode = dc.SetStretchBltMode(HALFTONE);
    CRect rtProfileGraph;
    GetDlgItem(IDC_STATIC_GRAPH_Y)->GetWindowRect(&rtProfileGraph);
    ScreenToClient(rtProfileGraph);
    rtProfileGraph.DeflateRect(1, 1, 1, 1);
    dc.StretchBlt(rtProfileGraph.left, rtProfileGraph.top, rtProfileGraph.Width(), rtProfileGraph.Height(), &memDC, 0,
        0, 256, (int)lHeight, SRCCOPY);

    dc.SetStretchBltMode(nOldMode);

    memDC.DeleteDC();
}

void CSurfaceDetectRectThreshDlg::DrawValidOption(CPaintDC& dc)
{
    CRect rtOption1, rtOption2;
    GetDlgItem(IDC_STATIC_VALID_AREA)->GetWindowRect(&rtOption1);
    GetDlgItem(IDC_STATIC_INVALID_AREA)->GetWindowRect(&rtOption2);
    ScreenToClient(rtOption1);
    ScreenToClient(rtOption2);
    rtOption1.DeflateRect(1, 1, 1, 1);
    rtOption2.DeflateRect(1, 1, 1, 1);

    CDC memDC1, memDC2;
    CBitmap memBitmap1, memBitmap2;

    memDC1.CreateCompatibleDC(&dc);
    memBitmap1.CreateCompatibleBitmap(&dc, rtOption1.Width(), rtOption1.Height());
    memDC1.SelectObject(&memBitmap1);

    memDC2.CreateCompatibleDC(&dc);
    memBitmap2.CreateCompatibleBitmap(&dc, rtOption2.Width(), rtOption2.Height());
    memDC2.SelectObject(&memBitmap2);

    CBrush brValid, brInvalid;
    if (m_pMaskOrder->m_validate)
    {
        brValid.CreateSolidBrush(RGB(0, 255, 0));
        brInvalid.CreateSolidBrush(RGB(255, 0, 0));
    }
    else
    {
        brValid.CreateSolidBrush(RGB(255, 0, 0));
        brInvalid.CreateSolidBrush(RGB(0, 255, 0));
    }

    memDC1.FillRect(CRect(0, 0, rtOption1.Width(), rtOption1.Height()), &brValid);
    memDC2.FillRect(CRect(0, 0, rtOption2.Width(), rtOption2.Height()), &brInvalid);

    long nOldMode = dc.SetStretchBltMode(HALFTONE);

    dc.StretchBlt(rtOption1.left, rtOption1.top, rtOption1.Width(), rtOption1.Height(), &memDC1, 0, 0,
        rtOption1.Width(), rtOption1.Height(), SRCCOPY);
    dc.StretchBlt(rtOption2.left, rtOption2.top, rtOption2.Width(), rtOption2.Height(), &memDC2, 0, 0,
        rtOption2.Width(), rtOption2.Height(), SRCCOPY);

    dc.SetStretchBltMode(nOldMode);

    memDC1.DeleteDC();
    memDC2.DeleteDC();
}

void CSurfaceDetectRectThreshDlg::OnEnChangeEditMinWidth()
{
    UpdateData(TRUE);

    m_pMaskOrder->m_cuttingWidth = GetDlgItemInt(IDC_EDIT_MIN_WIDTH);

    UpdateData(FALSE);

    Invalidate(FALSE);
    uUpdateData();
}
