#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/Component.h"
#include "../../InformationModule/dPI_DataBase/DMSArrayChip.h"
#include "../../InformationModule/dPI_DataBase/DMSBall.h"
#include "../../InformationModule/dPI_DataBase/DMSChip.h"
#include "../../InformationModule/dPI_DataBase/DMSHeatsink.h"
#include "../../InformationModule/dPI_DataBase/DMSPassiveChip.h"
#include "../../InformationModule/dPI_DataBase/DMSPatch.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CMapDataEditorDlg;
class CMapDataConverterDlg;
class CPackageSpec;
//class Component;
//struct DMSChip;
//struct DMSPassiveChip;
//struct DMSArrayChip;
//struct DMSHeatsink;
//struct DMSPatch;
//struct DMSBall;

//HDR_6_________________________________ Header body
//
class __VISION_MAPDATA_EDITOR_UI_CLASS__ VisionMapDataEditorUI
{
public:
    VisionMapDataEditorUI(void) = delete;
    VisionMapDataEditorUI(HWND hwndParent, CPackageSpec* i_pPackageSpec, Chip::DMSChip* i_pDmsChipInfo,
        Chip::DMSPassiveChip* i_pDmsPassiveinfo, Chip::DMSArrayChip* i_pDMSArrayInfo,
        Chip::DMSHeatsink* i_pDMSHeatSyncInfo, Chip::DMSPatch* i_pDMSPatchInfo, Chip::DMSBall* i_pDmsBallInfo,
        std::vector<Package::Component> i_vecCSV_MapData, BOOL i_bIsImportMode = FALSE);
    VisionMapDataEditorUI(HWND hwndParent, CPackageSpec* i_pPackageSpec,
        std::vector<std::pair<long, CString>>& convMapData, BOOL modifyMode);
    ~VisionMapDataEditorUI(void);

public:
    INT_PTR DoModal();
    INT_PTR ConverterDoModal();
    void ShowWindow(int cmdShow);

    CMapDataEditorDlg* m_pMapDataEditorDlg;
    CMapDataConverterDlg* m_pMapDataConverterDlg;

public: //DataGet
    std::vector<Chip::DMSChip> GetDMSChipData();
    std::vector<Chip::DMSPassiveChip> GetDMSPassiveData();
    std::vector<Chip::DMSArrayChip> GetDMSArrayData();
    std::vector<Chip::DMSHeatsink> GetDMSHeatSyncData();
};
