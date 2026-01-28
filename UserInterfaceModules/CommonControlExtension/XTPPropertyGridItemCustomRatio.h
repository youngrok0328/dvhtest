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
class XTPPropertyGridItemCustomRatio : public CXTPPropertyGridItem
{
public:
    XTPPropertyGridItemCustomRatio(LPCTSTR strCaption, const T originalValue, const T referenceValue,
        LPCTSTR strFormatRatio, LPCTSTR strFormatDesc, T& editingValue, const T minValue, const T maxValue,
        const T epsilon)
        : CXTPPropertyGridItem(strCaption)
        , m_originalValue(originalValue)
        , m_referenceValue(referenceValue)
        , m_strFormatRatio(strFormatRatio)
        , m_strFormatDesc(strFormatDesc)
        , m_editingValue(editingValue)
        , m_validator(new XTPPropertyGridItemQpmValidator<T>(minValue, maxValue, epsilon))
    {
        m_strDefaultValue = ToString(editingValue);
        SetValue(m_strDefaultValue);
    }

    virtual ~XTPPropertyGridItemCustomRatio()
    {
        delete m_validator;
    }

public:
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
        return fabs(m_editingValue - m_originalValue) >= m_validator->m_deltaValue);
    }

protected:
    CString ToString(T value)
    {
        CString strValue;
        strValue.Format(m_strFormatRatio, value);

        CString strDesc;
        strDesc.Format(m_strFormatDesc, m_referenceValue * value);

        return strValue + strDesc;
    }

protected:
    const T m_originalValue;
    const T m_referenceValue;
    const CString m_strFormatRatio;
    const CString m_strFormatDesc;
    T& m_editingValue; // Binded value.  This is a pointer to the variable bound to this item.
    XTPPropertyGridItemQpmValidator<T>* m_validator;
};
