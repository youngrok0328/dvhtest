#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ResultDevice.h"
#include "ResultPocket.h"
#include "ResultSprocketHole.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Polygon32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class Result_Pad
{
public:
    bool m_enable; // 활성화된 PAD 인가
    CString m_name;
    PAD_TYPE m_type;
    FPI_RECT m_spec_px;
    FPI_RECT m_align_px;
};

class Result
{
public:
    Result();
    ~Result();

    void Init();

    std::vector<ResultSprocketHole> m_sprocketHoles;
    Ipvm::Point32r2 m_sprocketHoleOffset_px {0.f, 0.f};
    ResultPocket m_pocket;
    ResultDevice m_device;
};
