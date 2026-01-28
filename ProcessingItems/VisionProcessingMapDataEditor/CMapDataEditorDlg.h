#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Component.h"
#include "../../InformationModule/dPI_DataBase/DMSArrayChip.h"
#include "../../InformationModule/dPI_DataBase/DMSBall.h"
#include "../../InformationModule/dPI_DataBase/DMSChip.h"
#include "../../InformationModule/dPI_DataBase/DMSHeatsink.h"
#include "../../InformationModule/dPI_DataBase/DMSPassiveChip.h"
#include "../../InformationModule/dPI_DataBase/DMSPatch.h"
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;

//HDR_6_________________________________ Header body
//
enum PassiveType //mc_Tpye별 Index로 구분이 되어있어 혼동된다..
{
    PassiveType_Capacitor, //Cap
    PassiveType_Resitor, //Res
    PassiveType_Tantalum, //Tantal
    PassiveType_LandSideCapacitor, //LSC
};

enum PropertyGridItemID_Common
{
    CommonItemID_DMS_TYPE = 1,
};

enum PropertyGridItemID_DIE
{
    Die_ID_WIDTH = 1,
    Die_ID_LENGTH,
    Die_ID_THICKNESS,
    Die_ID_ANGLE,
    Die_ID_UNDERFILLWIDTH,
    Die_ID_UNDERFILLLENGTH,
};

enum PropertyGridItemID_PASSIVE
{
    Passive_ID_WIDTH = 1,
    Passive_ID_LENGTH,
    Passive_ID_THICKNESS,
    Passive_ID_ANGLE,
    Passive_ID_ELECT_WIDTH,
    Passive_ID_ELECT_THICKNESS,
    Passive_ID_PAD_WIDTH,
    Passive_ID_PAD_LENGTH,
    Passive_ID_TYPE,
};

enum PropertyGridItemID_ARRARY
{
    Array_ID_WIDTH = 1,
    Array_ID_LENGTH,
    Array_ID_THICKNESS,
    Array_ID_ANGLE,
    Array_ID_ELECT_WIDTH,
    Array_ID_ELECT_LENGTH,
    Array_ID_ELECT_THICKNESS,
    Array_ID_ELECT_PITCH,
    Array_ID_ELECT_TIP,
    Array_ID_PAD_WIDTH,
    Array_ID_PAD_LENGTH,
    Array_ID_TYPE,
};

enum PropertyGridItemID_HEATSYNC
{
    HeatSync_ID_WIDTH = 1,
    HeatSync_ID_LENGTH,
    HeatSync_ID_ANGLE,
};

enum PropertyGridItemID_PATCH //kircheis_POI
{
    Patch_ID_WIDTH = 1,
    Patch_ID_LENGTH,
    Patch_ID_HEIGHT,
};

class CMapDataEditorDlg : public CDialog
{
    DECLARE_DYNAMIC(CMapDataEditorDlg)

public:
    CMapDataEditorDlg(CWnd* pParent = nullptr); // 표준 생성자입니다.
    virtual ~CMapDataEditorDlg();
    virtual BOOL OnInitDialog();
    virtual INT_PTR DoModal();

    //// 대화 상자 데이터입니다.
    //enum { IDD = IDD_DLG_MAPDATA_EDITOR };

#ifdef AFX_DESIGN_TIME
    enum
    {
        IDD = IDD_DLG_MAPDATA_EDITOR
    };
#endif

public:
    CXTPPropertyGrid* m_propertyGrid;
    CListBox m_ListRegisteredDMSData;

    CPackageSpec* m_pPackageSpec;
    long m_nChipType;
    BOOL m_bIsImportMode; //mc_Import시에만 별도로 기능을 적용하기 위해

    std::vector<Package::Component> m_vecCSVMapData; //MapData

    //{{ Component DB Data
    Chip::DMSChip* m_pDmsChipInfo;
    std::vector<Chip::DMSChip> m_vecChipDB;
    Chip::DMSPassiveChip* m_pDmsPassiveInfo;
    std::vector<Chip::DMSPassiveChip> m_vecPassveChipDB;
    Chip::DMSArrayChip* m_pDmsArrayInfo;
    std::vector<Chip::DMSArrayChip> m_vecArrayChipDB;
    Chip::DMSHeatsink* m_pDmsHeatsink;
    std::vector<Chip::DMSHeatsink> m_vecHeatsink;
    Chip::DMSPatch* m_pDmsPatch; //kircheis_POI
    std::vector<Chip::DMSPatch> m_vecPatch; //kircheis_POI
    Chip::DMSBall* m_pDmsBallInfo;
    std::vector<Chip::DMSBall> m_vecBallDB;
    //}}

protected:
    virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV 지원입니다.
    virtual BOOL DestroyWindow();

    DECLARE_MESSAGE_MAP()

public: //Function
    //Save & Load
    void SaveDMSDataDB(long i_nChipType);
    void LoadDMSDataDB(long i_nChipType);

    BOOL NewMapDataSet(Package::Component i_sMapData);
    BOOL SetPackageSpecData(CPackageSpec* i_pPackageSpec, Chip::DMSChip* i_pDmsChipInfo,
        Chip::DMSPassiveChip* i_pDmsPassiveinfo, Chip::DMSArrayChip* i_pDMSArrayInfo,
        Chip::DMSHeatsink* i_pDMSHeatSyncInfo, Chip::DMSPatch* i_pDMSPatchInfo, Chip::DMSBall* i_pDmsBallInfo,
        std::vector<Package::Component> i_vecCSV_MapData, BOOL i_bIsImportMode = FALSE);

private:
    //DMS Grid View
    void SetPropertyGrid();
    void SetChipDMSGrid();
    void SetPassiveDMSGrid();
    void SetArrayDMSGrid();
    void SetHeatSyncDMSGrid();
    void SetPatchSyncDMSGrid(); //kircheis_POI

    //Save & Load DMS Data
    //{{Save
    void SaveChipDMSDB(CString i_strDBName);
    void SavePassiveDMSDB(CString i_strDBName);
    void SaveArrayDMSDB(CString i_strDBName);
    void SaveHeatSyncDMSDB(CString i_strDBName);
    void SavePatchDMSDB(CString i_strDBName); //kircheis_POI
    //}}

    //{{Load
    void LoadChipDMSDB(CString i_strDBName);
    void LoadPassiveDMSDB(CString i_strDBName);
    void LoadArrayDMSDB(CString i_strDBName);
    void LoadHeatSyncDMSDB(CString i_strDBName);
    void LoadPatchDMSDB(CString i_strDBName); //kircheis_POI
    //}}

    void UpdateRegistedList(); //ListUpdate

    BOOL ModifyRegisterCompType(long i_nChipType, CString i_strSaveCompType);
    void PushChipDBData(long i_nChipType);
    void SetChangeDMSData(long i_nChipType);

protected: //Event
    afx_msg LRESULT OnGridNotify(WPARAM wparam, LPARAM lparam);
    afx_msg void OnBnClickedButtonRegisterDmsname();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint point);
    afx_msg void OnMenuDelete();
    afx_msg void OnMenuUpdate();
    afx_msg void OnLbnSelchangeListDmsdatabase();
};
