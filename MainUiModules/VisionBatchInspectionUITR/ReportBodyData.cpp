//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ReportBodyData.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../ManagementModules/VisionUnitTR/VisionUnit.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionResult.h"
#include "../../UserInterfaceModules/CommonControlExtension/AutoHidePopup.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Gadget/Miscellaneous.h>

//CPP_5_________________________________ Standard library headers
#include <utility>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
ReportBodyData::ReportBodyData()
    : m_inspectionItemCount(0)
{
}

ReportBodyData::~ReportBodyData()
{
}

bool ReportBodyData::isUnitValid(VisionUnit& visionUnit)
{
    long paneResult = visionUnit.GetInspTotalResult();
    if (paneResult == NOT_MEASURED || paneResult == EMPTY || paneResult == DOUBLEDEVICE || paneResult == COUPON
        || paneResult == INVALID)
    {
        // 검사결과가 문제가 있는 것은 데이터로 수집해서는 안된다
        return false;
    }

    return true;
}

void ReportBodyData::Reset()
{
    m_inspectionItemCount = 0;
    m_inspData_unit.clear();
    m_inspHeader.clear();
}

void ReportBodyData::Add(long dataIndex, LPCTSTR unitID, LPCTSTR barcodeID, VisionUnit& visionUnit)
{
    if (!isUnitValid(visionUnit))
        return;

    auto visionInspectionResults = visionUnit.GetVisionInspectionResults();

    if (m_inspectionItemCount == 0)
    {
        for (long resultIndex = 0; resultIndex < long(visionInspectionResults.size()); resultIndex++)
        {
            auto* result = visionInspectionResults[resultIndex];

            bool collect = false;

            // 검증에 필요한 다음 데이터만 저장한다
            // Body 검사 관련
            if (result->m_resultName == _T("BodySizeX"))
                collect = true;
            if (result->m_resultName == _T("BodySizeY"))
                collect = true;
            if (result->m_resultName == _T("BodyParallelism"))
                collect = true;
            if (result->m_resultName == _T("BodyOrth"))
                collect = true;

            //3D Unit 데이터 관련
            if (result->m_resultName == _T("3D Unit Copl"))
                collect = true;
            if (result->m_resultName == _T("3D Unit Warpage"))
                collect = true;

            if (!collect)
                continue;

            m_inspHeader.emplace_back(result->m_resultName, resultIndex);
        }

        m_inspectionItemCount = long(visionInspectionResults.size());
    }

    if (m_inspectionItemCount != visionInspectionResults.size())
    {
        ASSERT(!_T("??"));
        return;
    }

    m_inspData_unit.resize(max(m_inspData_unit.size(), dataIndex + 1));

    m_inspData_unit[dataIndex].m_unitID = unitID;
    m_inspData_unit[dataIndex].m_barcodeID = barcodeID;

    auto& objectlist = m_inspData_unit[dataIndex].m_objectlist;

    for (long headerIndex = 0; headerIndex < long(m_inspHeader.size()); headerIndex++)
    {
        auto& header = m_inspHeader[headerIndex];
        long resultIndex = header.second;

        auto* result = visionInspectionResults[resultIndex];
        const auto objCount = long(result->m_objectErrorValues.size());

        if (result->m_nominalValues.size() != objCount)
            continue;

        for (long obj = 0; obj < objCount; obj++)
        {
            CString objectName = result->m_objectNames[obj];

            auto& objectDataCollect = objectlist[obj];
            auto& values = objectDataCollect.m_inspValues;

            objectDataCollect.m_objectID = objectName;

            values.resize(m_inspHeader.size());
            if (result->m_objectErrorValues[obj] != Ipvm::k_noiseValue32r)
            {
                float value = result->m_nominalValues[obj] + result->m_objectErrorValues[obj];
                values[headerIndex].Format(_T("%.2f"), value);
            }
        }
    }
}

bool ReportBodyData::Save(bool saveBarcode, LPCTSTR saveFolder)
{
    if (m_inspData_unit.size() == 0)
    {
        // 저장할게 없다
        return true;
    }

    long totalLine = 0;

    for (auto& inspData : m_inspData_unit)
    {
        totalLine += long(inspData.m_objectlist.size());
    }

    if (totalLine == 0)
    {
        // 저장할게 없다
        return true;
    }

    Ipvm::CreateDirectories(saveFolder);

    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));
    CString strTotalFilePath;
    strTotalFilePath.Format(_T("%sBodyValue.csv"), saveFolder);

    FILE* fp = nullptr;
    _tfopen_s(&fp, strTotalFilePath, _T("w"));

    if (fp == nullptr)
    {
        return false;
    }

    BOOL bSaveInfo = true;

    if (saveBarcode)
    {
        fprintf(fp, "Unit,Barcode,Index,ID,");
    }
    else
    {
        fprintf(fp, "Unit,Index,ID,");
    }

    for (auto& header : m_inspHeader)
    {
        fprintf(fp, CStringA(header.first) + ",");
    }
    fprintf(fp, "\n");

    // 원래 Batch의 UnitID 순으로 데이터를 저장한다
    for (auto& unitInfo : m_inspData_unit)
    {
        for (auto& object : unitInfo.m_objectlist)
        {
            // Write header
            CString objID;
            objID.Format(_T("%d"), object.first);
            fprintf(fp, CStringA(unitInfo.m_unitID) + ","); // Unit
            if (saveBarcode)
            {
                fprintf(fp, CStringA(unitInfo.m_barcodeID) + ","); // Barcode
            }

            fprintf(fp, CStringA(objID) + ","); // Index
            fprintf(fp, CStringA(object.second.m_objectID) + ","); // ID

            for (auto& value : object.second.m_inspValues)
            {
                fprintf(fp, CStringA(value) + ","); // Value
            }

            fprintf(fp, "\n");
        }
    }

    fclose(fp);

    if (!bSaveInfo)
    {
        AutoHidePopup::add(AutoHidePopupType::Error, _T("Report Data Save Error!"));
    }

    return true;
}

bool ReportBodyData::SaveAutomationInfo(bool saveBarcode, LPCTSTR saveFolder)
{
    if (m_inspData_unit.size() == 0)
    {
        // 저장할게 없다
        return true;
    }

    long totalLine = 0;

    for (auto& inspData : m_inspData_unit)
    {
        totalLine += long(inspData.m_objectlist.size());
    }

    if (totalLine == 0)
    {
        // 저장할게 없다
        return true;
    }

    Ipvm::CreateDirectories(saveFolder);

    CTime time(CTime::GetCurrentTime());
    CString strTime = time.Format(_T("%Y%m%d_%H%M%S"));
    CString strTotalFilePath;
    strTotalFilePath.Format(_T("%sBodyValue.csv"), saveFolder);

    FILE* fp = nullptr;
    _tfopen_s(&fp, strTotalFilePath, _T("w"));

    if (fp == nullptr)
    {
        return false;
    }

    BOOL bSaveInfo = true;

    if (saveBarcode)
    {
        fprintf(fp, "Unit,Barcode,Index,");
    }
    else
    {
        fprintf(fp, "Unit,Index,");
    }

    for (auto& header : m_inspHeader)
    {
        fprintf(fp, CStringA(header.first) + ",");
    }
    fprintf(fp, "\n");

    // 원래 Batch의 UnitID 순으로 데이터를 저장한다
    for (auto& unitInfo : m_inspData_unit)
    {
        for (auto& object : unitInfo.m_objectlist)
        {
            // Write header
            CString objID;
            objID.Format(_T("%d"), object.first);
            fprintf(fp, CStringA(unitInfo.m_unitID) + ","); // Unit
            if (saveBarcode)
            {
                fprintf(fp, CStringA(unitInfo.m_barcodeID) + ","); // Barcode
            }

            fprintf(fp, CStringA(objID) + ","); // Index

            for (auto& value : object.second.m_inspValues)
            {
                fprintf(fp, CStringA(value) + ","); // Value
            }

            fprintf(fp, "\n");
        }
    }

    fclose(fp);

    if (!bSaveInfo)
    {
        AutoHidePopup::add(AutoHidePopupType::Error, _T("Report Data Save Error!"));
    }

    return true;
}