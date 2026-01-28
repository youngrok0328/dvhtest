#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionMarkPara.h"
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <afxcmn.h>
#include <afxwin.h>

//HDR_4_________________________________ Standard library headers
#include <Ipvm/Base/Image8u.h>

//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionInspectionMark;
class CDlgVisionInspectionMarkOperator;
class CDlgSelectEnableMarkMapData;
class ProcCommonSpecDlg;
class ProcCommonResultDlg;
class ProcCommonDetailResultDlg;
class ProcCommonLogDlg;
class ProcCommonDebugInfoDlg;
class VisionInspectionMarkSpec;
struct ProcessingDlgInfo;
struct sMarkROI_Info;

//HDR_6_________________________________ Header body
//
enum enumPropertyItemID
{
    ITEM_ID_COMBO_DEBUGVIEW_IMAGE = 1,
    ITEM_ID_BUTTON_ALGORITHM_PARAM,
    ITEM_ID_BUTTON_IMAGECOMBINE,
    ITEM_ID_SELECT_USE_MAP,
    ITEM_ID_MARK_ID_IMPORT_MAP,
    ITEM_ID_MARK_ID_EXPORT_MAP,
    ITEM_ID_MARK_ID_SELECT_ENABLE_MARK_MAPDATA,
    ITEM_ID_MARK_ID_SHOW_MAP,
    ITEM_ID_COMBO_CHARMERGE_ROINUM,
    ITEM_ID_COMBO_CHARIGNORE_ROINUM,
    ITEM_ID_MARK_ALIGNAREAIGNORE,
    ITEM_ID_MARK_IGNOREAREADILATECOUNT,
    ITEM_ID_SLIDER_THRESHOLD,
    ITEM_ID_MARK_ANGLE,
    ITEM_ID_MARK_INSPCTIONMODE,
    ITEM_ID_COMBO_IMAGE_PROCESSING_MODE,
    ITEM_ID_COMBO_LOCATOR_TARGETROI_USE,
    ITEM_ID_COMBO_BLACK_IMAGE_USE,
    ITEM_ID_COMBO_CHAR_INSP_MODE,
    ITEM_ID_COMBO_ANGLE_INSP_MODE,
    ITEM_ID_EDIT_LOCATOR_SEARCH_OFFSET_X,
    ITEM_ID_EDIT_LOCATOR_SEARCH_OFFSET_Y,
    ITEM_ID_EDIT_CHAR_SEARCH_OFFSET_X,
    ITEM_ID_EDIT_CHAR_SEARCH_OFFSET_Y,
    ITEM_ID_EDIT_IMAGE_SAMPLING,
    ITEM_ID_COMBO_MARK_DIRECTION,
    ITEM_ID_COMBO_INITAL_IGNOREROI_USE,
    ITEM_ID_BUTTON_SPLIT_CHAR_TEACH,
    ITEM_ID_BUTTON_TEACH,
    ITEM_ID_BUTTON_INSPECTION_TEST,
};

//{{ImageDebug ViewMode
enum enumDebugViewImageIndex
{
    DebugImage_Start = 0,
    DebugImage_Origin = DebugImage_Start,
    DebugImage_Binary,
    DebugImage_Under,
    DebugImage_Over,
    DebugImage_Blob,
    DebugImage_End,
};

static LPCTSTR g_szImageDebugModeName[] = {
    _T("Origin Image"),
    _T("Binary Image"),
    _T("Under Image"),
    _T("Over Image"),
    _T("Blob Image"),
};

//}}

//{{ Image Processing
static LPCTSTR g_szImageProcessingModeName[] = {
    _T("Nothing"),
    _T("Median + Low Pass"),
    _T("Low Pass + Median"),
    _T("Median"),
    _T("Low Pass"),
};

enum enumImageProcessingModeIndex
{
    ImageProcessingMode_Start = 0,
    ImageProcessingMode_Noting = ImageProcessingMode_Start,
    ImageProcessingMode_MedianLowPass,
    ImageProcessingMode_LowPassMedian,
    ImageProcessingMode_Median,
    ImageProcessingMode_LowPass,
    ImageProcessingMode_End,
};

//}}

//{{ Angle InspMode
static LPCTSTR g_szAngleInspModeName[] = {
    _T("Teaching Image"),
    _T("Body Align"),
};

enum enumAngleInspModeIndex
{
    AngleInspMode_Start = 0,
    AngleInspMode_Teaching_Image = AngleInspMode_Start,
    AngleInspMode_Body_Align,
    AngleInspMode_End,
};

//}}

//{{ Mark Direction

static LPCTSTR g_szMarkDirectionName[] = {
    _T("Horizontal"),
    _T("Vertical"),
};

enum enumMarkDirectionIndex
{
    MarkDirection_Start = 0,
    MarkDirection_Horizontal = MarkDirection_Start,
    MarkDirection_Vertical,
    MarkDirection_End,
};
//}}

class CDlgVisionInspectionMark : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspectionMark)

public:
    CDlgVisionInspectionMark(const ProcessingDlgInfo& procDlgInfo, VisionInspectionMark* pVisionInsp,
        CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspectionMark();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG
    };

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    const ProcessingDlgInfo& m_procDlgInfo;
    ImageLotView* m_imageLotView;
    CXTPPropertyGrid* m_propertyGrid;
    ProcCommonSpecDlg* m_procCommonSpecDlg;
    ProcCommonResultDlg* m_procCommonResultDlg;
    ProcCommonDetailResultDlg* m_procCommonDetailResultDlg;
    ProcCommonLogDlg* m_procCommonLogDlg;
    ProcCommonDebugInfoDlg* m_procCommonDebugInfoDlg;

    VisionInspectionMarkPara* m_pVisionPara;
    VisionInspectionMarkSpec* m_pVisionSpec;

    CDlgVisionInspectionMarkOperator* m_pVisionInspOpParaDlg;
    VisionInspectionMark* m_pVisionInsp;

    BOOL m_bShowResultDetail;

public:
    void SetInitDialog();
    void SetInitSetupWindow();
    void UpdatePropertyGrid();

    //void ShowImage(BOOL bChange = FALSE);
    void ShowImage(BOOL bAutoThreshold, long nMultiID);

    //{{정리 덜됨
    void ShowCombineImage(BOOL bAutoThreshold);
    void GetROI();
    void SetROI();
    void UpdateMergeROI();
    void UpdateIgnoreROI();
    void RefreshData();
    void MakeMarkInfoPixel();
    //}}

    //{{ Button Evnet

    void CharSplitTeach();
    void MarkTeach();
    void InspectionTest();
    //}}

    //kk 2019.11.20 Mark Map Data
    void ClickedButtonImportMarkROI_MapData();
    void ClickedButtonExportMarkROI_MapData();
    void DrawMarkMapROI(const BOOL isSplitTeach);
    void MakeMarkROI(sMarkROI_Info i_sROI_Info, FPI_RECT& o_frtResultROI);
    void SetRoiParameter();
    void SetROI_MapData();
    void SetROI(long nMode);
    void GetROI(long nMode);

private:
    sReadMarkMapInfo m_sMarkMapReadDataInfo_um; // mc_Import시 Enable 구조에 데이터 할당을 위한 사용
    sReadMarkMapInfo m_sMarkMapReadDataInfo_px; // mc_Enable 기능을 사용하기 위하여 필요하다

    void SetGroupData_um(const std::vector<sMarkReadDataInfo>& i_vecsReadDataInfo, sReadMarkMapInfo& o_sGroupInfo_um);
    void ReadMarkMapInfoFromCSV(CString strFileFullPath);
    void SetReadMapDataInfo_px(sReadMarkMapInfo i_sMarkMapInfo);

    void ClickButtonSelectEnableMarkMapData(bool i_bImportMode = false);

    void RefreshMarkDataInfo();

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnDestroy();
    afx_msg void OnClose();
    afx_msg void OnBnClickedButtonClose();
    afx_msg void OnBnClickedButtonInspect();
    afx_msg void OnTcnSelchangeTabResult(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg LRESULT OnRoiChanged(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnImageLotViewPaneSelChanged(WPARAM /*wparam*/, LPARAM /*lparam*/);

public:
    CTabCtrl m_TabResult;
    CTabCtrl m_TabLog;

    CButton m_buttonInspect;
    CButton m_buttonClose;

    long m_nDebugViewImageIndex;
    Ipvm::Image8u m_ThresholdImage;
    BOOL m_bBtnSplitePush;
    std::vector<Ipvm::Point32s2> m_vecptSplitePoint;

    BOOL SwitchParameterDialog(long i_nCurTabIndex);

    afx_msg LRESULT OnShowDetailResult(WPARAM wparam, LPARAM lparam);

    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    BOOL PreImageViewSeq(Ipvm::Image8u& o_DebugViewImage); //ImageView를 하기전 수행할 목록
    void RefreshSpecCtrl(long i_nInspectionType);
};
