#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CCustomItemCheckBox;

//HDR_6_________________________________ Header body
//
class CInplaceCheckBox : public CButton
{
public:
    afx_msg LRESULT OnCheck(WPARAM wParam, LPARAM lParam);
    afx_msg HBRUSH CtlColor(CDC* pDC, UINT /*nCtlColor*/);

    DECLARE_MESSAGE_MAP()

protected:
    CCustomItemCheckBox* m_pItem;
    COLORREF m_clrBack;
    CBrush m_brBack;

    friend class CCustomItemCheckBox;
};

class CCustomItemCheckBox : public CXTPPropertyGridItem
{
protected:

public:
    CCustomItemCheckBox(CString strCaption);

    BOOL GetBool();
    void SetBool(BOOL bValue);

protected:
    virtual void OnDeselect();
    virtual void OnSelect();
    virtual CRect GetValueRect();
    virtual BOOL OnDrawItemValue(CDC& dc, CRect rcValue);

    virtual BOOL IsValueChanged();

private:
    CInplaceCheckBox m_wndCheckBox;
    BOOL m_bValue;

    friend class CInplaceCheckBox;
};

class CCustomItemWidthLength : public CXTPPropertyGridItem
{
    class CCustomItemChildsWidthLength;

    friend class CCustomItemChildsWidthLength;

public:
    CCustomItemWidthLength(CString strCaption, CString strFirstTitle, CString strSecondTitle, CString strUnit,
        double wValue, double lValue);
    double GetFirstValue();
    double GetSecondValue();

protected:
    virtual void OnAddChildItem();
    virtual void SetValue(CString strValue);

private:
    void UpdateChilds();
    CString DoubleToString(double wValue, double lValue);

private:
    CCustomItemChildsWidthLength* m_itemWidth;
    CCustomItemChildsWidthLength* m_itemLength;

    double m_Width;
    double m_Length;

    CString strTitle1;
    CString strTitle2;

    CString strUnit;
};