#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "XTPPropertyGridItemCustomValidator.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
template<typename T>
class XTPPropertyGridItemCustomFloat : public CXTPPropertyGridItem
{
public:
    XTPPropertyGridItemCustomFloat(LPCTSTR strCaption, const T originalValue, LPCTSTR strFormat, T& editingValue,
        const T minValue, const T maxValue, const T epsilon)
        : CXTPPropertyGridItem(strCaption)
        , m_originalValue(originalValue)
        , m_strFormat(strFormat)
        , m_editingValue(editingValue)
        , m_validator(new XTPPropertyGridItemCustomValidator<T>(minValue, maxValue, epsilon))
    {
        m_strDefaultValue = ToString(editingValue);
        SetValue(m_strDefaultValue);
    }

    virtual ~XTPPropertyGridItemCustomFloat()
    {
        delete m_validator;
    }

    void SetValue(LPCTSTR strFormat, T value)
    {
        m_strFormat = strFormat;
        CString str;
        str.Format(_T("%lf"), value);

        SetValue(str);
    }

    void SetValue(T value)
    {
        CString str;
        str.Format(_T("%lf"), value);

        SetValue(str);
    }

    void SetMinMaxDelta(const T minValue, const T maxValue, const T delta)
    {
        delete m_validator;
        m_validator = new XTPPropertyGridItemCustomValidator<T>(minValue, maxValue, delta);

        SetValue(m_editingValue);
    }

protected:
    virtual void SetValue(CString strValue) override
    {
        m_editingValue = m_validator->Validate(T(::_ttof(strValue)));

        CXTPPropertyGridItem::SetValue(ToString(m_editingValue));
    }

    virtual void OnBeforeInsert() override
    {
        m_strValue = ToString(m_editingValue);
    }

    virtual BOOL IsValueChanged() const override
    {
        return fabs(m_editingValue - m_originalValue) >= m_validator->m_deltaValue;
    }

protected:
    CString ToString(T value)
    {
        CString strFormat(m_strFormat);
        if (strFormat.IsEmpty())
            strFormat = _T("%0.2f");

        CString strValue;
        strValue.Format(strFormat, value);

        return strValue;
    }

protected:
    const T m_originalValue;
    CString m_strFormat;
    T& m_editingValue; // Binded value.  This is a pointer to the variable bound to this item.
    XTPPropertyGridItemCustomValidator<T>* m_validator;
};
