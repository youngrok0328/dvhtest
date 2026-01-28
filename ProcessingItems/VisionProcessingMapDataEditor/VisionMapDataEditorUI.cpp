//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionMapDataEditorUI.h"

//CPP_2_________________________________ This project's headers
#include "CMapDataConverterDlg.h"
#include "CMapDataEditorDlg.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionMapDataEditorUI::VisionMapDataEditorUI(HWND hwndParent, CPackageSpec* i_pPackageSpec,
    Chip::DMSChip* i_pDmsChipInfo, Chip::DMSPassiveChip* i_pDmsPassiveinfo, Chip::DMSArrayChip* i_pDMSArrayInfo,
    Chip::DMSHeatsink* i_pDMSHeatSyncInfo, Chip::DMSPatch* i_pDMSPatchInfo, Chip::DMSBall* i_pDmsBallInfo,
    std::vector<Package::Component> i_vecCSV_MapData, BOOL i_bIsImportMode)
    : m_pMapDataEditorDlg(nullptr)
    , m_pMapDataConverterDlg(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWnd* parentWnd = CWnd::FromHandle(hwndParent);
    m_pMapDataEditorDlg = new CMapDataEditorDlg(parentWnd);
    if (m_pMapDataEditorDlg != NULL)
        m_pMapDataEditorDlg->SetPackageSpecData(i_pPackageSpec, i_pDmsChipInfo, i_pDmsPassiveinfo, i_pDMSArrayInfo,
            i_pDMSHeatSyncInfo, i_pDMSPatchInfo, i_pDmsBallInfo, i_vecCSV_MapData, i_bIsImportMode);
}

VisionMapDataEditorUI::VisionMapDataEditorUI(
    HWND hwndParent, CPackageSpec* i_pPackageSpec, std::vector<std::pair<long, CString>>& convMapData, BOOL modifyMode)
    : m_pMapDataEditorDlg(nullptr)
    , m_pMapDataConverterDlg(nullptr)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CWnd* parentWnd = CWnd::FromHandle(hwndParent);
    m_pMapDataConverterDlg = new CMapDataConverterDlg(parentWnd, i_pPackageSpec, convMapData, modifyMode);
}

VisionMapDataEditorUI::~VisionMapDataEditorUI()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (m_pMapDataEditorDlg != nullptr)
        delete m_pMapDataEditorDlg;

    if (m_pMapDataConverterDlg != nullptr)
        delete m_pMapDataConverterDlg;
}

INT_PTR VisionMapDataEditorUI::DoModal()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return m_pMapDataEditorDlg->DoModal();
}

INT_PTR VisionMapDataEditorUI::ConverterDoModal()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return m_pMapDataConverterDlg->DoModal();
}

std::vector<Chip::DMSChip> VisionMapDataEditorUI::GetDMSChipData()
{
    m_pMapDataEditorDlg->LoadDMSDataDB(MAPDATA_ID_CHIP);

    return m_pMapDataEditorDlg->m_vecChipDB;
}

std::vector<Chip::DMSPassiveChip> VisionMapDataEditorUI::GetDMSPassiveData()
{
    m_pMapDataEditorDlg->LoadDMSDataDB(MAPDATA_ID_PASSIVE);

    return m_pMapDataEditorDlg->m_vecPassveChipDB;
}

std::vector<Chip::DMSArrayChip> VisionMapDataEditorUI::GetDMSArrayData()
{
    m_pMapDataEditorDlg->LoadDMSDataDB(MAPDATA_ID_ARRAY);

    return m_pMapDataEditorDlg->m_vecArrayChipDB;
}

std::vector<Chip::DMSHeatsink> VisionMapDataEditorUI::GetDMSHeatSyncData()
{
    m_pMapDataEditorDlg->LoadDMSDataDB(MAPDATA_ID_HEATSYNC);

    return m_pMapDataEditorDlg->m_vecHeatsink;
}