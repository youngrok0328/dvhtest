#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CDlgVisionPrimaryUI;
class VisionProcessing;
class VisionInspectionSurface;
class VisionUnit;
class CVisionMain;
class CGrabItemFrameInfoNGRV; //kircheis_NGRV
class ImageViewEx;

//HDR_6_________________________________ Header body
//
enum enumInspectionItem
{
    INSP_ITEM_2D_BTM,
    INSP_ITEM_3D_BTM,
    INSP_ITEM_2D_TOP,
    INSP_ITEM_3D_TOP,
    INSP_ITEM_END,
};

enum enumInspectionItemColumn
{
    INSP_ITEM_COL_START = 0,
    INSP_ITEM_COL_NUMBER = INSP_ITEM_COL_START,
    INSP_ITEM_COL_NAME,
    INSP_ITEM_COL_AF_TYPE,
    INSP_ITEM_COL_PLANE_OFFSET,
    INSP_ITEM_COL_USAGE,
    INSP_ITEM_COL_END,
};

class CDlgVisionInspSpecNGRV : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspSpecNGRV)

public:
    CDlgVisionInspSpecNGRV(CVisionMain* visionMain, CWnd* pParent = NULL); // 표준 생성자입니다.//kircheis_NGRV
    virtual ~CDlgVisionInspSpecNGRV();

    // 대화 상자 데이터입니다.
    //#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DIALOG_NGRV_VISION_INSP_SPEC
    };
    //#endif

    CVisionMain* m_visionMain; //kircheis_NGRV
    CGridCtrl* m_gridVisionFixed[INSP_ITEM_END];
    ImageViewEx* m_ImageView_BTM_2D;
    ImageViewEx* m_ImageView_TOP_2D;

    std::vector<VisionProcessing*> m_availableFixedItemsBTM2D;
    std::vector<VisionProcessing*> m_availableFixedItemsBTM3D;
    std::vector<VisionProcessing*> m_availableFixedItemsTOP2D;
    std::vector<VisionProcessing*> m_availableFixedItemsTOP3D;

protected:
    CMemFile m_dbBackup;

    void SetGrid();
    void SetAFImage();
    long GetSelectedGridCellRow(CGridCtrl* grid);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    std::vector<std::vector<CGrabItemFrameInfoNGRV>> m_vec2GrabItemFrameInfoNGRV_Buf; //kircheis_NGRV
    void ClearFrameInfoBuffer(); //kircheis_NGRV

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();

    afx_msg void OnGridEditEnd_InspItem_2D_BTM(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridEditEnd_InspItem_3D_BTM(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridEditEnd_InspItem_2D_TOP(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridEditEnd_InspItem_3D_TOP(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnBnClickedNgrvApply();
    afx_msg void OnBnClickedNgrvCancel();
};
