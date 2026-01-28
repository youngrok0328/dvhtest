#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionInspectionSpec;

//HDR_6_________________________________ Header body
//
class CInspectionSpecConfig
{
public:
    CInspectionSpecConfig(void);
    virtual ~CInspectionSpecConfig(void);

    void SaveIni(std::vector<VisionInspectionSpec> vecSpecData);
    void LoadIni(std::vector<VisionInspectionSpec>& vecSpecData);
};
