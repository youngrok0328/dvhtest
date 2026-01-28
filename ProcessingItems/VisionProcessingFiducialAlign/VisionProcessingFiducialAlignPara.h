#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
enum UseImage
{
    use_start = 0,
    use_zmap = use_start,
    use_vmap,
    use_end,
};

class VisionProcessingFiducialAlignPara
{
public:
    VisionProcessingFiducialAlignPara(VisionProcessing& parent);
    ~VisionProcessingFiducialAlignPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    void Init();
    long FindAlignPAD_Index(LPCTSTR pad_name) const;
    void SetAlignPAD(LPCTSTR pad_name);
    long GetAlignPAD_Count() const;
    void DelAlignPAD(LPCTSTR pad_name);
    LPCTSTR GetAlignPAD_Name(long index) const;

    ImageProcPara m_imageProc;
    float m_SearchOffset_um; // Search 범위
    long m_select_use_image;

    std::vector<CString> m_use_pad_names;
    std::vector<CString> m_align_pad_names;
    std::map<CString, long> m_pad_name_to_index;

private:
    void Genereat_index_map();
};
