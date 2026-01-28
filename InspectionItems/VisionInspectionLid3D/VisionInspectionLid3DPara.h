#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/ImageProcPara.h"

//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
#include <vector>

//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
struct SLid3DROI_Info
{ //단위는 그때그때 다르게 사용한다. 파일을 읽은 넘을 저장하게나 Job으로 관리할 때 단위는 um. Job Open후 실제 계산에서 사용할 때는 pxl로 쓰자.
    CString strID;

    float fOffsetX;
    float fOffsetY;
    float fWidth;
    float fLength;
};

struct sLid3DReadDataInfo
{
    CString strID;

    float fOffsetX;
    float fOffsetY;
    float fWidth;
    float fLength;
    long nGroupID;
};

struct SLid3DGroupInfo
{
    CString strID;

    std::vector<SLid3DROI_Info> vecLidROI_Info;
};

class VisionInspectionLid3DPara
{
public:
    VisionInspectionLid3DPara(VisionProcessing& parent);
    ~VisionInspectionLid3DPara(void);

    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    std::vector<CString> ExportAlgoParaToText(
        CString strVisionName, CString strInspectionModuleName); //kircheis_TxtRecipe

    ImageProcPara m_ImageProcManagePara;

    long m_nEdgeDirection;
    long m_nSearchDirection;
    long m_nEdgeDetectMode;
    long m_nFirstEdgeValue;
    long m_nEdgeSearchWidthRatio;
    long m_nEdgeSearchLength_um;
    long m_nEdgeDetectGap_um;

    long m_nSubstrateROI_size_um;
    long m_nSubstrateROI_OffsetX_um;
    long m_nSubstrateROI_OffsetY_um;

    long m_nLidTiltROI_size_um;
    long m_nLidTiltROI_OffsetX_um;
    long m_nLidTiltROI_OffsetY_um;

    std::vector<SLid3DGroupInfo> m_vecsLid3DGroupInfo_um; //csv로 읽거나 Job에 저장할 땐 얘를 써라

    static void ClearLid3DGroupInfo(std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo);
    static void SetLid3DGroupInfo(long nGroupNum, std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo);
    static void SetLid3DROI_Info(long nGroupID, long nROI_Num, std::vector<SLid3DGroupInfo>& vecsLid3DGroupInfo);
    void ReadLid3DGroupInfoFromCSV(CString strFileFullPath);
    void GetGroupIDNum(std::vector<sLid3DReadDataInfo> i_vecsLid3DReadDataInfo, std::vector<long>& o_vecnGroupID);
    void SetGroupData_um(std::vector<sLid3DReadDataInfo> i_vecsLid3DReadDataInfo, std::vector<long> i_vecnGroupID,
        std::vector<SLid3DGroupInfo>& o_vecsLid3DGroupInfo_um);
    void GetROIDataNumAndValidIndex(std::vector<sLid3DReadDataInfo> i_vecsLid3DReadDataInfo, long i_GroupID,
        long& o_nROIDataNum, std::vector<long>& o_vecVaildindex);
    void WriteLid3DGroupInfoToCSV(CString strFileFullPath);
};
