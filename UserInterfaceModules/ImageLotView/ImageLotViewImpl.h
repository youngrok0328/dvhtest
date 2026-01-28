#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ImageLotView.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../InformationModule/dPI_DataBase/VisionTrayScanSpec.h"

//HDR_3_________________________________ External library headers
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class IllumInfo2D;
class ImageViewEx;
class InspectionAreaInfo;
class VisionImageLot;
class VisionImageLotInsp;
class VisionScale;
class VisionTrayScanSpec;
class ImageLotView;

//HDR_6_________________________________ Header body
//
class ImageLotViewImpl : public CDialog
{
    // Construction

public:
    ImageLotViewImpl(const CRect& roi, const IllumInfo2D& illumInfo2D, const InspectionAreaInfo& inspectionAreaInfo,
        VisionTrayScanSpec& visionTrayScanSpec, const VisionImageLot& imageLot, const VisionImageLotInsp& imageLotInsp,
        bool showInitialPaneArea, CWnd* pParent,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    ImageLotViewImpl(const CRect& roi, const IllumInfo2D& illumInfo2D, const InspectionAreaInfo& inspectionAreaInfo,
        VisionTrayScanSpec& visionTrayScanSpec, const VisionImageLot& imageLot, bool showInitialPaneArea, CWnd* pParent,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    ImageLotViewImpl(const CRect& roi, const IllumInfo2D& illumInfo2D, const InspectionAreaInfo& inspectionAreaInfo,
        VisionTrayScanSpec& visionTrayScanSpec, const VisionScale& scale, const VisionImageLotInsp& imageLotInsp,
        CWnd* pParent, const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT);

    virtual ~ImageLotViewImpl();

    void SystemEvent_BackPane();
    void SystemEvent_NextPane();
    void SystemEvent_UpdateCurrentPaneZoom();
    void SystemEvent_ChangeOverlayMode();

    void SetMode(ImageLotView::Mode mode);
    void SetPaneWindowEnabled(bool enabled);
    void SetInspectionAreaInfo(const InspectionAreaInfo& inspectionAreaInfo);

    void SetMessage_RoiChanged(UINT message);
    void SetMessage_SelChangedPane(UINT message);
    void SetMessage_SelChangedRaw(UINT message);
    void SetMessage_SelChangedImage(UINT message);
    void SetTarget(VisionImageLot& imageLot, VisionImageLotInsp& imageLotInsp);
    void ShowImage(const long frameIndex);
    void ShowPane(const long paneIndex, bool showRaw);
    void ShowImage(const long frameIndex, bool showRaw, bool bIsSrcRaw = false);
    void OnJobChanged(const bool updateImage, const bool showPaneArea);
    bool IsShowRaw();
    long GetCurrentFrame();
    long GetCurrentPane();
    void UpdateState();

    void ZoomImageFit();
    void ZoomPaneFit(bool displayPaneROI);
    void SetScanIndexField(LPCTSTR text);

    enum
    {
        IDD = IDD_DIALOG
    };

    ImageViewEx* m_coreView;
    const IllumInfo2D& m_illumInfo2D;
    InspectionAreaInfo m_inspectionAreaInfo;
    VisionTrayScanSpec& m_visionTrayScanSpec;
    const VisionImageLot* m_imageLot;
    const VisionImageLotInsp* m_imageLotInsp;

    UINT m_paneSelChangedMsg;
    UINT m_roiChangedMsg;
    UINT m_selChangedRawMsg;
    UINT m_selChangedImageMsg;
    long m_currentImageIndex;
    bool m_showInitialPaneArea;
    CString m_frameText;

protected:
    bool CheckRawButtonCheckForCurrentPane(bool& showRaw, bool bIsSrcRaw = false); //mc_Src가 Raw Image일경우..
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support

protected:
    const VisionScale& m_scale;
    ImageLotView::Mode m_mode;
    long m_overlay_mode;
    bool m_enabled_paneWindow;

    // 사용자가 원하는 Raw로 보여줄 것인가.
    // 검사 영상을 보여달라고 해도 영상이 안말들어졌으면 못보여준다

    bool m_display_raw;

    void UpdateUIState();

    bool getCurrentRawMode() const;
    long getImageFrameCount(
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    Ipvm::Image8u getCurrentImageFrame() const;
    Ipvm::Image8u getImageFrame(long frameIndex,
        const enSideVisionModule i_eSideVisionModule = enSideVisionModule::SIDE_VISIONMODULE_FRONT) const;
    Ipvm::Image32r getZmapFrame();
    Ipvm::Image16u getVmapFrame();

    //{{ Color Image Control - 2021.02
    Ipvm::Image8u3 getCurrentColorImageFrame() const;
    Ipvm::Image8u3 getColorImageFrame(long frameIndex) const;
    //}}

    virtual BOOL OnInitDialog();
    DECLARE_MESSAGE_MAP()

private:
    CRect m_startPosition;
    CComboBox m_comboPane;
    CComboBox m_comboPaneX;
    CComboBox m_comboPaneY;
    CButton m_checkRawImage;
    CButton m_button3DView;
    CButton m_button2DColor;
    CButton m_buttonPrev;
    CButton m_buttonNext;
    CStatic m_stateView;

    void arraymentUI();
    afx_msg void OnSize(UINT nType, int cx, int cy);

public:
    afx_msg void OnCbnSelchangeComboPane();
    afx_msg void OnCbnSelchangeComboPaneXY();
    afx_msg void OnBnClickedCheckRawImage();
    afx_msg void OnBnClickedButton3dview();
    afx_msg void OnBnClickedButton2dColorview();
    afx_msg void OnBnClickedButtonPrev();
    afx_msg void OnBnClickedButtonCurr();
    afx_msg void OnBnClickedButtonNext();
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    static void callback_roiChanged(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength);

    //{{Uniformity Test
    void CalcGrayTargetSubAreaIntensities(Ipvm::Image8u3 image);
    void CalcGrayTargetSubArea(
        const Ipvm::Image8u3& image, long i_nAreaCountX, long i_nAreaCountY, std::vector<Ipvm::Rect32s>& rtROIs);
    void SaveUniformity(std::vector<double> i_vecMeanR, std::vector<double> i_vecMeanG, std::vector<double> i_vecMeanB);
    //}}
    afx_msg void OnBnClickedButtonColorUnifrom();

    void SetSideVisionModule(const enSideVisionModule i_eSideVisionModule);
    void SetRawImageCheckBox(bool i_bCheck); // Stitch Section Image Combine

private:
    enSideVisionModule m_eSideVisionModule;
};
