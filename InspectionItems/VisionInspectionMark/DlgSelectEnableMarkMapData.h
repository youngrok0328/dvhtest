#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "VisionInspectionMark.h"
#include "VisionInspectionMarkPara.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionInspectionMark;

//HDR_6_________________________________ Header body
//
enum enumPropertyMarkSelectEnableItemID
{
    ITEM_ID_ENABLE_APPLY,
    ITEM_ID_SHOW_ALL_SETTING_ROI,
    ITEM_ID_SHOW_SELECT_TEACH_ROI,
    ITEM_ID_SHOW_SELECT_IGNORE_ROI,
    ITEM_ID_SHOW_SELECT_MERGE_ROI,
    ITEM_ID_TEACH_ROI_DEFINE = 10,
    ITEM_ID_IGNORE_ROI_DEFINE = 50,
    ITEM_ID_MERGE_ROI_DEFINE = 100,
};

enum class DefineSelectROI_Type
{
    Teach,
    Ignore,
    Merge,
};

class CDlgSelectEnableMarkMapData : public CDialog
{
    DECLARE_DYNAMIC(CDlgSelectEnableMarkMapData)

public:
    CDlgSelectEnableMarkMapData(VisionInspectionMark* i_pVisionMarkInsp, const sReadMarkMapInfo& i_sMarkReadDataInfo,
        bool i_bIsTeachROI, bool i_bImportMode, CWnd* pParent = nullptr);
    virtual ~CDlgSelectEnableMarkMapData();

#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_SELECT_ENABLE_MARK_MAPDATA
    };
#endif

private:
    CXTPPropertyGrid* m_propertyGrid;
    ImageLotView* m_imageLotView;
    VisionInspectionMark* m_pVisionMarkInsp;
    sReadMarkMapInfo m_sMarkReadDataInfo;
    sMarkMapInfo m_sEnableMarkMapInfo_px;

    bool m_bIsTeachROI; //User ROI Setting이 필요한지에 대한 변수
    bool m_bImportMode; //Import시에는 Cancle이 되지 않아야한다.

    long m_nPreSelect_TeachROI_idx;
    int m_nTeachROI_Count;
    std::vector<BOOL> m_vecbTeachROI_Enable;

    long m_nIgnoreROI_Count;
    std::vector<BOOL> m_vecbIgnoreROI_Enable;

    long m_nMergeROI_Count;
    std::vector<BOOL> m_vecbMergeROI_Enable;

private:
    void UpdatePropertyGrid();
    void SetReadOnlyItem(DefineSelectROI_Type i_DefineSelectROIType);
    void ShowSelectedROI(DefineSelectROI_Type i_DefineSelectROIType, std::vector<sMarkROI_Info> i_vecsMarkROI_Info);
    std::vector<BOOL> GetShowROIidx(std::vector<BOOL> i_vecbROI_Enable);

    FPI_RECT GetMarkROI_px(sMarkROI_Info i_sROI_Info);
    Ipvm::Point32r2 GetOverlayMarkNamePos(FPI_RECT i_frtMarkROIPos_px);

    void ShowViewROI(long i_nSelect_Show_ROIType);
    void SetEnableMarkData();
    void SetUserROI();
    void SetUserROIData(Ipvm::Rect32s i_rtROI_px, bool i_bInitailize = false);
    void SetDefalutEnable();
    void ShowAllOverlay();

public:
    sMarkMapInfo GetEnableMarkMapInfo();

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

private:
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg LRESULT OnRoiChanged(WPARAM wParam, LPARAM lParam);

public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};
