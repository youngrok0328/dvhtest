#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonGridBase.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <array>

//HDR_5_________________________________ Forward declarations
class Para;
class Inspection;
class ImageLotView;
class ProcCommonDebugInfoDlg;
class VisionProcessing;
class DlgSearchRoiPara;

//HDR_6_________________________________ Header body
//
enum class CurInspectionType : int32_t
{
    e_carrierTapeAlign,
    e_pocketAlign,
    e_end,
};

class PropertyGrid : public ProcCommonGridBase
{
public:
    PropertyGrid(VisionProcessing& processor, Inspection& inspection, ImageLotView* imageLotView,
        ProcCommonDebugInfoDlg& procDebugInfo, Para& para);
    ~PropertyGrid();

    long getGridCount() const override;
    bool create(long gridIndex, const RECT& rect, HWND parentWnd) override;
    LPCTSTR getName(long gridIndex) const override;
    void active(long gridIndex) override;
    void deactivate(long gridIndex) override;
    bool notify(long gridIndex, WPARAM wparam, LPARAM lparam) override;
    void event_afterInspection(long gridIndex) override;
    void event_changedRoi(long gridIndex) override;
    void event_changedPane(long gridIndex) override;
    void event_changedRaw(long gridIndex) override;
    void event_changedImage(long gridIndex) override;

    void RedrawEdgeRoi(long gridIndex);

private:
    VisionProcessing* m_processor;
    Inspection& m_inspection;
    std::array<CXTPPropertyGrid*, (int32_t)CurInspectionType::e_end> m_propertyGrid { nullptr, };
    std::array<DlgSearchRoiPara*, (int32_t)CurInspectionType::e_end> m_edgeSearchRoiPara { nullptr, };
    ImageLotView* m_imageLotView;
    ProcCommonDebugInfoDlg& m_procDebugInfo;
    CString m_resultEmpty_Text;
    CString m_resultEmpty_BlobCount;
    Para* m_para;

    float m_empty_minBlobSize_px;

    void UpdateROI(CurInspectionType gridIndex);
    void SetCarrierTapeROI();
    void SetPocketROI();
    void ShowDisplayImage(long gridIndex);
    void ShowCarierTapeEdgeAlignImage();
    void ShowPocketEdgeAlignImage();
    void ClickedButtonCarrierTapeImageCombine();
    void ClickedButtonPocketImageCombine();

    void InitPropertyCarrierTape(long gridIndex);
    void InitPropertyPocket(long gridIndex);

    bool NotifyCommon(long gridIndex, CXTPPropertyGridItem* item);
    void NotifyCarrierTape(long gridIndex, CXTPPropertyGridItem* item);
    void NotifyPocket(long gridIndex, CXTPPropertyGridItem* item);
};
