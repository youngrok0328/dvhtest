//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "VisionProcessingGlobalMaskMakerPara.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
CVisionProcessingGlobalMaskMakerPara::CVisionProcessingGlobalMaskMakerPara(void)
{
    Init();
}

CVisionProcessingGlobalMaskMakerPara::~CVisionProcessingGlobalMaskMakerPara(void)
{
}

BOOL CVisionProcessingGlobalMaskMakerPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!bSave)
    {
        m_layerList.clear();
    }

    auto& rois_db = db[_T("{7BA2B7BB-7A18-4E78-AAB5-8BD4B5956C10}")];

    long layer_count = long(m_layerList.size());
    if (!rois_db[_T("count")].Link(bSave, layer_count))
        layer_count = 0;

    if (!bSave)
    {
        m_layerList.resize(layer_count);
    }

    for (long layer = 0; layer < layer_count; layer++)
    {
        auto& roi_db = rois_db.GetSubDBFmt(_T("%d"), layer + 1);

        m_layerList[layer].LinkDataBase(bSave, roi_db);
    }

    if (!bSave)
    {
        bool findSurfaceIgnoreMask = false;
        for (long layer = 0; layer < layer_count; layer++)
        {
            if (m_layerList[layer].m_name == GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME)
            {
                findSurfaceIgnoreMask = true;
                break;
            }
        }

        if (!findSurfaceIgnoreMask)
        {
            // Surface Ignore Mask가 없다면 맨마지막에 Layer를 추가해 주자
            long lastIndex = long(m_layerList.size());
            m_layerList.resize(lastIndex + 1);
            m_layerList[lastIndex].m_name = GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME;
        }
    }

    return TRUE;
}

void CVisionProcessingGlobalMaskMakerPara::Init()
{
    m_layerList.clear();
    m_layerList.resize(1);
    m_layerList[0].m_name = GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME;
}

long CVisionProcessingGlobalMaskMakerPara::getLayerCount() const
{
    return long(m_layerList.size());
}

bool CVisionProcessingGlobalMaskMakerPara::addLayer(long index, LPCTSTR name)
{
    CString realName = name;
    realName = realName.Trim();
    realName = realName.MakeUpper();

    if (realName.IsEmpty())
        return false;

    for (auto& layer : m_layerList)
    {
        if (layer.m_name == realName)
            return false;
    }

    // 마지막은 Global Surface Ignore Mask Layer이므로 그전에 추가한다
    index = min(index, long(m_layerList.size() - 1));
    if (index < 0)
    {
        index = long(m_layerList.size() - 1);
    }

    m_layerList.insert(m_layerList.begin() + index, SpecLayer());
    m_layerList[index].m_name = realName;

    realName.Empty();

    return true;
}

bool CVisionProcessingGlobalMaskMakerPara::swapLayer(long index1, long index2)
{
    if (index1 < 0 || index1 >= long(m_layerList.size()))
        return false;
    if (index2 < 0 || index2 >= long(m_layerList.size()))
        return false;

    // Surface Ignore Mask는 Swap 불가
    if (m_layerList[index1].m_name == GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME)
        return false;
    if (m_layerList[index2].m_name == GLOBAL_SURFACE_IGNORE_MASK_LAYERNAME)
        return false;

    auto temp = m_layerList[index1];

    m_layerList[index1] = m_layerList[index2];
    m_layerList[index2] = temp;

    return true;
}

void CVisionProcessingGlobalMaskMakerPara::deleteLayer(long index)
{
    m_layerList.erase(m_layerList.begin() + index);
}

bool CVisionProcessingGlobalMaskMakerPara::setLayerName(long index, LPCTSTR name)
{
    if (index < 0 || index >= long(m_layerList.size()))
        return false;

    CString realName = name;
    realName = realName.Trim();
    realName = realName.MakeUpper();

    if (realName.IsEmpty())
        return false;

    for (long loop = 0; loop < long(m_layerList.size()); loop++)
    {
        if (loop == index)
            continue;
        if (m_layerList[loop].m_name == realName)
            return false;
    }

    m_layerList[index].m_name = realName;

    realName.Empty();

    return true;
}

long CVisionProcessingGlobalMaskMakerPara::getLayerIndex(LPCTSTR name)
{
    for (auto index = 0; index < long(m_layerList.size()); index++)
    {
        auto& layer = m_layerList[index];
        if (layer.m_name == name)
        {
            return index;
        }
    }

    return -1;
}

SpecLayer& CVisionProcessingGlobalMaskMakerPara::getLayer(long index)
{
    return m_layerList[index];
}

std::vector<CString> CVisionProcessingGlobalMaskMakerPara::ExportAlgoParaToText(
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

    //Module 특성 변수//채워야 함.
    for (long nLayer = 0; nLayer < (long)m_layerList.size(); nLayer++)
    {
        strCategory.Format(_T("%s"), (LPCTSTR)(m_layerList[nLayer].m_name));

        auto& LayerSet = m_layerList[nLayer].m_single;

        for (long nLayerSetSize = 0; nLayerSetSize < LayerSet.count(); nLayerSetSize++)
        {
            auto& LayerSetParameter = LayerSet.operator[](nLayerSetSize);

            strGroup.Format(_T("ROI%02d"), nLayerSetSize + 1);
            strParaName.Format(_T("%s"), (LPCTSTR)(LayerSetParameter.m_preparedObjectName));

            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, _T("ROI Type"), (long)LayerSetParameter.getType()));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
                strHeader, strCategory, strGroup, strParaName, _T("Operation"), (long)LayerSetParameter.m_operation));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Prepared Object Name"), LayerSetParameter.m_preparedObjectName));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Prepared Mask Dilate"), LayerSetParameter.m_preparedMaskDilateInUm));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Prepared ROI Expand X"), LayerSetParameter.m_preparedROIExpandX_um));
            vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory,
                strGroup, strParaName, _T("Prepared ROI Expand Y"), LayerSetParameter.m_preparedROIExpandY_um));
        }
    }

    return vecstrAlgorithmParameters;
}