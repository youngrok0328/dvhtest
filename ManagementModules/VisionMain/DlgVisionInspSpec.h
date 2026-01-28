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

//HDR_6_________________________________ Header body
//
class CDlgVisionInspSpec : public CDialog
{
    DECLARE_DYNAMIC(CDlgVisionInspSpec)

public:
    CDlgVisionInspSpec(VisionUnit* visionUnit, CWnd* pParent = NULL); // 표준 생성자입니다.
    virtual ~CDlgVisionInspSpec();

    // 대화 상자 데이터입니다.
    enum
    {
        IDD = IDD_DIALOG_VISION_INSP_SPEC
    };

    VisionUnit* m_visionUnit;
    CGridCtrl* m_gridVisionSM;
    CGridCtrl* m_gridVisionSI;
    CGridCtrl* m_gridVisionMark;
    CGridCtrl* m_gridVisionGeometry;
    CGridCtrl* m_gridVisionFixed;

    std::vector<VisionProcessing*> m_availableFixedItems;

    // 함수

protected:
    CMemFile m_dbBackup;

    void SetGrid();
    void SetGridSurface(CGridCtrl* grid);
    void SetGridMark(CGridCtrl* grid);
    void SetGridGeometry(CGridCtrl* grid);
    long GetSelectedGridCellRow(CGridCtrl* grid);

    long GetGridSurface_StartIndex(CGridCtrl* grid);
    long GetGridSurface_EndIndex(CGridCtrl* grid);
    long GetGridSurface_ItemCount(CGridCtrl* grid);
    long GetSelectedSurfaceIndex(CGridCtrl* grid);
    void ExportSurfaceItem(long surfaceItemIndex);
    void ImportSurfaceItem(long surfaceItemIndex);
    void EventGridSurfaceEditEnd(CGridCtrl* grid, NMHDR* pNotifyStruct, LRESULT* result);

    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.

    DECLARE_MESSAGE_MAP()

public:
    virtual BOOL OnInitDialog();
    virtual void OnOK() {};
    virtual void OnCancel() {};
    afx_msg void OnGridEditEnd_SM(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridEditEnd_SI(NMHDR* pNotifyStruct, LRESULT* result);

    afx_msg void OnGridEditEnd_Mark(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridDbClick_Mark(NMHDR* pNotifyStruct, LRESULT* result);

    afx_msg void OnGridEditEnd_Geometry(NMHDR* pNotifyStruct, LRESULT* result);

    afx_msg void OnGridEditEnd_InspItem(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridDbClick_InspItem(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnBnClickedButtonSmAdd();
    afx_msg void OnBnClickedButtonSmDelete();
    afx_msg void OnBnClickedButtonSmExport();
    afx_msg void OnBnClickedButtonSmImport();

    afx_msg void OnBnClickedButtonSiAdd();
    afx_msg void OnBnClickedButtonSiDelete();
    afx_msg void OnBnClickedButtonSiExport();
    afx_msg void OnBnClickedButtonSiImport();

    afx_msg void OnBnClickedButtonMarkAdd();
    afx_msg void OnBnClickedButtonMarkDelete();
    afx_msg void OnBnClickedBtnGeoDel();
    afx_msg void OnBnClickedBtnGeoAdd();
    afx_msg void OnBnClickedButtonApply();
    afx_msg void OnBnClickedButtonCancel();
};
