#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//

///////////////////// [In-Pocket Vision Items] //////////////////////////////////////////////////////
#define _INSP_ITEM_GUID_INPOCKET_EMPTY_STATUS _T("{01DD4C01-76A7-4BAE-9C74-2574B33A8670}")
#define _INSP_ITEM_GUID_INPOCKET_SPROCKET_HOLE_WIDTH _T("{B81B7FFB-F8D0-4AC5-99A4-00A66FC14A0D}")
#define _INSP_ITEM_GUID_INPOCKET_SPROCKET_HOLE_DAMAGE _T("{D8615238-3E39-4BFD-A4C8-D7297B62DB34}")
#define _INSP_ITEM_GUID_INPOCKET_MISPLACE _T("{09E20395-A9DF-4378-B869-339648712C51}")
#define _INSP_ITEM_GUID_INPOCKET_MISPLACE_ANGLE _T("{C9FC912E-87E5-4924-9987-34931B4594A8}")
#define _INSP_ITEM_GUID_INPOCKET_DEVICE_ORIENTATION _T("{E7C8BD02-5313-446A-BF6C-507597AEFEF5}")

enum enumInPocketInspection
{
    INPOCKET_INSPECTION_START = 0,
    INPOCKET_INSPECTION_EMPTY_STATUS = INPOCKET_INSPECTION_START,
    INPOCKET_INSPECTION_SPROCKET_HOLE_WIDTH,
    INPOCKET_INSPECTION_SPROCKET_HOLE_DAMAGE,
    INPOCKET_INSPECTION_MISPLACE,
    INPOCKET_INSPECTION_MISPLACE_ANGLE,
    INPOCKET_INSPECTION_DEVICE_ORIENTATION,
    INPOCKET_INSPECTION_END,
};

static LPCTSTR g_szInPocketInspectionName[] = {
    _T("Empty Status"),
    _T("Sprocket Hole Width"),
    _T("Sprocket Hole Damage"),
    _T("Misplace"),
    _T("Misplace(Angle)"),
    _T("Device Orientation"),
};
//////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////// [OTI Vision Items] ////////////////////////////////////////////////////

#define _INSP_ITEM_GUID_OTI_EMPTY_STATUS _T("{68C5E4CD-776B-487F-BBB6-C9B8513C8273}")
#define _INSP_ITEM_GUID_OTI_COVER_TAPE_SHIFT _T("{50365743-B453-4DEC-B5E8-6CC051D2E6D2}")
#define _INSP_ITEM_GUID_OTI_SEALING_SHIFT _T("{BEDB9922-A919-4631-8F7A-81F8B664E460}")
#define _INSP_ITEM_GUID_OTI_SEALING_WIDTH _T("{A8742D1B-35AA-4A44-9206-C2DF2643E26F}")
#define _INSP_ITEM_GUID_OTI_SEALING_SPAN _T("{C0F78463-1EB1-4ABC-9677-F15B80B56DE2}")
//#define _INSP_ITEM_GUID_OTI_SEALING_CUTTING _T("{6F288253-D9F8-42AB-9C5C-857912B6ECF6}")
#define _INSP_ITEM_GUID_OTI_SEALING_PARALLELISM _T("{D3E3C2D1-1F4E-4F2A-8D6E-3C8E3B6F9AFA}")

enum enumOTIInspection
{
    OTI_INSPECTION_START = 0,
    OTI_INSPECTION_EMPTY_STATUS = OTI_INSPECTION_START,
    OTI_INSPECTION_COVER_TAPE_START,
    OTI_INSPECTION_COVER_TAPE_SHIFT = OTI_INSPECTION_COVER_TAPE_START,
    OTI_INSPECTION_COVER_TAPE_END,
    OTI_INSPECTION_SEALING_START = OTI_INSPECTION_COVER_TAPE_END,
    OTI_INSPECTION_SEALING_SHIFT = OTI_INSPECTION_SEALING_START,
    OTI_INSPECTION_SEALING_WIDTH,
    OTI_INSPECTION_SEALING_SPAN,
   // OTI_INSPECTION_SEALING_CUTTING,
    OIT_INSPECTION_SEALING_PARALLELISM,
    OTI_INSPECTION_SEALING_END,
    OTI_INSPECTION_END = OTI_INSPECTION_SEALING_END,
};

static LPCTSTR g_szOTIInspectionName[] = {
    _T("Empty Status"),
    _T("Cover Tape Shift"),
    _T("Sealing Shift"),
    _T("Sealing Width"),
    _T("Sealing Span"),
  //  _T("Sealing Cutting"), 
    _T("Sealing Parallelism")};
////////////////////////////////////////////////////////////////////////////////////////////

enum enumFillPocketStatus
{
    FILL_POCKET_EMPTY = 0,
    FILL_POCKET_FILL = 1,
};