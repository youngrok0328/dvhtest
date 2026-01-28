#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Pad.h"
#include "../../InformationModule/dPI_DataBase/PadCollection.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class SurfaceLayerMask;
class VisionProcessingFiducialAlignPara;
class DlgVisionProcessingFiducialAlign;
class CImageProcPara;
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionImageLot;
class PadCollection;
class Pad;
class AlignPara;
class Result;
class Result_Pad;
struct FPI_RECT;

//HDR_6_________________________________ Header body
//
class __VISION_PAD_ALIGN_3D_CLASS VisionProcessingFiducialAlign : public VisionProcessing
{
public:
    VisionProcessingFiducialAlign(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingFiducialAlign(void);

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
    VisionProcessingFiducialAlignPara* m_VisionPara;
    DlgVisionProcessingFiducialAlign* m_pVisionInspDlg;

    bool teachObject(
        LPCTSTR pad_name, Ipvm::Polygon32r& o_boundary_px, Ipvm::Polygon32r& o_boundary_um, float& o_objectIntensity);

    CPI_EdgeDetect* m_pEdgeDetect;
    CPI_Blob* m_pBlob;
    Ipvm::Rect32s m_rtPaneROI;
    Ipvm::BlobInfo* m_psBlobInfo;
    float m_fCalcTime;
    Result* m_result;

private:
    bool align(const bool detailSetupMode);
    bool collect_debugResult(const bool detailSetupMode);
    bool collect_debugResult_mask(const bool detailSetupMode);

    bool getBodyCenter(Ipvm::Point32r2& o_center);
    bool getBodyAngle(float& o_angle_rad);
    void make_pad_imageCoordinates(
        const Ipvm::Point32r2& i_bodyCenter, const float i_bodyAngle, const Package::PadCollection& i_padInfo);
    void make_pad_imageCoordinate(const Ipvm::Point32r2& i_bodyCenter, const float i_bodyAngle,
        const Package::Pad& i_padSpec, Result_Pad& o_padPos);

    bool getSpec_fiducialMark2(const std::vector<Result_Pad>& i_padInfo, Result_Pad& o_mark1, Result_Pad& o_mark2);
    bool find_pad(const Ipvm::Point32r2& i_bodyCenter, bool fiducialPad);
    bool find_pad(const long padIndex, const Ipvm::Point32r2& i_bodyCenter, Result_Pad& io_mark);
    bool find_pad_circle(const long padIndex, AlignPara& para, Result_Pad& io_mark);
    bool find_pad_mask(const long padIndex, AlignPara& para, Result_Pad& io_mark);
    bool find_pad_pinmark(
        const long padIndex, const Ipvm::Point32r2& i_bodyCenter, AlignPara& para, Result_Pad& io_mark);
    bool find_pad_polygon_edge_detect(
        const long padIndex, AlignPara& para, const Ipvm::Polygon32r& i_polygon, Result_Pad& io_mark);

    bool calculation_offset_degree(const Ipvm::Point32r2& i_bodyCenter, const float i_bodyAngle,
        const Result_Pad& i_mark1, const Result_Pad& i_mark2, Ipvm::Point32r2& o_offset, float& o_degree);
    bool apply_other_pad_from_fiducial(const Ipvm::Point32r2& i_bodyCenter, const Ipvm::Point32r2& i_offset,
        const float i_degree, std::vector<Result_Pad>& io_padInfo);
    bool get_edge_detection_para(const AlignPara& para, Ipvm::EdgeDetectionPara& o_edge_para);

    bool get_insp_image_and_searchROI(const Ipvm::Rect32s& i_targetPAD, Ipvm::Point32r2 i_SearchOffset,
        Ipvm::Image8u& o_inspImage, Ipvm::Rect32s& o_searchROI);

    bool get_largeBlobBoundary(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& searchROI,
        std::vector<Ipvm::Point32s2>& o_boundaryPoints, float& o_averageBlobIntenisty);
    bool boundaryPoints_makes_simple(
        const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, Ipvm::Polygon32r& o_boundary);
    long getPolygon_Index(const Ipvm::Polygon32r& i_boundary, long index);
    void getContinuoesPoints(const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, long baseIndex,
        long& startIndex, long& endIndex);
    void getContinuoesPoints_end(const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, long startIndex,
        long baseEndIndex, long& endIndex, long limitEndIndex = -1);
    bool isValidLine(const Ipvm::Polygon32r& i_boundary, float i_pointDistanceForLine, long i_startIndex,
        long i_endIndex, const Ipvm::LineEq32r& i_line);

    Ipvm::Point32r2 get_imageRoi_to_gerberRoi(const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px,
        const float body_angle, const Ipvm::Point32r2& imagePos);
    Ipvm::Polygon32r get_imageRoi_to_gerberRoi(const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px,
        const float body_angle, const Ipvm::Polygon32r& imagePos);
    Ipvm::Point32r2 get_gerberRoi_to_imageRoi(const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px,
        const float body_angle, const Ipvm::Point32r2& gerberPos);
    FPI_RECT get_gerberRoi_to_imageRoi(const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px,
        const float body_angle, const FPI_RECT& gerberPos);
    Ipvm::Point32r2 getCenter_object2(const Ipvm::Point32r2& obj1, const Ipvm::Point32r2& obj2);
    FPI_RECT getRect32f_FPI_Rect(const Ipvm::Rect32r& object);
    FPI_RECT getEllipse_FPI_Rect(const Ipvm::EllipseEq32r& object);

    SurfaceLayerMask* getSurfaceLayerMask();
    bool addEdgeDirectionPoint(const Ipvm::Point32s2& outPoint, const Ipvm::Point32s2& inPoint, const AlignPara& para,
        std::vector<Ipvm::Point32s2>& o_startList, std::vector<Ipvm::Point32s2>& o_endList);

    bool CalcGabOfObjectCenter(const Ipvm::Image8u& i_image, const Ipvm::Rect32s& searchROI,
        Ipvm::Point32r2& o_ptGabSize, Ipvm::Rect32s& o_rtBlobROI, Ipvm::Image8u& o_binaryImage);
    bool MakeBodyROI(FPI_RECT& o_rtPackageROI);

private:
    CString m_errorLogText;
    void checkJobParameter();
};
