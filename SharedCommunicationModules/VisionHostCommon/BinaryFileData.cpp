//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "BinaryFileData.h"

//CPP_2_________________________________ This project's headers
#include "ArchiveAllType.h"
#include "DBObject.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
BinaryFileData::BinaryFileData()
    : m_datas(nullptr)
    , m_dataLength(0)
{
}

BinaryFileData::BinaryFileData(const BinaryFileData& object)
{
    m_dataLength = object.m_dataLength;
    m_datas = new BYTE[m_dataLength];

    *this = object;
}

BinaryFileData::~BinaryFileData()
{
    delete[] m_datas;
}

BinaryFileData& BinaryFileData::operator=(const BinaryFileData& object)
{
    delete[] m_datas;
    m_datas = new BYTE[object.m_dataLength];
    m_dataLength = object.m_dataLength;

    memcpy(m_datas, object.m_datas, m_dataLength);

    return *this;
}

void BinaryFileData::Reset()
{
    delete[] m_datas;
    m_datas = nullptr;
    m_dataLength = 0;
}

BOOL BinaryFileData::FromDB(CiDataBase& db)
{
    CMemFile memFile;
    ArchiveAllType ar(&memFile, ArchiveAllType::store);
    if (!db.Serialize(ar))
    {
        return FALSE;
    }

    ar.Close();
    m_dataLength = (ULONG)memFile.GetLength();

    delete[] m_datas;
    m_datas = memFile.Detach();

    return TRUE;
}

BOOL BinaryFileData::ToDB(CiDataBase& db) const
{
    CMemFile memFile(m_datas, m_dataLength);

    ArchiveAllType ar(&memFile, ArchiveAllType::load);

    return db.Serialize(ar);
}

BOOL BinaryFileData::Save(LPCTSTR filePath) const
{
    CFile file;
    if (!file.Open(filePath, CFile::modeCreate | CFile::modeWrite))
    {
        return FALSE;
    }

    if (m_dataLength)
    {
        file.Write(m_datas, m_dataLength);
    }

    file.Close();

    return TRUE;
}

BOOL BinaryFileData::SaveAppend(LPCTSTR filePath) const
{
    // 데이터가 없으면 아무것도 하지 않음
    if (m_datas == nullptr || m_dataLength == 0)
        return TRUE; // 빈 데이터는 append할 게 없음 (정상 종료)

    CFile file;
    CFileException ex;

    // 파일이 있으면 끝에 덧붙이고, 없으면 새로 생성
    if (!file.Open(filePath, CFile::modeWrite | CFile::modeNoTruncate | CFile::modeCreate | CFile::typeBinary, &ex))
    {
        return FALSE;
    }

    BOOL result = TRUE;

    TRY
    {
        // 파일 끝으로 이동
        file.Seek(0, CFile::end);

        // 데이터 추가
        file.Write(m_datas, m_dataLength);
    }
    CATCH(CFileException, e)
    {
        result = FALSE;
        e->Delete();
    }
    END_CATCH

    file.Close();
    return result;
}

BOOL BinaryFileData::Load(LPCTSTR filePath)
{
    CFile file;
    if (!file.Open(filePath, CFile::modeRead))
    {
        return FALSE;
    }

    delete[] m_datas;
    m_dataLength = (UINT)file.GetLength();
    m_datas = new BYTE[m_dataLength];

    if (m_dataLength > 0)
    {
        file.Read(m_datas, m_dataLength);
    }

    file.Close();

    return TRUE;
}

BOOL BinaryFileData::LoadPartial(LPCTSTR filePath, UINT startOffset, UINT length)
{
    CFile file;
    if (!file.Open(filePath, CFile::modeRead | CFile::typeBinary))
        return FALSE;

    ULONGLONG totalSize = file.GetLength();
    if (startOffset >= totalSize || (startOffset == 1 && length == 0))
    {
        // 기존 데이터 해제
        if (m_datas)
        {
            delete[] m_datas;
            m_datas = nullptr;
        }
        m_dataLength = 0; // 읽어 들일 것이 없는 것이다.
        m_datas = new BYTE[m_dataLength];
        file.Close();
        return FALSE; // 파일보다 큰 오프셋이면 읽을 수 없음
    }

    // length == 0 이면 끝까지 읽기
    if (length == 0 || (startOffset + length) > totalSize)
        length = static_cast<UINT>(totalSize - startOffset);

    // 기존 데이터 해제
    if (m_datas)
    {
        delete[] m_datas;
        m_datas = nullptr;
    }

    m_dataLength = length;
    m_datas = new BYTE[m_dataLength];

    // 지정된 위치로 이동 후 읽기
    file.Seek(startOffset, CFile::begin);
    UINT bytesRead = file.Read(m_datas, m_dataLength);

    file.Close();

    return (bytesRead == m_dataLength);
}

BOOL BinaryFileData::SerializeForComm(ArchiveAllType& ar)
{
    ar.Serialize_Element(m_dataLength);

    if (ar.IsLoading())
    {
        delete[] m_datas;
        m_datas = new BYTE[m_dataLength];
    }

    if (ar.IsStoring())
    {
        ar.Write(m_datas, m_dataLength);
    }
    else
    {
        ar.Read(m_datas, m_dataLength);
    }

    return TRUE;
}

ULONG BinaryFileData::GetSize()
{
    return m_dataLength;
}
