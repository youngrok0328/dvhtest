#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "resource.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommonUtilityModules/dPI_GridCtrl/GridCtrl.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionInspectionGeometry;
class VisionInspectionGeometryPara;
class VisionInspectionSpec;
class ImageViewEx;
class CPI_EdgeDetect;
class VisionDebugInfo;

//HDR_6_________________________________ Header body
//
class CDlgGeometryEditor : public CDialog
{
    DECLARE_DYNAMIC(CDlgGeometryEditor)

public:
    CDlgGeometryEditor(VisionInspectionGeometry* pVisionInsp, long nInspectionID = 0, CWnd* pParent = NULL);
    virtual ~CDlgGeometryEditor();

    enum
    {
        IDD = IDD_DIALOG_EDITOR
    };

public:
    std::vector<CGridCtrl*> m_pvecGridCtrl;

    CGridCtrl m_gridAlignInfoDataList;
    CGridCtrl m_gridSettingParameter_Ref;
    CGridCtrl m_gridSettingParameter_Tar;
    CGridCtrl m_gridParameter_Insp;
    CEdit m_editResult;
    CStatic m_staticImageView;

public:
    VisionInspectionGeometry* m_pVisionInsp;
    VisionInspectionGeometryPara* m_pVisionPara;
    std::vector<VisionInspectionSpec>* m_pvecSpec;

    ImageViewEx* m_imageView;
    VisionDebugInfo* m_pDebugInfo;

public:
    long m_nSizeX;
    long m_nSizeY;

    Ipvm::Image8u m_procImage;

    long m_nSelectInspectionID;

    CString m_strInspectionName;
    long m_nInspectionType;
    long m_nInspectionDistanceResult;
    long m_nInspectionCircleResult;
    long m_nInspDirection;
    float m_fReferenceSpec;

    // 마법의 변수
    // Reference 및 Target에 나타나는 파라미터들은 모두 정수형으로 입력 받도록 하여 현재 변수에 입력한다.
    std::vector<std::vector<long>> m_vecInfoParameter_0_Ref;
    std::vector<std::vector<long>> m_vecInfoParameter_0_Tar;

    // 두개 세개 추가하려고 했는데.. 이건 좀 아닌 것 같아서.. 차라리 구조체를 하나 만드는게 나을 지도 모르지만.. 현재는 이게 없이 진핸한다.
    // 	std::vector<std::vector<long>> *m_pvecInfoParameter_1_Ref;
    // 	std::vector<std::vector<long>> *m_pvecInfoParameter_1_Tar;
    //
    // 	std::vector<std::vector<long>> *m_pvecInfoParameter_2_Ref;
    // 	std::vector<std::vector<long>> *m_pvecInfoParameter_2_Tar;

    std::vector<CString> m_vecstrinfoName_Ref;
    std::vector<CString> m_vecstrinfoName_Tar;

    std::vector<Ipvm::Rect32r> m_vecrtInspectionROI_Ref_BCU;
    std::vector<Ipvm::Rect32r> m_vecrtInspectionROI_Tar_BCU;

public:
    void GetParameter(VisionInspectionGeometryPara* i_pVisionPara, long nInspectionID);

    void SetGridList_DataList();
    void SetGridList_Param_Insp();

    void SetGridList_Param_Ref_And_Tar(
        CGridCtrl* pGridList, std::vector<CString> vecstrName, std::vector<std::vector<long>>& vecnCheckboxList);
    void SetGridList_BodyInfo(long nBodyInfoID, CGridCtrl* pGridList, long nRowID, CString strName,
        std::vector<long> vecnParametrData, BOOL bFirst);
    void SetGridList_RoundCircleInfo(long nUserInfoID, CGridCtrl* pGridList, long nRowID, CString strName,
        std::vector<long> vecnParametrData, BOOL bFirst);
    void SetGridList_UserInfo(long nUserInfoID, CGridCtrl* pGridList, long nRowID, CString strName,
        std::vector<long> vecnParametrData, BOOL bFirst);

    void ShowImage();
    long GetSelectedGridCellRow(CGridCtrl* pGridList);

    void SetGrid_Text(CGridCtrl* pGridList, long nRow, long nCol, CString strText);
    long GetGrid_Text(CGridCtrl* pGridList, long nRow, long nCol);

    void SetGrid_CheckBox(
        CGridCtrl* pGridList, CString strTrueName, CString strFalseName, long nRow, long nCol, BOOL bCheck);
    void SetGrid_SlideBox(
        CGridCtrl* pGridList, long nRow, long nCol, std::vector<CString> vecstrDataList, long nSelectData);
    long GetGrid_SlideBox(CGridCtrl* pGridList, long nRow, long nCol);

    void SetEdit_Result(std::vector<float> vecfError);

    void ShowOverlay(CString strAlignInfoName, long nInspID, long nSelectID,
        std::vector<std::vector<long>>& vecInfoParameter_0, std::vector<Ipvm::Rect32r>& vecrtInspROI_BCU);

    void OnBnClickedBtnAdd_Data(CGridCtrl* pGridList_Data, CGridCtrl* pGridList_Param,
        std::vector<CString>& vecstrinfoName_Ref, std::vector<std::vector<long>>& vecInfoParameter_0,
        std::vector<Ipvm::Rect32r>& vecrtInspROI_BCU);
    void OnGridClicked_DataList(CGridCtrl* pGridList, long row, long col, std::vector<CString> m_vecstrinfoName,
        std::vector<std::vector<long>>& vecInfoParameter_0, std::vector<Ipvm::Rect32r>& vecrtInspROI_BCU);
    void OnGridEditEnd_DataList(CGridCtrl* pGridList, long row, long col, std::vector<CString> m_vecstrinfoName,
        std::vector<std::vector<long>>& vecInfoParameter_0);

    void SetGrid_inspParam_Hidding(CGridCtrl* pGridList, long nSelectInspType);

    BOOL m_bChangedROI;
    long m_nChangedInspIID;
    long m_nSelectRefAndTar;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

    DECLARE_MESSAGE_MAP()

public:
    CStatic m_stcWarningReference;
    CStatic m_stcWarningTarget;

    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedBtnDelRef();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedBtnRunRef();
    afx_msg void OnBnClickedBtnDelTar();
    afx_msg void OnBnClickedBtnRunTar();
    afx_msg void OnBnClickedBtnAddRef();
    afx_msg void OnBnClickedBtnAddTar();
    afx_msg void OnBnClickedBtnRun();
    afx_msg void OnGridClicked_AlignInfoData(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridClicked_DataList_Ref(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridClicked_DataList_Tar(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridClicked_DataList_InspParam(NMHDR* pNotifyStruct, LRESULT* result);

    afx_msg void OnGridEditEnd_DataList_Ref(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridEditEnd_DataList_Tar(NMHDR* pNotifyStruct, LRESULT* result);
    afx_msg void OnGridEditEnd_DataList_InspParam(NMHDR* pNotifyStruct, LRESULT* result);

    afx_msg void OnBnClickedBtnCancel();

    static void callRoiChanged(void* userData, const int32_t id, const wchar_t* key, const size_t keyLength);
    void callRoiChanged();
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
};
