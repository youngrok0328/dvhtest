#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionUnit;

//HDR_6_________________________________ Header body
//
class ReportLidWarpageData
{
public:
    ReportLidWarpageData();
    ~ReportLidWarpageData();

    static bool isUnitValid(VisionUnit& visionUnit);

    void Reset();
    void Add(long dataIndex, LPCTSTR unitID, LPCTSTR barcodeID, VisionUnit& visionUnit);
    bool Save(bool saveBarcode, LPCTSTR saveFolder);

private:
    enum class enumIsnpColumn
    {
        SEQ,
        Nominal_X,
        Nominal_Y,
        Warpage,
        END,
    };

    long m_inspIndex;
    long m_inspectionItemCount;

    // [Unit / ID + InspValues]
    struct UnitInfo
    {
        CString m_unitID;
        CString m_barcodeID;

        struct SObjectInfo
        {
            //long m_objectIndex;
            CString m_objectID;
            CString m_inspValues[long(enumIsnpColumn::END)];
        };

        std::map<long, SObjectInfo> m_objectlist;
    };

    std::vector<UnitInfo> m_inspData_unit;
};
