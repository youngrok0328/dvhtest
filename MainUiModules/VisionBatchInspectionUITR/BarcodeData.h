#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class BarcodeKey
{
public:
    long m_trayID;
    long m_scanID;
    long m_paneID;

    BarcodeKey(long trayID, long scanID, long paneID);

    bool operator<(const BarcodeKey& object) const;
};

class BarcodeData
{
public:
    BarcodeData();
    ~BarcodeData();

    void Reset();
    void AddFile(long visionIndex, LPCTSTR filePath);
    bool IsEnabled() const;

    LPCTSTR GetBarcode(long paneID, LPCTSTR filePath) const;

private:
    std::map<BarcodeKey, CString> m_visionBarcodeMap;

    long m_size;
    long m_NX;
    long m_NY;
};
