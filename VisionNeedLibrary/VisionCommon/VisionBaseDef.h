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
//--------------------------------------------------------------------------------------------------
const float g_fMilToUm = 25.4f;
const float g_fMilToMm = 0.0254f;
//--------------------------------------------------------------------------------------------------

//Predefine Vision Insp DLL GUID
#define _VISION_INSP_GUID_PACKAGE_SPEC _T("{2D2EA6F1-AB3C-48DC-8EAF-68B88EEFD0C4}")
#define _VISION_INSP_GUID_FOV _T("{586F13B8-AF4F-439D-8DA5-5FC49D2440F4}")
#define _VISION_INSP_GUID_COUPON_2D _T("{235A6A31-D50A-4EF0-81CF-114F7C03EFAF}") //SDY_Coupon
#define _VISION_INSP_GUID_CARRIER_TAPE_ALIGN _T("{C6582ABB-AADC-4772-8A3B-D50FA0F5CE0A}")
#define _VISION_INSP_GUID_TAPE_DETAIL_ALIGN _T("{C2FF7F77-36B0-4A2C-A689-2758B00411AB}")
#define _VISION_INSP_GUID_ALIGN_2D _T("{C19EBF32-85E4-4E03-8CD2-87A415A4FBFB}")
#define _VISION_INSP_GUID_ALIGN_3D _T("{8729E14F-11EF-4EB7-8C5D-4ED3AD725925}")
#define _VISION_INSP_GUID_FIDUCIAL_ALIGN _T("{CD4D3D66-E6E3-4D7E-8E50-9AB4F19CC238}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_PAD_ALIGN_2D _T("{3791E3A4-888E-4F11-87F0-B3470F3FF9A6}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_2D_MATRIX _T("{745D1710-9B5A-475A-9691-5693C8EA8C50}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_IN_POCKET_TR _T("{6D126DBD-B160-491B-AC5C-BB32228B1317}")
#define _VISION_INSP_GUID_OTI_TR _T("{6103AE07-4FC1-45AD-9EEC-34F83FF98E0E}")
#define _VISION_INSP_GUID_PACKAGE_SIZE _T("{876FBD7A-20C5-4369-A968-E7D3C9587026}")
#define _VISION_INSP_GUID_BGA_BALL_2D _T("{A4890483-FE0A-44FF-8F42-0376FAC304FC}") //ShowDlg (~PadAlign2D)
#define _VISION_INSP_GUID_BGA_BALL_3D _T("{F6BEFACA-6756-4047-B70C-721CC0B90892}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_BGA_BALL_BRIDGE _T("{F8D4D7DF-62CB-4C0E-915D-4FC6276002FF}")
#define _VISION_INSP_GUID_BGA_BALL_PQ _T("{C75BB8B0-B0FE-4D28-9C92-02F123E8A3A5}") //ShowDlg (~Ball2D)
#define _VISION_INSP_GUID_LGA_LAND_2D _T("{340D60FE-7D22-47E6-B301-EC850AEEAFA7}") //ShowDlg (~PadAlign2D)
#define _VISION_INSP_GUID_LGA_LAND_3D _T("{91149B65-1DB8-4732-91B3-43315140C21D}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_PASSIVE_2D _T("{7041AC57-5894-424B-8AE6-50B776900913}") //ShowDlg (~PadAlign2D)
#define _VISION_INSP_GUID_PASSIVE_3D _T("{0340312E-ED76-43E7-B07C-C7C49DE54FF5}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_LID_2D _T("{0B382B9D-9AAA-4786-89FF-522BB44A88A6}") //ShowDlg (~EdgeAlign)//kircheis_InspLID
#define _VISION_INSP_GUID_LID_3D _T("{B6C978A3-4D6F-46B2-BB78-A5C71D469EE4}") //ShowDlg (~EdgeAlign)//kircheis_InspLID
#define _VISION_INSP_GUID_PATCH_2D _T("{515A1DE8-DB19-48D0-92BD-EAE37031845F}") //ShowDlg (~EdgeAlign)//kircheis_POI
#define _VISION_INSP_GUID_PATCH_3D _T("{B491E999-0D7B-4410-B600-9CC5308EDB67}") //ShowDlg (~EdgeAlign)//kircheis_POI
#define _VISION_INSP_GUID_DIE_2D _T("{54AA4583-B3CF-41AB-85BC-4C63A632FD80}") //ShowDlg (~EdgeAlign)//kircheis_InspLID
#define _VISION_INSP_GUID_KOZ_2D _T("{3D99B50E-B706-45DD-AF71-BA36F4B1E87D}") //ShowDlg (~EdgeAlign)//kircheis_KOZ2D
#define _VISION_INSP_GUID_OTHER_ALIGN _T("{11EB2777-D430-4066-826B-5CC2418153FA}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_GLOBAL_MASK_MAKER _T("{86A5BAED-42FC-4FBB-9EA2-D3A994516713}") //ShowDlg (~EdgeAlign)
#define _VISION_INSP_GUID_INTENSITYCHECKER_2D \
    _T("{F8FAFA58-532A-4785-96C7-B6C2EBB7D5FD}") //ShowDlg (~EdgeAlign)//SDY_IntensityChecker
#define _VISION_INSP_GUID_GLASS_SUBSTRATE_2D \
    _T("{6073B6B6-227D-4D32-8ADC-6C740E60538C}") //ShowDlg (~EdgeAlign)//kircheis_SideInsp
#define _VISION_INSP_GUID_SIDE_DETAIL_ALIGN _T("{E3433F05-34AB-4D10-A60C-A0478D661DDE}")
#define _VISION_INSP_GUID_GLASS_SUBSTRATE_2D \
    _T("{6073B6B6-227D-4D32-8ADC-6C740E60538C}") //ShowDlg (~EdgeAlign)//kircheis_SideInsp
#define _VISION_INSP_GUID_SIDE_INSPECTION_2D _T("{0829E433-A33A-4A36-AE0E-BAFC71AA8E16}")
#define _VISION_INSP_GUID_NGRV _T("{C48C7833-DF57-4C95-BA77-DAABA760426C}") // MED#6_NGRV_JHB

#define _VISION_INSP_NAME_2D_MATRIX _T("2D Matrix")
#define _VISION_INSP_NAME_BGA_BALL_2D _T("Ball 2D")
#define _VISION_INSP_NAME_BGA_BALL_3D _T("Ball 3D")
#define _VISION_INSP_NAME_BGA_BALL_BRIDGE _T("Ball Bridge")
#define _VISION_INSP_NAME_BGA_BALL_PQ _T("Ball Pixel Quality")
#define _VISION_INSP_NAME_DIE_2D _T("Die 2D")
#define _VISION_INSP_NAME_LGA_LAND_2D _T("Land 2D")
#define _VISION_INSP_NAME_LGA_LAND_3D _T("Land 3D")
#define _VISION_INSP_NAME_LID_2D _T("Lid 2D")
#define _VISION_INSP_NAME_LID_3D _T("Lid 3D")
#define _VISION_INSP_NAME_PATCH_2D _T("Patch 2D") //kircheis_POI
#define _VISION_INSP_NAME_PATCH_3D _T("Patch 3D") //kircheis_POI
#define _VISION_INSP_NAME_PACKAGE_SIZE _T("Package Size")
#define _VISION_INSP_NAME_PASSIVE_2D _T("Component 2D")
#define _VISION_INSP_NAME_PASSIVE_3D _T("Component 3D")
#define _VISION_INSP_NAME_KOZ_2D _T("KOZ 2D") //kircheis_KOZ
#define _VISION_INSP_NAME_INTENSITYCHECKER_2D _T("Intensity Checker 2D") //SDY_IntensityChecker
#define _VISION_INSP_NAME_GLASS_SUBSTRATE_2D _T("Glass Substrate 2D")
#define _VISION_INSP_NAME_SIDE_INSPECTION_2D _T("Side 2D")
#define _VISION_INSP_NAME_TR_INPOCKET _T("In-Pocket")
#define _VISION_INSP_NAME_TR_OTI _T("OTI")

//

enum OverlayType
{
    OverlayType_ShowAll,
    OverlayType_Reject,
    OverlayType_HideAll,
};

__VISION_COMMON_API__ CString CreateGUID();
__VISION_COMMON_API__ BOOL IsPredefineModule(CString strGUID);
__VISION_COMMON_API__ CRect GetContentSubArea(const CRect& rtBase, const long colBegin, const long colEnd,
    const long colCount, const long rowBegin, const long rowEnd, const long rowCount);

//{{//kircheis_SideDetailAlign DebugInfo Define (새로운 Debug Info 추가할 때마다 철자 틀리는거 고민말고 이렇게 만드는게 낫지 않을까?)
#define DEBUGINFO_SDA_SpecROI _T("Spec ROI")
#define DEBUGINFO_SDA_SubstrateRoughSearchROI_TB _T("Substrate Rough Search ROI(T/B)")
#define DEBUGINFO_SDA_SubstrateRoughEdgePoint_TB _T("Substrate Rough Edge Point(T/B)")
#define DEBUGINFO_SDA_SubstrateSearchROI_TB _T("Substrate Search ROI(T/B)")
#define DEBUGINFO_SDA_SubstrateEdgePoint_TB _T("Substrate Edge Point(T/B)")
#define DEBUGINFO_SDA_SubstrateSearchROI_LR _T("Substrate Search ROI(L/R)")
#define DEBUGINFO_SDA_SubstrateEdgePoint_LR _T("Substrate Edge Point(L/R)")
#define DEBUGINFO_SDA_GlassRoughSearchROI_TB _T("Glass Rough Search ROI(T/B)")
#define DEBUGINFO_SDA_GlassRoughEdgePoint_TB _T("Glass Rough Edge Point(T/B)")
#define DEBUGINFO_SDA_GlassSearchROI_TB _T("Glass Search ROI(T/B)")
#define DEBUGINFO_SDA_GlassEdgePoint_TB _T("Glass Edge Point(T/B)")
#define DEBUGINFO_SDA_GlassSearchROI_LR _T("Glass Search ROI(L/R)")
#define DEBUGINFO_SDA_GlassEdgePoint_LR _T("Glass Edge Point(L/R)")
#define DEBUGINFO_SDA_NormalUnitAlignResult _T("Normal Unit Align Result")
#define DEBUGINFO_SDA_TopSubstrateAlignResult _T("Top Substrate Align Result")
#define DEBUGINFO_SDA_BottomSubstrateAlignResult _T("Bottom Substrate Align Result")
#define DEBUGINFO_SDA_GlassAlignResult _T("Glass Align Result")
//}}
