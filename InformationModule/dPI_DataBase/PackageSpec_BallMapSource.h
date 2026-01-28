#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

#include <vector>
#include "PackageSpec_BallSource.h"

class CiDataBase;

struct DPI_DATABASE_API PackageSpec_BallTypeDefinition
{
public:
	PackageSpec_BallTypeDefinition() = delete;
	PackageSpec_BallTypeDefinition(const PackageSpec_BallTypeDefinition &rhs) = default;
	PackageSpec_BallTypeDefinition(const double diameter_um, const double height_um);
	~PackageSpec_BallTypeDefinition() = default;

	PackageSpec_BallTypeDefinition &operator=(const PackageSpec_BallTypeDefinition &rhs) = default;

	BOOL LinkDataBase(BOOL bSave, CiDataBase &db);

public:
	double m_diameter_um;
	double m_height_um;
};

class DPI_DATABASE_API PackageSpec_BallMapSource
{
public:
	PackageSpec_BallMapSource();
	PackageSpec_BallMapSource(const PackageSpec_BallMapSource &rhs) = default;
	PackageSpec_BallMapSource(LPCTSTR pathName, CString &errorString);
	~PackageSpec_BallMapSource() = default;

	PackageSpec_BallMapSource &operator=(const PackageSpec_BallMapSource &rhs) = default;

	void UpdateTypeInfo();
	BOOL LinkDataBase(BOOL bSave, CiDataBase &db);

	BOOL Export(LPCTSTR pathName);

public:	// Ball Map 파일 로딩에서 결정되는 것.
	std::vector<PackageSpec_BallSource> m_balls;
	std::vector<PackageSpec_BallTypeDefinition>	m_ballTypes;	// Ball map 에 있는 Ball 의 Diameter 를 큰 녀석부터 내림 차순으로 정렬
																// 이 배열의 인덱스가 PackageSpec_BallSource::m_typeIndex 에 사용됨

	BOOL	m_validBallMap;

public: // 사용자 설정으로 변경되는 것
	// Flip 을 먼저 적용한 후, Rotation 을 적용하는 것을 기준으로 한다.
	BOOL m_ballMapFlipX;
	BOOL m_ballMapFlipY;

	enum class RotationIndex : long
	{
		Deg_0 = 0,
		Deg_90,
		Deg_180,
		Deg_270,
	};

	RotationIndex m_ballMapRotation;
};

