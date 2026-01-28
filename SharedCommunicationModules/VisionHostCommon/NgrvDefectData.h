#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
//헤더충돌 때문에 우회해서 사용....
struct NgrvDefectData
{
    CString UnitID{_T("Unknown")};
    CString Vision{_T("Unknown")};
    CString Name{_T("Unknown")}; //ImageData와 매핑을 위해 추가
    int ITEM_ID{-1};
    int DEFECT_ID{-1};
    CString Code{_T("Unknown")};
    CString Disposition{_T("Unknown")};
    CString DefectInspectionType{_T("Unknown")};
    CString DefectInspectionName{_T("Unknown")};
    CString DefectStatus{_T("Unknown")};
    int DefectXLocation{-1};
    int DefectYLocation{-1};
    int DefectLength{-1};
    int DefectWidth{-1};
};
