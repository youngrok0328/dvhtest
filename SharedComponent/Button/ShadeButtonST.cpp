//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ShadeButtonST.h"

//CPP_2_________________________________ This project's headers
#include "CeXDib.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CShadeButtonST::CShadeButtonST()
    : m_dNormal(new CCeXDib)
    , m_dDown(new CCeXDib)
    , m_dDisabled(new CCeXDib)
    , m_dOver(new CCeXDib)
    , m_dh(new CCeXDib)
    , m_dv(new CCeXDib)
{
    m_bIsRounded = FALSE;
    m_bIsBorder = FALSE;
}

CShadeButtonST::~CShadeButtonST()
{
    delete m_dNormal;
    delete m_dDown;
    delete m_dDisabled;
    delete m_dOver;
    delete m_dh;
    delete m_dv;
}

void CShadeButtonST::SetShade(UINT shadeID, BYTE granularity, BYTE highlight, BYTE coloring, COLORREF color)
{
    long sXSize, sYSize, bytes, j, i, k, h;
    BYTE *iDst, *posDst;

    RECT rect;
    GetWindowRect(&rect);
    sYSize = rect.bottom - rect.top;
    sXSize = rect.right - rect.left;

    m_dh->Create(max(1, sXSize /*-2*m_FocusRectMargin-1*/), 1, 8); //create the horizontal focus bitmap
    m_dv->Create(1, max(1, sYSize /*-2*m_FocusRectMargin*/), 8); //create the vertical focus bitmap

    m_dNormal->Create(sXSize, sYSize, 8); //create the default bitmap

    COLORREF hicr = GetSysColor(COLOR_BTNHIGHLIGHT); //get the button base colors
    COLORREF midcr = GetSysColor(COLOR_BTNFACE);
    COLORREF locr = GetSysColor(COLOR_BTNSHADOW);
    long r, g, b; //build the shaded palette
    for (i = 0; i < 129; i++)
    {
        r = ((128 - i) * GetRValue(locr) + i * GetRValue(midcr)) / 128;
        g = ((128 - i) * GetGValue(locr) + i * GetGValue(midcr)) / 128;
        b = ((128 - i) * GetBValue(locr) + i * GetBValue(midcr)) / 128;
        m_dNormal->SetPaletteIndex((BYTE)i, (BYTE)r, (BYTE)g, (BYTE)b);
        m_dh->SetPaletteIndex((BYTE)i, (BYTE)r, (BYTE)g, (BYTE)b);
        m_dv->SetPaletteIndex((BYTE)i, (BYTE)r, (BYTE)g, (BYTE)b);
    }
    for (i = 1; i < 129; i++)
    {
        r = ((128 - i) * GetRValue(midcr) + i * GetRValue(hicr)) / 128;
        g = ((128 - i) * GetGValue(midcr) + i * GetGValue(hicr)) / 128;
        b = ((128 - i) * GetBValue(midcr) + i * GetBValue(hicr)) / 128;
        m_dNormal->SetPaletteIndex((BYTE)(i + 127), (BYTE)r, (BYTE)g, (BYTE)b);
        m_dh->SetPaletteIndex((BYTE)(i + 127), (BYTE)r, (BYTE)g, (BYTE)b);
        m_dv->SetPaletteIndex((BYTE)(i + 127), (BYTE)r, (BYTE)g, (BYTE)b);
    }

    m_dNormal->BlendPalette(color, coloring); //color the palette

    iDst = m_dh->GetBits(); //build the horiz. dotted focus bitmap
    j = (long)m_dh->GetWidth();
    for (i = 0; i < j; i++)
    {
        //		iDst[i]=64+127*(i%2);	//soft
        iDst[i] = 255 * (i % 2); //hard
    }

    iDst = m_dv->GetBits(); //build the vert. dotted focus bitmap
    j = (long)m_dv->GetHeight();
    for (i = 0; i < j; i++)
    {
        //		*iDst=64+127*(i%2);		//soft
        *iDst = 255 * (i % 2); //hard
        iDst += 4;
    }

    bytes = m_dNormal->GetLineWidth();
    iDst = m_dNormal->GetBits();
    posDst = iDst;
    long a, x, y, d, xs, idxmax, idxmin;

    int grainx2 = RAND_MAX / (max(1, 2 * granularity));
    idxmax = 255 - granularity;
    idxmin = granularity;

    switch (shadeID)
    {
            //----------------------------------------------------
        case 8: //SHS_METAL
            m_dNormal->Clear();
            // create the strokes
            k = 40; //stroke granularity
            for (a = 0; a < 200; a++)
            {
                x = rand() / (RAND_MAX / sXSize); //stroke postion
                y = rand() / (RAND_MAX / sYSize); //stroke position
                xs = rand() / (RAND_MAX / min(sXSize, sYSize)) / 2; //stroke lenght
                d = rand() / (RAND_MAX / k); //stroke color
                for (i = 0; i < xs; i++)
                {
                    if (((x - i) > 0) && ((y + i) < sYSize))
                        m_dNormal->SetPixelIndex(x - i, y + i, (BYTE)d);
                    if (((x + i) < sXSize) && ((y - i) > 0))
                        m_dNormal->SetPixelIndex(sXSize - x + i, y - i, (BYTE)d);
                }
            }
            //blend strokes with SHS_DIAGONAL
            posDst = iDst;
            a = (idxmax - idxmin - k) / 2;
            for (i = 0; i < sYSize; i++)
            {
                for (j = 0; j < sXSize; j++)
                {
                    d = posDst[j] + ((a * i) / sYSize + (a * (sXSize - j)) / sXSize);
                    posDst[j] = static_cast<BYTE>(d);
                    posDst[j] += static_cast<BYTE>(rand() / grainx2);
                }
                posDst += bytes;
            }

            break;
            //----------------------------------------------------
        case 7: // SHS_HARDBUMP
            //set horizontal bump
            for (i = 0; i < sYSize; i++)
            {
                k = (255 * i / sYSize) - 127;
                k = (k * (k * k) / 128) / 128;
                k = (k * (128 - granularity * 2)) / 128 + 128;
                for (j = 0; j < sXSize; j++)
                {
                    posDst[j] = static_cast<BYTE>(k);
                    posDst[j] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
                posDst += bytes;
            }
            //set vertical bump
            d = min(16, sXSize / 6); //max edge=16
            a = sYSize * sYSize / 4;
            posDst = iDst;
            for (i = 0; i < sYSize; i++)
            {
                y = i - sYSize / 2;
                for (j = 0; j < sXSize; j++)
                {
                    x = j - sXSize / 2;
                    xs = sXSize / 2 - d + (y * y * d) / a;
                    if (x > xs)
                        posDst[j] = static_cast<BYTE>(idxmin + static_cast<BYTE>((sXSize - j) * 128 / d));
                    if ((x + xs) < 0)
                        posDst[j] = static_cast<BYTE>(idxmax - static_cast<BYTE>(j * 128 / d));
                    posDst[j] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
                posDst += bytes;
            }
            break;
            //----------------------------------------------------
        case 6: //SHS_SOFTBUMP
            for (i = 0; i < sYSize; i++)
            {
                h = (255 * i / sYSize) - 127;
                for (j = 0; j < sXSize; j++)
                {
                    k = (255 * (sXSize - j) / sXSize) - 127;
                    k = (h * (h * h) / 128) / 128 + (k * (k * k) / 128) / 128;
                    k = k * (128 - granularity) / 128 + 128;
                    if (k < idxmin)
                        k = idxmin;
                    if (k > idxmax)
                        k = idxmax;
                    posDst[j] = static_cast<BYTE>(k);
                    posDst[j] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
                posDst += bytes;
            }
            break;
            //----------------------------------------------------
        case 5: // SHS_VBUMP
            for (j = 0; j < sXSize; j++)
            {
                k = (255 * (sXSize - j) / sXSize) - 127;
                k = (k * (k * k) / 128) / 128;
                k = (k * (128 - granularity)) / 128 + 128;
                for (i = 0; i < sYSize; i++)
                {
                    posDst[j + i * bytes] = static_cast<BYTE>(k);
                    posDst[j + i * bytes] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
            }
            break;
            //----------------------------------------------------
        case 4: //SHS_HBUMP
            for (i = 0; i < sYSize; i++)
            {
                k = (255 * i / sYSize) - 127;
                k = (k * (k * k) / 128) / 128;
                k = (k * (128 - granularity)) / 128 + 128;
                for (j = 0; j < sXSize; j++)
                {
                    posDst[j] = static_cast<BYTE>(k);
                    posDst[j] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
                posDst += bytes;
            }
            break;
            //----------------------------------------------------
        case 1: //SHS_DIAGSHADE
            a = (idxmax - idxmin) / 2;
            for (i = 0; i < sYSize; i++)
            {
                for (j = 0; j < sXSize; j++)
                {
                    posDst[j] = static_cast<BYTE>((idxmin + a * i / sYSize + a * (sXSize - j) / sXSize));
                    posDst[j] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
                posDst += bytes;
            }
            break;
            //----------------------------------------------------
        case 2: //SHS_HSHADE
            a = idxmax - idxmin;
            for (i = 0; i < sYSize; i++)
            {
                k = a * i / sYSize + idxmin;
                for (j = 0; j < sXSize; j++)
                {
                    posDst[j] = static_cast<BYTE>(k);
                    posDst[j] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
                posDst += bytes;
            }
            break;
            //----------------------------------------------------
        case 3: //SHS_VSHADE:
            a = idxmax - idxmin;
            for (j = 0; j < sXSize; j++)
            {
                k = a * (sXSize - j) / sXSize + idxmin;
                for (i = 0; i < sYSize; i++)
                {
                    posDst[j + i * bytes] = static_cast<BYTE>(k);
                    posDst[j + i * bytes] += static_cast<BYTE>(rand() / grainx2 - granularity);
                }
            }
            break;
            //----------------------------------------------------
        default: //SHS_NOISE
            for (i = 0; i < sYSize; i++)
            {
                for (j = 0; j < sXSize; j++)
                {
                    posDst[j] = static_cast<BYTE>(128 + rand() / grainx2 - granularity);
                }
                posDst += bytes;
            }
    }
    //----------------------------------------------------
    m_dDisabled->Clone(m_dNormal); //build the other bitmaps
    m_dOver->Clone(m_dNormal);
    m_dOver->BlendPalette(hicr, highlight);
    m_dDown->Clone(m_dOver);
} // End of SetShade

DWORD CShadeButtonST::OnDrawBackground(CDC* pDC, CRect* pRect)
{
    if (m_bMouseOnButton && !m_bIsBorder)
    {
        if (m_bIsPressed)
            m_dOver->Draw(pDC->GetSafeHdc(), 1, 1);
        else
            m_dOver->Draw(pDC->GetSafeHdc(), 1, 1);
    }
    else
        m_dNormal->Draw(pDC->GetSafeHdc(), 0, 0);
    if (m_bIsBorder)
    {
        if (m_bIsRounded)
            pDC->RoundRect(pRect, CPoint(8, 8));
        else
            pDC->Rectangle(pRect);
    }

    return BTNST_OK;
} // End of OnDrawBackground

DWORD CShadeButtonST::OnDrawBorder(CDC* pDC, CRect* pRect)
{
    if (!m_bIsBorder)
        CButtonST::OnDrawBorder(pDC, pRect);

    return BTNST_OK;
} // End of OnDrawBorder

// Border를 그리고 싶어서 수정함 KWAK
DWORD CShadeButtonST::SetRounded(BOOL bRounded, BOOL bShowBorder)
{
    m_bIsRounded = bRounded;
    m_bIsBorder = bShowBorder;

    Invalidate();

    return BTNST_OK;
} // End of SetRounded
void CShadeButtonST::DefaultStyle()
{
    SetFontBold(TRUE);
    SetAlign(CButtonST::ST_ALIGN_HORIZ);
    SetShade(CShadeButtonST::SHS_HSHADE, 1, 0, 65, RGB(255, 255, 255));
    SetRounded(TRUE, TRUE);
    DrawFlatFocus(FALSE);
    SetFontBold(TRUE);
    SetFontSize(8);
}
