#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/PlaneEq64r.h>
#include <Ipvm/Base/Point32r3.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class VisionInspectionPatch3DPara;
class CDlgVisionInspectionPatch3D;
class CPI_EdgeDetect;
class VisionImageLot;
class Result;
struct SInterposerRefROI_Info;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_PATCH_3D_CLASS__ VisionInspectionPatch3D : public VisionInspection
{
public:
    VisionInspectionPatch3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionPatch3D(void);

public: // VisionProcessing virtual functions
    virtual BOOL DoInspection(const bool detailSetupMode,
        const enSideVisionModule i_ModuleStatus = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual BOOL OnInspection() override;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;
    virtual void ResetSpecAndPara() override;
    virtual void ResetResult() override;
    virtual void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode) override;
    virtual void AppendTextResult(CString& textResult) override;
    virtual long ShowDlg(const ProcessingDlgInfo& procDlgInfo) override;
    virtual void CloseDlg() override;
    virtual std::vector<CString> ExportRecipeToText() override; //kircheis_TxtRecipe

public:
    VisionInspectionPatch3DPara* m_VisionPara;
    CDlgVisionInspectionPatch3D* m_pVisionInspDlg;

    float m_fCalcTime;
    Result* m_result;

public:
    VisionAlignResult* m_sEdgeAlignResult;

    std::vector<VisionAlignResult> m_vecsPatchAlignResult;
    std::vector<SDieLidPatchInfoDB> m_vecsPatchInfoDB;

    Ipvm::LineEq32r m_linePackageCenterHor;
    Ipvm::LineEq32r m_linePackageCenterVer;
    std::vector<Ipvm::LineEq32r> m_veclinePatchCenterHor;
    std::vector<Ipvm::LineEq32r> m_veclinePatchCenterVer;

    CPI_EdgeDetect* m_pEdgeDetect;

    /////////////////////////////////////////////////
    BOOL GetPatchInfoDB(); //DMS에서 Data를 가져와서 m_vecsPatchInfoDB에 넣는다.
    BOOL CalcAlignedCenterLine(VisionAlignResult i_AlignResult, Ipvm::LineEq32r& o_HorLine, Ipvm::LineEq32r& o_VerLine);

    BOOL DoAlignPatch(const bool detailSetupMode);
    BOOL MakeEdgeSearchROI(SDieLidPatchInfoDB patchInfoDB, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI);
    BOOL GetPatchEdgePoint(
        Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge);
    BOOL CalcPatchAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge, std::vector<Ipvm::Point32r2> vecfptTopEdge,
        std::vector<Ipvm::Point32r2> vecfptRightEdge, std::vector<Ipvm::Point32r2> vecfptBottomEdge,
        VisionAlignResult& o_patchAlignResult);

    void MakeInterposerROI(
        std::vector<SInterposerRefROI_Info> i_vecsInterposerROI_Pxl, std::vector<Ipvm::Rect32s>& o_vecrtInterposerROI);
    void MakePatchROI(VisionAlignResult alignResult, float fOffsetX_um, float fOffsetY_um, float nSize_um,
        std::vector<Ipvm::Rect32s>& o_vecrtPatchROI);

    void Get3DPosData(std::vector<Ipvm::Rect32s> vecrtROI,
        std::vector<Ipvm::Point32r3>& o_vecfpt3Pos); //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
    void CalcPatchOrInterposerZ(Ipvm::Rect32s rtROI,
        Ipvm::Point32r3& o_fpt3Pos); //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
    BOOL Get3DPlane(std::vector<Ipvm::Point32r3> vecfpt3Pos, Ipvm::PlaneEq64r& o_plane);

    //BOOL DoInspUnitPatchTiltX(Ipvm::PlaneEq64r planeSub, std::vector<Ipvm::PlaneEq64r> vecPlanePatch);
    //BOOL DoInspUnitPatchTiltY(Ipvm::PlaneEq64r planeSub, std::vector<Ipvm::PlaneEq64r> vecPlanePatch);
    //	double GetDeltaTheta(const Ipvm::Point64r2 &v1, const Ipvm::Point64r2 &v2);//PlaneEq를 쓴다. 평면의 전체 Angle은 못 구하고, X, Y, Z 하나만 뽑을 수 있음(X : b,c  Y:a,c  Z:b,c 를 Point에 넣을 것)
    BOOL DoInspPatchHeight(Ipvm::PlaneEq64r planeInterposer, std::vector<std::vector<Ipvm::Rect32s>> vec2PatchROI,
        std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3Patch);

    Ipvm::Rect32r GetBodyRect();
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
};
