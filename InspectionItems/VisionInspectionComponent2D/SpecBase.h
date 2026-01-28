#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class SpecBase
{
public:
    SpecBase(const SpecBase& object) = delete;
    SpecBase& operator=(const SpecBase& object) = delete;

    SpecBase();
    BOOL CopyFrom(SpecBase& object);
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) = 0;
};
