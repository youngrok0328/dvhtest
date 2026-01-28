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
enum class DefineFolder
{
    Root,
    Binary,
    Config,
    System,
    Image,
    Job,
    Job_ChipAlgo,
    Job_PadAlignAlgo,
    Log,
    TimeLog,
    Pacakge_ViewerData,
    MarkImage,
    Temp,
};

enum class DefineFile
{
    System,
    IllumTableRefTable,
    IllumTableTable,
    SlitbeamDistortionProfile,
    IllumTableRefTableAll, //kircheis_IllumCal
    IllumTableTableAll, //kircheis_IllumCal

    IllumTableRefTableMirror,
    IllumTableTableMirror,
    IllumTableRefTableAllMirror, //SD_Dualcal
    IllumTableTableAllMirror, //SD_Dualcal

    //SD_SideVision
    IllumTableTableAll_F,
    IllumTableTableAllMirror_F,
    IllumTableTableAll_R,
    IllumTableTableAllMirror_R,
};

class _DA_BASE_MODULE_API DynamicSystemPath
{
public:
    static void setModulePath(LPCTSTR modulePath);
    static CString get(DefineFolder type);
    static CString get(DefineFile type);
    static CString getDistortionFullFilePath(
        double scanDepth_um, bool cameraVerticalBinning, bool isToUseNearestDistortionXEnabled);
};
