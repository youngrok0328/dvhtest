//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspection2DMatrixPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/semiinfo.h" //kircheis_MED5_13
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspection2DMatrixPara::VisionInspection2DMatrixPara(void)
    : m_validOnlyNumberAndAlphabet(TRUE)
    , m_minimumLengthForNormalString(0)
    , m_maximumLengthForNormalString(60)
{
    // ROI를 Vector로 수정
    m_nMajorID_Count = 0;
    m_nSubID_Count = 0;
    m_nBabyID_Count = 0;

    //{{//kircheis_MED5_13
    m_n1st2DIDforMatch = TYPE_2DID_MAJOR;
    m_n2nd2DIDforMatch = TYPE_2DID_SUB_0;
    //}}
}

VisionInspection2DMatrixPara::~VisionInspection2DMatrixPara(void)
{
}

BOOL VisionInspection2DMatrixPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    long version = 1;
    if (!db[_T("Version")].Link(bSave, version))
        version = 0;

    if (!db[_T("Valid only number and alphabet")].Link(bSave, m_validOnlyNumberAndAlphabet))
    {
        m_validOnlyNumberAndAlphabet = TRUE;
    }

    if (!db[_T("Minimum length for normal string")].Link(bSave, m_minimumLengthForNormalString))
    {
        m_minimumLengthForNormalString = 0;
    }

    if (!db[_T("Maximum length for normal string")].Link(bSave, m_maximumLengthForNormalString))
    {
        m_maximumLengthForNormalString = 60;
    }

    //{{ 2D Matrix
    if (!db[_T("MajorID_Count")].Link(bSave, m_nMajorID_Count))
        m_nMajorID_Count = 0;
    if (!db[_T("SubID_Count")].Link(bSave, m_nSubID_Count))
        m_nSubID_Count = 0;
    if (!db[_T("BabyID_Count")].Link(bSave, m_nBabyID_Count))
        m_nBabyID_Count = 0;

    // ROI를 Vector로 수정
    if (!bSave)
    {
        m_vecrtSearchMajorID_BCU.clear();
        m_vecrtSearchSubID_BCU.clear();
        m_vecrtSearchBabyID_BCU.clear();

        CString strKeyName;

        m_nMajorID_Count = (long)max(0, m_nMajorID_Count);
        m_nMajorID_Count = (long)min(1, m_nMajorID_Count);
        m_nSubID_Count = (long)max(0, m_nSubID_Count);
        m_nBabyID_Count = (long)max(0, m_nBabyID_Count);

        m_vecrtSearchMajorID_BCU.resize(m_nMajorID_Count);
        m_vecrtSearchSubID_BCU.resize(m_nSubID_Count);
        m_vecrtSearchBabyID_BCU.resize(m_nBabyID_Count);
        strKeyName.Empty();
    }
    else
    {
        long nROIVecNum = (long)m_vecrtSearchMajorID_BCU.size();
        if (m_nMajorID_Count != nROIVecNum)
        {
            ASSERT(!(_T("Warning Major 2DID Job")));
            m_nMajorID_Count = nROIVecNum;
            if (!db[_T("MajorID_Count")].Link(bSave, m_nMajorID_Count))
                m_nMajorID_Count = nROIVecNum;
        }
        nROIVecNum = (long)m_vecrtSearchSubID_BCU.size();
        if (m_nSubID_Count != nROIVecNum)
        {
            ASSERT(!(_T("Warning Sub 2DID Job")));
            m_nSubID_Count = nROIVecNum;
            if (!db[_T("SubID_Count")].Link(bSave, m_nSubID_Count))
                m_nSubID_Count = nROIVecNum;
        }
        nROIVecNum = (long)m_vecrtSearchBabyID_BCU.size();
        if (m_nBabyID_Count != nROIVecNum)
        {
            ASSERT(!(_T("Warning Baby 2DID Job")));
            m_nBabyID_Count = nROIVecNum;
            if (!db[_T("BabyID_Count")].Link(bSave, m_nBabyID_Count))
                m_nBabyID_Count = nROIVecNum;
        }
    }

    CString strKeyName;

    if (version == 0)
    {
        // 과거 IMAGE 좌표계 기준이었던 옛날 버전 Recipe이다. 더이상 지원하지 않는다
        return FALSE;
    }
    else
    {
        for (long nVecNo = 0; nVecNo < m_nMajorID_Count; nVecNo++)
        {
            strKeyName.Format(_T("Major2DID_SearchROI_%d"), nVecNo);
            if (!LinkEx(bSave, db[strKeyName], m_vecrtSearchMajorID_BCU[nVecNo]))
            {
                m_vecrtSearchMajorID_BCU[nVecNo] = Ipvm::Rect32r(-2000.f, -2000.f, 2000.f, 2000.f);
            }
        }
        for (long nVecNo = 0; nVecNo < m_nSubID_Count; nVecNo++)
        {
            strKeyName.Format(_T("Sub2DID_SearchROI_%d"), nVecNo);
            if (!LinkEx(bSave, db[strKeyName], m_vecrtSearchSubID_BCU[nVecNo]))
            {
                m_vecrtSearchSubID_BCU[nVecNo] = Ipvm::Rect32r(-3000.f, -2000.f, 1000.f, 2000.f);
            }
        }
        for (long nVecNo = 0; nVecNo < m_nBabyID_Count; nVecNo++)
        {
            strKeyName.Format(_T("Baby2DID_SearchROI_%d"), nVecNo);
            if (!LinkEx(bSave, db[strKeyName], m_vecrtSearchBabyID_BCU[nVecNo]))
            {
                m_vecrtSearchBabyID_BCU[nVecNo] = Ipvm::Rect32r(-1000.f, -2000.f, 3000.f, 2000.f);
            }
        }
    }

    //{{//kircheis_MED5_13
    if (bSave == TRUE && m_n1st2DIDforMatch == m_n2nd2DIDforMatch) //이 두개의 Target ID가 같으면 안되니까
    {
        m_n2nd2DIDforMatch = (m_n1st2DIDforMatch + 1)
            % TYPE_2DID_END; //같으면 두번째 ID를 첫번쨰 ID의 다음 ID로 하되 첫번째 ID가 마지막인 Baby_2이면 Major로 만든다.
    }

    if (!db[_T("2DID_A_ID_for_matching")].Link(bSave, m_n1st2DIDforMatch))
        m_n1st2DIDforMatch = TYPE_2DID_MAJOR;
    if (!db[_T("2DID_B_ID_for_matching")].Link(bSave, m_n2nd2DIDforMatch))
        m_n2nd2DIDforMatch = TYPE_2DID_SUB_0;

    if (m_n1st2DIDforMatch == TYPE_2DID_BABY_1)
        m_n1st2DIDforMatch = TYPE_2DID_MAJOR;

    if (m_n2nd2DIDforMatch == TYPE_2DID_BABY_1)
        m_n2nd2DIDforMatch = TYPE_2DID_SUB_0;

    if (bSave == FALSE && m_n1st2DIDforMatch == m_n2nd2DIDforMatch) //이 두개의 Target ID가 같으면 안되니까
    {
        m_n2nd2DIDforMatch = (m_n1st2DIDforMatch + 1)
            % TYPE_2DID_END; //같으면 두번째 ID를 첫번쨰 ID의 다음 ID로 하되 첫번째 ID가 마지막인 Baby_2이면 Major로 만든다.
    }
    //}}

    strKeyName.Empty();
    // }}
    return TRUE;
}

void VisionInspection2DMatrixPara::SetSearchMajorROI_Count(long nNewCount)
{
    nNewCount = (long)max(0, nNewCount);
    nNewCount = (long)min(1, nNewCount);

    long nOldVecRoiNum = (long)m_vecrtSearchMajorID_BCU.size();
    long nOldRoiNum = m_nMajorID_Count;

    if (nOldRoiNum != nOldVecRoiNum)
        ASSERT(!(_T("ROI Num is different in Matrix Job")));

    m_nMajorID_Count = nNewCount;
    m_vecrtSearchMajorID_BCU.resize(m_nMajorID_Count, Ipvm::Rect32r(-2000.f, -2000.f, 2000.f, 2000.f));
}

void VisionInspection2DMatrixPara::SetSearchSubROI_Count(long nNewCount)
{
    nNewCount = (long)max(0, nNewCount);

    long nOldVecRoiNum = (long)m_vecrtSearchSubID_BCU.size();
    long nOldRoiNum = m_nSubID_Count;

    if (nOldRoiNum != nOldVecRoiNum)
        ASSERT(!(_T("ROI Num is different in Matrix Job")));

    m_nSubID_Count = nNewCount;
    m_vecrtSearchSubID_BCU.resize(m_nSubID_Count, Ipvm::Rect32r(-3000.f, -2000.f, 1000.f, 2000.f));
}

void VisionInspection2DMatrixPara::SetSearchBabyROI_Count(long nNewCount)
{
    nNewCount = (long)max(0, nNewCount);
    nNewCount = (long)min(1, nNewCount);

    long nOldVecRoiNum = (long)m_vecrtSearchBabyID_BCU.size();
    long nOldRoiNum = m_nBabyID_Count;

    if (nOldRoiNum != nOldVecRoiNum)
        ASSERT(!(_T("ROI Num is different in Matrix Job")));

    m_nBabyID_Count = nNewCount;
    m_vecrtSearchBabyID_BCU.resize(m_nBabyID_Count, Ipvm::Rect32r(-1000.f, -2000.f, 3000.f, 2000.f));
}

std::vector<CString> VisionInspection2DMatrixPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), LPCTSTR(strVisionName), LPCTSTR(strInspectionModuleName));

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;
    CString strParaNameAux;
    CString strValue;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Module 특성 변수 // 채워야 함
    //Algorithm Parameters
    strCategory.Format(_T("Algorithm Parameters"));

    //Main Parameters
    strGroup.Format(_T("2D Barcode Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Valid only number and alphabet"), _T(""), (bool)m_validOnlyNumberAndAlphabet));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Minimum length for normal string"), _T(""), m_minimumLengthForNormalString));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        _T("Maximum length for normal string"), _T(""), m_maximumLengthForNormalString));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Major 2DID number"), _T(""), m_nMajorID_Count));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Sub 2DID number"), _T(""), m_nSubID_Count));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Baby 2DID number"), _T(""), m_nBabyID_Count));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("2DID A ID for matching"), _T(""), m_n1st2DIDforMatch));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("2DID B ID for matching"), _T(""), m_n2nd2DIDforMatch));

    //ROI
    std::vector<CString> vecStrRoiInfoBuffer(
        0); // ROI 리턴하는 함수는 벡터로 리턴되니 버퍼에 넣고 Insert 함수를 써야한다.
    strGroup.Format(_T("ROI"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Major ROI"), _T("Number"), m_nMajorID_Count));
    if (m_nMajorID_Count > 0)
    {
        vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, _T("Major"), m_vecrtSearchMajorID_BCU[0]);
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
    }

    long nROI_Num = (long)m_vecrtSearchSubID_BCU.size();
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Sub ROI"), _T("Number"), nROI_Num));
    for (long nROI = 0; nROI < nROI_Num; nROI++)
    {
        strParaName.Format(_T("Sub %d"), nROI + 1);
        vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, m_vecrtSearchSubID_BCU[nROI]);
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
    }

    nROI_Num = (long)m_vecrtSearchBabyID_BCU.size();
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Baby ROI"), _T("Number"), nROI_Num));
    for (long nROI = 0; nROI < nROI_Num; nROI++)
    {
        strParaName.Format(_T("Baby %d"), nROI + 1);
        vecStrRoiInfoBuffer = CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, m_vecrtSearchBabyID_BCU[nROI]);
        vecstrAlgorithmParameters.insert(
            vecstrAlgorithmParameters.end(), vecStrRoiInfoBuffer.begin(), vecStrRoiInfoBuffer.end());
    }

    return vecstrAlgorithmParameters;
}
