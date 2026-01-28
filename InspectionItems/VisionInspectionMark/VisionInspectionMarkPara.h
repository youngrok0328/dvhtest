#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Point32s2.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
enum enumPreProcMode_Mark // Mark Pre Proc Mode 추가 by kjy, 2007.10.08
{
    ENUM_PREPROC_NOTHING,
    ENUM_PREPROC_MEDIANSMOOTHING,
    ENUM_PREPROC_SMOOTHINGMEDIAN,
    ENUM_PREPROC_MEDIAN,
    ENUM_PREPROC_SMOOTHING,
};

enum enumTeachingThresholdMode
{
    ENUM_THRESHOLD_MANUAL,
    ENUM_THRESHOLD_AUTOSEMI,
    ENUM_THRESHOLD_AUTOFULL,
};

enum enumMarkDirection
{
    ENUM_MARKDIR_HOR,
    ENUM_MARKDIR_VER,
};

enum MarkAngleReference
{
    Mark_Angle_Ref_Teaching_Image,
    Mark_Angle_Ref_Body_Align,
};

enum MarkInspectionMode
{
    Mark_InspMode_Normal, //기존 방식
    Mark_InspMode_Simple, //Empty 개념 방식 Blob Size로만 체크
};

enum MarkROIType
{
    MARK_ROI_TEACH,
    MARK_ROI_MERGE,
    MARK_ROI_IGNORE,
};

static LPCTSTR g_szMarkROITypeName[] = {
    _T("TEACH"),
    _T("MERGE"),
    _T("IGNORE"),
};

struct sMarkROI_Info
{
    CString strID;
    CString strName;
    float fOffsetX;
    float fOffsetY;
    float fWidth;
    float fLength;
    float fAngle;
    long nType;

    void Copy(sMarkROI_Info i_input);
    void Clear();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

struct sMarkReadDataInfo
{
    CString strID;
    CString strName;
    float fOffsetX;
    float fOffsetY;
    float fWidth;
    float fLength;
    float fAngle;
    CString strType;
};

struct sMarkMapInfo
{
    sMarkROI_Info sTeachROI_Info;
    std::vector<sMarkROI_Info> vecIgnoreROI_Info;
    std::vector<sMarkROI_Info> vecMergeROI_Info;

    void Clear();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
};

struct sReadMarkMapInfo //mc_Mark Read시 전체 Map에 대한 정보
{
    std::vector<sMarkROI_Info> vecTeachROI_Info;
    std::vector<sMarkROI_Info> vecIgnoreROI_Info;
    std::vector<sMarkROI_Info> vecMergeROI_Info;

    void InitData();
};

struct SMarkAlgorithmParameter
{
    long nBlobSizeMin;
    long nBlobSizeMax;
    long nMinContrastForMarkCnt;
    float fMarkMergeGap;

    BOOL bUseAllCharReinsp;
    long nAllCharReinspNumber;
    long nAllCharThresholdDeviation;

    BOOL bUseEachCharReinsp;
    long nEachCharReinspNumber;
    long nEachCharThresholdDeviation;

    long nThinningIntensity;
    float fBlobSizeMergeGap;

    long nThresholdMode;
    long nMarkRoiMargin;

    void Init()
    {
        nBlobSizeMin = 30;
        nBlobSizeMax = 10000;
        nMinContrastForMarkCnt = 30;
        fMarkMergeGap = 0.f;

        bUseAllCharReinsp = TRUE;
        nAllCharReinspNumber = 3;
        nAllCharThresholdDeviation = 10;

        bUseEachCharReinsp = FALSE;
        nEachCharReinspNumber = 3;
        nEachCharThresholdDeviation = 10;

        nThinningIntensity = 1;
        fBlobSizeMergeGap = 5.0f;

        nThresholdMode = ENUM_THRESHOLD_AUTOSEMI;
        nMarkRoiMargin = 3;
    };

    SMarkAlgorithmParameter()
    {
        Init();
    }
};

class TeachMergeInfo
{
public:
    Ipvm::Rect32s m_roi;
    Ipvm::Point32s2 m_roiLtFromPackCenter;

    TeachMergeInfo()
        : m_roi(0, 0, 0, 0)
        , m_roiLtFromPackCenter(0, 0)
    {
    }
};

class CiDataBase;

class VisionInspectionMarkPara
{
public:
    VisionInspectionMarkPara(VisionProcessing& parent);
    ~VisionInspectionMarkPara(void);

    void Init();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName, const long nMarkThreshold); //kircheis_TxtRecipe

    VisionInspectionMarkPara& operator=(const VisionInspectionMarkPara& Src);

    ImageProcPara m_ImageProcMangePara;

    SMarkAlgorithmParameter sMarkAlgoParam;
    long m_nMarkPreProcMode;
    long m_nIgnoreROINum;
    BOOL m_bLocatorTargetROI;
    BOOL m_bWhiteBackGround;

    BOOL m_bInitialIgnoreBox; //kircheis_for_ITEK

    long m_nLocSearchOffsetX;
    long m_nLocSearchOffsetY;
    long m_nCharSearchOffsetX;
    long m_nCharSearchOffsetY;
    long m_nImageSampleRate;

    long m_nMarkDirection;

    //20141002 현건
    std::vector<BOOL> m_vecbSelectIgnore;

    BOOL m_bUseNormalSizeInspMode;
    long m_nSelectAngleReference;

    long m_nMarkAngle_deg;
    long m_nMarkInspMode; // 0: Mormal, 1: Simple
    long m_nIgnoreDilateCount;
    BOOL m_bAlignAreaIgnore; //TRUE : Use, FALSE : NotUse

    long m_teach_merge_num; // -1이면 ROI Merge or Merge Rois 수

    std::vector<TeachMergeInfo> m_teach_merge_infos; // Merge ROI 리스트

    std::vector<Ipvm::Rect32s> m_vecrtUserNumIgnore;
    std::vector<Ipvm::Rect32s> m_vecrtUserIgnore; //kircheis_AutoMark
    std::vector<Ipvm::Rect32s> m_vecrtUserSplite;

    //{{//kircheis_AutoTeach
    std::vector<Ipvm::Point32s2> m_vecptUserNumIgnoreRoiLtFromPackCenter;
    std::vector<Ipvm::Point32s2> m_vecptUserIgnoreRoiLtFromPackCenter;
    //}}

    //kk Mark Map
    long m_nROISettingMethod;

    sMarkMapInfo m_sMarkMapInfo_um; //원본, Load 및 Save에 사용

    void WriteMarkMapInfoToCSV(CString strFileFullPath);
};