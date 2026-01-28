//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "Passive2DAlgorithm.h"

//CPP_2_________________________________ This project's headers
#include "Result.h"

//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_EdgeDetect/dPI_EdgeDetectLib.h"
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h"
#include "../../DefineModules/dA_Base/VisionScale.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/Geometry.h>
#include <Ipvm/Algorithm/ImageProcessing.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/Image8u.h>

//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define ErrorMax 2.f
#define ErrorMin 0.4f

//CPP_7_________________________________ Implementation body
//
CPassive2DAlgorithm::CPassive2DAlgorithm(sPassive_InfoDB* i_pPassiveInfoDB,
    PassiveAlignSpec::CapAlignSpec* i_pPassiveAlignParam, CPI_EdgeDetect* i_pEdgeDetect,
    ResultPassiveItem* i_pPassiveResult, PassiveAlignResult& o_PassiveAlignResult)
{
    m_pPassiveInfoDB = i_pPassiveInfoDB;
    m_pPassiveAlignParam = i_pPassiveAlignParam;
    m_pPassiveAlignResult = &o_PassiveAlignResult;
    m_pPassiveResult_DebugItem = i_pPassiveResult;
    m_pEdgeDetect = i_pEdgeDetect;
}

CPassive2DAlgorithm::~CPassive2DAlgorithm(void)
{
}

bool CPassive2DAlgorithm::DoCapacitor_Align(const VisionScale& scale, const Ipvm::Image8u& i_RoughAlignImage,
    const Ipvm::Image8u& i_OutlineAlign_ElectrodeImage, const Ipvm::Image8u& i_OutlineAlign_BodyImage)
{
    if (!m_pPassiveInfoDB)
        return false;

    FPI_RECT frtRoughAlignResult;
    if (!RoughAlign(scale, i_RoughAlignImage, m_pPassiveInfoDB->sfrtSpecROI_px, frtRoughAlignResult))
        return false;

    if (m_pPassiveAlignParam->m_bRoughAlign_OnlyUse)
    {
        m_pPassiveAlignResult->m_chip_resFinal = frtRoughAlignResult;
        return true;
    }

    std::vector<std::vector<FPI_RECT>> vec2frtChip_SearchROI(4);
    if (!MakeDetailAlignROI_byRoughAlign(scale, frtRoughAlignResult, vec2frtChip_SearchROI))
        return false;

    long nPassiveAngleType(0);
    if (m_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Horizontal)
        nPassiveAngleType = Passive_Horizontal;
    else if (m_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Vertical)
        nPassiveAngleType = Passive_Vertical;

    if (!DoCapacitor_DetailAlign(i_OutlineAlign_ElectrodeImage, i_OutlineAlign_BodyImage, vec2frtChip_SearchROI,
            frtRoughAlignResult, nPassiveAngleType))
    {
        m_pPassiveResult_DebugItem->m_bAlignSuccess = false;
        return false;
    }
    else
        m_pPassiveResult_DebugItem->m_bAlignSuccess = true;

    Create_Capacitor_Electrode(nPassiveAngleType, m_pPassiveAlignResult->m_chip_resFinal);

    Create_PAD_Area(scale, nPassiveAngleType, m_pPassiveAlignResult->m_chip_resFinal);

    return true;
}

bool CPassive2DAlgorithm::DoMIA_Align(
    const VisionScale& scale, const Ipvm::Image8u& i_RoughAlignImage, const Ipvm::Image8u& i_OutlineAlign_BodyImage)
{
    if (!m_pPassiveInfoDB)
        return false;

    FPI_RECT frtRoughAlignResult;
    if (!RoughAlign(scale, i_RoughAlignImage, m_pPassiveInfoDB->sfrtSpecROI_px, frtRoughAlignResult))
        return false;

    if (m_pPassiveAlignParam->m_bRoughAlign_OnlyUse)
    {
        m_pPassiveAlignResult->m_chip_resFinal = frtRoughAlignResult;
        return true;
    }

    std::vector<std::vector<FPI_RECT>> vec2frtChip_SearchROI(4);
    if (!MakeDetailAlignROI_byRoughAlign(scale, frtRoughAlignResult, vec2frtChip_SearchROI))
        return false;

    if (!DoMIA_DetailAlign(i_OutlineAlign_BodyImage, vec2frtChip_SearchROI, frtRoughAlignResult, Passive_MIA))
    {
        m_pPassiveResult_DebugItem->m_bAlignSuccess = false;
        return false;
    }
    else
        m_pPassiveResult_DebugItem->m_bAlignSuccess = true;

    return true;
}

bool CPassive2DAlgorithm::RoughAlign(const VisionScale& scale, const Ipvm::Image8u& i_RoughAlignImage,
    FPI_RECT i_frtSpecROI, FPI_RECT& o_frtRoughROI) //kircheis_MED2Passive
{
    //{{//kircheis_MED2Passive
    const auto& um2px = scale.umToPixel();
    float fHorExtentionUM = (float)m_pPassiveAlignParam->m_nRougAlign_SearchOffsetX_um;
    float fVerExtentionUM = (float)m_pPassiveAlignParam->m_nRougAlign_SearchOffsetY_um;
    const long nHorExtention = (long)(fHorExtentionUM * um2px.m_x + .5f);
    const long nVerExtention = (long)(fVerExtentionUM * um2px.m_y + .5f);
    //}}

    o_frtRoughROI = FPI_RECT(Ipvm::Point32r2(-1.f, -1.f), Ipvm::Point32r2(-1.f, -1.f), Ipvm::Point32r2(-1.f, -1.f),
        Ipvm::Point32r2(-1.f, -1.f));
    Ipvm::Rect32s rtSpecROI = i_frtSpecROI.GetExtCRect();
    rtSpecROI.NormalizeRect();
    Ipvm::Rect32s rtHor = rtSpecROI;
    Ipvm::Rect32s rtVer = rtSpecROI;
    rtHor.InflateRect(nHorExtention, 0);
    rtVer.InflateRect(0, nVerExtention);

    if (m_pPassiveResult_DebugItem)
    {
        m_pPassiveResult_DebugItem->m_roughAlignROI_Hor = rtHor;
        m_pPassiveResult_DebugItem->m_roughAlignROI_Ver = rtVer;
    }

    long nHorLength = rtHor.Width();
    long nVerLength = rtVer.Height();
    long i(0), j(0);

    std::vector<BYTE> vecbyHorProfile(nHorLength);
    std::vector<BYTE> vecbyHorProfileReverse(nHorLength);
    std::vector<BYTE> vecbyVerProfile(nVerLength);
    std::vector<BYTE> vecbyVerProfileReverse(nVerLength);

    Ipvm::ImageProcessing::MakeProjectionProfileAxisX(i_RoughAlignImage, rtHor, &vecbyHorProfile[0]);

    for (i = 0, j = nHorLength - 1; i < nHorLength; i++, j--)
        vecbyHorProfileReverse[j] = vecbyHorProfile[i];

    Ipvm::ImageProcessing::MakeProjectionProfileAxisY(i_RoughAlignImage, rtVer, &vecbyVerProfile[0]);

    for (i = 0, j = nVerLength - 1; i < nVerLength; i++, j--)
        vecbyVerProfileReverse[j] = vecbyVerProfile[i];

    float fEdgeThreshold(10.f);
    float fEdgePos(0.f), fEdgeValue(0.f);
    float fEdgeThresholdOld = m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold);
    BOOL bFirstEdge = m_pPassiveAlignParam->m_bRoughAlign_ElectrodeColor;
    BOOL bSubPixelingDone(FALSE);

    o_frtRoughROI = i_frtSpecROI;

    for (i = 1; i <= 3;
        i++) //Left //무조건 성공하는게 좋으니까 Edge 추출에 실패하면 Edge Threshold를 낮춰가면서 재시도를 2회 더 하도록 한다.
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold / (float)i);
        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nHorLength / 2, &vecbyHorProfile[0], fEdgePos,
                fEdgeValue, bSubPixelingDone, bFirstEdge)) //Left
        {
            o_frtRoughROI.fptLT.m_x = o_frtRoughROI.fptLB.m_x
                = (float)rtSpecROI.m_left - (float)nHorExtention + fEdgePos;
            break;
        }
    }

    for (i = 1; i <= 3; i++) //Right
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold / (float)i);
        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nHorLength / 2, &vecbyHorProfileReverse[0],
                fEdgePos, fEdgeValue, bSubPixelingDone, bFirstEdge)) //Right
        {
            o_frtRoughROI.fptRT.m_x = o_frtRoughROI.fptRB.m_x
                = (float)rtSpecROI.m_right + (float)nHorExtention - fEdgePos;
            break;
        }
    }

    for (i = 1; i <= 3; i++) //Top
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold / (float)i);
        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nVerLength / 2, &vecbyVerProfile[0], fEdgePos,
                fEdgeValue, bSubPixelingDone, bFirstEdge)) //Top
        {
            o_frtRoughROI.fptLT.m_y = o_frtRoughROI.fptRT.m_y
                = (float)rtSpecROI.m_top - (float)nVerExtention + fEdgePos;
            break;
        }
    }

    for (i = 1; i <= 3; i++) //Down
    {
        m_pEdgeDetect->SetMininumThreshold(fEdgeThreshold / (float)i);
        if (m_pEdgeDetect->PI_ED_CalcEdgePointFromLine(PI_ED_DIR_RISING, nVerLength / 2, &vecbyVerProfileReverse[0],
                fEdgePos, fEdgeValue, bSubPixelingDone, bFirstEdge)) //Down
        {
            o_frtRoughROI.fptLB.m_y = o_frtRoughROI.fptRB.m_y
                = (float)rtSpecROI.m_bottom + (float)nVerExtention - fEdgePos;
            break;
        }
    }

    m_pEdgeDetect->SetMininumThreshold(fEdgeThresholdOld);

    m_pPassiveAlignResult->m_rtPassiveRough_Align_Result_ROI = o_frtRoughROI.GetExtCRect();

    if (m_pPassiveResult_DebugItem)
        m_pPassiveResult_DebugItem->m_roughAlignResult = o_frtRoughROI;

    return true;
}

bool CPassive2DAlgorithm::MakeDetailAlignROI_byRoughAlign(const VisionScale& scale, FPI_RECT i_frtRoughROI,
    std::vector<std::vector<FPI_RECT>>& o_vec2frtChip_SearchROI) //kircheis_MED2Passive
{
    if (!m_pPassiveInfoDB)
        return false;

    Ipvm::Point32r2 RoughAlignCenter = i_frtRoughROI.GetCenter();
    Ipvm::Rect32s rtRoughAlignROI = i_frtRoughROI.GetExtCRect();

    float fRoughAlignCenterX = RoughAlignCenter.m_x;
    float fRoughAlignCenterY = RoughAlignCenter.m_y;
    FPI_RECT frtDetailAlign_LeftSearchROI, frtDetailAlign_TopSearchROI, frtDetailAlign_RightSearchROI,
        frtDetailAlign_BottomSearchROI;

    const auto& um2px = scale.umToPixel();

    float fElectrode_Search_Width_Ratio = m_pPassiveAlignParam->m_nOutline_align_Electrode_Search_Width_Ratio / 100.f;
    float fElectrode_Search_Length_um = (float)m_pPassiveAlignParam->m_nOutline_align_Electrode_Search_Length_um;
    float fBody_Search_Width_Ratio = m_pPassiveAlignParam->m_nOutline_align_Body_Search_Width_Ratio / 100.f;
    float fBody_Search_Length_um = (float)m_pPassiveAlignParam->m_nOutline_align_Body_Search_Length_um;

    float fElectrode_Search_LengthOffset(0.f), fBody_Search_LengthOffset(0.f); //Direction별로 다르게 구성해야한다

    if (m_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Horizontal
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA
        || m_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Horizontal_reverse
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        float fSpecWidth = (float)rtRoughAlignROI.Height();
        float fSpecLength = (float)rtRoughAlignROI.Width();
        float fHalfSpecWidth = fSpecWidth * .5f;
        float fHalfSpecLength = fSpecLength * .5f;

        fElectrode_Search_LengthOffset = (fElectrode_Search_Length_um * um2px.m_x) + .5f;
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_y) + .5f;

        //{{Left
        float fLeftSearchROICenX = fRoughAlignCenterX - fHalfSpecLength;
        frtDetailAlign_LeftSearchROI.fptLT
            = Ipvm::Point32r2((fLeftSearchROICenX - (fElectrode_Search_LengthOffset * .5f)),
                (fRoughAlignCenterY - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRT
            = Ipvm::Point32r2((fLeftSearchROICenX + (fElectrode_Search_LengthOffset * .5f)),
                (fRoughAlignCenterY - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptLB
            = Ipvm::Point32r2((fLeftSearchROICenX - (fElectrode_Search_LengthOffset * .5f)),
                (fRoughAlignCenterY + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRB
            = Ipvm::Point32r2((fLeftSearchROICenX + (fElectrode_Search_LengthOffset * .5f)),
                (fRoughAlignCenterY + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[LEFT].push_back(frtDetailAlign_LeftSearchROI);

        ////{{RIGHT
        frtDetailAlign_RightSearchROI = frtDetailAlign_LeftSearchROI.Move(fSpecLength, 0);

        o_vec2frtChip_SearchROI[RIGHT].push_back(frtDetailAlign_RightSearchROI);

        //}}

        //{{TOP
        float fTopSearchROICenY = fRoughAlignCenterY - fHalfSpecWidth;
        frtDetailAlign_TopSearchROI.fptLT
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRT
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY + (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecLength * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY + (fBody_Search_LengthOffset * .5f)));

        o_vec2frtChip_SearchROI[UP].push_back(frtDetailAlign_TopSearchROI);

        //{{BOTTOM
        frtDetailAlign_BottomSearchROI = frtDetailAlign_TopSearchROI.Move(0, fSpecWidth);

        o_vec2frtChip_SearchROI[DOWN].push_back(frtDetailAlign_BottomSearchROI);
        //}}
    }
    else if (m_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Vertical
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA
        || m_pPassiveInfoDB->fPassiveAngle == PassiveAngle_Vertical_reverse
            && m_pPassiveInfoDB->ePassiveType != enumPassiveTypeDefine::enum_PassiveType_MIA)

    {
        float fSpecWidth = (float)rtRoughAlignROI.Width();
        float fSpecLength = (float)rtRoughAlignROI.Height();
        float fHalfSpecWidth = fSpecWidth * .5f;
        float fHalfSpecLength = fSpecLength * .5f;

        fElectrode_Search_LengthOffset = (fElectrode_Search_Length_um * um2px.m_y) + .5f;
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_x) + .5f;

        //{{LEFT
        float fLeftSearchROICenX = fRoughAlignCenterX - fHalfSpecWidth;
        frtDetailAlign_LeftSearchROI.fptLT = Ipvm::Point32r2((fLeftSearchROICenX - (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY - (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRT = Ipvm::Point32r2((fLeftSearchROICenX + (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY - (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptLB = Ipvm::Point32r2((fLeftSearchROICenX - (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY + (fHalfSpecLength * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRB = Ipvm::Point32r2((fLeftSearchROICenX + (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY + (fHalfSpecLength * fBody_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[LEFT].push_back(frtDetailAlign_LeftSearchROI);

        //{{Right
        frtDetailAlign_RightSearchROI = frtDetailAlign_LeftSearchROI.Move(fSpecWidth, 0);

        o_vec2frtChip_SearchROI[RIGHT].push_back(frtDetailAlign_RightSearchROI);

        //}}

        //{{TOP
        float fTopSearchROICenY = fRoughAlignCenterY - fHalfSpecLength;
        frtDetailAlign_TopSearchROI.fptLT
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fTopSearchROICenY - (fElectrode_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRT
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fTopSearchROICenY - (fElectrode_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fTopSearchROICenY + (fElectrode_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fHalfSpecWidth * fElectrode_Search_Width_Ratio)),
                (fTopSearchROICenY + (fElectrode_Search_LengthOffset * .5f)));

        o_vec2frtChip_SearchROI[UP].push_back(frtDetailAlign_TopSearchROI);

        //{{Bottom
        frtDetailAlign_BottomSearchROI = frtDetailAlign_TopSearchROI.Move(0, fSpecLength);

        o_vec2frtChip_SearchROI[DOWN].push_back(frtDetailAlign_BottomSearchROI);
        //}}
    }
    else if (m_pPassiveInfoDB->ePassiveType == enumPassiveTypeDefine::enum_PassiveType_MIA)
    {
        Ipvm::Point32r2 fDir_Hor_SpecSize(CAST_FLOAT(rtRoughAlignROI.Height()), CAST_FLOAT(rtRoughAlignROI.Width()));
        Ipvm::Point32r2 fDir_Ver_SpecSize(CAST_FLOAT(rtRoughAlignROI.Width()), CAST_FLOAT(rtRoughAlignROI.Height()));

        Ipvm::Point32r2 fDir_Hor_HalfSpecSize(fDir_Hor_SpecSize.m_x * .5f, fDir_Hor_SpecSize.m_y * .5f);
        Ipvm::Point32r2 fDir_Ver_HalfSpecSize(fDir_Ver_SpecSize.m_x * .5f, fDir_Ver_SpecSize.m_y * .5f);

        //{{Left
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_y) + .5f;
        float fLeftSearchROICenX = fRoughAlignCenterX - fDir_Hor_HalfSpecSize.m_y;
        frtDetailAlign_LeftSearchROI.fptLT = Ipvm::Point32r2((fLeftSearchROICenX - (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY - (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRT = Ipvm::Point32r2((fLeftSearchROICenX + (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY - (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptLB = Ipvm::Point32r2((fLeftSearchROICenX - (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY + (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));
        frtDetailAlign_LeftSearchROI.fptRB = Ipvm::Point32r2((fLeftSearchROICenX + (fBody_Search_LengthOffset * .5f)),
            (fRoughAlignCenterY + (fDir_Hor_HalfSpecSize.m_x * fBody_Search_Width_Ratio)));

        o_vec2frtChip_SearchROI[LEFT].push_back(frtDetailAlign_LeftSearchROI);
        //}}

        //{{RIGHT
        frtDetailAlign_RightSearchROI = frtDetailAlign_LeftSearchROI.Move(fDir_Hor_SpecSize.m_y, 0);

        o_vec2frtChip_SearchROI[RIGHT].push_back(frtDetailAlign_RightSearchROI);
        //}}

        //{{Top
        fBody_Search_LengthOffset = (fBody_Search_Length_um * um2px.m_x) + .5f;
        float fTopSearchROICenY = fRoughAlignCenterY - fDir_Ver_HalfSpecSize.m_y;
        frtDetailAlign_TopSearchROI.fptLT
            = Ipvm::Point32r2((fRoughAlignCenterX - (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRT
            = Ipvm::Point32r2((fRoughAlignCenterX + (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY - (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptLB
            = Ipvm::Point32r2((fRoughAlignCenterX - (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY + (fBody_Search_LengthOffset * .5f)));
        frtDetailAlign_TopSearchROI.fptRB
            = Ipvm::Point32r2((fRoughAlignCenterX + (fDir_Ver_HalfSpecSize.m_x * fBody_Search_Width_Ratio)),
                (fTopSearchROICenY + (fBody_Search_LengthOffset * .5f)));

        o_vec2frtChip_SearchROI[UP].push_back(frtDetailAlign_TopSearchROI);
        //}}

        //{{Bottom
        frtDetailAlign_BottomSearchROI = frtDetailAlign_TopSearchROI.Move(0, fDir_Ver_SpecSize.m_y);

        o_vec2frtChip_SearchROI[DOWN].push_back(frtDetailAlign_BottomSearchROI);
        //}}
    }
    else //처리할수 없는 값이면 return
        return false;

    if (m_pPassiveResult_DebugItem)
    {
        m_pPassiveResult_DebugItem->m_detailSearchROI[LEFT] = o_vec2frtChip_SearchROI[LEFT][0]; //Debug
        m_pPassiveResult_DebugItem->m_detailSearchROI[RIGHT] = o_vec2frtChip_SearchROI[RIGHT][0]; //Debug
        m_pPassiveResult_DebugItem->m_detailSearchROI[UP] = o_vec2frtChip_SearchROI[UP][0]; //Debug
        m_pPassiveResult_DebugItem->m_detailSearchROI[DOWN] = o_vec2frtChip_SearchROI[DOWN][0]; //Debug
    }

    for (long nidx = 0; nidx < (long)o_vec2frtChip_SearchROI.size(); nidx++)
        m_pPassiveAlignResult->m_vecfrtDebug_DetailSearchROI.push_back(o_vec2frtChip_SearchROI[nidx][0]);

    return true;
}

bool CPassive2DAlgorithm::DoMIA_DetailAlign(const Ipvm::Image8u& i_OutlineAlign_BodyImage,
    const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI, FPI_RECT i_frtRoughAlignROI,
    long i_nPassiveAngleType)
{
    for (long n = 0; n < (long)i_vec2frtDetailSearchROI.size(); n++)
    {
        if (i_vec2frtDetailSearchROI[n].size() <= 0
            || i_vec2frtDetailSearchROI[n].size() > 1) //없거나 2개이상이면 말도 안되는거다
            return false;
    }

    std::vector<sEdgeSearchNeedInfo> vecsMajorAxisEdgeSearchNeedInfo;
    std::vector<EDGEPOINTS> vecMajorAxisEdgePoints;

    GetMajorEdgeSearchNeedInfo(i_nPassiveAngleType, i_vec2frtDetailSearchROI, vecsMajorAxisEdgeSearchNeedInfo);

    for (long nidx = 0; nidx < vecsMajorAxisEdgeSearchNeedInfo.size(); nidx++)
    {
        EDGEPOINTS CurEdgePoints;
        if (!PassiveBody_FindMajorAxisEdgePoint(i_OutlineAlign_BodyImage, vecsMajorAxisEdgeSearchNeedInfo[nidx],
                i_nPassiveAngleType, i_frtRoughAlignROI, m_pPassiveAlignParam->m_nOutline_align_Body_Search_Count_Gap,
                CurEdgePoints, m_pPassiveAlignParam->m_nOutline_align_Body_Edge_Detect_Mode))
            return false;

        if (CurEdgePoints.size() < 2)
            return false;

        vecMajorAxisEdgePoints.push_back(CurEdgePoints);
    }

    auto& debugEdgePoints = m_pPassiveAlignResult->m_vecfptDebug_DetailSearchEdgePoints;
    for (auto fptEdgePoints : vecMajorAxisEdgePoints)
        debugEdgePoints.insert(debugEdgePoints.end(), fptEdgePoints.begin(), fptEdgePoints.end());

    FPI_RECT frtAlignResult;
    if (!MakeRectForLineProjection(vecMajorAxisEdgePoints, frtAlignResult))
        return false;

    long nResultLength = frtAlignResult.GetCRect().Width();
    long nResultWidth = frtAlignResult.GetCRect().Height();

    if (!ModifyBodyAlignRect(i_nPassiveAngleType, nResultWidth, nResultLength))
        return false;

    m_pPassiveAlignResult->m_chip_resDetailAlign = frtAlignResult; //ComboDebugView
    m_pPassiveAlignResult->m_chip_resFinal = m_pPassiveAlignResult->m_chip_resDetailAlign;

    if (m_pPassiveResult_DebugItem)
        m_pPassiveResult_DebugItem->m_detailAlignResult = frtAlignResult; //Debug

    return true;
}

bool CPassive2DAlgorithm::DoCapacitor_DetailAlign(const Ipvm::Image8u& i_OutlineAlign_ElectrodeImage,
    const Ipvm::Image8u& i_OutlineAlign_BodyImage, const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI,
    FPI_RECT i_frtRoughAlignROI, long i_nPassiveAngleType)
{
    for (long n = 0; n < (long)i_vec2frtDetailSearchROI.size(); n++)
    {
        if (i_vec2frtDetailSearchROI[n].size() <= 0
            || i_vec2frtDetailSearchROI[n].size() > 1) //없거나 2개이상이면 말도 안되는거다
            return false;
    }

    std::vector<sEdgeSearchNeedInfo> vecsMajorAxisEdgeSearchNeedInfo;
    std::vector<EDGEPOINTS> vecMajorAxisEdgePoints;

    GetMajorEdgeSearchNeedInfo(i_nPassiveAngleType, i_vec2frtDetailSearchROI, vecsMajorAxisEdgeSearchNeedInfo);

    for (long nidx = 0; nidx < vecsMajorAxisEdgeSearchNeedInfo.size(); nidx++)
    {
        EDGEPOINTS CurEdgePoints;
        if (!PassiveBody_FindMajorAxisEdgePoint(i_OutlineAlign_BodyImage, vecsMajorAxisEdgeSearchNeedInfo[nidx],
                i_nPassiveAngleType, i_frtRoughAlignROI, m_pPassiveAlignParam->m_nOutline_align_Body_Search_Count_Gap,
                CurEdgePoints, m_pPassiveAlignParam->m_nOutline_align_Body_Edge_Detect_Mode))
            return false;

        if (CurEdgePoints.size() < 2)
            return false;

        vecMajorAxisEdgePoints.push_back(CurEdgePoints);
    }

    m_pPassiveAlignResult->m_fRotateDegree
        = GetRotateDegree(m_pPassiveInfoDB->fPassiveAngle, vecMajorAxisEdgePoints[0], vecMajorAxisEdgePoints[1]);

    auto& debugEdgePoints = m_pPassiveAlignResult->m_vecfptDebug_DetailSearchEdgePoints;
    debugEdgePoints.insert(debugEdgePoints.end(), vecMajorAxisEdgePoints[0].begin(), vecMajorAxisEdgePoints[0].end());
    debugEdgePoints.insert(debugEdgePoints.end(), vecMajorAxisEdgePoints[1].begin(), vecMajorAxisEdgePoints[1].end());

    //mc_RoughAlign 결과를 믿는다
    Ipvm::LineEq32r RefLine1, RefLine2;
    CreateRefLine_UseFindMinorEdgePoint(i_nPassiveAngleType, i_frtRoughAlignROI, RefLine1, RefLine2);

    EDGEPOINTS vecMinorAxisEdgePoints;
    std::vector<long> vecnMinorSearchDirection = GetEdgeSearchSDirectionType(i_nPassiveAngleType, true);
    if (!PassiveBody_FindMinorAxisEdgePoint(i_OutlineAlign_ElectrodeImage, RefLine1, RefLine2, i_vec2frtDetailSearchROI,
            PI_ED_DIR_FALLING, i_nPassiveAngleType, vecnMinorSearchDirection[0], vecnMinorSearchDirection[1],
            m_pPassiveAlignParam->m_nOutline_align_Electrode_EdgeSearch_Direction, vecMinorAxisEdgePoints,
            m_pPassiveAlignParam->m_nOutline_align_Electrode_Edge_Detect_Mode))
        return false;

    FPI_RECT frtAlignResult;
    MakeRectForPointProjection(i_nPassiveAngleType, vecMajorAxisEdgePoints[0], vecMajorAxisEdgePoints[1],
        vecMinorAxisEdgePoints[0], vecMinorAxisEdgePoints[1], frtAlignResult);

    long nResultLength = frtAlignResult.GetCRect().Width();
    long nResultWidth = frtAlignResult.GetCRect().Height();

    if (!ModifyBodyAlignRect(i_nPassiveAngleType, nResultWidth, nResultLength))
        return false;

    m_pPassiveAlignResult->m_chip_resDetailAlign = frtAlignResult; //ComboDebugView
    m_pPassiveAlignResult->m_chip_resFinal = m_pPassiveAlignResult->m_chip_resDetailAlign;

    if (m_pPassiveResult_DebugItem)
    {
        m_pPassiveResult_DebugItem->m_detailAlignResult = frtAlignResult; //Debug
        switch (i_nPassiveAngleType)
        {
            case Passive_Horizontal:
                m_pPassiveResult_DebugItem->m_detailAlignResult_Horizontal = frtAlignResult;
                break;

            case Passive_Vertical:
                m_pPassiveResult_DebugItem->m_detailAlignResult_Vertical = frtAlignResult;
                break;
        }
    }

    return true;
}

bool CPassive2DAlgorithm::PassiveBody_FindMajorAxisEdgePoint(const Ipvm::Image8u i_OutlineAlign_BodyImage,
    sEdgeSearchNeedInfo i_sEdgeSearchNeedInfo, long i_nPaasiveAngleType, FPI_RECT i_frtSpecROI,
    long i_nMinimum_EdgePointCount, EDGEPOINTS& o_vecfptEdgePoint, BOOL i_bFristEdgeDeteceMode)
{
    if (i_OutlineAlign_BodyImage.GetMem() == nullptr)
        return false;

    o_vecfptEdgePoint.clear();
    float fOldEdgeThreshold
        = m_pEdgeDetect->SetMininumThreshold((float)m_pPassiveAlignParam->m_fOutline_align_Body_EdgeThreshold);

    Ipvm::Point32r2 fPointXY(0.f, 0.f);
    Ipvm::Point32s2 ptSearchStartPos(0, 0);
    long nSearchWdith = max(i_sEdgeSearchNeedInfo.nSearchWidth, 1); //Setup을 그지같이 하면 SearchWidth에 0이 들어온다.

    for (long n = 0; n < i_sEdgeSearchNeedInfo.nROISize; n += nSearchWdith)
    {
        switch (i_sEdgeSearchNeedInfo.nSearchDirection)
        {
            case UP:
            case DOWN:
                ptSearchStartPos = Ipvm::Point32s2(
                    i_sEdgeSearchNeedInfo.ptSearchStartPos.m_x + n, i_sEdgeSearchNeedInfo.ptSearchStartPos.m_y);
                break;
            case LEFT:
            case RIGHT:
                ptSearchStartPos = Ipvm::Point32s2(
                    i_sEdgeSearchNeedInfo.ptSearchStartPos.m_x, i_sEdgeSearchNeedInfo.ptSearchStartPos.m_y + n);
                break;
        }

        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(i_sEdgeSearchNeedInfo.nSearchType, ptSearchStartPos,
                i_sEdgeSearchNeedInfo.nSearchDirection, i_sEdgeSearchNeedInfo.nSearchLength,
                i_sEdgeSearchNeedInfo.nSearchWidth, i_OutlineAlign_BodyImage, fPointXY, i_bFristEdgeDeteceMode))
        {
            if (o_vecfptEdgePoint.size() == i_nMinimum_EdgePointCount)
                break;

            o_vecfptEdgePoint.push_back(fPointXY);
        }
    }
    m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);

    long nMinimum_EdgePointCount = (long)((i_nMinimum_EdgePointCount * .5f) + .5f); //최소갯수
    if (o_vecfptEdgePoint.size() < nMinimum_EdgePointCount)
        return FALSE;

    float fAveragePos = GetAveragePoint(o_vecfptEdgePoint, i_nPaasiveAngleType);

    Ipvm::LineSeg32r fSpecRefLine(0.f, 0.f, 0.f, 0.f);
    switch (i_sEdgeSearchNeedInfo.nOriginSearchDirection)
    {
        case LEFT:
            fSpecRefLine = Ipvm::LineSeg32r(fAveragePos, i_frtSpecROI.fptLT.m_y, fAveragePos, i_frtSpecROI.fptLB.m_y);
            break;
        case UP:
            fSpecRefLine = Ipvm::LineSeg32r(i_frtSpecROI.fptLT.m_x, fAveragePos, i_frtSpecROI.fptRT.m_x, fAveragePos);
            break;
        case RIGHT:
            fSpecRefLine = Ipvm::LineSeg32r(fAveragePos, i_frtSpecROI.fptRT.m_y, fAveragePos, i_frtSpecROI.fptRB.m_y);
            break;
        case DOWN:
            fSpecRefLine = Ipvm::LineSeg32r(i_frtSpecROI.fptLB.m_x, fAveragePos, i_frtSpecROI.fptRB.m_x, fAveragePos);
            break;
    }

    RemoveNoiseEdgePoint(o_vecfptEdgePoint, i_nPaasiveAngleType, fSpecRefLine); //Nosie Edge Filtering

    //{{Debug
    if (m_pPassiveResult_DebugItem)
    {
        m_pPassiveResult_DebugItem->m_specLines.push_back(fSpecRefLine);

        for (auto EdgePoints : o_vecfptEdgePoint)
        {
            // Debug
            m_pPassiveResult_DebugItem->m_debugEdgePoints[i_sEdgeSearchNeedInfo.nOriginSearchDirection].push_back(
                EdgePoints);
        }
    }
    //}}
    return true;
}

float CPassive2DAlgorithm::GetAveragePoint(const EDGEPOINTS& i_vecEdgePoint, long i_nPassiveAngleType)
{
    if (i_vecEdgePoint.size() <= 0)
        return 0.f;

    float fSumValue(0.f);
    switch (i_nPassiveAngleType)
    {
        case Passive_Vertical:
            for (auto XPosValue : i_vecEdgePoint)
                fSumValue += XPosValue.m_x;
            break;
        case Passive_Horizontal:
            for (auto YPosValue : i_vecEdgePoint)
                fSumValue += YPosValue.m_y;
            break;
        default:
            break;
    }

    return fSumValue / (long)i_vecEdgePoint.size();
}

bool CPassive2DAlgorithm::RemoveNoiseEdgePoint(
    EDGEPOINTS& io_vecEdgePoint, long i_nPassiveAngleType, Ipvm::LineSeg32r i_SpecRefLine)
{
    if (io_vecEdgePoint.size() < 2) //EdgePoint가 2개이하면 할 필요없다. LineFitting을 못하니까
        return false;

    //1. Spec상의 Line의 Center와 거리를 구함
    EDGEPOINTS vecfptDistValueForSpecLine;
    for (auto FirstFittingEdgePoint : io_vecEdgePoint)
    {
        Ipvm::Point32r2 fptDistValue(i_SpecRefLine.CenterPoint().m_x - FirstFittingEdgePoint.m_x,
            i_SpecRefLine.CenterPoint().m_y - FirstFittingEdgePoint.m_y);
        vecfptDistValueForSpecLine.push_back(fptDistValue);
    }

    //2. 분포도를 측정하기 위해 Min,Max Range 계산
    std::vector<float> vecfRefValueIndex; // 최종Index의 결과를 알아야 함으로 가지고있어야한다.
    float fSearchMax(0.f), fSearchMin(0.f);
    for (long n = 0; n < vecfptDistValueForSpecLine.size(); n++)
    {
        float fRefValue(0.f);
        //Type 별 분기
        switch (i_nPassiveAngleType)
        {
            case Passive_Vertical: //X축
                fRefValue = vecfptDistValueForSpecLine[n].m_x;
                break;
            case Passive_Horizontal: //Y축
                fRefValue = vecfptDistValueForSpecLine[n].m_y;
                break;
            default:
                break;
        }

        if (n == 0)
        {
            fSearchMax = fRefValue;
            fSearchMin = fRefValue;
        }
        else
        {
            fSearchMax = max(fSearchMax, fRefValue);
            fSearchMin = min(fSearchMin, fRefValue);
        }

        vecfRefValueIndex.push_back(fRefValue);
    }

    if ((fSearchMin + 1.f) >= fSearchMax) //Type정의할께 없다면 return하는게 맞지
        return false;

    //3. Min ~ Max 까지 1.0단위로 Type을 정의
    std::vector<float> vecfValueType;
    for (float fMainType = fSearchMin; fMainType <= fSearchMax; fMainType += 1.f)
    {
        if (fMainType > fSearchMax) //증가한 Type이 Max보다 크다면 Max를 넣어줌
            fMainType = fSearchMax;

        vecfValueType.push_back(fMainType);
    }

    if (vecfValueType.size() <= 2) //Type이 2개이상은 나와야지 비교하는 의미가 있다.
        return false;

    //4. 분류된 Type에 따라 조건을 만족하는 Index 저장(분포도를 구함)
    std::vector<std::vector<long>> vec2nDistributionchart;
    vec2nDistributionchart.resize(vecfValueType.size());
    for (long nValueTypeIndex = 0; nValueTypeIndex < vecfValueType.size(); nValueTypeIndex++)
    {
        float fMinimumValue = vecfValueType[nValueTypeIndex];
        float fMaximumValue(0.f);
        if (nValueTypeIndex == vecfValueType.size() - 1)
            fMaximumValue = fSearchMax;
        else
            fMaximumValue = vecfValueType[nValueTypeIndex + 1];

        for (long nValueIndex = 0; nValueIndex < vecfRefValueIndex.size(); nValueIndex++)
        {
            if (vecfRefValueIndex[nValueIndex] > fMinimumValue && vecfRefValueIndex[nValueIndex] < fMaximumValue)
                vec2nDistributionchart[nValueTypeIndex].push_back(nValueIndex);
        }
    }

    //5. 분포가 가장 많은 Index Find
    long nMaximumSizeIndex(0);
    long nCurSize(0), nMaxSize(0);
    for (long n = 0; n < vec2nDistributionchart.size(); n++)
    {
        nCurSize = (long)vec2nDistributionchart[n].size();
        if (nCurSize > nMaxSize)
        {
            nMaxSize = nCurSize;
            nMaximumSizeIndex = n;
        }
    }

    //6. Buff를 이용하여 Index의 값을 복사
    EDGEPOINTS vecfptBuffEdgePoints;
    for (auto MaxDistribution : vec2nDistributionchart[nMaximumSizeIndex])
        vecfptBuffEdgePoints.push_back(io_vecEdgePoint[MaxDistribution]);

    //7. 기존의 EdgePoint vector를 Clear하고 Buff에 할당된값 복사
    io_vecEdgePoint.clear();
    for (auto FinalEdgePoint : vecfptBuffEdgePoints)
        io_vecEdgePoint.push_back(FinalEdgePoint);

    return true;
}

float CPassive2DAlgorithm::GetRotateDegree(
    float i_fSpecAngle_Degree, const EDGEPOINTS& i_vecfptEdgePonts1, const EDGEPOINTS& i_vecfptEdgePonts2)
{
    Ipvm::LineEq32r CalcuationLine1(0.f, 0.f, 0.f), CalcuationLine2(0.f, 0.f, 0.f);

    CPI_Geometry::LineFitting_RemoveNoise(i_vecfptEdgePonts1, CalcuationLine1);
    CPI_Geometry::LineFitting_RemoveNoise(i_vecfptEdgePonts2, CalcuationLine2);

    Ipvm::LineEq32r CalcuationLine_Center(0.f, 0.f, 0.f);
    Ipvm::Geometry::GetBisection(CalcuationLine1, CalcuationLine2, CalcuationLine_Center);

    float fRadianLineY = -(CalcuationLine_Center.m_a / CalcuationLine_Center.m_b);
    float fRadian = CAST_FLOAT(atan(fRadianLineY));

    float fSpecAngle_Degree = fmodf(i_fSpecAngle_Degree, 180.f);
    float fCalcAngle_Degree = (fRadian * 180) / PI;

    float fReturnValue_Degree(0.f);
    if (fCalcAngle_Degree < 0)
        fReturnValue_Degree = fSpecAngle_Degree + fCalcAngle_Degree;
    else if (fCalcAngle_Degree > 0)
        fReturnValue_Degree = fSpecAngle_Degree - fCalcAngle_Degree;
    else //어떤상황에서 일어나는지 모르겠다
        fReturnValue_Degree = Ipvm::k_noiseValue32r;

    return fReturnValue_Degree;
}

void CPassive2DAlgorithm::CreateRefLine_UseFindMinorEdgePoint(long i_nPassiveType, FPI_RECT i_frtRoughAlign_Result,
    Ipvm::LineEq32r& o_line_Ref1_RoughAlignPoint, Ipvm::LineEq32r& o_line_Ref2_RoughAlignPoint)
{
    EDGEPOINTS vecfptRefPoint1_byRoughAlign;
    EDGEPOINTS vecfptRefPoint2_byRoughAlign;

    if (i_nPassiveType == Passive_Horizontal)
    {
        vecfptRefPoint1_byRoughAlign.push_back(i_frtRoughAlign_Result.fptLT);
        vecfptRefPoint1_byRoughAlign.push_back(i_frtRoughAlign_Result.fptRT);
        vecfptRefPoint2_byRoughAlign.push_back(i_frtRoughAlign_Result.fptLB);
        vecfptRefPoint2_byRoughAlign.push_back(i_frtRoughAlign_Result.fptRB);
    }
    else if (i_nPassiveType == Passive_Vertical)
    {
        vecfptRefPoint1_byRoughAlign.push_back(i_frtRoughAlign_Result.fptLT);
        vecfptRefPoint1_byRoughAlign.push_back(i_frtRoughAlign_Result.fptLB);
        vecfptRefPoint2_byRoughAlign.push_back(i_frtRoughAlign_Result.fptRT);
        vecfptRefPoint2_byRoughAlign.push_back(i_frtRoughAlign_Result.fptRB);
    }

    CPI_Geometry::LineFitting_RemoveNoise(vecfptRefPoint1_byRoughAlign, o_line_Ref1_RoughAlignPoint);
    CPI_Geometry::LineFitting_RemoveNoise(vecfptRefPoint2_byRoughAlign, o_line_Ref2_RoughAlignPoint);
}

bool CPassive2DAlgorithm::PassiveBody_FindMinorAxisEdgePoint(const Ipvm::Image8u i_OutlineAlign_ElectrodeImage,
    Ipvm::LineEq32r& i_line_Ref1, Ipvm::LineEq32r& i_line_Ref2,
    const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI, long i_nSearchType, long i_nPassiveDirection,
    long i_nSearchDirection1, long i_nSearchDirection2, long i_nSearchDirection,
    EDGEPOINTS& o_vecfptMinorAxisEdgePoints, BOOL i_bFristEdgeDeteceMode)
{
    long nEdgeSearchDir(-1);

    Ipvm::Point32r2 fptDir1_Ref1_CenterLine(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(
        i_line_Ref1, i_vec2frtDetailSearchROI[i_nSearchDirection1][0].GetCenter(), fptDir1_Ref1_CenterLine);

    Ipvm::Point32r2 fptDir1_Ref2_CenterLine(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(
        i_line_Ref2, i_vec2frtDetailSearchROI[i_nSearchDirection1][0].GetCenter(), fptDir1_Ref2_CenterLine);

    Ipvm::Point32r2 fptDir1_CenterPoint(0.f, 0.f);
    fptDir1_CenterPoint.m_x = (fptDir1_Ref1_CenterLine.m_x + fptDir1_Ref2_CenterLine.m_x) * .5f;
    fptDir1_CenterPoint.m_y = (fptDir1_Ref1_CenterLine.m_y + fptDir1_Ref2_CenterLine.m_y) * .5f;

    Ipvm::Point32r2 fDir1_SearchSize
        = Ipvm::Point32r2(i_vec2frtDetailSearchROI[i_nSearchDirection1][0].GetFRect().Width(),
            i_vec2frtDetailSearchROI[i_nSearchDirection1][0].GetFRect().Height());

    std::vector<Ipvm::Rect32s> vecrtDir1_SearchROI;
    if (!CreateMinor_EdgeDetectROI(fptDir1_CenterPoint, fptDir1_Ref1_CenterLine, fptDir1_Ref2_CenterLine,
            fDir1_SearchSize //이함수 디버깅 해볼것 이상하다
            ,
            i_nPassiveDirection, vecrtDir1_SearchROI))
        return false;

    if ((vecrtDir1_SearchROI[0] & Ipvm::Rect32s(i_OutlineAlign_ElectrodeImage)) != vecrtDir1_SearchROI[0])
        return false;
    if ((vecrtDir1_SearchROI[1] & Ipvm::Rect32s(i_OutlineAlign_ElectrodeImage)) != vecrtDir1_SearchROI[1])
        return false;

    for (auto rtDir1_SearchShortenROI : vecrtDir1_SearchROI)
        m_pPassiveAlignResult->m_vecrtShortenROI.push_back(rtDir1_SearchShortenROI);

    if (i_nSearchDirection == PI_ED_DIR_OUTER)
        nEdgeSearchDir = i_nSearchDirection1;
    else
        nEdgeSearchDir = i_nSearchDirection2;

    EDGEPOINTS vecfptLeftEdgePoint;

    Ipvm::Point32r2 fptDir1_EdgePoint(0.f, 0.f);
    if (!MinorDetectEdgePoint(i_OutlineAlign_ElectrodeImage, i_nPassiveDirection, i_nSearchType, nEdgeSearchDir,
            vecrtDir1_SearchROI, fptDir1_EdgePoint, i_bFristEdgeDeteceMode))
        return false;

    Ipvm::Point32r2 fptDir2_Ref1_CenterLine(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(
        i_line_Ref1, i_vec2frtDetailSearchROI[i_nSearchDirection2][0].GetCenter(), fptDir2_Ref1_CenterLine);

    Ipvm::Point32r2 fptDir2_Ref2_CenterLine(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(
        i_line_Ref2, i_vec2frtDetailSearchROI[i_nSearchDirection2][0].GetCenter(), fptDir2_Ref2_CenterLine);

    Ipvm::Point32r2 fptDir2_CenterPoint(0.f, 0.f);
    fptDir2_CenterPoint.m_x = (fptDir2_Ref1_CenterLine.m_x + fptDir2_Ref2_CenterLine.m_x) * .5f;
    fptDir2_CenterPoint.m_y = (fptDir2_Ref1_CenterLine.m_y + fptDir2_Ref2_CenterLine.m_y) * .5f;

    Ipvm::Point32r2 fDir2_SearchSize
        = Ipvm::Point32r2(i_vec2frtDetailSearchROI[i_nSearchDirection2][0].GetFRect().Width(),
            i_vec2frtDetailSearchROI[i_nSearchDirection2][0].GetFRect().Height());

    std::vector<Ipvm::Rect32s> vecrtDIr2_SearchROI;
    if (!CreateMinor_EdgeDetectROI(fptDir2_CenterPoint, fptDir2_Ref1_CenterLine, fptDir2_Ref2_CenterLine,
            fDir2_SearchSize, i_nPassiveDirection, vecrtDIr2_SearchROI))
        return false;

    if ((vecrtDIr2_SearchROI[0] & Ipvm::Rect32s(i_OutlineAlign_ElectrodeImage)) != vecrtDIr2_SearchROI[0])
        return false;
    if ((vecrtDIr2_SearchROI[1] & Ipvm::Rect32s(i_OutlineAlign_ElectrodeImage)) != vecrtDIr2_SearchROI[1])
        return false;

    for (auto rtDir2_SearchShortenROI : vecrtDIr2_SearchROI)
        m_pPassiveAlignResult->m_vecrtShortenROI.push_back(rtDir2_SearchShortenROI);

    if (i_nSearchDirection == PI_ED_DIR_OUTER)
        nEdgeSearchDir = i_nSearchDirection2;
    else
        nEdgeSearchDir = i_nSearchDirection1;

    EDGEPOINTS vecfptRightEdgePoint;

    Ipvm::Point32r2 fptDir2_EdgePoint(0.f, 0.f);
    if (!MinorDetectEdgePoint(i_OutlineAlign_ElectrodeImage, i_nPassiveDirection, i_nSearchType, nEdgeSearchDir,
            vecrtDIr2_SearchROI, fptDir2_EdgePoint, i_bFristEdgeDeteceMode))
        return false;

    auto& debugEdgePoints = m_pPassiveAlignResult->m_vecfptDebug_DetailSearchEdgePoints;
    debugEdgePoints.push_back(fptDir1_EdgePoint);
    debugEdgePoints.push_back(fptDir2_EdgePoint);

    o_vecfptMinorAxisEdgePoints.push_back(fptDir1_EdgePoint);
    o_vecfptMinorAxisEdgePoints.push_back(fptDir2_EdgePoint);

    return true;
}

bool CPassive2DAlgorithm::CreateMinor_EdgeDetectROI(Ipvm::Point32r2 i_fptCenterPointBy2Line,
    Ipvm::Point32r2 i_fptF_LineCenter, Ipvm::Point32r2 i_fptS_LineCenter, Ipvm::Point32r2 i_fptSearchROISize,
    long i_nPassiveAngleType, std::vector<Ipvm::Rect32s>& o_vecrtSearchROI)
{
    if (i_fptCenterPointBy2Line.m_x <= 0 || i_fptCenterPointBy2Line.m_y <= 0) //0보다 작으면 말이 안되지
        return false;

    o_vecrtSearchROI.clear();
    o_vecrtSearchROI.resize(2);

    float fSearchROIHalfWidth = i_fptSearchROISize.m_x * .5f;
    float fSearchROIHalfHeight = i_fptSearchROISize.m_y * .5f;

    if (i_nPassiveAngleType == Passive_Horizontal)
    {
        o_vecrtSearchROI[0].m_left = (long)(i_fptCenterPointBy2Line.m_x - fSearchROIHalfWidth);
        o_vecrtSearchROI[0].m_right = (long)(i_fptCenterPointBy2Line.m_x + fSearchROIHalfWidth);
        o_vecrtSearchROI[0].m_top = (long)(i_fptF_LineCenter.m_y - fSearchROIHalfHeight);
        o_vecrtSearchROI[0].m_bottom = (long)(i_fptF_LineCenter.m_y + fSearchROIHalfHeight);

        o_vecrtSearchROI[1].m_left = (long)(i_fptCenterPointBy2Line.m_x - fSearchROIHalfWidth);
        o_vecrtSearchROI[1].m_right = (long)(i_fptCenterPointBy2Line.m_x + fSearchROIHalfWidth);
        o_vecrtSearchROI[1].m_top = (long)(i_fptS_LineCenter.m_y - fSearchROIHalfHeight);
        o_vecrtSearchROI[1].m_bottom = (long)(i_fptS_LineCenter.m_y + fSearchROIHalfHeight);

        long nOffsetY = o_vecrtSearchROI[0].Height() / 4;

        o_vecrtSearchROI[0].OffsetRect(0, nOffsetY);
        o_vecrtSearchROI[1].OffsetRect(0, -nOffsetY);
    }
    else if (i_nPassiveAngleType == Passive_Vertical)
    {
        o_vecrtSearchROI[0].m_left = (long)(i_fptF_LineCenter.m_x - fSearchROIHalfWidth);
        o_vecrtSearchROI[0].m_right = (long)(i_fptF_LineCenter.m_x + fSearchROIHalfWidth);
        o_vecrtSearchROI[0].m_top = (long)(i_fptCenterPointBy2Line.m_y - fSearchROIHalfHeight);
        o_vecrtSearchROI[0].m_bottom = (long)(i_fptCenterPointBy2Line.m_y + fSearchROIHalfHeight);

        o_vecrtSearchROI[1].m_left = (long)(i_fptS_LineCenter.m_x - fSearchROIHalfWidth);
        o_vecrtSearchROI[1].m_right = (long)(i_fptS_LineCenter.m_x + fSearchROIHalfWidth);
        o_vecrtSearchROI[1].m_top = (long)(i_fptCenterPointBy2Line.m_y - fSearchROIHalfHeight);
        o_vecrtSearchROI[1].m_bottom = (long)(i_fptCenterPointBy2Line.m_y + fSearchROIHalfHeight);

        long nOffsetX = o_vecrtSearchROI[0].Width() / 4;

        o_vecrtSearchROI[0].OffsetRect(nOffsetX, 0);
        o_vecrtSearchROI[1].OffsetRect(-nOffsetX, 0);
    }

    if (m_pPassiveResult_DebugItem)
    {
        m_pPassiveResult_DebugItem->m_shortenSearchROIs.push_back(o_vecrtSearchROI[0]);
        m_pPassiveResult_DebugItem->m_shortenSearchROIs.push_back(o_vecrtSearchROI[1]);
    }

    return TRUE;
}

bool CPassive2DAlgorithm::MinorDetectEdgePoint(const Ipvm::Image8u& i_OutlineAlign_ElectrodeImage,
    long i_nPassiveAngleType, long i_nSearchType, long i_nEdgeSearchDir,
    const std::vector<Ipvm::Rect32s>& i_vecrtShortenSearchROI, Ipvm::Point32r2& o_fptMinoraxisEdgePoint,
    BOOL i_bFristEdgeDeteceMode)
{
    if (i_OutlineAlign_ElectrodeImage.GetMem() == nullptr)
        return false;

    float fOldEdgeThreshold
        = m_pEdgeDetect->SetMininumThreshold((float)m_pPassiveAlignParam->m_fOutline_align_Electrode_EdgeThreshold);

    EDGEPOINTS vecfptMinoraxisEdge;
    Ipvm::Point32s2 ptSearchStartPos(0, 0);

    for (auto rtShortenROI : i_vecrtShortenSearchROI)
    {
        Ipvm::Point32r2 fMainPointXY(0.f, 0.f);
        long nEdgeSearchWidth(0), nEdgeSearchLength(0);

        switch (i_nEdgeSearchDir)
        {
            case LEFT:
                ptSearchStartPos
                    = Ipvm::Point32s2(rtShortenROI.m_right, (rtShortenROI.m_top + rtShortenROI.m_bottom) / 2);
                break;
            case UP:
                ptSearchStartPos
                    = Ipvm::Point32s2((rtShortenROI.m_left + rtShortenROI.m_right) / 2, rtShortenROI.m_bottom);
                break;
            case RIGHT:
                ptSearchStartPos
                    = Ipvm::Point32s2(rtShortenROI.m_left, (rtShortenROI.m_top + rtShortenROI.m_bottom) / 2);
                break;
            case DOWN:
                ptSearchStartPos
                    = Ipvm::Point32s2((rtShortenROI.m_left + rtShortenROI.m_right) / 2, rtShortenROI.m_top);
                break;
        }

        switch (i_nPassiveAngleType)
        {
            case Passive_Horizontal:
                nEdgeSearchWidth = rtShortenROI.Height();
                nEdgeSearchLength = rtShortenROI.Width();
                //nEdgeSearchWidth	= long(rtShortenROI.Height() * 1.5f);
                //nEdgeSearchLength	= long(rtShortenROI.Width() * .7f);
                break;
            case Passive_Vertical:
                nEdgeSearchWidth = rtShortenROI.Width();
                nEdgeSearchLength = rtShortenROI.Height();
                /*nEdgeSearchWidth = long(rtShortenROI.Width() * 1.5f);
			nEdgeSearchLength = long(rtShortenROI.Height() * .7f);*/
                break;
            default:
                break;
        }

        if (m_pEdgeDetect->PI_ED_CalcAvgEdgePointFromImage(i_nSearchType, ptSearchStartPos, i_nEdgeSearchDir,
                nEdgeSearchLength, nEdgeSearchWidth, i_OutlineAlign_ElectrodeImage, fMainPointXY,
                i_bFristEdgeDeteceMode))
        {
            if (m_pPassiveResult_DebugItem)
            {
                m_pPassiveResult_DebugItem->m_debugEdgePoints[i_nEdgeSearchDir].push_back(fMainPointXY);
            }
            vecfptMinoraxisEdge.push_back(fMainPointXY);
        }
    }
    m_pEdgeDetect->SetMininumThreshold(fOldEdgeThreshold);

    o_fptMinoraxisEdgePoint = GetMinorEdgePoint(vecfptMinoraxisEdge);

    for (auto fptMinoraxisEdgePoint : vecfptMinoraxisEdge)
        m_pPassiveAlignResult->m_vecfptMinoraxisEdgePoint.push_back(fptMinoraxisEdgePoint); //mc_DebugView

    return TRUE;
}

Ipvm::Point32r2 CPassive2DAlgorithm::GetMinorEdgePoint(const EDGEPOINTS& i_vecfptEdgePoint)
{
    Ipvm::Point32r2 fptSearchEdgePoint(0.f, 0.f), fptreturnEdgePoint(0.f, 0.f);
    for (auto EdgePoint : i_vecfptEdgePoint)
    {
        fptSearchEdgePoint.m_x += EdgePoint.m_x;
        fptSearchEdgePoint.m_y += EdgePoint.m_y;
    }

    if (i_vecfptEdgePoint.size() > 1)
        fptreturnEdgePoint = Ipvm::Point32r2((fptSearchEdgePoint.m_x * .5f), (fptSearchEdgePoint.m_y * .5f));
    else
        fptreturnEdgePoint = Ipvm::Point32r2((fptSearchEdgePoint.m_x), (fptSearchEdgePoint.m_y));

    return fptreturnEdgePoint;
}

bool CPassive2DAlgorithm::MakeRectForPointProjection(long i_nPassiveAngleType, const EDGEPOINTS& i_vecfptMajorPoints1,
    const EDGEPOINTS& i_vecfptMajorPoints2, Ipvm::Point32r2 i_fptMinorAxisPoint1, Ipvm::Point32r2 i_fptMinorAxisPoint2,
    FPI_RECT& o_frtBodyAlignResult)
{
    Ipvm::LineEq32r lineRef1_Major, lineRef2_Major;

    CPI_Geometry::LineFitting_RemoveNoise(i_vecfptMajorPoints1, lineRef1_Major);
    CPI_Geometry::LineFitting_RemoveNoise(i_vecfptMajorPoints2, lineRef2_Major);

    Ipvm::Point32r2 fptLT(0.f, 0.f), fptRT(0.f, 0.f), fptLB(0.f, 0.f), fptRB(0.f, 0.f);

    switch (i_nPassiveAngleType)
    {
        case Passive_Horizontal:
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef1_Major, i_fptMinorAxisPoint1, fptLT);
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef2_Major, i_fptMinorAxisPoint1, fptLB);
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef1_Major, i_fptMinorAxisPoint2, fptRT);
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef2_Major, i_fptMinorAxisPoint2, fptRB);
            break;
        case Passive_Vertical:
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef1_Major, i_fptMinorAxisPoint1, fptLT);
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef2_Major, i_fptMinorAxisPoint1, fptRT);
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef1_Major, i_fptMinorAxisPoint2, fptLB);
            Ipvm::Geometry::GetFootOfPerpendicular(lineRef2_Major, i_fptMinorAxisPoint2, fptRB);
            break;
        default:
            break;
    }

    o_frtBodyAlignResult.fptLT = fptLT;
    o_frtBodyAlignResult.fptRT = fptRT;
    o_frtBodyAlignResult.fptLB = fptLB;
    o_frtBodyAlignResult.fptRB = fptRB;

    return true;
}

bool CPassive2DAlgorithm::MakeRectForLineProjection(
    const std::vector<EDGEPOINTS>& i_vecfptMajorPoints, FPI_RECT& o_frtBodyAlignResult)
{
    if (i_vecfptMajorPoints.size() <= 0) //BUG: 크기 조건 실수
        return false;

    std::vector<Ipvm::Point32r2> vec_UP_DirectionMajorPoints = i_vecfptMajorPoints[UP];
    std::vector<Ipvm::Point32r2> vec_DOWN_DirectionMajorPoints = i_vecfptMajorPoints[DOWN];
    std::vector<Ipvm::Point32r2> vec_LEFT_DirectionMajorPoints = i_vecfptMajorPoints[LEFT];
    std::vector<Ipvm::Point32r2> vec_RIGHT_DirectionMajorPoints = i_vecfptMajorPoints[RIGHT];

    Ipvm::LineEq32r lineDirection[LOWTOP];

    if (!CPI_Geometry::LineFitting_RemoveNoise(i_vecfptMajorPoints[UP], lineDirection[UP])
        || !CPI_Geometry::LineFitting_RemoveNoise(i_vecfptMajorPoints[DOWN], lineDirection[DOWN])
        || !CPI_Geometry::LineFitting_RemoveNoise(i_vecfptMajorPoints[LEFT], lineDirection[LEFT])
        || !CPI_Geometry::LineFitting_RemoveNoise(i_vecfptMajorPoints[RIGHT], lineDirection[RIGHT]))
    {
        return false;
    }

    // 직선 방정식으로부터 바디 센터 라인을 계산한다.
    Ipvm::Geometry::GetBisection(lineDirection[LEFT], lineDirection[RIGHT], lineDirection[VER_CENTER_LINE]);
    Ipvm::Geometry::GetBisection(lineDirection[UP], lineDirection[DOWN], lineDirection[HOR_CENTER_LINE]);

    Ipvm::Point32r2 fptLeftTop(0.f, 0.f);
    Ipvm::Point32r2 fptRightTop(0.f, 0.f);
    Ipvm::Point32r2 fptRightBottom(0.f, 0.f);
    Ipvm::Point32r2 fptLeftBottom(0.f, 0.f);

    if (Ipvm::Geometry::GetCrossPoint(lineDirection[LEFT], lineDirection[UP], fptLeftTop) != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::Geometry::GetCrossPoint(lineDirection[UP], lineDirection[RIGHT], fptRightTop) != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::Geometry::GetCrossPoint(lineDirection[RIGHT], lineDirection[DOWN], fptRightBottom) != Ipvm::Status::e_ok)
        return false;
    if (Ipvm::Geometry::GetCrossPoint(lineDirection[DOWN], lineDirection[LEFT], fptLeftBottom) != Ipvm::Status::e_ok)
        return false;

    // 네 모서리로부터 바디 앵글을 얻는다.
    const float angleLower
        = (float)atan2(fptRightBottom.m_y - fptLeftBottom.m_y, fptRightBottom.m_x - fptLeftBottom.m_x);
    const float angleUpper = (float)atan2(fptRightTop.m_y - fptLeftTop.m_y, fptRightTop.m_x - fptLeftTop.m_x);
    float fTopBottomAngle = (angleLower + angleUpper) * 0.5f;

    float fAngleLeft = CAST_FLOAT(
        atan2(fptLeftBottom.m_y - fptLeftTop.m_y, fptLeftBottom.m_x - fptLeftTop.m_x) - (90. * 0.017453292519943f));
    float fAngleRight = CAST_FLOAT(
        atan2(fptRightBottom.m_y - fptRightTop.m_y, fptRightBottom.m_x - fptRightTop.m_x) - (90. * 0.017453292519943f));
    float fLeftRightAngle = (fAngleLeft + fAngleRight) / 2.f;

    m_pPassiveAlignResult->m_fRotateDegree = (fTopBottomAngle + fLeftRightAngle) * 0.5f;

    o_frtBodyAlignResult.fptLT = fptLeftTop;
    o_frtBodyAlignResult.fptRT = fptRightTop;
    o_frtBodyAlignResult.fptLB = fptLeftBottom;
    o_frtBodyAlignResult.fptRB = fptRightBottom;

    return true;
}

bool CPassive2DAlgorithm::ModifyBodyAlignRect(long i_nPassiveAngleType, long i_nResultWidth, long i_nResultLength)
{
    long nSpecWidth(0), nSpecLength(0);
    switch (i_nPassiveAngleType)
    {
        case Passive_Horizontal:
            nSpecWidth = (long)m_pPassiveAlignResult->m_chip_spec.Height();
            nSpecLength = (long)m_pPassiveAlignResult->m_chip_spec.Width();
            break;
        case Passive_Vertical:
            nSpecWidth = (long)m_pPassiveAlignResult->m_chip_spec.Width();
            nSpecLength = (long)m_pPassiveAlignResult->m_chip_spec.Height();
            break;
        case Passive_MIA:
            return true;
            break;
    }

    long nAlignedWidth(0), nAlignedLength(0);
    if (nSpecLength > nSpecWidth)
    {
        nAlignedWidth = (long)min(i_nResultWidth, i_nResultLength);
        nAlignedLength = (long)max(i_nResultWidth, i_nResultLength);
    }
    else
    {
        nAlignedWidth = (long)max(i_nResultWidth, i_nResultLength);
        nAlignedLength = (long)min(i_nResultWidth, i_nResultLength);
    }

    if (nAlignedLength > (nSpecLength * ErrorMax) || nAlignedWidth > (nSpecWidth * ErrorMax))
        return false;

    if (nAlignedLength < (nSpecLength * ErrorMin) || nAlignedWidth < (nSpecWidth * ErrorMin))
        return false;

    return true;
}

bool CPassive2DAlgorithm::Create_Capacitor_Electrode(long i_nPassiveType, FPI_RECT i_frtAlignResult)
{
    long nElectordeWidth_ForAlignResult_px(0);
    std::vector<long> vecnDirection(2);

    switch (i_nPassiveType)
    {
        case Passive_Horizontal:
            nElectordeWidth_ForAlignResult_px = i_frtAlignResult.GetCRect().Width() / 4;
            vecnDirection[0] = LEFT;
            vecnDirection[1] = RIGHT;
            break;
        case Passive_Vertical:
            nElectordeWidth_ForAlignResult_px = i_frtAlignResult.GetCRect().Height() / 4;
            vecnDirection[0] = UP;
            vecnDirection[1] = DOWN;
            break;
        default:
            break;
    }

    long nSelectElectrodeWidth
        = nElectordeWidth_ForAlignResult_px; //min(nElectordeWidth_ForAlignResult_px, nElectordeWidth_ForSpec_px);

    for (long nDirectionidx = 0; nDirectionidx < vecnDirection.size(); nDirectionidx++)
    {
        long nDirection = vecnDirection[nDirectionidx];
        if (!Set_ElectrodeArea(nDirection, i_frtAlignResult, nSelectElectrodeWidth))
            return false;
    }

    return true;
}

bool CPassive2DAlgorithm::Create_PAD_Area(const VisionScale& scale, long i_nPassiveAngleType, FPI_RECT i_frtAlignResult)
{
    long nPadNum(2); //pad는 한쌍으로 2개씩, "-" 방향이 우선
    bool bMinusDirection(true);
    for (long nPadCount = 0; nPadCount < nPadNum; nPadCount++)
    {
        if (nPadCount > 0)
            bMinusDirection = false;

        //Center Pos는 Gap 만큼 Pos Move
        Ipvm::Point32r2 fptOffsetPadCenter
            = Get_PAD_Center(scale, i_nPassiveAngleType, i_frtAlignResult, bMinusDirection);

        //Passive Length, Width Angle Type으로 분류
        Ipvm::Rect32r rtPAD_SpecROI = Get_PAD_Area(scale, i_nPassiveAngleType, fptOffsetPadCenter);

        m_pPassiveAlignResult->m_pad_spec.push_back(rtPAD_SpecROI);
        m_pPassiveResult_DebugItem->m_pad_specROIs.push_back(rtPAD_SpecROI);
    }

    return true;
}

bool CPassive2DAlgorithm::Set_ElectrodeArea(long i_nDirection, FPI_RECT i_frtAlign_Result, long i_nElectrodeWidth)
{
    if (i_nElectrodeWidth <= 0)
        return false;

    //Direction별로 RefPoint 및 Target Point정의
    EDGEPOINTS vecRefPoints = GetElectordeTargetPoints(i_nDirection, i_frtAlign_Result, i_nElectrodeWidth);
    EDGEPOINTS vecTargetPoints = GetElectordeTargetPoints(i_nDirection, i_frtAlign_Result, i_nElectrodeWidth, false);

    //Line획득
    Ipvm::LineEq32r line_byRefPoint{};
    Ipvm::LineEq32r line_byTargetPoint{}; //C++14 이 후로는 이렇게 해도 멤버 변수가 초기화된다.
    CPI_Geometry::LineFitting_RemoveNoise(vecRefPoints, line_byRefPoint);
    CPI_Geometry::LineFitting_RemoveNoise(vecTargetPoints, line_byTargetPoint);

    //각 Point들 RefLine을 이용하여 Projection하여 Rect구성
    Ipvm::LineEq32r line_Ref{};
    Ipvm::LineEq32r line_Target{};
    Ipvm::Point32r2 fptLTPos(0.f, 0.f), fptRTPos(0.f, 0.f), fptLBPos(0.f, 0.f), fptRBPos(0.f, 0.f);
    switch (i_nDirection)
    {
        case UP:
            line_Ref = line_byRefPoint;
            line_Target = line_byTargetPoint;
            fptLTPos = vecRefPoints[0];
            fptRTPos = vecTargetPoints[0];
            fptLBPos = vecRefPoints[1];
            fptRBPos = vecTargetPoints[1];
            break;
        case DOWN:
            line_Ref = line_byTargetPoint;
            line_Target = line_byRefPoint;
            fptLTPos = vecTargetPoints[0];
            fptRTPos = vecRefPoints[0];
            fptLBPos = vecTargetPoints[1];
            fptRBPos = vecRefPoints[1];
            break;
        case LEFT:
            line_Ref = line_byRefPoint;
            line_Target = line_byTargetPoint;
            fptLTPos = vecRefPoints[0];
            fptRTPos = vecTargetPoints[0];
            fptLBPos = vecRefPoints[1];
            fptRBPos = vecTargetPoints[1];
            break;
        case RIGHT:
            line_Ref = line_byTargetPoint;
            line_Target = line_byRefPoint;
            fptLTPos = vecTargetPoints[0];
            fptRTPos = vecRefPoints[0];
            fptLBPos = vecTargetPoints[1];
            fptRBPos = vecRefPoints[1];
            break;
    }

    Ipvm::Point32r2 fptElectPos_LT(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(line_Ref, fptLTPos, fptElectPos_LT);

    Ipvm::Point32r2 fptElectPos_RT(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(line_Target, fptRTPos, fptElectPos_RT);

    Ipvm::Point32r2 fptElectPos_LB(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(line_Ref, fptLBPos, fptElectPos_LB);

    Ipvm::Point32r2 fptElectPos_RB(0.f, 0.f);
    Ipvm::Geometry::GetFootOfPerpendicular(line_Target, fptRBPos, fptElectPos_RB);

    FPI_RECT frtElectordeArea(fptElectPos_LT, fptElectPos_RT, fptElectPos_LB, fptElectPos_RB);

    //Debug변수에 적용 후 끝!
    m_pPassiveAlignResult->m_elect_result.push_back(frtElectordeArea);
    if (m_pPassiveResult_DebugItem)
    {
        m_pPassiveResult_DebugItem->m_electAlignResult = frtElectordeArea;
    }

    return true;
}

Ipvm::Point32r2 CPassive2DAlgorithm::Get_PAD_Center(
    const VisionScale& scale, long i_nPassiveAngleType, FPI_RECT i_frtAlignResult, bool i_bMinusDirection)
{
    const auto& mm2px = scale.umToPixel();
    Ipvm::Point32r2 fptOffsetPadCenter(i_frtAlignResult.GetCenter().m_x, i_frtAlignResult.GetCenter().m_y);

    float fShiftValue_px(0.f);
    switch (i_nPassiveAngleType)
    {
        case Passive_Horizontal:
        {
            fShiftValue_px = m_pPassiveInfoDB->fPassivePAD_Gap_mm * mm2px.m_x;
            if (i_bMinusDirection)
                fptOffsetPadCenter.Offset(-fShiftValue_px, 0.f);
            else
                fptOffsetPadCenter.Offset(fShiftValue_px, 0.f);

            break;
        }
        case Passive_Vertical:
        {
            fShiftValue_px = m_pPassiveInfoDB->fPassivePAD_Gap_mm * mm2px.m_y;
            if (i_bMinusDirection)
                fptOffsetPadCenter.Offset(0.f, -fShiftValue_px);
            else
                fptOffsetPadCenter.Offset(0.f, fShiftValue_px);

            break;
        }
    }

    return fptOffsetPadCenter;
}

Ipvm::Rect32r CPassive2DAlgorithm::Get_PAD_Area(
    const VisionScale& scale, long i_nPassiveAngleType, Ipvm::Point32r2 i_fptOffsetPadCenter)
{
    const auto& mm2px = scale.mmToPixel();
    Ipvm::Rect32r frtPAD_SpecROI(0.f, 0.f, 0.f, 0.f);

    Ipvm::Point32r2 fPAD_SpecSize(0.f, 0.f);
    switch (i_nPassiveAngleType)
    {
        case Passive_Horizontal:
        {
            fPAD_SpecSize = Ipvm::Point32r2((m_pPassiveInfoDB->fPassivePAD_Width_mm * mm2px.m_y),
                (m_pPassiveInfoDB->fPassivePAD_Length_mm * mm2px.m_x));

            frtPAD_SpecROI.m_left = i_fptOffsetPadCenter.m_x - fPAD_SpecSize.m_y;
            frtPAD_SpecROI.m_right = i_fptOffsetPadCenter.m_x + fPAD_SpecSize.m_y;
            frtPAD_SpecROI.m_top = i_fptOffsetPadCenter.m_y - fPAD_SpecSize.m_x;
            frtPAD_SpecROI.m_bottom = i_fptOffsetPadCenter.m_y + fPAD_SpecSize.m_x;
            break;
        }
        case Passive_Vertical:
        {
            fPAD_SpecSize = Ipvm::Point32r2((m_pPassiveInfoDB->fPassivePAD_Width_mm * mm2px.m_x),
                (m_pPassiveInfoDB->fPassivePAD_Length_mm * mm2px.m_y));

            frtPAD_SpecROI.m_left = i_fptOffsetPadCenter.m_x - fPAD_SpecSize.m_x;
            frtPAD_SpecROI.m_right = i_fptOffsetPadCenter.m_x + fPAD_SpecSize.m_x;
            frtPAD_SpecROI.m_top = i_fptOffsetPadCenter.m_y - fPAD_SpecSize.m_y;
            frtPAD_SpecROI.m_bottom = i_fptOffsetPadCenter.m_y + fPAD_SpecSize.m_y;
            break;
        }
    }

    return frtPAD_SpecROI;
}

EDGEPOINTS CPassive2DAlgorithm::GetElectordeTargetPoints(
    long i_nDirection, FPI_RECT i_frtAlign_Result, long i_nElectrodeWidth, bool i_bRefPoints)
{
    Ipvm::Point32r2 fptTargetPos1(0.f, 0.f), fptTargetPos2(0.f, 0.f);

    if (i_bRefPoints)
        i_nElectrodeWidth = 0;

    switch (i_nDirection)
    {
        case UP:
            fptTargetPos1
                = Ipvm::Point32r2(i_frtAlign_Result.fptLT.m_x, i_frtAlign_Result.fptLT.m_y + i_nElectrodeWidth);
            fptTargetPos2
                = Ipvm::Point32r2(i_frtAlign_Result.fptRT.m_x, i_frtAlign_Result.fptRT.m_y + i_nElectrodeWidth);
            break;
        case DOWN:
            fptTargetPos1
                = Ipvm::Point32r2(i_frtAlign_Result.fptLB.m_x, i_frtAlign_Result.fptLB.m_y - i_nElectrodeWidth);
            fptTargetPos2
                = Ipvm::Point32r2(i_frtAlign_Result.fptRB.m_x, i_frtAlign_Result.fptRB.m_y - i_nElectrodeWidth);
            break;
        case LEFT:
            fptTargetPos1
                = Ipvm::Point32r2(i_frtAlign_Result.fptLT.m_x + i_nElectrodeWidth, i_frtAlign_Result.fptLT.m_y);
            fptTargetPos2
                = Ipvm::Point32r2(i_frtAlign_Result.fptLB.m_x + i_nElectrodeWidth, i_frtAlign_Result.fptLB.m_y);
            break;
        case RIGHT:
            fptTargetPos1
                = Ipvm::Point32r2(i_frtAlign_Result.fptRT.m_x - i_nElectrodeWidth, i_frtAlign_Result.fptRT.m_y);
            fptTargetPos2
                = Ipvm::Point32r2(i_frtAlign_Result.fptRB.m_x - i_nElectrodeWidth, i_frtAlign_Result.fptRB.m_y);
            break;
    }

    EDGEPOINTS vecfptreturnPoints;
    vecfptreturnPoints.push_back(fptTargetPos1);
    vecfptreturnPoints.push_back(fptTargetPos2);

    return vecfptreturnPoints;
}

bool CPassive2DAlgorithm::GetMajorEdgeSearchNeedInfo(long i_nPassiveType,
    const std::vector<std::vector<FPI_RECT>>& i_vec2frtDetailSearchROI,
    std::vector<sEdgeSearchNeedInfo>& o_vecsEdgeSearchNeedInfo)
{
    if (o_vecsEdgeSearchNeedInfo.size() > 0)
        o_vecsEdgeSearchNeedInfo.clear();

    std::vector<long> vecnDirectionType;
    sEdgeSearchNeedInfo sEdgeSearchNeedInfo;

    switch (m_pPassiveAlignParam->m_nOutline_align_Body_ElectrodeColor)
    {
        case DARK:
            sEdgeSearchNeedInfo.nSearchType = PI_ED_DIR_RISING;
            break;
        case BRIGHT:
            sEdgeSearchNeedInfo.nSearchType = PI_ED_DIR_FALLING;
            break;
        case BOTH:
            sEdgeSearchNeedInfo.nSearchType = PI_ED_DIR_BOTH;
            break;
    }

    sEdgeSearchSizeInfo sCurEdgeSearchSizeInfo;
    vecnDirectionType = GetEdgeSearchSDirectionType(i_nPassiveType, false);
    switch (i_nPassiveType)
    {
        case Passive_Horizontal:
            for (auto DirectionType : vecnDirectionType)
            {
                sEdgeSearchNeedInfo.nOriginSearchDirection = DirectionType;
                sEdgeSearchNeedInfo.rtSearchROI = i_vec2frtDetailSearchROI[DirectionType][0].GetCRect();
                sEdgeSearchNeedInfo.nSearchLength = sEdgeSearchNeedInfo.rtSearchROI.Height();
                sEdgeSearchNeedInfo.nSearchWidth = long((sEdgeSearchNeedInfo.rtSearchROI.Width() * .1f) * 1.5f);
                sEdgeSearchNeedInfo.nROISize = sEdgeSearchNeedInfo.rtSearchROI.Width();
                sEdgeSearchNeedInfo.nSearchDirection = GetEdgeSearchDirection(DirectionType);
                sEdgeSearchNeedInfo.ptSearchStartPos = Get_EdgeSearch_StartPoint(i_nPassiveType, DirectionType,
                    m_pPassiveAlignParam->m_nOutline_align_Body_EdgeSearch_Direction, sEdgeSearchNeedInfo.rtSearchROI);
                o_vecsEdgeSearchNeedInfo.push_back(sEdgeSearchNeedInfo);
            }
            break;
        case Passive_Vertical:
            for (auto DirectionType : vecnDirectionType)
            {
                sEdgeSearchNeedInfo.nOriginSearchDirection = DirectionType;
                sEdgeSearchNeedInfo.rtSearchROI = i_vec2frtDetailSearchROI[DirectionType][0].GetCRect();
                sEdgeSearchNeedInfo.nSearchLength = sEdgeSearchNeedInfo.rtSearchROI.Width();
                sEdgeSearchNeedInfo.nSearchWidth = long((sEdgeSearchNeedInfo.rtSearchROI.Height() * .1f) * 1.5f);
                sEdgeSearchNeedInfo.nROISize = sEdgeSearchNeedInfo.rtSearchROI.Height();
                sEdgeSearchNeedInfo.nSearchDirection = GetEdgeSearchDirection(DirectionType);
                sEdgeSearchNeedInfo.ptSearchStartPos = Get_EdgeSearch_StartPoint(i_nPassiveType, DirectionType,
                    m_pPassiveAlignParam->m_nOutline_align_Body_EdgeSearch_Direction, sEdgeSearchNeedInfo.rtSearchROI);
                o_vecsEdgeSearchNeedInfo.push_back(sEdgeSearchNeedInfo);
            }
            break;
        case Passive_MIA:
            for (auto DirectionType : vecnDirectionType)
            {
                sEdgeSearchNeedInfo.nOriginSearchDirection = DirectionType;
                sEdgeSearchNeedInfo.rtSearchROI = i_vec2frtDetailSearchROI[DirectionType][0].GetCRect();
                sCurEdgeSearchSizeInfo
                    = GetEdgeSearchSize_DirectionType(DirectionType, sEdgeSearchNeedInfo.rtSearchROI);
                sEdgeSearchNeedInfo.nSearchLength = sCurEdgeSearchSizeInfo.nSearchLength;
                sEdgeSearchNeedInfo.nSearchWidth = sCurEdgeSearchSizeInfo.nSearchWidth;
                sEdgeSearchNeedInfo.nROISize = sCurEdgeSearchSizeInfo.nROISize;
                sEdgeSearchNeedInfo.nSearchDirection = GetEdgeSearchDirection(DirectionType);
                sEdgeSearchNeedInfo.ptSearchStartPos = Get_EdgeSearch_StartPoint(i_nPassiveType, DirectionType,
                    m_pPassiveAlignParam->m_nOutline_align_Body_EdgeSearch_Direction, sEdgeSearchNeedInfo.rtSearchROI);
                o_vecsEdgeSearchNeedInfo.push_back(sEdgeSearchNeedInfo);
            }
            break;
    }

    return true;
}

std::vector<long> CPassive2DAlgorithm::GetEdgeSearchSDirectionType(long i_nPassiveType, bool i_bIsMinorEdgeType)
{
    std::vector<long> vecnDirectionType;
    if (!i_bIsMinorEdgeType)
    {
        switch (i_nPassiveType)
        {
            case Passive_Horizontal:
                vecnDirectionType.push_back(UP);
                vecnDirectionType.push_back(DOWN);
                break;
            case Passive_Vertical:
                vecnDirectionType.push_back(LEFT);
                vecnDirectionType.push_back(RIGHT);
                break;
            case Passive_MIA:
                vecnDirectionType.push_back(UP);
                vecnDirectionType.push_back(DOWN);
                vecnDirectionType.push_back(LEFT);
                vecnDirectionType.push_back(RIGHT);
                break;
        }
    }
    else //MinorType
    {
        switch (i_nPassiveType)
        {
            case Passive_Horizontal:
                vecnDirectionType.push_back(LEFT);
                vecnDirectionType.push_back(RIGHT);
                break;
            case Passive_Vertical:
                vecnDirectionType.push_back(UP);
                vecnDirectionType.push_back(DOWN);
                break;
        }
    }

    return vecnDirectionType;
}

sEdgeSearchSizeInfo CPassive2DAlgorithm::GetEdgeSearchSize_DirectionType(
    const long i_nDirection, const Ipvm::Rect32s i_rtSearchROI)
{
    sEdgeSearchSizeInfo sEdgeSearchSizeInfo{};

    switch (i_nDirection)
    {
        case UP:
        case DOWN:
        {
            sEdgeSearchSizeInfo.nSearchLength = i_rtSearchROI.Height();
            sEdgeSearchSizeInfo.nSearchWidth = long((i_rtSearchROI.Width() * .1f) * 1.5f);
            sEdgeSearchSizeInfo.nROISize = i_rtSearchROI.Width();
        }
        break;
        case LEFT:
        case RIGHT:
        {
            sEdgeSearchSizeInfo.nSearchLength = i_rtSearchROI.Width();
            sEdgeSearchSizeInfo.nSearchWidth = long((i_rtSearchROI.Height() * .1f) * 1.5f);
            sEdgeSearchSizeInfo.nROISize = i_rtSearchROI.Height();
        }
        break;
        default:
            ASSERT(!(_T("Direction error")));
            break;
    }

    return sEdgeSearchSizeInfo;
}

long CPassive2DAlgorithm::GetEdgeSearchDirection(long i_nCurDirectionType)
{
    long nReturnEdgeSearchDirection(0);
    switch (i_nCurDirectionType)
    {
        case enumPassiveSearchROIPos::SearchROI_POS_TOP:
            if (m_pPassiveAlignParam->m_nOutline_align_Body_EdgeSearch_Direction == PI_ED_DIR_OUTER)
                nReturnEdgeSearchDirection = UP;
            else
                nReturnEdgeSearchDirection = DOWN;
            break;
        case enumPassiveSearchROIPos::SearchROI_POS_BTM:
            if (m_pPassiveAlignParam->m_nOutline_align_Body_EdgeSearch_Direction == PI_ED_DIR_OUTER)
                nReturnEdgeSearchDirection = DOWN;
            else
                nReturnEdgeSearchDirection = UP;
            break;
        case enumPassiveSearchROIPos::SearchROI_POS_LEFT:
            if (m_pPassiveAlignParam->m_nOutline_align_Body_EdgeSearch_Direction == PI_ED_DIR_OUTER)
                nReturnEdgeSearchDirection = LEFT;
            else
                nReturnEdgeSearchDirection = RIGHT;
            break;
        case enumPassiveSearchROIPos::SearchROI_POS_RIGHT:
            if (m_pPassiveAlignParam->m_nOutline_align_Body_EdgeSearch_Direction == PI_ED_DIR_OUTER)
                nReturnEdgeSearchDirection = RIGHT;
            else
                nReturnEdgeSearchDirection = LEFT;
            break;
    }

    return nReturnEdgeSearchDirection;
}

Ipvm::Point32s2 CPassive2DAlgorithm::Get_EdgeSearch_StartPoint(
    long i_nPassiveDirection, long i_nEdgeSearchDir, long i_SearchDir, Ipvm::Rect32s i_rtSearchROI)
{
    Ipvm::Point32s2 ptSearch_StartPos(0, 0);

    //if (i_nPassiveDirection == Passive_Horizontal)
    //{
    //	switch (i_nEdgeSearchDir)
    //	{
    //	case UP:
    //	{
    //		ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
    //		if (i_SearchDir == PI_ED_DIR_OUTER)
    //			ptSearch_StartPos.m_y = i_rtSearchROI.m_bottom;
    //		else
    //			ptSearch_StartPos.m_y = i_rtSearchROI.m_top;

    //		break;
    //	}
    //	case DOWN:
    //	{
    //		ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
    //		if (i_SearchDir == PI_ED_DIR_OUTER)
    //			ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
    //		else // Out -> In
    //			ptSearch_StartPos.m_y = i_rtSearchROI.m_bottom;

    //		break;
    //	}
    //	case LEFT:
    //		break;
    //	case RIGHT:
    //		break;
    //	}
    //}
    //else if (i_nPassiveDirection == Passive_Vertical)
    //{
    //	switch (i_nEdgeSearchDir)
    //	{
    //	case UP: break;
    //	case DOWN: break;
    //	case LEFT:
    //	{
    //		ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
    //		if (i_SearchDir == PI_ED_DIR_OUTER)
    //			ptSearch_StartPos.m_x = i_rtSearchROI.m_right;
    //		else
    //			ptSearch_StartPos.m_x = i_rtSearchROI.m_left;

    //		break;
    //	}
    //	case RIGHT:
    //	{
    //		ptSearch_StartPos.m_y = i_rtSearchROI.m_top;

    //		if (i_SearchDir == PI_ED_DIR_OUTER)
    //			ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
    //		else
    //			ptSearch_StartPos.m_x = i_rtSearchROI.m_right;

    //		break;
    //	}

    //	}
    //}

    switch (i_nPassiveDirection) //mc_2019.11.12 수정가능할꺼 같다 하나로
    {
        case Passive_Horizontal:
            switch (i_nEdgeSearchDir)
            {
                case UP:
                {
                    ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_bottom;
                    else
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_top;

                    break;
                }
                case DOWN:
                {
                    ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
                    else // Out -> In
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_bottom;

                    break;
                }
            }
            break;
        case Passive_Vertical:
            switch (i_nEdgeSearchDir)
            {
                case LEFT:
                {
                    ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_right;
                    else
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_left;

                    break;
                }
                case RIGHT:
                {
                    ptSearch_StartPos.m_y = i_rtSearchROI.m_top;

                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
                    else
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_right;

                    break;
                }
            }
        case Passive_MIA:
            switch (i_nEdgeSearchDir)
            {
                case UP:
                {
                    ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_bottom;
                    else
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_top;

                    break;
                }
                case DOWN:
                {
                    ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
                    else // Out -> In
                        ptSearch_StartPos.m_y = i_rtSearchROI.m_bottom;

                    break;
                }
                case LEFT:
                {
                    ptSearch_StartPos.m_y = i_rtSearchROI.m_top;
                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_right;
                    else
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_left;

                    break;
                }
                case RIGHT:
                {
                    ptSearch_StartPos.m_y = i_rtSearchROI.m_top;

                    if (i_SearchDir == PI_ED_DIR_OUTER)
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_left;
                    else
                        ptSearch_StartPos.m_x = i_rtSearchROI.m_right;

                    break;
                }
                break;
            }
    }

    return ptSearch_StartPos;
}