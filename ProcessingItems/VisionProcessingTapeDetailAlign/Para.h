#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "ParaAlignment.hpp"
#include "ParaDeviceAlign.h"
#include "ParaPocketAlign.h"
#include "ParaSprocketHoleAlign.h"

//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_DataBase/PackageSpec.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <map>

//HDR_5_________________________________ Forward declarations
class CiDataBase;
class ImageProcPara;
class ParaDB;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
class Para
{
public:
    Para(const Para& object) = delete;
    Para(VisionProcessing& parent);
    ~Para();

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Init();
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    ParaAlignment<ParaSprocketHoleAlign> m_sprocketHoleAlign;
    ParaAlignment<ParaPocketAlign> m_pocketAlign;
    ParaAlignment<ParaDeviceAlign> m_deviceAlign;
};