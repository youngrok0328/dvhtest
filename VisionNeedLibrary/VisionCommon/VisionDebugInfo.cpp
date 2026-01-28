//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionDebugInfo.h"

//CPP_2_________________________________ This project's headers
#include "VisionAlignResult.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/FPI_RECT.h"
#include "../../DefineModules/dA_Base/PI_RECT.h"
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionWarpageShapeResult.h"
#include "../../UserInterfaceModules/ImageViewEx/ImageViewEx.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Base/BlobInfo.h>
#include <Ipvm/Base/Conversion.h>
#include <Ipvm/Base/EllipseEq32r.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image16u.h>
#include <Ipvm/Base/Image32r.h>
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/LineSeg32r.h>
#include <Ipvm/Base/Quadrangle32r.h>
#include <Ipvm/Base/Polygon32r.h>
#include <Ipvm/Base/Rect32r.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CString DebugInfoType2String(enumDebugInfoType type)
{
    switch (type)
    {
        case enumDebugInfoType::Image_8u_C1:
            return _T("Image_8u_C1");
        case enumDebugInfoType::Image_32f_C1:
            return _T("Image_32f_C1");
        case enumDebugInfoType::Long:
            return _T("Long");
        case enumDebugInfoType::Float:
            return _T("Float");
        case enumDebugInfoType::Double:
            return _T("Double");
        case enumDebugInfoType::Point:
            return _T("Point");
        case enumDebugInfoType::Point_32f_C2:
            return _T("Float Point");
        case enumDebugInfoType::LineSeg_32f:
            return _T("LineSeg");
        case enumDebugInfoType::Rect:
            return _T("Rectangle");
        case enumDebugInfoType::Rect_32f:
            return _T("Float Rectangle");
        case enumDebugInfoType::PI_Rect:
            return _T("QuadRangle");
        case enumDebugInfoType::FPI_Rect:
            return _T("Float QuadRangle");
        case enumDebugInfoType::BlobInfo:
            return _T("Blob Info");
        case enumDebugInfoType::EllipseEq_32f:
            return _T("EllipseEq");
        case enumDebugInfoType::Polygon_32f:
            return _T("Polygon");
        case enumDebugInfoType::Quadrangle_32f:
            return _T("Quadrangle");
        case enumDebugInfoType::CSTRING:
            return _T("CString");
    }

    return _T("Undefined");
}

VisionDebugInfo::VisionDebugInfo(
    LPCTSTR _moduleGUID, LPCTSTR moduleName, LPCTSTR infoName, enumDebugInfoType type, long key)
    : moduleGuid(_moduleGUID)
    , strInspName(moduleName)
    , strDebugInfoName(infoName)
    , nDataType(type)
    , nDataNum(0)
    , pData(nullptr)
    , m_key(key)
{
}

VisionDebugInfo::~VisionDebugInfo() //kircheis_ClearBlob
{
    Reset();
}

void VisionDebugInfo::Reset()
{
    switch (nDataType)
    {
        case enumDebugInfoType::Image_8u_C1:
            delete (Ipvm::Image8u*)pData;
            break;
        case enumDebugInfoType::Image_32f_C1:
            delete (Ipvm::Image32r*)pData;
            break;
        case enumDebugInfoType::Long:
            delete[] (long*)pData;
            break;
        case enumDebugInfoType::Float:
            delete[] (float*)pData;
            break;
        case enumDebugInfoType::Double:
            delete[] (double*)pData;
            break;
        case enumDebugInfoType::Point:
            delete[] (Ipvm::Point32s2*)pData;
            break;
        case enumDebugInfoType::Point_32f_C2:
            delete[] (Ipvm::Point32r2*)pData;
            break;
        case enumDebugInfoType::LineSeg_32f:
            delete[] (Ipvm::LineSeg32r*)pData;
            break;
        case enumDebugInfoType::Rect:
            delete[] (Ipvm::Rect32s*)pData;
            break;
        case enumDebugInfoType::Rect_32f:
            delete[] (Ipvm::Rect32r*)pData;
            break;
        case enumDebugInfoType::PI_Rect:
            delete[] (PI_RECT*)pData;
            break;
        case enumDebugInfoType::FPI_Rect:
            delete[] (FPI_RECT*)pData;
            break;
        case enumDebugInfoType::BlobInfo:
            delete[] (Ipvm::BlobInfo*)pData;
            break;
        case enumDebugInfoType::EllipseEq_32f:
            delete[] (Ipvm::EllipseEq32r*)pData;
            break;
        case enumDebugInfoType::Polygon_32f:
            delete[] (Ipvm::Polygon32r*)pData;
            break;
        case enumDebugInfoType::Quadrangle_32f:
            delete[] (Ipvm::Quadrangle32r*)pData;
            break;
        case enumDebugInfoType::CSTRING:
            delete[] (CString*)pData;
            break;

        case enumDebugInfoType::None:
            if (strDebugInfoName == _T("Ball Align Result") || strDebugInfoName == _T("Ball Align Result by Ball Insp")
                || strDebugInfoName == _T("SLand Align Result by SLand Insp")
                || strDebugInfoName == _T("EDGE Align Result"))
            {
                if (pData != NULL)
                {
                    VisionAlignResult* pTemp = (VisionAlignResult*)pData;
                    delete[] pTemp;
                }
            }
            else if (strDebugInfoName == _T("Warpage Shape Info")
                || strDebugInfoName == _T("Lid Warpage Shape Info")) //kircheis_InspLid
            {
                if (pData != NULL)
                {
                    VisionWarpageShapeResult* pTemp = (VisionWarpageShapeResult*)pData;
                    delete[] pTemp;
                }
            }
            else
            {
                CString strMessage;
                strMessage.Format(_T("Please.!! Delete Debug info memory!! \n ( %s )"), (LPCTSTR)strDebugInfoName);
                AfxMessageBox(strMessage);
                strMessage.Empty();
            }
            break;

        default:
        {
            CString strMessage;
            strMessage.Format(_T("Please.!! Delete Debug info memory!!"));
            AfxMessageBox(strMessage);
            strMessage.Empty();
        }
        break;
    }

    nDataNum = 0;
    pData = nullptr;
}

VisionDebugInfoGroup::VisionDebugInfoGroup(LPCTSTR _moduleGUID, LPCTSTR moduleName)
    : m_moduleGuid(_moduleGUID)
    , m_moduleName(moduleName)
{
}

VisionDebugInfoGroup::~VisionDebugInfoGroup() //kircheis_ClearBlob
{
}

void VisionDebugInfoGroup::Add(LPCTSTR infoName, enumDebugInfoType type, long key)
{
    m_vecDebugInfo.emplace_back(m_moduleGuid, m_moduleName, infoName, type, key);
}

VisionDebugInfo* VisionDebugInfoGroup::GetDebugInfo(LPCTSTR infoName)
{
    for (long i = 0; i < (long)(m_vecDebugInfo.size()); i++)
    {
        if (m_vecDebugInfo[i].strDebugInfoName == infoName)
        {
            return &m_vecDebugInfo[i];
        }
    }

    return NULL;
}

void VisionDebugInfo::ToTextOverlay(CString& strData, ImageViewEx* imageView)
{
    if (this == nullptr || nDataNum == 0)
        return;

    CString strTmp;

    //
    strData += _T("[") + strDebugInfoName + _T("]");
    strData += _T("\r\n");
    strTmp.Format(_T("Data Count : %d"), nDataNum);
    strData += strTmp;
    strData += _T("\r\n");
    strData += _T("\r\n");

    long i;
    if (pData != NULL)
    {
        switch (nDataType)
        {
            case enumDebugInfoType::Image_8u_C1:
            {
                Ipvm::Image8u* pImage = (Ipvm::Image8u*)pData;
                if (pImage)
                {
                    imageView->SetImage(*pImage);
                }

                strTmp.Format(_T("Width=%d, Height=%d"), pImage->GetSizeX(), pImage->GetSizeY());
                strData += strTmp;
                strData += _T("\r\n");

                break;
            }

            case enumDebugInfoType::Image_32f_C1:
            {
                Ipvm::Image32r* pImage = (Ipvm::Image32r*)pData;
                if (pImage)
                {
                    imageView->SetImage(*pImage, Ipvm::k_noiseValue32r, Ipvm::ColorMapIndex::e_rainbow);
                }

                strTmp.Format(_T("Width=%d, Height=%d"), pImage->GetSizeX(), pImage->GetSizeY());
                strData += strTmp;
                strData += _T("\r\n");

                break;
            }

            case enumDebugInfoType::Long:
            {
                for (i = 0; i < nDataNum; i++)
                {
                    long data = ((long*)pData)[i];
                    strTmp.Format(_T("[%04d] %d"), i + 1, data);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Float:
            {
                for (i = 0; i < nDataNum; i++)
                {
                    float fData = ((float*)pData)[i];
                    strTmp.Format(_T("[%04d] %f"), i + 1, fData);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Double:
            {
                for (i = 0; i < nDataNum; i++)
                {
                    double data = ((double*)pData)[i];
                    strTmp.Format(_T("[%04d] %f"), i + 1, data);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Point:
            {
                Ipvm::Point32s2* pptData = ((Ipvm::Point32s2*)pData);
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(Ipvm::Conversion::ToPoint32r2(pptData[i]), RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] %d, %d"), i + 1, pptData[i].m_x, pptData[i].m_y);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Point_32f_C2:
            {
                Ipvm::Point32r2* pfptData = (Ipvm::Point32r2*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    Ipvm::Point32r2 pt = pfptData[i];

                    imageView->ImageOverlayAdd(
                        Ipvm::LineSeg32r(pt.m_x, pt.m_y - 5.f, pt.m_x, pt.m_y + 5.f), RGB(0, 255, 0), 3.f);
                    imageView->ImageOverlayAdd(
                        Ipvm::LineSeg32r(pt.m_x - 5.f, pt.m_y, pt.m_x + 5.f, pt.m_y), RGB(0, 255, 0), 3.f);
                    //imageView->ImageOverlayAdd(pt, RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] %.3f, %.3f"), i + 1, pfptData[i].m_x, pfptData[i].m_y);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Rect:
            {
                Ipvm::Rect32s* prtData = (Ipvm::Rect32s*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(prtData[i], RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] %d, %d, %d, %d"), i + 1, prtData[i].m_left, prtData[i].m_right,
                        prtData[i].m_top, prtData[i].m_bottom);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Rect_32f:
            {
                Ipvm::Rect32r* prtData = (Ipvm::Rect32r*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(prtData[i], RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] %.3f, %.3f, %.3f, %.3f"), i + 1, prtData[i].m_left, prtData[i].m_right,
                        prtData[i].m_top, prtData[i].m_bottom);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::PI_Rect:
            {
                // bk 데이터 타입 잘못 연결되어 수정.
                PI_RECT* psrtData = (PI_RECT*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(psrtData[i], RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] LT(%d, %d), RT(%d, %d), LB(%d, %d), RB(%d, %d)"), i + 1,
                        psrtData[i].GetLTPoint().m_x, psrtData[i].GetLTPoint().m_y, psrtData[i].GetRTPoint().m_x,
                        psrtData[i].GetRTPoint().m_y, psrtData[i].GetLBPoint().m_x, psrtData[i].GetLBPoint().m_y,
                        psrtData[i].GetRBPoint().m_x, psrtData[i].GetRBPoint().m_y);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }

            case enumDebugInfoType::FPI_Rect:
            {
                // bk 데이터 타입 잘못 연결되어 수정.
                FPI_RECT* psfrtData = (FPI_RECT*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(psfrtData[i], RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f, %.3f"), i + 1,
                        psfrtData[i].fptLT.m_x, psfrtData[i].fptLT.m_y, psfrtData[i].fptRT.m_x, psfrtData[i].fptRT.m_y,
                        psfrtData[i].fptLB.m_x, psfrtData[i].fptLB.m_y, psfrtData[i].fptRB.m_x, psfrtData[i].fptRB.m_y);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }

            case enumDebugInfoType::EllipseEq_32f:
            {
                //Ipvm::Rect32r* pfData = (Ipvm::Rect32r*) pData;//kircheis_RoundPKG
                Ipvm::EllipseEq32r* pfData = (Ipvm::EllipseEq32r*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(pfData[i], RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] %.3f, %.3f, %.3f, %.3f"), i + 1, pfData[i].m_x, pfData[i].m_y,
                        pfData[i].m_xradius, pfData[i].m_yradius);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::LineSeg_32f:
            {
                Ipvm::LineSeg32r* prtData = (Ipvm::LineSeg32r*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(prtData[i], RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] %.2f, %.2f, %.2f, %.2f"), i + 1, prtData[i].m_sx, prtData[i].m_sy,
                        prtData[i].m_ex, prtData[i].m_ex);
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Polygon_32f:
            {
                Ipvm::Polygon32r* prtData = (Ipvm::Polygon32r*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(prtData[i], RGB(0, 255, 0));
                }

                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] VertexCount : %d"), i + 1, prtData[i].GetVertexNum());
                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::Quadrangle_32f:
            {
                Ipvm::Quadrangle32r* prtData = (Ipvm::Quadrangle32r*)pData;
                for (i = 0; i < nDataNum; i++)
                {
                    imageView->ImageOverlayAdd(prtData[i], RGB(0, 255, 0));
                }
                for (i = 0; i < nDataNum; i++)
                {
                    strTmp.Format(_T("[%04d] LT(%.3f, %.3f), RT(%.3f, %.3f), LB(%.3f, %.3f), RB(%.3f, %.3f)"), i + 1,
                        prtData[i].m_ltX, prtData[i].m_ltY, prtData[i].m_rtX, prtData[i].m_rtY, prtData[i].m_lbX,
                        prtData[i].m_lbY, prtData[i].m_rbX, prtData[i].m_rbY);

                    strData += strTmp;
                    strData += _T("\r\n");
                }
                break;
            }
            case enumDebugInfoType::CSTRING:
            {
                for (i = 0; i < nDataNum; i++)
                {
                    CString strTemp = ((CString*)pData)[i];
                    if (strTemp.IsEmpty())
                        strTemp.Format(_T("- EMPTY - "));
                    strData.AppendFormat(_T("[%04d] %s"), i + 1, (LPCTSTR)strTemp);
                    strData += _T("\r\n");
                    strTemp.Empty();
                }
                break;
            }
        }
    }
    strTmp.Empty();
}
