#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CPackageSpec;
class CPI_EdgeDetect;
class CPI_Blob;
class VisionAlignResult;
class VisionInspection2DMatrixPara;
class VisionImageLot;
class CDlgVisionInspection2DMatrix;

//HDR_6_________________________________ Header body
//
class __VISION_INSP_2DMATRIX_INSPECTION_CLASS__ VisionInspection2DMatrix : public VisionInspection
{
public:
    VisionInspection2DMatrix(VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);
    virtual ~VisionInspection2DMatrix(void);

public: // VisionProcessing virtual functions
    virtual BOOL DoInspection(const bool detailSetupMode,
        const enSideVisionModule i_ModuleStatus = enSideVisionModule::SIDE_VISIONMODULE_FRONT) override;
    virtual BOOL OnInspection() override;
    virtual BOOL LinkDataBase(BOOL bSave, CiDataBase& db) override;
    virtual void ResetSpecAndPara() override;
    virtual void ResetResult() override;
    virtual void GetOverlayResult(VisionInspectionOverlayResult* overlayResult, const bool detailSetupMode) override;
    virtual void AppendTextResult(CString& textResult) override;
    virtual long ShowDlg(const ProcessingDlgInfo& procDlgInfo) override;
    virtual void CloseDlg() override;
    virtual std::vector<CString> ExportRecipeToText() override; //kircheis_TxtRecipe

public:
    VisionInspection2DMatrixPara* m_VisionPara;
    CDlgVisionInspection2DMatrix* m_pVisionInspDlg;

    float m_fCalcTime;

public:
    VisionAlignResult* m_sEdgeAlignResult;

    /////////////////////////////////////////////////
    BOOL DoInsp2DMatrix(const bool detailSetupMode, std::vector<Ipvm::Image8u> vecImage, Ipvm::Rect32r frtBody);

    BOOL Reading2DID(
        Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtROI, CString& o_strBarcode, Ipvm::Rect32s& o_rtBarcodeROI);
    BOOL Reading2DIDbyEuresys(
        Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtROI, CString& o_strBarcode, Ipvm::Rect32s& o_rtBarcodeROI);
    BOOL Reading2DIDbyClient(
        Ipvm::Image8u& image, const Ipvm::Rect32s& i_rtROI, CString& o_strBarcode, Ipvm::Rect32s& o_rtBarcodeROI);
    Ipvm::Rect32r GetBodyRect();
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void AbsoluteRectToImageRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32s& rtROI);
    void ImageRectToAbsoluteRectByBodyCenter(Ipvm::Rect32r frtBodyRect, Ipvm::Rect32s& rtROI);

    //{{//kircheis_MED5_13
    std::vector<Ipvm::Rect32s> m_vecrtROI; //DoInsp2DMatrix()에서 읽은 문자열의 ROI를 각각 저장하기 위한 변수
    std::vector<CString> m_vecstr2DID; //DoInsp2DMatrix()에서 읽은 문자열을 각 ROI별로 저장하기 위한 변수
    BOOL
        m_bIsTurnOn2DMatrix; //DoInspMatchString()은 DoInsp2DMatrix()가 켜있어야 구동 가능하다. On/Off 여부를 확인하기 위한 변수.. 따로 확인하기 귀찮다.
    BOOL DoInspMatchString(const bool detailSetupMode);
    CString Get2DIDText(const long n2DIDType); //2DID Type에 맞는 문자열 리턴 (입력 Param : enum MATCH_2DID_TYPE 참조)
    CString Get2DIDName(
        const long n2DIDType); //2DID Type의 이름을 문자열로 리턴 (입력 Param : enum MATCH_2DID_TYPE 참조)
    //}}

    //{{//kircheis_NGRV Bypass
    Ipvm::Rect32r GetPaneRect();
    BOOL m_bUseBypassMode;
    BOOL Is2DMarixReading();

    BOOL GetMajorRoiCenter(Ipvm::Point32s2& ptCenter);
    //}}

    BOOL IsTurnOn2DMatrix(); //kircheis_20230126

private: //mc_23.05.24 2DID Cropping image Save [Debug용도]
    struct s2DID_Cropping_image_Save_Info
    {
        std::vector<Ipvm::Rect32s> m_vecrt2DID_ROI;
        std::vector<CString> m_vecstr_Major_Result;
        std::vector<CString> m_vecstr_Sub_Result;
        std::vector<CString> m_vecstr_Baby_Result;

        void Init()
        {
            m_vecrt2DID_ROI.clear();
            m_vecstr_Major_Result.clear();
            m_vecstr_Sub_Result.clear();
            m_vecstr_Baby_Result.clear();
        }

        void SetCropping_image_Save_info(const std::vector<Ipvm::Rect32s> i_strrt2DID_ROI,
            const std::vector<CString> i_vecstr_Marjor, const std::vector<CString> i_vecstr_Sub,
            const std::vector<CString> i_vecstr_Baby)
        {
            Init();

            m_vecrt2DID_ROI.resize(i_strrt2DID_ROI.size());
            m_vecstr_Major_Result.resize(i_vecstr_Marjor.size());
            m_vecstr_Sub_Result.resize(i_vecstr_Sub.size());
            m_vecstr_Baby_Result.resize(i_vecstr_Baby.size());

            std::copy(i_strrt2DID_ROI.begin(), i_strrt2DID_ROI.end(), m_vecrt2DID_ROI.begin());
            std::copy(i_vecstr_Marjor.begin(), i_vecstr_Marjor.end(), m_vecstr_Major_Result.begin());
            std::copy(i_vecstr_Sub.begin(), i_vecstr_Sub.end(), m_vecstr_Sub_Result.begin());
            std::copy(i_vecstr_Baby.begin(), i_vecstr_Baby.end(), m_vecstr_Baby_Result.begin());
        }
    };

    struct s2DID_Using_Frame_info
    {
        long m_nFrameidx;
        Ipvm::Image8u m_image;

        void Init()
        {
            m_nFrameidx = 0;
            m_image.FillZero();
        }

        void SetFrameInfo(const long i_nFrameidx, const Ipvm::Image8u i_image)
        {
            Init();

            m_nFrameidx = i_nFrameidx;
            m_image = i_image;
        }
    };

    s2DID_Cropping_image_Save_Info m_s2DID_Cropping_image_Save_Info;
    void Save_2DID_Cropping_image_Debug(const e2DIDCroppingimageSaveOption i_eOption,
        const std::vector<s2DID_Using_Frame_info> i_vecFrameinfo, const s2DID_Cropping_image_Save_Info i_sSaveInfo);
    void DeleteOldDirectory(const CString i_strSearchDirectoryPath); //7일이 지나면 무조건 삭제한다
    bool isRemoveOldDirectory(const CTime i_CurCreateDirecotryDate);
    bool isRemoveOldDirectoryinFolder(const CString i_strSearchDirectoryPath, std::vector<CString>& o_vecFolderPath);
    bool RemoveOldDirectoryinFolderinFiles(const CString i_strSearchDirectoryPath);
};
