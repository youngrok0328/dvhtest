//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionSurfacePara.h"

//CPP_2_________________________________ This project's headers
#include "SurfaceCustomROIItem.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionSurfacePara::VisionInspectionSurfacePara(VisionProcessing& parent)
    : m_ImageProcMangePara(parent)
{
}

VisionInspectionSurfacePara::~VisionInspectionSurfacePara(void)
{
}

BOOL VisionInspectionSurfacePara::LinkDataBase(BOOL bSave, CiDataBase& db, LPCTSTR moduleName)
{
    long version = 1;

    if (!db[_T("{97744EC1-3CDD-4AFD-84B6-B3AC1C7BD1BD}")].Link(bSave, version))
    {
        version = 0;
    }

    m_SurfaceItem.LinkDataBase(bSave, db[_T("{CB44F581-60F0-4622-9802-F5DF19C098AD}")]);
    m_ImageProcMangePara.LinkDataBase(bSave, db[_T("{4C399FB4-D7D5-479E-8148-3BA7FBEE5EE4}")]);

    // Surface Mask를 쓰는데 항상 이미지가지고 새로 만드는 것이 아니면 저장하자
    if (m_SurfaceItem.m_bSurfaceMaskMode && !m_SurfaceItem.m_bSurfaceMask_GenerateAlways)
    {
        auto& dbMask = db[_T("{E2121F92-0779-4D55-BCEA-29F43CEAB244}")];

        if (version == 0)
        {
            long nVecSize = 0;

            if (!dbMask[_T("SurfaceMask_Size")].Link(bSave, nVecSize))
                nVecSize = 0;

            for (long n = 0; n < nVecSize; n++)
            {
                auto& subMask = dbMask.GetSubDBFmt(_T("Mask%d"), n + 1);

                BOOL bSaveCheck = TRUE;
                subMask[_T("Save")].Link(bSave, bSaveCheck);
                if (!bSaveCheck)
                    continue;

                CString name;
                subMask[_T("Name")].Link(bSave, name);
                if (name != moduleName)
                    continue;

                name.Empty();

                auto& image = m_SurfaceBitmapMask;

                long imageSizeX = image.GetSizeX();
                long imageSizeY = image.GetSizeY();

                if (!subMask[_T("SizeX")].Link(bSave, imageSizeX))
                    imageSizeX = 0;
                if (!subMask[_T("SizeY")].Link(bSave, imageSizeY))
                    imageSizeY = 0;

                image.Create(imageSizeX, imageSizeY);

                if (!subMask[_T("Buffer")].LinkArray(
                        bSave, image.GetMem(), sizeof(BYTE) * image.GetWidthBytes() * image.GetSizeX()))
                {
                    memset(image.GetMem(), 0, sizeof(BYTE) * image.GetWidthBytes() * image.GetSizeX());
                }
                break;
            }
        }
        else
        {
            auto& image = m_SurfaceBitmapMask;

            long imageSizeX = image.GetSizeX();
            long imageSizeY = image.GetSizeY();

            if (!dbMask[_T("SizeX")].Link(bSave, imageSizeX))
                imageSizeX = 0;
            if (!dbMask[_T("SizeY")].Link(bSave, imageSizeY))
                imageSizeY = 0;

            if (!bSave)
            {
                image.Create(imageSizeX, imageSizeY);
            }

            if (!dbMask[_T("Buffer")].LinkArray(
                    bSave, image.GetMem(), sizeof(BYTE) * image.GetWidthBytes() * image.GetSizeY()))
            {
                memset(image.GetMem(), 0, sizeof(BYTE) * image.GetWidthBytes() * image.GetSizeY());
            }
        }
    }

    return TRUE;
}

std::vector<CString> VisionInspectionSurfacePara::ExportAlgoParaToText(
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

    CString arStrModuleName[] = {
        _T("BOTTOM2D"),
        _T("BOTTOM3D"),
        _T("TOP2D"),
        _T("TOP3D"),
        _T("REVIEW"),
    };

    CString arStrValidation[] = {_T("-"), _T("+")};

    bool bIs3D = false;

    //Module 특성 변수 //채워야 함

    /********************************************************************* Basic Parameter **********************************************************************/
    strCategory.Format(_T("Algorithm Parameters"));

    // Basic Parameter
    strGroup.Format(_T("Basic Parameter"));

    strParaName.Format(_T("Surface Mask Mode"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, (bool)m_SurfaceItem.m_bSurfaceMaskMode));

    //if (strVisionName == arStrModuleName[1] || strVisionName == arStrModuleName[3])
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
    {
        //3D Vision일 경우에만 표시한다.
        strParaName.Format(_T("Use Image"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.nUse3DImageType));
    }

    strParaName.Format(_T("Minimum Area for Key Contrast (Bright)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.fMinAreaBrightContrast));
    strParaName.Format(_T("Minimum Area for Key Contrast (Dark)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.fMinAreaDarkContrast));
    strParaName.Format(_T("Merge Distance"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_mergeDistance));
    strParaName.Format(_T("Merge Only The Same Color"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, strParaNameAux, (bool)m_SurfaceItem.AlgoPara.m_mergeOnlyTheSameColor));
    strParaName.Format(_T("Load Contrast Area Span"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.fLocalContrastAreaSpan));

    // Background Parameter
    strGroup.Format(_T("Background Parameter"));

    strParaName.Format(_T("Window Size X"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_dbackground_window_size_x));
    strParaName.Format(_T("Window Size Y"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_dbackground_window_size_y));

    // Basic Parameter
    strGroup.Format(_T("Blob Parameter"));

    strParaName.Format(_T("Minimum Bright Blob Area"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.fMinimumBrightBlobArea_umSqure));
    strParaName.Format(_T("Minimum Dark Blob Area"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.fMininumDarkBlobArea_umSqure));
    strParaName.Format(_T("Maximum Bright Blbo Count"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.nMaxBrightBlobNum_New));
    strParaName.Format(_T("Maximum Dark Blob Count"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.nMaxDarkBlobNum_New));

    //Image Combine
    //if ((strVisionName == arStrModuleName[1] || strVisionName == arStrModuleName[3]) && m_SurfaceItem.AlgoPara.nUse3DImageType == SURFACE_3D_USE_ZMAP) // 3D 검사에 대해 확인한다. VMap을 사용하는 경우에는 2D 검사와 동일하게 검사를 진행하므로 bis3D = False가 된다.
    if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP
        && m_SurfaceItem.AlgoPara.nUse3DImageType == SURFACE_3D_USE_ZMAP)
    {
        //3D ZMAP일 경우에 활성화되는 영역
        bIs3D = true;
    }
    else
    {
        //if (strVisionName == arStrModuleName[1] || strVisionName == arStrModuleName[3])
        if (SystemConfig::GetInstance().GetVisionType() == VISIONTYPE_3D_INSP)
        {
            //3D VMAP일 경우에 활성화되는 영역 Image Combine을 비활성화 한다.
            bIs3D = false;
        }
        else
        {
            //2D일 경우에 활성화되는 영역
            bIs3D = false;
            std::vector<CString> vecstrImageCombineParameters(0);
            vecstrImageCombineParameters = m_ImageProcMangePara.ExportImageCombineParaToText(
                strVisionName, strInspectionModuleName, _T("Image Combine"));
            vecstrAlgorithmParameters.insert(vecstrAlgorithmParameters.end(), vecstrImageCombineParameters.begin(),
                vecstrImageCombineParameters.end());
        }
    }

    /********************************************************************* Detail Parameter **********************************************************************/
    strCategory.Format(_T("Surface Detail Parameter"));
    /********************************************************************* Threshold ROI **********************************************************************/
    // Threshold ROI
    strCategory.Format(_T("Threshold ROI"));

    // Thresholding
    strGroup.Format(_T("Thresholding"));

    strParaName.Format(_T("Threshold Type"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
        strParaName, strParaNameAux, (bool)m_SurfaceItem.AlgoPara.GetThresholdNormal()));
    if (m_SurfaceItem.AlgoPara.GetThresholdNormal())
    {
        strParaName.Format(_T("Min Dark Contrast"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.GetThresholdLow()));
        strParaName.Format(_T("Min Bright Contrast"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.GetThresholdHigh()));
    }
    else
    {
        if (bIs3D)
        {
            strParaName.Format(_T("Limit Low Height"));
        }
        else
        {
            strParaName.Format(_T("Min Dark Contrast"));
        }
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.GetThresholdDark(bIs3D)));
        if (bIs3D)
        {
            strParaName.Format(_T("Limit High Height"));
        }
        else
        {
            strParaName.Format(_T("Min Bright Contrast"));
        }
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.GetThresholdBright(bIs3D)));
    }

    // Inspection Area Size Offset
    strGroup.Format(_T("Inspection Area Size Offset"));

    strParaName.Format(_T("Insp ROI Offset Left"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_offsetLeft_um));
    strParaName.Format(_T("Insp ROI Offset Right"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_offsetLeft_um));
    strParaName.Format(_T("Insp ROI Offset Top"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_offsetTop_um));
    strParaName.Format(_T("Insp ROI Offset Bottom"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_offsetBottom_um));

    // Mask Order
    strGroup.Format(_T("Mask Order"));

    strParaName.Format(_T("Mask Order Size"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.AlgoPara.m_maskOrder.GetCount()));

    for (long Idx = 0; Idx < m_SurfaceItem.AlgoPara.m_maskOrder.GetCount(); Idx++)
    {
        auto& Mask = m_SurfaceItem.AlgoPara.m_maskOrder.GetAt(Idx);

        // Offset 류의 데이터를 입력한다. > 해당 데이터에 대해서 형태에 대한 항목이 없음 > 일단 0인지 확인하고 넣는 방식으로 생성
        strParaName.Format(_T("[%s]%s"), (LPCTSTR)arStrValidation[Mask.m_validate], (LPCTSTR)Mask.m_strName);

        // customROI에서 ROI Type을 받아서 Mask의 ROI와 유사하게 나오는지를 확인한다.
        BOOL bIsPointOffset
            = TRUE; // Point offset을 사용하는지 여부를 확인하는 함수 기본값을 True로 두어 불러오기 실패시를 대비한다.
        auto strCustomRoi = m_SurfaceCutemRoi.CustomROI_Search(LPCTSTR(Mask.m_strName));
        if (strCustomRoi) // Custom ROI를 불러온다 만약 불러오지 못했다면 해당 ROI는 Ignore ROI이다.
        {
            bIsPointOffset = (strCustomRoi->nType == 2) ? FALSE : TRUE; // customROI의 type이 image일 경우에만 사용 불가
        }

        if (bIsPointOffset == FALSE)
        {
            strParaNameAux.Format(_T("O-X"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, Mask.m_offset_um.m_x));
            strParaNameAux.Format(_T("O-Y"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, Mask.m_offset_um.m_y));
        }
        else
        {
            for (long nPolygonPointIdx = 0; nPolygonPointIdx < Mask.m_pointOffsets_um.size(); nPolygonPointIdx++)
            {
                strParaNameAux.Format(_T("P%dX"), nPolygonPointIdx);
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, strParaNameAux, Mask.m_pointOffsets_um[nPolygonPointIdx].m_x));
                strParaNameAux.Format(_T("P%dY"), nPolygonPointIdx);
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, strParaNameAux, Mask.m_pointOffsets_um[nPolygonPointIdx].m_y));
            }
        }

        strParaNameAux.Format(_T("Dilate"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, Mask.m_dilateInUm));

        // Ignore Shape 추가 > ROI가 Polygon 형태로 저장되어 있으므로 해당 파라미터들을 보여준다.
        if (Mask.m_makeType == 1)
        {
            auto MaskIndex = m_SurfaceItem.AlgoPara.m_ignore.FindIndex(Mask.m_strName);
            auto IgnoreMask = m_SurfaceItem.AlgoPara.m_ignore.GetAt(MaskIndex);

            strParaNameAux.Format(_T("PolygonType"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, strParaNameAux, IgnoreMask.nPolygonType));

            strParaNameAux.Format(_T("PolygonPointSize"));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, strParaNameAux, (long)IgnoreMask.vecptPolygon_BCU.size()));

            for (long PolygonIdx = 0; PolygonIdx < IgnoreMask.vecptPolygon_BCU.size(); PolygonIdx++)
            {
                strParaNameAux.Format(_T("PolygonInfo[%d] X"), PolygonIdx);
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, strParaNameAux, IgnoreMask.vecptPolygon_BCU[PolygonIdx].m_x));
                strParaNameAux.Format(_T("PolygonInfo[%d] Y"), PolygonIdx);
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, strParaNameAux, IgnoreMask.vecptPolygon_BCU[PolygonIdx].m_y));
            }
        }
    }
    strParaNameAux.Empty();

    /********************************************************************* Criteria **********************************************************************/
    // Criteria
    strCategory.Format(_T("Criteria"));

    // Decision Area
    strGroup.Format(_T("Decision Area"));

    strParaName.Format(_T("Decision Area Num"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, strParaName, strParaNameAux, m_SurfaceItem.m_decisionArea.GetCount()));

    for (long AreaIdx = 0; AreaIdx < m_SurfaceItem.m_decisionArea.GetCount(); AreaIdx++)
    {
        auto DecisionArea = m_SurfaceItem.m_decisionArea.GetAt(AreaIdx);
        strParaName = DecisionArea.strName;

        strParaNameAux.Format(_T("PolygonType"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, DecisionArea.nPolygonType));

        strParaNameAux.Format(_T("PolygonPointSize"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, strParaNameAux, (long)DecisionArea.vecptPolygon_BCU.size()));

        for (long PolygonIdx = 0; PolygonIdx < DecisionArea.vecptPolygon_BCU.size(); PolygonIdx++)
        {
            strParaNameAux.Format(_T("PolygonInfo[%d] X"), PolygonIdx);
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, strParaNameAux, DecisionArea.vecptPolygon_BCU[PolygonIdx].m_x));
            strParaNameAux.Format(_T("PolygonInfo[%d] Y"), PolygonIdx);
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, strParaNameAux, DecisionArea.vecptPolygon_BCU[PolygonIdx].m_y));
        }
    }
    strParaNameAux.Empty();

    // Surface Criteria
    strGroup.Format(_T("Surface Criteria"));

    for (long CriteriaIdx = 0; CriteriaIdx < m_SurfaceItem.vecCriteria.size(); CriteriaIdx++)
    {
        strParaName.Format(_T("%d"), CriteriaIdx);

        strParaNameAux.Format(_T("Use"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, (bool)m_SurfaceItem.vecCriteria[CriteriaIdx].bInsp));
        strParaNameAux.Format(_T("Spec"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_SurfaceItem.vecCriteria[CriteriaIdx].strName));

        for (long DecisionAreaIdx = 0;
            DecisionAreaIdx < m_SurfaceItem.vecCriteria[CriteriaIdx].vecbUseDecisionArea.size(); DecisionAreaIdx++)
        {
            strParaNameAux.Format(
                _T("Decisino Area No %d"), DecisionAreaIdx + 1); // 실제 표시되는 Decision Area는 1부터 시작한다.
            vecstrAlgorithmParameters.push_back(
                CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup, strParaName, strParaNameAux,
                    (bool)m_SurfaceItem.vecCriteria[CriteriaIdx].vecbUseDecisionArea[DecisionAreaIdx]));
        }

        strParaNameAux.Format(_T("Color"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_SurfaceItem.vecCriteria[CriteriaIdx].m_color));
        strParaNameAux.Format(_T("Type"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, m_SurfaceItem.vecCriteria[CriteriaIdx].m_type));

        SItemSpec sCriteriaSpec; // Criteria 스펙이 private로 되어있어 해당 항목을 받아서 사용

        for (long InspIdx = 0; InspIdx < long(VisionSurfaceCriteria_Column::Surface_Criteria_END);
            InspIdx++) // Criteria 스펙이 있는 경우에는 해당 txt를 던져준다. 3D와 2D가 다른 값을 갖는데 해당 항목은 다른 Vision Type 이면 0을 출력하므로 for 루프문으로 처리
        {
            strParaNameAux.Format(_T("%s >"), (LPCTSTR)g_strVisionSurfCriteria_Col[InspIdx]);
            sCriteriaSpec = m_SurfaceItem.vecCriteria[CriteriaIdx].GetSpecMin(VisionSurfaceCriteria_Column(InspIdx));
            if (sCriteriaSpec.GetPassMax() != 0)
            {
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, strParaNameAux, GetCriteriaValue(InspIdx, sCriteriaSpec.GetPassMax())));
            }

            strParaNameAux.Format(_T("%s <"), (LPCTSTR)g_strVisionSurfCriteria_Col[InspIdx]);
            sCriteriaSpec = m_SurfaceItem.vecCriteria[CriteriaIdx].GetSpecMax(VisionSurfaceCriteria_Column(InspIdx));
            if (sCriteriaSpec.GetPassMin() != 0)
            {
                vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                    strGroup, strParaName, strParaNameAux, GetCriteriaValue(InspIdx, sCriteriaSpec.GetPassMin())));
            }
        }
    }
    strParaNameAux.Empty();
    /***************************************************************************************************************************************************/

    // Surface Criteria
    strGroup.Format(_T("Surface Mask"));
    if (m_SurfaceItem.m_bSurfaceMaskMode)
    {
        strParaName.Format(_T("Generate Mask Always"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, strParaNameAux, (bool)m_SurfaceItem.m_bSurfaceMask_GenerateAlways));
    }
    strParaNameAux.Empty();
    /***************************************************************************************************************************************************/

    return vecstrAlgorithmParameters;
}

double VisionInspectionSurfacePara::GetCriteriaValue(long CriteriaIdx,
    double
        i_InputValue) // SDY Criteria에 나가는 value 값과 실제 저장되는 값이 다른 항목이 존재 > 해당 항목 처리를 위한 함수 > 이외는 default로 그대로 return
{
    double o_OutputValue;
    /****************************************************************/

    const double fPixelsToMils = g_fMilToUm;
    const double fPixelsToMils2 = fPixelsToMils * fPixelsToMils;

    /****************************************************************/
    switch (CriteriaIdx)
    {
        case long(VisionSurfaceCriteria_Column::Area):
            o_OutputValue = i_InputValue * fPixelsToMils2;
            break;
        case long(VisionSurfaceCriteria_Column::Dist_X_to_BodyCenter):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::Dist_Y_to_BodyCenter):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::Dist_to_Body):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::Width):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::Length):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::LengthX):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::LengthY):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::TotalLength):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;

        case long(VisionSurfaceCriteria_Column::Thickness):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::Locus):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;

        case long(VisionSurfaceCriteria_Column::t3D_AvgDeltaHeight):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;
        case long(VisionSurfaceCriteria_Column::t3D_KeyDeltaHeight):
            o_OutputValue = i_InputValue * fPixelsToMils;
            break;

        default:
            o_OutputValue = i_InputValue;
            break;
    }
    /****************************************************************/
    return o_OutputValue;
}