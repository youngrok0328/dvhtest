#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "GeometryDefines.h"

//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionGeometryPara;
class CDlgVisionInspectionGeometry;
class ImageProcPara;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionImageLot;

//HDR_6_________________________________ Header body
//
class __DPI_GEOMETRY_MEASURED__CLASS__ VisionInspectionGeometry : public VisionInspection
{
public:
    VisionInspectionGeometry(
        LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspectionGeometry(void);

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
    // DebugInfo
    void DeleteSpec(long specIndex);
    void UpdateSpec();

    VisionInspectionGeometryPara* m_VisionPara;
    CDlgVisionInspectionGeometry* m_pVisionInspDlg;

    BOOL DoAlign();
    BOOL DoInspectoin_Align_Ref_Tar(SAlignInfoDataList& AlignInfoList, std::vector<CString> strvecinfoName,
        const std::vector<Ipvm::Rect32r>& vecrtSearchROI_BCU, std::vector<std::vector<long>> vecInfoParameter_0,
        const Ipvm::Image8u& image);
    BOOL DoInspectoin_Final(VisionInspectionSpec* pSpec, VisionInspectionResult* pResult, long nInspectionType,
        float fReferenceSpec, long nDistanceResult, long nCircleResult, std::vector<float>& vecfError,
        std::vector<Ipvm::Rect32s>& vecrtROI);

    BOOL GetAlignInfo(std::vector<CString>& vecstrUseAlignName);

    BOOL GetAlignData_Line_And_Point(SAlignInfoDataList& AlignInfoList,
        std::vector<std::vector<long>> vecInfoParameter_0, std::vector<Ipvm::Rect32s> vecrtSearchROI, long nSpecID,
        const Ipvm::Image8u& image);

    BOOL GetEdgeDetect(long nSpecID, const Ipvm::Image8u& image, std::vector<long> vecnParamData,
        Ipvm::Rect32s i_rtSearchROI, std::vector<Ipvm::Point32r2>& vecfptEdgeData);
    BOOL GetEdgeDetect_All_Line(const Ipvm::Image8u& image, std::vector<long> vecnParamData,
        Ipvm::Rect32s i_rtSearchROI, const Ipvm::LineEq32r& lineData, std::vector<Ipvm::Point32r2>& vecfptEdgeData);
    BOOL RoughAlign_LineAlign(std::vector<Ipvm::Point32r2>& vecfptEdgeData);

    // Line 성분의 Center Point를 계산한다.
    BOOL Get_LineCenter_Point(SAlignInfoDataList& AlignInfoData, Ipvm::Point32r2& fptBeginPoint,
        Ipvm::Point32r2& fptCenterPoint, Ipvm::Point32r2& fptEndPoint);

    BOOL GetEdgeDetect_Ellips(long nSpecID, const Ipvm::Image8u& image, std::vector<long> vecnParamData,
        Ipvm::Rect32s i_rtSearchROI, std::vector<Ipvm::Point32r2>& vecfptEdgeData);
    BOOL GetEdgeDetect_Round(long nSpecID, const Ipvm::Image8u& image, std::vector<long> vecnParamData,
        Ipvm::Rect32s i_rtSearchROI, std::vector<Ipvm::Point32r2>& vecfptEdgeData);
    BOOL NoiseFilter_CircleFitting(SAlignInfoDataList& AlignInfoList, std::vector<long> vecnParamData);

    void UpdateSweepAngleTable(long nSectionNum, std::vector<float>& vecfCos, std::vector<float>& vecfSin);
    void SetDebugInfo(const bool detailSetupMode);

    LPCTSTR GetFinalCheckError_Reference() const;
    LPCTSTR GetFinalCheckError_Target() const;

    CPI_EdgeDetect* m_pEdgeDetect;

    float m_fPixelToUm;

    Ipvm::Rect32r m_frtOtherDetectROI;
    Ipvm::Rect32r* m_pfrtRoundDetectROI;

    std::vector<float> m_vecfCosForBallCenter;
    std::vector<float> m_vecfSinForBallCenter;

    std::vector<std::vector<Ipvm::Point32r2>> m_vecfptlineData_Start;
    std::vector<std::vector<Ipvm::Point32r2>> m_vecfptlineData_End;

    VisionAlignResult* m_sEdgeAlignResult;

    // Align Result...
    BOOL GetAlignInfoData();
    BOOL GetAlignInfoData_BodyAlign(VisionAlignResult sEdgeAlignResult);
    BOOL GetAlignInfoData_OtherAlign(VisionAlignResult sEdgeAlignResult, Ipvm::Rect32r frtOtherDetectROI);
    BOOL GetAlignInfoData_RoundAlign(VisionAlignResult sEdgeAlignResult, Ipvm::Rect32r* pfrtRoundDetectROI);
    BOOL GetAlignInfoData_UserAlign();

    std::vector<SAlignInfoDataList> m_vecAlignInfoList;
    SAlignInfoDataList m_AlignInfoData_Ref;
    SAlignInfoDataList m_AlignInfoData_Tar;

    std::vector<SAlignInfoDataList> m_vecAlignInfoData_Ref;
    std::vector<SAlignInfoDataList> m_vecAlignInfoData_Tar;

    float m_fCalcTime;

private:
    CString m_resultFinalCheckError_reference;
    CString m_resultFinalCheckError_target;
    void ResizeResultAndDebugInfo();

    BOOL DoInspectoin_Final_Distance(
        long nDistanceResult, std::vector<float>& vecfError, std::vector<Ipvm::Rect32s>& vecrtROI);
    BOOL DoInspectoin_Final_Circle(
        long nCircleResult, std::vector<float>& vecfError, std::vector<Ipvm::Rect32s>& vecrtROI);
    BOOL DoInspectoin_Final_Angle(std::vector<float>& vecfError, std::vector<Ipvm::Rect32s>& vecrtROI);

private:
    BOOL SetPassResultGeometryForSide();
};