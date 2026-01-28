#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "SpecLayer.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
class CVisionProcessingGlobalMaskMakerPara
{
public:
    CVisionProcessingGlobalMaskMakerPara(void);
    ~CVisionProcessingGlobalMaskMakerPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);

    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    void Init();
    long getLayerCount() const;
    bool addLayer(long index, LPCTSTR name);
    bool swapLayer(long index1, long index2);
    void deleteLayer(long index);
    bool setLayerName(long index, LPCTSTR name);
    long getLayerIndex(LPCTSTR name);

    SpecLayer& getLayer(long index);

private:
    std::vector<SpecLayer> m_layerList;
};
