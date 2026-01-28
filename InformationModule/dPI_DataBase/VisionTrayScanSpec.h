#pragma once
//=====================================================================================
// Tray Scan Spec을 상속받아 비전에서 사용하는 클래스
// Vision에서만 사용하는 일부 함수나 기능을 추가하여 사용하기 위해 상속받아서 만들었다.
//=====================================================================================

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/TrayScanSpec.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <memory>

//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class OldScanSpec;
class VisionTrayScanSpec;

//HDR_6_________________________________ Header body
//
class DPI_DATABASE_API InspectionAreaInfo
{
public:
    InspectionAreaInfo(VisionTrayScanSpec* parent, long id);

    long getStitchIndex(long fovIndex) const;
    void CopyFrom(const InspectionAreaInfo& object);

    VisionTrayScanSpec* m_parent;
    long m_id;

    std::vector<long> m_pocketArrayX; // 사용된 Device Pocket X
    std::vector<long> m_pocketArrayY; // 사용된 Device Pocket Y
    std::vector<int> m_fovList; // 필요한 FOV List
    long m_stichCountX; // 가로 Stitch가 얼마나 필요한가
    long m_stichCountY; // 세로 Stitch가 얼마나 필요한가
    std::vector<long> m_unitIndexList; // 검사영역내에 Unit에 무엇이 존재하는가
};

class DPI_DATABASE_API VisionTrayScanSpec : public TrayScanSpec
{
public:
    VisionTrayScanSpec();
    VisionTrayScanSpec(const VisionTrayScanSpec& object);
    ~VisionTrayScanSpec();

    VisionTrayScanSpec& operator=(const VisionTrayScanSpec& object);

    long GetInspectionItemCount() const;
    const InspectionAreaInfo& GetInspectionItem(long inspectionID) const;
    const Ipvm::Point32r2 GetFovCenter(long fovIndex) const;
    const Ipvm::Point32r2 GetFovCenter(long fovIndex, double paneOffsetX_um, double paneOffsetY_um) const;
    const Ipvm::Point32r2 GetUnitCenterInFOV(long fovIndex, long unitID) const;
    const Ipvm::Rect32r GetUnitValidRegionInFOV(long fovIndex, long unitID) const;
    const Ipvm::Rect32r GetUnitRegionInFOV(long fovIndex, long unitID) const;
    const Ipvm::Rect32r GetUnitPocketRegionInFOV(long fovIndex, long unitID) const;
    const long GetMaxStitchCountX() const;
    const long GetMaxStitchCountY() const;

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;
    void MakeInspectionInfo();
    void RebuildParaForSide(const long i_nSideVisionNumber);
    void SetFromOldRecipe(
        float fovSizeX_mm, float fovSizeY_mm, const CPackageSpec& packageSpec, const OldScanSpec& oldScanSpec);

    // Vision 파라메터 관련
    long GetCurrentPaneID() const;
    long GetCurrentPaneID(long paneCount) const;
    bool SetCurrentPaneID(long nID);
    double GetHorPaneOffset_um() const;
    double GetVerPaneOffset_um() const;
    void SetHorPaneOffset(const double horPaneOffset_um);
    void SetVerPaneOffset(const double verPaneOffset_um);
    void SetBatchKey(long key, LPCTSTR batchBarcode);
    long GetBatchKey() const;
    LPCTSTR GetBatchBarcode() const;

private:
    std::vector<std::shared_ptr<InspectionAreaInfo>> m_infoForInspection;

    void AddFOV(float offsetX_mm, float offsetY_mm, float fovSizeX_mm, float fovSizeY_mm, long inspectionID, long sx,
        long sy, long stitchX, long stitchY, const CPackageSpec& packageSpec, const OldScanSpec& oldScanSpec);
    Ipvm::Rect32r GetUnitLocation(float offsetX_mm, float offsetY_mm, long pocketX, long pocketY,
        const CPackageSpec& packageSpec, const OldScanSpec& oldScanSpec);
    void AddNonOverlappingValueToList(std::vector<long>& dataList, long value);
    void AddStitchList(std::vector<std::vector<int>>& stitchList, long stitchX, long stitchY, long fovIndex);

    // Vision에서만 저장하는 파라메터
    double m_horPaneOffset_um;
    double m_verPaneOffset_um;
    long m_currentPaneIndex;

    // Batch Inspection을 위한 임시 변수
    long m_batchKey;
    CString m_batchBarcode;
};
