#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Image8u3.h>

//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageView;

//HDR_6_________________________________ Header body
//

typedef long RoiIndex;
typedef double IntensityLevel_gv;
typedef long ChannelIndex;

typedef struct VerifyResult
{
    double intensity_max;
    double intensity_max_percent;
    double intensity_min;
    double intensity_min_percent;
    long pane_X_max;
    long pane_Y_max;
    long pane_X_min;
    long pane_Y_min;
    double intensity_avg;
    bool isPass;

    VerifyResult()
        : intensity_max(0.)
        , intensity_max_percent(0.)
        , intensity_min(0.)
        , intensity_min_percent(0.)
        , pane_X_max(0)
        , pane_Y_max(0)
        , pane_X_min(0)
        , pane_Y_min(0)
        , intensity_avg(0.)
        , isPass(FALSE)
    {
    }
} VerifyResult;

enum color_split_idx
{
    color_split_idx_Start = 0,
    color_split_idx_Red = color_split_idx_Start,
    color_split_idx_Green,
    color_split_idx_Blue,
    color_split_idx_End,
};

class CDlgSub2DVisionTune : public CDialog
{
    DECLARE_DYNAMIC(CDlgSub2DVisionTune)

public:
    CDlgSub2DVisionTune(CWnd* pParent /* = NULL*/, const long& vision_type); // standard constructor
    virtual ~CDlgSub2DVisionTune();

// Dialog Data
#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_ILLUM_CALIBRATION_TUNING
    };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

private:
    Ipvm::ImageView* m_imageView;
    Ipvm::ProfileView* histogram_view;
    Ipvm::ProfileView* intensity_profile_view;

    CXTPPropertyGrid* grid_Function;
    CXTPPropertyGrid* grid_Calibration;
    CXTPPropertyGrid* grid_Verify;
    int function_type;
    int calibration_target_type;
    int m_cur_side_module;

private:
    CStatic scanTime;
    CStatic procStatus;
    CTabCtrl tabChannel;
    CGridCtrl gridResult;
    CGridCtrl gridSpec;

    long m_vision_type;
    long use_illumation_Count;

    long UV_channel_id;
    long IR_channel_id;
    bool simulationMode;

    int uniformity_live_update_mode;
    int view_color_image_RGB_idx;

    float illumination_times_ms_live[LED_ILLUM_CHANNEL_MAX];
    float verify_ref_illumination_ms[LED_ILLUM_CHANNEL_MAX];
    float verify_ref_illumination_gv[LED_ILLUM_CHANNEL_MAX];
    float verify_illumination_spec_values_percent[LED_ILLUM_CHANNEL_MAX]; // 균일도 spec value [%]

    long scan_time;
    bool stop_calibration;
    bool isLive_mode;
    bool isLiveScan_mode;

    //Live image
    Ipvm::Image8u* mono_Image;
    Ipvm::Image8u3* color_Image;

    // illumination verify
    std::vector<Ipvm::Rect32s> result_areas;
    std::vector<VerifyResult> verify_results_gv;
    std::vector<std::vector<VerifyResult>> verify_results_color_gv;
    std::map<ChannelIndex, std::map<RoiIndex, IntensityLevel_gv>> resultIntensities_mono_gv;
    std::map<ChannelIndex, std::map<RoiIndex, IntensityLevel_gv>>
        resultIntensities_color_gv[color_split_idx::color_split_idx_End];

    //calc image
    std::vector<Ipvm::Image8u> verify_grabed_images;
    std::vector<Ipvm::Image8u3> verify_grabed_color_images;

private:
    const long getUse_illum_count(const long& probeID);

    void InitWindowSize();

    void SetPropertyGrid_General();
    void SetPropertyGrid_Calibration();
    void SetPropertyGrid_Verify();

    LPCWSTR GetChannelNames(const long& probeID, const long& channel_index);
    COLORREF GetChannelColors(
        const long& vision_type, const long& channel_index, const long& UV_channel_idx, const long& IR_channel_idx);

    void UpdateLiveButtons();
    void SetReadOnlyButton_Scan(const BOOL i_bReadOnly);
    void SetReadOnlyButton_Live(const BOOL i_bReadOnly);

    void liveOnOff(const bool& enable);

    void change_scan_function(bool bShow);
    void SetGridCtrl_Result(const long& vision_type, const int& color_idx, const long& channel = 0);
    void SetGridCtrl_Spec(const long& vision_type);

    void StartScan_Uniformity(const long& vision_type, const bool& isusing_gv_mode);
    void StartScan_Uniformity_Single(const long& vision_type);
    bool grab_Uniformity_images(const long& vision_type, const long& channel, const std::array<float, 16>& illums_ms,
        const int& cur_side_module, const bool& isuse_gv_mode);
    float get_use_gv_illum_ms(const long& channel_id, const float& value_gv);
    void CalculateVerify_mono(const long& vision_type, const std::vector<Ipvm::Image8u>& images);
    void CalculateVerify_color(const long& vision_type, const std::vector<Ipvm::Image8u3>& images);
    void CalculateVerify_mono(const long& vision_type, const Ipvm::Image8u& image);
    void CalculateVerify_color(const long& vision_type, const Ipvm::Image8u3& image);
    void CalcSubAreas(const long& vision_type, const Ipvm::Image8u& image, long areaCountX, long areaCountY,
        long deflateValueX, long deflateValueY, std::vector<Ipvm::Rect32s>& rois);

    bool GetNeedimageResize(const Ipvm::Image8u& image, const long& division_X, const long& division_Y);
    bool GetNeedimageResize(const Ipvm::Image8u3& image, const long& division_X, const long& division_Y);
    bool Resizeimage(
        const Ipvm::Image8u& image, const long& division_X, const long division_Y, Ipvm::Image8u& o_ResizeImage);
    bool Resizeimage(
        const Ipvm::Image8u3& image, const long& division_X, const long division_Y, Ipvm::Image8u3& o_ResizeImage);
    bool GetSplitGrayImage(
        const Ipvm::Image8u3& image_Color, const color_split_idx& spit_color_idx, Ipvm::Image8u& o_splitimage);

    void CalculateVerify_mono_realtime(const long& vision_type, const Ipvm::Image8u& image);
    void CalculateVerify_color_realtime(const long& vision_type, const Ipvm::Image8u3& image);

    void SaveReport_Verify(const long& vision_type);
    void SaveReport_Uniformity_Gray(const CString i_strSavePath, const CString i_strSaveTime);
    void SaveReport_Uniformity_Color(const CString i_strSavePath, const CString i_strSaveTime);

    void TestSimulation();
    void TestSimulation_Live();

    void view_avg_gv(const long& vision_type);

private:
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnGrabEndEvent(WPARAM wparam, LPARAM lparam);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void On2DGridDataChanged(NMHDR* pNotifyStruct, LRESULT* result);
};
