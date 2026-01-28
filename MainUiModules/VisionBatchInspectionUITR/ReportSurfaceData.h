#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionUnit;

//HDR_6_________________________________ Header body
//
class ReportSurfaceData
{
public:
    ReportSurfaceData();
    ~ReportSurfaceData();

    void Reset();
    void Add(long dataIndex, LPCTSTR unitID, LPCTSTR barcodeID, VisionUnit& visionUnit);
    bool Save(bool saveBarcode, LPCTSTR saveFolder);
    bool SaveAutomationInfo(
        bool saveBarcode, LPCTSTR saveFolder, VisionUnit& visionUnit); // SDY_Automation Viewer에서 읽을 파일 생성

private:
    struct UnitInfo
    {
        CString m_unitID;
        CString m_barcodeID;
        std::vector<std::vector<CString>> m_objectlist;
    };

    std::vector<UnitInfo> m_inspData_unit;
};
