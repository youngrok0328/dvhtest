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
class XTPPropertyGridItemCustomValidator
{
public:
    XTPPropertyGridItemCustomValidator() = delete;
    XTPPropertyGridItemCustomValidator(const T minValue, const T maxValue, const T deltaValue)
        : m_minValue(minValue)
        , m_maxValue(maxValue)
        , m_deltaValue(deltaValue)
    {
    }
    ~XTPPropertyGridItemCustomValidator() = default;

    T Validate(const T value)
    {
        T ret = value;

        if (ret < m_minValue)
        {
            ret = m_minValue;
        }
        else if (ret > m_maxValue)
        {
            ret = m_maxValue;
        }

        if (m_deltaValue != 0)
        {
            ret = static_cast<T>(::round(ret / m_deltaValue) * m_deltaValue);
        }

        return ret;
    }

    const T m_minValue;
    const T m_maxValue;
    const T m_deltaValue;
};
