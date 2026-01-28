#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
class Inspection;
} // namespace VisionEdgeAlign

class ImageLotView;
class ProcCommonDebugInfoDlg;
class VisionProcessing;
class DlgSearchRoiPara;

//HDR_6_________________________________ Header body
//
enum enCurInspectionType
{
    Type_EdgeAlign,
    Type_EmptyChecker,
};

namespace VisionEdgeAlign
{
class PropertyGrid_Impl
{
public:
    PropertyGrid_Impl(VisionProcessing& processor, Inspection& inspection, ImageLotView* imageLotView,
        ProcCommonDebugInfoDlg& procDebugInfo, VisionEdgeAlign::Para& para);
    ~PropertyGrid_Impl();

    void redrawEdgeRoi();
    long getGridCount() const;

    bool create(long gridIndex, RECT rect, HWND parentWnd);
    bool notify(long gridIndex, WPARAM wparam, LPARAM lparam);
    LPCTSTR getName(long gridIndex) const;
    void active(long gridIndex);
    void deactivate(long gridIndex);
    void event_afterInspection(long gridIndex);
    void event_changedRoi(long gridIndex);
    void event_changedPane(long gridIndex);
    void event_changedRaw(long gridIndex);
    void event_changedImage(long gridIndex);

private:
    VisionProcessing* m_processor;
    Inspection& m_inspection;
    CXTPPropertyGrid* m_propertyGrid[2];
    long m_propertyIndexs[2];
    ImageLotView* m_imageLotView;
    ProcCommonDebugInfoDlg& m_procDebugInfo;
    CString m_resultEmpty_Text;
    CString m_resultEmpty_BlobCount;
    Para* m_para;
    DlgSearchRoiPara* m_edgeSearchRoiPara;

    float m_empty_minBlobSize_px;

    void UpdateROI_edgeAlign();
    void GetROI_edgeAlign();
    void GetROI_empty2D();
    void SetROI_edgeAlign();
    void SetROI_empty2D();
    void ShowDisplayImage(long gridIndex);
    void ShowEdgeAlignImage();
    void ShowEmpty2D(bool thresholdDisplay);
    void ClickedButtonImageCombine();

    void active_edgeAlign(long gridIndex);
    void active_empty2D(long gridIndex);

    bool notify_edgeAlign(long gridIndex, WPARAM wparam, LPARAM lparam);
    bool notify_empty2D(long gridIndex, WPARAM wparam, LPARAM lparam);

    void ClickedButtonSetAutoFocusImage(); // JHB_AutoFocusImageSet
};
} // namespace VisionEdgeAlign
