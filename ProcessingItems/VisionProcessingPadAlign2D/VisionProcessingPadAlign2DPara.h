#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "AlignPara.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class ImageProcPara;
class ParaDB;

//HDR_6_________________________________ Header body
//
enum class enumPAD_Category
{
    Blank = -1,
    Normal,
    Fiducial,
    PinIndex,
    END
};

LPCTSTR GetPAD_CategoryName(enumPAD_Category category);

class VisionProcessingPadAlign2DPara
{
public:
    VisionProcessingPadAlign2DPara(const VisionProcessingPadAlign2DPara& object) = delete;
    VisionProcessingPadAlign2DPara(VisionProcessing& parent, CPackageSpec& packageSpec);
    ~VisionProcessingPadAlign2DPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    void Init();
    long GetAlignPAD_Count() const;
    long FindAlignPAD_Index(LPCTSTR pad_name) const;
    LPCTSTR GetAlignPAD_Name(long index) const;
    const AlignPara* GetAlignPAD_Para(LPCTSTR pad_name) const;
    AlignPara* GetAlignPAD_Para(LPCTSTR pad_name);
    LPCTSTR GetAlignPAD_ParaName(LPCTSTR pad_name) const;
    LPCTSTR GetAlignPAD_ParaName(long index) const;
    enumPAD_Category GetAlignPAD_Category(LPCTSTR pad_name) const;
    enumPAD_Category GetAlignPAD_Category(long index) const;
    long GetParaCount() const;
    LPCTSTR GetParaName(long index) const;
    const AlignPara* GetPara(LPCTSTR para_name) const;
    void SetPara(LPCTSTR para_name, const AlignPara& para);

    void SetAlignPAD(ParaDB& paraDB, LPCTSTR pad_name, LPCTSTR algo_name, enumPAD_Category category);
    void DelAlignPAD(LPCTSTR pad_name);

private:
    VisionProcessing& m_parent;
    CPackageSpec& m_packageSpec;

    std::vector<CString> m_align_pad_names; // PAD 의 이름은 무엇인가
    std::vector<enumPAD_Category> m_align_pad_category; // PAD 는 구분은 무엇인가
    std::vector<CString> m_align_pad_algo_names; // PAD 에 대한 세팅된 알고리즘 이름

    class AlgoParameter
    {
    public:
        CString m_name;
        AlignPara m_para;

        AlgoParameter(VisionProcessing& parent);
    };

    std::vector<AlgoParameter*> m_algo_ptr_list;

    // 내부 자동생성 변수
    std::map<CString, long> m_pad_name_to_index;
    std::map<CString, long> m_algo_name_to_index;

    void Genereat_index_map();
    void ResetAlgoParameter();
    AlgoParameter& AddAlgoParameter();

private: //mc_TextRecipe
    bool IsNonDuplicated_AlgorithmName(const CString i_strPadAlgorithmName);
    std::vector<CString> m_vecstr_Pad_AlgorithmNames; //할당된 Algorithm에 대한 정보를 중복으로 Export하지 않기 위하여
    PAD_TYPE GetPadType(const CString i_strPadName);
    void PushTextRecipeAlgorithmNames(const CString i_strPadAlgorithmName); //중복으로 Export하면 않되니까..
    void ResetTextRecipeAlgorithmNames();
};