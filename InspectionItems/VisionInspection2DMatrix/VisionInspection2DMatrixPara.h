#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
enum enumMarkAngleType //kircheis_580 OCV
{
    ENUM_MARK_ANGLE_000 = 0,
    ENUM_MARK_ANGLE_090,
    ENUM_MARK_ANGLE_180,
    ENUM_MARK_ANGLE_270,
};

class VisionInspection2DMatrixPara
{
public:
    VisionInspection2DMatrixPara(void);
    ~VisionInspection2DMatrixPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    BOOL m_validOnlyNumberAndAlphabet;
    long m_minimumLengthForNormalString;
    long m_maximumLengthForNormalString;

    // ROI를 Vector로 수정
    std::vector<Ipvm::Rect32r> m_vecrtSearchMajorID_BCU;
    long m_nMajorID_Count;
    void SetSearchMajorROI_Count(long nNewCount);

    std::vector<Ipvm::Rect32r> m_vecrtSearchSubID_BCU;
    long m_nSubID_Count;
    void SetSearchSubROI_Count(long nNewCount);

    std::vector<Ipvm::Rect32r> m_vecrtSearchBabyID_BCU;
    long m_nBabyID_Count;
    void SetSearchBabyROI_Count(long nNewCount);

    //{{//kircheis_MED5_13
    long m_n1st2DIDforMatch; //2DID 매칭 검사 시 첫번째 매칭대상의 ID. enum MATCH_2DID_TYPE 참조
    long m_n2nd2DIDforMatch; //2DID 매칭 검사 시 두번째 매칭대상의 ID
    //}}
};
