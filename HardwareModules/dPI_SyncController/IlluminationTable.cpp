//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "IlluminationTable.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../AlgorithmModules/dPI_Geometry/dPI_GeometryLib.h" //kircheis_IllumCal
#include "../../DefineModules/dA_Base/DynamicSystemPath.h"
#include "../../InformationModule/dPI_SystemIni/SystemConfig.h"

//CPP_4_________________________________ External library headers
#include <Ipvm/Algorithm/DataFitting.h>
#include <Ipvm/Base/Enum.h>
#include <Ipvm/Base/LineEq32r.h>
#include <Ipvm/Base/Point32r2.h>

//CPP_5_________________________________ Standard library headers
#include <fstream>
#include <map>

//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IlluminationTable::IlluminationTable()
{
    memset(m_timeToIntensity, 0, sizeof(m_timeToIntensity));
}

IlluminationTable::~IlluminationTable()
{
}

bool IlluminationTable::Load(LPCTSTR pathName, CString& errorDesc)
{
    typedef std::basic_string<char>::size_type temp_string_size_type;

    auto& systemConfig = SystemConfig::GetInstance(); //kircheis_IllumCal
    //{{//kircheis_WB
    CString strFileName;
    strFileName.Format(_T("%s"), pathName);
    BOOL bIsRefIncludedCurve
        = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableRefTableAll)); //kircheis_IllumCal
    BOOL bIsCurIncludedCurve
        = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableTableAll)); //kircheis_IllumCal
    BOOL bIsRefIncludedCurveMirror
        = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableRefTableAllMirror)); //SDY_DualCal
    BOOL bIsCurIncludedCurveMirror
        = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror)); //SDY_DualCal

    BOOL bIsRef = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableRefTable)) || bIsRefIncludedCurve
        || bIsRefIncludedCurveMirror;
    std::vector<Ipvm::LineEq32r> vecLineEq;

    //SDY_Side Vision Calibration
    BOOL bIsCurIncludedCurve_F = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableTableAll_F));
    BOOL bIsCurIncludedCurveMirror_F = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror_F));

    BOOL bIsCurIncludedCurve_R = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableTableAll_R));
    BOOL bIsCurIncludedCurveMirror_R = (strFileName == DynamicSystemPath::get(DefineFile::IllumTableTableAllMirror_R));

    //BOOL bIsSide = (bIsCurIncludedCurve_F || bIsCurIncludedCurve_R || bIsCurIncludedCurveMirror_F
    //    || bIsCurIncludedCurveMirror_R);
    //}}

    //long UseChannelCount = systemConfig.GetUseIllumChannelCount();
    long nChannelMaxCount = systemConfig.GetMaxIllumChannelCount();

    std::vector<BOOL> vecbvalidillumination(nChannelMaxCount,
        FALSE); //mc_Ring 조명 사용시 초기에 CH14,15,16은 데이터가 없을수 있기 때문에 유효성 판단으로 사용한다

    std::ifstream infile(pathName);

    if (infile.is_open() == false)
    {
        errorDesc.Format(_T("Cannot find \"%s\" file."), pathName);
        return false;
    }

    char buffer[1000];

    long columnLink[FX_END];
    long headerColumnCount = 0;

    // Read header
    if (infile.getline(buffer, 1000))
    {
        //---------------------------------------------------------------------------------------
        // Read Header
        //---------------------------------------------------------------------------------------
        std::basic_string<char> temp(buffer);

        temp_string_size_type findStart = 0;
        temp_string_size_type findEnd = 0;

        std::map<CStringA, long> headerIndexMap;
        long index = 0;
        while (1)
        {
            findEnd = temp.find_first_of(',', findStart);
            CStringA text;
            if (findEnd == std::basic_string<char>::npos)
            {
                text.SetString(buffer + findStart, CAST_INT(temp.length() - findStart));
                text = text.MakeUpper();
                headerIndexMap[text] = index;
                break;
            }
            else
            {
                text.SetString(buffer + findStart, CAST_INT(findEnd - findStart));
                text = text.MakeUpper();

                headerIndexMap[text] = index;
                findStart = findEnd + 1;
                index++;
            }
        }

        // 헤더 정보중 필수 항목들이 모두 들어왔는지 확인한다

        for (long item = 0; item <= nChannelMaxCount; item++)
        {
            CStringA columnName;
            switch (item)
            {
                case FX_TIME_MS:
                    columnName = "TIME_MS";
                    break;
                case FX_CH_1:
                    columnName = "CH_1";
                    break;
                case FX_CH_2:
                    columnName = "CH_2";
                    break;
                case FX_CH_3:
                    columnName = "CH_3";
                    break;
                case FX_CH_4:
                    columnName = "CH_4";
                    break;
                case FX_CH_5:
                    columnName = "CH_5";
                    break;
                case FX_CH_6:
                    columnName = "CH_6";
                    break;
                case FX_CH_7:
                    columnName = "CH_7";
                    break;
                case FX_CH_8:
                    columnName = "CH_8";
                    break;
                case FX_CH_9:
                    columnName = "CH_9";
                    break;
                case FX_CH_10:
                    columnName = "CH_10";
                    break;
                case FX_CH_11:
                    columnName = "CH_11";
                    break;
                case FX_CH_12:
                    columnName = "CH_12";
                    break;
                case FX_CH_13:
                    columnName = "CH_13";
                    break;
                case FX_CH_14:
                    columnName = "CH_14";
                    break;
                case FX_CH_15:
                    columnName = "CH_15";
                    break;
                case FX_CH_16:
                    columnName = "CH_16";
                    break;
            }

            if (columnName.IsEmpty())
                break;

            if (headerIndexMap.find(columnName) == headerIndexMap.end())
            {
                if (SystemConfig::GetInstance().IsVisionTypeNGRV()
                    && item
                        >= LED_ILLUM_CHANNEL_DEFAULT) // NGRV Vision 일 때는 CH12 까지 밖에 없어서 뒤의 데이터는 제외한다 - JHB_NGRV
                {
                    continue;
                }

                if (item
                    <= LED_ILLUM_CHANNEL_DEFAULT) //MC_CH14,15,16 Data가 없기 때문에 해당 데이터는 제외하고 확인한다
                {
                    errorDesc.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
                    return FALSE;
                }
            }
            else
            {
                if (item == FX_TIME_MS)
                {
                    columnLink[item] = headerIndexMap[columnName];
                    continue;
                }

                vecbvalidillumination[item - FX_CH_1] = TRUE; //mc_첫번째 item은 ms이다
            }

            columnLink[item] = headerIndexMap[columnName];
        }

        headerColumnCount = long(headerIndexMap.size());
    }
    else
    {
        return false;
    }

    long rowIndex = 1;

    while (infile.getline(buffer, 1000))
    {
        rowIndex++;

        std::basic_string<char> temp(buffer);

        temp_string_size_type findStart = 0;
        temp_string_size_type findEnd = 0;

        long nChannelidx(0);
        CStringA dataList[LED_ILLUM_CHANNEL_MAX + FX_CH_1] = {
            "",
        };

        //여기까지 온거는 데이터를 읽기만 하면된다
        while (1)
        {
            findEnd = temp.find_first_of(',', findStart);
            CStringA text;
            if (findEnd == std::basic_string<char>::npos)
            {
                text.SetString(buffer + findStart, CAST_INT(temp.length() - findStart));
                dataList[nChannelidx] = text;
                break;
            }
            else
            {
                text.SetString(buffer + findStart, CAST_INT(findEnd - findStart));
                dataList[nChannelidx] = text;
                findStart = findEnd + 1;
            }

            nChannelidx++;
        }

        //kk 임시용, Ring 조명 Table이 없으므로 Ring조명일땐 넘겨라!
        //BOOL bSequencePass(TRUE);

        //for (int i = 0; i < LED_ILLUM_CHANNEL_DEFAULT; i++)
        //{
        //	bSequencePass &= vecbuseillumination[i];
        //}

        //if (dataList.size() != headerColumnCount)
        //{
        //	if (!bSequencePass)
        //	{
        //		// 헤더 컬럼과 데이터 컴럼의 수가 다르다
        //		errorDesc.Format(_T("The number of columns is different. (%d Line)"), rowIndex);
        //		return false;
        //	}
        //}

        m_illuminationTimes_ms.push_back(::atof(dataList[columnLink[FX_TIME_MS]]));

        for (long channel = 0; channel < nChannelMaxCount; channel++)
        {
            if (vecbvalidillumination[channel] == TRUE)
                m_intensities_ms[channel].push_back(::atof(dataList[columnLink[channel + FX_CH_1]]));
        }
    }

    const auto dataCount = m_illuminationTimes_ms.size();

    // Fitting
    if (bIsRefIncludedCurve || bIsCurIncludedCurve || bIsCurIncludedCurve_F)
    {
        auto& vecIllumFitting = (bIsRef == TRUE ? SystemConfig::GetInstance().m_vecIllumFittingRef
                                                : SystemConfig::GetInstance().m_vecIllumFittingCur);
        vecIllumFitting.clear();
        //{{//kircheis_IllumCal
        auto& vecIllumCurveFitting
            = (bIsRef == TRUE ? systemConfig.m_vecIllumCurveFittingRef : systemConfig.m_vecIllumCurveFittingCur);
        vecIllumCurveFitting.clear();
        auto& vecIllumCurveFittingReverse = (bIsRef == TRUE ? systemConfig.m_vecIllumCurveFittingRefReverse
                                                            : systemConfig.m_vecIllumCurveFittingCurReverse);
        vecIllumCurveFittingReverse.clear();

        // SDY_DualCal 중복되는 초기화는 삭제

        CalcCurveFitting(vecIllumFitting, vecIllumCurveFitting, vecIllumCurveFittingReverse, vecbvalidillumination,
            m_timeToIntensity, errorDesc);
    }

    else if (bIsRefIncludedCurveMirror || bIsCurIncludedCurveMirror || bIsCurIncludedCurveMirror_F)
    {
        //{{//kircheis_IllumCal
        auto& vecIllumFitting = (bIsRef == TRUE ? SystemConfig::GetInstance().m_vecIllumFittingRefMirror
                                                : SystemConfig::GetInstance().m_vecIllumFittingCurMirror);
        vecIllumFitting.clear();

        CalcMirrorFitting(vecIllumFitting, vecbvalidillumination, m_timeToIntensity, errorDesc);
    }

    else if (bIsCurIncludedCurve_R)
    {
        auto& vecIllumFitting_R = (bIsRef == TRUE ? SystemConfig::GetInstance().m_vecIllumFittingRef
                                                  : SystemConfig::GetInstance().m_vecIllumFittingCur_R);
        vecIllumFitting_R.clear();
        //{{//kircheis_IllumCal
        auto& vecIllumCurveFitting_R
            = (bIsRef == TRUE ? systemConfig.m_vecIllumCurveFittingRef : systemConfig.m_vecIllumCurveFittingCur_R);
        vecIllumCurveFitting_R.clear();
        auto& vecIllumCurveFittingReverse_R = (bIsRef == TRUE ? systemConfig.m_vecIllumCurveFittingRefReverse
                                                              : systemConfig.m_vecIllumCurveFittingCurReverse_R);
        vecIllumCurveFittingReverse_R.clear();

        // SDY_DualCal 중복되는 초기화는 삭제

        CalcCurveFitting(vecIllumFitting_R, vecIllumCurveFitting_R, vecIllumCurveFittingReverse_R,
            vecbvalidillumination, m_timeToIntensity_R, errorDesc);
    }

    else if (bIsCurIncludedCurveMirror_R)
    {
        auto& vecIllumFitting = (bIsRef == TRUE ? SystemConfig::GetInstance().m_vecIllumFittingRefMirror
                                                : SystemConfig::GetInstance().m_vecIllumFittingCurMirror_R);
        vecIllumFitting.clear();

        CalcMirrorFitting(vecIllumFitting, vecbvalidillumination, m_timeToIntensity, errorDesc);
    }

    else
    {
        // SDY_DualCal 덤프 제작시 라인 보정 결과가 필요하므로 기존 Gray Target의 결과를 리턴한다
        auto& vecIllumFitting = (bIsRef == TRUE ? SystemConfig::GetInstance().m_vecIllumFittingRef
                                                : SystemConfig::GetInstance().m_vecIllumFittingCur);
        vecIllumFitting.clear();

        for (long channel = 0; channel < nChannelMaxCount; channel++)
        {
            if (vecbvalidillumination[channel] == FALSE) //mc_Ring 조명사용시 보정된 결과가 없다면
            {
                Ipvm::LineEq32r lineEq_dump(1.f, -1.f, 0);
                vecIllumFitting.push_back(lineEq_dump); //kircheis_WB
                continue;
            }

            std::vector<Ipvm::Point32r2> data;

            for (size_t index = 0; index < dataCount; index++)
            {
                if (m_intensities_ms[channel][index] > 0.)
                {
                    data.emplace_back(float(m_illuminationTimes_ms[index]), float(m_intensities_ms[channel][index]));
                }
            }

            if (data.size() < 5)
            {
                errorDesc.Format(_T("Data count of channel %d is less than 5"), channel + 1);
                vecIllumFitting.clear(); //kircheis_WB
                return false;
            }

            Ipvm::LineEq32r lineEq;

            if (Ipvm::Status::e_ok != Ipvm::DataFitting::FitToLine(CAST_INT32T(data.size()), &data[0], lineEq))
            {
                vecIllumFitting.clear(); //kircheis_WB
                errorDesc.Format(_T("Can't calculate illumination coefficient of channel %d"), channel + 1);
                return false;
            }

            vecIllumFitting.push_back(lineEq); //kircheis_WB

            m_timeToIntensity[channel] = -lineEq.m_a / lineEq.m_b;
        }
    }

    return true;
}

void IlluminationTable::CalcIlluminationGain(const IlluminationTable& refTable, double gains[LED_ILLUM_CHANNEL_MAX])
{
    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();

    for (long channel = 0; channel < nChannelMaxCount; channel++)
    {
        gains[channel] = refTable.m_timeToIntensity[channel] / m_timeToIntensity[channel];
    }
}

void IlluminationTable::CalcIlluminationGainRear(const IlluminationTable& refTable, double gains[LED_ILLUM_CHANNEL_MAX])
{
    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();

    for (long channel = 0; channel < nChannelMaxCount; channel++)
    {
        gains[channel] = refTable.m_timeToIntensity[channel] / m_timeToIntensity_R[channel];
    }
}

bool IlluminationTable::CalcCurveFitting(std::vector<Ipvm::LineEq32r>& o_vecIllumFitting,
    std::vector<Curve2DEq>& o_vecIllumCurveFitting, std::vector<Curve2DEq>& o_vecIllumCurveFittingReverse,
    std::vector<BOOL> vecbvalidillumination, double timeToIntensity[LED_ILLUM_CHANNEL_MAX], CString& o_errorDesc)
{
    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();

    const auto dataCount = m_illuminationTimes_ms.size();

    for (long channel = 0; channel < nChannelMaxCount; channel++)
    {
        if (vecbvalidillumination[channel] == FALSE) //mc_Ring 조명사용시 보정된 결과가 없다면
        {
            Ipvm::LineEq32r lineEq_dump(1.f, -1.f, 0);
            o_vecIllumFitting.push_back(lineEq_dump); //kircheis_WB

            timeToIntensity[channel] = -lineEq_dump.m_a / lineEq_dump.m_b;
            Curve2DEq curveEq_dump, curveEqReverse_dump; //kircheis_IllumCal
            curveEq_dump.InitData(0., -1., 0.);
            curveEqReverse_dump.InitData(0., -1., 0.);

            o_vecIllumCurveFitting.push_back(curveEq_dump);
            o_vecIllumCurveFittingReverse.push_back(curveEqReverse_dump);

            continue;
        }

        std::vector<Ipvm::Point32r2> dataForLine;
        std::vector<Ipvm::Point32r2> dataForCurve;
        std::vector<Ipvm::Point32r2> dataForCurveReverse;

        for (size_t index = 0; index < dataCount; index++)
        {
            if (m_intensities_ms[channel][index] > 19.)
            {
                dataForLine.emplace_back(float(m_illuminationTimes_ms[index]), float(m_intensities_ms[channel][index]));
            }

            if (m_intensities_ms[channel][index] > 0.
                && m_intensities_ms[channel][index] < SystemConfig::GetInstance().m_dEndOfCurveIntensity)
            {
                dataForCurve.emplace_back(
                    float(m_illuminationTimes_ms[index]), float(m_intensities_ms[channel][index]));
                dataForCurveReverse.emplace_back(
                    float(m_intensities_ms[channel][index]), float(m_illuminationTimes_ms[index]));
            }
        }

        if (SystemConfig::GetInstance()
                .IsVisionTypeNGRV()) // NGRV Vision 이면서 UV, IR, CH10, CH11이면 조명 보정 결과가 없다 - 2022.01.24_JHB_NGRV
        {
            static const long nChannel_UV = SystemConfig::GetInstance().m_nNgrvUVchID;
            static const long nChannel_IR = SystemConfig::GetInstance().m_nNgrvIRchID;

            if (channel == nChannel_UV || channel == nChannel_IR || channel + FX_CH_1 == FX_CH_10
                || channel + FX_CH_1 == FX_CH_11)
            {
                Ipvm::LineEq32r lineEq_dump(1.f, -1.f, 0);
                o_vecIllumFitting.push_back(lineEq_dump); //kircheis_WB

                timeToIntensity[channel] = -lineEq_dump.m_a / lineEq_dump.m_b;
                Curve2DEq curveEq_dump, curveEqReverse_dump; //kircheis_IllumCal
                curveEq_dump.InitData(0., 1., 0.);
                curveEqReverse_dump.InitData(0., 1., 0.);

                o_vecIllumCurveFitting.push_back(curveEq_dump);
                o_vecIllumCurveFittingReverse.push_back(curveEqReverse_dump);

                continue;
            }
        }

        if (dataForLine.size() < 5) //SDY_DualCal 50% Target으로 바뀌면서 최소 5개 이상이면 보정이 진행되게 설정
        {
            o_errorDesc.Format(_T("Data count of channel %d is less than 5"), channel + 1);
            o_vecIllumFitting.clear(); //kircheis_WB
            return false;
        }

        Ipvm::LineEq32r lineEq;
        //Ipvm::LineEq32r lineEqReverse;

        if (Ipvm::Status::e_ok
            != Ipvm::DataFitting::FitToLine(CAST_INT32T(dataForLine.size()), &dataForLine[0], lineEq))
        {
            o_vecIllumFitting.clear(); //kircheis_WB
            o_errorDesc.Format(_T("Can't calculate illumination coefficient of channel %d"), channel + 1);
            return false;
        }

        o_vecIllumFitting.push_back(lineEq); //kircheis_WB

        timeToIntensity[channel] = -lineEq.m_a / lineEq.m_b;
        //float fC = -lineEq.m_c / lineEq.m_b;

        Curve2DEq curveEq, curveEqReverse; //kircheis_IllumCal
        if (dataForCurve.size() > 3 && CPI_Geometry::Get2DCurveFitting(dataForCurve, curveEq))
            o_vecIllumCurveFitting.push_back(curveEq);
        else
        {
            //SDY_DualCal 커브 피팅이 실패했을 시에는 라인피팅의 결과를 집어넣는다.
            curveEq.m_a = -(lineEq.m_c / lineEq.m_b);
            curveEq.m_b = (-lineEq.m_a / lineEq.m_b);
            curveEq.m_c = 0;
            o_vecIllumCurveFitting.push_back(curveEq);
        }
        if (dataForCurveReverse.size() > 3 && CPI_Geometry::Get2DCurveFitting(dataForCurveReverse, curveEqReverse))
            o_vecIllumCurveFittingReverse.push_back(curveEqReverse);
        else
        {
            // 커브 리버스 피팅이 실패했을 시에는 라인피팅의 결과를 집어넣는다.
            curveEqReverse.m_a = -(curveEq.m_a / curveEq.m_b); // -(fCurLineB / fCurLineb);
            curveEqReverse.m_b = (1 / curveEq.m_b); //
            curveEqReverse.m_c = 0;
            o_vecIllumCurveFittingReverse.push_back(curveEqReverse);
        }
    }
    return true;
}

bool IlluminationTable::CalcMirrorFitting(std::vector<Ipvm::LineEq32r>& vecIllumFitting,
    std::vector<BOOL> vecbvalidillumination, double timeToIntensity[LED_ILLUM_CHANNEL_MAX], CString& errorDesc)
{
    UNREFERENCED_PARAMETER(vecbvalidillumination);
    UNREFERENCED_PARAMETER(timeToIntensity);

    //long UseChannelCount = SystemConfig::GetInstance().GetUseIllumChannelCount();
    long nChannelMaxCount = SystemConfig::GetInstance().GetMaxIllumChannelCount();

    const auto dataCount = m_illuminationTimes_ms.size();

    for (long channel = 0; channel < nChannelMaxCount; channel++)
    {
        if (vecbvalidillumination[channel] == FALSE) //mc_Ring 조명사용시 보정된 결과가 없다면
        {
            Ipvm::LineEq32r lineEq_dump(1.f, -1.f, 0);
            vecIllumFitting.push_back(lineEq_dump); //kircheis_WB

            m_timeToIntensity[channel] = -lineEq_dump.m_a / lineEq_dump.m_b;

            continue;
        }

        std::vector<Ipvm::Point32r2> dataForLine;

        for (size_t index = 0; index < dataCount; index++)
        {
            if (m_intensities_ms[channel][index] > 19.)
            {
                dataForLine.emplace_back(float(m_illuminationTimes_ms[index]), float(m_intensities_ms[channel][index]));
            }
        }

        if ((dataForLine.size() < 3) || dataForLine[dataForLine.size() - 1].m_y < 150)
        {
            errorDesc.Format(_T("Data count of channel %d is less than 5"), channel + 1);
            //vecIllumFitting.clear();//kircheis_WB
        }

        Ipvm::LineEq32r lineEq;
        //Ipvm::LineEq32r lineEqReverse;

        if (SystemConfig::GetInstance().GetIllumType(channel) == IllumType_Coaxial)
        {
            if (Ipvm::Status::e_ok
                != Ipvm::DataFitting::FitToLine(CAST_INT32T(dataForLine.size()), &dataForLine[0], lineEq))
            {
                errorDesc.Format(_T("Can't calculate illumination coefficient of channel %d"), channel + 1);
            }

            vecIllumFitting.push_back(lineEq); //kircheis_WB

            m_timeToIntensity[channel] = -lineEq.m_a / lineEq.m_b;
        }
        else
        {
            lineEq.m_a = 1;
            lineEq.m_b = 1;
            lineEq.m_c = 1;

            vecIllumFitting.push_back(lineEq); //kircheis_WB

            m_timeToIntensity[channel] = -lineEq.m_a / lineEq.m_b;
        }
    }

    return true;
}

double IlluminationTable::GetTimeToIntensity(long nCh)
{
    return m_timeToIntensity[nCh];
}

double IlluminationTable::GetTimeToIntensity_R(long nCh)
{
    return m_timeToIntensity_R[nCh];
}