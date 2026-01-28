//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingFiducialAlignPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionProcessingFiducialAlignPara::VisionProcessingFiducialAlignPara(VisionProcessing& parent)
    : m_imageProc(parent)
{
    Init();
}

VisionProcessingFiducialAlignPara::~VisionProcessingFiducialAlignPara(void)
{
}

BOOL VisionProcessingFiducialAlignPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        Init();
    }

    m_imageProc.LinkDataBase(bSave, db[_T("{22DFC72F-76B9-4EF9-81BE-1FC7A7FC84E0}")]);

    if (!db[_T("{C5279ABE-B05A-468F-A804-9E1DA4F722C4}")].Link(bSave, m_select_use_image))
        m_select_use_image = use_zmap; // Search 범위
    if (!db[_T("{E8A878A8-AB88-4322-A5AD-2A570F56BE43}")].Link(bSave, m_SearchOffset_um))
        m_SearchOffset_um = 100; // Search 범위

    long nSize = 0;

    if (!bSave)
    {
        if (!db[_T("{231C1FF4-9C32-4E7C-B58F-479A95C821E2}")].Link(bSave, nSize))
            nSize = 0;
        m_use_pad_names.resize(nSize);
    }
    else
    {
        nSize = (long)(m_use_pad_names.size());
        if (!db[_T("{231C1FF4-9C32-4E7C-B58F-479A95C821E2}")].Link(bSave, nSize))
            nSize = 0;
    }

    CString strTemp = _T("");
    for (long nidx = 0; nidx < nSize; nidx++)
    {
        strTemp.Format(_T("{060ED8C3-0F84-44DA-81BE-763CAE070DE6}_%d"), nidx);
        if (!db[strTemp].Link(bSave, m_use_pad_names[nidx]))
            m_use_pad_names[nidx] = _T("");
    }

    strTemp.Empty();

    return TRUE;
}

void VisionProcessingFiducialAlignPara::Init()
{
    m_imageProc.Init();
    m_select_use_image = use_zmap;
    m_SearchOffset_um = 100;

    m_use_pad_names.clear();
    m_align_pad_names.clear();
    m_pad_name_to_index.clear();
}

long VisionProcessingFiducialAlignPara::FindAlignPAD_Index(LPCTSTR pad_name) const
{
    auto itIndex = m_pad_name_to_index.find(pad_name);
    if (itIndex == m_pad_name_to_index.end())
        return -1;

    return itIndex->second;
}

void VisionProcessingFiducialAlignPara::SetAlignPAD(LPCTSTR pad_name)
{
    if (FindAlignPAD_Index(pad_name) != -1)
        return;

    m_align_pad_names.push_back(pad_name);

    Genereat_index_map();
}

void VisionProcessingFiducialAlignPara::DelAlignPAD(LPCTSTR pad_name)
{
    for (long n = 0; n < long(m_align_pad_names.size()); n++)
    {
        if (m_align_pad_names[n].Compare(pad_name) == 0)
        {
            m_align_pad_names.erase(m_align_pad_names.begin() + n);
            break;
        }
    }

    Genereat_index_map();
}

void VisionProcessingFiducialAlignPara::Genereat_index_map()
{
    m_pad_name_to_index.clear();

    for (long fiducial = 0; fiducial < long(m_align_pad_names.size()); fiducial++)
    {
        m_pad_name_to_index[m_align_pad_names[fiducial]] = fiducial;
    }
}

LPCTSTR VisionProcessingFiducialAlignPara::GetAlignPAD_Name(long index) const
{
    return m_align_pad_names[index];
}

long VisionProcessingFiducialAlignPara::GetAlignPAD_Count() const
{
    return long(m_align_pad_names.size());
}

std::vector<CString> VisionProcessingFiducialAlignPara::ExportAlgoParaToText(
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

    //Module 특성 변수 //내용 채워야 함
    strCategory.Format(_T("Algorithm"));

    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, _T("Select PAD(1)"), _T(""), _T(""), m_use_pad_names[0]));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, _T("Select PAD(2)"), _T(""), _T(""), m_use_pad_names[1]));

    strCategory.Format(_T("Center Detection"));

    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, _T("Search Offset(um)"), _T(""), _T(""), m_SearchOffset_um));

    //Image Combine - Fiducial Align은 해당 항목이 제일 마지막에 있음
    std::vector<CString> vecstrImageCombineParameters(0);
    vecstrImageCombineParameters
        = m_imageProc.ExportImageCombineParaToText(strVisionName, strInspectionModuleName, _T("Image Combine"));

    vecstrAlgorithmParameters.insert(
        vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());

    return vecstrAlgorithmParameters;
}