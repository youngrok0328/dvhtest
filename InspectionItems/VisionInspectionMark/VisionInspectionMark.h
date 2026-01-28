#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
#include "../../VisionNeedLibrary/VisionCommon/VisionInspection.h"

//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Image8u.h>
#include <Ipvm/Base/Point32s2.h>
#include <Ipvm/Base/Rect32s.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class VisionInspectionMarkPara;
class VisionInspectionMarkSpec;
class CPackageSpec;
class CPI_Blob;
class VisionAlignResult;
class BlobAttribute;
class CDlgVisionInspectionMark;
class VisionImageLot;
class TeachMergeInfo;
class InspResult;
struct SMarkAlgorithmParameter;

//HDR_6_________________________________ Header body
//
#define NUM_OF_LOCATOR 2

class __VISION_INSP_MARK_INSPECTION_CLASS__ VisionInspectionMark : public VisionInspection
{
public:
    VisionInspectionMark(
        LPCTSTR moduleGuid, LPCTSTR moduleName, VisionUnitAgent& visionUnit, CPackageSpec& packageSpec);

    virtual ~VisionInspectionMark(void);

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
    std::vector<CString> ExportROIRecipeToText(const CString strHeader, const CString strCategory); //kircheis_TxtRecipe
    void SetPoint(Ipvm::Point32s2 ptDispoint);

    VisionInspectionMarkSpec* m_VisionTempSpec; // 표면적인 검사Spec(껍데기)

    VisionInspectionMarkPara* m_VisionPara; // Detail Setup용
    CDlgVisionInspectionMark* m_pVisionInspDlg;
    VisionInspectionMarkSpec* m_VisionSpec[NUM_OF_MARKMULTI];
    void SetSwitchMarkParameterView(long i_nTabCurAccessMode);

private:
    void GetDebugInfo(const bool detailSetupMode, const Ipvm::Point32r2& imageRotateCenter, float imageAngle_deg);

public:
    CPI_Blob* m_pBlob;

public:
    void InitMemory(BOOL bJobChange, long& nTeachSlot);
    BOOL CheckROI(Ipvm::Rect32s rtROI, long i_width, long i_height);
    void ResetInspItem();

    BOOL DoTeach(const bool detailSetupMode);
    BOOL DoTeach(const bool detailSetupMode, VisionInspectionMarkSpec* i_pMarkSpec, Ipvm::Rect32s rtPane,
        const Ipvm::Rect32s& teachROI, const Ipvm::Image8u& rotateImage, Ipvm::Image8u& grayProcImage,
        Ipvm::Image8u& thresImage, long nReTeachThreshold);
    BOOL CheckMarkAngle(long nLineNumber, float& fAngleChek);

    /// Align정보 얻어오기.
    VisionAlignResult* GetAlignInfo();
    /// 옵션에 따라서 Meadian, Lowpass돌림.
    BOOL ImplProcessing(const Ipvm::Image8u& i_grayImage, const Ipvm::Rect32s& i_rtProcessROI, long i_nPreProcMode,
        Ipvm::Image8u& o_image);
    /// 이진영상에서 Blob을 구함.
    BOOL GetCharBlob(const Ipvm::Image8u& i_binImage, Ipvm::Rect32s i_rtProcessROI,
        SMarkAlgorithmParameter* i_psMarkParam, bool i_roiMerge, std::vector<TeachMergeInfo>& io_mergeInfos,
        const std::vector<Ipvm::Rect32s>& i_vecrtIgnoreNumROI, const std::vector<Ipvm::Rect32s>& i_vecrtIgnoreROI,
        Ipvm::BlobInfo* o_psMarkInfo, BlobAttribute* o_blobAttributes, Ipvm::Image32s& o_imageLabel,
        int32_t& o_nCharBlobNum);
    /// 이진영상에서 Mark Slite 처리 함수
    BOOL GetProfile(const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& io_binImage, Ipvm::Rect32s rtChar,
        std::vector<float>& vecfFilteredProf, BOOL bVirtical);
    BOOL LineFiltering(std::vector<float>& vecfLineProfile);
    BOOL SetSpliteChar(const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& io_binImage, Ipvm::Rect32s rtChar);
    BOOL GetAvgRectWidthHeight(std::vector<Ipvm::Rect32s>& i_vecrtROI, float& o_fAvgWidth, float& o_fAvgHeight);
    BOOL SetSpliteChar(const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& io_binImage,
        std::vector<Ipvm::Rect32s>& i_vecrtSpecSpliteROI, Ipvm::BlobInfo* i_pCharBlobInfo);
    //// Gray 영상에서 영상처리 후, 이진화, Blob까지 해서 Char 정보를 구함.
    BOOL GetCharBlobInfoForTeach(VisionInspectionMarkPara* i_pVisionPara, const Ipvm::Image8u& i_image,
        Ipvm::Image8u& i_grayProcessImage, Ipvm::Image8u& i_binImage, Ipvm::Image32s& imageLabel,
        Ipvm::Rect32s i_rtProcessROI, Ipvm::BlobInfo* i_pCharBlobInfo, BlobAttribute* i_charAttribute,
        int32_t& i_nCharBlobNum, long nReTeachThreshold, BOOL bSplite);
    //// Mark Line을 구하자
    BOOL GetMarkLineForTeach(SMarkAlgorithmParameter* i_pMarkAlgoPara, Ipvm::BlobInfo* i_pCharBlobInfo,
        BlobAttribute* i_charAttribute, long i_nCharBlobNum, short& i_nLineNum);
    //// Mark중심으로부터의 각각의 Char(LeftTop) offset을 구하자.
    BOOL GetCharOffsetFromMarkGroupCenter(
        Ipvm::BlobInfo* i_pCharBlobInfo, short i_nCharBlobNum, std::vector<Ipvm::Point32r2>& o_vecfptTeachCharOffset);
    //// Char Blob만 따로 관리하자.
    BOOL MakeTeachingCharImage(Ipvm::BlobInfo* i_pCharBlobInfo, short i_nCharBlobNum, Ipvm::Rect32s* o_prtCharSpecROI,
        long* o_plCharArea, Ipvm::Rect32s* o_prtCharPosition);
    //// Locate를 구하고, 다른 정보를 취합하자.
    BOOL MakeLocatorForTeach(VisionInspectionMarkPara* i_pVisionPara, const Ipvm::Image8u& i_binImage,
        Ipvm::BlobInfo* i_pCharBlobInfo, BlobAttribute* i_charAttribute, short i_nCharBlobNum, int* i_pnLocID,
        Ipvm::Rect32s* i_prtLocaterSearchROI);
    //// Binary 영상으로 Spec이미지 만들자.
    BOOL MakeMarkSpecImageFromTeachPane(const Ipvm::Image8u& i_binImage, Ipvm::Image8u& o_specImage);
    /// Char Blob및 그외 데이터 저장해 둔다.
    //	void uOnChangeMarkSpec(VisionInspectionMarkSpec* i_pVisionSpec, VisionInspectionMarkPara* i_pVisionPara);

    BOOL UpdateResult();
    BOOL UpdateEachInspResult(CString strInspName, long nCharIndex, float fCurrValue, long& nCurrResult);

    BOOL DoCharNumberCheckTeach(
        Ipvm::Image32s& io_label, Ipvm::BlobInfo* psCharInfo, int32_t& nCharBlobNum, float fMergeGap);
    BOOL MergeMarkForTeach(
        Ipvm::BlobInfo* psCharInfo, int32_t& nCharBlobNum, Ipvm::Image32s& io_label, float fBlobDist);
    BOOL DoAreaThreshold(Ipvm::BlobInfo* psCharInfo, int32_t& nCharBlobNum);
    void SortingCharsY(Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute, long nCharBlobNum, BOOL bTopToBottom);
    BOOL CalTextLineNumber(Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute, long nCharBlobNum,
        short& nLineNumber, std::vector<long>& vCharNum, std::vector<long>& vLineStartCharID, BOOL bToptoBottom);
    void SortingCharsX(Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute, long nCharBlobNum, BOOL bLeftToRight);
    void SetMarkROI(long markIndex, const Ipvm::Rect32s& roi, const float fMarkAngle);

    BOOL CalContrast(const Ipvm::Image8u& i_imageGray, const Ipvm::Image8u& i_imageThreshold, Ipvm::Rect32s rtROI,
        int& nContrast, int nMargin, BOOL bBlackImage);
    BOOL GetDefaultRefID_Auto(const Ipvm::Image8u& i_binImage, Ipvm::BlobInfo* psCharInfo, BlobAttribute* charAttribute,
        long nNum, int* pnRefID);
    BOOL GetDefaultRefID_Manual(const Ipvm::Image8u& i_binImage, Ipvm::BlobInfo* psCharInfo,
        BlobAttribute* charAttribute, long nNum, Ipvm::Rect32s* prtLocaterSearchROI, int* pnRefID);
    BOOL CharMatching(const Ipvm::Image8u& i_theshImage, Ipvm::Rect32s rtROIRef, Ipvm::Rect32s rtROI, int& nMisMatch);
    BOOL GetLocaterSearchBox(Ipvm::Rect32s* o_prtLocaterSearch, Ipvm::Point32r2& o_ptLocaterCenter);

    BOOL GetMatchScore(const Ipvm::Image8u& refImage, const Ipvm::Image8u& sourceImage, double& o_pdScore);
    BOOL FindLocater(const Ipvm::Image8u& i_refImage, const Ipvm::Image8u& i_binImage, Ipvm::Point32s2* o_pptLocater,
        BOOL bTeaching);
    BOOL FindLocater(long i_nNumOfLocator, const Ipvm::Image8u& i_refImage, const Ipvm::Image8u& i_binImage,
        long i_nImageSampleRate, Ipvm::Rect32s* i_prtRefROI, Ipvm::Rect32s* i_prtSerachROI,
        Ipvm::Point32s2* o_pptLocater);
    BOOL FindLocaterReformed(long i_nNumOfLocator, const Ipvm::Image8u& i_refImage, const Ipvm::Image8u& i_binImage,
        long i_nImageSampleRate, Ipvm::Rect32s* i_prtRefROI, Ipvm::Rect32s* i_prtSerachROI,
        Ipvm::Point32s2* o_pptLocater); //kircheis_MarkLoc

    void Get2PntLocaterLineAngle(const Ipvm::Image8u& i_binImage, Ipvm::Point32s2* pptLocator,
        Ipvm::Rect32s* prtLocatorROI, float& fAngle, BOOL bTeach); //kircheis_MarkLoc

    BOOL DoMarkInsp(const Ipvm::Image8u& image, Ipvm::BlobInfo* psMarkInfo, const Ipvm::Rect32s& inspROI);
    BOOL DoMarkCountInsp(
        const Ipvm::Image8u& i_imgInsp, Ipvm::BlobInfo* i_psMarkInfo, const Ipvm::Rect32s& i_rtTeachROI);
    BOOL GetMarkInspImage(
        const Ipvm::Image8u& i_imgInsp, const Ipvm::Rect32s& i_rtTeachROI, Ipvm::Image8u& o_imgInspMarkBlob);
    BOOL GetIgnoreImageByDebugInfo(
        const Ipvm::Image8u& i_imgOrigin, Ipvm::Rect32s i_rtTeachROI, Ipvm::Image8u& o_imgIgnore);
    BOOL IgnoreImageDilate(
        const Ipvm::Image8u i_Image, Ipvm::Rect32s i_rtTeachROI, long i_nDilateCount, Ipvm::Image8u& o_DilateImage);
    BOOL GetMarkBlobInfo(const Ipvm::Image8u& i_MarkBlobImage, const Ipvm::Rect32s& i_rtTeachROI,
        Ipvm::BlobInfo* i_psBlobInfo, Ipvm::Image32s& o_imageLabel, std::vector<Ipvm::BlobInfo>& o_vecCurBlobInfo);
    std::vector<Ipvm::Rect32s> m_vecrtDebugROI; //mc_잠깐확인용
    float GetMarkContrastForMarkCount(const Ipvm::Image8u& i_imageGray, const Ipvm::Image8u& i_imageThreshold,
        const Ipvm::Image32s& i_ImageLabel, Ipvm::Rect32s i_rtROI, long i_nTargetLabel);
    BOOL MarkPositionInsp(Ipvm::Point32r2 i_ptDiffLocatorPos, const Ipvm::Rect32s& inspROI);

    BOOL LoopCharInsp(const Ipvm::Image8u& i_grayImage, Ipvm::Image8u& i_theshImage, Ipvm::Point32r2 i_ptLocatorCenter,
        int nRefThreshold, float& fMarkAngle);
    BOOL CharInsp(const Ipvm::Image8u& i_grayImage, const float fMarkAngle, Ipvm::Image8u& i_theshImage, BOOL bReinsp,
        BYTE byInitThreshold);
    BOOL CharMatching_Blob(const Ipvm::Image8u& i_theshImage, const Ipvm::Rect32s& rtROI, const float fMarkAngle,
        int index, int& nUnderPrintNum, int& nOverPrintNum, int& nMatchingRate, Ipvm::Image8u& refBuffer,
        Ipvm::Image8u& binBuffer, Ipvm::Image8u& underBuffer, Ipvm::Image8u& overBuffer);
    BOOL CharMatching_Blob_NormalSize(const Ipvm::Image8u& i_theshImage, const Ipvm::Rect32s& rtROI,
        const float fMarkAngle, int index, int& nUnderPrintNum, int& nOverPrintNum, int& nMatchingRate,
        Ipvm::Image8u& refBuffer, Ipvm::Image8u& binBuffer, Ipvm::Image8u& underBuffer, Ipvm::Image8u& overBuffer);
    long FindCharAndMatching(const Ipvm::Image8u& i_theshImage, const Ipvm::Image8u& i_refImage, Ipvm::Rect32s rtROIRef,
        Ipvm::Rect32s rtSearchROI, float& o_fMatchSocre);

    BOOL MarkAngleInsp(float fCurrentAngle, const Ipvm::Rect32s& inspROI);

    BOOL CharContrastInsp(const Ipvm::Image8u& i_imageGray, const Ipvm::Image8u& i_imageThreshold,
        std::vector<Ipvm::Rect32s>& vecrtCharROI);
    BOOL CharContrastInsp(
        const Ipvm::Image8u& i_imageGray, const Ipvm::Image8u& i_imageThreshold, Ipvm::Rect32s rtROI, int index);
    BOOL MarkBlobSizebyUnderOverImage();
    void DisplayResultOverlay(BOOL bTeach);

    BOOL GetCharSearchROI(long i_nCharNum, Ipvm::Point32r2 i_ptLocCenter,
        const std::vector<Ipvm::Point32r2>& i_ptCharPosFromLC, std::vector<Ipvm::Rect32s>& o_vecroiCharROI);

    BOOL ResultUpdate(CString strInspName, long& nTotalResult, BOOL& bUse);
    BOOL GetTotalROIFromSmallROIs(std::vector<Ipvm::Rect32s>& i_vecrtROI, Ipvm::Rect32s& o_rtTotalROI);

    //OCR 관련 함수
    BOOL SimilerStringCompare(BOOL bSimilerComp, CString strRef, CString strRes);
    CString RealignFailPartID(CString strPartID);
    BOOL SimilerStringFind(BOOL bSimilerComp, CString strRef, CString strRes);

public:
    InspResult* m_result;
    std::vector<Ipvm::Rect32s> m_vecrtBlobROI;

    BOOL m_bMarkSpliteOptionTeach;

    Ipvm::BlobInfo* m_psBlobInfo;
    BlobAttribute* m_blobAttributes;

    long m_nTotalResult;

    VisionAlignResult* m_bodyAlignResult;

    Ipvm::Image8u* m_oriViewer;
    Ipvm::Image8u* m_binViewer;
    Ipvm::Image8u* m_overViewer;
    Ipvm::Image8u* m_underViewer;
    Ipvm::Image8u* m_blobSizeUnderViewer;
    Ipvm::Image8u* m_blobSizeOverViewer;

    // Save 필요없는 변수들...
    Ipvm::Point32s2 m_pptLocater[NUM_OF_LOCATOR];
    Ipvm::Rect32s m_prtLocaterSearchROI[NUM_OF_LOCATOR];
    Ipvm::Rect32s m_rtAllChar;
    float m_fCalcTime;

    BOOL m_bTeach;

    Ipvm::Image8u m_SimpleAlgorithmImage; //MarkSimple Algorithm에서 추출한 Blob의 Image를 Surface로 연동할 놈

    BOOL m_bAutoTeachMode; //kircheis_AutoTeach
    //이 변수는 Mark Teaching 시 Auto Teach인지를 확인하는 변수로 각 ROI의 Offset을 계산할건지 적용할 건지와
    //Teach 시 UI 관련 작업 진행 여부를 확인하는 변수 이므로 사용상의 주의를 요함
    void CalcAutoTeachRoiOffset(); //kircheis_AutoTeach

    //{{ //kircheis_for_ITEK
    void ResetIgnoreBoxStatus();
    //}}

    BOOL IsNeedMarkTeach();

    void ModifyMemory();
};
