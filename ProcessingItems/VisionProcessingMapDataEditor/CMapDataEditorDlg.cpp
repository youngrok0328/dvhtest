//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "CMapDataEditorDlg.h"

//CPP_2_________________________________ This project's headers
#include "resource.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../SharedCommonUtilityModules/dPI_MsgDialog/SimpleMessage.h"
#include "../../SharedComponent/Persistence/IniHelper.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <regex>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define IDC_FRAME_PROPERTY_GRID 100

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNAMIC(CMapDataEditorDlg, CDialog)

CMapDataEditorDlg::CMapDataEditorDlg(CWnd* pParent /* = nullptr*/)
    : CDialog(IDD_DLG_MAPDATA_EDITOR, pParent)
    , m_propertyGrid(nullptr)
    , m_pPackageSpec(NULL)
    , m_pDmsChipInfo(NULL)
    , m_pDmsPassiveInfo(NULL)
    , m_pDmsArrayInfo(NULL)
    , m_pDmsHeatsink(NULL)
    , m_pDmsPatch(NULL) //kircheis_POI
    , m_pDmsBallInfo(NULL)
    , m_nChipType(MAPDATA_ID_CHIP)
    , m_bIsImportMode(FALSE)
{
}

CMapDataEditorDlg::~CMapDataEditorDlg()
{
    delete m_propertyGrid;
}

void CMapDataEditorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_DMSDATABASE, m_ListRegisteredDMSData);
}

BEGIN_MESSAGE_MAP(CMapDataEditorDlg, CDialog)
ON_BN_CLICKED(IDC_BUTTON_REGISTER_DMSNAME, &CMapDataEditorDlg::OnBnClickedButtonRegisterDmsname)
ON_LBN_SELCHANGE(IDC_LIST_DMSDATABASE, &CMapDataEditorDlg::OnLbnSelchangeListDmsdatabase)
ON_MESSAGE(XTPWM_PROPERTYGRID_NOTIFY, OnGridNotify)
ON_COMMAND(ID_MENU_DELETE, &CMapDataEditorDlg::OnMenuDelete)
ON_COMMAND(ID_MENU_UPDATE, &CMapDataEditorDlg::OnMenuUpdate)
ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

// CMapDataEditorDlg 메시지 처리기
BOOL CMapDataEditorDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CRect rtCustomGrid;
    GetDlgItem(IDC_STATIC_GRIDVIEWER)->GetWindowRect(rtCustomGrid);
    ScreenToClient(rtCustomGrid);

    m_propertyGrid = new CXTPPropertyGrid;
    m_propertyGrid->Create(rtCustomGrid, this, IDC_FRAME_PROPERTY_GRID);
    m_propertyGrid->ShowHelp(FALSE);

    if (!m_bIsImportMode)
        GetDlgItem(IDC_BUTTON_REGISTER_DMSNAME)->SetWindowTextW(_T("Save"));

    if (m_vecCSVMapData.size() > 0)
    {
        m_nChipType = m_vecCSVMapData[0].GetCompType();

        ////너는 무조건 첫번째 데이터로 초기화가 되어야한다
        NewMapDataSet(m_vecCSVMapData[0]);

        SetDlgItemText(IDC_EDIT_REGISTER_DMSNAME, m_vecCSVMapData[0].strCompType); //새로운 Data가 존재할시에만
    }

    for (long nLoadIndex = 0; nLoadIndex < MAPDATA_ID_HEATSYNC; nLoadIndex++)
        LoadDMSDataDB(nLoadIndex);

    SetPropertyGrid();
    UpdateRegistedList();

    return TRUE; // return TRUE unless you set the focus to a control
        // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CMapDataEditorDlg::SetPackageSpecData(CPackageSpec* i_pPackageSpec, Chip::DMSChip* i_pDmsChipInfo,
    Chip::DMSPassiveChip* i_pDmsPassiveinfo, Chip::DMSArrayChip* i_pDMSArrayInfo, Chip::DMSHeatsink* i_pDMSHeatSyncInfo,
    Chip::DMSPatch* i_pDMSPatchInfo, Chip::DMSBall* i_pDmsBallInfo, std::vector<Package::Component> i_vecCSV_MapData,
    BOOL i_bIsImportMode)
{
    if (i_pPackageSpec == NULL || i_pDmsPassiveinfo == NULL)
        return FALSE;

    m_pPackageSpec = i_pPackageSpec;

    m_pDmsChipInfo = i_pDmsChipInfo;
    m_pDmsPassiveInfo = i_pDmsPassiveinfo;
    m_pDmsArrayInfo = i_pDMSArrayInfo;
    m_pDmsHeatsink = i_pDMSHeatSyncInfo;
    m_pDmsPatch = i_pDMSPatchInfo; //kircheis_POI
    m_pDmsBallInfo = i_pDmsBallInfo;
    m_vecCSVMapData = i_vecCSV_MapData;
    m_bIsImportMode = i_bIsImportMode;

    return TRUE;
}

BOOL CMapDataEditorDlg::NewMapDataSet(Package::Component i_sMapData)
{
    //MapData로 얻을수 있는 기본적인 Data Set
    long nCurCompType = i_sMapData.GetCompType();

    switch (nCurCompType)
    {
        case _typeChip:
        {
            m_pDmsChipInfo->strCompType = i_sMapData.strCompType;
            m_pDmsChipInfo->fChipWidth = i_sMapData.fWidth;
            m_pDmsChipInfo->fChipLength = i_sMapData.fLength;
            m_pDmsChipInfo->fChipAngle = (float)i_sMapData.nAngle;
            m_pDmsChipInfo->fChipThickness = i_sMapData.fHeight;
            break;
        }
        case _typePassive:
        {
            m_pDmsPassiveInfo->strCompType = i_sMapData.strCompType;
            m_pDmsPassiveInfo->fChipWidth = i_sMapData.fWidth;
            m_pDmsPassiveInfo->fChipLength = i_sMapData.fLength;
            m_pDmsPassiveInfo->fChipAngle = (float)i_sMapData.nAngle;
            m_pDmsPassiveInfo->fElectThickness = i_sMapData.fHeight;
            m_pDmsPassiveInfo->fChipThickness = i_sMapData.fHeight;
            break;
        }
        case _typeArray:
        {
            m_pDmsArrayInfo->strCompType = i_sMapData.strCompType;
            m_pDmsArrayInfo->fChipWidth = i_sMapData.fWidth;
            m_pDmsArrayInfo->fChipLength = i_sMapData.fLength;
            m_pDmsArrayInfo->fChipAngle = (float)i_sMapData.nAngle;
            m_pDmsArrayInfo->fChipThickness = i_sMapData.fHeight;
            break;
        }
        case _typeHeatSync:
        {
            m_pDmsHeatsink->strCompType = i_sMapData.strCompType;
            m_pDmsHeatsink->fChipWidth = i_sMapData.fWidth;
            m_pDmsHeatsink->fChipLength = i_sMapData.fLength;
            m_pDmsHeatsink->fChipAngle = (float)i_sMapData.nAngle;
            m_pDmsHeatsink->fChipThickness = i_sMapData.fHeight;
            break;
        }
        case _typePatch: //kircheis_POI
        {
            m_pDmsPatch->strCompType = i_sMapData.strCompType;
            m_pDmsPatch->fChipWidth = i_sMapData.fWidth;
            m_pDmsPatch->fChipLength = i_sMapData.fLength;
            m_pDmsPatch->fChipAngle = (float)i_sMapData.nAngle;
            m_pDmsPatch->fChipThickness = i_sMapData.fHeight;
            break;
        }
        default:
            break;
    }

    return TRUE;
}

void CMapDataEditorDlg::SetPropertyGrid()
{
    m_propertyGrid->ResetContent();

    if (auto* category = m_propertyGrid->AddCategory(_T("DMS Data Editor")))
    {
        if (auto* Item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("DMS Type"), m_nChipType, (int*)&m_nChipType)))
        {
            Item->GetConstraints()->AddConstraint(_T("Chip/Die"), MAPDATA_ID_CHIP);
            Item->GetConstraints()->AddConstraint(_T("Passive"), MAPDATA_ID_PASSIVE);
            Item->GetConstraints()->AddConstraint(_T("Array"), MAPDATA_ID_ARRAY);
            Item->GetConstraints()->AddConstraint(_T("HeatSync"), MAPDATA_ID_HEATSYNC);
            Item->GetConstraints()->AddConstraint(_T("Patch"), MAPDATA_ID_PATCH); //kircheis_POI
        }
        category->Expand();
    }

    switch (m_nChipType)
    {
        case MAPDATA_ID_CHIP:
            SetChipDMSGrid();
            break;
        case MAPDATA_ID_PASSIVE:
            SetPassiveDMSGrid();
            break;
        case MAPDATA_ID_ARRAY:
            SetArrayDMSGrid();
            break;
        case MAPDATA_ID_HEATSYNC:
            SetHeatSyncDMSGrid();
            break;
        case MAPDATA_ID_PATCH:
            SetPatchSyncDMSGrid(); //kircheis_POI
            break;
        default:
            break;
    }

    m_propertyGrid->SetViewDivider(0.50);
    m_propertyGrid->HighlightChangedItems(TRUE);
    m_propertyGrid->Refresh();
}

void CMapDataEditorDlg::SetChipDMSGrid()
{
    if (auto* category = m_propertyGrid->AddCategory(_T("Chip/Die DMS Information")))
    {
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Die Width"), m_pDmsChipInfo->fChipWidth, _T("%.2f mm"))))
            Item->SetID(Die_ID_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Die Length"), m_pDmsChipInfo->fChipLength, _T("%.2f mm"))))
            Item->SetID(Die_ID_LENGTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Die Thickness"), m_pDmsChipInfo->fChipThickness, _T("%.2f mm"))))
            Item->SetID(Die_ID_THICKNESS);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Die Angle"), m_pDmsChipInfo->fChipAngle, _T("%.2f"))))
            Item->SetID(Die_ID_ANGLE);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Underfill Width"), m_pDmsChipInfo->fUnderfillWidth, _T("%.2f mm"))))
            Item->SetID(Die_ID_UNDERFILLWIDTH);
        if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Underfill Length"), m_pDmsChipInfo->fUnderfillLength, _T("%.2f mm"))))
            Item->SetID(Die_ID_UNDERFILLLENGTH);

        if (!m_bIsImportMode) //mc_Change Mode면 공용 정보는 변경하지 못하게 한다
        {
            for (long nItemIndex = Die_ID_WIDTH; nItemIndex <= Die_ID_ANGLE; nItemIndex++)
                m_propertyGrid->FindItem(nItemIndex)->SetReadOnly(TRUE);
        }

        category->Expand();
    }
}

void CMapDataEditorDlg::SetPassiveDMSGrid()
{
    if (auto* category = m_propertyGrid->AddCategory(_T("Passive DMS Information")))
    {
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Passive Width"), m_pDmsPassiveInfo->fChipWidth, _T("%.2f mm"))))
            Item->SetID(Passive_ID_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Passive Length"), m_pDmsPassiveInfo->fChipLength, _T("%.2f mm"))))
            Item->SetID(Passive_ID_LENGTH);
        if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Passive Thickness"), m_pDmsPassiveInfo->fChipThickness, _T("%.2f mm"))))
            Item->SetID(Passive_ID_THICKNESS);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Passive Angle"), m_pDmsPassiveInfo->fChipAngle, _T("%.2f"))))
            Item->SetID(Passive_ID_ANGLE);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Electrode Width"), m_pDmsPassiveInfo->fElectWidth, _T("%.2f mm"))))
            Item->SetID(Passive_ID_ELECT_WIDTH);
        if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Electrode Thickness"), m_pDmsPassiveInfo->fElectThickness, _T("%.2f mm"))))
            Item->SetID(Passive_ID_ELECT_THICKNESS);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Pad Width"), m_pDmsPassiveInfo->fChipPAD_Width, _T("%.2f mm"))))
            Item->SetID(Passive_ID_PAD_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Pad Length"), m_pDmsPassiveInfo->fChipPAD_Length, _T("%.2f mm"))))
            Item->SetID(Passive_ID_PAD_LENGTH);
        if (auto* Item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Passive Type"), m_pDmsPassiveInfo->nPassiveType)))
        {
            Item->GetConstraints()->AddConstraint(_T("Capacitor"), PassiveType_Capacitor);
            Item->GetConstraints()->AddConstraint(_T("Resitor"), PassiveType_Resitor);
            Item->GetConstraints()->AddConstraint(_T("Tantalum"), PassiveType_Tantalum);
            Item->GetConstraints()->AddConstraint(_T("LandSideCapacitor"), PassiveType_LandSideCapacitor);

            Item->SetID(Passive_ID_TYPE);
        }

        if (!m_bIsImportMode) //mc_Change Mode면 공용 정보는 변경하지 못하게 한다
        {
            for (long nItemIndex = Passive_ID_WIDTH; nItemIndex <= Passive_ID_ANGLE; nItemIndex++)
                m_propertyGrid->FindItem(nItemIndex)->SetReadOnly(TRUE);
        }

        category->Expand();
    }
}

void CMapDataEditorDlg::SetArrayDMSGrid()
{
    if (auto* category = m_propertyGrid->AddCategory(_T("Array DMS Information")))
    {
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Array Width"), m_pDmsArrayInfo->fChipWidth, _T("%.2f mm"))))
            Item->SetID(Array_ID_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Array Length"), m_pDmsArrayInfo->fChipLength, _T("%.2f mm"))))
            Item->SetID(Array_ID_LENGTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Array Thickness"), m_pDmsArrayInfo->fChipThickness, _T("%.2f mm"))))
            Item->SetID(Array_ID_THICKNESS);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Array Angle"), m_pDmsArrayInfo->fChipAngle, _T("%.2f"))))
            Item->SetID(Array_ID_ANGLE);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Electrode Width"), m_pDmsArrayInfo->fElectWidth, _T("%.2f mm"))))
            Item->SetID(Array_ID_ELECT_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Electrode Width"), m_pDmsArrayInfo->fElectLength, _T("%.2f mm"))))
            Item->SetID(Array_ID_ELECT_LENGTH);
        if (auto* Item = category->AddChildItem(new CXTPPropertyGridItemDouble(
                _T("Electrode Thickness"), m_pDmsArrayInfo->fElectThickness, _T("%.2f mm"))))
            Item->SetID(Array_ID_ELECT_THICKNESS);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Electrode Pitch"), m_pDmsArrayInfo->fElectPitch, _T("%.2f mm"))))
            Item->SetID(Array_ID_ELECT_PITCH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Electrode Tip"), m_pDmsArrayInfo->fElectTip, _T("%.2f mm"))))
            Item->SetID(Array_ID_ELECT_TIP);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Pad Width"), m_pDmsArrayInfo->fPadSizeWidth, _T("%.2f mm"))))
            Item->SetID(Array_ID_PAD_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Pad Length"), m_pDmsArrayInfo->fPadSizeLength, _T("%.2f mm"))))
            Item->SetID(Array_ID_PAD_LENGTH);
        if (auto* Item
            = category->AddChildItem(new CXTPPropertyGridItemEnum(_T("Passive Type"), m_pDmsArrayInfo->nArrayType)))
        {
            Item->GetConstraints()->AddConstraint(_T("??"), 0); //mc_20180626 뭐가 존재할지 모른다 추후에 수정예정
            Item->SetID(Array_ID_TYPE);
        }

        if (!m_bIsImportMode) //mc_Change Mode면 공용 정보는 변경하지 못하게 한다
        {
            for (long nItemIndex = Array_ID_WIDTH; nItemIndex <= Array_ID_ANGLE; nItemIndex++)
                m_propertyGrid->FindItem(nItemIndex)->SetReadOnly(TRUE);
        }

        category->Expand();
    }
}

void CMapDataEditorDlg::SetHeatSyncDMSGrid()
{
    if (auto* category = m_propertyGrid->AddCategory(_T("HeatSync DMS Information")))
    {
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("HeatSync Width"), m_pDmsHeatsink->fChipWidth, _T("%.2f mm"))))
            Item->SetID(HeatSync_ID_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("HeatSync Length"), m_pDmsHeatsink->fChipLength, _T("%.2f mm"))))
            Item->SetID(HeatSync_ID_LENGTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("HeatSync Angle"), m_pDmsHeatsink->fChipAngle, _T("%.2f"))))
            Item->SetID(HeatSync_ID_ANGLE);

        if (!m_bIsImportMode) //mc_Change Mode면 공용 정보는 변경하지 못하게 한다
        {
            for (long nItemIndex = HeatSync_ID_WIDTH; nItemIndex <= HeatSync_ID_ANGLE; nItemIndex++)
                m_propertyGrid->FindItem(nItemIndex)->SetReadOnly(TRUE);
        }

        category->Expand();
    }
}

void CMapDataEditorDlg::SetPatchSyncDMSGrid() //kircheis_POI
{
    if (auto* category = m_propertyGrid->AddCategory(_T("Patch DMS Information")))
    {
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Patch Width"), m_pDmsPatch->fChipWidth, _T("%.2f mm"))))
            Item->SetID(Patch_ID_WIDTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Patch Length"), m_pDmsPatch->fChipLength, _T("%.2f mm"))))
            Item->SetID(Patch_ID_LENGTH);
        if (auto* Item = category->AddChildItem(
                new CXTPPropertyGridItemDouble(_T("Patch Height"), m_pDmsPatch->fChipThickness, _T("%.2f mm"))))
            Item->SetID(Patch_ID_HEIGHT);

        if (!m_bIsImportMode) //mc_Change Mode면 공용 정보는 변경하지 못하게 한다
        {
            for (long nItemIndex = Patch_ID_WIDTH; nItemIndex <= Patch_ID_HEIGHT; nItemIndex++)
                m_propertyGrid->FindItem(nItemIndex)->SetReadOnly(TRUE);
        }

        category->Expand();
    }
}

void CMapDataEditorDlg::UpdateRegistedList() //ListUpdate
{
    m_ListRegisteredDMSData.ResetContent();

    if (m_nChipType == MAPDATA_ID_CHIP)
    {
        long nChipListNum = (long)m_vecChipDB.size();
        for (long nList = 0; nList < nChipListNum; nList++)
        {
            m_ListRegisteredDMSData.AddString(m_vecChipDB[nList].strCompType);
        }
    }

    else if (m_nChipType == MAPDATA_ID_PASSIVE)
    {
        long nChipListNum = (long)m_vecPassveChipDB.size();
        for (long nList = 0; nList < nChipListNum; nList++)
        {
            m_ListRegisteredDMSData.AddString(m_vecPassveChipDB[nList].strCompType);
        }
    }

    else if (m_nChipType == MAPDATA_ID_ARRAY)
    {
        long nChipListNum = (long)m_vecArrayChipDB.size();
        for (long nList = 0; nList < nChipListNum; nList++)
        {
            m_ListRegisteredDMSData.AddString(m_vecArrayChipDB[nList].strCompType);
        }
    }

    else if (m_nChipType == MAPDATA_ID_HEATSYNC)
    {
        long nChipListNum = (long)m_vecHeatsink.size();
        for (long nList = 0; nList < nChipListNum; nList++)
        {
            m_ListRegisteredDMSData.AddString(m_vecHeatsink[nList].strCompType);
        }
    }

    else if (m_nChipType == MAPDATA_ID_PATCH) //kircheis_POI
    {
        long nChipListNum = (long)m_vecPatch.size();
        for (long nList = 0; nList < nChipListNum; nList++)
        {
            m_ListRegisteredDMSData.AddString(m_vecPatch[nList].strCompType);
        }
    }

    else if (m_nChipType == MAPDATA_ID_BALL)
    {
        long nChipListNum = (long)m_vecBallDB.size();
        for (long nList = 0; nList < nChipListNum; nList++)
        {
            m_ListRegisteredDMSData.AddString(m_vecBallDB[nList].strCompType);
        }
    }
}

//{{Save Load 부분
void CMapDataEditorDlg::SaveDMSDataDB(long i_nChipType)
{
    if (i_nChipType < 0)
        return;

    switch (i_nChipType)
    {
        case MAPDATA_ID_CHIP:
        {
            CString strChipDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Chip.ini"));
            SaveChipDMSDB(strChipDB);
            break;
        }
        case MAPDATA_ID_PASSIVE:
        {
            CString strPassiveDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Passive.ini"));
            SavePassiveDMSDB(strPassiveDB);
            break;
        }
        case MAPDATA_ID_ARRAY:
        {
            CString strArrayDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Array.ini"));
            SaveArrayDMSDB(strArrayDB);
            break;
        }
        case MAPDATA_ID_HEATSYNC:
        {
            CString strArrayDB(DynamicSystemPath::get(DefineFolder::Config) + _T("HeatSync.ini"));
            SaveHeatSyncDMSDB(strArrayDB);
            break;
        }
        case MAPDATA_ID_PATCH: //kircheis_POI
        {
            CString strArrayDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Patch.ini"));
            SavePatchDMSDB(strArrayDB);
            break;
        }
        default:
            break;
    }
}

void CMapDataEditorDlg::LoadDMSDataDB(long i_nChipType)
{
    if (i_nChipType == MAPDATA_ID_CHIP)
    {
        CString strChipDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Chip.ini"));
        LoadChipDMSDB(strChipDB);
    }
    else if (i_nChipType == MAPDATA_ID_PASSIVE)
    {
        CString strPassiveDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Passive.ini"));
        LoadPassiveDMSDB(strPassiveDB);
    }
    else if (i_nChipType == MAPDATA_ID_ARRAY)
    {
        CString strArrayDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Array.ini"));
        LoadArrayDMSDB(strArrayDB);
    }

    else if (i_nChipType == MAPDATA_ID_HEATSYNC)
    {
        CString strHeatSyncDB(DynamicSystemPath::get(DefineFolder::Config) + _T("HeatSync.ini"));
        LoadHeatSyncDMSDB(strHeatSyncDB);
    }

    else if (i_nChipType == MAPDATA_ID_HEATSYNC) //kircheis_POI
    {
        CString strPatchDB(DynamicSystemPath::get(DefineFolder::Config) + _T("Patch.ini"));
        LoadPatchDMSDB(strPatchDB);
    }
}

void CMapDataEditorDlg::SaveChipDMSDB(CString i_strDBName)
{
    long nChipDBSize = (long)m_vecChipDB.size();

    if (nChipDBSize < 0)
    {
        ::AfxMessageBox(_T("Chip Data does Not Exist"));
        return;
    }

    IniHelper::SaveINT(i_strDBName, _T("Chip"), _T("Chip Type Num"), nChipDBSize);

    for (long nChipDBIndex = 0; nChipDBIndex < nChipDBSize; nChipDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("Chip_%d"), nChipDBIndex);
        IniHelper::SaveSTRING(i_strDBName, strSection, _T("Type Name"), m_vecChipDB[nChipDBIndex].strCompType);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Width"), m_vecChipDB[nChipDBIndex].fChipWidth);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Length"), m_vecChipDB[nChipDBIndex].fChipLength);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Angle"), m_vecChipDB[nChipDBIndex].fChipAngle);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Chip fChipThickness"), m_vecChipDB[nChipDBIndex].fChipThickness);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Underfill Width"), m_vecChipDB[nChipDBIndex].fUnderfillWidth);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Underfill Length"), m_vecChipDB[nChipDBIndex].fUnderfillLength);
    }
}

void CMapDataEditorDlg::SavePassiveDMSDB(CString i_strDBName)
{
    long nPassiveDBSize = (long)m_vecPassveChipDB.size();

    if (nPassiveDBSize < 0)
    {
        ::AfxMessageBox(_T("Passive Data does Not Exist"));
        return;
    }

    IniHelper::SaveINT(i_strDBName, _T("PassiveChip"), _T("Chip Type Num"), nPassiveDBSize);

    for (long nPassiveDBIndex = 0; nPassiveDBIndex < nPassiveDBSize; nPassiveDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("PassiveChip_%d"), nPassiveDBIndex);
        IniHelper::SaveSTRING(i_strDBName, strSection, _T("Type Name"), m_vecPassveChipDB[nPassiveDBIndex].strCompType);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Width"), m_vecPassveChipDB[nPassiveDBIndex].fChipWidth);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Chip Length"), m_vecPassveChipDB[nPassiveDBIndex].fChipLength);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Electrode Width"), m_vecPassveChipDB[nPassiveDBIndex].fElectWidth);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Chip Width with PAD"), m_vecPassveChipDB[nPassiveDBIndex].fChipPAD_Width);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Chip Length with PAD"), m_vecPassveChipDB[nPassiveDBIndex].fChipPAD_Length);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("fChipAngle"), m_vecPassveChipDB[nPassiveDBIndex].fChipAngle);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("fChipThickness"), m_vecPassveChipDB[nPassiveDBIndex].fChipThickness);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("fElectThickness"), m_vecPassveChipDB[nPassiveDBIndex].fElectThickness);
        IniHelper::SaveINT(
            i_strDBName, strSection, _T("nPassiveType"), m_vecPassveChipDB[nPassiveDBIndex].nPassiveType);
    }
}

void CMapDataEditorDlg::SaveArrayDMSDB(CString i_strDBName)
{
    long nArrayDBSize = (long)m_vecArrayChipDB.size();

    if (nArrayDBSize < 0)
    {
        ::AfxMessageBox(_T("Array Data does Not Exist"));
        return;
    }

    IniHelper::SaveINT(i_strDBName, _T("ArrayChip"), _T("Chip Type Num"), nArrayDBSize);

    for (long nArrayDBIndex = 0; nArrayDBIndex < nArrayDBSize; nArrayDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("ArrayChip_%d"), nArrayDBIndex);
        IniHelper::SaveSTRING(i_strDBName, strSection, _T("Type Name"), m_vecArrayChipDB[nArrayDBIndex].strCompType);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Width"), m_vecArrayChipDB[nArrayDBIndex].fChipWidth);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Length"), m_vecArrayChipDB[nArrayDBIndex].fChipLength);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Electrode Width"), m_vecArrayChipDB[nArrayDBIndex].fElectWidth);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Electrode Length"), m_vecArrayChipDB[nArrayDBIndex].fElectLength);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Electrode Pitch"), m_vecArrayChipDB[nArrayDBIndex].fElectPitch);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Electrode Tip"), m_vecArrayChipDB[nArrayDBIndex].fElectTip);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Pad Size Width"), m_vecArrayChipDB[nArrayDBIndex].fPadSizeWidth);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Pad Size Length"), m_vecArrayChipDB[nArrayDBIndex].fPadSizeLength);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Angle"), m_vecArrayChipDB[nArrayDBIndex].fChipAngle);
        IniHelper::SaveINT(i_strDBName, strSection, _T("Array Type"), m_vecArrayChipDB[nArrayDBIndex].nArrayType);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Chip fChipThickness"), m_vecArrayChipDB[nArrayDBIndex].fChipThickness);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Chip fElectThickness"), m_vecArrayChipDB[nArrayDBIndex].fElectThickness);
    }
}

void CMapDataEditorDlg::SaveHeatSyncDMSDB(CString i_strDBName)
{
    long nHeatSyncDBSize = (long)m_vecHeatsink.size();

    if (nHeatSyncDBSize < 0)
    {
        ::AfxMessageBox(_T("HeatSync Data does Not Exist"));
        return;
    }

    IniHelper::SaveINT(i_strDBName, _T("HeatSync"), _T("Chip Type Num"), nHeatSyncDBSize);

    for (long nHeatSyncDBIndex = 0; nHeatSyncDBIndex < nHeatSyncDBSize; nHeatSyncDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("HeatSync_%d"), nHeatSyncDBIndex);
        IniHelper::SaveSTRING(i_strDBName, strSection, _T("Type Name"), m_vecHeatsink[nHeatSyncDBIndex].strCompType);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Width"), m_vecHeatsink[nHeatSyncDBIndex].fChipWidth);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Length"), m_vecHeatsink[nHeatSyncDBIndex].fChipLength);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Chip Angle"), m_vecHeatsink[nHeatSyncDBIndex].fChipAngle);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Chip fChipThickness"), m_vecHeatsink[nHeatSyncDBIndex].fChipThickness);
    }
}

void CMapDataEditorDlg::SavePatchDMSDB(CString i_strDBName) //kircheis_POI
{
    long nPatchDBSize = (long)m_vecPatch.size();

    if (nPatchDBSize < 0)
    {
        ::AfxMessageBox(_T("Patch Data does Not Exist"));
        return;
    }

    IniHelper::SaveINT(i_strDBName, _T("Patch"), _T("Patch Num"), nPatchDBSize);

    for (long nPatchDBIndex = 0; nPatchDBIndex < nPatchDBSize; nPatchDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("Patch_%d"), nPatchDBIndex);
        IniHelper::SaveSTRING(i_strDBName, strSection, _T("Type Name"), m_vecPatch[nPatchDBIndex].strCompType);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Patch Width"), m_vecPatch[nPatchDBIndex].fChipWidth);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Patch Length"), m_vecPatch[nPatchDBIndex].fChipLength);
        IniHelper::SaveFLOAT(i_strDBName, strSection, _T("Patch Angle"), m_vecPatch[nPatchDBIndex].fChipAngle);
        IniHelper::SaveFLOAT(
            i_strDBName, strSection, _T("Patch ChipThickness"), m_vecPatch[nPatchDBIndex].fChipThickness);
    }
}

void CMapDataEditorDlg::LoadChipDMSDB(CString i_strDBName)
{
    long nChipTypeNum = IniHelper::LoadINT(i_strDBName, _T("Chip"), _T("Chip Type Num"), 0);

    m_vecChipDB.clear();
    m_vecChipDB.resize(nChipTypeNum);

    for (long nChipDBIndex = 0; nChipDBIndex < nChipTypeNum; nChipDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("Chip_%d"), nChipDBIndex);
        m_vecChipDB[nChipDBIndex].strCompType = IniHelper::LoadSTRING(i_strDBName, strSection, _T("Type Name"), _T(""));
        m_vecChipDB[nChipDBIndex].fChipWidth = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Width"), 0.5f);
        m_vecChipDB[nChipDBIndex].fChipLength = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Length"), 0.5f);
        m_vecChipDB[nChipDBIndex].fChipAngle = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Angle"), 0.8f);
        m_vecChipDB[nChipDBIndex].fChipThickness
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip fChipThickness"), 0.3f);
        m_vecChipDB[nChipDBIndex].fUnderfillWidth
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Underfill Width"), 0.8f);
        m_vecChipDB[nChipDBIndex].fUnderfillLength
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Underfill Length"), 0.8f);
    }
}

void CMapDataEditorDlg::LoadPassiveDMSDB(CString i_strDBName)
{
    long nPassiveTypeNum = IniHelper::LoadINT(i_strDBName, _T("PassiveChip"), _T("Chip Type Num"), 0);
    m_vecPassveChipDB.clear();
    m_vecPassveChipDB.resize(nPassiveTypeNum);

    for (long nPassiveDBIndex = 0; nPassiveDBIndex < nPassiveTypeNum; nPassiveDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("PassiveChip_%d"), nPassiveDBIndex);
        m_vecPassveChipDB[nPassiveDBIndex].strCompType
            = IniHelper::LoadSTRING(i_strDBName, strSection, _T("Type Name"), _T(""));
        m_vecPassveChipDB[nPassiveDBIndex].fChipWidth
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Width"), 0.3f);
        m_vecPassveChipDB[nPassiveDBIndex].fChipLength
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Length"), 0.6f);
        m_vecPassveChipDB[nPassiveDBIndex].fElectWidth
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Electrode Width"), 0.15f);
        m_vecPassveChipDB[nPassiveDBIndex].fChipPAD_Width
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Width with PAD"), 0.15f);
        m_vecPassveChipDB[nPassiveDBIndex].fChipPAD_Length
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Length with PAD"), 0.15f);
        m_vecPassveChipDB[nPassiveDBIndex].fChipAngle
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("fChipAngle"), 0);
        m_vecPassveChipDB[nPassiveDBIndex].fChipThickness
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("fChipThickness"), 0.3f);
        m_vecPassveChipDB[nPassiveDBIndex].fElectThickness = IniHelper::LoadFLOAT(
            i_strDBName, strSection, _T("fElectThickness"), m_vecPassveChipDB[nPassiveDBIndex].fChipThickness);
        m_vecPassveChipDB[nPassiveDBIndex].nPassiveType
            = IniHelper::LoadINT(i_strDBName, strSection, _T("nPassiveType"), 0);
        m_vecPassveChipDB[nPassiveDBIndex].fElectThickness
            = (float)max(m_vecPassveChipDB[nPassiveDBIndex].fElectThickness,
                m_vecPassveChipDB[nPassiveDBIndex].fChipThickness); //ElectThickness는 PassiveHeight보다 작을 수 없다
    }
}

void CMapDataEditorDlg::LoadArrayDMSDB(CString i_strDBName)
{
    long nArrayTypeNum = IniHelper::LoadINT(i_strDBName, _T("ArrayChip"), _T("Chip Type Num"), 0);
    m_vecArrayChipDB.clear();
    m_vecArrayChipDB.resize(nArrayTypeNum);

    for (long nArrayDBIndex = 0; nArrayDBIndex < nArrayTypeNum; nArrayDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("ArrayChip_%d"), nArrayDBIndex);
        m_vecArrayChipDB[nArrayDBIndex].strCompType
            = IniHelper::LoadSTRING(i_strDBName, strSection, _T("Type Name"), _T(""));
        m_vecArrayChipDB[nArrayDBIndex].fChipWidth
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Width"), 0.3f);
        m_vecArrayChipDB[nArrayDBIndex].fChipLength
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Length"), 0.6f);
        m_vecArrayChipDB[nArrayDBIndex].fElectWidth
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Electrode Width"), 0.05f);
        m_vecArrayChipDB[nArrayDBIndex].fElectLength
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Electrode Length"), 0.05f);
        m_vecArrayChipDB[nArrayDBIndex].fElectPitch
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Electrode Pitch"), 0.03f);
        m_vecArrayChipDB[nArrayDBIndex].fElectTip
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Electrode Tip"), 0.02f);
        m_vecArrayChipDB[nArrayDBIndex].fPadSizeWidth
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Pad Size Width"), 0.05f);
        m_vecArrayChipDB[nArrayDBIndex].fPadSizeLength
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Pad Size Length"), 0.05f);
        m_vecArrayChipDB[nArrayDBIndex].fChipAngle = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Angle"), 0);
        //m_vecArrayChipDB[nArrayDBIndex].nArrayType	= IniHelper::LoadINT(i_strDBName, strSection, _T("ElectNumber"), 1);
        m_vecArrayChipDB[nArrayDBIndex].nArrayType = IniHelper::LoadINT(i_strDBName, strSection, _T("Array Type"), 1);
        m_vecArrayChipDB[nArrayDBIndex].fChipThickness
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip fChipThickness"), 0.3f);
        m_vecArrayChipDB[nArrayDBIndex].fElectThickness
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip fElectThickness"), 0.3f);
    }
}

void CMapDataEditorDlg::LoadHeatSyncDMSDB(CString i_strDBName)
{
    long nHeatSyncTypeNum = IniHelper::LoadINT(i_strDBName, _T("HeatSync"), _T("Chip Type Num"), 0);
    m_vecHeatsink.clear();
    m_vecHeatsink.resize(nHeatSyncTypeNum);

    for (long nHeatSyncDBIndex = 0; nHeatSyncDBIndex < nHeatSyncTypeNum; nHeatSyncDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("HeatSync_%d"), nHeatSyncDBIndex);
        m_vecHeatsink[nHeatSyncDBIndex].strCompType
            = IniHelper::LoadSTRING(i_strDBName, strSection, _T("Type Name"), _T(""));
        m_vecHeatsink[nHeatSyncDBIndex].fChipWidth
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Width"), 0.3f);
        m_vecHeatsink[nHeatSyncDBIndex].fChipLength
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Length"), 0.6f);
        m_vecHeatsink[nHeatSyncDBIndex].fChipAngle = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip Angle"), 0);
        m_vecHeatsink[nHeatSyncDBIndex].fChipThickness
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Chip fChipThickness"), 0.3f);
    }
}

void CMapDataEditorDlg::LoadPatchDMSDB(CString i_strDBName) //kircheis_POI
{
    long nPatchTypeNum = IniHelper::LoadINT(i_strDBName, _T("Patch"), _T("Patch Num"), 0);
    m_vecPatch.clear();
    m_vecPatch.resize(nPatchTypeNum);

    for (long nPatchDBIndex = 0; nPatchDBIndex < nPatchTypeNum; nPatchDBIndex++)
    {
        CString strSection;
        strSection.Format(_T("Patch_%d"), nPatchDBIndex);
        m_vecPatch[nPatchDBIndex].strCompType = IniHelper::LoadSTRING(i_strDBName, strSection, _T("Type Name"), _T(""));
        m_vecPatch[nPatchDBIndex].fChipWidth = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Patch Width"), 0.3f);
        m_vecPatch[nPatchDBIndex].fChipLength = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Patch Length"), 0.6f);
        m_vecPatch[nPatchDBIndex].fChipAngle = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Patch Angle"), 0);
        m_vecPatch[nPatchDBIndex].fChipThickness
            = IniHelper::LoadFLOAT(i_strDBName, strSection, _T("Patch ChipThickness"), 0.3f);
    }
}
//}}

INT_PTR CMapDataEditorDlg::DoModal()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
    return CDialog::DoModal();
}

BOOL CMapDataEditorDlg::DestroyWindow()
{
    return CDialog::DestroyWindow();
}

void CMapDataEditorDlg::OnBnClickedButtonRegisterDmsname()
{
    CString strSaveCompType;
    GetDlgItemText(IDC_EDIT_REGISTER_DMSNAME, strSaveCompType);

    if (m_bIsImportMode) //mc_ImportMapData일때만
    {
        if (!ModifyRegisterCompType(
                m_nChipType, strSaveCompType)) //ChipDataDB에 CompType이 존재하고, User가 추가시, 방지용
            return;
    }

    if ((long)m_vecCSVMapData.size() > 0) //여기서 DataSet 한다
    {
        NewMapDataSet(m_vecCSVMapData[0]);
        m_vecCSVMapData.erase(m_vecCSVMapData.begin());
    }
    else
    {
        CString strLastChar = strSaveCompType.Right(1);
        std::regex pattern("[^0-9]");
        //std::string compType = CT2A(strLastChar);
        CT2A asciiStr(strLastChar);
        std::string compType(asciiStr);
        std::smatch m;

        if (std::regex_match(compType, m, pattern))
        {
            if (SimpleMessage(_T("Dimension name format is not matching. \n\r Do you want to register? \r\n Example ")
                              _T("of dimension name : \r\n chip type+length+width_height_angle (C1005_05_090)\r\n"),
                    MB_OKCANCEL)
                == IDOK)
            {
                switch (m_nChipType)
                {
                    case MAPDATA_ID_CHIP:
                        m_pDmsChipInfo->strCompType = strSaveCompType;
                        break;
                    case MAPDATA_ID_PASSIVE:
                        m_pDmsPassiveInfo->strCompType = strSaveCompType;
                        break;
                    case MAPDATA_ID_ARRAY:
                        m_pDmsArrayInfo->strCompType = strSaveCompType;
                        break;
                    case MAPDATA_ID_HEATSYNC:
                        m_pDmsHeatsink->strCompType = strSaveCompType;
                        break;
                    case MAPDATA_ID_PATCH:
                        m_pDmsPatch->strCompType = strSaveCompType;
                        break;
                }
            }
            else
                return;
        }
    }
    if (m_bIsImportMode) //mc_ImportMapData일때만
        PushChipDBData(m_nChipType);
    else //Change DMS일때..
        SetChangeDMSData(m_nChipType);

    SaveDMSDataDB(m_nChipType);
    LoadDMSDataDB(m_nChipType);

    if (m_bIsImportMode)
        UpdateRegistedList(); //Push하고 List Update를 해야한다

    if ((long)m_vecCSVMapData.size() > 0)
    {
        NewMapDataSet(m_vecCSVMapData[0]);
        SetDlgItemText(IDC_EDIT_REGISTER_DMSNAME, m_vecCSVMapData[0].strCompType);
    }
}

BOOL CMapDataEditorDlg::ModifyRegisterCompType(long i_nChipType, CString i_strSaveCompType)
{
    switch (i_nChipType)
    {
        case MAPDATA_ID_CHIP:
            for (auto ChipDBCompType : m_vecChipDB)
            {
                if (ChipDBCompType.strCompType == i_strSaveCompType)
                    return FALSE;
            }
            break;
        case MAPDATA_ID_PASSIVE:
            for (auto PassiveDBCompType : m_vecPassveChipDB)
            {
                if (PassiveDBCompType.strCompType == i_strSaveCompType)
                    return FALSE;
            }
            break;
        case MAPDATA_ID_ARRAY:
            for (auto ArraryDBCompType : m_vecArrayChipDB)
            {
                if (ArraryDBCompType.strCompType == i_strSaveCompType)
                    return FALSE;
            }
            break;
        case MAPDATA_ID_HEATSYNC:
            for (auto HeatSyncDBCompType : m_vecHeatsink)
            {
                if (HeatSyncDBCompType.strCompType == i_strSaveCompType)
                    return FALSE;
            }
            break;
        case MAPDATA_ID_PATCH: //kircheis_POI
            for (auto patchDBCompType : m_vecPatch)
            {
                if (patchDBCompType.strCompType == i_strSaveCompType)
                    return FALSE;
            }
            break;
        default:
            break;
    }

    return TRUE;
}

void CMapDataEditorDlg::SetChangeDMSData(long i_nChipType)
{
    UNREFERENCED_PARAMETER(i_nChipType);

    long nSelectIndex = m_ListRegisteredDMSData.GetCurSel();
    if (nSelectIndex < 0)
        return;

    switch (m_nChipType)
    {
        case MAPDATA_ID_CHIP:
            m_vecChipDB[nSelectIndex] = *m_pDmsChipInfo;
            break;
        case MAPDATA_ID_PASSIVE:
            m_vecPassveChipDB[nSelectIndex] = *m_pDmsPassiveInfo;
            break;
        case MAPDATA_ID_ARRAY:
            m_vecArrayChipDB[nSelectIndex] = *m_pDmsArrayInfo;
            break;
        case MAPDATA_ID_HEATSYNC:
            m_vecHeatsink[nSelectIndex] = *m_pDmsHeatsink;
            break;
        case MAPDATA_ID_PATCH: //kircheis_POI
            m_vecPatch[nSelectIndex] = *m_pDmsPatch;
            break;
    }
    //
}

void CMapDataEditorDlg::PushChipDBData(long i_nChipType)
{
    if (i_nChipType < 0)
        return;

    switch (i_nChipType)
    {
        case MAPDATA_ID_CHIP:
            m_vecChipDB.push_back(*m_pDmsChipInfo);
            break;
        case MAPDATA_ID_PASSIVE:
            m_vecPassveChipDB.push_back(*m_pDmsPassiveInfo);
            break;
        case MAPDATA_ID_ARRAY:
            m_vecArrayChipDB.push_back(*m_pDmsArrayInfo);
            break;
        case MAPDATA_ID_HEATSYNC:
            m_vecHeatsink.push_back(*m_pDmsHeatsink);
            break;
        case MAPDATA_ID_PATCH: //kircheis_POI
            m_vecPatch.push_back(*m_pDmsPatch);
            break;
        default:
            break;
    }
}

void CMapDataEditorDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CMenu MenuItem;
    if (!MenuItem.LoadMenu(IDR_MENU_EDITEVENT))
        return;
    CMenu* pMenuSub = MenuItem.GetSubMenu(0);
    pMenuSub->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN | TPM_NONOTIFY, point.x, point.y, this);
}

void CMapDataEditorDlg::OnMenuDelete()
{
    long nChipListNum(0);
    long nSel = m_ListRegisteredDMSData.GetCurSel();

    if (m_nChipType == MAPDATA_ID_CHIP)
    {
        nChipListNum = (long)m_vecChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        m_vecChipDB.erase(m_vecChipDB.begin() + nSel);
    }

    else if (m_nChipType == MAPDATA_ID_PASSIVE)
    {
        nChipListNum = (long)m_vecPassveChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        m_vecPassveChipDB.erase(m_vecPassveChipDB.begin() + nSel);
    }

    else if (m_nChipType == MAPDATA_ID_ARRAY)
    {
        nChipListNum = (long)m_vecArrayChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        m_vecArrayChipDB.erase(m_vecArrayChipDB.begin() + nSel);
    }

    else if (m_nChipType == MAPDATA_ID_HEATSYNC)
    {
        nChipListNum = (long)m_vecHeatsink.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        m_vecHeatsink.erase(m_vecHeatsink.begin() + nSel);
    }

    else if (m_nChipType == MAPDATA_ID_PATCH) //kircheis_POI
    {
        nChipListNum = (long)m_vecPatch.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        m_vecPatch.erase(m_vecPatch.begin() + nSel);
    }

    else if (m_nChipType == MAPDATA_ID_BALL)
    {
        nChipListNum = (long)m_vecBallDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        m_vecBallDB.erase(m_vecBallDB.begin() + nSel);
        //SaveDmsBallDB();
    }

    SaveDMSDataDB(m_nChipType);
    UpdateRegistedList();
}

void CMapDataEditorDlg::OnMenuUpdate()
{
    long nChipListNum(0);
    long nSel = m_ListRegisteredDMSData.GetCurSel();

    if (m_nChipType == MAPDATA_ID_CHIP)
    {
        nChipListNum = (long)m_vecChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        if (m_pDmsChipInfo->strCompType != m_vecChipDB[nSel].strCompType)
            return;

        m_vecChipDB[nSel] = *m_pDmsChipInfo;
    }

    else if (m_nChipType == MAPDATA_ID_PASSIVE)
    {
        nChipListNum = (long)m_vecPassveChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        if (m_pDmsPassiveInfo->strCompType != m_vecPassveChipDB[nSel].strCompType)
            return;

        m_vecPassveChipDB[nSel] = *m_pDmsPassiveInfo;
    }

    else if (m_nChipType == MAPDATA_ID_ARRAY)
    {
        nChipListNum = (long)m_vecArrayChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        if (m_pDmsArrayInfo->strCompType != m_vecArrayChipDB[nSel].strCompType)
            return;

        m_vecArrayChipDB[nSel] = *m_pDmsArrayInfo;
    }

    else if (m_nChipType == MAPDATA_ID_HEATSYNC)
    {
        nChipListNum = (long)m_vecHeatsink.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        if (m_pDmsHeatsink->strCompType != m_vecHeatsink[nSel].strCompType)
            return;

        m_vecHeatsink[nSel] = *m_pDmsHeatsink;
    }

    else if (m_nChipType == MAPDATA_ID_PATCH) //kircheis_POI
    {
        nChipListNum = (long)m_vecPatch.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        if (m_pDmsPatch->strCompType != m_vecPatch[nSel].strCompType)
            return;

        m_vecPatch[nSel] = *m_pDmsPatch;
    }

    else if (m_nChipType == MAPDATA_ID_BALL)
    {
        nChipListNum = (long)m_vecBallDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        if (m_pDmsBallInfo->strCompType != m_vecBallDB[nSel].strCompType)
            return;

        m_vecBallDB[nSel] = *m_pDmsBallInfo;

        //SaveDmsBallDB();
    }

    SaveDMSDataDB(m_nChipType);

    UpdateRegistedList();
}

void CMapDataEditorDlg::OnLbnSelchangeListDmsdatabase()
{
    long nSel = m_ListRegisteredDMSData.GetCurSel();
    if (nSel < 0)
        return;

    long nChipListNum(0);
    CString strSelDMSName; //선택한놈이 뭔지는 알아야지
    if (m_nChipType == MAPDATA_ID_CHIP)
    {
        nChipListNum = (long)m_vecChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        *m_pDmsChipInfo = m_vecChipDB[nSel];
        strSelDMSName = m_pDmsChipInfo->strCompType;
    }

    else if (m_nChipType == MAPDATA_ID_PASSIVE)
    {
        nChipListNum = (long)m_vecPassveChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        *m_pDmsPassiveInfo = m_vecPassveChipDB[nSel];
        SetPassiveDMSGrid();
        strSelDMSName = m_pDmsPassiveInfo->strCompType;
    }

    else if (m_nChipType == MAPDATA_ID_ARRAY)
    {
        nChipListNum = (long)m_vecArrayChipDB.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        *m_pDmsArrayInfo = m_vecArrayChipDB[nSel];
        strSelDMSName = m_pDmsArrayInfo->strCompType;
    }

    else if (m_nChipType == MAPDATA_ID_HEATSYNC)
    {
        nChipListNum = (long)m_vecHeatsink.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        *m_pDmsHeatsink = m_vecHeatsink[nSel];
        strSelDMSName = m_pDmsHeatsink->strCompType;
    }

    else if (m_nChipType == MAPDATA_ID_HEATSYNC) //kircheis_POI
    {
        nChipListNum = (long)m_vecPatch.size();
        if (nSel < 0 || nSel >= nChipListNum)
            return;

        *m_pDmsPatch = m_vecPatch[nSel];
        strSelDMSName = m_pDmsPatch->strCompType;
    }

    SetPropertyGrid();

    SetDlgItemText(IDC_EDIT_REGISTER_DMSNAME, strSelDMSName);
}

LRESULT CMapDataEditorDlg::OnGridNotify(WPARAM wparam, LPARAM lparam)
{
    if (wparam != XTP_PGN_ITEMVALUE_CHANGED)
        return 0;

    CXTPPropertyGridItem* item = (CXTPPropertyGridItem*)lparam;

    if (auto* ComboValue = dynamic_cast<CXTPPropertyGridItemEnum*>(item))
    {
        int data = ComboValue->GetEnum();

        if (m_nChipType == MAPDATA_ID_PASSIVE)
        {
            switch (item->GetID())
            {
                case Passive_ID_TYPE:
                    m_pDmsPassiveInfo->nPassiveType = data;
                    break;
            }
        }
        else if (m_nChipType == MAPDATA_ID_ARRAY)
        {
            switch (item->GetID())
            {
                case Array_ID_TYPE:
                    m_pDmsArrayInfo->nArrayType = data;
                    break;
            }
        }

        SetPropertyGrid();
        UpdateRegistedList();
    }
    else if (auto* value = dynamic_cast<CXTPPropertyGridItemDouble*>(item))
    {
        double data = value->GetDouble();
        long nSelectDMSInfoIndex = m_ListRegisteredDMSData.GetCurSel();

        if (m_nChipType == MAPDATA_ID_CHIP)
        {
            switch (item->GetID())
            {
                case Die_ID_WIDTH:
                    m_pDmsChipInfo->fChipWidth = (float)data;
                    break;
                case Die_ID_LENGTH:
                    m_pDmsChipInfo->fChipLength = (float)data;
                    break;
                case Die_ID_THICKNESS:
                    m_pDmsChipInfo->fChipThickness = (float)data;
                    break;
                case Die_ID_ANGLE:
                    m_pDmsChipInfo->fChipAngle = (float)data;
                    break;
                case Die_ID_UNDERFILLWIDTH:
                    m_pDmsChipInfo->fUnderfillWidth = (float)data;
                    break;
                case Die_ID_UNDERFILLLENGTH:
                    m_pDmsChipInfo->fChipLength = (float)data;
                    break;
            }

            if (!m_bIsImportMode && nSelectDMSInfoIndex > 0)
                m_vecChipDB[nSelectDMSInfoIndex] = *m_pDmsChipInfo;
        }
        else if (m_nChipType == MAPDATA_ID_PASSIVE)
        {
            switch (item->GetID())
            {
                case Passive_ID_WIDTH:
                    m_pDmsPassiveInfo->fChipWidth = (float)data;
                    break;
                case Passive_ID_LENGTH:
                    m_pDmsPassiveInfo->fChipLength = (float)data;
                    break;
                case Passive_ID_THICKNESS:
                    m_pDmsPassiveInfo->fChipThickness = (float)data;
                    break;
                case Passive_ID_ANGLE:
                    m_pDmsPassiveInfo->fChipAngle = (float)data;
                    break;
                case Passive_ID_ELECT_WIDTH:
                    m_pDmsPassiveInfo->fElectWidth = (float)data;
                    break;
                case Passive_ID_ELECT_THICKNESS:
                    m_pDmsPassiveInfo->fElectThickness = (float)data;
                    break;
                case Passive_ID_PAD_WIDTH:
                    m_pDmsPassiveInfo->fChipPAD_Width = (float)data;
                    break;
                case Passive_ID_PAD_LENGTH:
                    m_pDmsPassiveInfo->fChipPAD_Length = (float)data;
                    break;
            }

            if (!m_bIsImportMode && nSelectDMSInfoIndex > 0)
                m_vecPassveChipDB[nSelectDMSInfoIndex] = *m_pDmsPassiveInfo;
        }
        else if (m_nChipType == MAPDATA_ID_ARRAY)
        {
            switch (item->GetID())
            {
                case Array_ID_WIDTH:
                    m_pDmsArrayInfo->fChipWidth = (float)data;
                    break;
                case Array_ID_LENGTH:
                    m_pDmsArrayInfo->fChipLength = (float)data;
                    break;
                case Array_ID_THICKNESS:
                    m_pDmsArrayInfo->fChipThickness = (float)data;
                    break;
                case Array_ID_ANGLE:
                    m_pDmsArrayInfo->fChipAngle = (float)data;
                    break;
                case Array_ID_ELECT_WIDTH:
                    m_pDmsArrayInfo->fElectWidth = (float)data;
                    break;
                case Array_ID_ELECT_LENGTH:
                    m_pDmsArrayInfo->fElectLength = (float)data;
                    break;
                case Array_ID_ELECT_THICKNESS:
                    m_pDmsArrayInfo->fElectThickness = (float)data;
                    break;
                case Array_ID_ELECT_PITCH:
                    m_pDmsArrayInfo->fElectPitch = (float)data;
                    break;
                case Array_ID_ELECT_TIP:
                    m_pDmsArrayInfo->fElectTip = (float)data;
                    break;
                case Array_ID_PAD_WIDTH:
                    m_pDmsArrayInfo->fPadSizeWidth = (float)data;
                    break;
                case Array_ID_PAD_LENGTH:
                    m_pDmsArrayInfo->fPadSizeLength = (float)data;
                    break;
            }

            if (!m_bIsImportMode && nSelectDMSInfoIndex > 0)
                m_vecArrayChipDB[nSelectDMSInfoIndex] = *m_pDmsArrayInfo;
        }
        else if (m_nChipType == MAPDATA_ID_HEATSYNC)
        {
            switch (item->GetID())
            {
                case HeatSync_ID_WIDTH:
                    m_pDmsHeatsink->fChipWidth = (float)data;
                    break;
                case HeatSync_ID_LENGTH:
                    m_pDmsHeatsink->fChipLength = (float)data;
                    break;
                case HeatSync_ID_ANGLE:
                    m_pDmsHeatsink->fChipAngle = (float)data;
                    break;
            }

            if (!m_bIsImportMode && nSelectDMSInfoIndex > 0)
                m_vecHeatsink[nSelectDMSInfoIndex] = *m_pDmsHeatsink;
        }
        else if (m_nChipType == MAPDATA_ID_PATCH) //kircheis_POI
        {
            switch (item->GetID())
            {
                case Patch_ID_WIDTH:
                    m_pDmsPatch->fChipWidth = (float)data;
                    break;
                case Patch_ID_LENGTH:
                    m_pDmsPatch->fChipLength = (float)data;
                    break;
                case Patch_ID_HEIGHT:
                    m_pDmsPatch->fChipThickness = (float)data;
                    break;
            }

            if (!m_bIsImportMode && nSelectDMSInfoIndex > 0)
                m_vecPatch[nSelectDMSInfoIndex] = *m_pDmsPatch;
        }
    }

    m_propertyGrid->Refresh();

    return 0;
}