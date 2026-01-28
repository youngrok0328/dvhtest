//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "ReportData.h"

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
ReportData::ReportData()
    : m_inspectionItemCount(0)
{
}

ReportData::~ReportData()
{
}

bool ReportData::isUnitValid(VisionUnit& visionUnit)
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

void ReportData::Reset()
{
    m_inspectionItemCount = 0;
    m_inspData_unit.clear();
    m_inspHeader.clear();
}

void ReportData::Add(long dataIndex, LPCTSTR unitID, LPCTSTR barcodeID, VisionUnit& visionUnit)
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
            // Ball 검사 관련
            // - 3D
            if (result->m_resultName == _T("3D Copl"))
                collect = true;
            if (result->m_resultName == _T("3D Height"))
                collect = true;
            if (result->m_resultName == _T("3D Warpage"))
                collect = true;
            // - 2D
            if (result->m_resultName == _T("Ball Offset X"))
                collect = true;
            if (result->m_resultName == _T("Ball Offset Y"))
                collect = true;
            if (result->m_resultName == _T("Ball Offset R"))
                collect = true;
            if (result->m_resultName == _T("Ball Grid Offset X"))
                collect = true;
            if (result->m_resultName == _T("Ball Grid Offset Y"))
                collect = true;
            if (result->m_resultName == _T("Ball Width"))
                collect = true;
            if (result->m_resultName == _T("Ball Quality"))
                collect = true;
            if (result->m_resultName == _T("Ball Contrast"))
                collect = true;
            if (result->m_resultName == _T("Ball Bridge"))
                collect = true;

            // Land 검사 관련
            // - 3D
            if (result->m_resultName == _T("Land Copl"))
                collect = true;
            if (result->m_resultName == _T("Land Height"))
                collect = true;
            if (result->m_resultName == _T("Land Warpage"))
                collect = true;
            // - 2D
            if (result->m_resultName == _T("Land Offset X"))
                collect = true;
            if (result->m_resultName == _T("Land Offset Y"))
                collect = true;
            if (result->m_resultName == _T("Land Offset R"))
                collect = true;
            if (result->m_resultName == _T("Land Width"))
                collect = true;
            if (result->m_resultName == _T("Land Length"))
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

            // Automation Report 작성을 위해 Position 정보를 받는다.
            objectDataCollect.m_objectPosX = result->m_objectPositionX[obj];
            objectDataCollect.m_objectPosY = result->m_objectPositionY[obj];

            values.resize(m_inspHeader.size());
            if (result->m_objectErrorValues[obj] != Ipvm::k_noiseValue32r)
            {
                float value = result->m_nominalValues[obj] + result->m_objectErrorValues[obj];
                values[headerIndex].Format(_T("%.2f"), value);
            }
        }
    }
}

bool ReportData::Save(bool saveBarcode, LPCTSTR saveFolder)
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
    strTotalFilePath.Format(_T("%sValue.csv"), saveFolder);

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

bool ReportData::SaveAutomationInfo(bool saveBarcode, LPCTSTR saveFolder)
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
    strTotalFilePath.Format(_T("%sValue.csv"), saveFolder);

    FILE* fp = nullptr;
    _tfopen_s(&fp, strTotalFilePath, _T("w"));

    if (fp == nullptr)
    {
        return false;
    }

    BOOL bSaveInfo = true;

    if (saveBarcode)
    {
        fprintf(fp, "Unit,Barcode,Index,ID,Pos X,Pos Y,");
    }
    else
    {
        fprintf(fp, "Unit,Index,ID,Pos X,Pos Y,");
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

            CString PosX;
            PosX.Format(_T("%.2f"), object.second.m_objectPosX);
            CString PosY;
            PosY.Format(_T("%.2f"), object.second.m_objectPosY);

            fprintf(fp, (CStringA(PosX) + ",")); // Pos X
            fprintf(fp, (CStringA(PosY) + ",")); // Pos Y

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
