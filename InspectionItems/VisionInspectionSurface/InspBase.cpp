//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "InspBase.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
#include "../../SharedCommunicationModules/VisionHostCommon/DBObject.h"
#include "../../SharedCommunicationModules/VisionHostCommon/VisionHostBaseDef.h"
#include "../../VisionNeedLibrary/VisionCommon/VisionBaseDef.h"

//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
const float SItemSpec::fLimitMin = -FLT_MAX / 1000.f;
const float SItemSpec::fLimitMax = FLT_MAX / 1000.f;

// =========================================================
/** @fn		SItemSpec::SItemSpec()
@brief		
@return		: 
@date		: 2005-09-14 오후 12:36:09
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
SItemSpec::SItemSpec()
    : bInsp(FALSE)
    , fMarginalMin(0.f)
    , fPassMin(0.f)
    , fNominal(0.f)
    , fPassMax(0.f)
    , fMarginalMax(0.f)
    , nSpecType(MEASURE)
    , bVariable(FALSE)
    , bAbsolute(FALSE)
{
    //memset(_ReservedData, 0, __nReservedDataLen);
    ;
}

BOOL SItemSpec::LinkDataBase(BOOL save, CiDataBase& db)
{
    if (!db[_T("bInsp")].Link(save, bInsp))
        bInsp = FALSE;

    if (!db[_T("fMarginalMin")].Link(save, fMarginalMin))
        fMarginalMin = 0.f;
    if (!db[_T("fPassMin")].Link(save, fPassMin))
        fPassMin = 0.f;

    if (!db[_T("fNominal")].Link(save, fNominal))
        fNominal = 0.f;
    if (!db[_T("fPassMax")].Link(save, fPassMax))
        fPassMax = 0.f;

    if (!db[_T("fMarginalMax")].Link(save, fMarginalMax))
        fMarginalMax = 0.f;
    if (!db[_T("nSpecType")].Link(save, nSpecType))
        nSpecType = MEASURE;
    if (!db[_T("bVariable")].Link(save, bVariable))
        bVariable = FALSE;
    if (!db[_T("bAbsolute")].Link(save, bAbsolute))
        bAbsolute = FALSE;

    return TRUE;
}

// =========================================================
/** @fn		SItemSpec::SItemSpec(const SItemSpec& Src)
@brief		
@param		const SItemSpec& Src : 
@return		: 
@date		: 2005-09-14 오후 12:36:11
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
SItemSpec::SItemSpec(const SItemSpec& Src)
    : bInsp(Src.bInsp)
    , fMarginalMin(Src.fMarginalMin)
    , fPassMin(Src.fPassMin)
    , fNominal(Src.fNominal)
    , fPassMax(Src.fPassMax)
    , fMarginalMax(Src.fMarginalMax)
    , nSpecType(Src.nSpecType)
    , bVariable(Src.bVariable)
    , bAbsolute(Src.bAbsolute)
{
    //memset(_ReservedData, 0, __nReservedDataLen);
}

// =========================================================
/** @fn		SItemSpec::SItemSpec(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fNominal, float _fPassMax, float _fMarginalMax, long _nSpecType)
@brief		
@param		BOOL _bInsp : 
@param		float _fMarginalMin : 
@param		float _fPassMin : 
@param		float _fNominal : 
@param		float _fPassMax : 
@param		float _fMarginalMax : 
@param		long _nSpecType : 
@return		: 
@date		: 2005-09-14 오후 12:36:14
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
SItemSpec::SItemSpec(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fNominal, float _fPassMax,
    float _fMarginalMax, long _nSpecType, BOOL _bVariable, BOOL _bAbsolute)
    : bInsp(_bInsp)
    , fMarginalMin(_fMarginalMin)
    , fPassMin(_fPassMin)
    , fNominal(_fNominal)
    , fPassMax(_fPassMax)
    , fMarginalMax(_fMarginalMax)
    , nSpecType(_nSpecType)
    , bVariable(_bVariable)
    , bAbsolute(_bAbsolute)
{
    //memset(_ReservedData, 0, __nReservedDataLen);
}

// =========================================================
/** @fn		SItemSpec::SItemSpec(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fNominal, float _fPassMax, float _fMarginalMax, long _nSpecType)
@brief		
@param		BOOL _bInsp : 
@param		float _fMarginalMin : 
@param		float _fPassMin : 
@param		float _fNominal : 
@param		float _fPassMax : 
@param		float _fMarginalMax : 
@param		long _nSpecType : 
@return		: 
@date		: 2005-09-14 오후 12:36:14
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
SItemSpec::SItemSpec(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fPassMax, float _fMarginalMax,
    long _nSpecType, BOOL _bVariable, BOOL _bAbsolute)
    : bInsp(_bInsp)
    , fMarginalMin(_fMarginalMin)
    , fPassMin(_fPassMin)
    , fNominal(0.5f * (_fPassMin + _fPassMax))
    , fPassMax(_fPassMax)
    , fMarginalMax(_fMarginalMax)
    , nSpecType(_nSpecType)
    , bVariable(_bVariable)
    , bAbsolute(_bAbsolute)
{
    //	memset(_ReservedData, 0, __nReservedDataLen);
}

void SItemSpec::Limit()
{
    if (fMarginalMin < fLimitMin)
        fMarginalMin = fLimitMin;
    if (fMarginalMax > fLimitMax)
        fMarginalMax = fLimitMax;
    if (fPassMin < fLimitMin)
        fPassMin = fLimitMin;
    if (fPassMax > fLimitMax)
        fPassMax = fLimitMax;
}
// =========================================================
/** @fn		SItemSpec& SItemSpec::operator=(const SItemSpec &Src)
@brief		
@param		const SItemSpec &Src : 
@return		: 
@date		: 2005-09-14 오후 12:36:17
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
SItemSpec& SItemSpec::operator=(const SItemSpec& Src)
{
    bInsp = Src.bInsp;

    fMarginalMin = Src.fMarginalMin;
    fPassMin = Src.fPassMin;
    fNominal = Src.fNominal;
    fPassMax = Src.fPassMax;
    fMarginalMax = Src.fMarginalMax;

    nSpecType = Src.nSpecType;

    bVariable = Src.bVariable;
    bAbsolute = Src.bAbsolute;

    return *this;
}

// =========================================================
/** @fn		void SItemSpec::Init()
@brief		
@return		: 
@date		: 2005-09-14 오후 12:36:20
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
void SItemSpec::Init()
{
    bInsp = FALSE;

    fMarginalMin = 0.f;
    fPassMin = 0.f;
    fNominal = 0.f;
    fPassMax = 0.f;
    fMarginalMax = 0.f;

    // 스펙 타입은 초기화하지 않는다.
    //	nSpecType = MEASURE;
}

// =========================================================
/** @fn		void SItemSpec::Get(BOOL &_bInsp, float &_fMarginalMin, float &_fPassMin, float &_fNominal, float &_fPassMax, float &_fMarginalMax, long nUnit)
@brief		
@param		BOOL &_bInsp : 
@param		float &_fMarginalMin : 
@param		float &_fPassMin : 
@param		float &_fNominal : 
@param		float &_fPassMax : 
@param		float &_fMarginalMax : 
@param		long nUnit : 인수로 전달되는 스펙의 단위.
@return		: 
@date		: 2005-09-14 오후 12:36:23
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
void SItemSpec::Get(BOOL& _bInsp, float& _fMarginalMin, float& _fPassMin, float& _fNominal, float& _fPassMax,
    float& _fMarginalMax, long nUnit, BOOL& _bAbsolute) const
{
    _bInsp = bInsp;

    switch (nSpecType)
    {
        case MEASURE:
        {
            switch (nUnit)
            {
                case MM:
                    _fMarginalMin = fMarginalMin * g_fMilToMm;
                    _fPassMin = fPassMin * g_fMilToMm;
                    _fNominal = fNominal * g_fMilToMm;
                    _fPassMax = fPassMax * g_fMilToMm;
                    _fMarginalMax = fMarginalMax * g_fMilToMm;
                    break;
                case MIL:
                    _fMarginalMin = fMarginalMin;
                    _fPassMin = fPassMin;
                    _fNominal = fNominal;
                    _fPassMax = fPassMax;
                    _fMarginalMax = fMarginalMax;
                    break;
                case UM:
                    _fMarginalMin = fMarginalMin * g_fMilToUm;
                    _fPassMin = fPassMin * g_fMilToUm;
                    _fNominal = fNominal * g_fMilToUm;
                    _fPassMax = fPassMax * g_fMilToUm;
                    _fMarginalMax = fMarginalMax * g_fMilToUm;
                    break;
                default:
                    ASSERT(!"Invalid Spec Unit");
            }
        }
        break;
        case NUMBER:
        {
            _fMarginalMin = fMarginalMin;
            _fPassMin = fPassMin;
            _fNominal = fNominal;
            _fPassMax = fPassMax;
            _fMarginalMax = fMarginalMax;
        }
        break;
        case MEASURE2:
        {
            switch (nUnit)
            {
                case MM:
                    _fMarginalMin = fMarginalMin * g_fMilToMm * g_fMilToMm;
                    _fPassMin = fPassMin * g_fMilToMm * g_fMilToMm;
                    _fNominal = fNominal * g_fMilToMm * g_fMilToMm;
                    _fPassMax = fPassMax * g_fMilToMm * g_fMilToMm;
                    _fMarginalMax = fMarginalMax * g_fMilToMm * g_fMilToMm;
                    break;
                case MIL:
                    _fMarginalMin = fMarginalMin;
                    _fPassMin = fPassMin;
                    _fNominal = fNominal;
                    _fPassMax = fPassMax;
                    _fMarginalMax = fMarginalMax;
                    break;
                case UM:
                    _fMarginalMin = fMarginalMin * g_fMilToUm * g_fMilToUm;
                    _fPassMin = fPassMin * g_fMilToUm * g_fMilToUm;
                    _fNominal = fNominal * g_fMilToUm * g_fMilToUm;
                    _fPassMax = fPassMax * g_fMilToUm * g_fMilToUm;
                    _fMarginalMax = fMarginalMax * g_fMilToUm * g_fMilToUm;
                    break;
                default:
                    ASSERT(!"Invalid Spec Unit");
            }
        }
        break;
        case PERCENT:
        {
            _fMarginalMin = fMarginalMin;
            _fPassMin = fPassMin;
            _fNominal = fNominal;
            _fPassMax = fPassMax;
            _fMarginalMax = fMarginalMax;
        }
        break;
        case DEGREE:
        {
            _fMarginalMin = fMarginalMin;
            _fPassMin = fPassMin;
            _fNominal = fNominal;
            _fPassMax = fPassMax;
            _fMarginalMax = fMarginalMax;
        }
        break;
        default:
            ASSERT(!"Invalid Spec Type");
    }

    _bAbsolute = bAbsolute;
};

// =========================================================
/** @fn		void SItemSpec::Set(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fNominal, float _fPassMax, float _fMarginalMax, long nUnit)
@brief		
@param		BOOL _bInsp : 
@param		float _fMarginalMin : 
@param		float _fPassMin : 
@param		float _fNominal : 
@param		float _fPassMax : 
@param		float _fMarginalMax : 
@param		long nUnit : 결과로 받고싶은 스펙의 단위
@return		: 
@date		: 2005-09-14 오후 12:35:57
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
void SItemSpec::Set(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fNominal, float _fPassMax,
    float _fMarginalMax, long nUnit, BOOL _bAbsolute)
{
    // 스펙의 범위는 아래 조건을 항상 만족해야 한다.
    //	ASSERT(_fMarginalMin <= _fPassMin);
    //	ASSERT(_fPassMin <= _fNominal);
    //	ASSERT(_fNominal <= _fPassMax);
    //	ASSERT(_fPassMax <= _fMarginalMax);

    bInsp = _bInsp;

    switch (nSpecType)
    {
        case MEASURE:
            switch (nUnit)
            {
                case MM:
                    fMarginalMin = _fMarginalMin / g_fMilToMm;
                    fPassMin = _fPassMin / g_fMilToMm;
                    fNominal = _fNominal / g_fMilToMm;
                    fPassMax = _fPassMax / g_fMilToMm;
                    fMarginalMax = _fMarginalMax / g_fMilToMm;
                    break;
                case MIL:
                    fMarginalMin = _fMarginalMin;
                    fPassMin = _fPassMin;
                    fNominal = _fNominal;
                    fPassMax = _fPassMax;
                    fMarginalMax = _fMarginalMax;
                    break;
                case UM:
                    fMarginalMin = _fMarginalMin / g_fMilToUm;
                    fPassMin = _fPassMin / g_fMilToUm;
                    fNominal = _fNominal / g_fMilToUm;
                    fPassMax = _fPassMax / g_fMilToUm;
                    fMarginalMax = _fMarginalMax / g_fMilToUm;
                    break;
                default:
                    ASSERT(!"Invalid Spec Unit");
            }
            break;
        case NUMBER:
            fMarginalMin = _fMarginalMin;
            fPassMin = _fPassMin;
            fNominal = _fNominal;
            fPassMax = _fPassMax;
            fMarginalMax = _fMarginalMax;
            break;
        case MEASURE2:
            switch (nUnit)
            {
                case MM:
                    fMarginalMin = _fMarginalMin / g_fMilToMm / g_fMilToMm;
                    fPassMin = _fPassMin / g_fMilToMm / g_fMilToMm;
                    fNominal = _fNominal / g_fMilToMm / g_fMilToMm;
                    fPassMax = _fPassMax / g_fMilToMm / g_fMilToMm;
                    fMarginalMax = _fMarginalMax / g_fMilToMm / g_fMilToMm;
                    break;
                case MIL:
                    fMarginalMin = _fMarginalMin;
                    fPassMin = _fPassMin;
                    fNominal = _fNominal;
                    fPassMax = _fPassMax;
                    fMarginalMax = _fMarginalMax;
                    break;
                case UM:
                    fMarginalMin = _fMarginalMin / g_fMilToUm / g_fMilToUm;
                    fPassMin = _fPassMin / g_fMilToUm / g_fMilToUm;
                    fNominal = _fNominal / g_fMilToUm / g_fMilToUm;
                    fPassMax = _fPassMax / g_fMilToUm / g_fMilToUm;
                    fMarginalMax = _fMarginalMax / g_fMilToUm / g_fMilToUm;
                    break;
                default:
                    ASSERT(!"Invalid Spec Unit");
            }
            break;
        case PERCENT:
            fMarginalMin = _fMarginalMin;
            fPassMin = _fPassMin;
            fNominal = _fNominal;
            fPassMax = _fPassMax;
            fMarginalMax = _fMarginalMax;
            break;
        case DEGREE:
            fMarginalMin = _fMarginalMin;
            fPassMin = _fPassMin;
            fNominal = _fNominal;
            fPassMax = _fPassMax;
            fMarginalMax = _fMarginalMax;
            break;
        default:
            ASSERT(!"Invalid Spec Type");
    }

    bAbsolute = _bAbsolute;
};

void SItemSpec::Link(BOOL** _pbInsp, float** _pfMarginalMin, float** _pfPassMin, float** _pfNominal, float** _pfPassMax,
    float** _pfMarginalMax)
{
    if (_pbInsp)
        *_pbInsp = &bInsp;
    if (_pfMarginalMin)
        *_pfMarginalMin = &fMarginalMin;
    if (_pfPassMin)
        *_pfPassMin = &fPassMin;
    if (_pfNominal)
        *_pfNominal = &fNominal;
    if (_pfPassMax)
        *_pfPassMax = &fPassMax;
    if (_pfMarginalMax)
        *_pfMarginalMax = &fMarginalMax;
}

//long SItemSpec::GetSpecType() const
//{
//	return nSpecType;
//}

//void SItemSpec::SetSpecType(enumSpecType _nSpecType)
//{
//	ASSERT(nSpecType >= MEASURE && nSpecType <= DEGREE);
//
//	nSpecType = _nSpecType;
//}

LPCTSTR SItemSpec::GetSpecTypeName(long nUnit) const
{
    switch (nSpecType)
    {
        case MEASURE:
            switch (nUnit)
            {
                case MM:
                    return _T("mm");
                case MIL:
                    return _T("mil");
                case UM:
                    return _T("um");
                default:
                    ASSERT(!"Invalid Unit");
            }
            break;
        case NUMBER:
            return _T("");
        case MEASURE2:
            switch (nUnit)
            {
                case MM:
                    return _T("mm^2");
                case MIL:
                    return _T("mil^2");
                case UM:
                    return _T("um^2");
                default:
                    ASSERT(!"Invalid Unit");
            }
            break;
        case PERCENT:
            return _T("%");
        case DEGREE:
            return _T("Deg");
        default:
            ASSERT(!"Invalid Spec Type");
    }

    return _T("");
}

long SItemSpec::GetSpecType() const
{
    return nSpecType;
}

void SItemSpec::SetSpecType(long nNewSpecType) //kircheis_Grid
{
    nSpecType = nNewSpecType;
}

long SItemSpec::IntToUnitEnum(long nUnit)
{
    switch (nUnit)
    {
        case 0:
            return SItemSpec::MM;
        case 1:
            return SItemSpec::MIL;
        case 2:
            return SItemSpec::UM;
        default:
            ASSERT(!"xx");
    }

    return SItemSpec::MIL;
}

long SItemSpec::IntToTypeEnum(long nType)
{
    switch (nType)
    {
        case 0:
            return SItemSpec::MEASURE;
        case 1:
            return SItemSpec::NUMBER;
        case 2:
            return SItemSpec::MEASURE2;
        case 3:
            return SItemSpec::PERCENT;
        case 4:
            return SItemSpec::DEGREE;
        default:
            ASSERT(!"xx");
    }

    return SItemSpec::MEASURE;
}

/*
void SItemSpec::Get(SItemSpec &Dst)
{
Dst.bInsp = bInsp;
Dst.fMarginalMin = fMarginalMin;
Dst.m_passMin = m_passMin;
Dst.fNominal = fNominal;
Dst.m_passMax = m_passMax;
Dst.fMarginalMax = fMarginalMax;
};

void SItemSpec::Set(const SItemSpec &Src)
{
ASSERT(Src.fMarginalMin <= Src.m_passMin);
ASSERT(Src.m_passMin <= Src.fNominal);
ASSERT(Src.fNominal <= Src.m_passMax);
ASSERT(Src.m_passMax <= Src.fMarginalMax);

bInsp = Src.bInsp;
fMarginalMin = Src.fMarginalMin;
m_passMin = Src.m_passMin;
fNominal = Src.fNominal;
m_passMax = Src.m_passMax;
fMarginalMax = Src.fMarginalMax;
};
*/

// =========================================================
/** @fn		long SItemSpec::Judge(float fValue)
@brief		
@param		float fValue : 
@return		: 
@date		: 2005-09-14 오후 12:36:01
@author		: 
@warning	: 
@sa			: 
@note		: 
*/ // ======================================================
long SItemSpec::Judge(float fValue) const
{
    // 검사하게 되어있지 않은데도 이 함수가 호출된다면
    // 코드 로직에 문제가 있다는 얘기임.
    // bInsp 이 FALSE 인데 이 함수가 호출되면 안됨.
    // 예외 처리를 위해서 bInsp 이 FALSE 인 경우 NOT_MEASURED 를 반환도록 함.

    ASSERT(bInsp);

    if (bInsp)
    {
        if (fValue < fMarginalMin || fValue > fMarginalMax)
        {
            return REJECT;
        }
        else if (fValue < fPassMin || fValue > fPassMax)
        {
            return MARGINAL;
        }
        else
        {
            return PASS;
        }
    }
    else
    {
        return NOT_MEASURED;
    }
};
