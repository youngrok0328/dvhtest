//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionPatch3DPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
#include <fstream>
#include <map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionPatch3DPara::VisionInspectionPatch3DPara(VisionProcessing& parent)
    : m_ImageProcManagePara(parent)
{
}

VisionInspectionPatch3DPara::~VisionInspectionPatch3DPara(void)
{
}

BOOL VisionInspectionPatch3DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    m_ImageProcManagePara.LinkDataBase(bSave, db[_T("{DDD51AD3-4F83-48B9-9AE9-0070F43A77AD}")]);

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

    if (!db[_T("m_nPatchHeightROI_size_um")].Link(bSave, m_nPatchHeightROI_size_um))
        m_nPatchHeightROI_size_um = 500;
    if (!db[_T("m_nPatchHeightROI_OffsetX_um")].Link(bSave, m_nPatchHeightROI_OffsetX_um))
        m_nPatchHeightROI_OffsetX_um = 500;
    if (!db[_T("m_nPatchHeightROI_OffsetY_um")].Link(bSave, m_nPatchHeightROI_OffsetY_um))
        m_nPatchHeightROI_OffsetY_um = 500;

    //{{InterposerRefROIInfo
    long nRefROI_Num = (long)m_vecsInterposerRefROI_Info_um.size();
    std::vector<long> vecnROINum(0);
    if (!db[_T("InterposerRefROI_InfoNum")].Link(bSave, nRefROI_Num))
        nRefROI_Num = 0;
    if (!bSave)
    {
        m_vecsInterposerRefROI_Info_um.clear();
        m_vecsInterposerRefROI_Info_um.resize(nRefROI_Num);
    }

    for (long nROI = 0; nROI < nRefROI_Num; nROI++)
    {
        auto& sInterposer3DROI = m_vecsInterposerRefROI_Info_um[nROI];

        strTemp.Format(_T("Interposer3DROI[%d]_OffsetX"), nROI);
        if (!db[strTemp].Link(bSave, sInterposer3DROI.fOffsetX))
            sInterposer3DROI.fOffsetX = 0.f;

        strTemp.Format(_T("Patch3DGroupROI[%d]_OffsetY"), nROI);
        if (!db[strTemp].Link(bSave, sInterposer3DROI.fOffsetY))
            sInterposer3DROI.fOffsetY = 0.f;

        strTemp.Format(_T("Patch3DGroupROI[%d]_Width"), nROI);
        if (!db[strTemp].Link(bSave, sInterposer3DROI.fWidth))
            sInterposer3DROI.fWidth = 200.f;

        strTemp.Format(_T("Patch3DGroupROI[%d]_Length"), nROI);
        if (!db[strTemp].Link(bSave, sInterposer3DROI.fLength))
            sInterposer3DROI.fLength = 200.f;
    }

    //}}

    strTemp.Empty();

    return TRUE;
}

void VisionInspectionPatch3DPara::ReadInterposerRefInfoFromCSV(CString strFileFullPath)
{
    m_vecsInterposerRefROI_Info_um.clear();

    typedef std::basic_string<char>::size_type temp_string_size_type;

    std::ifstream infile(strFileFullPath);
    bool headerRead = false;

    char buffer[1000];

    enum fixColumn
    {
        FX_X,
        FX_Y,
        FX_WIDTH,
        FX_LENGTH,
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
                    case FX_X:
                        columnName = _T("InterposerOffset_X (mm)");
                        break;
                    case FX_Y:
                        columnName = _T("InterposerOffset_Y (mm)");
                        break;
                    case FX_WIDTH:
                        columnName = _T("Width (mm)");
                        break;
                    case FX_LENGTH:
                        columnName = _T("Length (mm)");
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

            SInterposerRefROI_Info sInterposerRefROI_Info;
            sInterposerRefROI_Info.fOffsetX = CAST_FLOAT(::atof(dataList[columnLink[FX_X]]) * 1000.);
            sInterposerRefROI_Info.fOffsetY = CAST_FLOAT(::atof(dataList[columnLink[FX_Y]]) * 1000.);
            sInterposerRefROI_Info.fWidth = CAST_FLOAT(::atof(dataList[columnLink[FX_WIDTH]]) * 1000.);
            sInterposerRefROI_Info.fLength = CAST_FLOAT(::atof(dataList[columnLink[FX_LENGTH]]) * 1000.);

            m_vecsInterposerRefROI_Info_um.push_back(sInterposerRefROI_Info);
        }

        for (int nIdx = 0; nIdx < dataList.size(); nIdx++)
        {
            dataList[nIdx].Empty();
        }
    }

    strErrorMessage.Empty();
}

void VisionInspectionPatch3DPara::WriteInterposerRefInfoToCSV(CString strFileFullPath)
{
    CFile file;
    if (file.Open(strFileFullPath, CFile::modeCreate | CFile::modeWrite))
    {
        CArchive ar(&file, CArchive::store);

        CString str;

        str.Format(_T("InterposerOffset_X (mm),InterposerOffset_Y (mm),Width (mm),Length (mm)\r\n"));
        ar.WriteString(str);
        for (auto& sInterposerRoiInfo : m_vecsInterposerRefROI_Info_um)
        {
            str.Format(_T("%f,%f,%f,%f\r\n"), sInterposerRoiInfo.fOffsetX * .001f, sInterposerRoiInfo.fOffsetY * .001f,
                sInterposerRoiInfo.fWidth * .001f, sInterposerRoiInfo.fLength * .001f);
            ar.WriteString(str);
        }
        ar.Close();
        file.Close();

        str.Empty();
    }
}

std::vector<CString> VisionInspectionPatch3DPara::ExportAlgoParaToText(
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
        strVisionName, strInspectionModuleName, _T("Patch 3D Align-Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    //Module 특성 변수 //채워야 함
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

    strGroup.Format(_T("Patch ROI parameters"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Patch ROI size (um)"), _T(""), m_nPatchHeightROI_size_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Patch ROI offset X (um)"), _T(""), m_nPatchHeightROI_OffsetX_um));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Patch ROI offset Y (um)"), _T(""), m_nPatchHeightROI_OffsetY_um));

    return vecstrAlgorithmParameters;
}