#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
#include "Para_MaskOrder.h"
#include "Para_PolygonRects.h"

//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
#include <Ipvm/Base/Rect32r.h>

//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;
class CSurfaceTeachingImage;

//HDR_6_________________________________ Header body
//
class __DPI_SURFACE_API__ CSurfaceAlgoPara
{
public:
    // Interface
    CSurfaceAlgoPara();
    CSurfaceAlgoPara(const CSurfaceAlgoPara& object);
    ~CSurfaceAlgoPara();

    void Init();

    enum enumAlignOption
    {
        enumAlign_Grid,
        enumAlign_Body,
        enumAlign_Corner,
        enumAlign_MarkLocator,
    };
    enum enum3DUseImage
    {
        enumUse_Zmap,
        enumUse_Vmap,
    };

    int nAlignOption;
    int nUse3DImageType; // 2019.10.31 3D Surface VMAP

    double fMinAreaBrightContrast; // unit : um^2
    double fMinAreaDarkContrast; // unit : um^2
    double m_mergeDistance; // unit : um
    BOOL m_mergeOnlyTheSameColor; // 같은 색상끼리만 Merge 하기

    double fLocalContrastAreaSpan; // unit : um

    //}}
    // 이 값만큼 Object 의 ROI 를 상하좌우로 확장한다.
    float m_offsetLeft_um;
    float m_offsetRight_um;
    float m_offsetTop_um;
    float m_offsetBottom_um;

    //=======================================================

    // Blob Filtering
    double fMinimumBrightBlobArea_umSqure; // unit : um
    double fMininumDarkBlobArea_umSqure; // unit : um

    long nMaxBrightBlobNum_New;
    long nMaxDarkBlobNum_New;
    //}}

    // 2nd Inspection
    BOOL m_use2ndInspection; // 2nd 검사 사용 유무 (딥러닝)
    CString m_str2ndInspCode; // 딥러닝 모델과 매칭을 위한 코드
    BOOL m_useOnlyDLInsp; // 비전 검사를 하지 않고 딥러닝 검사만...

    //{{ Contrast & ROI : 편집 인터페이스 2

    double m_dbackground_window_size_x; //Unit : um
    double m_dbackground_window_size_y; //Unit : um

    Ipvm::Rect32r m_rtROI_BCU;
    Para_PolygonRects m_ignore;
    Para_MaskOrder m_maskOrder;

    float& GetThresholdDark(bool is3D);
    float& GetThresholdBright(bool is3D);
    BOOL& GetThresholdNormal();
    const float& GetThresholdDark(bool is3D) const;
    const float& GetThresholdBright(bool is3D) const;
    const BOOL& GetThresholdNormal() const;
    long& GetThresholdLow();
    long& GetThresholdHigh();
    const long& GetThresholdLow() const;
    const long& GetThresholdHigh() const;

public:
    BOOL LinkDataBase(BOOL bSave, CiDataBase& db);
    CSurfaceAlgoPara& operator=(const CSurfaceAlgoPara& object);

private:
    float m_min_dark_contrast;
    float m_min_bright_contrast;
    long m_low_Threshold;
    long m_high_Threshold;
    float m_min_dark_contrast_3D;
    float m_min_bright_contrast_3D;
    BOOL m_bisNormalThreshold;
};