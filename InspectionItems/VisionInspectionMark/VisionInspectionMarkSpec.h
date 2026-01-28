#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/iDataType.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32r2.h>
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class VisionInspectionMark;

//HDR_6_________________________________ Header body
//
class VisionInspectionMarkSpec
{
public:
    VisionInspectionMarkSpec(void);
    ~VisionInspectionMarkSpec(void);

    void Init();
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void AllocateMemory(int i_nCharNum);
    VisionInspectionMarkSpec& operator=(const VisionInspectionMarkSpec& Src);
    std::vector<CString> ExportAlgoMarkSpecToText(
        CString strHeader, CString strCategory, CString strGroup); //kircheis_TxtRecipe

    Ipvm::Image8u m_specImage;
    Ipvm::Rect32r m_rtMarkTeachROI_BCU; // kircheis_AutoMark
    Ipvm::Point32s2 m_ptTeachRoiLtFromPackCenter; // kircheis_AutoTeach
    BOOL m_bTeachDone;
    BOOL m_bSaveTeachImage;
    long m_nMarkThreshold;

    long m_nCharNum;
    long m_nCharLineNum;
    float m_fLocAngleForBody;
    Ipvm::Point32r2 m_ptLocCenterforScreen;
    long m_nCharMinArea; //kircheis_MarkAngle

    int m_pnLocID[NUM_OF_LOCATOR];
    Ipvm::Rect32s m_prtLocaterTeachingBox[NUM_OF_LOCATOR];
    Ipvm::Rect32s m_rtLocatorTargetROI[NUM_OF_LOCATOR];
    Ipvm::Point32s2 m_pptLocaterTargetRoiLtFromPackCenter[NUM_OF_LOCATOR]; //kircheis_AutoTeach

    Ipvm::Rect32s* m_plTeachCharROI;
    long* m_plTeachCharArea;
    std::vector<Ipvm::Point32r2> m_teachCharOffset;

    std::vector<long> m_veclTeachCharNumPerLine;
    std::vector<long> m_veclTeachStartCharIDPerLine;
};
