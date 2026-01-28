//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingPadAlign2DPara.h"

//CPP_2_________________________________ This project's headers
#include "ParaDB.h"

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
LPCTSTR GetPAD_CategoryName(enumPAD_Category category)
{
    switch (category)
    {
        case enumPAD_Category::Normal:
            return _T("Normal");
            break;
        case enumPAD_Category::Fiducial:
            return _T("Fiducial");
            break;
        case enumPAD_Category::PinIndex:
            return _T("PinIndex");
            break;
        case enumPAD_Category::Blank:
            return _T("");
            break;
    }

    return _T("");
}

VisionProcessingPadAlign2DPara::AlgoParameter::AlgoParameter(VisionProcessing& parent)
    : m_para(parent)
{
}

VisionProcessingPadAlign2DPara::VisionProcessingPadAlign2DPara(VisionProcessing& parent, CPackageSpec& packageSpec)
    : m_parent(parent)
    , m_packageSpec(packageSpec)
{
    Init();
}

VisionProcessingPadAlign2DPara::~VisionProcessingPadAlign2DPara(void)
{
    ResetAlgoParameter();
}

BOOL VisionProcessingPadAlign2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        Init();
    }

    long alignPADCount = long(m_align_pad_names.size());
    long algoCount = long(m_algo_ptr_list.size());

    if (!db[_T("AlignPAD Count")].Link(bSave, alignPADCount))
        alignPADCount = 0;
    if (!db[_T("Algorithm Count")].Link(bSave, algoCount))
        algoCount = 0;

    if (!bSave)
    {
        m_align_pad_names.resize(alignPADCount);
        m_align_pad_category.resize(alignPADCount);
        m_align_pad_algo_names.resize(alignPADCount);

        ResetAlgoParameter();
        for (long index = 0; index < algoCount; index++)
        {
            AddAlgoParameter();
        }
    }

    for (long index = 0; index < alignPADCount; index++)
    {
        auto& sub_db = db.GetSubDBFmt(_T("AlignPAD%d"), index + 1);

        auto pad_category = long(m_align_pad_category[index]);

        sub_db[_T("name"		)].Link(bSave, m_align_pad_names[index]);
        sub_db[_T("Algo name"	)].Link(bSave, m_align_pad_algo_names[index]);

        if (!sub_db[_T("category")].Link(bSave, pad_category))
            pad_category = long(enumPAD_Category::Normal);

        if (!bSave)
            m_align_pad_category[index] = enumPAD_Category(pad_category);
    }

    for (long index = 0; index < algoCount; index++)
    {
        auto& sub_db = db.GetSubDBFmt(_T("Algorithm%d"), index + 1);
        sub_db[_T("Algo name")].Link(bSave, m_algo_ptr_list[index]->m_name);
        m_algo_ptr_list[index]->m_para.LinkDataBase(bSave, sub_db[_T("Algo Para")]);
    }

    if (!bSave)
    {
        Genereat_index_map();
    }

    return TRUE;
}

void VisionProcessingPadAlign2DPara::Init()
{
    m_align_pad_names.clear();
    m_align_pad_category.clear();
    m_align_pad_algo_names.clear();
    ResetAlgoParameter();

    m_pad_name_to_index.clear();
    m_algo_name_to_index.clear();
}

long VisionProcessingPadAlign2DPara::GetAlignPAD_Count() const
{
    return long(m_align_pad_names.size());
}

long VisionProcessingPadAlign2DPara::FindAlignPAD_Index(LPCTSTR pad_name) const
{
    auto itIndex = m_pad_name_to_index.find(pad_name);
    if (itIndex == m_pad_name_to_index.end())
        return -1;

    return itIndex->second;
}

LPCTSTR VisionProcessingPadAlign2DPara::GetAlignPAD_Name(long index) const
{
    return m_align_pad_names[index];
}

const AlignPara* VisionProcessingPadAlign2DPara::GetAlignPAD_Para(LPCTSTR pad_name) const
{
    auto itIndex = m_pad_name_to_index.find(pad_name);
    if (itIndex == m_pad_name_to_index.end())
        return nullptr;

    CString algoName = m_align_pad_algo_names[itIndex->second];

    auto itAlgoIndex = m_algo_name_to_index.find(algoName);
    if (itAlgoIndex == m_algo_name_to_index.end())
        return nullptr;

    algoName.Empty();

    return &m_algo_ptr_list[itAlgoIndex->second]->m_para;
}

AlignPara* VisionProcessingPadAlign2DPara::GetAlignPAD_Para(LPCTSTR pad_name)
{
    auto itIndex = m_pad_name_to_index.find(pad_name);
    if (itIndex == m_pad_name_to_index.end())
        return nullptr;

    CString algoName = m_align_pad_algo_names[itIndex->second];

    auto itAlgoIndex = m_algo_name_to_index.find(algoName);
    if (itAlgoIndex == m_algo_name_to_index.end())
        return nullptr;

    algoName.Empty();

    return &m_algo_ptr_list[itAlgoIndex->second]->m_para;
}

LPCTSTR VisionProcessingPadAlign2DPara::GetAlignPAD_ParaName(LPCTSTR pad_name) const
{
    auto itIndex = m_pad_name_to_index.find(pad_name);
    if (itIndex == m_pad_name_to_index.end())
        return _T("");

    CString algoName = m_align_pad_algo_names[itIndex->second];

    auto itAlgoIndex = m_algo_name_to_index.find(algoName);
    if (itAlgoIndex == m_algo_name_to_index.end())
        return _T("");

    algoName.Empty();

    return m_algo_ptr_list[itAlgoIndex->second]->m_name;
}

LPCTSTR VisionProcessingPadAlign2DPara::GetAlignPAD_ParaName(long index) const
{
    CString algoName = m_align_pad_algo_names[index];

    auto itAlgoIndex = m_algo_name_to_index.find(algoName);
    if (itAlgoIndex == m_algo_name_to_index.end())
        return _T("");

    algoName.Empty();

    return m_algo_ptr_list[itAlgoIndex->second]->m_name;
}

enumPAD_Category VisionProcessingPadAlign2DPara::GetAlignPAD_Category(LPCTSTR pad_name) const
{
    auto itIndex = m_pad_name_to_index.find(pad_name);
    if (itIndex == m_pad_name_to_index.end())
        return enumPAD_Category::Blank;

    return m_align_pad_category[itIndex->second];
}

enumPAD_Category VisionProcessingPadAlign2DPara::GetAlignPAD_Category(long index) const
{
    return m_align_pad_category[index];
}

long VisionProcessingPadAlign2DPara::GetParaCount() const
{
    return long(m_algo_ptr_list.size());
}

LPCTSTR VisionProcessingPadAlign2DPara::GetParaName(long index) const
{
    return m_algo_ptr_list[index]->m_name;
}

const AlignPara* VisionProcessingPadAlign2DPara::GetPara(LPCTSTR para_name) const
{
    auto itAlgoIndex = m_algo_name_to_index.find(para_name);
    if (itAlgoIndex == m_algo_name_to_index.end())
        return nullptr;

    return &m_algo_ptr_list[itAlgoIndex->second]->m_para;
}

void VisionProcessingPadAlign2DPara::SetPara(LPCTSTR para_name, const AlignPara& para)
{
    auto itAlgoIndex = m_algo_name_to_index.find(para_name);
    if (itAlgoIndex == m_algo_name_to_index.end())
        return;

    m_algo_ptr_list[itAlgoIndex->second]->m_para = para;
}

void VisionProcessingPadAlign2DPara::SetAlignPAD(
    ParaDB& paraDB, LPCTSTR pad_name, LPCTSTR algo_name, enumPAD_Category category)
{
    CString old_algo_name = GetAlignPAD_ParaName(pad_name);
    if (old_algo_name == algo_name)
    {
        m_align_pad_category[m_pad_name_to_index[pad_name]] = category;
        return;
    }

    DelAlignPAD(pad_name);

    if (nullptr == GetPara(algo_name))
    {
        auto& para = AddAlgoParameter();
        para.m_name = algo_name;

        if (paraDB.ExistAlgorithmName(algo_name))
        {
            // Database 에 있는 Algorithm이 처음으로 추가되었다면 Parameter 를 복사해 준다
            para.m_para = *paraDB.getAlgorithmPara(algo_name);
        }

        Genereat_index_map();
    }

    m_align_pad_names.push_back(pad_name);
    m_align_pad_category.push_back(category);
    m_align_pad_algo_names.push_back(algo_name);

    Genereat_index_map();

    old_algo_name.Empty();
}

void VisionProcessingPadAlign2DPara::DelAlignPAD(LPCTSTR pad_name)
{
    CString target_algo_name = GetAlignPAD_ParaName(pad_name);
    if (target_algo_name.IsEmpty())
        return;

    for (long n = 0; n < long(m_align_pad_names.size()); n++)
    {
        if (m_align_pad_names[n] == pad_name)
        {
            m_align_pad_names.erase(m_align_pad_names.begin() + n);
            m_align_pad_category.erase(m_align_pad_category.begin() + n);
            m_align_pad_algo_names.erase(m_align_pad_algo_names.begin() + n);
            break;
        }
    }

    Genereat_index_map();

    for (auto& algo_name : m_align_pad_algo_names)
    {
        if (algo_name == target_algo_name)
        {
            // 아직 Algorithm Parameter를 쓰는 곳이 있으므로 지우지 말고 여기서 끝내자
            return;
        }
    }

    long algoIndex = m_algo_name_to_index[target_algo_name];

    // 쓰는 곳이 없으니 Algorithm Parameter 도 지우자
    delete m_algo_ptr_list[algoIndex];
    m_algo_ptr_list.erase(m_algo_ptr_list.begin() + algoIndex);

    Genereat_index_map();

    target_algo_name.Empty();
}

void VisionProcessingPadAlign2DPara::Genereat_index_map()
{
    m_pad_name_to_index.clear();
    m_algo_name_to_index.clear();

    for (long algo = 0; algo < long(m_algo_ptr_list.size()); algo++)
    {
        m_algo_name_to_index[m_algo_ptr_list[algo]->m_name] = algo;
    }

    for (long fiducial = 0; fiducial < long(m_align_pad_names.size()); fiducial++)
    {
        m_pad_name_to_index[m_align_pad_names[fiducial]] = fiducial;
    }
}

void VisionProcessingPadAlign2DPara::ResetAlgoParameter()
{
    for (auto* ptr : m_algo_ptr_list)
    {
        delete ptr;
    }

    m_algo_ptr_list.clear();
}

VisionProcessingPadAlign2DPara::AlgoParameter& VisionProcessingPadAlign2DPara::AddAlgoParameter()
{
    auto* object = new AlgoParameter(m_parent);
    m_algo_ptr_list.push_back(object);

    return *object;
}

std::vector<CString> VisionProcessingPadAlign2DPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    ResetTextRecipeAlgorithmNames();

    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Module 특성 변수 //채워야함
    strCategory.Format(_T("AlgorithmDB"));
    strGroup.Format(_T("Pad Align 2D Paramter"));

    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Algorithm DB Count"), _T(""), (long)m_algo_ptr_list.size()));

    for (long nAlgoSpecidx = 0; nAlgoSpecidx < (long)m_align_pad_names.size(); nAlgoSpecidx++)
    {
        strParaName = GetAlignPAD_ParaName(m_align_pad_names[nAlgoSpecidx]);

        strGroup.Format(_T("%s"), (LPCTSTR)strParaName);
        auto AlignSpec = GetAlignPAD_Para(m_align_pad_names[nAlgoSpecidx]);
        if (AlignSpec == nullptr)
            continue;

        //할당된 Pad Algorithm의 Name 검색
        if (IsNonDuplicated_AlgorithmName(strParaName) != false) //중복된 Pad Algorithm Name이 있는지 확인
            continue;

        PushTextRecipeAlgorithmNames(strParaName);

        //Non Circle Type
        if (GetPadType(m_align_pad_names[nAlgoSpecidx]) != PAD_TYPE::_typeCircle)
        {
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Blob (Prealign)"), _T("Threshold"), AlignSpec->m_blobThreshold));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Blob (Prealign)"), _T("Search Range (um)"), AlignSpec->m_blobSearchOffset_um));
        }
        //

        //Circle || Pinindex
        if (GetPadType(m_align_pad_names[nAlgoSpecidx]) == PAD_TYPE::_typeCircle
            || GetPadType(m_align_pad_names[nAlgoSpecidx]) == PAD_TYPE::_typePinIndex
                && GetAlignPAD_Category(m_align_pad_names[nAlgoSpecidx]) == enumPAD_Category::PinIndex)
        {
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Edge Detection"), _T("Search Offset (um)"), AlignSpec->m_edgeSearchOffset_um));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Edge Detection"), _T("Search Length (um)"), AlignSpec->m_edgeSearchLength_um));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Edge Detection"), _T("Search Direction"), AlignSpec->m_edgeSearchDirection));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Edge Detection"), _T("Threshold"), AlignSpec->m_edgeThreshold));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, _T("Edge Detection"), _T("Edge Direction"), AlignSpec->m_edgeType));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Edge Detection"), _T("Find First Edge"), (bool)AlignSpec->m_findFirstEdge));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Edge Detection"), _T("Search count"), AlignSpec->m_edgeSearchCount));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, _T("Edge Detection"), _T("Search Width"), AlignSpec->m_edgeSearchWidth));
        }
        //

        //Image Combine
        CString strPadCombineName("");
        strPadCombineName.Format(_T("%s-Image Combine"), (LPCTSTR)strParaName);
        std::vector<CString> vecstrImageCombineParameters(0);
        vecstrImageCombineParameters = AlignSpec->m_imageProc.ExportImageCombineParaToText(
            strVisionName, strInspectionModuleName, strPadCombineName);

        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(), vecstrImageCombineParameters.end());
    }

    strCategory.Format(_T("Align Spec"));
    for (long nPadidx = 0; nPadidx < (long)m_align_pad_names.size(); nPadidx++)
    {
        strGroup.Format(_T("%d"), nPadidx + 1);

        CString strPadName = m_align_pad_names[nPadidx];
        strParaName = GetAlignPAD_ParaName(m_align_pad_names[nPadidx]);

        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strPadName, _T(""), strParaName));
    }

    return vecstrAlgorithmParameters;
}

bool VisionProcessingPadAlign2DPara::IsNonDuplicated_AlgorithmName(const CString i_strPadAlgorithmName)
{
    for (auto AlgorithmName : m_vecstr_Pad_AlgorithmNames)
    {
        if (AlgorithmName.Find(i_strPadAlgorithmName) > -1) //찾은 녀석이 있는
            return true;
    }

    return false;
}

PAD_TYPE VisionProcessingPadAlign2DPara::GetPadType(const CString i_strPadName)
{
    auto& padList = m_packageSpec.m_PadMapManager->vecPadData;
    for (auto& padInfo : padList)
    {
        if (padInfo.strPadName == i_strPadName)
        {
            return padInfo.GetType();
        }
    }

    return _typeRectangle;
}

void VisionProcessingPadAlign2DPara::PushTextRecipeAlgorithmNames(const CString i_strPadAlgorithmName)
{
    m_vecstr_Pad_AlgorithmNames.push_back(i_strPadAlgorithmName);
}

void VisionProcessingPadAlign2DPara::ResetTextRecipeAlgorithmNames()
{
    m_vecstr_Pad_AlgorithmNames.clear();
}