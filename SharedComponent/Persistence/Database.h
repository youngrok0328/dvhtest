#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <ipvmbasedef.h>

//HDR_4_________________________________ Standard library headers
#include <atlcoll.h>
#include <map>
#include <vector>

//HDR_5_________________________________ Forward declarations
#if defined(IPVMLIB_MAJOR_VERSION) && IPVMLIB_MAJOR_VERSION >= 9
namespace Ipvm
{
struct Point32s2;
struct Size32s2;
struct Rect32s;
} // namespace Ipvm
#else
namespace IntekPlus
{
namespace VisionMadang
{
using Point32s2 = Point_32s_C2;
using Size32s2 = Size_32s_C2;
using Rect32s = Rect;
} // namespace VisionMadang
} // namespace IntekPlus
namespace Ipvm = IntekPlus::VisionMadang;
#endif
class ArchiveAllType;
typedef unsigned int uint32_t;

//HDR_6_________________________________ Header body
//
//======================================================================================================
// DB 내용
//======================================================================================================

class __INTEKPLUS_SHARED_PERSISTENCE_API__ Database
{
public:
    Database(void);
    Database(const Database& Object);

    ~Database(void);

public:
    long GetType(void);
    bool operator!=(const Database& Object);
    bool operator==(const Database& Object);
    Database& operator=(const Database& Object);
    Database& operator[](const wchar_t* strIndex);

    //=================================================================================
    // Link 관련
    //=================================================================================

    bool Link(bool bSave, uint8_t& nData);
    bool Link(bool bSave, int16_t& nData);
    bool Link(bool bSave, int32_t& bData);
    bool Link(bool bSave, bool& bData);
    bool Link(bool bSave, long& nData, long nDef = 0);
    bool Link(bool bSave, uint32_t& nData);
    bool Link(bool bSave, unsigned long& nData);
    bool Link(bool bSave, float& fData);
    bool Link(bool bSave, double& fData);
    bool Link(bool bSave, CStringW& strData);
    bool Link(bool bSave, CPoint& ptData);
    bool Link(bool bSave, Ipvm::Point32s2& ptData);
    bool Link(bool bSave, CSize& szData);
    bool Link(bool bSave, Ipvm::Size32s2& szData);
    bool Link(bool bSave, CRect& rtData);
    bool Link(bool bSave, Ipvm::Rect32s& rtData);
    bool Link(bool bSave, int64_t& nData);
    bool Link(bool bSave, uint64_t& nData);

    template<typename T>
    bool Link(bool bSave, std::vector<T>& vecData)
    {
        long nElementNum = (long)vecData.size();

        bool ret = (*this)[_T("size")].Link(bSave, nElementNum);

        if (true != ret)
        {
            nElementNum = 0;
        }

        if (!bSave)
        {
            vecData.resize(nElementNum);
        }

        if (nElementNum > 0)
        {
            T* pT = &vecData[0];
            (*this)[_T("vector")].LinkArray(bSave, pT, nElementNum);
        }

        return ret;
    }

    template<typename T>
    bool LinkVectorClass(bool bSave, std::vector<T>& vecData)
    {
        long nElementNum = (long)vecData.size();

        bool ret = (*this)[_T("size")].Link(bSave, nElementNum);

        if (true != ret)
        {
            nElementNum = 0;
        }

        if (!bSave)
        {
            vecData.resize(nElementNum);
        }

        CStringW str;
        for (int i = 0; i < nElementNum; i++)
        {
            str.Format(_T("%d"), i);
            (*this)[str].Link(bSave, vecData[i]);
        }

        return ret;
    }

    template<typename T1, typename T2>
    bool Link(bool bSave, std::map<T1, T2>& mapData)
    {
        long nElementNum = (long)mapData.size();

        bool ret = (*this)[_T("size")].Link(bSave, nElementNum);

        if (true != ret)
        {
            nElementNum = 0;
        }

        if (!bSave)
        {
            mapData.clear();
        }

        CStringW str;
        auto it = mapData.begin();

        for (int i = 0; i < nElementNum; i++)
        {
            T1 First;
            T2 Second;

            if (bSave)
            {
                First = it->first;
                Second = it->second;
            }

            str.Format(_T("%d"), i);
            (*this)[str][_T("First")].Link(bSave, First);
            (*this)[str][_T("Second")].Link(bSave, Second);

            if (bSave)
                it++;
            else
            {
                mapData[First] = Second;
            }
        }

        return ret;
    }

    template<typename T>
    bool Link(bool bSave, T& Data)
    {
        return Data.LinkDataBase(bSave, *this);
    }

    bool SafeLink(bool bSave, long& nData);
    bool SafeLink(bool bSave, uint32_t& nData);
    bool SafeLink(bool bSave, uint64_t& nData);

    template<typename T>
    bool SafeLink(bool bSave, std::vector<T>& vecData)
    {
        return __SafeLinkVector_Base(bSave, vecData);
    }

    long GetLinkArrayNum();
    bool LinkArray(bool bSave, uint8_t* pnArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, int16_t* pnArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, int32_t* pbArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, uint32_t* pbArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, long* plArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, unsigned long* plArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, float* pfArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, double* pfArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, CStringW* pStrArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, CPoint* pptArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, Ipvm::Point32s2* pptArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, CSize* pszArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, Ipvm::Size32s2* pszArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, CRect* prtArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, Ipvm::Rect32s* prtArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, int64_t* plArray, const uint32_t elementCount);
    bool LinkArray(bool bSave, uint64_t* plArray, const uint32_t elementCount);
    bool SafeLinkArray(bool bSave, uint64_t* plArray, const uint32_t elementCount);

    Database& GetSubDBFmt(const wchar_t* szFmt, ...);
    long GetSubDBNum();
    POSITION GetSubDBName_FirstPosition();
    void GetSubDBName_NextPosition(POSITION& Position);
    const wchar_t* GetSubDBName(POSITION Position);
    void DelSubDB(const wchar_t* strSubDBName);
    void Reset(void);

    bool Load(const void* buffer, long bufferSize);
    bool Load(LPCTSTR strFileName);
    bool Save(LPCTSTR strFileName);

    void GetBuffer(void** buffer, long* bufferSize);
    void ReleaseBuffer(void* buffer);

    bool Serialize(ArchiveAllType& ar);

private:
    bool Load(CFile& File);
    bool Save(CFile& File);
    template<typename T>
    bool __SafeLinkVector_Base(bool bSave, std::vector<T>& vecData)
    {
        long nElementNum = (long)vecData.size();

        bool ret = (*this)[_T("size")].Link(bSave, nElementNum);

        if (true != ret)
        {
            nElementNum = 0;
        }

        if (!bSave)
        {
            vecData.resize(nElementNum);
        }

        if (nElementNum > 0)
        {
            T* pT = &vecData[0];
            (*this)[_T("vector")].SafeLinkArray(bSave, pT, nElementNum);
        }

        return ret;
    }

    long __GetStringBufferSize(CStringW& strData, WCHAR** pStrBuffer = nullptr);
    void __BinaryData_SetSize(ULONG nSize);
    void __BinaryData_Link(bool bSave, BYTE* pData, long nSize);
    void __BinaryData_LinkString(bool bSave, CStringW& strData);
    bool __LinkReady(bool bSave, long nLinkType);

    bool __Serialize_Old(ArchiveAllType& ar);

    long __nBinaryDataCount;
    long __nDataType;
    BYTE* __pBinary;
    ULONG __nBinarySize;

    CAtlMap<CStringW, Database, CStringElementTraits<CStringW>>* __pDataDB; // 하위 DB
};
