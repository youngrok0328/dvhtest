#pragma once
#include "VersionInfo.h"

/////////////////////////////////////////////////////////////////
// Directory 는 항상 "\\" 를 끝에 붙여준다.
#define CRASH_DUMP_DIRECTORY _T("C:\\IntekPlus\\iPIS-500I\\Vision\\Crash Dump\\")
#define CRASH_DUMPFILE_PATH CRASH_DUMP_DIRECTORY _T("MiniDump.dmp")
#define RAW_IMAGE_DIRECTORY _T("D:\\Raw images\\")
#define REVIEW_IMAGE_DIRECTORY _T("D:\\Review Image\\")
#define CROPPING_IMAGE_DIRECTORY _T("D:\\Cropping Image\\")

#define LED_ILLUM_FRAME_MAX 10
#define LED_ILLUM_CHANNEL_MAX 16
#define LED_ILLUM_CHANNEL_RING_MAX 3
#define LED_ILLUM_NGRV_CHANNEL_MAX 12
#define LED_ILLUM_CHANNEL_OBLIQUE 11 //SDY_DualCal 경사조명의 갯수를 매크로로 처리한다.
#define LED_ILLUM_CHANNEL_DEFAULT 13
#define LED_ILLUM_CHANNEL_SIDE_DEFAULT 10
#define LED_ILLUM_CHANNEL_SIDE_OBLIQUE 9
#define LED_ILLUM_CHANNEL_SWIR_DEFAULT 14
#define LED_ILLUM_CHANNEL_INPOCKET_DEFAULT 3
#define LED_ILLUM_CHANNEL_OTI_DEFAULT 3
#define NUM_OF_MARKMULTI 3
#define IV_DEG_TO_RAD (0.017453292519943)
#define IV_RAD_TO_DEG (57.295779513082)
#define MAX_DL_IMAGE_SIZE_X 10240
#define MAX_DL_IMAGE_SIZE_Y 10240
#define MAX_SEND_RESULT_SIZE 40000000
