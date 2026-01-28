//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionDeviceResult.h"

//CPP_2_________________________________ This project's headers
#include "Analysis.h"
#include "ArchiveAllType.h"
#include "VisionHostBaseDef.h"
#include "VisionInspectionResult.h"
#include "VisionWarpageShapeResult.h"

//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionDeviceResult::VisionDeviceResult()
    : m_reviewRoi(0, 0, 0, 0)
    , m_analysis(new Analysis)
{
    Init();
}

VisionDeviceResult::~VisionDeviceResult()
{
#if defined(USE_BY_HANDLER)
    for (int n = 0; n < (long)(m_vecVisionInspResult.size()); n++)
    {
        if (m_vecVisionInspResult[n] != NULL)
            delete m_vecVisionInspResult[n];
        m_vecVisionInspResult[n] = NULL;
    }
    m_vecVisionInspResult.clear();
    for (int n = 0; n < (long)(m_vecTempResult.size()); n++)
    {
        if (m_vecTempResult[n] != NULL)
            delete m_vecTempResult[n];
        m_vecTempResult[n] = NULL;
    }
    m_vecTempResult.clear();

#else
    const long nVisionInspResultNum = (long)m_vecVisionInspResult.size();
    for (long nIdx = 0; nIdx < nVisionInspResultNum; nIdx++)
        delete m_vecVisionInspResult[nIdx];
    m_vecVisionInspResult.clear();
    //std::vector<VisionInspectionResult*>().swap(m_vecVisionInspResult);

#endif
    delete m_analysis;
}

Ipvm::Rect32s VisionDeviceResult::GetRoiForReview(const Ipvm::Rect32s& roi) const
{
    Ipvm::Rect32s rtDummy(1, 1, 2, 2);
    Ipvm::Rect32s rtResult = roi - m_reviewRoi.TopLeft();
    Ipvm::Point32s2 ptRectCenter = rtResult.CenterPoint();

    if (ptRectCenter.m_x < 0 || ptRectCenter.m_y < 0)
        return rtDummy;

    rtResult.m_left = (long)max(rtResult.m_left, 1);
    rtResult.m_top = (long)max(rtResult.m_top, 1);

    return rtResult;
}

void VisionDeviceResult::Init()
{
    m_analysis->Init();
    m_strMajor2DID.Empty();
    m_vecStrSub2DID.clear();
    m_vecStrBaby2DID.clear();
    m_nProbeID = -1;
    m_nTrayID = 0;
    m_nInspectionRepeatIndex = 0; //mc_KMAT Repeat Crash
    m_nPocketID = 0;
    m_nScanID = 0;
    m_nPane = 0;
    m_nTotalResult = NOT_MEASURED;
    m_strReviewColorImageName = _T("");
    m_reviewRoi = Ipvm::Rect32s(0, 0, 0, 0);
    m_visionWarpageShapeResult.Init();
    m_vecVisionLidWarpageShapeResult.clear(); //kircheis_InspLid
    InitSystemTime();
    m_pocketShiftY_TR = -999.f; //Pocket Shift Y [Unit:mm](TR기준)
}

void VisionDeviceResult::InitSystemTime()
{
    m_inspectionStartTime.wDay = 0;
    m_inspectionStartTime.wDayOfWeek = 0;
    m_inspectionStartTime.wHour = 0;
    m_inspectionStartTime.wMilliseconds = 0;
    m_inspectionStartTime.wMinute = 0;
    m_inspectionStartTime.wMonth = 0;
    m_inspectionStartTime.wSecond = 0;
    m_inspectionStartTime.wYear = 0;
}

void VisionDeviceResult::Serialize(ArchiveAllType& ar)
{
    long nDateVersion = 20230207;

    ar.Serialize_Element(nDateVersion);

    if (nDateVersion != 20230207)
    {
        ASSERT(!_T("Fuck the DeviceResult!"));
    }
    else
    {
        m_analysis->Serialize(ar);

        ar.Serialize_Element(m_nProbeID);
        ar.Serialize_Element(m_nThreadID);
        ar.Serialize_Element(m_nTrayID);
        ar.Serialize_Element(m_nInspectionRepeatIndex);
        ar.Serialize_Element(m_nPocketID);
        ar.Serialize_Element(m_nScanID);
        ar.Serialize_Element(m_nPane);
        ar.Serialize_Element(m_nTotalResult);
        ar.Serialize_Element(m_strReviewColorImageName);
        ar.Serialize_Element(m_reviewRoi);

        if (ar.IsStoring())
        {
            ar << (long)(m_vecVisionInspResult.size());
            for (auto* result : m_vecVisionInspResult)
            {
                ar << *result;
            }

            ar << m_strMajor2DID;
            long n2DID_Num = (long)m_vecStrSub2DID.size();
            ar << n2DID_Num;
            for (long nID = 0; nID < n2DID_Num; nID++)
                ar << m_vecStrSub2DID[nID];

            n2DID_Num = (long)m_vecStrBaby2DID.size();
            ar << n2DID_Num;
            for (long nID = 0; nID < n2DID_Num; nID++)
                ar << m_vecStrBaby2DID[nID];

            ar << m_visionWarpageShapeResult;

            long nLidNum = (long)m_vecVisionLidWarpageShapeResult.size();
            ar << nLidNum;
            for (long nLid = 0; nLid < nLidNum; nLid++)
                ar << m_vecVisionLidWarpageShapeResult[nLid];

            ar << m_inspectionStartTime.wYear;
            ar << m_inspectionStartTime.wMonth;
            ar << m_inspectionStartTime.wDayOfWeek;
            ar << m_inspectionStartTime.wDay;
            ar << m_inspectionStartTime.wHour;
            ar << m_inspectionStartTime.wMinute;
            ar << m_inspectionStartTime.wSecond;
            ar << m_inspectionStartTime.wMilliseconds;

            ar << m_pocketShiftY_TR;
        }
        else
        {
            long nCount;
            ar >> nCount;

            TempResult.resize(nCount,
                VisionInspectionResult(nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                    II_RESULT_TYPE::MEASURED, HostReportCategory::MEASURE));
            m_vecTempResult.resize(nCount);
            m_vecVisionInspResult.clear();

            for (int n = 0; n < nCount; n++)
            {
                VisionInspectionResult* pData = new VisionInspectionResult(nullptr, nullptr, nullptr, nullptr, nullptr,
                    nullptr, nullptr, II_RESULT_TYPE::MEASURED, HostReportCategory::MEASURE);
                ar >> *pData; // TempResult[n];
                m_vecVisionInspResult.push_back(pData); // &TempResult[n]);
            }

            ar >> m_strMajor2DID;
            long n2DID_Num = 0;
            ar >> n2DID_Num;
            m_vecStrSub2DID.clear();
            m_vecStrSub2DID.resize(n2DID_Num);
            for (long nID = 0; nID < n2DID_Num; nID++)
                ar >> m_vecStrSub2DID[nID];

            n2DID_Num = 0;
            ar >> n2DID_Num;
            m_vecStrBaby2DID.clear();
            m_vecStrBaby2DID.resize(n2DID_Num);
            for (long nID = 0; nID < n2DID_Num; nID++)
                ar >> m_vecStrBaby2DID[nID];

            ar >> m_visionWarpageShapeResult;

            long nLidNum = 0;
            m_vecVisionLidWarpageShapeResult.clear();
            ar >> nLidNum;
            m_vecVisionLidWarpageShapeResult.resize(nLidNum);
            for (long nLid = 0; nLid < nLidNum; nLid++)
                ar >> m_vecVisionLidWarpageShapeResult[nLid];

            ar >> m_inspectionStartTime.wYear;
            ar >> m_inspectionStartTime.wMonth;
            ar >> m_inspectionStartTime.wDayOfWeek;
            ar >> m_inspectionStartTime.wDay;
            ar >> m_inspectionStartTime.wHour;
            ar >> m_inspectionStartTime.wMinute;
            ar >> m_inspectionStartTime.wSecond;
            ar >> m_inspectionStartTime.wMilliseconds;

            ar >> m_pocketShiftY_TR;
        }
    }

#ifdef EXCHANGE_SURFACE_ITEM_NAME
    if (ar.IsLoading())
    {
        for (auto* inspResult : m_vecVisionInspResult)
        {
            if (inspResult->m_hostReportCategory != HostReportCategory::SURFACE)
            {
                continue;
            }

            inspResult->m_resultName = inspResult->m_inspName;
            inspResult->m_inspName = _T("Surface");
            inspResult->m_fullName.Format(
                _T("[%s]%s"), LPCTSTR(inspResult->m_inspName), LPCTSTR(inspResult->m_resultName));
        }
    }
#endif
}

void VisionDeviceResult::SetInspectResults(long nTheadNum, long nTrayID, long nInspectionRepeatIndex, long nPocketID,
    long nScanID, long nPane, long nTotalReuslt, std::vector<VisionInspectionResult*> vecVisionInspResult,
    CString strMajorID, std::vector<CString> vecStrSub2DID, std::vector<CString> vecStrBaby2DID,
    VisionWarpageShapeResult visionWarpageShapeResult,
    std::vector<VisionWarpageShapeResult> vecLidWarpageShapeResult) //kircheis_InspLid
{
    m_nThreadID = nTheadNum;
    m_nTrayID = nTrayID;
    m_nInspectionRepeatIndex = nInspectionRepeatIndex;
    m_nPocketID = nPocketID;
    m_nScanID = nScanID;
    m_nPane = nPane;
    m_nTotalResult = nTotalReuslt;
    //m_vecVisionInspResult = vecVisionInspResult;

    const long nInspNum = (long)vecVisionInspResult.size();
    //{{//원래 필요는 없는데 혹시나 해서 넣는다. 외부에서 신규 객체가 아닌 재활용 객체를 사용할 수도 있다
    const long nVisionInspResultSize = (long)m_vecVisionInspResult.size();
    for (long nIdx = 0; nIdx < nVisionInspResultSize; nIdx++)
        delete m_vecVisionInspResult[nIdx];
    //}}
    m_vecVisionInspResult.clear();
    m_vecVisionInspResult.resize(nInspNum);
    for (long nInsp = 0; nInsp < nInspNum; nInsp++)
        m_vecVisionInspResult[nInsp] = new VisionInspectionResult(*vecVisionInspResult[nInsp]);

    m_strMajor2DID = strMajorID;

    long n2DID_Num = (long)vecStrSub2DID.size();
    m_vecStrSub2DID.clear();
    m_vecStrSub2DID.resize(n2DID_Num);
    for (long nID = 0; nID < n2DID_Num; nID++)
        m_vecStrSub2DID[nID] = vecStrSub2DID[nID];

    n2DID_Num = (long)vecStrBaby2DID.size();
    m_vecStrBaby2DID.clear();
    m_vecStrBaby2DID.resize(n2DID_Num);
    for (long nID = 0; nID < n2DID_Num; nID++)
        m_vecStrBaby2DID[nID] = vecStrBaby2DID[nID];

    m_visionWarpageShapeResult.SetResult(visionWarpageShapeResult);

    m_vecVisionLidWarpageShapeResult.clear();
    long nLidNum = (long)vecLidWarpageShapeResult.size();
    m_vecVisionLidWarpageShapeResult.resize(nLidNum);
    for (long nLid = 0; nLid < nLidNum; nLid++)
        m_vecVisionLidWarpageShapeResult[nLid] = vecLidWarpageShapeResult[nLid];

    m_pocketShiftY_TR = -999.f;
}

void VisionDeviceResult::SetInspectResults(long nTheadNum, long nTrayID, long nInspectionRepeatIndex, long nPocketID,
    long nScanID, long nPane, long nTotalReuslt, CString strColorImageName,
    std::vector<VisionInspectionResult*> vecVisionInspResult, CString strMajorID, std::vector<CString> vecStrSub2DID,
    std::vector<CString> vecStrBaby2DID, VisionWarpageShapeResult visionWarpageShapeResult,
    std::vector<VisionWarpageShapeResult> vecLidWarpageShapeResult, float pocketShiftY_TR)
{
    m_nThreadID = nTheadNum;
    m_nTrayID = nTrayID;
    m_nInspectionRepeatIndex = nInspectionRepeatIndex;
    m_nPocketID = nPocketID;
    m_nScanID = nScanID;
    m_nPane = nPane;
    m_nTotalResult = nTotalReuslt;
    m_strReviewColorImageName = strColorImageName; // 20190619 Color Image Name
    //m_vecVisionInspResult = vecVisionInspResult;

    const long nInspNum = (long)vecVisionInspResult.size();
    //{{//원래 필요는 없는데 혹시나 해서 넣는다. 외부에서 신규 객체가 아닌 재활용 객체를 사용할 수도 있다
    const long nVisionInspResultSize = (long)m_vecVisionInspResult.size();
    for (long nIdx = 0; nIdx < nVisionInspResultSize; nIdx++)
        delete m_vecVisionInspResult[nIdx];
    //}}
    m_vecVisionInspResult.clear();
    m_vecVisionInspResult.resize(nInspNum);
    for (long nInsp = 0; nInsp < nInspNum; nInsp++)
        m_vecVisionInspResult[nInsp] = new VisionInspectionResult(*vecVisionInspResult[nInsp]);

    m_strMajor2DID = strMajorID;

    long n2DID_Num = (long)vecStrSub2DID.size();
    m_vecStrSub2DID.clear();
    m_vecStrSub2DID.resize(n2DID_Num);
    for (long nID = 0; nID < n2DID_Num; nID++)
        m_vecStrSub2DID[nID] = vecStrSub2DID[nID];

    n2DID_Num = (long)vecStrBaby2DID.size();
    m_vecStrBaby2DID.clear();
    m_vecStrBaby2DID.resize(n2DID_Num);
    for (long nID = 0; nID < n2DID_Num; nID++)
        m_vecStrBaby2DID[nID] = vecStrBaby2DID[nID];

    m_visionWarpageShapeResult.SetResult(visionWarpageShapeResult);

    m_vecVisionLidWarpageShapeResult.clear();
    long nLidNum = (long)vecLidWarpageShapeResult.size();
    m_vecVisionLidWarpageShapeResult.resize(nLidNum);
    for (long nLid = 0; nLid < nLidNum; nLid++)
        m_vecVisionLidWarpageShapeResult[nLid] = vecLidWarpageShapeResult[nLid];

    m_pocketShiftY_TR = pocketShiftY_TR;
}
#if defined(USE_BY_HANDLER)

void VisionDeviceResult::SetInspectResults(long nTheadNum, long nTrayID, long nPocketID, long nPane, long nTotalReuslt,
    std::vector<VisionInspectionResult> vecVisionInspResult)
{
    //	핸들러가 만든 결과는 무조건 -1로 한다.
    m_nThreadID = -1; // nTheadNum;
    m_nTrayID = nTrayID;
    m_nPocketID = nPocketID;
    m_nPane = nPane;
    m_nTotalResult = nTotalReuslt;
    int nCount = vecVisionInspResult.size();
    m_vecVisionInspResult.clear();
    m_vecTempResult.resize(nCount);
    for (int n = 0; n < nCount; n++)
    {
        VisionInspectionResult* pData = new VisionInspectionResult(vecVisionInspResult[n]);
        m_vecVisionInspResult.push_back(pData);
    }
    // Invalid일경우 사용하므로 2D ID는 아몰랑
    m_strMajor2DID.Empty();
    m_vecStrSub2DID.clear();
    m_vecStrBaby2DID.clear();
    m_visionWarpageShapeResult.Init();
    m_pocketShiftY_TR = -999.f;
}
void VisionDeviceResult::SetInspectResults(std::vector<VisionInspectionResult> vecVisionInspResult)
{
    int nCount = vecVisionInspResult.size();
    m_vecVisionInspResult.clear();
    for (int n = 0; n < nCount; n++)
    {
        m_vecVisionInspResult.push_back(&vecVisionInspResult[n]);
    }
}

VisionDeviceResult& VisionDeviceResult::operator=(const VisionDeviceResult& Obj)
{
    *m_analysis = *Obj.m_analysis;
    m_nThreadID = Obj.m_nThreadID;
    m_nProbeID = Obj.m_nProbeID;
    m_nTrayID = Obj.m_nTrayID;
    m_nInspectionRepeatIndex = Obj.m_nInspectionRepeatIndex;
    m_nPocketID = Obj.m_nPocketID;
    m_nScanID = Obj.m_nScanID;
    m_nPane = Obj.m_nPane;
    m_nTotalResult = Obj.m_nTotalResult;
    m_reviewRoi = Obj.m_reviewRoi;
    TempResult = Obj.TempResult;
    int nCount = m_vecVisionInspResult.size();
    int nCountObj = Obj.m_vecVisionInspResult.size();
    if (nCountObj != nCount)
    {
        for (int n = 0; n < m_vecVisionInspResult.size(); n++)
        {
            if (m_vecVisionInspResult[n] != NULL)
                delete m_vecVisionInspResult[n];
            m_vecVisionInspResult[n] = NULL;
        }
        m_vecVisionInspResult.resize(nCountObj);
    }
    for (int n = 0; n < nCountObj; n++)
    {
        if (m_vecVisionInspResult[n] == NULL)
            m_vecVisionInspResult[n] = new VisionInspectionResult();
        *m_vecVisionInspResult[n] = *Obj.m_vecVisionInspResult[n];
    }
    nCount = m_vecTempResult.size();
    nCountObj = Obj.m_vecTempResult.size();
    if (nCountObj != nCount)
    {
        for (int n = 0; n < m_vecTempResult.size(); n++)
        {
            if (m_vecTempResult[n] != NULL)
                delete m_vecTempResult[n];
            m_vecTempResult[n] = NULL;
        }
        m_vecTempResult.resize(nCountObj);
    }
    for (int n = 0; n < nCountObj; n++)
    {
        if (m_vecTempResult[n] == NULL)
            m_vecTempResult[n] = new VisionInspectionResult();
        if (Obj.m_vecTempResult[n] == NULL)
            continue;
        *m_vecTempResult[n] = *Obj.m_vecTempResult[n];
    }

    m_strMajor2DID = Obj.m_strMajor2DID;
    m_vecStrSub2DID = Obj.m_vecStrSub2DID;
    m_vecStrBaby2DID = Obj.m_vecStrBaby2DID;
    m_strReviewColorImageName = Obj.m_strReviewColorImageName;
    m_visionWarpageShapeResult = Obj.m_visionWarpageShapeResult;
    m_inspectionStartTime = Obj.m_inspectionStartTime;

    m_pocketShiftY_TR = Obj.m_pocketShiftY_TR;

    return *this;
}
void VisionDeviceResult::DeleteMemVisionResult()
{
    for (int n = 0; n < m_vecVisionInspResult.size(); n++)
    {
        if (m_vecVisionInspResult[n] != NULL)
        {
            delete m_vecVisionInspResult[n];
        }
        m_vecVisionInspResult[n] = NULL;
    }
    for (int n = 0; n < m_vecTempResult.size(); n++)
    {
        if (m_vecTempResult[n] != NULL)
            delete m_vecTempResult[n];
        m_vecTempResult[n] = NULL;
    }
}

#endif

void VisionDeviceResult::SetinspectionStartTime(const SYSTEMTIME i_SysTime)
{
    m_inspectionStartTime = i_SysTime;
}

SYSTEMTIME VisionDeviceResult::GetinspectionStartTime()
{
    return m_inspectionStartTime;
}
