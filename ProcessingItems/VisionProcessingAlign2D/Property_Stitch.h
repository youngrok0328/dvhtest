#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../UserInterfaceModules/ProcessingCommonDialog/ProcCommonGridBase.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class DlgBodySizeROIPara;
class StitchPara;
class ImageLotView;
class ProcCommonDebugInfoDlg;
class VisionProcessingAlign2D;

//HDR_6_________________________________ Header body
//
class Property_Stitch : public ProcCommonGridBase
{
public:
    Property_Stitch(VisionProcessingAlign2D& proccsor, ImageLotView* imageLotView,
        ProcCommonDebugInfoDlg& procDebugInfo, StitchPara& para);
    Property_Stitch(VisionProcessingAlign2D& proccsor, ImageLotView* imageLotView, ImageLotView* imageLotViewRearSide,
        ProcCommonDebugInfoDlg& procDebugInfo, StitchPara& para);
    virtual ~Property_Stitch();

    void redrawOverlayAndROI();
    void ClickedButtonStitchImageCombine(); // Stitch Section Image Combine 기능 추가 - 2024.11.06_JHB

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
    VisionProcessingAlign2D& m_processor;
    StitchPara& m_para;
    CXTPPropertyGrid* m_grid;
    ImageLotView* m_imageLotView;
    ImageLotView* m_imageLotViewRearSide;
    DlgBodySizeROIPara* m_subDlgBodyROIs;
    ProcCommonDebugInfoDlg& m_procDebugInfo;

    struct SDebugOption
    {
        float m_offsetX[4];
        float m_offsetY[4];
        float m_angle[4];

        SDebugOption()
        {
            for (long n = 0; n < 4; n++)
            {
                m_offsetX[n] = 0.f;
                m_offsetY[n] = 0.f;
                m_angle[n] = 0.f;
            }
        }
    };

    SDebugOption m_debug;
    CRect m_region;

    void SetGrid_BaseOn();
    void SetGrid_BaseOnMatching(CXTPPropertyGridItem* category);
    void SetGrid_BaseOnBodySize(CXTPPropertyGridItem* category);
    bool Nodifiy_BaseOnMatching(long gridIndex, WPARAM wparam, LPARAM lparam);
    bool Nodifiy_BaseOnBodySize(long gridIndex, WPARAM wparam, LPARAM lparam);

    BOOL SplitCopyImage(
        std::vector<Ipvm::Image8u>& vecSrcImage, std::vector<Ipvm::Image8u>& vecDstImage, Ipvm::Rect32s rtDestROI);
    void ShowImage();
    bool SetROI();
    void SetROI_BaseOnMatching();
    void SetROI_BaseOnBodySize();
    void GetROI();
    void GetROI_BaseOnMatching();
    void GetROI_BaseOnBodySize();
    void UpdateROI();
    void ShowOverlayROI();
    void ShowOverlayROI_BaseOnMatching();
    void ShowOverlayROI_BaseOnBodySize();
    void SelChangedRaw();
    void buttonClick_DefaultTemplateRoi();
};
