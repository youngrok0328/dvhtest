#pragma once

// 공유 메모리 키를 여기서 정의한다.
// 한 번 정해진 공유 메모리 키는 변경하지 않는다.
// 필요한 경우 새로운 이름을 추가한다.

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//

#define SHAREDMEMORY_INTERAFCE_START _T("AE7D0E98-E7BD-4A71-B8B2-236FF8F8A0D7")
#define SHAREDMEMORY_INTERAFCE_VISION_READY _T("EC48F4FA-A065-406C-8A36-89DB2CA62961")
#define SHAREDMEMORY_INTERAFCE_VISION_ACQUISITION _T("0054244C-BB40-4503-9E59-D69A224D9E02")
#define SHAREDMEMORY_INTERAFCE_VISION_EXPOSE _T("255E4983-5C3B-44C6-8EE1-22536635C030")

#define SHAREDMEMORY_INTERAFCE_STRIP_ID _T("19CEA40B-B40E-425E-A880-E9E590951972")
#define SHAREDMEMORY_INTERAFCE_SCAN_AREA_ID _T("97F15B73-9605-4721-BA36-52129987F46D")
#define SHAREDMEMORY_INTERAFCE_UNIT_COUNT_X _T("CFD1DE67-90C4-47B3-B776-D5094A59C9CF")
#define SHAREDMEMORY_INTERAFCE_UNIT_COUNT_Y _T("C3F6D5DE-2F51-4394-BF78-53C14039D467")

#define SHAREDMEMORY_INTERAFCE_FILL_POCKET _T("F90C6EFB-F0D8-44E5-A117-F2B8BC114581")
#define SHAREDMEMORY_INTERAFCE_GRAB_NUMBER _T("8FCE2F82-F98D-4EFB-8568-6F89123FDCDF")
#define SHAREDMEMORY_INTERAFCE_FAULT _T("E9C0A63D-9CA0-448A-967A-6ABE254A4BD5")

inline CString MarcoMakeKey(const wchar_t* base, int id)
{
    CString key;
    key.Format(_T("%s_%d"), base, id);
    return key;
}
