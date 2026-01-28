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
class CVisionProcessingFOVPara
{
public:
    CVisionProcessingFOVPara(void);
    ~CVisionProcessingFOVPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};
