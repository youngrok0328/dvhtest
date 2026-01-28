//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionLid3DPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <algorithm>
#include <fstream>
#include <map>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionLid3DPara::VisionInspectionLid3DPara(VisionProcessing& parent)
    : m_ImageProcManagePara(parent)
{
}

VisionInspectionLid3DPara::~VisionInspectionLid3DPara(void)
{
}

BOOL VisionInspectionLid3DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    m_ImageProcManagePara.LinkDataBase(bSave, db[_T("{1146FCA3-3ADE-4A50-9B8E-D5DA174B1533}")]);

    CString strTemp;

    if (!db[_T("m_nEdgeDirection")].Link(bSave, m_nEdgeDirection))
        m_nEdgeDirection = PI_ED_DIR_FALLING;
    if (!db[_T("m_nSearchDirection")].Link(bSave, m_nSearchDirection))
        m_nSearchDirection = PI_ED_DIR_OUTER;
    if (!db[_T("m_nEdgeDetectMode")].Link(bSave, m_nEdgeDetectMode))
        m_nEdgeDetectMode = PI_ED_MAX_EDGE;
    if (!db[_T("m_nFirstEdgeValue")].Link(bSave, m_nFirstEdgeValue))
        m_nFirstEdgeValue = 10;
    if (!db[_T("m_nEdgeSearchLength_um")].Link(bSave, m_nEdgeSearchLength_um))
        m_nEdgeSearchLength_um = 500;
    if (!db[_T("m_nEdgeSearchWidthRatio")].Link(bSave, m_nEdgeSearchWidthRatio))
        m_nEdgeSearchWidthRatio = 80;
    if (!db[_T("m_nEdgeDetectGap_um")].Link(bSave, m_nEdgeDetectGap_um))
        m_nEdgeDetectGap_um = 500;

    if (!db[_T("m_nSubstrateROI_size_um")].Link(bSave, m_nSubstrateROI_size_um))
        m_nSubstrateROI_size_um = 500;
    if (!db[_T("m_nSubstrateROI_OffsetX_um")].Link(bSave, m_nSubstrateROI_OffsetX_um))
        m_nSubstrateROI_OffsetX_um = 500;
    if (!db[_T("m_nSubstrateROI_OffsetY_um")].Link(bSave, m_nSubstrateROI_OffsetY_um))
        m_nSubstrateROI_OffsetY_um = 500;

    if (!db[_T("m_nLidTiltROI_size_um")].Link(bSave, m_nLidTiltROI_size_um))
        m_nLidTiltROI_size_um = 500;
    if (!db[_T("m_nLidTiltROI_OffsetX_um")].Link(bSave, m_nLidTiltROI_OffsetX_um))
        m_nLidTiltROI_OffsetX_um = 500;
    if (!db[_T("m_nLidTiltROI_OffsetY_um")].Link(bSave, m_nLidTiltROI_OffsetY_um))
        m_nLidTiltROI_OffsetY_um = 500;

    //{{Lid3DGroupInfo
    long nGroupNum = (long)m_vecsLid3DGroupInfo_um.size();
    long nROINum = 0;
    std::vector<long> vecnROINum(0);
    if (bSave)
    {
        db[_T("Lid3DGroupInfoNum")].Link(bSave, nGroupNum);
        long nGroup = 0;
        for (auto lidGroupInfo : m_vecsLid3DGroupInfo_um)
        {
            nROINum = (long)lidGroupInfo.vecLidROI_Info.size();
            vecnROINum.push_back(nROINum);
            strTemp.Format(_T("Lid3DGroupInfo[%d]_ROINum"), nGroup);
            db[strTemp].Link(bSave, nROINum);
            nGroup++;
        }
    }
    else
    {
        if (!db[_T("Lid3DGroupInfoNum")].Link(bSave, nGroupNum))
            nGroupNum = 0;
        SetLid3DGroupInfo(nGroupNum, m_vecsLid3DGroupInfo_um);
        if (nGroupNum > 0)
        {
            for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
            {
                strTemp.Format(_T("Lid3DGroupInfo[%d]_ROINum"), nGroup);
                if (!db[strTemp].Link(bSave, nROINum))
                    nROINum = 0;
                if (nROINum > 0)
                    SetLid3DROI_Info(nGroup, nROINum, m_vecsLid3DGroupInfo_um);
            }
        }
    }

    for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
    {
        nROINum = (long)m_vecsLid3DGroupInfo_um[nGroup].vecLidROI_Info.size();
        strTemp.Format(_T("Lid3DGroupInfo[%d]_ID"), nGroup);
        if (!db[strTemp].Link(bSave, m_vecsLid3DGroupInfo_um[nGroup].strID))
            m_vecsLid3DGroupInfo_um[nGroup].strID.Format(_T("G%d"), nGroup);

        for (long nROI = 0; nROI < nROINum; nROI++)
        {
            auto& sLid3DROIInfo = m_vecsLid3DGroupInfo_um[nGroup].vecLidROI_Info[nROI];

            strTemp.Format(_T("Lid3DGroupInfo[%d]ROI[%d]_ID"), nGroup, nROI);
            if (!db[strTemp].Link(bSave, sLid3DROIInfo.strID))
                sLid3DROIInfo.strID.Format(_T("L%d"), nROI);

            strTemp.Format(_T("Lid3DGroupInfo[%d]ROI[%d]_OffsetX"), nGroup, nROI);
            if (!db[strTemp].Link(bSave, sLid3DROIInfo.fOffsetX))
                sLid3DROIInfo.fOffsetX = 0.f;

            strTemp.Format(_T("Lid3DGroupInfo[%d]ROI[%d]_OffsetY"), nGroup, nROI);
            if (!db[strTemp].Link(bSave, sLid3DROIInfo.fOffsetY))
                sLid3DROIInfo.fOffsetY = 0.f;

            strTemp.Format(_T("Lid3DGroupInfo[%d]ROI[%d]_Width"), nGroup, nROI);
            if (!db[strTemp].Link(bSave, sLid3DROIInfo.fWidth))
                sLid3DROIInfo.fWidth = 200.f;

            strTemp.Format(_T("Lid3DGroupInfo[%d]ROI[%d]_Length"), nGroup, nROI);
            if (!db[strTemp].Link(bSave, sLid3DROIInfo.fLength))
                sLid3DROIInfo.fLength = 200.f;
        }
    }

    //}}

    return TRUE;
}

void VisionInspectionLid3DPara::ClearLid3DGroupInfo(std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo)
{
    for (auto& sLidGroupInfo : vecsLid3DGroupInfo)
    {
        sLidGroupInfo.vecLidROI_Info.clear();
    }
}

void VisionInspectionLid3DPara::SetLid3DGroupInfo(long nGroupNum, std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo)
{
    ClearLid3DGroupInfo(vecsLid3DGroupInfo);
    if (nGroupNum > 0)
        vecsLid3DGroupInfo.resize(nGroupNum);
}

void VisionInspectionLid3DPara::SetLid3DROI_Info(
    long nGroupID, long nROI_Num, std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo)
{
    long nGroupNum = (long)vecsLid3DGroupInfo.size();
    if (nGroupNum <= nGroupID)
        return;

    vecsLid3DGroupInfo[nGroupID].vecLidROI_Info.clear();
    vecsLid3DGroupInfo[nGroupID].vecLidROI_Info.resize(nROI_Num);
}

void VisionInspectionLid3DPara::ReadLid3DGroupInfoFromCSV(CString strFileFullPath)
{
    std::vector<sLid3DReadDataInfo> vecsLid3DReadDataInfo;

    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(strFileFullPath);
    bool headerRead = false;

    char buffer[1000];

    enum fixColumn
    {
        FX_NAME,
        FX_X,
        FX_Y,
        FX_WIDTH,
        FX_LENGTH,
        FX_GROUPID,
        FX_END,
    };

    long columnLink[FX_END];
    long headerColumnCount(0);
    long rowIndex(0);

    CString strErrorMessage;
    while (infile.getline(buffer, 1000))
    {
        rowIndex++;

        std::basic_string<char> temp(buffer);

        temp_string_size_type findStart = 0;
        temp_string_size_type findEnd = 0;

        if (!headerRead)
        {
            //---------------------------------------------------------------------------------------
            // Read Header
            //---------------------------------------------------------------------------------------

            std::map<CStringA, long> headerIndexMap;
            long index = 0;
            while (1)
            {
                findEnd = temp.find_first_of(',', findStart);
                CStringA text;
                if (findEnd == std::basic_string<char>::npos)
                {
                    text.SetString(buffer + findStart, CAST_INT(temp.length() - findStart));
                    headerIndexMap[text] = index;
                    break;
                }
                else
                {
                    text.SetString(buffer + findStart, CAST_INT(findEnd - findStart));
                    headerIndexMap[text] = index;
                    findStart = findEnd + 1;
                    index++;
                }
                text.Empty();
            }

            // 헤더 정보중 필수 항목들이 모두 들어왔는지 확인한다

            for (long item = 0; item < FX_END; item++)
            {
                CStringA columnName;
                switch (item)
                {
                    case FX_NAME:
                        columnName = _T("LID_SEQ");
                        break;
                    case FX_X:
                        columnName = _T("LID_X (mm)");
                        break;
                    case FX_Y:
                        columnName = _T("LID_Y (mm)");
                        break;
                    case FX_WIDTH:
                        columnName = _T("Width (mm)");
                        break;
                    case FX_LENGTH:
                        columnName = _T("Length (mm)");
                        break;
                    case FX_GROUPID:
                        columnName = _T("Group");
                        break;
                }

                if (columnName.IsEmpty())
                {
                    strErrorMessage.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                    ::AfxMessageBox(strErrorMessage);
                    return;
                }

                columnLink[item] = headerIndexMap[columnName];

                columnName.Empty();
            }

            headerColumnCount = long(headerIndexMap.size());
            headerRead = true;
            continue;
        }

        std::vector<CStringA> dataList;
        while (1)
        {
            findEnd = temp.find_first_of(',', findStart);
            CStringA text;
            if (findEnd == std::basic_string<char>::npos)
            {
                text.SetString(buffer + findStart, CAST_INT(temp.length() - findStart));
                dataList.push_back(text);
                break;
            }
            else
            {
                text.SetString(buffer + findStart, CAST_INT(findEnd - findStart));
                dataList.push_back(text);
                findStart = findEnd + 1;
            }

            text.Empty();
        }

        if (dataList.size())
        {
            if (dataList.size() != headerColumnCount)
            {
                // 헤더 컬럼과 데이터 컴럼의 수가 다르다
                strErrorMessage.Format(_T("The number of columns is different. (%d Line)"), rowIndex);
                return;
            }

            sLid3DReadDataInfo sLid3DReadDataInfo;
            sLid3DReadDataInfo.strID = dataList[columnLink[FX_NAME]];
            sLid3DReadDataInfo.fOffsetX = CAST_FLOAT(::atof(dataList[columnLink[FX_X]]));
            sLid3DReadDataInfo.fOffsetY = CAST_FLOAT(::atof(dataList[columnLink[FX_Y]]));
            sLid3DReadDataInfo.fWidth = CAST_FLOAT(::atof(dataList[columnLink[FX_WIDTH]]));
            sLid3DReadDataInfo.fLength = CAST_FLOAT(::atof(dataList[columnLink[FX_LENGTH]]));
            sLid3DReadDataInfo.nGroupID = CAST_LONG(::atof(dataList[columnLink[FX_GROUPID]]));

            vecsLid3DReadDataInfo.push_back(sLid3DReadDataInfo);
        }

        for (int nIdx = 0; nIdx < dataList.size(); nIdx++)
        {
            dataList[nIdx].Empty();
        }
    }

    std::vector<long> vecnGroupID;
    GetGroupIDNum(vecsLid3DReadDataInfo, vecnGroupID);
    SetLid3DGroupInfo(CAST_LONG(vecnGroupID.size()), m_vecsLid3DGroupInfo_um);
    SetGroupData_um(vecsLid3DReadDataInfo, vecnGroupID, m_vecsLid3DGroupInfo_um);
    strErrorMessage.Empty();
}

void VisionInspectionLid3DPara::WriteLid3DGroupInfoToCSV(CString strFileFullPath)
{
    CFile file;
    if (file.Open(strFileFullPath, CFile::modeCreate | CFile::modeWrite))
    {
        CArchive ar(&file, CArchive::store);

        CString str;

        str.Format(_T("LID_SEQ,LID_X (mm),LID_Y (mm),Width (mm),Length (mm),Group\r\n"));
        ar.WriteString(str);
        long nGroupID = 0;
        for (auto& lidRoiGroupInfo : m_vecsLid3DGroupInfo_um)
        {
            for (auto& lidRoiInfo : lidRoiGroupInfo.vecLidROI_Info)
            {
                str.Format(_T("%s,%f,%f,%f,%f,%d\r\n"), (LPCTSTR)lidRoiInfo.strID, lidRoiInfo.fOffsetX * .001f,
                    lidRoiInfo.fOffsetY * .001f, lidRoiInfo.fWidth * .001f, lidRoiInfo.fLength * .001f, nGroupID);

                ar.WriteString(str);
            }
            nGroupID++;
        }

        str.Empty();
        ar.Close();
        file.Close();
    }
}

void VisionInspectionLid3DPara::SetGroupData_um(std::vector<sLid3DReadDataInfo> i_vecsLid3DReadDataInfo,
    std::vector<long> i_vecnGroupID, std::vector<SLid3DGroupInfo>& o_vecsLid3DGroupInfo_um)
{
    UNREFERENCED_PARAMETER(o_vecsLid3DGroupInfo_um);

    if (i_vecsLid3DReadDataInfo.size() < 0)
        return;

    long ROIDataNum(0);
    std::vector<long> vecVaildindex;
    for (long nidx = 0; nidx < i_vecnGroupID.size(); nidx++)
    {
        GetROIDataNumAndValidIndex(i_vecsLid3DReadDataInfo, i_vecnGroupID[nidx], ROIDataNum, vecVaildindex);
        SetLid3DROI_Info(nidx, ROIDataNum, m_vecsLid3DGroupInfo_um); //Resize

        CString strGroupID;
        strGroupID.Format(_T("%d"), i_vecnGroupID[nidx]);
        m_vecsLid3DGroupInfo_um[nidx].strID = strGroupID;

        for (long nRepeatidx = 0; nRepeatidx < vecVaildindex.size(); nRepeatidx++)
        {
            m_vecsLid3DGroupInfo_um[nidx].vecLidROI_Info[nRepeatidx].strID
                = i_vecsLid3DReadDataInfo[vecVaildindex[nRepeatidx]].strID;
            m_vecsLid3DGroupInfo_um[nidx].vecLidROI_Info[nRepeatidx].fOffsetX
                = i_vecsLid3DReadDataInfo[vecVaildindex[nRepeatidx]].fOffsetX * 1000.f;
            m_vecsLid3DGroupInfo_um[nidx].vecLidROI_Info[nRepeatidx].fOffsetY
                = i_vecsLid3DReadDataInfo[vecVaildindex[nRepeatidx]].fOffsetY * 1000.f;
            m_vecsLid3DGroupInfo_um[nidx].vecLidROI_Info[nRepeatidx].fWidth
                = i_vecsLid3DReadDataInfo[vecVaildindex[nRepeatidx]].fWidth * 1000.f;
            m_vecsLid3DGroupInfo_um[nidx].vecLidROI_Info[nRepeatidx].fLength
                = i_vecsLid3DReadDataInfo[vecVaildindex[nRepeatidx]].fLength * 1000.f;
        }

        strGroupID.Empty();
    }
}

void VisionInspectionLid3DPara::GetGroupIDNum(
    std::vector<sLid3DReadDataInfo> i_vecsLid3DReadDataInfo, std::vector<long>& o_vecnGroupID)
{
    if (i_vecsLid3DReadDataInfo.size() < 0)
        return;

    for (auto Lid3DReadDataInfo : i_vecsLid3DReadDataInfo)
        o_vecnGroupID.push_back(Lid3DReadDataInfo.nGroupID);

    std::sort(o_vecnGroupID.begin(), o_vecnGroupID.end()); //내림차순으로 Sorting 해주고

    o_vecnGroupID.erase(std::unique(o_vecnGroupID.begin(), o_vecnGroupID.end()), o_vecnGroupID.end()); //중복원소들 제거
}

void VisionInspectionLid3DPara::GetROIDataNumAndValidIndex(std::vector<sLid3DReadDataInfo> i_vecsLid3DReadDataInfo,
    long i_GroupID, long& o_nROIDataNum, std::vector<long>& o_vecVaildindex)
{
    o_nROIDataNum = 0;
    o_vecVaildindex.clear();

    for (long nidx = 0; nidx < i_vecsLid3DReadDataInfo.size(); nidx++)
    {
        if (i_vecsLid3DReadDataInfo[nidx].nGroupID == i_GroupID)
        {
            o_nROIDataNum++;
            o_vecVaildindex.push_back(nidx);
        }
    }
}

std::vector<CString> VisionInspectionLid3DPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
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

    //Image Combine
    std::vector<CString> vecstrImageCombineParameters(0);
    vecstrImageCombineParameters = m_ImageProcManagePara.ExportImageCombineParaToText(
        strVisionName, strInspectionModuleName, _T("Lid 3D Align-Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수
    strCategory.Format(_T("Algorithm Parameter"));

    strGroup.Format(_T("Edge detect parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Search direction"), _T(""), m_nSearchDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge direction"), _T(""), m_nEdgeDirection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge detect mode"), _T(""), m_nEdgeDetectMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("First edge value"), _T(""), m_nFirstEdgeValue));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge search length (um)"), _T(""), m_nEdgeSearchLength_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Edge search width ratio (%)"), _T(""), m_nEdgeSearchWidthRatio));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Detected edge gap (um)"), _T(""), m_nEdgeDetectGap_um));

    strGroup.Format(_T("Substrate ROI parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Substrate ROI size (um)"), _T(""), m_nSubstrateROI_size_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Substrate ROI offset X (um)"), _T(""), m_nSubstrateROI_OffsetX_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Substrate ROI offset Y (um)"), _T(""), m_nSubstrateROI_OffsetY_um));

    strGroup.Format(_T("Lid tilt & height ROI parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Lid tilt & height ROI size (um)"), _T(""), m_nLidTiltROI_size_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Lid tilt & height ROI offset X (um)"), _T(""), m_nLidTiltROI_OffsetX_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Lid tilt & height ROI offset Y (um)"), _T(""), m_nLidTiltROI_OffsetY_um));

    return vecstrAlgorithmParameters;
}