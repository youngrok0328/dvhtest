//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "SurfaceAlgoPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CSurfaceAlgoPara::CSurfaceAlgoPara()
    : nAlignOption(enumAlign_Body)
    , m_min_dark_contrast(30.f)
    , m_min_bright_contrast(30.f)
    , m_low_Threshold(150)
    , m_high_Threshold(250)
    , m_min_dark_contrast_3D(9.f)
    , m_min_bright_contrast_3D(9.f)
    , m_dbackground_window_size_x(40.f)
    , m_dbackground_window_size_y(40.f)
    , nUse3DImageType(enumUse_Zmap)
    , fMinAreaBrightContrast(1.f)
    , fMinAreaDarkContrast(1.f)
    , m_mergeDistance(0.f)
    , m_mergeOnlyTheSameColor(FALSE)
    , fLocalContrastAreaSpan(5.f)
    // Auto Threshold Sub Area
    , m_offsetLeft_um(0.f)
    , m_offsetRight_um(0.f)
    , m_offsetTop_um(0.f)
    , m_offsetBottom_um(0.f)
    // Blob Filtering
    , fMinimumBrightBlobArea_umSqure(0.f) // unit : um^2
    , fMininumDarkBlobArea_umSqure(0.f) // unit : um^2
    , nMaxBrightBlobNum_New(BLOB_INFO_MAX_SIZE)
    , nMaxDarkBlobNum_New(BLOB_INFO_MAX_SIZE)
    , m_bisNormalThreshold(FALSE)
    , m_use2ndInspection(FALSE)
    , m_str2ndInspCode(_T(""))
    , m_useOnlyDLInsp(FALSE)
{
    m_rtROI_BCU.m_left = -100;
    m_rtROI_BCU.m_right = 100;
    m_rtROI_BCU.m_top = -100;
    m_rtROI_BCU.m_bottom = 100;
}

CSurfaceAlgoPara::CSurfaceAlgoPara(const CSurfaceAlgoPara& object)
    : m_ignore(object.m_ignore)
{
    *this = object;
}

CSurfaceAlgoPara::~CSurfaceAlgoPara()
{
}

void CSurfaceAlgoPara::Init()
{
    nAlignOption = enumAlign_Body;
    m_min_dark_contrast = 30.f;
    m_min_bright_contrast = 30.f;
    m_low_Threshold = 150;
    m_high_Threshold = 250;
    m_min_dark_contrast_3D = 9.f;
    m_min_bright_contrast_3D = 9.f;
    m_dbackground_window_size_x = 40.f;
    m_dbackground_window_size_y = 40.f;
    nUse3DImageType = enumUse_Zmap;
    fMinAreaBrightContrast = 1.f;
    fMinAreaDarkContrast = 1.f;
    m_mergeDistance = 0.f;
    m_mergeOnlyTheSameColor = FALSE;
    fLocalContrastAreaSpan = 5.f;

    m_rtROI_BCU.SetRect(-100.f, -100.f, 100.f, 100.f);

    m_maskOrder.Init();
    m_ignore.Init();

    m_offsetLeft_um = 30.f;
    m_offsetRight_um = 30.f;
    m_offsetTop_um = 30.f;
    m_offsetBottom_um = 3.f;

    // 2nd Inspection
    m_use2ndInspection = FALSE;
    m_str2ndInspCode = _T("");
    m_useOnlyDLInsp = FALSE;

    // Blob Filtering
    fMinimumBrightBlobArea_umSqure = 0.f;
    fMininumDarkBlobArea_umSqure = 0.f;

    m_bisNormalThreshold = FALSE;
}

float& CSurfaceAlgoPara::GetThresholdDark(bool is3D)
{
    if (is3D)
        return m_min_dark_contrast_3D;
    return m_min_dark_contrast;
}
float& CSurfaceAlgoPara::GetThresholdBright(bool is3D)
{
    if (is3D)
        return m_min_bright_contrast_3D;
    return m_min_bright_contrast;
}

long& CSurfaceAlgoPara::GetThresholdLow()
{
    return m_low_Threshold;
}

long& CSurfaceAlgoPara::GetThresholdHigh()
{
    return m_high_Threshold;
}

BOOL& CSurfaceAlgoPara::GetThresholdNormal()
{
    return m_bisNormalThreshold;
}

const float& CSurfaceAlgoPara::GetThresholdDark(bool is3D) const
{
    if (is3D)
        return m_min_dark_contrast_3D;
    return m_min_dark_contrast;
}
const float& CSurfaceAlgoPara::GetThresholdBright(bool is3D) const
{
    if (is3D)
        return m_min_bright_contrast_3D;
    return m_min_bright_contrast;
}

const long& CSurfaceAlgoPara::GetThresholdLow() const
{
    return m_low_Threshold;
}

const long& CSurfaceAlgoPara::GetThresholdHigh() const
{
    return m_high_Threshold;
}

const BOOL& CSurfaceAlgoPara::GetThresholdNormal() const
{
    return m_bisNormalThreshold;
}

BOOL CSurfaceAlgoPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;
    if (!db[_T("nAlignOption")].Link(bSave, nAlignOption))
        nAlignOption = CSurfaceAlgoPara::enumAlign_Body;
    if (!db[_T("MinDarkContrast")].Link(bSave, m_min_dark_contrast))
        m_min_dark_contrast = 30.f;
    if (!db[_T("MinBrightContrast")].Link(bSave, m_min_bright_contrast))
        m_min_bright_contrast = 30.f;
    if (!db[_T("MinDarkContrast_3D")].Link(bSave, m_min_dark_contrast_3D))
        m_min_dark_contrast_3D = 9.f;
    if (!db[_T("MinBrightContrast_3D")].Link(bSave, m_min_bright_contrast_3D))
        m_min_bright_contrast_3D = 9.f;
    if (!db[_T("LowThreshold")].Link(bSave, m_low_Threshold))
        m_low_Threshold = 150;
    if (!db[_T("HighThreshold")].Link(bSave, m_high_Threshold))
        m_high_Threshold = 250;
    if (!db[_T("fManualBackgroundGV")].Link(bSave, m_dbackground_window_size_x))
        m_dbackground_window_size_x = 40.f;
    if (!db[_T("fMinHistogramThreshold")].Link(bSave, m_dbackground_window_size_y))
        m_dbackground_window_size_y = 40.f;
    if (!db[_T("fMinAreaDarkContrast")].Link(bSave, fMinAreaDarkContrast))
        fMinAreaDarkContrast = 1.f;
    if (!db[_T("fMinAreaBrightContrast")].Link(bSave, fMinAreaBrightContrast))
        fMinAreaBrightContrast = 1.f;
    if (!db[_T("nUseVmapImage")].Link(bSave, nUse3DImageType))
        nUse3DImageType = enumUse_Zmap;
    if (!db[_T("fMergeDistance")].Link(bSave, m_mergeDistance))
        m_mergeDistance = 0.f;
    if (!db[_T("Merge Only The Same Color")].Link(bSave, m_mergeOnlyTheSameColor))
        m_mergeOnlyTheSameColor = FALSE;
    if (!db[_T("fLocalContrastAreaSpan")].Link(bSave, fLocalContrastAreaSpan))
        fLocalContrastAreaSpan = 5.f;

    // Blob Filtering
    if (!db[_T("fMinimumBrightBlobArea")].Link(bSave, fMinimumBrightBlobArea_umSqure))
        fMinimumBrightBlobArea_umSqure = 0.f;
    if (!db[_T("fMininumDarkBlobArea")].Link(bSave, fMininumDarkBlobArea_umSqure))
        fMininumDarkBlobArea_umSqure = 0.f;
    if (!db[_T("nMaxBrightBlobNum_New")].Link(bSave, nMaxBrightBlobNum_New))
        nMaxBrightBlobNum_New = BLOB_INFO_MAX_SIZE;
    if (!db[_T("nMaxDarkBlobNum_New")].Link(bSave, nMaxDarkBlobNum_New))
        nMaxDarkBlobNum_New = BLOB_INFO_MAX_SIZE;
    if (!db[_T("bThresholdTypeisNormal")].Link(bSave, m_bisNormalThreshold))
        m_bisNormalThreshold = FALSE;

    // 2nd Inspection
    if (!db[_T("Use2ndInspection")].Link(bSave, m_use2ndInspection))
        m_use2ndInspection = FALSE;
    if (!db[_T("str2ndInspCode")].Link(bSave, m_str2ndInspCode))
        m_str2ndInspCode = _T("");
    if (!db[_T("UseOnlyDLInsp")].Link(bSave, m_useOnlyDLInsp))
        m_useOnlyDLInsp = FALSE;

    if (!m_maskOrder.LinkDataBase(bSave, db[_T("MaskOrder")]))
    {
        m_maskOrder.Init();
    }

    if (!bSave)
    {
        for (long nOrder = 0; nOrder < m_maskOrder.GetCount(); nOrder++)
        {
            if (m_maskOrder.GetAt(nOrder).m_strName == _T("Component Origin_Mask Image"))
            {
                m_maskOrder.GetAt(nOrder).m_strName == _T("Measured - Component Mask with Pad");
            }
            else if (m_maskOrder.GetAt(nOrder).m_strName == _T("Component Mask Image"))
            {
                m_maskOrder.GetAt(nOrder).m_strName == _T("Measured - Component Mask"); //Measured - Component Mask
            }
        }
    }

    if (!m_ignore.LinkDataBase(bSave, db[_T("Ignore")]))
    {
        m_ignore.Init();
    }

    if (!bSave)
    {
        for (long nOrder = 0; nOrder < m_ignore.GetCount(); nOrder++)
        {
            if (m_ignore.GetAt(nOrder).strName == _T("Component Origin_Mask Image"))
            {
                m_ignore.GetAt(nOrder).strName == _T("Measured - Component Mask with Pad"); //
            }
            else if (m_ignore.GetAt(nOrder).strName == _T("Component Mask Image"))
            {
                m_ignore.GetAt(nOrder).strName == _T("Measured - Component Mask");
            }
        }
    }

    if (version == 0)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }
    else
    {
        if (!LinkEx(bSave, db[_T("rtROI")], m_rtROI_BCU))
        {
            m_rtROI_BCU = Ipvm::Rect32r(-100.f, -100.f, 100.f, 100.f);
        }

        if (!db[_T("Offset Left")].Link(bSave, m_offsetLeft_um))
            m_offsetLeft_um = 30.f;
        if (!db[_T("Offset Right")].Link(bSave, m_offsetRight_um))
            m_offsetRight_um = 30.f;
        if (!db[_T("Offset Top")].Link(bSave, m_offsetTop_um))
            m_offsetTop_um = 30.f;
        if (!db[_T("Offset Bottom")].Link(bSave, m_offsetBottom_um))
            m_offsetBottom_um = 30.f;
    }

    return TRUE;
}

CSurfaceAlgoPara& CSurfaceAlgoPara::operator=(const CSurfaceAlgoPara& object)
{
    nAlignOption = object.nAlignOption;
    nUse3DImageType = object.nUse3DImageType;
    fMinAreaBrightContrast = object.fMinAreaBrightContrast;
    fMinAreaDarkContrast = object.fMinAreaDarkContrast;
    m_mergeDistance = object.m_mergeDistance;
    m_mergeOnlyTheSameColor = object.m_mergeOnlyTheSameColor;
    fLocalContrastAreaSpan = object.fLocalContrastAreaSpan;
    m_offsetLeft_um = object.m_offsetLeft_um;
    m_offsetRight_um = object.m_offsetRight_um;
    m_offsetTop_um = object.m_offsetTop_um;
    m_offsetBottom_um = object.m_offsetBottom_um;
    fMinimumBrightBlobArea_umSqure = object.fMinimumBrightBlobArea_umSqure;
    fMininumDarkBlobArea_umSqure = object.fMininumDarkBlobArea_umSqure;
    nMaxBrightBlobNum_New = object.nMaxBrightBlobNum_New;
    nMaxDarkBlobNum_New = object.nMaxDarkBlobNum_New;
    m_dbackground_window_size_x = object.m_dbackground_window_size_x;
    m_dbackground_window_size_y = object.m_dbackground_window_size_y;
    m_rtROI_BCU = object.m_rtROI_BCU;
    m_ignore = object.m_ignore;
    m_maskOrder = object.m_maskOrder;

    m_min_dark_contrast = object.m_min_dark_contrast;
    m_min_bright_contrast = object.m_min_bright_contrast;
    m_low_Threshold = object.m_low_Threshold;
    m_high_Threshold = object.m_high_Threshold;
    m_min_dark_contrast_3D = object.m_min_dark_contrast_3D;
    m_min_bright_contrast_3D = object.m_min_bright_contrast_3D;
    m_bisNormalThreshold = object.m_bisNormalThreshold;

    return *this;
}
