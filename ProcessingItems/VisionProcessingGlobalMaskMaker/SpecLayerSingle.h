#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "Predefine.h"
#include "SpecRoi.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class SpecLayerSingle
{
public:
    SpecLayerSingle();
    ~SpecLayerSingle();

    void Init();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    const SpecRoi& operator[](long index) const;
    SpecRoi& operator[](long index);

    long count() const;
    void add(UserRoiType type);
    bool swap(long index1, long index2);
    void del(long index);

private:
    //-------------------------------------------------
    // Single Layer Info
    //-------------------------------------------------
    std::vector<SpecRoi> m_rois;
};
