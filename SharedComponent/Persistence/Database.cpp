//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Database.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
class ArchiveAllType : public CArchive
{
public:
    ArchiveAllType(CFile* pFile, UINT nMode)
        : CArchive(pFile, nMode) {};
    virtual ~ArchiveAllType() {};

    template<class T>
    void Serialize_Element(T& t)
    {
        if (IsStoring())
            (*this) << t;
        else
            (*this) >> t;
    };

    typedef BYTE* BYTEPTR;

    void Serialize_Element(CStringW& istr)
    {
        try
        {
            int nValid = istr.IsEmpty() ? -1 : 1;

            Serialize_Element(nValid);

            if (nValid == 1)
            {
                if (IsStoring())
                {
                    (*this) << istr;
                }
                else
                {
                    (*this) >> istr;
                }
            }
        }
        catch (CArchiveException* ae)
        {
            throw ae;
        }
    }

    void Serialize_Binary(BYTEPTR& pData, ULONG& nDataNum)
    {
        if (IsStoring())
        {
            (*this) << nDataNum;
            Write(pData, nDataNum);
        }
        else
        {
            (*this) >> nDataNum;

            delete[] pData;
            pData = new BYTE[nDataNum];
            Read(pData, nDataNum);
        }
    }

    void Serialize_RawData(BYTE* pData, ULONG nDataNum)
    {
        if (IsStoring())
        {
            Write(pData, nDataNum);
        }
        else
        {
            Read(pData, nDataNum);
        }
    }
};

enum REGISTERDB_KEYTYPE_LIST
{
    REGISTERDB_KEYTYPE_NOSELECT,
    REGISTERDB_KEYTYPE_UINT8,
    REGISTERDB_KEYTYPE_INT16,
    REGISTERDB_KEYTYPE_INT32,
    REGISTERDB_KEYTYPE_LONG,
    REGISTERDB_KEYTYPE_FLOAT,
    REGISTERDB_KEYTYPE_DOUBLE,
    REGISTERDB_KEYTYPE_STRING,
    REGISTERDB_KEYTYPE_POINT,
    REGISTERDB_KEYTYPE_SIZE,
    REGISTERDB_KEYTYPE_RECT,
    REGISTERDB_KEYTYPE_ARRAY_UINT8,
    REGISTERDB_KEYTYPE_ARRAY_INT16,
    REGISTERDB_KEYTYPE_ARRAY_INT32,
    REGISTERDB_KEYTYPE_ARRAY_LONG,
    REGISTERDB_KEYTYPE_ARRAY_FLOAT,
    REGISTERDB_KEYTYPE_ARRAY_DOUBLE,
    REGISTERDB_KEYTYPE_ARRAY_STRING,
    REGISTERDB_KEYTYPE_ARRAY_POINT,
    REGISTERDB_KEYTYPE_ARRAY_SIZE,
    REGISTERDB_KEYTYPE_ARRAY_RECT,
    REGISTERDB_KEYTYPE_INT64,
    REGISTERDB_KEYTYPE_ARRAY_INT64,
    REGISTERDB_KEYTYPE_NATIVEBOOL,
    REGISTERDB_KEYTYPE_ARRAY_NATIVEBOOL,
    REGISTERDB_KEYTYPE_END,
};

Database::Database(void)
{
    __nBinaryDataCount = 0;
    __nDataType = REGISTERDB_KEYTYPE_NOSELECT;
    __pBinary = nullptr;
    __nBinarySize = 0;

    __pDataDB = new CAtlMap<CStringW, Database, CStringElementTraits<CStringW>>;
}

Database::Database(const Database& Object)
{
    __nBinaryDataCount = 0;
    __nDataType = REGISTERDB_KEYTYPE_NOSELECT;
    __pBinary = nullptr;
    __nBinarySize = 0;

    __pDataDB = new CAtlMap<CStringW, Database, CStringElementTraits<CStringW>>;

    *this = Object;
}

Database::~Database(void)
{
    delete[] __pBinary;
    delete __pDataDB;
}

long Database::GetType(void)
{
    return __nDataType;
}

bool Database::operator!=(const Database& Object)
{
    return !(*this == Object);
}

bool Database::operator==(const Database& Object)
{
    if (__nBinaryDataCount != Object.__nBinaryDataCount)
        return false;
    if (__nDataType != Object.__nDataType)
        return false;
    if (__nBinarySize != Object.__nBinarySize)
        return false;
    if (memcmp(__pBinary, Object.__pBinary, __nBinarySize))
        return false;

    long nDataNum1 = (long)__pDataDB->GetCount();
    long nDataNum2 = (long)Object.__pDataDB->GetCount();

    if (nDataNum1 != nDataNum2)
        return false;

    POSITION Cur1 = __pDataDB->GetStartPosition();
    POSITION Cur2 = Object.__pDataDB->GetStartPosition();

    CAtlMap<CStringW, BOOL, CStringElementTraits<CStringW>> StringDB;

    for (int32_t nData = 0; nData < nDataNum1; nData++)
    {
        const CStringW& strKey1 = __pDataDB->GetNextKey(Cur1);

        StringDB[strKey1] = TRUE;
    }

    for (int32_t nData = 0; nData < nDataNum2; nData++)
    {
        const CStringW& strKey2 = Object.__pDataDB->GetNextKey(Cur2);
        if (!StringDB[strKey2])
            return false;

        if ((*__pDataDB)[strKey2] != (*Object.__pDataDB)[strKey2])
            return false;
    }

    return true;
}

Database& Database::operator=(const Database& Object)
{
    __nBinaryDataCount = Object.__nBinaryDataCount;
    __nDataType = Object.__nDataType;
    __nBinarySize = Object.__nBinarySize;

    __BinaryData_SetSize(__nBinarySize);
    memcpy(__pBinary, Object.__pBinary, __nBinarySize);

    __pDataDB->RemoveAll();

    long nDataNum = (long)Object.__pDataDB->GetCount();

    POSITION Cur = Object.__pDataDB->GetStartPosition();

    for (int32_t nData = 0; nData < nDataNum; nData++)
    {
        const CStringW& strKey = Object.__pDataDB->GetNextKey(Cur);
        (*__pDataDB)[strKey] = (*Object.__pDataDB)[strKey];
    }

    return *this;
}

Database& Database::operator[](const wchar_t* strIndex)
{
    return (*__pDataDB)[strIndex];
}

//=================================================================================
// Link 관련
//=================================================================================

bool Database::Link(bool bSave, uint8_t& nData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_UINT8))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(uint8_t));
    __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(uint8_t));

    return true;
}

bool Database::Link(bool bSave, int16_t& nData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_INT16))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(int16_t));
    __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(int16_t));

    return true;
}

bool Database::Link(bool bSave, int32_t& bData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_INT32))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(BOOL));
    __BinaryData_Link(bSave, (BYTE*)&bData, sizeof(BOOL));

    return true;
}

bool Database::Link(bool bSave, bool& bData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_NATIVEBOOL))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(bool));
    __BinaryData_Link(bSave, (BYTE*)&bData, sizeof(bool));

    return true;
}

bool Database::Link(bool bSave, long& nData, long nDef)
{
    UNREFERENCED_PARAMETER(nDef);

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_LONG))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(long));
    __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(long));

    return true;
}

bool Database::Link(bool bSave, unsigned long& nData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_LONG))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(unsigned long));
    __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(unsigned long));

    return true;
}

bool Database::Link(bool bSave, uint32_t& nData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_LONG))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(uint32_t));
    __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(uint32_t));

    return true;
}

bool Database::Link(bool bSave, float& fData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_FLOAT))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(float));
    __BinaryData_Link(bSave, (BYTE*)&fData, sizeof(float));

    return true;
}

bool Database::Link(bool bSave, double& fData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_DOUBLE))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(double));
    __BinaryData_Link(bSave, (BYTE*)&fData, sizeof(double));

    return true;
}

bool Database::Link(bool bSave, CStringW& strData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_STRING))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(sizeof(long) + sizeof(WCHAR) * __GetStringBufferSize(strData));
    }

    __BinaryData_LinkString(bSave, strData);

    return true;
}

bool Database::Link(bool bSave, CPoint& ptData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_POINT))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(sizeof(long) * 2);
    }

    CPoint ptDataPoint(ptData.x, ptData.y);

    __BinaryData_Link(bSave, (BYTE*)&ptDataPoint.x, sizeof(long));
    __BinaryData_Link(bSave, (BYTE*)&ptDataPoint.y, sizeof(long));

    return true;
}

bool Database::Link(bool bSave, Ipvm::Point32s2& ptData)
{
    return Link(bSave, *(CPoint*)&ptData);
}

bool Database::Link(bool bSave, CSize& szData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_SIZE))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(sizeof(long) * 2);
    }

    __BinaryData_Link(bSave, (BYTE*)&szData.cx, sizeof(long));
    __BinaryData_Link(bSave, (BYTE*)&szData.cy, sizeof(long));

    return true;
}

bool Database::Link(bool bSave, Ipvm::Size32s2& szData)
{
    return Link(bSave, *(CSize*)&szData);
}

bool Database::Link(bool bSave, CRect& rtData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_RECT))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(sizeof(long) * 4);
    }

    __BinaryData_Link(bSave, (BYTE*)&rtData.left, sizeof(long));
    __BinaryData_Link(bSave, (BYTE*)&rtData.top, sizeof(long));
    __BinaryData_Link(bSave, (BYTE*)&rtData.right, sizeof(long));
    __BinaryData_Link(bSave, (BYTE*)&rtData.bottom, sizeof(long));

    return true;
}

bool Database::Link(bool bSave, Ipvm::Rect32s& rtData)
{
    return Link(bSave, *(CRect*)&rtData);
}

bool Database::Link(bool bSave, int64_t& nData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_INT64))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(int64_t));
    __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(int64_t));

    return true;
}

bool Database::Link(bool bSave, uint64_t& nData)
{
    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_INT64))
    {
        return false;
    }

    if (bSave)
        __BinaryData_SetSize(sizeof(int64_t));
    __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(int64_t));

    return true;
}

bool Database::SafeLink(bool bSave, long& nData)
{
    if (bSave)
    {
        return Link(bSave, nData);
    }

    bool ret = false;

    switch (__nDataType)
    {
        case REGISTERDB_KEYTYPE_LONG:
            return Link(bSave, nData);
        case REGISTERDB_KEYTYPE_INT32:
        {
            BOOL bBool = FALSE;
            ret = Link(bSave, bBool);
            nData = bBool;
        }
        break;
        case REGISTERDB_KEYTYPE_INT16:
        {
            BOOL nShortData = FALSE;
            ret = Link(bSave, nShortData);
            nData = nShortData;
        }
        break;
    }

    return ret;
}

bool Database::SafeLink(bool bSave, uint32_t& nData)
{
    if (bSave || __nDataType == REGISTERDB_KEYTYPE_INT32)
    {
        return Link(bSave, nData);
    }

    uint32_t nLongData = 0;
    bool ret = Link(bSave, nLongData);
    nData = nLongData;

    return ret;
}

bool Database::SafeLink(bool bSave, uint64_t& nData)
{
    if (bSave || __nDataType == REGISTERDB_KEYTYPE_INT64)
    {
        return Link(bSave, nData);
    }

    ULONG nLongData = 0;
    bool ret = Link(bSave, nLongData);
    nData = nLongData;

    return ret;
}

long Database::GetLinkArrayNum()
{
    long nDataNum = 0;

    switch (__nDataType)
    {
        case REGISTERDB_KEYTYPE_ARRAY_UINT8:
        case REGISTERDB_KEYTYPE_ARRAY_INT16:
        case REGISTERDB_KEYTYPE_ARRAY_INT32:
        case REGISTERDB_KEYTYPE_ARRAY_LONG:
        case REGISTERDB_KEYTYPE_ARRAY_FLOAT:
        case REGISTERDB_KEYTYPE_ARRAY_DOUBLE:
        case REGISTERDB_KEYTYPE_ARRAY_STRING:
        case REGISTERDB_KEYTYPE_ARRAY_POINT:
        case REGISTERDB_KEYTYPE_ARRAY_SIZE:
        case REGISTERDB_KEYTYPE_ARRAY_RECT:
        case REGISTERDB_KEYTYPE_ARRAY_INT64:

            if (true != __LinkReady(FALSE, __nDataType))
            {
                return -1;
            }

            __BinaryData_Link(FALSE, (BYTE*)&nDataNum, sizeof(nDataNum));
            return nDataNum;
            break;
    }

    return -1;
}

bool Database::LinkArray(bool bSave, uint8_t* pnArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_UINT8))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(uint8_t) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)pnArray, sizeof(uint8_t) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, int16_t* pnArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_INT16))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(int16_t) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)pnArray, sizeof(int16_t) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, int32_t* pbArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_INT32))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(BOOL) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)pbArray, sizeof(BOOL) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, uint32_t* pbArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_INT32))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(BOOL) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)pbArray, sizeof(uint32_t) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, long* plArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_LONG))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(long) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(long) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, unsigned long* plArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_LONG))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(unsigned long) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(unsigned long) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, float* pfArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_FLOAT))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(float) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)pfArray, sizeof(float) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, double* pfArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_DOUBLE))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(double) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)pfArray, sizeof(double) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, CStringW* pStrArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_STRING))
    {
        return false;
    }

    if (bSave)
    {
        uint32_t nLength = 0;
        for (uint32_t nArray = 0; nArray < elementCount; nArray++)
        {
            nLength += sizeof(uint32_t) + sizeof(WCHAR) * __GetStringBufferSize(pStrArray[nArray]);
        }

        __BinaryData_SetSize(nLength + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

    for (uint32_t nArray = 0; nArray < nDataArray; nArray++)
    {
        __BinaryData_LinkString(bSave, pStrArray[nArray]);
    }

    return true;
}

bool Database::LinkArray(bool bSave, CPoint* pptArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_POINT))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(int32_t) * 2 + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

    for (uint32_t nArray = 0; nArray < nDataArray; nArray++)
    {
        __BinaryData_Link(bSave, (BYTE*)&pptArray[nArray].x, sizeof(int32_t));
        __BinaryData_Link(bSave, (BYTE*)&pptArray[nArray].y, sizeof(int32_t));
    }

    return true;
}

bool Database::LinkArray(bool bSave, Ipvm::Point32s2* pptArray, const uint32_t elementCount)
{
    return LinkArray(bSave, (CPoint*)pptArray, elementCount);
}

bool Database::LinkArray(bool bSave, CSize* pszArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_SIZE))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(long) * 2 + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

    for (uint32_t nArray = 0; nArray < nDataArray; nArray++)
    {
        __BinaryData_Link(bSave, (BYTE*)&pszArray[nArray].cx, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&pszArray[nArray].cy, sizeof(long));
    }

    return true;
}

bool Database::LinkArray(bool bSave, Ipvm::Size32s2* pszArray, const uint32_t elementCount)
{
    return LinkArray(bSave, (CSize*)pszArray, elementCount);
}

bool Database::LinkArray(bool bSave, CRect* prtArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_RECT))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(long) * 4 + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

    for (uint32_t nArray = 0; nArray < nDataArray; nArray++)
    {
        __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].left, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].top, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].right, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].bottom, sizeof(long));
    }

    return true;
}

bool Database::LinkArray(bool bSave, Ipvm::Rect32s* prtArray, const uint32_t elementCount)
{
    return LinkArray(bSave, (CRect*)prtArray, elementCount);
}

bool Database::LinkArray(bool bSave, int64_t* plArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_INT64))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(int64_t) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(int64_t) * nDataArray);

    return true;
}

bool Database::LinkArray(bool bSave, uint64_t* plArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_INT64))
    {
        return false;
    }

    if (bSave)
    {
        __BinaryData_SetSize(elementCount * sizeof(uint64_t) + sizeof(nDataArray));
    }

    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
    __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(uint64_t) * nDataArray);

    return true;
}

bool Database::SafeLinkArray(bool bSave, uint64_t* plArray, const uint32_t elementCount)
{
    uint32_t nDataArray = elementCount;

    if (bSave)
    {
        if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_INT64))
        {
            return false;
        }

        __BinaryData_SetSize(elementCount * sizeof(uint64_t) + sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(uint64_t) * nDataArray);
    }
    else
    {
        switch (__nDataType)
        {
            case REGISTERDB_KEYTYPE_ARRAY_LONG:
                if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_LONG))
                {
                    return false;
                }
                {
                    long* pOldArray = new long[nDataArray];
                    __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
                    __BinaryData_Link(bSave, (BYTE*)pOldArray, sizeof(ULONG) * nDataArray);

                    for (uint32_t n = 0; n < nDataArray; n++)
                        plArray[n] = pOldArray[n];

                    delete[] pOldArray;
                }
                break;
            case REGISTERDB_KEYTYPE_ARRAY_INT64:
                if (true != __LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_INT64))
                {
                    return false;
                }
                __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
                __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(uint64_t) * nDataArray);
                break;
        }
    }

    return true;
}

Database& Database::GetSubDBFmt(const wchar_t* szFmt, ...)
{
    CStringW strText;

    va_list argptr;

    va_start(argptr, szFmt);
    strText.FormatV(szFmt, argptr);
    va_end(argptr);

    return (*__pDataDB)[strText];
}

long Database::GetSubDBNum()
{
    return (long)__pDataDB->GetCount();
}

POSITION Database::GetSubDBName_FirstPosition()
{
    return __pDataDB->GetStartPosition();
}

void Database::GetSubDBName_NextPosition(POSITION& Position)
{
    __pDataDB->GetNext(Position);
}

const wchar_t* Database::GetSubDBName(POSITION Position)
{
    return __pDataDB->GetKeyAt(Position);
}

void Database::DelSubDB(const wchar_t* strSubDBName)
{
    __pDataDB->RemoveKey(strSubDBName);
}

void Database::Reset(void)
{
    __pDataDB->RemoveAll();
}

bool Database::Load(CFile& File)
{
    bool ret = false;
    long nVersion = 1;

    //-----------------------------------------------------------------------
    // 신비전 체크
    //-----------------------------------------------------------------------

    if (File.GetLength() > 4)
    {
        ArchiveAllType ar(&File, CArchive::load);
        long nNewVersionCheck;
        ar.Read(&nNewVersionCheck, sizeof(long));

        if (nNewVersionCheck == ULONG_MAX)
        {
            // 새로운 버전의 iDB 이다.
            ar.Read(&nVersion, sizeof(long));

            switch (nVersion)
            {
                case 2:
                    ret = Serialize(ar);
                    break;
                default:
                    // 알수 없는 버전
                    break;
            }

            ar.Close();
            File.SeekToBegin();

            return ret;
        }

        ar.Close();
    }

    File.SeekToBegin();

    //-----------------------------------------------------------------------
    // 구비전임
    //-----------------------------------------------------------------------

    ArchiveAllType ar(&File, CArchive::load);
    ret = __Serialize_Old(ar);

    ar.Close();

    return ret;
}

bool Database::Load(const void* buffer, long bufferSize)
{
    CMemFile file((BYTE*)buffer, bufferSize);
    return Load(file);
}

bool Database::Load(LPCTSTR strFileName)
{
    CFile file;
    CFileException fe;

    if (!file.Open(strFileName, CFile::modeRead, &fe))
    {
        return false;
    }

    return Load(file);
}

bool Database::Save(CFile& File)
{
    long nVersion = 2;
    ArchiveAllType ar(&File, CArchive::store);

    long nNewVersionCheck = ULONG_MAX;
    ar.Write(&nNewVersionCheck, sizeof(long));
    ar.Write(&nVersion, sizeof(long));

    bool ret = Serialize(ar);

    ar.Close();

    return ret;
}

bool Database::Save(LPCTSTR strFileName)
{
    CFile file;
    CFileException fe;

    if (!file.Open(strFileName, CFile::modeCreate | CFile::modeWrite))
    {
        return false;
    }

    return Save(file);
}

void Database::GetBuffer(void** buffer, long* bufferSize)
{
    CMemFile file;
    Save(file);

    long nLength = (long)file.GetLength();
    uint8_t* pByte = file.Detach();

    *buffer = pByte;
    *bufferSize = nLength;
}

void Database::ReleaseBuffer(void* buffer)
{
    delete[] buffer;
}

bool Database::Serialize(ArchiveAllType& ar)
{
    if (ar.IsLoading())
    {
        // Load 시에는 일단 기존의 데이터를 제거한다
        (*__pDataDB).RemoveAll();
    }

    ULONG nBlockNum = (ULONG)__pDataDB->GetCount();
    ULONG nBlockCount = 0;

    // 하위 DB Serialize

    CStringW strBlockName;

    ar.Serialize_Element(nBlockNum);

    POSITION PosCur = __pDataDB->GetStartPosition();

    for (nBlockCount = 0; nBlockCount < nBlockNum; nBlockCount++)
    {
        if (ar.IsStoring())
        {
            strBlockName = __pDataDB->GetNextKey(PosCur);
        }

        ar.Serialize_Element(strBlockName);
        (*__pDataDB)[strBlockName].Serialize(ar);
    }

    // Data Serialize

    ar.Serialize_Element(__nDataType);
    ar.Serialize_Binary(__pBinary, __nBinarySize);

    return true;
}

bool Database::__Serialize_Old(ArchiveAllType& ar)
{
    if (ar.IsLoading())
    {
        // Load 시에는 일단 기존의 데이터를 제거한다
        (*__pDataDB).RemoveAll();
    }

    ULONG nBlockNum = (ULONG)__pDataDB->GetCount();
    ULONG nBlockCount = 0;

    // 하위 DB Serialize

    CStringW strBlockName;

    ar.Serialize_Element(nBlockNum);

    POSITION PosCur = __pDataDB->GetStartPosition();

    for (nBlockCount = 0; nBlockCount < nBlockNum; nBlockCount++)
    {
        if (ar.IsStoring())
        {
            strBlockName = __pDataDB->GetNextKey(PosCur);
        }

        ar.Serialize_Element(strBlockName);
        (*__pDataDB)[strBlockName].__Serialize_Old(ar);
    }

    // Data Serialize

    long nTypeSub = 0; // 없어짐

    ar.Serialize_Element(__nDataType);
    ar.Serialize_Element(nTypeSub);
    ar.Serialize_Binary(__pBinary, __nBinarySize);

    return true;
}

long Database::__GetStringBufferSize(CStringW& strData, WCHAR** pStrBuffer)
{
    if (pStrBuffer)
    {
        *pStrBuffer = strData.GetBuffer();
    }
    return strData.GetLength() + 1;
}

void Database::__BinaryData_SetSize(ULONG nSize)
{
    delete[] __pBinary;

    __nBinarySize = nSize;
    __pBinary = new BYTE[__nBinarySize];
}

void Database::__BinaryData_Link(bool bSave, BYTE* pData, long nSize)
{
    if (bSave)
    {
        memcpy(__pBinary + __nBinaryDataCount, pData, nSize);
    }
    else
    {
        memcpy(pData, __pBinary + __nBinaryDataCount, nSize);
    }

    __nBinaryDataCount += nSize;
}
void Database::__BinaryData_LinkString(bool bSave, CStringW& strData)
{
    WCHAR* pStrData;
    long nBufferLength = __GetStringBufferSize(strData, &pStrData) * sizeof(WCHAR);

    __BinaryData_Link(bSave, (BYTE*)&nBufferLength, sizeof(long));

    long nStrLength = nBufferLength / sizeof(WCHAR);

    if (bSave)
    {
        memcpy(__pBinary + __nBinaryDataCount, pStrData, sizeof(WCHAR) * nStrLength);
    }
    else
    {
        pStrData = (WCHAR*)(__pBinary + __nBinaryDataCount);
        strData = pStrData;
    }

    __nBinaryDataCount += nBufferLength;
}

bool Database::__LinkReady(bool bSave, long nLinkType)
{
    __nBinaryDataCount = 0;

    if (bSave)
    {
        // 다른 Type 이 공간에 들어가있는 곳에 무언가를 넣으려 한다
        ASSERT(__nDataType == REGISTERDB_KEYTYPE_NOSELECT || __nDataType == nLinkType);

        __nDataType = nLinkType;

        return true;
    }
    else
    {
        if (__nDataType == nLinkType)
        {
            return true;
        }
        else if (__nDataType == REGISTERDB_KEYTYPE_INT32 && nLinkType == REGISTERDB_KEYTYPE_LONG) // 사실상 같은 타입
        {
            return true;
        }
        else if (__nDataType == REGISTERDB_KEYTYPE_LONG && nLinkType == REGISTERDB_KEYTYPE_INT32) // 사실상 같은 타입
        {
            return true;
        }
        else if (__nDataType == REGISTERDB_KEYTYPE_ARRAY_INT32
            && nLinkType == REGISTERDB_KEYTYPE_ARRAY_LONG) // 사실상 같은 타입
        {
            return true;
        }
        else if (__nDataType == REGISTERDB_KEYTYPE_ARRAY_LONG
            && nLinkType == REGISTERDB_KEYTYPE_ARRAY_INT32) // 사실상 같은 타입
        {
            return true;
        }

        if (__nDataType != REGISTERDB_KEYTYPE_NOSELECT)
        {
            // 다른 Type 이 공간에 들어가있는 곳에서 데이터를 읽으려 한다
            ASSERT(!_T("뭐하는 짓인가?"));

            // 일단 Default 값을 읽을 수 있게 빠져나오게 하였다.
        }
    }

    // 여기는 Loading 시 Key 가 없을 때 들어온다. 이때는 Default 값을 읽어라
    return false;
}
