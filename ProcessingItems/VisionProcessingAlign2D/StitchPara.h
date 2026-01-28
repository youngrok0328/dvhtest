#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
#include "StitchPara_BasedOnBodySize.h"
#include "StitchPara_BasedOnMatching.h"

//HDR_2_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h" // Stitch Section Image Combine
#include "../../VisionNeedLibrary/VisionCommon/VisionInspFrameIndex.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class ImageProcPara;
class VisionProcessing;

//HDR_6_________________________________ Header body
//
enum enumAlgorithmType : int
{
    Matching,
    BodySize
};

class StitchPara
{
public:
    StitchPara(VisionProcessing& parent);
    ~StitchPara(void);

    //------------------------------------------------------------------
    // 저장되지 않고 Detail Setup을 위해 존재하는 값들
    //------------------------------------------------------------------
    bool m_skipEdgeAlign;

    //------------------------------------------------------------------

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    void Init();
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    VisionInspFrameIndex m_calcFrameIndex;
    ImageProcPara m_stitchImageProcManagePara; // Stitch Section Image Combine
    enumAlgorithmType m_type;
    long m_boundaryInterpolation;

    long m_saveTemplateDebugImage;

    StitchPara_BasedOnMatching m_paraBasedOnMatching;
    StitchPara_BasedOnBodySize m_paraBasedOnBodySize;

    float m_fMoveDistanceXbetweenFOV;
    float m_fMoveDistanceYbetweenFOV;

    long m_nStitchCountX;
    long m_nStitchCountY;
};
