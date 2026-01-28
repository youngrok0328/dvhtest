//CPP_0_________________________________ Precompiled header
#include "stdAfx.h"

//CPP_1_________________________________ Main header
#include "VisionInspectionBgaBottom2DPara.h"

//CPP_2_________________________________ This project's headers
#include "VisionInspectionBgaBottom2D.h"

//CPP_3_________________________________ Other projects' headers
#include "../../DefineModules/dA_Base/TxtRecipeParameter.h" //kircheis_TxtRecipe
#include "../../DefineModules/dA_Base/semiinfo.h"
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionInspectionSpec.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionProcessing.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
VisionInspectionBgaBottom2DPara::VisionInspectionBgaBottom2DPara(VisionProcessing& parent)
    : m_obliqueImageIndex(parent, FALSE)
    , m_coaxialImageIndex(parent, FALSE)
    , m_qualityObliqueImageIndex(parent, FALSE)
    , m_qualityCoaxialImageIndex(parent, FALSE)
    , m_parameters(1)
{
    Init();
}

VisionInspectionBgaBottom2DPara::~VisionInspectionBgaBottom2DPara(void)
{
}

BallLevelParameter::BallLevelParameter()
{
    Init();
}

void BallLevelParameter::Init()
{
    // Ball Width
    m_nBallWidthMethod = enumGetPittingValue; //kircheis_20160602
    m_widthRatio = 0.1f;
    m_nBallEdgeCountMode = enumBallEdgeFastMode; //kircheis_BallFlux
    m_ballWidthRoughSearchLengthRatio = 0.75f;
    m_ballWidthDetailSearchLengthRatio = 0.7f; //kircheis_BallFlux
    m_ballWidthSearchIgnoreLengthRatio = 0.3f; //kk
    m_nBallEdgeReSearchMode = enumBallEdgeReSearchNotUse; //kircheis_BallFlux
    m_ballReSearchLengthRatioInner = 0.2f; //kircheis_BallFlux
    m_ballReSearchLengthRatioOuter = 0.3f; //kircheis_BallFlux
    m_ballReSearchNoiseFilterThresholdRatio = 0.12f; //kircheis_BallFluxModify
    m_nRoughAlignMethod = enumBallRoughAlignMethod_DefaultCircleFitting; //kircheis_LKF

    // Ball Quality
    m_qualityThresholdValue = 128;
    m_qualityCheckMethod = VisionInspectionBgaBottom2DPara::BallQualityCheckMethod::enumNormalMethod;
    m_qualityCheckOuterVoid = 1.2f;
    m_qualityCheckInnerVoid = 0.f; // (Ball Quality Check)
    m_qualityCheckSamplingInterval = 2.f; // (Ball Quality Check)
    m_qualityCheckRangeMin = 0.5f; // (Normal Method)
    m_qualityCheckRangeMax = 1.1f; // (Normal Method)
    m_qualityRingThicknessRate = 0.25f; // (Ring Thickness)
    m_qualityRingRadius_px = 0.f;
}

BOOL BallLevelParameter::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    db[_T("{00C026C9-56D8-498B-B977-C9BC23FC55C5}")].Link(bSave, m_widthRatio);
    db[_T("{82F7A1E7-6531-44C2-B53F-9E6E773CE5F0}")].Link(bSave, m_ballWidthRoughSearchLengthRatio);
    if (!bSave)
        m_ballWidthDetailSearchLengthRatio = m_ballWidthRoughSearchLengthRatio;
    db[_T("{46B9CB64-38F1-408D-8CDD-45F465306829}")].Link(
        bSave, m_ballWidthDetailSearchLengthRatio); //kircheis_BallFlux
    db[_T("{D89A1097-353E-489E-B61A-B16E19876542}")].Link(bSave, m_ballWidthSearchIgnoreLengthRatio); //kk
    db[_T("{FD05235B-17EE-4DEE-B704-3C60A09B813D}")].Link(bSave, m_nBallEdgeCountMode); //kircheis_BallFlux
    db[_T("{8EAE38E4-6F2D-4111-AABE-CFDACF9073A5}")].Link(bSave, m_nBallEdgeReSearchMode); //kircheis_BallFlux
    db[_T("{B98A752D-35DC-4F18-81A1-97A4603D1A33}")].Link(bSave, m_ballReSearchLengthRatioInner); //kircheis_BallFlux
    db[_T("{9918663E-2A49-45BB-8C1A-66997810D065}")].Link(bSave, m_ballReSearchLengthRatioOuter); //kircheis_BallFlux
    db[_T("{9DD01D6B-04AA-4CCA-BDCA-D796E2B334E3}")].Link(
        bSave, m_ballReSearchNoiseFilterThresholdRatio); //kircheis_BallFluxModify
    db[_T("{D4407A8A-4D4D-46D8-BE20-222A25EBBCF7}")].Link(bSave, m_nBallWidthMethod);
    db[_T("{5C51F565-5884-4113-A8A6-F37C10A32A0E}")].Link(bSave, m_nRoughAlignMethod); //kircheis_LKF

    db[_T("{7B6F2EBF-6669-46B7-9460-77C3568786A5}")].Link(bSave, m_qualityThresholdValue);
    db[_T("{FA8E26F7-5A8C-4B10-8D82-E185657A7F82}")].Link(bSave, m_qualityCheckMethod);
    db[_T("{68F0C82B-304B-4A31-AE44-91FA7EE6CED7}")].Link(bSave, m_qualityCheckOuterVoid);
    db[_T("{9B670ECF-6488-4696-B542-5B4DF6A80FAA}")].Link(bSave, m_qualityCheckInnerVoid);
    db[_T("{5412E2DE-BDCF-4423-B240-59BA79DCDFFA}")].Link(bSave, m_qualityCheckSamplingInterval);
    db[_T("{3EE20893-6B0A-415B-83AE-3E333155AC8F}")].Link(bSave, m_qualityCheckRangeMin);
    db[_T("{934F54BE-6EE3-4544-9D84-8EDB0E5AB07F}")].Link(bSave, m_qualityCheckRangeMax);
    db[_T("{91186C69-2363-41DC-BCB8-2BCBC93DE535}")].Link(bSave, m_qualityRingThicknessRate);
    db[_T("{248123DF-8350-4655-ACF5-4BA2994F8A31}")].Link(bSave, m_qualityRingRadius_px);

    return TRUE;
}

void BallLevelParameter::Copy(BallLevelParameter Data)
{
    // Ball Width
    m_nBallWidthMethod = Data.m_nBallWidthMethod;
    m_widthRatio = Data.m_widthRatio;
    m_nBallEdgeCountMode = Data.m_nBallEdgeCountMode;
    m_ballWidthRoughSearchLengthRatio = Data.m_ballWidthRoughSearchLengthRatio;
    m_ballWidthDetailSearchLengthRatio = Data.m_ballWidthDetailSearchLengthRatio;
    m_ballWidthSearchIgnoreLengthRatio = Data.m_ballWidthSearchIgnoreLengthRatio;
    m_nBallEdgeReSearchMode = Data.m_nBallEdgeReSearchMode;
    m_ballReSearchLengthRatioInner = Data.m_ballReSearchLengthRatioInner;
    m_ballReSearchLengthRatioOuter = Data.m_ballReSearchLengthRatioOuter;
    m_ballReSearchNoiseFilterThresholdRatio = Data.m_ballReSearchNoiseFilterThresholdRatio; //kircheis_BallFluxModify
    m_nRoughAlignMethod = Data.m_nRoughAlignMethod; //kircheis_LKF

    // Ball Quality
    m_qualityThresholdValue = Data.m_qualityThresholdValue;
    m_qualityCheckMethod = Data.m_qualityCheckMethod;
    m_qualityCheckOuterVoid = Data.m_qualityCheckOuterVoid;
    m_qualityCheckInnerVoid = Data.m_qualityCheckInnerVoid;
    m_qualityCheckSamplingInterval = Data.m_qualityCheckSamplingInterval;
    m_qualityCheckRangeMin = Data.m_qualityCheckRangeMin;
    m_qualityCheckRangeMax = Data.m_qualityCheckRangeMax;
    m_qualityRingThicknessRate = Data.m_qualityRingThicknessRate;
    m_qualityRingRadius_px = Data.m_qualityRingRadius_px;
}

BOOL VisionInspectionBgaBottom2DPara::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    CString strTemp;

    if (!bSave)
    {
        Init();
    }

    long version = 1;

    if (!db[_T("{78C7F9FB-52BF-45A7-820F-01224666F135}")].Link(bSave, version))
        version = 0;

    switch (version)
    {
        case 0:
        {
            if (m_parameters.size() <= 0)
                m_parameters.resize(1);

            float QualityCheckOuterVoid;
            float QualityCheckInnerVoid;
            float QualityCheckSamplingInterval;
            float WidthRatio;
            float WidthSearchLineLength;
            float QualityCheckRangeMin;
            float QualityCheckRangeMax;
            float QualityRingThicknessRate;
            float SquashBallAreaRate;
            float QualityRingRadius;

            if (!db[_T("VisionBGABottomObliqueCoaxial")][_T("Threshold_Value_Oblique")].Link(
                    bSave, m_parameters[0].m_qualityThresholdValue))
                m_parameters[0].m_qualityThresholdValue = 128;
            if (!db[_T("Ball_Quality_Check_Method")].Link(bSave, m_parameters[0].m_qualityCheckMethod))
                m_parameters[0].m_qualityCheckMethod = enumNormalMethod;
            if (!db[_T("Quality_Check_Outer_Void")].Link(bSave, QualityCheckOuterVoid))
                QualityCheckOuterVoid = 1.2f;
            if (!db[_T("Quality_Check_Inner_Void")].Link(bSave, QualityCheckInnerVoid))
                QualityCheckInnerVoid = 0.f;
            if (!db[_T("Quality_Check_Sampling_Interval")].Link(bSave, QualityCheckSamplingInterval))
                QualityCheckSamplingInterval = 2.f;
            if (!db[_T("Quality_Check_Range_Min")].Link(bSave, QualityCheckRangeMin))
                QualityCheckRangeMin = 0.5f;
            if (!db[_T("Quality_Check_Range_Max")].Link(bSave, QualityCheckRangeMax))
                QualityCheckRangeMax = 1.1f;
            if (!db[_T("Quality_Ring_Thickness_Rate")].Link(bSave, QualityRingThicknessRate))
                QualityRingThicknessRate = 0.25f;
            if (!db[_T("fSquashBallAreaRate")].Link(bSave, SquashBallAreaRate))
                SquashBallAreaRate = 0.7f;
            if (!db[_T("QualityRingRadius")].Link(bSave, QualityRingRadius))
                QualityRingRadius = 0.0f;
            if (!db[_T("WidthRatio")].Link(bSave, WidthRatio))
                WidthRatio = 0.1f;
            if (!db[_T("WidthSearchLineLength")].Link(bSave, WidthSearchLineLength))
                WidthSearchLineLength = 0.75f;
            if (!db[_T("m_nBallWidthMethod")].Link(bSave, m_parameters[0].m_nBallWidthMethod))
                m_parameters[0].m_nBallWidthMethod = enumGetPittingValue; //kircheis_20160602
            if (!db[_T("nImageCombineMode")].Link(bSave, m_nImageCombineMode))
                m_nImageCombineMode = enumCombineMode_Default;
            if (!db[_T("m_nRoughAlignMethod")].Link(bSave, m_parameters[0].m_nRoughAlignMethod))
                m_parameters[0].m_nRoughAlignMethod = enumBallRoughAlignMethod_DefaultCircleFitting; //kircheis_LKF

            m_qualityCombineMode = enumQualityCombineMode_Default;
            m_parameters[0].m_qualityCheckOuterVoid = QualityCheckOuterVoid;
            m_parameters[0].m_qualityCheckInnerVoid = QualityCheckInnerVoid;
            m_parameters[0].m_qualityCheckSamplingInterval = QualityCheckSamplingInterval;
            m_parameters[0].m_widthRatio = WidthRatio;
            m_parameters[0].m_nBallEdgeCountMode = enumBallEdgeFastMode;
            m_parameters[0].m_ballWidthRoughSearchLengthRatio = WidthSearchLineLength;
            m_parameters[0].m_ballWidthDetailSearchLengthRatio = WidthSearchLineLength; //kircheis_BallFlux
            m_parameters[0].m_nBallEdgeReSearchMode = enumBallEdgeReSearchNotUse; //kircheis_BallFlux
            m_parameters[0].m_ballReSearchLengthRatioInner = 0.2f; //kircheis_BallFlux
            m_parameters[0].m_ballReSearchLengthRatioOuter = 0.2f; //kircheis_BallFlux
            m_parameters[0].m_ballReSearchNoiseFilterThresholdRatio = 0.12f; //kircheis_BallFluxModify
            m_parameters[0].m_qualityCheckRangeMin = QualityCheckRangeMin;
            m_parameters[0].m_qualityCheckRangeMax = QualityCheckRangeMax;
            m_parameters[0].m_qualityRingThicknessRate = QualityRingThicknessRate;
            m_parameters[0].m_qualityRingRadius_px = QualityRingRadius;

            long obliqueImageNo = 0;
            long coaxialImageNo = 0;
            if (!db[_T("VisionBGABottomAlignPara")][_T("Oblique_Image_No")].Link(bSave, obliqueImageNo))
                obliqueImageNo = 0;
            if (!db[_T("m_nCoaxialImageNo")].Link(bSave, coaxialImageNo))
                coaxialImageNo = 0;

            m_obliqueImageIndex.setFrameListIndex(obliqueImageNo);
            m_coaxialImageIndex.setFrameListIndex(coaxialImageNo);
            m_qualityObliqueImageIndex.setFrameListIndex(obliqueImageNo);
            m_qualityCoaxialImageIndex.setFrameListIndex(coaxialImageNo);
        }
        break;
        case 1:
        {
            db[_T("{45FFD15E-B449-41CE-86B3-786980EF6261}")].Link(bSave, m_nImageCombineMode);

            if (!bSave)
            {
                m_qualityCombineMode = 0;
            }

            db[_T("{EA21D163-6985-454D-B918-11896F2CF95E}")].Link(bSave, m_qualityCombineMode);

            if (!m_obliqueImageIndex.LinkDataBase(bSave, db[_T("{1D59596F-57EC-42BB-A39F-969C65777687}")]))
                m_obliqueImageIndex.setFrameListIndex(0);
            if (!m_coaxialImageIndex.LinkDataBase(bSave, db[_T("{17B58F83-A10B-4954-A007-9E1536C26F24}")]))
                m_coaxialImageIndex.setFrameListIndex(0);

            if (!bSave)
            {
                m_qualityObliqueImageIndex = m_obliqueImageIndex;
                m_qualityCoaxialImageIndex = m_coaxialImageIndex;
            }

            // Quaility Coaxial, Oblique이 분리됨
            m_qualityObliqueImageIndex.LinkDataBase(bSave, db[_T("{9C9894F3-A24B-465B-A707-54A8149C36BA}")]);
            m_qualityCoaxialImageIndex.LinkDataBase(bSave, db[_T("{ADF27484-8112-44A2-B8DD-4B8DB244D16A}")]);

            long nGroupCount = 0;
            if (bSave)
                nGroupCount = (long)m_parameters.size();
            if (!db[_T("{3EFA85F0-87AB-4551-8DD7-3F898F774EAE}")].Link(bSave, nGroupCount))
                nGroupCount = 0;

            m_parameters.resize(nGroupCount);

            for (int i = 0; i < nGroupCount; i++)
            {
                m_parameters[i].LinkDataBase(bSave, db.GetSubDBFmt(_T("{7A0FA7FE-462C-4CF0-99EE-E348BB61C746}_%d"), i));
            }

            if (nGroupCount == 0)
            {
                BallLevelParameter temp;

                db[_T("{45A76FE2-546A-4903-9E4D-2F5E00303B0A}")].Link(bSave, temp.m_widthRatio);
                db[_T("{18B49983-BE59-47E0-8151-57EB641953E2}")].Link(bSave, temp.m_ballWidthRoughSearchLengthRatio);
                if (!bSave)
                    temp.m_ballWidthDetailSearchLengthRatio = temp.m_ballWidthRoughSearchLengthRatio;
                db[_T("{FC3EFB9F-9760-4186-B1EC-68BF589EF310}")].Link(
                    bSave, temp.m_ballWidthDetailSearchLengthRatio); //kircheis_BallFlux
                db[_T("{559EB30B-A185-4AAB-9605-8951431A9494}")].Link(
                    bSave, temp.m_nBallEdgeCountMode); //kircheis_BallFlux
                db[_T("{3F361A2A-3BCE-4228-B0E2-C180D5460793}")].Link(
                    bSave, temp.m_nBallEdgeReSearchMode); //kircheis_BallFlux
                db[_T("{3408BE3E-62A8-4601-80F0-570C1BBDE659}")].Link(
                    bSave, temp.m_ballReSearchLengthRatioInner); //kircheis_BallFlux
                db[_T("{3D24EBF8-92CD-4BBC-8996-85E3733289F0}")].Link(
                    bSave, temp.m_ballReSearchLengthRatioOuter); //kircheis_BallFlux
                db[_T("{4D15CF6B-E2B9-4588-9EE8-F61DE79A5D10}")].Link(
                    bSave, temp.m_ballReSearchNoiseFilterThresholdRatio); //kircheis_BallFluxModify
                db[_T("{D59C4ADA-8528-4902-8D7A-6BD4C6B75350}")].Link(bSave, temp.m_nBallWidthMethod);
                db[_T("{DD3F38C2-69A0-4530-B699-83EE16C9C652}")].Link(bSave, temp.m_nRoughAlignMethod); //kircheis_LKF

                db[_T("{76524958-CE02-4B36-95DC-C67ED11EEBD1}")].Link(bSave, temp.m_qualityThresholdValue);
                db[_T("{634B9EF8-8D10-42FB-BCA7-4AAFC6937473}")].Link(bSave, temp.m_qualityCheckMethod);
                db[_T("{512011BD-070F-4177-B6E6-B166594B9F8C}")].Link(bSave, temp.m_qualityCheckOuterVoid);
                db[_T("{BBD0DAA9-9067-465A-AF7A-5B1BA6C265E0}")].Link(bSave, temp.m_qualityCheckInnerVoid);
                db[_T("{F1E4F78C-BB0F-4E1D-8DAC-57D056572AA1}")].Link(bSave, temp.m_qualityCheckSamplingInterval);
                db[_T("{78522AE5-D3E6-40F9-B2A1-51E1663B8814}")].Link(bSave, temp.m_qualityCheckRangeMin);
                db[_T("{7F30839E-9C7B-4BD8-950D-59D1BC8D6594}")].Link(bSave, temp.m_qualityCheckRangeMax);
                db[_T("{BA4342EF-0DF7-4B1F-A129-6C395034A4E7}")].Link(bSave, temp.m_qualityRingThicknessRate);
                db[_T("{7E943F44-9C17-4222-9376-90792993A0B7}")].Link(bSave, temp.m_qualityRingRadius_px);

                if (m_parameters.size() <= 0)
                    m_parameters.resize(1);
                m_parameters[0].Copy(temp);
            }

            //kk Group Spec 저장 부분

            long GroupSpec = 0;

            if (bSave)
                GroupSpec = (long)m_vecstrGroupInspName.size();

            if (!db[_T("{6F203D0B-CF85-424A-BFB5-17CCBA24BDA2}")].Link(bSave, GroupSpec))
                GroupSpec = 0;

            m_vecstrGroupInspName.resize(GroupSpec);

            for (int i = 0; i < GroupSpec; i++)
            {
                CString str;

                str.Format(_T("{AB7DD425-D026-4A0E-8C48-E6B0081D6F44}_%d"), i);

                m_vecstrGroupInspName[i].LinkDataBase(
                    bSave, db.GetSubDBFmt(_T("{099E5E79-B27F-4606-BB3C-9D97E660BF09}_%d"), i));

                str.Empty();
            }

            // ksy deeplearning 저장 부분
            if (!db[_T("{31732990-3557-4E4D-BEDB-5C245B3E6C36}")].Link(bSave, m_use2ndInspection))
                m_use2ndInspection = FALSE;
            if (!db[_T("{4AA501A8-34CD-4B2A-AB56-C0ACE66EA5B8}")].Link(bSave, m_str2ndInspCode))
                m_str2ndInspCode = _T("No Code");
        }
        break;
    }

    strTemp.Empty();

    return TRUE;
}

std::vector<CString> VisionInspectionBgaBottom2DPara::ExportAlgoParaToText(
    CString strVisionName, CString strInspectionModuleName) //kircheis_TxtRecipe
{
    //본 모듈의 공통 Parameter용 변수 (미리 값을 Fix)
    CString strHeader;
    strHeader.Format(_T("%s,%s"), (LPCTSTR)strVisionName, (LPCTSTR)strInspectionModuleName);

    //개별 Parameter용 변수
    CString strCategory;
    CString strGroup;
    CString strParaName;

    //Output Parameter 생성
    std::vector<CString> vecstrAlgorithmParameters(0);

    //Algorithm Parameters
    strCategory.Format(_T("Algorithm Parameters"));

    //Main Parameters
    strGroup.Format(_T("Ball Inspection Common Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Image Combine Mode"), _T(""), m_nImageCombineMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Oblique image"), _T(""), m_obliqueImageIndex.getFrameIndex()));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Coaxial image"), _T(""), m_coaxialImageIndex.getFrameIndex()));

    strGroup.Format(_T("Ball quality Inspection Common Parameter"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Image Combine Mode"), _T(""), m_qualityCombineMode));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Oblique image"), _T(""), m_qualityObliqueImageIndex.getFrameIndex()));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Coaxial image"), _T(""), m_qualityCoaxialImageIndex.getFrameIndex()));

    //SubDlg Parameters
    long nGroupNum = (long)m_parameters.size();
    vecstrAlgorithmParameters.push_back(
        CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, _T("Group Num"), _T(""), _T(""), nGroupNum));

    for (long nGroup = 0; nGroup < nGroupNum; nGroup++)
    {
        auto& ballParam = m_parameters[nGroup];
        strGroup.Format(_T("Group %d"), nGroup + 1);

        strParaName.Format(_T("Ball width calculation parameters"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball edge search mode"), ballParam.m_nBallEdgeCountMode));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Inspection method"), ballParam.m_nBallWidthMethod));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball edge rough search length ratio"), ballParam.m_ballWidthRoughSearchLengthRatio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball edge detail search length ratio"), ballParam.m_ballWidthDetailSearchLengthRatio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball edge search ignore length ratio"), ballParam.m_ballWidthSearchIgnoreLengthRatio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Edge re-search mode by oblique"), ballParam.m_nBallEdgeReSearchMode));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball edge re-search length ratio (Inner)"), ballParam.m_ballReSearchLengthRatioInner));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball edge re-search length ratio (Outer)"), ballParam.m_ballReSearchLengthRatioOuter));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball edge re-search noise filter threshold ratio"),
            ballParam.m_ballReSearchNoiseFilterThresholdRatio));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Ball rough align method"), ballParam.m_nRoughAlignMethod));

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        strParaName.Format(_T("Ball quality inspection parameters"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Threshold"), ballParam.m_qualityThresholdValue));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Inspection method"), ballParam.m_qualityCheckMethod));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Void checking position in"), ballParam.m_qualityCheckInnerVoid));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Void checking position out"), ballParam.m_qualityCheckOuterVoid));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Sampling interval"), ballParam.m_qualityCheckSamplingInterval));

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        strParaName.Format(_T("Normal method"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Checking range begin"), ballParam.m_qualityCheckRangeMin));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Checking range end"), ballParam.m_qualityCheckRangeMax));

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        strParaName.Format(_T("Ring thickness method"));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(strHeader, strCategory, strGroup,
            strParaName, _T("Checking range ratio"), ballParam.m_qualityRingThicknessRate));
        vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
            strHeader, strCategory, strGroup, strParaName, _T("Taught ring radius"), ballParam.m_qualityRingRadius_px));
    }

    //DL Parameters
    strCategory.Format(_T("Deep Learning Parameters"));
    strGroup.Format(_T("2nd Inspection Parameter (Deep Learning)"));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("Use Inspection"), _T(""), (bool)m_use2ndInspection));
    vecstrAlgorithmParameters.push_back(CTxtRecipeParameter::GetTextFromParmeter(
        strHeader, strCategory, strGroup, _T("DL Model Match Code"), _T(""), m_str2ndInspCode));

    return vecstrAlgorithmParameters;
}

void VisionInspectionBgaBottom2DPara::Init()
{
    // 2D Algorithm Parameter...
    m_nImageCombineMode = enumCombineMode_Default;

    m_qualityCombineMode = enumQualityCombineMode_Default;

    m_use2ndInspection = FALSE;
    m_str2ndInspCode = _T("");

    m_obliqueImageIndex.setFrameListIndex(0);
    m_coaxialImageIndex.setFrameListIndex(0);
    m_qualityObliqueImageIndex.setFrameListIndex(0);
    m_qualityCoaxialImageIndex.setFrameListIndex(0);
}

void Ball2DGroupInsp::SetName(LPCTSTR groupID)
{
    CString strGroupID = groupID;
    strGroupID.MakeUpper();
    TCHAR chHeader = strGroupID.GetAt(0);
    if (chHeader != _T('G'))
    {
        strGroupID = groupID;
        strGroupID.Insert(1, _T("G"));
        groupID = strGroupID;
    }

    BallOffsetXName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_X]);
    BallOffsetYName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_Y]);
    BallOffsetRName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_OFFSET_R]);
    BallGridOffsetXName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_GRID_OFFSET_X]);
    BallGridOffsetYName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_GRID_OFFSET_Y]);
    BallWidthName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_WIDTH]);
    BallQualityName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_QUALITY]);
    BallContrastName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_CONTRAST]);
    BallEllipticityName.Format(_T("(%s) %s"), groupID, g_szBallInspectionName[BALL_INSPECTION_BALL_ELLIPTICITY]);

    strGroupID.Empty();
}

BOOL Ball2DGroupInsp::LinkDataBase(BOOL bSave, CiDataBase& db)
{
    if (!db[_T("{E2592B86-C30E-4E59-98E7-F7184DE528B1}")].Link(bSave, BallOffsetXName))
        BallOffsetXName = _T("Untitle");
    if (!db[_T("{11898831-DFE3-4ACE-9F13-2FAAFB2FC3A5}")].Link(bSave, BallOffsetYName))
        BallOffsetYName = _T("Untitle");
    if (!db[_T("{FCE3A94F-56A0-4A0F-BD88-3F16BC406D53}")].Link(bSave, BallOffsetRName))
        BallOffsetRName = _T("Untitle");
    if (!db[_T("{E253A8B8-C72D-45CA-AC63-E64C65BCE0CD}")].Link(bSave, BallGridOffsetXName))
        BallGridOffsetXName = _T("Untitle");
    if (!db[_T("{E097B190-A063-4798-86A0-C8A0D3D4B6F5}")].Link(bSave, BallGridOffsetYName))
        BallGridOffsetYName = _T("Untitle");
    if (!db[_T("{F1A0812F-FE0B-468F-80B7-43AE3A6EB554}")].Link(bSave, BallWidthName))
        BallWidthName = _T("Untitle");
    if (!db[_T("{41AF4245-E210-4D5F-B1D6-D49650E35F0C}")].Link(bSave, BallQualityName))
        BallQualityName = _T("Untitle");
    if (!db[_T("{BE111873-A980-4C99-93A9-5106E9EFC8F4}")].Link(bSave, BallContrastName))
        BallContrastName = _T("Untitle");
    if (!db[_T("{52E02050-0A4F-43ED-B17D-C2E95BE7EE7B}")].Link(bSave, BallEllipticityName))
        BallEllipticityName = _T("Untitle");

    CString strGroupID = BallOffsetXName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallOffsetXName;
        strGroupID.Insert(1, _T("G"));
        BallOffsetXName = strGroupID;
    }

    strGroupID = BallOffsetYName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallOffsetYName;
        strGroupID.Insert(1, _T("G"));
        BallOffsetYName = strGroupID;
    }

    strGroupID = BallOffsetRName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallOffsetRName;
        strGroupID.Insert(1, _T("G"));
        BallOffsetRName = strGroupID;
    }

    strGroupID = BallGridOffsetXName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallGridOffsetXName;
        strGroupID.Insert(1, _T("G"));
        BallGridOffsetXName = strGroupID;
    }

    strGroupID = BallGridOffsetYName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallGridOffsetYName;
        strGroupID.Insert(1, _T("G"));
        BallGridOffsetYName = strGroupID;
    }

    strGroupID = BallWidthName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallWidthName;
        strGroupID.Insert(1, _T("G"));
        BallWidthName = strGroupID;
    }

    strGroupID = BallQualityName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallQualityName;
        strGroupID.Insert(1, _T("G"));
        BallQualityName = strGroupID;
    }

    strGroupID = BallContrastName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallContrastName;
        strGroupID.Insert(1, _T("G"));
        BallContrastName = strGroupID;
    }

    strGroupID = BallEllipticityName;
    strGroupID.MakeUpper();
    if (strGroupID.GetAt(1) != 'G')
    {
        strGroupID = BallEllipticityName;
        strGroupID.Insert(1, _T("G"));
        BallEllipticityName = strGroupID;
    }

    strGroupID.Empty();

    return TRUE;
}
