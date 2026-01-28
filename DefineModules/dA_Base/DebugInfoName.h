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

/////////////////////// [Carrier Tape Align] DebugInfoName ///////////////////////
#define _DEBUG_INFO_CTA_CARRIER_LEFT_EDGE_POINT _T("Carrier Tape Edge Point(Left)")
#define _DEBUG_INFO_CTA_CARRIER_RIGHT_EDGE_POINT _T("Carrier Tape Edge Point(Right)")
#define _DEBUG_INFO_CTA_CARRIER_EDGE_LINE _T("Carrier Tape Line")
//#define _DEBUG_INFO_CTA_SPROCKET_HOLE_ROUGH_SEARCH_ROI _T("Sprocket Hole Rough Search ROI")
//#define _DEBUG_INFO_CTA_SPROCKET_HOLE_LEFT_ROUGH_CENTER _T("Sprocket Hole Rough Center(Left)")
//#define _DEBUG_INFO_CTA_SPROCKET_HOLE_RIGHT_ROUGH_CENTER _T("Sprocket Hole Rough Center(Right)")
#define _DEBUG_INFO_CTA_CARRIER_TAPE_ALIGN_FRECT _T("Carrier Tape Align FRect")
#define _DEBUG_INFO_CTA_CARRIER_TAPE_ALIGN_RECT _T("Carrier Tape Align Rect") //검사에 사용
#define _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_QRECT _T("Pocket Rough Align QuadRect")
#define _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_RECT _T("Pocket Rough Align Rect")
#define _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_CENTER _T("Pocket Rough Align Center")
#define _DEBUG_INFO_CTA_POCKET_ROUGH_ALIGN_CENTER_GAP_Y _T("Pocket Rough Align Center Gap Y(mm)") //핸들러에 전송 필요

///////////////////////////////////////////////////////////////////////////////

////////////////////// [Tape Detail Align] DebugInfoName ///////////////////////
#define _DEBUG_INFO_TDA_SPROCKET_HOLE_SEARCH_ROI _T("Sprocket Hole Search ROI")
#define _DEBUG_INFO_TDA_SPROCKET_HOLE_SEARCH_LINE _T("Sprocket Hole Search Line")
#define _DEBUG_INFO_TDA_SPROCKET_HOLE_EDGE_POINT _T("Sprocket Hole Edge Point") //검사에 사용
#define _DEBUG_INFO_TDA_SPROCKET_HOLE_CIRCLE _T("Sprocket Hole Circle") //검사에 사용
#define _DEBUG_INFO_TDA_POCKET_SEARCH_ROI _T("Pocket Search ROI")
#define _DEBUG_INFO_TDA_POCKET_EDGE_POINT _T("Pocket Edge Point")
#define _DEBUG_INFO_TDA_POCKET_ALIGN_QRECT _T("Pocket Align QuadRect")
#define _DEBUG_INFO_TDA_POCKET_RECT _T("Pocket Align Rect") //검사에 사용
#define _DEBUG_INFO_TDA_DEVICE_SEARCH_ROI _T("Device Search ROI")
#define _DEBUG_INFO_TDA_DEVICE_EDGE_POINT _T("Device Edge Point")
#define _DEBUG_INFO_TDA_DEVICE_ALIGN_QRECT _T("Device Align QuadRect") //검사에 사용

/////////////////////////////////////////////////////////////////////////////////

////////////////////// [In-Pocket] DebugInfoName ///////////////////////

///////////////////////////////////////////////////////////////////////////////

////////////////////// [OTI] DebugInfoName ///////////////////////
#define _DEBUG_INFO_OTI_COVER_TAPE_SEARCH_ROI _T("Cover Tape Search ROI")
#define _DEBUG_INFO_OTI_COVER_TAPE_LEFT_EDGE_POINT _T("Cover Tape Edge Point(Left)")
#define _DEBUG_INFO_OTI_COVER_TAPE_RIGHT_EDGE_POINT _T("Cover Tape Edge Point(Right)")
#define _DEBUG_INFO_OTI_COVER_TAPE_EDGE_LEFT_LINE _T("Cover Tape Line(Left)")
#define _DEBUG_INFO_OTI_COVER_TAPE_EDGE_RIGHT_LINE _T("Cover Tape Line(Right)")
#define _DEBUG_INFO_OTI_COVER_TAPE_ALIGN_QRECT _T("Cover Tape Align QuadRect") //검사에 사용
#define _DEBUG_INFO_OTI_SEALING_SEARCH_ROI _T("Sealing Search ROI")
#define _DEBUG_INFO_OTI_LEFT_SEALING_LEFT_EDGE_POINT _T("Left Sealing Left Edge Point")
#define _DEBUG_INFO_OTI_LEFT_SEALING_RIGHT_EDGE_POINT _T("Left Sealing Right Edge Point")
#define _DEBUG_INFO_OTI_RIGHT_SEALING_LEFT_EDGE_POINT _T("Right Sealing Left Edge Point")
#define _DEBUG_INFO_OTI_RIGHT_SEALING_RIGHT_EDGE_POINT _T("Right Sealing Right Edge Point")
#define _DEBUG_INFO_OTI_SEALING_LINE _T("Sealing Line")
#define _DEBUG_INFO_OTI_SEALING_QRECT _T("Sealing QuadRect") //검사에 사용

///////////////////////////////////////////////////////////////////////////////