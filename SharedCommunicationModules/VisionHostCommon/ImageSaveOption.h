#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ImageSaveOption
{
public:
    enum VisionType
    {
        VisionType_Start = 0,
        VisionType_2D_TOP = VisionType_Start,
        VisionType_2D_BTM,
        VisionType_3D_TOP,
        VisionType_3D_BTM,
        VisionType_End,
    };

    //RawImage SaveOption
    //vector에 하나씩 집어넣어서 사용합니다.
    enum RawImageSaveOption
    {
        RawImage_Start = 0,
        RawImage_AllPackages = RawImage_Start,
        RawImage_Pass,
        RawImage_Marginal,
        RawImage_Reject,
        RawImage_Invalid,
        RawImage_Empty,
        RawImage_End,
    };

    enum ReviewImageROI
    {
        ReviewROI_Start = 0,
        ReviewROI_Not = ReviewROI_Start,
        ReviewROI_Insert,
        ReviewROI_End,
    };

    enum CroppingImageSaveOption
    {
        CroppingImage_Start = 0,
        CroppingImage_All = CroppingImage_Start,
        CroppingImage_NotPass,
        CroppingImage_Marginal,
        CroppingImage_Reject,
        CroppingImage_End,
    };

    enum CroppingImageROI
    {
        CroppingROI_Start = 0,
        CroppingROI_Not = CroppingROI_Start,
        CroppingROI_Insert,
        CroppingROI_End,
    };

    enum CroppingImageFrameOption
    {
        CroppingFrame_Start = 0,
        CroppingFrame_NotSave = CroppingFrame_Start,
        CroppingFrame_Inspection,
        CroppingFrame_Color,
        CroppingFrame_Frame01,
        CroppingFrame_Frame02,
        CroppingFrame_Frame03,
        CroppingFrame_Frame04,
        CroppingFrame_Frame05,
        CroppingFrame_Frame06,
        CroppingFrame_Frame07,
        CroppingFrame_Frame08,
        CroppingFrame_Frame09,
        CroppingFrame_Frame10,
        CroppingFrame_End,
    };

    enum NGRVImageSaveFormat
    {
        NGRV_IMAGE_FORMAT_BMP = 0,
        NGRV_IMAGE_FORMAT_JPG,
        NGRV_IMAGE_FORMAT_PNG,
        NGRV_IMAGE_FORMAT_END,
    };
};
