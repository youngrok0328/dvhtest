#pragma once
//=====================================================================================
// Tray Scan Spec을 상속받아 비전에서 사용하는 클래스
// Vision에서만 사용하는 일부 함수나 기능을 추가하여 사용하기 위해 상속받아서 만들었다.
//=====================================================================================

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Enum.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class VisionTrayScanSpec;

//HDR_6_________________________________ Header body
//
class DPI_DATABASE_API VisionTapeSpec
{
public:
    VisionTapeSpec() = default;
    BOOL LinkDataBase(BOOL save, CiDataBase& db);
    void ApplyToTrayScanSpec(VisionTrayScanSpec& trayScanSpec) const;

    TapeSprocketHoleExistType m_sprocketHoleExistType = TapeSprocketHoleExistType::e_both; // Sprocket Hole Exist Type
    float m_w0 = 16.f; // carrier tape width

    float m_a0 = 7.f; // pocket height
    float m_b0 = 7.f; // pocket width

    float m_p0 = 4.f; // sprocket hole pitch
    float m_p1 = 12.f; // pcocket pitch

    float m_d0 = 3.f; // Pocket Center hole diameter
    float m_d1 = 1.f; //Sprocket Hole diameter

    float m_s0 = 14.f; //Sprocket Hole Span
    float m_s1 = 12.f; // Cover Tape Width
    float m_s2 = 10.f; // Sealing Pitch

    float m_e1 = 1.f; // Carrier Tape Edge To Sprocket Hole Distance

    float m_f0 = 7.f; // Sprocket Hole Center To Pocket Center Distance
    float m_f1 = 1.f; // Sprocket Hole Center To Cover Tape Edge Distance
    float m_f2 = 2.f; // Sprocket Hole Center To Sealing Edge Distance
};
