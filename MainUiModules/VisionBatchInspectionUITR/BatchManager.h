#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "BarcodeData.h"
#include "ReportBodydata.h"
#include "ReportData.h"
#include "ReportLidWarpageData.h"
#include "ReportSurfaceData.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class ImageLotView;
class VisionUnit;
class CGridCtrl;
struct stBatchInspResult;

//HDR_6_________________________________ Header body
//
enum BatchPreColumn
{
    BatchPreColumn_Index,
    BatchPreColumn_FileName,
    BatchPreColumn_Barcode,
    BatchPreColumn_ScanID,
    BatchPreColumn_PaneID,
    BatchPreColumn_TotalResult,
    BatchPreColumn_InspItemStart,
};

struct SBatchUnitInfo
{
    CString strFileName;
    CString strFileDir;

    long m_scanIndex;
    long m_paneIndex;
    long m_result;

    struct SubResult
    {
        CString m_value;
        COLORREF m_color;
    };

    std::vector<SubResult> m_subResults;

    SBatchUnitInfo()
        : m_result(-1)
        , m_scanIndex(-1)
        , m_paneIndex(-1)
    {
    }
};

class BatchManager
{
public:
    BatchManager();
    ~BatchManager();

    void Reset();
    void Inspect_StartReady(VisionUnit& visionUnit);
    void Inspect_DataCollect(VisionUnit& visionUnit, long index);
    void UpdateGridCtrl_List(CGridCtrl& ctrl);
    void UpdateGridCtrl_Batch(CGridCtrl& ctrl);
    void UpdateGridCtrl_Batch(CGridCtrl& ctrl, long gridIndex);

    std::vector<SBatchUnitInfo> m_vecBatchList;
    std::vector<stBatchInspResult> m_vecstBatchInspResult;

    BarcodeData m_barcodeData;
    ReportData m_reportData;
    ReportLidWarpageData m_reportWarpageData;
    ReportSurfaceData m_reportSurfaceData;
    ReportBodyData m_reportBodyData;

    long m_lastFrameIndex;
    long m_nImageNum;
    long m_nGridNum;
    BOOL m_bInfiniteInspectionMode;

    typedef CString RnRDataName;
    typedef CString RnRDataObjectID;
    typedef CString RnRDataScanPane;
    typedef std::pair<RnRDataName, RnRDataScanPane> RnRDataKey;
    typedef std::vector<std::pair<RnRDataObjectID, std::vector<float>>> RnRDataValue;
    std::map<RnRDataKey, RnRDataValue> m_mapRnRData;
};
