#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class ArchiveAllType;
class CiDataBase;

//HDR_6_________________________________ Header body
//
class __VISION_HOST_COMMON_API__ BinaryFileData
{
public:
    BinaryFileData();
    BinaryFileData(const BinaryFileData& object);
    ~BinaryFileData();

    BinaryFileData& operator=(const BinaryFileData& object);

    void Reset();
    BOOL FromDB(CiDataBase& db);
    BOOL ToDB(CiDataBase& db) const;

    BOOL Save(LPCTSTR filePath) const;
    BOOL SaveAppend(LPCTSTR filePath) const;
    BOOL Load(LPCTSTR filePath);
    BOOL LoadPartial(LPCTSTR filePath, UINT startOffset, UINT length);
    ULONG GetSize();
    BOOL SerializeForComm(ArchiveAllType& ar);

private:
    BYTE* m_datas;
    ULONG m_dataLength;
};
