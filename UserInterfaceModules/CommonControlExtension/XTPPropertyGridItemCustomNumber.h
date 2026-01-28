#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
template<typename T>
class XTPPropertyGridItemCustomNumber : public CXTPPropertyGridItem
{
public:
    XTPPropertyGridItemCustomNumber(LPCTSTR strCaption, const T originalValue, LPCTSTR strFormat, T& editingValue,
        const T minValue = 0, const T maxValue = 0)
        : CXTPPropertyGridItem(strCaption)
        , m_originalValue(originalValue)
        , m_strFormat(strFormat)
        , m_editingValue(editingValue)
        , m_useRange(minValue != 0 || maxValue != 0)
        , m_minValue(minValue)
        , m_maxValue(maxValue)
    {
        m_strDefaultValue = ToString(editingValue);
        SetValue(m_strDefaultValue);
    }

    virtual ~XTPPropertyGridItemCustomNumber()
    {
    }

protected:
    virtual void SetValue(CString strValue) override
    {
        m_editingValue = T(::_ttoi(strValue));

        if (m_useRange)
        {
            if (m_editingValue < m_minValue)
            {
                m_editingValue = m_minValue;
            }
            else if (m_editingValue > m_maxValue)
            {
                m_editingValue = m_maxValue;
            }
        }

        CXTPPropertyGridItem::SetValue(ToString(m_editingValue));
    }

    virtual void OnBeforeInsert() override
    {
        m_strValue = ToString(m_editingValue);
    }

    virtual BOOL IsValueChanged() const override
    {
        return m_editingValue != m_originalValue;
    }

protected:
    CString ToString(T value)
    {
        CString strValue;
        strValue.Format(m_strFormat, value);

        return strValue;
    }

protected:
    const T m_originalValue;
    const CString m_strFormat;
    T& m_editingValue;
    bool m_useRange;
    const T m_minValue;
    const T m_maxValue;
};
