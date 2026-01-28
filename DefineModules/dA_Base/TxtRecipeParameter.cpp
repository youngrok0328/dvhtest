//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "TxtRecipeParameter.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CTxtRecipeParameter::CTxtRecipeParameter()
{
}
CTxtRecipeParameter::~CTxtRecipeParameter()
{
}

CString CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const CString strParaNameAux,
    const bool bValue) //strHeader : VisionName,ModuleName
{
    long nResult = bValue ? 1 : 0;
    return GetTextFromParmeter(strHeader, strCateroty, strGroup, strParaName, strParaNameAux, nResult);
}

CString CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const CString strParaNameAux,
    const short sValue) //strHeader : VisionName,ModuleName
{
    return GetTextFromParmeter(strHeader, strCateroty, strGroup, strParaName, strParaNameAux, (long)sValue);
}

CString CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const CString strParaNameAux,
    const int nValue) //strHeader : VisionName,ModuleName
{
    return GetTextFromParmeter(strHeader, strCateroty, strGroup, strParaName, strParaNameAux, (long)nValue);
}

CString CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const CString strParaNameAux,
    const long lValue) //strHeader : VisionName,ModuleName
{
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%d"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, lValue);
    return strResult;
}

CString CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const CString strParaNameAux,
    const float fValue) //strHeader : VisionName,ModuleName
{
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, fValue);
    return strResult;
}

CString CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const CString strParaNameAux,
    const double dValue) //strHeader : VisionName,ModuleName
{
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%lf"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, dValue);
    return strResult;
}

CString CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const CString strParaNameAux,
    const CString strValue) //strHeader : VisionName,ModuleName
{
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%s"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, (LPCTSTR)strValue);
    return strResult;
}

std::vector<CString> CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const Ipvm::Rect32s rtRect,
    const bool bInsertROI) //strHeader : VisionName,ModuleName
{
    std::vector<CString> vecStrResult(0);
    CString strResult;
    CString strParaNameAux;
    strParaNameAux = bInsertROI ? (_T("[ROI]Left")) : (_T("Left"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%d"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, rtRect.m_left);
    vecStrResult.push_back(strResult);
    strParaNameAux = bInsertROI ? (_T("[ROI]Top")) : (_T("Top"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%d"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, rtRect.m_top);
    vecStrResult.push_back(strResult);
    strParaNameAux = bInsertROI ? (_T("[ROI]Right")) : (_T("Right"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%d"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, rtRect.m_right);
    vecStrResult.push_back(strResult);
    strParaNameAux = bInsertROI ? (_T("[ROI]Bottom")) : (_T("Bottom"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%d"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, rtRect.m_bottom);
    vecStrResult.push_back(strResult);

    return vecStrResult;
}

std::vector<CString> CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName, const Ipvm::Rect32r frtRect,
    const bool bInsertROI) //strHeader : VisionName,ModuleName
{
    std::vector<CString> vecStrResult(0);
    CString strResult;
    CString strParaNameAux;
    strParaNameAux = bInsertROI ? (_T("[ROI]Left")) : (_T("Left"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, frtRect.m_left);
    vecStrResult.push_back(strResult);
    strParaNameAux = bInsertROI ? (_T("[ROI]Top")) : (_T("Top"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, frtRect.m_top);
    vecStrResult.push_back(strResult);
    strParaNameAux = bInsertROI ? (_T("[ROI]Right")) : (_T("Right"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, frtRect.m_right);
    vecStrResult.push_back(strResult);
    strParaNameAux = bInsertROI ? (_T("[ROI]Bottom")) : (_T("Bottom"));
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, (LPCTSTR)strParaNameAux, frtRect.m_bottom);
    vecStrResult.push_back(strResult);

    return vecStrResult;
}

std::vector<CString> CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName,
    const Ipvm::Rectangle32r frtRect) //strHeader : VisionName,ModuleName
{
    std::vector<CString> vecStrResult(0);
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Center X"), frtRect.m_x);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Center Y"), frtRect.m_y);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Width"), frtRect.m_width);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Height"), frtRect.m_height);
    vecStrResult.push_back(strResult);

    return vecStrResult;
}

std::vector<CString> CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName,
    const Ipvm::Point32s2 ptPoint) //strHeader : VisionName,ModuleName
{
    std::vector<CString> vecStrResult(0);
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%d"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Point X"), ptPoint.m_x);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%d"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Point Y"), ptPoint.m_y);
    vecStrResult.push_back(strResult);

    return vecStrResult;
}

std::vector<CString> CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName,
    const Ipvm::Point32r2 fptPoint) //strHeader : VisionName,ModuleName
{
    std::vector<CString> vecStrResult(0);
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Point X"), fptPoint.m_x);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Point Y"), fptPoint.m_y);
    vecStrResult.push_back(strResult);

    return vecStrResult;
}

std::vector<CString> CTxtRecipeParameter::GetTextFromParmeter(const CString strHeader, const CString strCateroty,
    const CString strGroup, const CString strParaName,
    const Ipvm::EllipseEq32r eqEllipse) //strHeader : VisionName,ModuleName
{
    std::vector<CString> vecStrResult(0);
    CString strResult;
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Center X"), eqEllipse.m_x);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Center Y"), eqEllipse.m_y);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Radius X"), eqEllipse.m_xradius);
    vecStrResult.push_back(strResult);
    strResult.Format(_T("%s,%s,%s,%s,%s,%f"), (LPCTSTR)strHeader, (LPCTSTR)strCateroty, (LPCTSTR)strGroup,
        (LPCTSTR)strParaName, _T("Radius Y"), eqEllipse.m_yradius);
    vecStrResult.push_back(strResult);

    return vecStrResult;
}

CString CTxtRecipeParameter::GetStringCount(long nCnt)
{
    CString str;
    if (nCnt <= 0)
        return _T("");
    else if (nCnt == 1)
        str.Format(_T("1st"));
    else if (nCnt == 2)
        str.Format(_T("2nd"));
    else if (nCnt == 3)
        str.Format(_T("3rd"));
    else
        str.Format(_T("%dth"), nCnt);

    return str;
}