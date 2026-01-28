#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum class VisionSurfaceCriteria_Column : long // > (min), < (max)
{
    // 공용
    Area, //Convert^2
    Defect_num,
    Dist_X_to_BodyCenter, //Convert
    Dist_Y_to_BodyCenter, //Convert
    Dist_to_Body, //Convert
    Width, //Convert
    Length, //Convert
    LengthX, //Convert
    LengthY, //Convert
    TotalLength, //Convert
    LocalArea_Percent_all,
    LocalArea_Percent,
    DefectROI_ratio,
    Thickness, //Convert
    Locus, //Convert
    Aspect_ratio,
    Binary_density,

    // 3D
    t3D_AvgDeltaHeight, //Convert
    t3D_KeyDeltaHeight, //Convert

    // 2D
    t2D_KeyContrast,
    t2D_AvgContrast,
    t2D_LocalContrast,
    t2D_AuxLocalContrast,
    t2D_Deviation,
    t2D_AuxDeviation,
    t2D_EdgeEnegry,
    t2D_MergedCount, // Merged Blob Count
    t2D_BlobGV_AVR, // Blob Average GV value
    t2D_BlobGV_Min, // Blob Min GV value
    t2D_BlobGV_Max, // Blob Max GV value

    Surface_Criteria_END
};

static CString g_strVisionSurfCriteria_Col[] = {
    _T("Area"),
    _T("DefectNum"),
    _T("XtoBodyC"),
    _T("YtoBodyC"),
    _T("ToBody"),
    _T("Width"),
    _T("Length"),
    _T("LengthX"),
    _T("LengthY"),
    _T("Total Length"),
    _T("L_Area_AllPercent"),
    _T("L_AreaPercent"),
    _T("Defect_ROI_Ratio"),
    _T("Thickness"),
    _T("Locus"),
    _T("A_Ratio"),
    _T("B_Density"),

    _T("AvgD_Height"),
    _T("KeyD_Height"),

    _T("K_Contrast"),
    _T("A_Contrast"),
    _T("L_Contrast"),
    _T("L_Contrast2"),
    _T("Deviation"),
    _T("Deviation2"),
    _T("E_Energy"),
    _T("Merged Blob Count"),
    _T("Blob AVR GV"),
    _T("Blob Min GV"),
    _T("Blob Max GV"),
};
