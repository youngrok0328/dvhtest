#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonGridBase.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
namespace VisionEdgeAlign
{
class Para;
class Inspection;
class PropertyGrid_Impl;
} // namespace VisionEdgeAlign

class ImageLotView;
class ProcCommonDebugInfoDlg;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
namespace VisionEdgeAlign
{
class __VISION_EDGE_ALIGN_CLASS PropertyGrid : public ProcCommonGridBase
{
public:
    PropertyGrid(VisionProcessing& processor, Inspection& inspection, ImageLotView* imageLotView,
        ProcCommonDebugInfoDlg& procDebugInfo, VisionEdgeAlign::Para& para);
    virtual ~PropertyGrid();

    virtual long getGridCount() const;

    virtual bool create(long gridIndex, const RECT& rect, HWND parentWnd) override;
    virtual LPCTSTR getName(long gridIndex) const override;
    virtual void active(long gridIndex) override;
    virtual void deactivate(long gridIndex) override;
    virtual bool notify(long gridIndex, WPARAM wparam, LPARAM lparam) override;
    virtual void event_afterInspection(long gridIndex) override;
    virtual void event_changedRoi(long gridIndex) override;
    virtual void event_changedPane(long gridIndex) override;
    virtual void event_changedRaw(long gridIndex) override;
    virtual void event_changedImage(long gridIndex) override;

private:
    PropertyGrid_Impl* m_impl;
};
} // namespace VisionEdgeAlign
