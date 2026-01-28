#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
#define MAX_POCKET_COUNT 6

//HDR_6_________________________________ Header body
//

struct FillPocketResult
{
    BOOL PocketData[MAX_POCKET_COUNT];
};
