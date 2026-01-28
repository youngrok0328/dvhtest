#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionAlignResult.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Image32s.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/LineSeg32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class VisionInspectionKOZ2DPara;
class CDlgVisionInspectionKOZ2D;
class CPI_EdgeDetect;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
struct SNotchInfo
{
    Ipvm::EllipseEq32r ellipseNotchSpec_pxl;
    long nNotchPositionDir; //어느 방향의 Package Edge Line에 있는지. UP, DOWN, LEFT, RIGHT
};

class __VISION_INSP_KOZ_2D_CLASS__ VisionInspectionKOZ2D : public VisionInspection
{
public:
    VisionInspectionKOZ2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionKOZ2D(void);

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
    VisionInspectionKOZ2DPara* m_VisionPara;
    CDlgVisionInspectionKOZ2D* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    //{{//Spec Data 및 이전에 계산된 정보들
    VisionAlignResult* m_sEdgeAlignResult;

    std::vector<SNotchInfo>
        m_vecNotchSpecInfo; //Spec을 가져와서 여기다 넣을 때 중심 좌표를 Align Line으로 Projection 시켜 재구성해야 한다.
    std::vector<float> m_vecfKOZSpecWidth_um;
    std::vector<float> m_vecfKOZSpecWidth_pxl;
    std::vector<BOOL> m_vecbKOZSpecWidth_Valid; //KOZ Width가 0이하면 false
    //}}

    //{{//Align등의 연산 결과들
    std::vector<Ipvm::EllipseEq32r> m_vecEllipseNotchAlignResult; //
    std::vector<long> m_vecnNotchDirection;
    VisionAlignResult m_kozAlignResult;
    std::vector<Ipvm::EllipseEq32r> m_vecEllipseKOZNotchAlignResult; //
    std::vector<Ipvm::LineEq32r> m_veclineKOZ;
    std::vector<Ipvm::LineEq32r> m_veclinePackage;
    Ipvm::Image8u
        m_imagePackageAlignMask; //이 모듈에서 만드는 Mask를 제대로 만들기 위해 Package Align 결과와 And 시켜야 한다. 이를 위한 거다.
    Ipvm::Image8u m_imageChippingInspMask;
    Ipvm::Image8u m_imageChippingBackground;
    //}}

    //{{//Setup UI에서 필요한 변수들
    BOOL m_bIsNeedToMakeChippingMask;
    BOOL m_bIsNeedToMakeChippingBackground;
    //}}

    CPI_EdgeDetect* m_pEdgeDetect;

    /////////////////<<  Notch Align  >>////////////////////////////////
    BOOL GetNotchInfo(
        const bool detailSetupMode); //Notch가 없으면 False를 Return. Notch 정보를 가져와서 m_vecNotchSpecInfo를 채운다.
    BOOL GetPackageOutlineLineEq(std::vector<Ipvm::LineEq32r>& o_vecLinePackage);
    long GetNotchDirectionAndProjectionEdgeLine(std::vector<Ipvm::LineEq32r> i_vecLinePackage,
        Ipvm::EllipseEq32r&
            io_ellipseNotch_pxl); //Notch가 있는 Line의 Direction을 리턴함과 동시에 중심좌표를 Edge Line으로 이동시킨다
    BOOL DoAlignNotch(const bool detailSetupMode);
    BOOL AlignOneNotch(Ipvm::Image8u i_image, Ipvm::EllipseEq32r i_ellipseNotch_pxl, long i_nNotchDir, long i_nEdgeDir,
        float i_fEdgeThrehsold, float i_fSearchLength, float i_fIgnoreLength, bool bFirstEdge,
        std::vector<Ipvm::Point32r2>& o_vecfptEdge, std::vector<Ipvm::LineSeg32r>& o_vecLineNotchSweepLine);
    void GetNotchEdgeSearchLine(
        std::vector<Ipvm::Point32r2>& o_vecfptLineStart, std::vector<Ipvm::Point32r2>& o_vecfptLineEnd);

    /////////////////<<  KOZ Align  >>////////////////////////////////
    BOOL DoAlign(const bool detailSetupMode);
    BOOL GetKOZWidthData(); //PackageSpec에서 Data를 가져와서 이 클래스의 Data에 값을 쓴다.
    BOOL DoAlignKOZ(const bool detailSetupMode);
    BOOL MakeEdgeSearchROI(std::vector<Ipvm::Rect32s>& o_vecrtSearchROI,
        PI_RECT& prtKOZ); //원래 Search ROI만 만들려 했으나 덤으로 Debug Info용 KOZ Spec도 계산한다.
    BOOL AlignNotchOnKOZ(Ipvm::Image8u i_image, const bool detailSetupMode); //Notch 영역의  KOZ 경계를 Align하는 함수
    BOOL GetKOZEdgePoint(
        Ipvm::Image8u image, long nDir, Ipvm::Rect32s rtSearchROI, std::vector<Ipvm::Point32r2>& o_vecfptEdge);
    BOOL IsPtInNotchArea(
        Ipvm::Point32s2 ptStart); //Edge 탐색을 위한 Start Point가 Notch 영역인지 확인해서  Notch 영역이면 return true
    BOOL CalcKOZAlignResult(std::vector<Ipvm::Point32r2> vecfptLeftEdge, std::vector<Ipvm::Point32r2> vecfptTopEdge,
        std::vector<Ipvm::Point32r2> vecfptRightEdge, std::vector<Ipvm::Point32r2> vecfptBottomEdge,
        Ipvm::LineEq32r& o_alignLeftLine, Ipvm::LineEq32r& o_alignTopLine, Ipvm::LineEq32r& o_alignRightLine,
        Ipvm::LineEq32r& o_alignBottomLine, VisionAlignResult& o_dieAlignResult);

    /////////////////<<  Inspection  >>////////////////////////////////
    BOOL DoInspKOZWidth(); //얜 Align 결과로 계산
    BOOL DoInspKOZChipping(const bool detailSetupMode); //얜 내부에서 다 계산해야함.
    void MakeKOZChippingAreaMask(); //모든 Align 결과를 참조하여 Chipping 영역의 Mask를 만드는 함수
    BOOL MakeIgnoreMask(long nDialationCount, Ipvm::Image8u& o_imageMask); //false가 리턴되는건 Ignore 할게 없다는 의미
    void MakeKOZChippingBackgroundImage(Ipvm::Image8u combinedImage);
    void MakeKOZChippingThresholdImage(const Ipvm::Image8u i_combinedImage, const Ipvm::Image8u i_backgroundImage,
        const Ipvm::Image8u i_maskImage, Ipvm::Image8u& o_darkThreshImage, Ipvm::Image8u& o_brightThreshImage);
    void CheckDirectionAndExcludeSpecOutBlob(const std::vector<Ipvm::BlobInfo> i_vecObjBlobCandidate,
        long i_nMinWidth_um, long i_nMinLength_um, long& io_nBlobNum, std::vector<Ipvm::BlobInfo>& o_vecFilterdObjBlob,
        std::vector<long>& o_vecnDirectionOfBlob);
    float GetChippingDistanceOfBlob_pxl(const Ipvm::BlobInfo objBlobInfo, const long nDirection);
    void CalcChippingDistanceValue_pxl(const Ipvm::BlobInfo i_objBlobInfo, const Ipvm::Image32s i_blobLabel,
        const long i_nDirection, float& o_fChippingTotalLength, float& o_fKOZWidth);
    void GetChippingBlobInnerPoint(const Ipvm::BlobInfo i_objBlobInfo, const Ipvm::Image32s i_blobLabel,
        const long i_nDirection, Ipvm::Point32r2& o_point);

    /////////////////<<  보조 알고리즘  >>////////////////////////////////
    Ipvm::Rect32r GetBodyRect();
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);

    float GetCurrentEdgeThresholdValueKOZ();

    std::vector<float> m_vecfCosForNotchAlign;
    std::vector<float> m_vecfSinForNotchAlign;
    void UpdateSweepAngleTable(long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin);
};
