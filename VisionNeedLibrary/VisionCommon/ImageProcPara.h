#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "VisionInspFrameIndex.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
enum enumImageCombine
{
    enumImageCombine_Begin = 0,
    enumImageCombine_NotThing = enumImageCombine_Begin,
    enumImageCombine_Add,
    enumImageCombine_Sub1_2,
    enumImageCombine_Sub2_1,
    enumImageCombine_And,
    enumImageCombine_Or,
    enumImageCombine_Add_NS,
    enumImageCombine_Sub1_2_NS,
    enumImageCombine_Sub2_1_NS,
    enumImageCombine_Div_NS,
    enumImageCombine_Mul_NS,
    enumImageCombine_MAX, //kircheis_DieCrack
    enumImageCombine_MIN, //kircheis_DieCrack
    enumImageCombine_End,
};

class __VISION_COMMON_API__ ImageProcPara
{
public:
    ImageProcPara(const ImageProcPara& object) = delete;
    ImageProcPara(VisionProcessing& parent);
    ~ImageProcPara();

    bool isCombine();

    VisionInspFrameIndex m_aux1FrameID;
    VisionInspFrameIndex m_aux2FrameID;
    short nFirstCombineType;
    short nSecondCombineType;

    std::vector<short> vecnCurProc;
    std::vector<short> vecnCurProcParams;

    std::vector<short> vecnAux1Proc;
    std::vector<short> vecnAux1ProcParams;

    std::vector<short> vecnAux2Proc;
    std::vector<short> vecnAux2ProcParams;

    std::vector<short> vecnFirstCombineProc;
    std::vector<short> vecnFirstCombineProcParams;

    std::vector<short> vecnSecondCombineProc;
    std::vector<short> vecnSecondCombineProcParams;

    void Init();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& procParaDB);

    ImageProcPara& operator=(const ImageProcPara& srcObj);

    std::vector<CString> ExportImageCombineParaToText(
        const CString strVisionName, const CString strModuleName, const CString strCategory); //kircheis_TxtRecipe

private:
    VisionProcessing& m_parent;
};
