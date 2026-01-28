#pragma once

//HDR_0_________________________________ Configuration header
#include "_libsetup.h"

//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
class CiDataBase;

//HDR_6_________________________________ Header body
//
struct __DPI_SURFACE_API__ SItemSpec
{
public:
    // 스펙 값의 종류
    enum enumSpecType
    {
        MEASURE, // 0. 실측 치수값. mil, um, mm 등의..
        NUMBER, // 1. 무단위수
        MEASURE2, // 2. 실측 면적값. mil^2, um^2, mm^2 등의..
        PERCENT, // 3. %
        DEGREE, // 4. Degree
    };

    // 실측 치수값의 단위
    enum enumSpecUnit
    {
        MM, // 0. mm
        MIL, // 1. mil
        UM, // 2. um
    };

public:
    // Constructor, Destructor
    SItemSpec();
    SItemSpec(const SItemSpec& Src);
    SItemSpec(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fNominal, float _fPassMax, float _fMarginalMax,
        long _nSpecType, BOOL _bVariable, BOOL _bAbsolute);
    SItemSpec(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fPassMax, float _fMarginalMax, long _nSpecType,
        BOOL _bVariable, BOOL _bAbsolute);

    // Copy Operator
    SItemSpec& operator=(const SItemSpec& Src);

    // General Interfaces
    void Init();

    void Get(BOOL& _bInsp, float& _fMarginalMin, float& _fPassMin, float& _fNominal, float& _fPassMax,
        float& _fMarginalMax, long nUnit, BOOL& _bAbsolute) const;
    void Set(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fNominal, float _fPassMax, float _fMarginalMax,
        long nUnit, BOOL _bAbsolute);
    // 	void Get(BOOL &_bInsp, float &_fMarginalMin, float &_fPassMin, float &_fNominal, float &_fPassMax, float &_fMarginalMax, long nUnit, BOOL& _bVariable, BOOL& _bAbsolute) const;
    //void Set(BOOL _bInsp, float _fMarginalMin, float _fPassMin, float _fPassMax, float _fMarginalMax, long nUnit, BOOL _bVariable, BOOL _bAbsolute);
    void Link(BOOL** _pbInsp, float** _pfMarginalMin, float** _pfPassMin, float** _pfNominal, float** _pfPassMax,
        float** _pfMarginalMax);

    long Judge(float fValue) const;

    // Attributes Interfaces
    BOOL GetInsp() const
    {
        return bInsp;
    };
    void SetInsp(BOOL _bInsp)
    {
        bInsp = _bInsp;
    };

    float GetMarginalMin() const
    {
        return fMarginalMin;
    };
    float GetPassMin() const
    {
        return fPassMin;
    };
    float GetNominal() const
    {
        return fNominal;
    };
    float GetPassMax() const
    {
        return fPassMax;
    };
    float GetMarginalMax() const
    {
        return fMarginalMax;
    };

    BOOL IsVariableType() const
    {
        return bVariable;
    };
    BOOL IsAbsoluteType() const
    {
        return bAbsolute;
    };
    BOOL GetVariableType() const
    {
        return bVariable;
    };
    BOOL GetAbsoluteType() const
    {
        return bAbsolute;
    };
    void SetVariableType(BOOL _bVariable)
    {
        bVariable = _bVariable;
    };
    void SetAbsoluteType(BOOL _bAbsolute)
    {
        bAbsolute = _bAbsolute;
    };
    void SetVariAbsType(BOOL _bVariable, BOOL _bAbsolute)
    {
        bVariable = _bVariable;
        bAbsolute = _bAbsolute;
    };
    void GetVariAbsType(BOOL& _bVariable, BOOL& _bAbsolute)
    {
        _bVariable = bVariable;
        _bAbsolute = bAbsolute;
    };

    LPCTSTR GetSpecTypeName(long nUnit) const;
    long GetSpecType() const;
    void SetSpecType(long nNewSpecType); //kircheis_Grid

    long IntToUnitEnum(long nUnit);
    long IntToTypeEnum(long nType);

    static const float fLimitMin;
    static const float fLimitMax;

    void Limit();

    BOOL LinkDataBase(BOOL save, CiDataBase& db);

private:
    // Attributes
    BOOL bInsp;

    // 이 아이템의 순서는 스펙 범위의 의미를 가지고 있으므로 바꾸지 않는다.
    //
    //		-----------<-----------<----------|----------<----------<-----------------
    //			   MarginalMin  PassMin    Nominal    PassMax  MarginalMax
    //
    float fMarginalMin;
    float fPassMin;
    float fNominal;
    float fPassMax;
    float fMarginalMax;

    long nSpecType;

    BOOL bVariable; // 가변형 Spec임을 나타낸다. True면 bAbsolue를 변화 시킬수 있다
    // 이 변수는 클래스 오브젝트 생성시에 결정된 후에 바뀌면 안되므로
    // 시리얼라이즈를 하지 않는다.

    BOOL
        bAbsolute; // True(Absolute)면 실제 계산된 값을 Result에 저장. False(Relative)면 Package Spec 에서 벗어난 양을 Result에 저장.

    // 기타 나중을 위해 예약함.
    // 적당한 멤버가 추가될 때마다
    // 여기서 빼서 쓴다.
    //	char _ReservedData[__nReservedDataLen];

    // 	friend CArchive& operator<<(CArchive& ar, const SItemSpec& Spec);
    // 	friend CArchive& operator>>(CArchive& ar, SItemSpec& Spec);
};
