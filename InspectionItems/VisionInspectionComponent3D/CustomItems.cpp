//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CustomItems.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef IDC_HAND
#define IDC_HAND MAKEINTRESOURCE(32649)
#endif

//CPP_7_________________________________ Implementation body
//
BEGIN_MESSAGE_MAP(CInplaceCheckBox, CButton)
ON_MESSAGE(BM_SETCHECK, OnCheck)
ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

HBRUSH CInplaceCheckBox::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
    class CGridView : public CXTPPropertyGridView
    {
        friend class CInplaceCheckBox;
    };

    CGridView* pGrid = (CGridView*)m_pItem->m_pGrid;

    COLORREF clr = pGrid->GetPaintManager()->GetItemMetrics()->m_clrBack;

    if (clr != m_clrBack || !m_brBack.GetSafeHandle())
    {
        m_brBack.DeleteObject();
        m_brBack.CreateSolidBrush(clr);
        m_clrBack = clr;
    }

    pDC->SetBkColor(m_clrBack);
    return m_brBack;
}

LRESULT CInplaceCheckBox::OnCheck(WPARAM wParam, LPARAM lParam)
{
    m_pItem->m_bValue = (wParam == BST_CHECKED);
    m_pItem->OnValueChanged(m_pItem->GetValue());

    return CButton::DefWindowProc(BM_SETCHECK, wParam, lParam);
}

CCustomItemCheckBox::CCustomItemCheckBox(CString strCaption)
    : CXTPPropertyGridItem(strCaption)
{
    m_wndCheckBox.m_pItem = this;
    m_nFlags = 0;
    m_bValue = FALSE;
}

void CCustomItemCheckBox::OnDeselect()
{
    CXTPPropertyGridItem::OnDeselect();

    if (m_wndCheckBox.m_hWnd)
        m_wndCheckBox.DestroyWindow();
}

void CCustomItemCheckBox::OnSelect()
{
    CXTPPropertyGridItem::OnSelect();

    if (!m_bReadOnly)
    {
        CRect rc = GetValueRect();
        rc.left -= XTP_DPI_X(15);
        rc.right = rc.left + XTP_DPI_X(15);

        if (!m_wndCheckBox.m_hWnd)
        {
            m_wndCheckBox.Create(NULL, WS_CHILD | BS_AUTOCHECKBOX | BS_FLAT, rc, (CWnd*)m_pGrid, 0);
        }
        if (m_wndCheckBox.GetCheck() != m_bValue)
            m_wndCheckBox.SetCheck(m_bValue);
        m_wndCheckBox.MoveWindow(rc);
        m_wndCheckBox.ShowWindow(SW_SHOW);
    }
}

CRect CCustomItemCheckBox::GetValueRect()
{
    CRect rcValue(CXTPPropertyGridItem::GetValueRect());
    rcValue.left += XTP_DPI_X(17);
    return rcValue;
}

BOOL CCustomItemCheckBox::OnDrawItemValue(CDC& dc, CRect rcValue)
{
    CRect rcText(rcValue);

    if (m_wndCheckBox.GetSafeHwnd() == 0 && m_bValue)
    {
        CRect rcCheck(rcText.left, rcText.top, rcText.left + XTP_DPI_X(13), rcText.bottom - XTP_DPI_Y(1));
        dc.DrawFrameControl(rcCheck, DFC_MENU, DFCS_MENUCHECK);
    }

    rcText.left += XTP_DPI_X(17);
    dc.DrawText(GetValue(), rcText, DT_SINGLELINE | DT_VCENTER);
    return TRUE;
}

BOOL CCustomItemCheckBox::GetBool()
{
    return m_bValue;
}
void CCustomItemCheckBox::SetBool(BOOL bValue)
{
    m_bValue = bValue;

    if (m_wndCheckBox.GetSafeHwnd())
        m_wndCheckBox.SetCheck(bValue);
}

BOOL CCustomItemCheckBox::IsValueChanged()
{
    return !m_bValue;
}

////////////////////////////////////////////////////////////////////////////////////////////////

class CCustomItemWidthLength::CCustomItemChildsWidthLength : public CXTPPropertyGridItemDouble
{
public:
    CCustomItemChildsWidthLength(CString strCaption, double& nPad)
        : CXTPPropertyGridItemDouble(strCaption)
        , m_nPad(nPad)
    {
    }
    virtual void OnValueChanged(CString strValue)
    {
        SetValue(strValue);

        CCustomItemWidthLength* pParent = ((CCustomItemWidthLength*)m_pParent);
        m_nPad = GetDouble();
        pParent->OnValueChanged(pParent->DoubleToString(pParent->m_Width, pParent->m_Length));
    }
    double& m_nPad;
};

CCustomItemWidthLength::CCustomItemWidthLength(
    CString strCaption, CString strFirstTitle, CString strSecondTitle, CString Unit, double wValue, double lValue)
    : CXTPPropertyGridItem(strCaption)
{
    strTitle1 = strFirstTitle;
    strTitle2 = strSecondTitle;
    strUnit = Unit;
    m_Width = wValue;
    m_Length = lValue;
    m_strValue = DoubleToString(wValue, lValue);
    m_nFlags = 0;
}

void CCustomItemWidthLength::OnAddChildItem()
{
    /*double tmpWidth = floor(100.*(m_Width+0.005))/100.;
	double tmpLength = floor(100.*(m_Length + 0.005)) / 100.;*/

    m_itemWidth = (CCustomItemChildsWidthLength*)AddChildItem(new CCustomItemChildsWidthLength(strTitle1, m_Width));
    m_itemLength = (CCustomItemChildsWidthLength*)AddChildItem(new CCustomItemChildsWidthLength(strTitle2, m_Length));

    UpdateChilds();
}

void CCustomItemWidthLength::UpdateChilds()
{
    m_Width = floor(100. * (m_Width + 0.005)) / 100.;
    m_Length = floor(100. * (m_Length + 0.005)) / 100.;

    m_itemWidth->SetDouble(m_Width);
    m_itemLength->SetDouble(m_Length);
}

void CCustomItemWidthLength::SetValue(CString strValue)
{
    CXTPPropertyGridItem::SetValue(strValue);
    UpdateChilds();
}

CString CCustomItemWidthLength::DoubleToString(double wValue, double lValue)
{
    CString str;
    str.Format(_T("%.2f%s; %.2f%s"), wValue, (LPCTSTR)strUnit, lValue, (LPCTSTR)strUnit);
    return str;
}

double CCustomItemWidthLength::GetFirstValue()
{
    return m_Width;
}

double CCustomItemWidthLength::GetSecondValue()
{
    return m_Length;
}