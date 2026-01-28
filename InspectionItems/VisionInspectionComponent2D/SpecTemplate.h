#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
template<class _specClass>
class SpecTemplate
{
public:
    SpecTemplate(VisionProcessing& parent, long count = 0)
        : m_parent(parent)
    {
        resize(count);
    }

    ~SpecTemplate()
    {
        for (auto* spec : m_specs)
        {
            delete spec;
        }
    }

    long size() const
    {
        return long(m_specs.size());
    }

    void clear()
    {
        resize(0);
    }

    _specClass& operator[](long index)
    {
        return *m_specs[index];
    }

    void push_back(_specClass& object)
    {
        long index = size();
        resize(index + 1);
        m_specs[index]->CopyFrom(object);
    }

    void erase(long index)
    {
        delete m_specs[index];
        m_specs.erase(m_specs.begin() + index);
    }

    _specClass& add()
    {
        long index = size();
        resize(index + 1);

        return *m_specs[index];
    }

    void resize(long size)
    {
        if (size >= long(m_specs.size()))
        {
            while (size != long(m_specs.size()))
            {
                m_specs.push_back(new _specClass(m_parent));
            }
        }
        else
        {
            for (long n = size; n < long(m_specs.size()); n++)
            {
                delete m_specs[n];
            }

            m_specs.resize(size);
        }
    }

private:
    CString m_specGUID;

    std::vector<_specClass*> m_specs;

    VisionProcessing& m_parent;
};
