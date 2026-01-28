#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonGridBase.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CVisionProcessingAlign3DPara;
class ImageLotView;
class ProcCommonDebugInfoDlg;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
class Property_Stitch : public ProcCommonGridBase
{
public:
    Property_Stitch(VisionProcessing& proccsor, ImageLotView* imageLotView, ProcCommonDebugInfoDlg& procDebugInfo,
        CVisionProcessingAlign3DPara& para);
    virtual ~Property_Stitch();

    virtual bool create(long gridIndex, const RECT& rect, HWND parentWnd) override;
    virtual LPCTSTR getName(long gridIndex) const override;
    virtual void active(long gridIndex) override;
    virtual void deactivate(long gridIndex) override;
    virtual bool notify(long gridIndex, WPARAM wparam, LPARAM lparam) override;
    virtual void event_afterInspection(long gridIndex) override;
    virtual void event_changedRoi(long gridIndex) override;
    virtual void event_changedPane(long gridIndex) override;
    virtual void event_changedRaw(long gridIndex) override;

private:
    VisionProcessing& m_proccsor;
    CVisionProcessingAlign3DPara& m_para;
    CXTPPropertyGrid* m_grid;
    ImageLotView* m_imageLotView;
    ProcCommonDebugInfoDlg& m_procDebugInfo;

    CString GetKey(long type, long index);

    void ShowImage();
    void SetROI();
};
