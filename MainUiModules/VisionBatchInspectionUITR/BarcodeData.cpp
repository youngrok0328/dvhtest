//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BarcodeData.h"

//CPP_2_________________________________ This project's headers
#include "ImageFileParser.h"

//CPP_3_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedComponent/Persistence/IniHelper.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BarcodeKey::BarcodeKey(long trayID, long scanID, long paneID)
    : m_trayID(trayID)
    , m_scanID(scanID)
    , m_paneID(paneID)
{
}

bool BarcodeKey::operator<(const BarcodeKey& object) const
{
    if (m_trayID < object.m_trayID)
        return true;
    if (m_trayID > object.m_trayID)
        return false;

    if (m_scanID < object.m_scanID)
        return true;
    if (m_scanID > object.m_scanID)
        return false;

    if (m_paneID < object.m_paneID)
        return true;
    if (m_paneID > object.m_paneID)
        return false;

    return false;
}

BarcodeData::BarcodeData()
{
}

BarcodeData::~BarcodeData()
{
}

void BarcodeData::Reset()
{
    m_visionBarcodeMap.clear();
}

void BarcodeData::AddFile(long visionIndex, LPCTSTR filePath)
{
    int m_pocketCount = 0;
    m_pocketCount = IniHelper::LoadINT(filePath, _T("TrayInfo"), _T("Size"), 0);
    m_NX = IniHelper::LoadINT(filePath, _T("TrayInfo"), _T("NX"), 0);
    m_NY = IniHelper::LoadINT(filePath, _T("TrayInfo"), _T("NY"), 0);

    for (long pocketIndex = 0; pocketIndex < m_pocketCount; pocketIndex++)
    {
        CString category;
        category.Format(_T("Pocket%d"), pocketIndex);

        int visionCount = IniHelper::LoadINT(filePath, category, _T("VisionCount"), 0);
        if (visionIndex >= visionCount)
            continue;

        CString barcode = IniHelper::LoadSTRING(filePath, category, _T("Barcode"));

        CString keyHead;
        keyHead.Format(_T("Vision%d_"), visionIndex);

        long trayID = IniHelper::LoadINT(filePath, category, keyHead + _T("TrayID"));
        long scanID = IniHelper::LoadINT(filePath, category, keyHead + _T("ScanID"));
        long paneID = IniHelper::LoadINT(filePath, category, keyHead + _T("Pane"));

        m_visionBarcodeMap[BarcodeKey(trayID, scanID, paneID)] = barcode;
    }
}

bool BarcodeData::IsEnabled() const
{
    return m_visionBarcodeMap.size() > 0 ? true : false;
}

LPCTSTR BarcodeData::GetBarcode(long paneID, LPCTSTR filePath) const
{
    long trayID = ImageFileParser::GetTrayID(filePath);
    long scanID = 0;

    if (SystemConfig::GetInstance().IsVisionType3D() == TRUE)
    {
        scanID = (ImageFileParser::GetScanID(filePath) * m_NX) + paneID;
    }
    else
    {
        scanID = ImageFileParser::GetScanID(filePath);
    }

    auto itBarcode = m_visionBarcodeMap.find(BarcodeKey(trayID, scanID, paneID));
    if (itBarcode == m_visionBarcodeMap.end())
        return _T("");

    return itBarcode->second;
}
