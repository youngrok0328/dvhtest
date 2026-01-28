#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ArchiveAllType.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>
#include <atlcoll.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum REGISTERDB_KEYTYPE_LIST
{
    REGISTERDB_KEYTYPE_NOSELECT,
    REGISTERDB_KEYTYPE_BYTE,
    REGISTERDB_KEYTYPE_SHORT,
    REGISTERDB_KEYTYPE_BOOL,
    REGISTERDB_KEYTYPE_LONG,
    REGISTERDB_KEYTYPE_FLOAT,
    REGISTERDB_KEYTYPE_DOUBLE,
    REGISTERDB_KEYTYPE_STRING,
    REGISTERDB_KEYTYPE_POINT,
    REGISTERDB_KEYTYPE_SIZE,
    REGISTERDB_KEYTYPE_RECT,
    REGISTERDB_KEYTYPE_ARRAY_BYTE,
    REGISTERDB_KEYTYPE_ARRAY_SHORT,
    REGISTERDB_KEYTYPE_ARRAY_BOOL,
    REGISTERDB_KEYTYPE_ARRAY_LONG,
    REGISTERDB_KEYTYPE_ARRAY_FLOAT,
    REGISTERDB_KEYTYPE_ARRAY_DOUBLE,
    REGISTERDB_KEYTYPE_ARRAY_STRING,
    REGISTERDB_KEYTYPE_ARRAY_POINT,
    REGISTERDB_KEYTYPE_ARRAY_SIZE,
    REGISTERDB_KEYTYPE_ARRAY_RECT,
    REGISTERDB_KEYTYPE_END,
};

//======================================================================================================
// DB 내용
//======================================================================================================

class CiDataBase
{
public:
    CiDataBase(void)
    {
        __nBinaryDataCount = 0;
        __nDataType = REGISTERDB_KEYTYPE_NOSELECT;
        __nDataTypeSub = 0;
        __pBinary = NULL;
        __nBinarySize = 0;

        __pDataDB = new CAtlMap<CString, CiDataBase, CStringElementTraits<CString>>;
    }

    CiDataBase(const CiDataBase& Object)
    {
        __nBinaryDataCount = 0;
        __nDataType = REGISTERDB_KEYTYPE_NOSELECT;
        __nDataTypeSub = 0;
        __pBinary = NULL;
        __nBinarySize = 0;

        __pDataDB = new CAtlMap<CString, CiDataBase, CStringElementTraits<CString>>;

        *this = Object;
    }

public:
    ~CiDataBase(void)
    {
        delete[] __pBinary;
        delete __pDataDB;
    }

public:
    long GetType(void)
    {
        return __nDataType;
    }

    long GetSubType(void)
    {
        return __nDataTypeSub;
    }

    CiDataBase& operator=(const CiDataBase& Object)
    {
        __nBinaryDataCount = Object.__nBinaryDataCount;
        __nDataType = Object.__nDataType;
        __nDataTypeSub = Object.__nDataTypeSub;
        __nBinarySize = Object.__nBinarySize;

        __BinaryData_SetSize(__nBinarySize);
        memcpy(__pBinary, Object.__pBinary, __nBinarySize);

        long nDataNum = (long)(Object.__pDataDB->GetCount());

        __pDataDB->RemoveAll();
        POSITION Cur = Object.__pDataDB->GetStartPosition();

        for (long nData = 0; nData < nDataNum; nData++)
        {
            const CString& strKey = Object.__pDataDB->GetNextKey(Cur);
            (*__pDataDB)[strKey] = (*Object.__pDataDB)[strKey];
        }

        return *this;
    }

    bool operator==(const CiDataBase& Object) const
    {
        if (!IsLinkDataSame(Object))
        {
            return false;
        }

        if (__pDataDB->GetCount() != Object.__pDataDB->GetCount())
            return false;

        long nDataNum = (long)(Object.__pDataDB->GetCount());

        POSITION Cur = __pDataDB->GetStartPosition();

        for (long nData = 0; nData < nDataNum; nData++)
        {
            const CString& strKey = __pDataDB->GetNextKey(Cur);

            auto objSub = Object.__pDataDB->Lookup(strKey);
            if (objSub == nullptr)
                return false;

            if ((*__pDataDB)[strKey] != objSub->m_value)
                return false;
        }

        return true;
    }

    bool operator!=(const CiDataBase& Object) const
    {
        return !(*this == Object);
    }

    CiDataBase& operator[](LPCTSTR strIndex)
    {
        return (*__pDataDB)[strIndex];
    }

    //=================================================================================
    // Link 관련
    //=================================================================================

    BOOL Link(BOOL bSave, BYTE& nData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_BYTE))
        {
            return FALSE;
        }

        if (bSave)
            __BinaryData_SetSize(sizeof(BYTE));
        __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(BYTE));

        return TRUE;
    }

    BOOL Link(BOOL bSave, short& nData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_SHORT))
        {
            return FALSE;
        }

        if (bSave)
            __BinaryData_SetSize(sizeof(short));
        __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(short));

        return TRUE;
    }

    BOOL Link(BOOL bSave, BOOL& bData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_BOOL))
        {
            return FALSE;
        }

        if (bSave)
            __BinaryData_SetSize(sizeof(BOOL));
        __BinaryData_Link(bSave, (BYTE*)&bData, sizeof(BOOL));

        return TRUE;
    }

    BOOL Link(BOOL bSave, long& nData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_LONG))
        {
            return FALSE;
        }

        if (bSave)
            __BinaryData_SetSize(sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(long));

        return TRUE;
    }

    BOOL Link(BOOL bSave, unsigned long& nData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_LONG))
        {
            return FALSE;
        }

        if (bSave)
            __BinaryData_SetSize(sizeof(unsigned long));
        __BinaryData_Link(bSave, (BYTE*)&nData, sizeof(unsigned long));

        return TRUE;
    }

    BOOL Link(BOOL bSave, float& fData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_FLOAT))
        {
            return FALSE;
        }

        if (bSave)
            __BinaryData_SetSize(sizeof(float));
        __BinaryData_Link(bSave, (BYTE*)&fData, sizeof(float));

        return TRUE;
    }

    BOOL Link(BOOL bSave, double& fData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_DOUBLE))
        {
            return FALSE;
        }

        if (bSave)
            __BinaryData_SetSize(sizeof(double));
        __BinaryData_Link(bSave, (BYTE*)&fData, sizeof(double));

        return TRUE;
    }

    BOOL Link(BOOL bSave, CString& strData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_STRING))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(sizeof(long) + sizeof(WCHAR) * __GetStringBufferSize(strData));
        }

        __BinaryData_LinkString(bSave, strData);

        return TRUE;
    }

    BOOL Link(BOOL bSave, Ipvm::Point32s2& ptData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_POINT))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(sizeof(long) * 2);
        }

        __BinaryData_Link(bSave, (BYTE*)&ptData.m_x, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&ptData.m_y, sizeof(long));

        return TRUE;
    }

    BOOL Link(BOOL bSave, CSize& szData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_SIZE))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(sizeof(long) * 2);
        }

        __BinaryData_Link(bSave, (BYTE*)&szData.cx, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&szData.cy, sizeof(long));

        return TRUE;
    }

    BOOL Link(BOOL bSave, Ipvm::Rect32s& rtData)
    {
        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_RECT))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(sizeof(long) * 4);
        }

        __BinaryData_Link(bSave, (BYTE*)&rtData.m_left, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&rtData.m_top, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&rtData.m_right, sizeof(long));
        __BinaryData_Link(bSave, (BYTE*)&rtData.m_bottom, sizeof(long));

        return TRUE;
    }

    long GetLinkArrayNum()
    {
        long nDataNum = 0;

        switch (__nDataType)
        {
            case REGISTERDB_KEYTYPE_ARRAY_BYTE:
            case REGISTERDB_KEYTYPE_ARRAY_SHORT:
            case REGISTERDB_KEYTYPE_ARRAY_BOOL:
            case REGISTERDB_KEYTYPE_ARRAY_LONG:
            case REGISTERDB_KEYTYPE_ARRAY_FLOAT:
            case REGISTERDB_KEYTYPE_ARRAY_DOUBLE:
            case REGISTERDB_KEYTYPE_ARRAY_STRING:
            case REGISTERDB_KEYTYPE_ARRAY_POINT:
            case REGISTERDB_KEYTYPE_ARRAY_SIZE:
            case REGISTERDB_KEYTYPE_ARRAY_RECT:

                if (!__LinkReady(FALSE, __nDataType))
                {
                    return -1;
                }

                __BinaryData_Link(FALSE, (BYTE*)&nDataNum, sizeof(nDataNum));
                return nDataNum;
                break;
        }

        return -1;
    }

    BOOL LinkArray(BOOL bSave, BYTE* pnArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_BYTE))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(BYTE) + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        if (nDataArray
            != nArrayNum) //kircheis_20170309Crash //현재 설정된 Camera의 영상 사이즈가 Job의 영상 사이즈와 다르면 문제가 된다.
            return FALSE;

        __BinaryData_Link(bSave, (BYTE*)pnArray, sizeof(BYTE) * nDataArray);

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, short* pnArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_SHORT))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(short) + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)pnArray, sizeof(short) * nDataArray);

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, BOOL* pbArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_BOOL))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(BOOL) + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)pbArray, sizeof(BOOL) * nDataArray);

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, long* plArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_LONG))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(long) + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(long) * nDataArray);

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, unsigned long* plArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_LONG))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(unsigned long) + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)plArray, sizeof(unsigned long) * nDataArray);

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, float* pfArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_FLOAT))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(float) + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)pfArray, sizeof(float) * nDataArray);

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, double* pfArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_DOUBLE))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(double) + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));
        __BinaryData_Link(bSave, (BYTE*)pfArray, sizeof(double) * nDataArray);

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, CString* pStrArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;
        long nArray = 0;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_STRING))
        {
            return FALSE;
        }

        if (bSave)
        {
            long nLength = 0;
            for (nArray = 0; nArray < nArrayNum; nArray++)
            {
                nLength += sizeof(long) + sizeof(WCHAR) * __GetStringBufferSize(pStrArray[nArray]);
            }

            __BinaryData_SetSize(nLength + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

        for (nArray = 0; nArray < nDataArray; nArray++)
        {
            __BinaryData_LinkString(bSave, pStrArray[nArray]);
        }

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, CPoint* pptArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;
        long nArray = 0;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_POINT))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(long) * 2 + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

        for (nArray = 0; nArray < nDataArray; nArray++)
        {
            __BinaryData_Link(bSave, (BYTE*)&pptArray[nArray].x, sizeof(long));
            __BinaryData_Link(bSave, (BYTE*)&pptArray[nArray].y, sizeof(long));
        }

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, CSize* pszArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;
        long nArray = 0;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_SIZE))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(long) * 2 + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

        for (nArray = 0; nArray < nDataArray; nArray++)
        {
            __BinaryData_Link(bSave, (BYTE*)&pszArray[nArray].cx, sizeof(long));
            __BinaryData_Link(bSave, (BYTE*)&pszArray[nArray].cy, sizeof(long));
        }

        return TRUE;
    }

    BOOL LinkArray(BOOL bSave, Ipvm::Rect32s* prtArray, long nArrayNum)
    {
        long nDataArray = nArrayNum;
        long nArray = 0;

        if (!__LinkReady(bSave, REGISTERDB_KEYTYPE_ARRAY_RECT))
        {
            return FALSE;
        }

        if (bSave)
        {
            __BinaryData_SetSize(nArrayNum * sizeof(long) * 4 + sizeof(nDataArray));
        }

        __BinaryData_Link(bSave, (BYTE*)&nDataArray, sizeof(nDataArray));

        for (nArray = 0; nArray < nDataArray; nArray++)
        {
            __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].m_left, sizeof(long));
            __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].m_top, sizeof(long));
            __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].m_right, sizeof(long));
            __BinaryData_Link(bSave, (BYTE*)&prtArray[nArray].m_bottom, sizeof(long));
        }

        return TRUE;
    }

    CiDataBase& GetSubDBFmt(LPCTSTR szFmt, ...)
    {
        CString strText;

        va_list argptr;

        va_start(argptr, szFmt);
        strText.FormatV(szFmt, argptr);
        va_end(argptr);

        return (*__pDataDB)[strText];
    }

    CiDataBase* FindSubDB(LPCTSTR strIndex)
    {
        auto findItem = __pDataDB->Lookup(strIndex);
        if (findItem == nullptr)
            return nullptr;

        return &findItem->m_value;
    }

    bool IsLinkDataSame(const CiDataBase& target) const
    {
        if (__nDataType != target.__nDataType)
            return false;
        if (__nDataTypeSub != target.__nDataTypeSub)
            return false;
        if (__nBinarySize != target.__nBinarySize)
            return false;

        if (__nBinarySize > 0)
        {
            if (memcmp(__pBinary, target.__pBinary, __nBinarySize) != 0)
                return false;
        }

        return true;
    }

    long GetSubDBNum()
    {
        return (long)(__pDataDB->GetCount());
    }

    POSITION GetSubDBName_FirstPosition()
    {
        return __pDataDB->GetStartPosition();
    }

    void GetSubDBName_NextPosition(POSITION& Position)
    {
        __pDataDB->GetNext(Position);
    }

    CString GetSubDBName(POSITION Position)
    {
        return __pDataDB->GetKeyAt(Position);
    }

    void DelSubDB(CString strSubDBName)
    {
        __pDataDB->RemoveKey(strSubDBName);
    }

    void Reset(void)
    {
        (*__pDataDB).RemoveAll();
    }

    BOOL Load(LPCTSTR strFileName)
    {
        CFile file;
        CFileException fe;

        if (!file.Open(strFileName, CFile::modeRead, &fe))
        {
            return FALSE;
        }

        ArchiveAllType ar(&file, CArchive::load);

        BOOL bReturn = Serialize(ar);

        ar.Close();
        file.Close();

        return bReturn;
    }

    BOOL Save(LPCTSTR strFileName)
    {
        CFile file;
        CFileException fe;

        if (!file.Open(strFileName, CFile::modeCreate | CFile::modeWrite))
        {
            return FALSE;
        }

        ArchiveAllType ar(&file, CArchive::store);

        BOOL bReturn = Serialize(ar);

        ar.Close();
        file.Close();

        return bReturn;
    }

    BOOL MakeBuffer(long i_nDefaultSize, long i_nGrowSize, BYTE** o_pResult, long* o_pnSize)
    {
        BYTE* pBuffer = new BYTE[i_nDefaultSize];
        CMemFile file(pBuffer, i_nDefaultSize, i_nGrowSize);
        CFileException fe;

        ArchiveAllType ar(&file, CArchive::store);
        BOOL bReturn = Serialize(ar);
        ar.Close();

        long nLength = (long)(file.GetLength());
        BYTE* pByte = file.Detach();

        *o_pResult = pByte;
        *o_pnSize = nLength;

        return bReturn;
    }

    BOOL Serialize(ArchiveAllType& ar)
    {
        if (ar.IsLoading())
        {
            // Load 시에는 일단 기존의 데이터를 제거한다
            (*__pDataDB).RemoveAll();
        }

        ULONG nBlockNum = (ULONG)__pDataDB->GetCount();
        ULONG nBlockCount = 0;

        // 하위 DB Serialize

        CString strBlockName;
        CString strKeyName;

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
        ar.Serialize_Element(__nDataTypeSub);
        ar.Serialize_Binary(__pBinary, __nBinarySize);

        return TRUE;
    }

private:
    long __GetStringBufferSize(CString& strData, WCHAR** pStrBuffer = NULL)
    {
#ifdef _UNICODE
        if (pStrBuffer)
        {
            *pStrBuffer = strData.GetBuffer();
        }
        return strData.GetLength() + 1;
#else
        long nStringLength = (long)(strData.GetLength());

        static std::vector<WCHAR> vecwStrData;
        vecwStrData.resize(strData.GetLength() + 1);
        MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, strData, nStringLength + 1, &vecwStrData[0], nStringLength);

        vecwStrData[nStringLength] = 0;

        if (pStrBuffer)
        {
            *pStrBuffer = &vecwStrData[0];
        }

        return nStringLength + 1;
#endif
    }

    void __BinaryData_SetSize(ULONG nSize)
    {
        delete[] __pBinary;

        __nBinarySize = nSize;
        __pBinary = new BYTE[__nBinarySize];
    }

    void __BinaryData_Link(BOOL bSave, BYTE* pData, long nSize)
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
    void __BinaryData_LinkString(BOOL bSave, CString& strData)
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

    BOOL __LinkReady(BOOL bSave, long nLinkType)
    {
        __nBinaryDataCount = 0;

        if (bSave)
        {
            // 다른 Type 이 공간에 들어가있는 곳에 무언가를 넣으려 한다
            ASSERT(__nDataType == REGISTERDB_KEYTYPE_NOSELECT || __nDataType == nLinkType);

            __nDataType = nLinkType;

            return TRUE;
        }
        else
        {
            if (__nDataType == nLinkType)
            {
                return TRUE;
            }

            if (__nDataType != REGISTERDB_KEYTYPE_NOSELECT)
            {
                // 다른 Type 이 공간에 들어가있는 곳에서 데이터를 읽으려 한다
                //ASSERT(!"뭐하는 짓인가?");

                // 일단 Default 값을 읽을 수 있게 빠져나오게 하였다.
            }
        }

        // 여기는 Loading 시 Key 가 없을 때 들어온다. 이때는 Default 값을 읽어라
        return FALSE;
    }

    long __nBinaryDataCount;
    long __nDataType;
    long __nDataTypeSub;
    BYTE* __pBinary;
    ULONG __nBinarySize;

    CAtlMap<CString, CiDataBase, CStringElementTraits<CString>>* __pDataDB; // 하위 DB
};

//======================================================================================================
// Object 들 첨부
//======================================================================================================
