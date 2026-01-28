//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "PackageSpec.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/VisionScale.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <regex>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
CPackageSpec::CPackageSpec()
    : m_bodyInfoMaster(new Package::BodyInfo)
    , m_ballMap(new Package::BallCollection)
    , m_ballMapAll(new Package::BallCollection)
    , m_originalballMap(new Package::BallCollectionOriginal)
    , m_LandMapManager(new Package::LandCollection)
    , m_LandMapConvertOrigin(new Package::LandCollection)
    , m_OriginLandData(new Package::LandCollectionOriginal)
    , m_PadMapManager(new Package::PadCollection)
    , m_OriginPadData(new Package::PadCollectionOriginal)
    , m_CompMapManager(new Package::ComponentCollection)
    , m_OriginCompData(new Package::ComponentCollectionOriginal)
    , m_CustomFixedMapManager(new Package::CustomFixedCollection)
    , m_OriginCustomFixedData(new Package::CustomFixedCollectionOriginal)
    , m_CustomPolygonMapManager(new Package::CustomPolygonCollection)
    , m_OriginCustomPolygonData(new Package::CustomPolygonCollectionOriginal)
{
    Init();
}

CPackageSpec::~CPackageSpec()
{
    delete m_bodyInfoMaster;
    delete m_ballMap;
    delete m_ballMapAll;
    delete m_originalballMap;
    delete m_LandMapManager;
    delete m_LandMapConvertOrigin;
    delete m_OriginLandData;
    delete m_PadMapManager;
    delete m_OriginPadData;
    delete m_CompMapManager;
    delete m_OriginCompData;
    delete m_CustomFixedMapManager;
    delete m_OriginCustomFixedData;
    delete m_CustomPolygonMapManager;
    delete m_OriginCustomPolygonData;
}

void CPackageSpec::Init()
{
    // Package Type
    nPackageType = enPackageType::UNKNOWN;
    nDeviceType = enDeviceType::PACKAGE_UNKNOWN;
    m_deadBug = FALSE;

    // Common Dimensions
    m_bodyInfoMaster->Init();

    // MapData 초가화 20180314
    m_originalballMap->m_balls.clear();
    m_originalballMap->m_ballTypes.clear();
    m_ballMap->m_balls.clear();
    m_ballMapAll->m_balls.clear();
    m_CompMapManager->vecCompData.clear();
    m_OriginCompData->vecOriginCompData.clear();
    m_PadMapManager->vecPadData.clear();
    m_OriginPadData->vecOriginPadData.clear();
    m_LandMapManager->vecLandData.clear();
    m_LandMapConvertOrigin->vecLandData.clear();
    m_OriginLandData->vecOriginLandData.clear();
    m_CustomFixedMapManager->vecCustomFixedData.clear();
    m_CustomPolygonMapManager->vecCustomPolygonData.clear();

    m_dependent_um2px_x = 0.f;
    m_dependent_um2px_y = 0.f;
}

void CPackageSpec::CalcDependentVariables(const VisionScale& scale)
{
    m_dependent_um2px_x = scale.umToPixel().m_x;
    m_dependent_um2px_y = scale.umToPixel().m_y;

    // 왼손좌표계를 이용하므로 -을 곱해준다
    m_ballMap->Setup(*m_originalballMap, m_dependent_um2px_x, -m_dependent_um2px_y, true);
    m_ballMapAll->Setup(*m_originalballMap, m_dependent_um2px_x, -m_dependent_um2px_y, false);
    m_originalballMap->GetAllGroupName(); //kircheis_MED2.5
}

bool CPackageSpec::IsSameDepedentScale(const VisionScale& scale) const
{
    if (m_dependent_um2px_x != scale.umToPixel().m_x)
        return false;
    if (m_dependent_um2px_y != scale.umToPixel().m_y)
        return false;

    return true;
}

void CPackageSpec::SetPackageType(enPackageType nNewPackageType)
{
    switch (nPackageType = nNewPackageType)
    {
        case enPackageType::TSOP_1:
        case enPackageType::ACCU_TSOP_1:
        case enPackageType::TSOP_2:
        case enPackageType::ACCU_TSOP_2:
        case enPackageType::QFP:
        case enPackageType::ACCU_QFP:
            nDeviceType = enDeviceType::PACKAGE_GULLWING;
            break;
        case enPackageType::BGA:
        case enPackageType::ACCU_BGA:
            nDeviceType = enDeviceType::PACKAGE_BALL;
            break;
        case enPackageType::LGA: //mc_2017. 09.11 Simple Land
            nDeviceType = enDeviceType::PACKAGE_LAND;
            break;
        case enPackageType::QFN:
        case enPackageType::CARD:
            nDeviceType = enDeviceType::PACKAGE_LEADLESS;
            break;
        case enPackageType::FC_BGA:
            nDeviceType = enDeviceType::PACKAGE_CHIP;
            break;
        default:
            ASSERT(!"No implementation for that package type.");
    }
}

// Save & Load...
BOOL CPackageSpec::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
        Init();

    long version = 1;

    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    if (!db[_T("Device_Type")].Link(bSave, *(long*)&nDeviceType))
        nDeviceType = enDeviceType::PACKAGE_UNKNOWN;
    if (!db[_T("Package_Type")].Link(bSave, *(long*)&nPackageType))
        nPackageType = enPackageType::UNKNOWN;
    if (!bSave
        && nPackageType
            == enPackageType::
                FC_BGA) //임시 Code. 기존 FC BGA로 만든 잡을 BGA로 컨버젼. 이제 Package Type은 BGA 아니면, LGA만 쓰는 걸로.. 20180110
        nPackageType = enPackageType::BGA;

    if (!db[_T("Dead_Bug")].Link(bSave, m_deadBug))
        m_deadBug = FALSE;

    BOOL bUseCoord = TRUE;

    //{{//kircheis_Land
    if (!db[_T("UseCoord")].Link(bSave, bUseCoord))
        bUseCoord = TRUE;
    //}}

    if (version < 1)
    {
        //{{mc_bug발견 HQ 수정예정
        // Component
        //if (bSave)
        //	SyncOriginCompData();
        //}}
        m_OriginCompData->LinkDataBase(bSave, db); //원본데이터를 Job에 저장
        if (!bSave)
        {
            SetCompData();
            CreateDMSInfo();
        }

        // Pad
        m_OriginPadData->LinkDataBase(bSave, db);
        if (!bSave)
        {
            SetPadData();
        }

        //Land
        m_OriginLandData->LinkDataBase(bSave, db);
        if (!bSave)
        {
            SetLandData();
        }

        //Custom Fixed
        m_OriginCustomFixedData->LinkDataBase(bSave, db);
        if (!bSave)
        {
            SetCustomFixedData();
        }

        //Custom Polygon
        m_OriginCustomPolygonData->LinkDataBase(bSave, db);
        if (!bSave)
        {
            SetCustomPolygonData();
        }
    }
    else
    {
        //{{mc_bug발견 HQ 수정예정
        // Component
        //if (bSave)
        //	SyncOriginCompData();
        //}}

        m_OriginCompData->LinkDataBase(
            bSave, db[_T("{FD089AEE-8B26-47A0-973A-A2F212AE4080}")]); //원본데이터를 Job에 저장
        if (!bSave)
        {
            SetCompData();
            CreateDMSInfo();
        }

        // Pad
        m_OriginPadData->LinkDataBase(bSave, db[_T("{401B75DA-036A-4046-BACA-6F973DC89979}")]);
        if (!bSave)
        {
            SetPadData();
        }

        //Land
        m_OriginLandData->LinkDataBase(bSave, db[_T("{BAE70268-B1D7-49A3-9733-5A7D9C3CCDA5}")]);
        if (!bSave)
        {
            SetLandData();
        }

        //Custom Fixed
        m_OriginCustomFixedData->LinkDataBase(bSave, db[_T("{9E6C8BC4-DDC5-4E7A-A881-951F3F90EDDD}")]);
        if (!bSave)
        {
            SetCustomFixedData();
        }

        //Custom Polygon
        m_OriginCustomPolygonData->LinkDataBase(bSave, db[_T("{0A5B8764-1354-4C17-BB18-F0F8B7E2268D}")]);
        if (!bSave)
        {
            SetCustomPolygonData();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // 여기서부터는 구조 정리를 시작하면서 추가되는 부분임.
    // 나중에 잡 구조가 안정되면 앞 부분은 모두 버리고, 버전 정보도 없앨 예정
    // 제대로 정리된 구조체에 대한 것만 여기에 넣을 것.
    // 이전 잡 호환성을 위해서 버전 체크도 진행함.
    // 이현민.

    // m_bodyInfoMaster
    {
        long bodyInfoVersion = 1;

        if (!db[_T("{7E371574-2B9E-4909-B5A4-2E306F0CABC7}")].Link(bSave, bodyInfoVersion))
            bodyInfoVersion = 0;

        if (bodyInfoVersion > 0)
        {
            m_bodyInfoMaster->LinkDataBase(bSave, db[_T("{47013B98-CF71-4CBB-B515-A3F79E9CEAFB}")]);
        }
        else
        {
            // Body Info Master...
            m_bodyInfoMaster->LinkDataBase_0(bSave, db);
        }
    }

    // m_originalballMap
    {
        long ballMapSourceVersion = 2;

        if (!db[_T("{3B98692F-BDC1-4383-892E-D4E04EB125AF}")].Link(bSave, ballMapSourceVersion))
            ballMapSourceVersion = 0;

        if (ballMapSourceVersion > 0)
        {
            m_originalballMap->LinkDataBase(bSave, db[_T("{A7B47DDD-9D0C-41D6-A21F-30B025CB3831}")]);
        }

        if (ballMapSourceVersion < 2)
        {
            // 이때는 Ball Map이 GroupID를 가지고 있지 않을 때이다.
            // 그냥 Diameter, Height를 이용해서 Groupping 해서 사용했을 때이므로
            // 이전잡과의 호환성을 위해 Group을 자동으로 만들어주자

            m_originalballMap->makeGroupID_usingShape();
            m_originalballMap->updateTypeInfo();
        }
    }

    return TRUE;
}

//mc_Component 관련
BOOL CPackageSpec::ClearDMSInfo()
{
    vecDMSChip.clear();
    vecDMSPassive.clear();
    vecDMSArray.clear();
    vecDMSHeatsink.clear();
    vecDMSPatch.clear(); //kircheis_POI

    return TRUE;
}

void CPackageSpec::CreateDMSInfo()
{
    ClearDMSInfo();

    //Size 반환
    long nSize = m_CompMapManager->GetCount();
    Package::ComponentCollection MapDataManager[_typeComponentEnd];

    for (long n = 0; n < nSize; n++)
    {
        Package::Component compMap = m_CompMapManager->GetMapData(n);
        long compType = compMap.GetCompType();

        if (compType < 0)
            continue;
        if (compMap.bIgnore)
            continue;

        MapDataManager[compType].AddData(compMap);
    }

    if (MapDataManager[_typeChip].GetCount() > 0)
    {
        PushDMSInfoChip(MapDataManager[_typeChip].vecCompData);
    }
    if (MapDataManager[_typePassive].GetCount() > 0)
    {
        PushDMSInfoPassive(MapDataManager[_typePassive].vecCompData);
    }
    if (MapDataManager[_typeArray].GetCount() > 0)
    {
        PushDMSInfoArray(MapDataManager[_typeArray].vecCompData);
    }
    if (MapDataManager[_typeHeatSync].GetCount() > 0)
    {
        PushDMSInfoHeatSync(MapDataManager[_typeHeatSync].vecCompData);
    }
    if (MapDataManager[_typePatch].GetCount() > 0) //kircheis_POI
    {
        PushDMSInfoPatch(MapDataManager[_typePatch].vecCompData);
    }
}

BOOL CPackageSpec::PushDMSInfoChip(std::vector<Package::Component> i_vecChipComponentMap)
{
    long nCompChipDataSize = (long)i_vecChipComponentMap.size();
    vecDMSChip.resize(nCompChipDataSize);

    for (long nIndex = 0; nIndex < (long)i_vecChipComponentMap.size(); nIndex++)
    {
        CString strPassiveCompType = i_vecChipComponentMap[nIndex].strCompType;
        SetDMS_ChipcData(strPassiveCompType, i_vecChipComponentMap[nIndex], nIndex);
    }

    return TRUE;
}

BOOL CPackageSpec::SetDMS_ChipcData(CString i_strComType, Package::Component i_ChipComponentMap, long i_nIndex)
{
    vecDMSChip[i_nIndex].strCompSpec = MakeCompSpecToString(i_ChipComponentMap.strCompType);
    vecDMSChip[i_nIndex].strCompID = i_ChipComponentMap.strCompName;
    vecDMSChip[i_nIndex].strCompType = i_ChipComponentMap.strCompType;
    vecDMSChip[i_nIndex].fChipOffsetX = i_ChipComponentMap.fOffsetX;
    vecDMSChip[i_nIndex].fChipOffsetY = i_ChipComponentMap.fOffsetY;
    vecDMSChip[i_nIndex].fChipWidth = i_ChipComponentMap.fWidth;
    vecDMSChip[i_nIndex].fChipLength = i_ChipComponentMap.fLength;
    vecDMSChip[i_nIndex].fChipAngle = static_cast<float>(i_ChipComponentMap.nAngle);
    vecDMSChip[i_nIndex].fChipThickness = i_ChipComponentMap.fHeight;
    //공통변수

    vecDMSChip[i_nIndex].nComponentType = eComponentTypeDefine(tyCHIP);
    vecDMSChip[i_nIndex].fUnderfillWidth = 0.;
    vecDMSChip[i_nIndex].fUnderfillLength = 0.;

    vecDMSChip[i_nIndex].frtChip = i_ChipComponentMap.fsrtRealROI.GetFRect();
    vecDMSChip[i_nIndex].sfrtChip = i_ChipComponentMap.fsrtRealROI;

    return TRUE;
}

BOOL CPackageSpec::PushDMSInfoPassive(std::vector<Package::Component> i_vecPassiveComponentMap)
{
    long nCompPassiveDataSize = (long)i_vecPassiveComponentMap.size();
    vecDMSPassive.resize(nCompPassiveDataSize);

    for (long nIndex = 0; nIndex < (long)i_vecPassiveComponentMap.size(); nIndex++)
    {
        CString strPassiveCompType = i_vecPassiveComponentMap[nIndex].strCompType;
        SetDMS_PassiveData(strPassiveCompType, i_vecPassiveComponentMap[nIndex], nIndex);
    }

    return TRUE;
}

BOOL CPackageSpec::SetDMS_PassiveData(CString i_strComType, Package::Component i_PassiveComponentMap, long i_nIndex)
{
    vecDMSPassive[i_nIndex].strCompSpec = MakeCompSpecToString(i_PassiveComponentMap.strCompType);
    vecDMSPassive[i_nIndex].strCompID = i_PassiveComponentMap.strCompName;
    vecDMSPassive[i_nIndex].strCompType = i_PassiveComponentMap.strCompType;
    vecDMSPassive[i_nIndex].fChipOffsetX = i_PassiveComponentMap.fOffsetX;
    vecDMSPassive[i_nIndex].fChipOffsetY = i_PassiveComponentMap.fOffsetY;
    vecDMSPassive[i_nIndex].fChipWidth = i_PassiveComponentMap.fWidth;
    vecDMSPassive[i_nIndex].fChipLength = i_PassiveComponentMap.fLength;
    vecDMSPassive[i_nIndex].fChipAngle = static_cast<float>(i_PassiveComponentMap.nAngle);
    vecDMSPassive[i_nIndex].fChipThickness = i_PassiveComponentMap.fHeight;
    //공통변수

    vecDMSPassive[i_nIndex].nComponentType = eComponentTypeDefine(tyPASSIVE);
    vecDMSPassive[i_nIndex].nPassiveType = enumPassiveTypeDefine(GetPassiveType(i_PassiveComponentMap.strCompCategory));
    vecDMSPassive[i_nIndex].fElectWidth = i_PassiveComponentMap.fPassiveElectLenght;
    vecDMSPassive[i_nIndex].fElectThickness = i_PassiveComponentMap.fPassiveElectHeight;
    vecDMSPassive[i_nIndex].fChipPAD_Gap = i_PassiveComponentMap.fPassivePadGap;
    vecDMSPassive[i_nIndex].fChipPAD_Width = i_PassiveComponentMap.fPassivePadWidth;
    vecDMSPassive[i_nIndex].fChipPAD_Length = i_PassiveComponentMap.fPassivePadLenght;

    vecDMSPassive[i_nIndex].fElectThickness = (float)max(vecDMSPassive[i_nIndex].fElectThickness,
        i_PassiveComponentMap.fHeight); //ElectThickness는 PassiveHeight보다 작을 수 없다

    vecDMSPassive[i_nIndex].frtChip = i_PassiveComponentMap.fsrtRealROI.GetFRect();
    vecDMSPassive[i_nIndex].sfrtChip = i_PassiveComponentMap.fsrtRealROI;

    return TRUE;
}

BOOL CPackageSpec::PushDMSInfoArray(std::vector<Package::Component> i_vecArrayComponentMap)
{
    long nCompArrayDataSize = (long)i_vecArrayComponentMap.size();
    vecDMSArray.resize(nCompArrayDataSize);

    for (long nIndex = 0; nIndex < (long)i_vecArrayComponentMap.size(); nIndex++)
    {
        CString strArrayCompType = i_vecArrayComponentMap[nIndex].strCompType;
        SetDMS_ArrayData(strArrayCompType, i_vecArrayComponentMap[nIndex], nIndex);
    }

    return TRUE;
}

BOOL CPackageSpec::SetDMS_ArrayData(CString i_strComType, Package::Component i_ArrayComponentMap, long i_nIndex)
{
    vecDMSArray[i_nIndex].strCompSpec = MakeCompSpecToString(i_ArrayComponentMap.strCompType);
    vecDMSArray[i_nIndex].strCompID = i_ArrayComponentMap.strCompName;
    vecDMSArray[i_nIndex].strCompType = i_ArrayComponentMap.strCompType;
    vecDMSArray[i_nIndex].fChipOffsetX = i_ArrayComponentMap.fOffsetX;
    vecDMSArray[i_nIndex].fChipOffsetY = i_ArrayComponentMap.fOffsetY;
    vecDMSArray[i_nIndex].fChipWidth = i_ArrayComponentMap.fWidth;
    vecDMSArray[i_nIndex].fChipLength = i_ArrayComponentMap.fLength;
    vecDMSArray[i_nIndex].fChipAngle = static_cast<float>(i_ArrayComponentMap.nAngle);
    vecDMSArray[i_nIndex].fChipThickness = i_ArrayComponentMap.fHeight;
    //공통변수

    vecDMSArray[i_nIndex].nComponentType = eComponentTypeDefine(tyARRAY);
    vecDMSArray[i_nIndex].fElectWidth = i_ArrayComponentMap.fPassiveElectLenght;
    vecDMSArray[i_nIndex].fElectLength = i_ArrayComponentMap.fPassiveElectLenght;
    vecDMSArray[i_nIndex].fElectTip = i_ArrayComponentMap.fPassivePadGap;
    vecDMSArray[i_nIndex].fPadSizeWidth = i_ArrayComponentMap.fPassivePadWidth;
    vecDMSArray[i_nIndex].fPadSizeLength = i_ArrayComponentMap.fPassivePadLenght;
    vecDMSArray[i_nIndex].fElectThickness = i_ArrayComponentMap.fPassiveElectHeight;

    return TRUE;
}

BOOL CPackageSpec::PushDMSInfoHeatSync(std::vector<Package::Component> i_vecHeatSynComponent)
{
    long nCompHeatSyncDataSize = (long)i_vecHeatSynComponent.size();
    vecDMSHeatsink.resize(nCompHeatSyncDataSize);

    for (long nIndex = 0; nIndex < (long)i_vecHeatSynComponent.size(); nIndex++)
    {
        CString strHeatSnycCompType = i_vecHeatSynComponent[nIndex].strCompType;
        SetDMS_HeatSyncData(strHeatSnycCompType, i_vecHeatSynComponent[nIndex], nIndex);
    }

    return TRUE;
}

BOOL CPackageSpec::SetDMS_HeatSyncData(CString i_strComType, Package::Component i_HeatSynComponent, long i_nIndex)
{
    vecDMSHeatsink[i_nIndex].strCompSpec = MakeCompSpecToString(i_HeatSynComponent.strCompType);
    vecDMSHeatsink[i_nIndex].strCompID = i_HeatSynComponent.strCompName;
    vecDMSHeatsink[i_nIndex].strCompType = i_HeatSynComponent.strCompType;
    vecDMSHeatsink[i_nIndex].fChipOffsetX = i_HeatSynComponent.fOffsetX;
    vecDMSHeatsink[i_nIndex].fChipOffsetY = i_HeatSynComponent.fOffsetY;
    vecDMSHeatsink[i_nIndex].fChipWidth = i_HeatSynComponent.fWidth;
    vecDMSHeatsink[i_nIndex].fChipLength = i_HeatSynComponent.fLength;
    vecDMSHeatsink[i_nIndex].fChipAngle = static_cast<float>(i_HeatSynComponent.nAngle);
    vecDMSHeatsink[i_nIndex].fChipThickness = i_HeatSynComponent.fHeight;
    //공통변수

    vecDMSHeatsink[i_nIndex].nComponentType = eComponentTypeDefine(tyHEATSINK);
    vecDMSHeatsink[i_nIndex].frtChip = i_HeatSynComponent.fsrtRealROI.GetFRect();
    vecDMSHeatsink[i_nIndex].sfrtChip = i_HeatSynComponent.fsrtRealROI;

    return TRUE;
}

BOOL CPackageSpec::PushDMSInfoPatch(std::vector<Package::Component> i_vecPatchComponentMap) //kircheis_POI
{
    long nCompPatchDataSize = (long)i_vecPatchComponentMap.size();
    vecDMSPatch.resize(nCompPatchDataSize);

    for (long nIndex = 0; nIndex < (long)i_vecPatchComponentMap.size(); nIndex++)
    {
        CString strHeatSnycCompType = i_vecPatchComponentMap[nIndex].strCompType;
        SetDMS_PatchData(strHeatSnycCompType, i_vecPatchComponentMap[nIndex], nIndex);
    }

    return TRUE;
}

BOOL CPackageSpec::SetDMS_PatchData(CString i_strComType, Package::Component i_PatchComponentMap, long i_nIndex)
{
    vecDMSPatch[i_nIndex].strCompSpec = MakeCompSpecToString(i_PatchComponentMap.strCompType);
    vecDMSPatch[i_nIndex].strCompID = i_PatchComponentMap.strCompName;
    vecDMSPatch[i_nIndex].strCompType = i_PatchComponentMap.strCompType;
    vecDMSPatch[i_nIndex].fChipOffsetX = i_PatchComponentMap.fOffsetX;
    vecDMSPatch[i_nIndex].fChipOffsetY = i_PatchComponentMap.fOffsetY;
    vecDMSPatch[i_nIndex].fChipWidth = i_PatchComponentMap.fWidth;
    vecDMSPatch[i_nIndex].fChipLength = i_PatchComponentMap.fLength;
    vecDMSPatch[i_nIndex].fChipAngle = static_cast<float>(i_PatchComponentMap.nAngle);
    vecDMSPatch[i_nIndex].fChipThickness = i_PatchComponentMap.fHeight;
    //공통변수

    vecDMSPatch[i_nIndex].frtChip = i_PatchComponentMap.fsrtRealROI.GetFRect();
    vecDMSPatch[i_nIndex].sfrtChip = i_PatchComponentMap.fsrtRealROI;

    return TRUE;
}

enumPassiveTypeDefine CPackageSpec::GetPassiveType(CString i_strCompCategory)
{
    enumPassiveTypeDefine nPassiveType = enumPassiveTypeDefine::euum_PassiveType_Unknown;

    CString strFindCompType = i_strCompCategory;
    strFindCompType = strFindCompType.MakeUpper();

    std::regex pattern("(^[a-zA-Z]{1,5})[0-9_]*[0-9a-zA-Z_]*[0-9.]*");
    //std::string compType = CT2A(strFindCompType);
    CT2A asciiStr(strFindCompType);
    std::string compType(asciiStr);
    std::smatch m;
    if (std::regex_match(compType, m, pattern))
    {
        std::string strMatch = m[1].str();

        if (strMatch == "C" || strMatch == "CAP" || strMatch == "CAPACITOR")
            nPassiveType = enumPassiveTypeDefine::enum_PassiveType_Capacitor;
        else if (strMatch == "R" || strMatch == "RES" || strMatch == "RESITOR")
            nPassiveType = enumPassiveTypeDefine::enum_PassiveType_Register;
        else if (strMatch == "M" || strMatch == "MIA")
            nPassiveType = enumPassiveTypeDefine::enum_PassiveType_MIA;
        else
            nPassiveType = enumPassiveTypeDefine::euum_PassiveType_Unknown;
    }
    else
    {
        nPassiveType = enumPassiveTypeDefine::euum_PassiveType_Unknown;
    }

    return nPassiveType;
}

BOOL CPackageSpec::DeleteCompMapData(long i_nIndexID)
{
    if (i_nIndexID <= 0 || m_CompMapManager->GetCount() <= i_nIndexID) //쓰레기값이니
        return FALSE;

    m_CompMapManager->DeleteData(i_nIndexID);

    return TRUE;
}

BOOL CPackageSpec::SyncOriginCompData()
{
    if (m_CompMapManager->vecCompData.size() > 0)
    {
        m_OriginCompData->vecOriginCompData.resize((long)m_CompMapManager->vecCompData.size());
    }
    else
    {
        CreateDMSInfo();
        return FALSE;
    }

    /*for (int n = 0; n < m_OriginCompData->vecOriginCompData.size(); n++)
	{
		m_CompMapManager->vecCompData[n].nAngle = m_OriginCompData->vecOriginCompData[n].nAngle;
	}*/

    if (m_OriginCompData->m_bCompMapFlipX)
        m_CompMapManager->MirrorMap();

    if (m_OriginCompData->m_bCompMapFlipY)
        m_CompMapManager->FlipMap();

    int rotateIdx = abs(4 - abs(m_CompMapManager->m_nPrevRotateIdx + m_OriginCompData->m_nRotateIdx));
    m_CompMapManager->RotateMap(rotateIdx);
    rotateIdx = abs(2 - abs(m_CompMapManager->m_nPrevRotateIdx + m_OriginCompData->m_nRotateIdx));
    ;
    m_CompMapManager->RotateComp(rotateIdx);

    for (int n = 0; n < m_OriginCompData->vecOriginCompData.size(); n++)
    {
        m_OriginCompData->vecOriginCompData[n].nAngle = m_CompMapManager->vecCompData[n].nAngle;
        m_OriginCompData->vecOriginCompData[n].strCompType = m_CompMapManager->vecCompData[n].strCompType;
        m_OriginCompData->vecOriginCompData[n].fHeight = m_CompMapManager->vecCompData[n].fHeight;
        m_OriginCompData->vecOriginCompData[n].fWidth = m_CompMapManager->vecCompData[n].fWidth;
        m_OriginCompData->vecOriginCompData[n].fLength = m_CompMapManager->vecCompData[n].fLength;
    }

    //m_OriginCompData->vecOriginCompData = m_CompMapManager->vecCompData;

    CreateDMSInfo();

    return TRUE;
}

BOOL CPackageSpec::SetCompData()
{
    if (m_CompMapManager->vecCompData.size() > 0)
    {
        m_CompMapManager->vecCompData.clear();
        if (m_OriginCompData->vecOriginCompData.size() > 0)
            m_CompMapManager->vecCompData.resize((long)m_OriginCompData->vecOriginCompData.size());
        else
        {
            CreateDMSInfo();
            return FALSE;
        }
    }

    //mc_MapDataRead시 Type을 정의할수 없으므로, 여기서 진행한다
    for (auto& OriginCompData : m_OriginCompData->vecOriginCompData)
    {
        OriginCompData.nCompType = OriginCompData.GetCompType();
    }
    //

    m_CompMapManager->vecCompData = m_OriginCompData->vecOriginCompData;

    m_CompMapManager->m_nPrevRotateIdx = 0;

    m_CompMapManager->RotateMap(m_OriginCompData->m_nRotateIdx);

    if (m_OriginCompData->m_bCompMapFlipX)
        m_CompMapManager->MirrorMap();

    if (m_OriginCompData->m_bCompMapFlipY)
        m_CompMapManager->FlipMap();

    m_CompMapManager->m_nPreRotateCompIdx = 0;
    m_CompMapManager->RotateComp(m_OriginCompData->m_nRotateCompIdx);

    CreateDMSInfo();

    return TRUE;
}

BOOL CPackageSpec::SetVisibleData_CurPerOrigin()
{
    if (m_OriginCompData->vecOriginCompData.size() <= 0)
        return FALSE;

    long nOriginDataNum = (long)m_OriginCompData->vecOriginCompData.size();
    long nCurDataNum = (long)m_CompMapManager->vecCompData.size();

    if (nOriginDataNum == nCurDataNum)
    {
        for (long nIndex = 0; nIndex < nOriginDataNum; nIndex++)
            m_OriginCompData->vecOriginCompData[nIndex].bIgnore = m_CompMapManager->vecCompData[nIndex].bIgnore;
    }

    return TRUE;
}

//Pad관련
BOOL CPackageSpec::SetPadData()
{
    if (m_PadMapManager->vecPadData.size() > 0)
    {
        m_PadMapManager->vecPadData.clear();
        if (m_OriginPadData->vecOriginPadData.size() > 0)
            m_PadMapManager->vecPadData.resize((long)m_OriginPadData->vecOriginPadData.size());
        else
            return FALSE;
    }

    m_PadMapManager->vecPadData = m_OriginPadData->vecOriginPadData;

    m_PadMapManager->m_nPrevRotateIdx = 0;

    m_PadMapManager->RotateMap(m_OriginPadData->m_nRotateIdx);

    if (m_OriginPadData->m_bPadMapFlipX)
        m_PadMapManager->MirrorMap();

    if (m_OriginPadData->m_bPadMapFlipY)
        m_PadMapManager->FlipMap();

    return TRUE;
}

BOOL CPackageSpec::SetVisibleData_CurPerOrigin_Pad()
{
    if (m_OriginPadData->vecOriginPadData.size() <= 0)
        return FALSE;

    long nOriginDataNum = (long)m_OriginPadData->vecOriginPadData.size();
    long nCurDataNum = (long)m_PadMapManager->vecPadData.size();

    if (nOriginDataNum == nCurDataNum)
    {
        for (long nIndex = 0; nIndex < nOriginDataNum; nIndex++)
            m_OriginPadData->vecOriginPadData[nIndex].bIgnore = m_PadMapManager->vecPadData[nIndex].bIgnore;
    }

    return TRUE;
}

CString CPackageSpec::GetPinIndexDirection() //kircheis_PinIdxDir
{
    CString strDirection;
    strDirection.Format(_T("None"));

    //Pad Map이 유효한지 확인
    long nPadNum = (long)m_OriginPadData->vecOriginPadData.size();
    if (nPadNum <= 0)
        return strDirection;

    //Pin Index의 Id를 검색
    long nPinIdxID = -1;
    for (long nID = 0; nID < nPadNum; nID++)
    {
        if (m_OriginPadData->vecOriginPadData[nID].GetType() == _typePinIndex)
        {
            nPinIdxID = nID;
            break;
        }
    }

    //Pin Index로 정의된 넘이 있는지 확인
    if (nPinIdxID < 0)
        return strDirection;

    long nRotateIndex = m_OriginPadData->m_nRotateIdx;
    auto pinIndexInfo = m_OriginPadData->vecOriginPadData[nPinIdxID];
    Ipvm::Point32r2 fptPosition = pinIndexInfo.GetOffset();
    Ipvm::Point32r2 fptFlipPosition;
    Ipvm::Point32r2 fptRotatePosition = fptPosition;

    //어리버리하게 Pin Index가 Center 축상에 존재하는지 확인
    if (fptPosition.m_x * fptPosition.m_y == 0.f)
        return strDirection;

    //Flip X/Y 적용
    float fFlipX = m_OriginPadData->m_bPadMapFlipY ? -1.f : 1.f; //Y축을 기준으로 Flip이면 X의 부호가 달라짐
    float fFlipY = m_OriginPadData->m_bPadMapFlipX ? -1.f : 1.f; //X축을 기준으로 Flip이면 Y의 부호가 달라짐

    fptFlipPosition.m_x = fptPosition.m_x * fFlipX;
    fptFlipPosition.m_y = fptPosition.m_y * fFlipY;

    fptRotatePosition = fptFlipPosition;

    //회전 변환 적용
    if (nRotateIndex == Rotate_90)
    {
        fptRotatePosition.m_x = -fptFlipPosition.m_y;
        fptRotatePosition.m_y = fptFlipPosition.m_x;
    }
    else if (nRotateIndex == Rotate_180)
    {
        fptRotatePosition.m_x = -fptFlipPosition.m_x;
        fptRotatePosition.m_y = -fptFlipPosition.m_y;
    }
    else if (nRotateIndex == Rotate_270)
    {
        fptRotatePosition.m_x = fptFlipPosition.m_y;
        fptRotatePosition.m_y = -fptFlipPosition.m_x;
    }

    //Flip & Rotate가 적용된 상태에서의 Pad 위치 확인
    if (fptRotatePosition.m_x >= 0.f && fptRotatePosition.m_y >= 0.f) //1사분면
        strDirection.Format(_T("TopRight"));
    else if (fptRotatePosition.m_x < 0.f && fptRotatePosition.m_y >= 0.f) //2사분면
        strDirection.Format(_T("TopLeft"));
    else if (fptRotatePosition.m_x < 0.f && fptRotatePosition.m_y < 0.f) //3사분면
        strDirection.Format(_T("BottomLeft"));
    else if (fptRotatePosition.m_x >= 0.f && fptRotatePosition.m_y < 0.f) //4사분면
        strDirection.Format(_T("BottomRight"));

    return strDirection;
}
//

//{{Land관련

BOOL CPackageSpec::SetLandData()
{
    if (m_LandMapConvertOrigin->vecLandData.size() > 0)
    {
        m_LandMapConvertOrigin->vecLandData.clear();
    }
    if (m_OriginLandData->vecOriginLandData.size() > 0)
        m_LandMapConvertOrigin->vecLandData.resize((long)m_OriginLandData->vecOriginLandData.size());
    else
    {
        m_LandMapConvertOrigin->vecLandData.clear();
        UpdateLandData();
        return FALSE;
    }

    m_LandMapConvertOrigin->vecLandData = m_OriginLandData->vecOriginLandData;

    m_LandMapConvertOrigin->m_nPrevRotateIdx = 0;

    m_LandMapConvertOrigin->RotateMap(m_OriginLandData->m_nRotateIdx);

    if (m_OriginLandData->m_bLandMapFlipX)
        m_LandMapConvertOrigin->MirrorMap();

    if (m_OriginLandData->m_bLandMapFlipY)
        m_LandMapConvertOrigin->FlipMap();

    m_LandMapConvertOrigin->m_nPreRotateLandIdx = 0;
    m_LandMapConvertOrigin->RotateLand(m_OriginLandData->m_nRotateLandIdx);

    UpdateLandData();

    return TRUE;
}

BOOL CPackageSpec::SetVisibleData_CurPerOrigin_Land()
{
    if (m_OriginLandData->vecOriginLandData.size() <= 0)
        return FALSE;

    long nOriginDataNum = (long)m_OriginLandData->vecOriginLandData.size();
    long nCurDataNum = (long)m_LandMapConvertOrigin->vecLandData.size();

    if (nOriginDataNum == nCurDataNum)
    {
        for (long nIndex = 0; nIndex < nOriginDataNum; nIndex++)
            m_OriginLandData->vecOriginLandData[nIndex].bIgnore = m_LandMapConvertOrigin->vecLandData[nIndex].bIgnore;
    }
    UpdateLandData();

    return TRUE;
}

BOOL CPackageSpec::SetShapeData_CurPerOrigin_Land()
{
    if (m_OriginLandData->vecOriginLandData.size() <= 0)
        return FALSE;

    long nOriginDataNum = (long)m_OriginLandData->vecOriginLandData.size();
    long nCurDataNum = (long)m_LandMapConvertOrigin->vecLandData.size();

    if (nOriginDataNum == nCurDataNum)
    {
        for (long nIndex = 0; nIndex < nOriginDataNum; nIndex++)
            m_OriginLandData->vecOriginLandData[nIndex].nLandShapeType
                = m_LandMapConvertOrigin->vecLandData[nIndex].nLandShapeType;
    }
    UpdateLandData();

    return TRUE;
}

void CPackageSpec::UpdateLandData()
{
    m_LandMapManager->vecLandData.clear();

    long nLandNum = (long)m_LandMapConvertOrigin->vecLandData.size();
    if (nLandNum <= 0)
        return;

    long nLandIndex = 0;

    for (long nID = 0; nID < nLandNum; nID++)
    {
        if (!m_LandMapConvertOrigin->vecLandData[nID].bIgnore)
        {
            m_LandMapManager->vecLandData.push_back(m_LandMapConvertOrigin->vecLandData[nID]);
            m_LandMapManager->vecLandData.back().nIndex = nLandIndex++;
        }
    }

    m_LandMapManager->m_nPrevRotateIdx = m_LandMapConvertOrigin->m_nPrevRotateIdx;
    m_LandMapManager->m_nPreRotateLandIdx = m_LandMapConvertOrigin->m_nPreRotateLandIdx;
}
//}}

//Custom Layer 관련
BOOL CPackageSpec::SetCustomFixedData() //OriginCompData Set
{
    if (m_CustomFixedMapManager->vecCustomFixedData.size() > 0)
    {
        m_CustomFixedMapManager->vecCustomFixedData.clear();
        if (m_OriginCustomFixedData->vecOriginCustomFixedData.size() > 0)
            m_CustomFixedMapManager->vecCustomFixedData.resize(
                (long)m_OriginCustomFixedData->vecOriginCustomFixedData.size());
        else
            return FALSE;
    }

    m_CustomFixedMapManager->vecCustomFixedData = m_OriginCustomFixedData->vecOriginCustomFixedData;

    m_CustomFixedMapManager->m_nPrevRotateIdx = 0;

    m_CustomFixedMapManager->RotateMap(m_OriginCustomFixedData->m_nRotateIdx);

    if (m_OriginCustomFixedData->m_bCustomMapFlipX)
        m_CustomFixedMapManager->MirrorMap();

    if (m_OriginCustomFixedData->m_bCustomMapFlipY)
        m_CustomFixedMapManager->FlipMap();

    return TRUE;
}

BOOL CPackageSpec::SetVisibleData_CurPerOrigin_CustomFixed()
{
    if (m_OriginCustomFixedData->vecOriginCustomFixedData.size() <= 0)
        return FALSE;

    long nOriginDataNum = (long)m_OriginCustomFixedData->vecOriginCustomFixedData.size();
    long nCurDataNum = (long)m_CustomFixedMapManager->vecCustomFixedData.size();

    if (nOriginDataNum == nCurDataNum)
    {
        for (long nIndex = 0; nIndex < nOriginDataNum; nIndex++)
            m_OriginCustomFixedData->vecOriginCustomFixedData[nIndex].m_bIgnore
                = m_CustomFixedMapManager->vecCustomFixedData[nIndex].m_bIgnore;
    }

    return TRUE;
}

BOOL CPackageSpec::SetCustomPolygonData()
{
    if (m_CustomPolygonMapManager->vecCustomPolygonData.size() > 0)
    {
        m_CustomPolygonMapManager->vecCustomPolygonData.clear();
        if (m_OriginCustomPolygonData->vecOriginCustomPolygonData.size() > 0)
            m_CustomPolygonMapManager->vecCustomPolygonData.resize(
                (long)m_OriginCustomPolygonData->vecOriginCustomPolygonData.size());
        else
            return FALSE;
    }

    m_CustomPolygonMapManager->vecCustomPolygonData = m_OriginCustomPolygonData->vecOriginCustomPolygonData;

    m_CustomPolygonMapManager->m_nPrevRotateIdx = 0;

    m_CustomPolygonMapManager->RotateMap(m_OriginCustomPolygonData->m_nRotateIdx);

    if (m_OriginCustomPolygonData->m_bCustomMapFlipX)
        m_CustomPolygonMapManager->MirrorMap();

    if (m_OriginCustomPolygonData->m_bCustomMapFlipY)
        m_CustomPolygonMapManager->FlipMap();

    return TRUE;
}

BOOL CPackageSpec::SetVisibleData_CurPerOrigin_CustomPolygon()
{
    if (m_OriginCustomPolygonData->vecOriginCustomPolygonData.size() <= 0)
        return FALSE;

    long nOriginDataNum = (long)m_OriginCustomPolygonData->vecOriginCustomPolygonData.size();
    long nCurDataNum = (long)m_CustomPolygonMapManager->vecCustomPolygonData.size();

    if (nOriginDataNum == nCurDataNum)
    {
        for (long nIndex = 0; nIndex < nOriginDataNum; nIndex++)
            m_OriginCustomPolygonData->vecOriginCustomPolygonData[nIndex].m_bIgnore
                = m_CustomPolygonMapManager->vecCustomPolygonData[nIndex].m_bIgnore;
    }

    return TRUE;
}

CString CPackageSpec::MakeCompSpecToString(CString strGroupID)
{
    CString strCurCompType = strGroupID;
    CString strCompType;
    BOOL bFineUnderBar = FALSE;
    char ch = (char)strCurCompType.GetAt(0);
    if ((ch == 'R' || ch == 'r' || ch == 'C' || ch == 'c') && strCurCompType.Find(_T("_")) >= 0)
    {
        long nLength = strCurCompType.GetLength();
        strCompType.Empty();
        long nIdx = 0;
        for (nIdx = 0; nIdx < nLength; nIdx++)
        {
            ch = (char)strCurCompType.GetAt(nIdx);
            if (ch == '_')
            {
                if (bFineUnderBar)
                    break;
                bFineUnderBar = TRUE;
            }
        }
        if (nIdx < nLength)
            strCompType = strCurCompType.Left(nIdx);
        else
            strCompType = strGroupID;
    }
    else
        strCompType = strGroupID;

    return strCompType;
}
