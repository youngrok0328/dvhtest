#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "BallCollection.h"
#include "BallCollectionOriginal.h"
#include "BodyInfo.h"
#include "Component.h"
#include "ComponentCollection.h"
#include "ComponentCollectionOriginal.h"
#include "CustomFixedCollection.h"
#include "CustomFixedCollectionOriginal.h"
#include "CustomPolygonCollection.h"
#include "CustomPolygonCollectionOriginal.h"
#include "DMSArrayChip.h"
#include "DMSChip.h"
#include "DMSHeatsink.h"
#include "DMSPassiveChip.h"
#include "DMSPatch.h"
#include "Enum.h"
#include "LandCollection.h"
#include "LandCollectionOriginal.h"
#include "PadCollection.h"
#include "PadCollectionOriginal.h"

//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionScale;
class CFigure;
class CiDataBase;

//namespace Package
//{
//struct BodyInfo;
//class BallCollection;
//class BallCollectionOriginal;
//class ComponentCollection;
//class ComponentCollectionOriginal;
//class CustomFixedCollection;
//class CustomFixedCollectionOriginal;
//class CustomPolygonCollection;
//class CustomPolygonCollectionOriginal;
//class Component;
//class LandCollection;
//class LandCollectionOriginal;
//class PadCollection;
//class PadCollectionOriginal;
//} // namespace Package

const float gf_MILS_TO_MM = 0.0254f;
const float gf_MM_TO_MILS = 1000.f / 25.4f;

//HDR_6_________________________________ Header body
//
class DPI_DATABASE_API CPackageSpec
{
public:
    CPackageSpec();
    CPackageSpec(const CPackageSpec& rhs) = delete;
    ~CPackageSpec();

    CPackageSpec& operator=(CPackageSpec* psrcObj) = delete;

    void CalcDependentVariables(const VisionScale& scale);
    bool IsSameDepedentScale(const VisionScale& scale) const;

    void SetPackageType(enPackageType nNewPackageType);

    void Init();

public:
    // Depedent Variable 계산에 사용된 Scale
    float m_dependent_um2px_x;
    float m_dependent_um2px_y;

    //////////////////////////////////////////////////////////////////
    //	Serialized Data
    //	데이터 시리얼라이즈는 여기 있는 순서를 꼭 지킬 것.
    //	멤버 변수의 선언 순서에 따라 애들을 시리얼라이즈하고,
    //	이 순서 그대로 디폴트 값을 메기고, 복사하고... 알간?

    enDeviceType nDeviceType;
    enPackageType nPackageType;
    BOOL m_deadBug;

    Package::BodyInfo* m_bodyInfoMaster;

    Package::BallCollection* m_ballMap; // 실제 알고리즘에서 사용할 녀석임.
    Package::BallCollection* m_ballMapAll; // 검사에서 제외된 볼들까지 모두 포함됨. 얼라인 등에서 사용
    Package::BallCollectionOriginal* m_originalballMap; // 볼맵 정보를 설정할 때만 사용하고,
    // 알고리즘에서 직접 사용하지는 않도록 한다.

    Package::LandCollection* m_LandMapManager;
    Package::LandCollection* m_LandMapConvertOrigin;
    Package::LandCollectionOriginal* m_OriginLandData;
    BOOL SetLandData(); //OriginCompData Set
    BOOL SetVisibleData_CurPerOrigin_Land(); //VisibleData는 주자.. 현재 적용된 값을 원본에
    BOOL SetShapeData_CurPerOrigin_Land();
    void UpdateLandData();

    Package::PadCollection* m_PadMapManager; //Algorithm에서 사용할 Data Set..
    // ex) DMSInfo Set Add 및 Delete할때 Origin까지 같이 수정할려는 경우
    Package::PadCollectionOriginal* m_OriginPadData; //Origin PadMap
    BOOL SetPadData(); //OriginCompData Set
    BOOL SetVisibleData_CurPerOrigin_Pad(); //VisibleData는 주자.. 현재 적용된 값을 원본에
    CString GetPinIndexDirection(); //kircheis_PinIdxDir

    Package::ComponentCollection* m_CompMapManager; //Algorithm에서 사용할 Data Set..
    // ex) DMSInfo Set Add 및 Delete할때 Origin까지 같이 수정할려는 경우
    Package::ComponentCollectionOriginal* m_OriginCompData; //Origin CompMap
    BOOL SyncOriginCompData();
    BOOL SetCompData(); //OriginCompData Set
    BOOL SetVisibleData_CurPerOrigin(); //VisibleData는 주자.. 현재 적용된 값을 원본에

    Package::CustomFixedCollection* m_CustomFixedMapManager;
    Package::CustomFixedCollectionOriginal* m_OriginCustomFixedData;
    BOOL SetCustomFixedData(); //OriginCompData Set
    BOOL SetVisibleData_CurPerOrigin_CustomFixed(); //VisibleData는 주자.. 현재 적용된 값을 원본에

    Package::CustomPolygonCollection* m_CustomPolygonMapManager;
    Package::CustomPolygonCollectionOriginal* m_OriginCustomPolygonData;
    BOOL SetCustomPolygonData(); //OriginCompData Set
    BOOL SetVisibleData_CurPerOrigin_CustomPolygon(); //VisibleData는 주자.. 현재 적용된 값을 원본에

    // Component DMS information
    std::vector<Chip::DMSChip> vecDMSChip;
    std::vector<Chip::DMSPassiveChip> vecDMSPassive;
    std::vector<Chip::DMSArrayChip> vecDMSArray;
    std::vector<Chip::DMSHeatsink> vecDMSHeatsink;
    std::vector<Chip::DMSPatch> vecDMSPatch; //kircheis_POI

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    /////inteksjs
    BOOL m_bLowtopEdge;

    /////

    //mc_Component관련 함수
    BOOL ClearDMSInfo();
    void CreateDMSInfo();
    BOOL DeleteCompMapData(long i_nIndexID);
    CString MakeCompSpecToString(CString strGroupID);

private:
    BOOL PushDMSInfoChip(std::vector<Package::Component> i_vecChipComponentMap);
    BOOL SetDMS_ChipcData(CString i_strComType, Package::Component i_ChipComponentMap, long i_nIndex);
    BOOL PushDMSInfoPassive(std::vector<Package::Component> i_vecPassiveComponentMap);
    BOOL SetDMS_PassiveData(CString i_strComType, Package::Component i_PassiveComponentMap, long i_nIndex);
    enumPassiveTypeDefine GetPassiveType(CString i_strCompCategory);
    BOOL PushDMSInfoArray(std::vector<Package::Component> i_vecArrayComponentMap);
    BOOL SetDMS_ArrayData(CString i_strComType, Package::Component i_ArrayComponentMap, long i_nIndex);
    BOOL PushDMSInfoHeatSync(std::vector<Package::Component> i_vecHeatSynComponent);
    BOOL SetDMS_HeatSyncData(CString i_strComType, Package::Component i_HeatSynComponent, long i_nIndex);
    BOOL PushDMSInfoPatch(std::vector<Package::Component> i_vecPatchComponentMap); //kircheis_POI
    BOOL SetDMS_PatchData(CString i_strComType, Package::Component i_PatchComponentMap, long i_nIndex); //kircheis_POI
    //
};
