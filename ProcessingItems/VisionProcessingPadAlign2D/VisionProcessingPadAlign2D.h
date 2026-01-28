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
class VisionProcessingPadAlign2DPara;
class DlgVisionProcessingPadAlign2D;
class ImageProcPara;
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
class __VISION_PAD_ALIGN_2D_CLASS VisionProcessingPadAlign2D : public VisionProcessing
{
public:
    VisionProcessingPadAlign2D(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionProcessingPadAlign2D(void);

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
    VisionProcessingPadAlign2DPara* m_VisionPara;
    DlgVisionProcessingPadAlign2D* m_pVisionInspDlg;

    CPI_EdgeDetect* m_pEdgeDetect;
    CPI_Blob* m_pBlob;
    Ipvm::BlobInfo* m_psBlobInfo;
    float m_fCalcTime;
    Result* m_result;

    bool getBlobThresholdImage(LPCTSTR padName, Ipvm::Image8u& o_image, Ipvm::Rect32s& o_searchRoi);

private:
    bool align(const bool detailSetupMode);
    bool collect_debugResult(const bool detailSetupMode);

    void make_pad_imageCoordinates(const Ipvm::Point32r2& i_bodyCenter, const Package::PadCollection& i_padInfo);
    void make_pad_imageCoordinate(
        const Ipvm::Point32r2& i_bodyCenter, const Package::Pad& i_padSpec, Result_Pad& o_padPos);

    bool getSpec_fiducialMark2(const std::vector<Result_Pad>& i_padInfo, Result_Pad& o_mark1, Result_Pad& o_mark2);
    bool find_pad(const Ipvm::Point32r2& i_bodyCenter, bool fiducialPad);
    bool find_pad(const long padIndex, const Ipvm::Point32r2& i_bodyCenter, Result_Pad& io_mark);
    bool find_pad_circle(const long padIndex, AlignPara& para, Result_Pad& io_mark);
    bool find_pad_mask(const long padIndex, AlignPara& para, Result_Pad& io_mark);
    bool find_pad_pinmark(
        const long padIndex, const Ipvm::Point32r2& i_bodyCenter, AlignPara& para, Result_Pad& io_mark);
    bool find_pad_polygon_edge_detect(
        const long padIndex, AlignPara& para, const Ipvm::Polygon32r& i_polygon, Result_Pad& io_mark);

    bool calculation_offset_degree(const Ipvm::Point32r2& i_bodyCenter, const Result_Pad& i_mark1,
        const Result_Pad& i_mark2, Ipvm::Point32r2& o_offset, float& o_degree);
    bool apply_other_pad_from_fiducial(const Ipvm::Point32r2& i_bodyCenter, const Ipvm::Point32r2& i_offset,
        const float i_degree, std::vector<Result_Pad>& io_padInfo);
    bool get_edge_detection_para(const AlignPara& para, Ipvm::EdgeDetectionPara& o_edge_para);

    bool get_insp_image_and_searchROI(
        const Result_Pad& i_targetPAD, float searhRange_um, Ipvm::Image8u& o_inspImage, Ipvm::Rect32s& o_searchROI);

    bool get_largeBlob(const Ipvm::Image8u& i_image, long threshold, const Ipvm::Rect32s& searchROI,
        Ipvm::BlobInfo& o_largestBlob, SurfaceLayerMask* o_mask = nullptr);
    bool boundaryPoints_makes_simple(
        const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, Ipvm::Polygon32r& o_boundary);
    long getPolygon_Index(const Ipvm::Polygon32r& i_boundary, long index);
    void getContinuoesPoints(const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, long baseIndex,
        long& startIndex, long& endIndex);
    void getContinuoesPoints_end(const Ipvm::Polygon32r& i_boundary, float pointDistanceForLine, long startIndex,
        long baseEndIndex, long& endIndex, long limitEndIndex = -1);
    bool isValidLine(const Ipvm::Polygon32r& i_boundary, float i_pointDistanceForLine, long i_startIndex,
        long i_endIndex, const Ipvm::LineEq32r& i_line);

    Ipvm::Point32r2 get_imageRoi_to_gerberRoi(
        const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const Ipvm::Point32r2& imagePos);
    Ipvm::Polygon32r get_imageRoi_to_gerberRoi(
        const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const Ipvm::Polygon32r& imagePos);
    Ipvm::Point32r2 get_gerberRoi_to_imageRoi(
        const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const Ipvm::Point32r2& gerberPos);
    FPI_RECT get_gerberRoi_to_imageRoi(
        const Ipvm::Point32r2& pixelperMM, const Ipvm::Point32r2& body_center_px, const FPI_RECT& gerberPos);
    Ipvm::Point32r2 getCenter_object2(const Ipvm::Point32r2& obj1, const Ipvm::Point32r2& obj2);
    FPI_RECT getRect32f_FPI_Rect(const Ipvm::Rect32r& object);
    FPI_RECT getEllipse_FPI_Rect(const Ipvm::EllipseEq32r& object);

    SurfaceLayerMask* getSurfaceLayerMask();
    bool addEdgeDirectionPoint(const Ipvm::Point32s2& outPoint, const Ipvm::Point32s2& inPoint, const AlignPara& para,
        std::vector<Ipvm::Point32s2>& o_startList, std::vector<Ipvm::Point32s2>& o_endList);
    bool getBodyCenter(Ipvm::Point32r2& o_center); //2019.04.08 kk

private:
    Ipvm::EdgeDetection* m_edgeDetection;
    CString m_errorLogText;
    void checkJobParameter();
};
