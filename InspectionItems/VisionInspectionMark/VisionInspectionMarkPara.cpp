//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionMarkPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionMarkPara::VisionInspectionMarkPara(VisionProcessing& parent)
    : m_ImageProcMangePara(parent)
{
    Init();
}

VisionInspectionMarkPara::~VisionInspectionMarkPara(void)
{
}

VisionInspectionMarkPara& VisionInspectionMarkPara::operator=(const VisionInspectionMarkPara& Src)
{
    m_teach_merge_num = Src.m_teach_merge_num;
    m_teach_merge_infos = Src.m_teach_merge_infos;

    m_nMarkPreProcMode = Src.m_nMarkPreProcMode;
    m_nIgnoreROINum = Src.m_nIgnoreROINum;
    m_bLocatorTargetROI = Src.m_bLocatorTargetROI;
    m_bWhiteBackGround = Src.m_bWhiteBackGround;
    m_vecrtUserIgnore = Src.m_vecrtUserIgnore;
    m_vecrtUserNumIgnore = Src.m_vecrtUserNumIgnore;

    m_nLocSearchOffsetX = Src.m_nLocSearchOffsetX;
    m_nLocSearchOffsetY = Src.m_nLocSearchOffsetY;
    m_nCharSearchOffsetX = Src.m_nCharSearchOffsetX;
    m_nCharSearchOffsetY = Src.m_nCharSearchOffsetY;
    m_nImageSampleRate = Src.m_nImageSampleRate;
    m_nMarkAngle_deg = Src.m_nMarkAngle_deg;
    m_nMarkDirection = Src.m_nMarkDirection;

    m_vecptUserNumIgnoreRoiLtFromPackCenter = Src.m_vecptUserNumIgnoreRoiLtFromPackCenter;
    m_vecptUserIgnoreRoiLtFromPackCenter = Src.m_vecptUserIgnoreRoiLtFromPackCenter;

    sMarkAlgoParam = Src.sMarkAlgoParam;

    //20141004 현건
    m_vecbSelectIgnore = Src.m_vecbSelectIgnore;

    m_bInitialIgnoreBox = Src.m_bInitialIgnoreBox; //kircheis_for_ITEK

    return *this;
}

BOOL VisionInspectionMarkPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long nSize(0);
    long nAutoTeachParamSize(0); //kircheis_Crash170106

    if (!bSave)
    {
        Init();
    }

    if (!db[_T("Mark_Pre_Proc_Mode")].Link(bSave, m_nMarkPreProcMode))
        m_nMarkPreProcMode = ENUM_PREPROC_NOTHING;
    if (!db[_T("UseAngleInspection")].Link(bSave, m_nSelectAngleReference))
        m_nSelectAngleReference = Mark_Angle_Ref_Teaching_Image;
    if (!db[_T("UseNormalSizeInspMode")].Link(bSave, m_bUseNormalSizeInspMode))
        m_bUseNormalSizeInspMode = FALSE;
    if (!db[_T("Ignore_ROI_Num")].Link(bSave, m_nIgnoreROINum))
        m_nIgnoreROINum = 0;
    if (!db[_T("Locator_Target_ROI")].Link(bSave, m_bLocatorTargetROI))
        m_bLocatorTargetROI = 0;
    if (!db[_T("Black_Image")].Link(bSave, m_bWhiteBackGround))
        m_bWhiteBackGround = FALSE;
    if (!db[_T("Mark_Direction")].Link(bSave, m_nMarkDirection))
        m_nMarkDirection = ENUM_MARKDIR_HOR;
    if (!db[_T("Loc_Search_Offset_X")].Link(bSave, m_nLocSearchOffsetX))
        m_nLocSearchOffsetX = 30;
    if (!db[_T("Loc_Search_Offset_Y")].Link(bSave, m_nLocSearchOffsetY))
        m_nLocSearchOffsetY = 30;
    if (!db[_T("Char_Search_Offset_X")].Link(bSave, m_nCharSearchOffsetX))
        m_nCharSearchOffsetX = 5;
    if (!db[_T("Char_Search_Offset_Y")].Link(bSave, m_nCharSearchOffsetY))
        m_nCharSearchOffsetY = 5;
    if (!db[_T("Image_Sample_Rate")].Link(bSave, m_nImageSampleRate))
        m_nImageSampleRate = 3;
    if (!db[_T("MarkAngle")].Link(bSave, m_nMarkAngle_deg))
        m_nMarkAngle_deg = 0;
    if (!db[_T("MarkInspctionMode")].Link(bSave, m_nMarkInspMode))
        m_nMarkInspMode = Mark_InspMode_Simple;
    if (!db[_T("Ignore Area Dilate Count")].Link(bSave, m_nIgnoreDilateCount))
        m_nIgnoreDilateCount = 0;
    if (!db[_T("Align Area Ignore")].Link(bSave, m_bAlignAreaIgnore))
        m_bAlignAreaIgnore = FALSE;

    // m_vecrtUserNumIgnore 저장
    if (!bSave)
    {
        if (!db[_T("Ignore_Num_Type_ROI_Num_Size")].Link(bSave, nSize))
            nSize = 0;
        m_vecrtUserNumIgnore.resize(nSize);
        m_vecptUserNumIgnoreRoiLtFromPackCenter.resize(nSize); //kircheis_AutoTeach
    }
    else
    {
        nSize = (long)(m_vecrtUserNumIgnore.size());
        if (!db[_T("Ignore_Num_Type_ROI_Num_Size")].Link(bSave, nSize))
            nSize = 0;
    }

    CString strTemp;

    nAutoTeachParamSize = (long)m_vecptUserNumIgnoreRoiLtFromPackCenter.size(); ////kircheis_Crash170106
    for (long i = 0; i < nSize; i++)
    {
        strTemp.Format(
            _T("Ignore_Num_Type_ROI_%d"), i); //strTemp.Format(_T("Ignore_ROI_%d"), i);//kircheis_AutoTeach.. 컥 버그다
        if (!db[strTemp].Link(bSave, m_vecrtUserNumIgnore[i]))
            m_vecrtUserNumIgnore[i] = Ipvm::Rect32s(0, 0, 10, 10);

        if (i < nAutoTeachParamSize) //kircheis_Crash170106
        {
            strTemp.Format(_T("Ignore_Num_Type_ROI_Offset_%d"), i); //kircheis_AutoTeach
            if (!db[strTemp].Link(bSave, m_vecptUserNumIgnoreRoiLtFromPackCenter[i]))
                m_vecptUserNumIgnoreRoiLtFromPackCenter[i] = Ipvm::Point32s2(0, 0);
        }
    }

    //20141004 현건
    m_vecbSelectIgnore.resize(70);

    if (!db[_T("Ignore Num Type Array")].Link(bSave, nSize))
        nSize = 0;

    for (long i = 0; i < 70; i++)
    {
        strTemp.Format(_T("Ignore_Num_Type_ROI_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecbSelectIgnore[i]))
            m_vecbSelectIgnore[i] = 0;
    }

    // m_vecrtUserIgnore 저장
    if (!bSave)
    {
        if (!db[_T("Ignore_ROI_Num_Size")].Link(bSave, nSize))
            nSize = 0;
        m_vecrtUserIgnore.resize(nSize);
        m_vecptUserIgnoreRoiLtFromPackCenter.resize(nSize); //kircheis_AutoTeach
    }
    else
    {
        nSize = (long)(m_vecrtUserIgnore.size());
        if (!db[_T("Ignore_ROI_Num_Size")].Link(bSave, nSize))
            nSize = 0;
    }

    nAutoTeachParamSize = (long)m_vecptUserIgnoreRoiLtFromPackCenter.size(); ////kircheis_Crash170106
    for (long i = 0; i < nSize; i++)
    {
        strTemp.Format(_T("Ignore_ROI_%d"), i);
        if (!db[strTemp].Link(bSave, m_vecrtUserIgnore[i]))
            m_vecrtUserIgnore[i] = Ipvm::Rect32s(0, 0, 10, 10);

        if (i < nAutoTeachParamSize) //kircheis_Crash170106
        {
            strTemp.Format(_T("Ignore_ROI_Offset_%d"), i); //kircheis_AutoTeach
            if (!db[strTemp].Link(bSave, m_vecptUserIgnoreRoiLtFromPackCenter[i]))
                m_vecptUserIgnoreRoiLtFromPackCenter[i] = Ipvm::Point32s2(0, 0);
        }
    }

    // m_vecrtMerge 저장
    if (!db[_T("Merge_ROI_Num")].Link(bSave, m_teach_merge_num))
        m_teach_merge_num = 0;

    if (!bSave)
    {
        m_teach_merge_infos.resize(max(0, m_teach_merge_num));
    }

    for (long i = 0; i < m_teach_merge_num; i++)
    {
        auto& info = m_teach_merge_infos[i];

        if (!db.GetSubDBFmt(_T("Merge_ROI_%d"), i).Link(bSave, info.m_roi))
            info.m_roi = Ipvm::Rect32s(0, 0, 10, 10);
        if (!db.GetSubDBFmt(_T("Merge_ROI_Offset_%d"), i).Link(bSave, info.m_roiLtFromPackCenter))
            info.m_roiLtFromPackCenter = Ipvm::Point32s2(0, 0);
    }

    auto& algoParaDB = db[_T("{F0EE8493-12D1-4E3A-8711-7ADA186F5AA5}")];

    if (!algoParaDB[_T("Blob_Size_Min")].Link(bSave, sMarkAlgoParam.nBlobSizeMin))
        sMarkAlgoParam.nBlobSizeMin = 30;
    if (!algoParaDB[_T("Blob_Size_Max")].Link(bSave, sMarkAlgoParam.nBlobSizeMax))
        sMarkAlgoParam.nBlobSizeMax = 50000;
    if (!algoParaDB[_T("Min_Mark_Contrast_For_Mark_Count")].Link(bSave, sMarkAlgoParam.nMinContrastForMarkCnt))
        sMarkAlgoParam.nMinContrastForMarkCnt = 30;
    if (!algoParaDB[_T("Mark_Merge_Gap")].Link(bSave, sMarkAlgoParam.fMarkMergeGap))
        sMarkAlgoParam.fMarkMergeGap = 0.f;
    if (!algoParaDB[_T("Use_All_Char_Reinsp")].Link(bSave, sMarkAlgoParam.bUseAllCharReinsp))
        sMarkAlgoParam.bUseAllCharReinsp = TRUE;
    if (!algoParaDB[_T("All_Char_Reinsp_Number")].Link(bSave, sMarkAlgoParam.nAllCharReinspNumber))
        sMarkAlgoParam.nAllCharReinspNumber = 3;
    if (!algoParaDB[_T("All_Char_Threshold_Deviation")].Link(bSave, sMarkAlgoParam.nAllCharThresholdDeviation))
        sMarkAlgoParam.nAllCharThresholdDeviation = 10;

    if (!algoParaDB[_T("Use_Each_Char_Reinsp")].Link(bSave, sMarkAlgoParam.bUseEachCharReinsp))
        sMarkAlgoParam.bUseEachCharReinsp = TRUE;
    if (!algoParaDB[_T("Each_Char_Reinsp_Number")].Link(bSave, sMarkAlgoParam.nEachCharReinspNumber))
        sMarkAlgoParam.nEachCharReinspNumber = 3;
    if (!algoParaDB[_T("Each_Char_Threshold_Deviation")].Link(bSave, sMarkAlgoParam.nEachCharThresholdDeviation))
        sMarkAlgoParam.nEachCharThresholdDeviation = 10;
    if (!algoParaDB[_T("Thinning_Intensity")].Link(bSave, sMarkAlgoParam.nThinningIntensity))
        sMarkAlgoParam.nThinningIntensity = 3;
    if (!algoParaDB[_T("BlobSize_Merge_Gap")].Link(bSave, sMarkAlgoParam.fBlobSizeMergeGap))
        sMarkAlgoParam.fBlobSizeMergeGap = 5.0f;
    if (!algoParaDB[_T("Teaching_Threshold_Operation_Mode")].Link(bSave, sMarkAlgoParam.nThresholdMode))
        sMarkAlgoParam.nThresholdMode = ENUM_THRESHOLD_AUTOFULL;
    if (!algoParaDB[_T("Mark_Roi_Margin")].Link(bSave, sMarkAlgoParam.nMarkRoiMargin))
        sMarkAlgoParam.nMarkRoiMargin = 3;

    m_ImageProcMangePara.LinkDataBase(bSave, db[_T("{74CD5D8E-004A-41A6-B43C-B27F006DC4BF}")]);

    //{{ //kircheis_for_ITEK
    if (!db[_T("Initial_Ignore_Box")].Link(bSave, m_bInitialIgnoreBox))
        m_bInitialIgnoreBox = FALSE;
    //}}

    //kk Mark Map Data Info Save
    if (!db[_T("{F717076B-8C14-459F-8B33-D2B24B58EEE0}")].Link(bSave, m_nROISettingMethod))
        m_nROISettingMethod = 0;
    m_sMarkMapInfo_um.LinkDataBase(bSave, db[_T("{28103DA7-6336-4931-BAD2-C3DCA8E20BD7}")]);

    return TRUE;
}

void VisionInspectionMarkPara::Init()
{
    m_teach_merge_num = 0;
    m_teach_merge_infos.clear();

    sMarkAlgoParam.Init();

    m_bAlignAreaIgnore = FALSE;
    m_nMarkInspMode = Mark_InspMode_Simple;
    m_nMarkPreProcMode = ENUM_PREPROC_NOTHING;
    m_nIgnoreROINum = 0;
    m_bLocatorTargetROI = FALSE;
    m_bWhiteBackGround = FALSE;
    m_vecrtUserIgnore.clear();

    m_nLocSearchOffsetX = 30;
    m_nLocSearchOffsetY = 30;
    m_nCharSearchOffsetX = 5;
    m_nCharSearchOffsetY = 5;
    m_nImageSampleRate = 3;
    m_nMarkAngle_deg = 0;
    m_nIgnoreDilateCount = 0;

    m_nMarkDirection = ENUM_MARKDIR_HOR;

    m_bUseNormalSizeInspMode = FALSE;
    m_nSelectAngleReference = Mark_Angle_Ref_Teaching_Image;

    m_vecrtUserIgnore.clear();
    m_vecrtUserSplite.clear();
    m_vecrtUserNumIgnore.clear();

    //{{ //kircheis_AutoTeach
    m_vecptUserNumIgnoreRoiLtFromPackCenter.clear();
    m_vecptUserIgnoreRoiLtFromPackCenter.clear();
    //}}

    //20141004 현건
    //m_vecbSelectIgnore.clear();
    if (m_vecbSelectIgnore.size() == 0)
    {
        m_vecbSelectIgnore.resize(MARK_MAX_IGNORE_AREA);
    }

    m_ImageProcMangePara.Init();

    m_bInitialIgnoreBox = FALSE; //kircheis_for_ITEK

    //kk
    m_sMarkMapInfo_um.Clear();
    m_nROISettingMethod = 0;
}

std::vector<CString> VisionInspectionMarkPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName, const long nMarkThreshold) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Module 특성 변수
    strCategory.Format(_T("Algorithm Parameters"));

    //Sub Dialog
    strGroup.Format(_T("Mark Algorithm Parameter"));
    strParaName.Format(_T("Mark Blob"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, _T("Min Blob Size (Px)"), sMarkAlgoParam.nBlobSizeMin));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, _T("Max Blob Size (Px)"), sMarkAlgoParam.nBlobSizeMax));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, _T("Min Mark Contrast for [Mark Count] (GV)"), sMarkAlgoParam.nMinContrastForMarkCnt));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, _T("Merge Gap (Px)"), sMarkAlgoParam.fMarkMergeGap));
    strParaName.Format(_T("Mark Blob Size"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, _T("Blob Thinning Intensity"), sMarkAlgoParam.nThinningIntensity));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, _T("Dist Blob to Mark"), sMarkAlgoParam.fBlobSizeMergeGap));
    strParaName.Format(_T("Under/Over ReInsp (All Characters)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, _T("Use All Charcters"), (bool)sMarkAlgoParam.bUseAllCharReinsp));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, _T("ReInsp Number"), sMarkAlgoParam.nAllCharReinspNumber));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, _T("Threshold Dev(GV)"), sMarkAlgoParam.nAllCharThresholdDeviation));
    strParaName.Format(_T("Under/Over ReInsp (Each Characters)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, _T("Use Each Charcters"), (bool)sMarkAlgoParam.bUseEachCharReinsp));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, _T("ReInsp Number"), sMarkAlgoParam.nEachCharReinspNumber));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, _T("Threshold Dev(GV)"), sMarkAlgoParam.nEachCharThresholdDeviation));
    strParaName.Format(_T("ETC"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, _T("Threshold Mode"), sMarkAlgoParam.nThresholdMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, _T("Mark Charcter Contrast ROI Margin"), sMarkAlgoParam.nMarkRoiMargin));

    //Image Combine
    std::vector<CString> vecstrImageCombineParameters(0);
    vecstrImageCombineParameters = m_ImageProcMangePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Main Parameters
    strGroup.Format(_T("Reference Modity Tool"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("ROI Setting Method"), _T(""), m_nROISettingMethod));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Char Merge Box Num"), _T(""), m_teach_merge_num));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Char Ignore Box Num"), _T(""), m_nIgnoreROINum));

    strGroup.Format(_T("Threshold & Modify Tool"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Align Area Ignore"), _T(""), (bool)m_bAlignAreaIgnore));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Ignore Area Dilate Count"), _T(""), m_nIgnoreDilateCount));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Threshold"), _T(""), nMarkThreshold));

    strGroup.Format(_T("Detail Setup"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Mark Angle"), _T(""), m_nMarkAngle_deg));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Mark Inspection Mode"), _T(""), m_nMarkInspMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Image Processing"), _T(""), m_nMarkPreProcMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Locator Target ROI"), _T(""), (bool)m_bLocatorTargetROI));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("White Back Ground"), _T(""), (bool)m_bWhiteBackGround));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Select Char Insp Mode Use Blob"), _T(""), (bool)m_bUseNormalSizeInspMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Select Angle Insp Mode"), _T(""), m_nSelectAngleReference));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Locator Search Offset X"), _T(""), m_nLocSearchOffsetX));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Locator Search Offset Y"), _T(""), m_nLocSearchOffsetY));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Char Search Offset X"), _T(""), m_nCharSearchOffsetX));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Char Search Offset Y"), _T(""), m_nCharSearchOffsetY));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Image Sampling"), _T(""), m_nImageSampleRate));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Mark Direction"), _T(""), m_nMarkDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Inital Ignore ROI Use"), _T(""), (bool)m_bInitialIgnoreBox));

    return vecstrAlgorithmParameters;
}

void VisionInspectionMarkPara::WriteMarkMapInfoToCSV(CString strFileFullPath)
{
    CStdioFile file;
    CFileException ex;

    if (file.Open(strFileFullPath, CFile::modeCreate | CFile::modeReadWrite, &ex))
    {
        CString Data(_T(""));
        CString tmpData;

        Data = _T("MarkID,Name,CenterPosX(mm),CenterPosY(mm),ROI Width(mm),ROI Height(mm),Angle(DEG),Type\n");

        if (m_sMarkMapInfo_um.sTeachROI_Info.nType == MARK_ROI_TEACH)
        {
            tmpData.Format(_T("%s,%s,%f,%f,%f,%f,%f,%s\n"), (LPCTSTR)m_sMarkMapInfo_um.sTeachROI_Info.strID,
                (LPCTSTR)m_sMarkMapInfo_um.sTeachROI_Info.strName, m_sMarkMapInfo_um.sTeachROI_Info.fOffsetX * .001f,
                m_sMarkMapInfo_um.sTeachROI_Info.fOffsetY * .001f, m_sMarkMapInfo_um.sTeachROI_Info.fWidth * .001f,
                m_sMarkMapInfo_um.sTeachROI_Info.fLength * .001f, m_sMarkMapInfo_um.sTeachROI_Info.fAngle,
                g_szMarkROITypeName[MARK_ROI_TEACH]);

            Data += tmpData;
        }

        for (auto& MarkRoiInfo : m_sMarkMapInfo_um.vecMergeROI_Info)
        {
            CString strType;

            if (MarkRoiInfo.nType == MARK_ROI_MERGE)
                strType = g_szMarkROITypeName[MARK_ROI_MERGE];
            else
                strType = _T("X");

            tmpData.Format(_T("%s,%s,%f,%f,%f,%f,%f,%s\n"), (LPCTSTR)MarkRoiInfo.strID, (LPCTSTR)MarkRoiInfo.strName,
                MarkRoiInfo.fOffsetX * .001f, MarkRoiInfo.fOffsetY * .001f, MarkRoiInfo.fWidth * .001f,
                MarkRoiInfo.fLength * .001f, MarkRoiInfo.fAngle, (LPCTSTR)strType);

            Data += tmpData;
        }

        for (auto& MarkRoiInfo : m_sMarkMapInfo_um.vecIgnoreROI_Info)
        {
            CString strType;

            if (MarkRoiInfo.nType == MARK_ROI_IGNORE)
                strType = g_szMarkROITypeName[MARK_ROI_IGNORE];
            else
                strType = _T("X");

            tmpData.Format(_T("%s,%s,%f,%f,%f,%f,%f,%s\n"), (LPCTSTR)MarkRoiInfo.strID, (LPCTSTR)MarkRoiInfo.strName,
                MarkRoiInfo.fOffsetX * .001f, MarkRoiInfo.fOffsetY * .001f, MarkRoiInfo.fWidth * .001f,
                MarkRoiInfo.fLength * .001f, MarkRoiInfo.fAngle, (LPCTSTR)strType);

            Data += tmpData;
        }

        file.WriteString(Data);
        file.Close();
    }
}

void sReadMarkMapInfo::InitData()
{
    vecTeachROI_Info.clear();
    vecIgnoreROI_Info.clear();
    vecMergeROI_Info.clear();
}

void sMarkROI_Info::Copy(sMarkROI_Info i_input)
{
    strID = i_input.strID;
    strName = i_input.strName;
    fOffsetX = i_input.fOffsetX;
    fOffsetY = i_input.fOffsetY;
    fWidth = i_input.fWidth;
    fLength = i_input.fLength;
    fAngle = i_input.fAngle;
    nType = i_input.nType;
}

void sMarkROI_Info::Clear()
{
    strID = "";
    strName = "";
    fOffsetX = 0;
    fOffsetY = 0;
    fWidth = 0;
    fLength = 0;
    fAngle = 0;
    nType = -1;
}

BOOL sMarkROI_Info::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{DE428D48-C2A7-4A5A-9ECF-85A8C0C63CFB}")].Link(bSave, strID))
        strID = _T("");
    if (!db[_T("{04A83216-FDAD-409C-8EE5-6B7C4A82F6F2}")].Link(bSave, strName))
        strName = _T("");
    if (!db[_T("{0E32DCA5-3259-401E-AAE7-5677722CA90E}")].Link(bSave, fOffsetX))
        fOffsetX = 0;
    if (!db[_T("{55F9F216-127E-4EA7-BE34-5E312E5E1C29}")].Link(bSave, fOffsetY))
        fOffsetY = 0;
    if (!db[_T("{0B20B549-ABA4-4293-AFAE-6993A690925B}")].Link(bSave, fWidth))
        fWidth = 0;
    if (!db[_T("{E59A8BAA-D549-4695-8C64-A89717BE4F4E}")].Link(bSave, fLength))
        fLength = 0;
    if (!db[_T("{93CF07D8-4892-48DE-A9A6-59A9FE975871}")].Link(bSave, fAngle))
        fAngle = 0;
    if (!db[_T("{00FA47FF-2E89-4E8D-A73C-BC65AB4390C6}")].Link(bSave, nType))
        nType = -1;

    return TRUE;
}

void sMarkMapInfo::Clear()
{
    sTeachROI_Info.Clear();
    vecIgnoreROI_Info.clear();
    vecMergeROI_Info.clear();
}

BOOL sMarkMapInfo::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    sTeachROI_Info.LinkDataBase(bSave, db[_T("{7BD4AC66-B061-4EB2-9C8E-E65737FBD787}")]);

    long nMergeROICount = 0;

    if (bSave)
        nMergeROICount = (long)vecMergeROI_Info.size();

    if (!db[_T("{F6B0B594-A91D-4828-9471-BAF41549D37A}")].Link(bSave, nMergeROICount))
        nMergeROICount = 0;

    if (!bSave)
        vecMergeROI_Info.resize(nMergeROICount);

    for (int i = 0; i < nMergeROICount; i++)
    {
        CString strTemp;

        strTemp.Format(_T("{3E70BBD6-FD1A-4E30-95BC-F7093F2658FC}_%d"), i);
        vecMergeROI_Info[i].LinkDataBase(bSave, db[strTemp]);
    }

    long nIgnoreROICount = 0;

    if (bSave)
        nIgnoreROICount = (long)vecIgnoreROI_Info.size();

    if (!db[_T("{B3B061EA-2370-4754-80D7-775FB45C91EE}")].Link(bSave, nIgnoreROICount))
        nIgnoreROICount = 0;

    if (!bSave)
        vecIgnoreROI_Info.resize(nIgnoreROICount);

    for (int i = 0; i < nIgnoreROICount; i++)
    {
        CString strTemp;

        strTemp.Format(_T("{CF2D685A-4612-4E15-A991-AFB364BE388E}_%d"), i);
        vecIgnoreROI_Info[i].LinkDataBase(bSave, db[strTemp]);
    }

    return TRUE;
}