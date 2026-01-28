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
#include <Ipvm/Base/Point64r2.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class VisionInspectionLid3DPara;
class CDlgVisionInspectionLid3D;
class CPI_EdgeDetect;
class VisionImageLot;
class Result;
struct SLid3DGroupInfo;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_LID_3D_CLASS__ VisionInspectionLid3D : public VisionInspection
{
public:
    VisionInspectionLid3D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionLid3D(void);

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
    VisionInspectionLid3DPara* m_VisionPara;
    CDlgVisionInspectionLid3D* m_pVisionInspDlg;

    float m_fCalcTime;
    Result* m_result;

public:
    VisionAlignResult* m_sEdgeAlignResult;

    std::vector<VisionAlignResult> m_vecsLidAlignResult;
    std::vector<SDieLidPatchInfoDB> m_vecsLidInfoDB;

    Ipvm::LineEq32r m_linePackageCenterHor;
    Ipvm::LineEq32r m_linePackageCenterVer;
    std::vector<Ipvm::LineEq32r> m_veclineLidCenterHor;
    std::vector<Ipvm::LineEq32r> m_veclineLidCenterVer;

    CPI_EdgeDetect* m_pEdgeDetect;

    /////////////////////////////////////////////////
    BOOL GetLidInfoDB(); //DMS에서 Data를 가져와서 m_vecsLidInfoDB에 넣는다.
    BOOL CalcAlignedCenterLine(VisionAlignResult i_AlignResult, Ipvm::LineEq32r& o_HorLine, Ipvm::LineEq32r& o_VerLine);

    BOOL DoAlignLid(const bool detailSetupMode);
    BOOL MakeEdgeSearchROI(SDieLidPatchInfoDB lidInfoDB, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI);
    BOOL GetLidEdgePoint(
        Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge);
    BOOL CalcLidAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge, std::vector<Ipvm::Point32r2> vecfptTopEdge,
        std::vector<Ipvm::Point32r2> vecfptRightEdge, std::vector<Ipvm::Point32r2> vecfptBottomEdge,
        VisionAlignResult& o_lidAlignResult);

    void MakeBasicROI(VisionAlignResult alignResult, float fOffsetX_um, float fOffsetY_um, float nSize_um,
        std::vector<Ipvm::Rect32s>& o_vecrtBasicROI);
    void MakeLidROI(long nLidID, std::vector<Ipvm::Rect32s>& o_vecrtLidROI);

    void Get3DPosData(std::vector<Ipvm::Rect32s> vecrtROI,
        std::vector<Ipvm::Point32r3>& o_vecfpt3Pos); //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
    void CalcLidOrSubstrateZ(Ipvm::Rect32s rtROI,
        Ipvm::Point32r3& o_fpt3Pos); //출력 변수 구조체 xyz 중 xy는 Pxl Rect Center, z는 실좌표를 쓴다.
    BOOL Get3DPlane(std::vector<Ipvm::Point32r3> vecfpt3Pos, Ipvm::PlaneEq64r& o_plane);

    BOOL DoInspUnitLidTiltX(Ipvm::PlaneEq64r planeSub, std::vector<Ipvm::PlaneEq64r> vecPlaneLid);
    BOOL DoInspUnitLidTiltY(Ipvm::PlaneEq64r planeSub, std::vector<Ipvm::PlaneEq64r> vecPlaneLid);
    double GetDeltaTheta(const Ipvm::Point64r2& v1,
        const Ipvm::Point64r2&
            v2); //PlaneEq를 쓴다. 평면의 전체 Angle은 못 구하고, X, Y, Z 하나만 뽑을 수 있음(X : b,c  Y:a,c  Z:b,c 를 Point에 넣을 것)
    BOOL DoInspUnitLidHeight(Ipvm::PlaneEq64r planeSub, std::vector<std::vector<Ipvm::Rect32s>> vec2LidROI,
        std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3Lid);
    BOOL DoInspLidWarpageWithUnitWarpage(std::vector<Ipvm::PlaneEq64r> vecPlaneLid,
        std::vector<std::vector<Ipvm::Rect32s>> vec2LidROI, std::vector<std::vector<Ipvm::Point32r3>> vec2fpt3Lid,
        const bool detailSetupMode);

    Ipvm::Rect32r GetBodyRect();
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
};
