#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

class CiDataBase;

// 볼 위치는 유닛의 중앙을 원점으로 하는 오른손 좌표계를 사용한다.

class DPI_DATABASE_API PackageSpec_BallSource
{
public:
	PackageSpec_BallSource() = delete;
	PackageSpec_BallSource(const PackageSpec_BallSource &rhs) = default;
	PackageSpec_BallSource(LPCSTR name, long index, double posX_um, double posY_um, double diameter_um, double height_um);
	~PackageSpec_BallSource() = default;

	PackageSpec_BallSource &operator=(const PackageSpec_BallSource &rhs) = default;

	BOOL LinkDataBase(BOOL bSave, CiDataBase &db);

	CString m_name;
	long m_index;				// Ball map 상에서 몇 번째인지를 나타냄..순서.
	double m_posX_um;
	double m_posY_um;
	double m_diameter_um;
	double m_height_um;
	bool m_ignored;				// true 면 검사에서 제외

	long m_typeIndex;			// Diameter 별로 다른 알고리즘 파라미터를 적용해야 하는 경우가 있음
								// 이때, 어떤 Diameter 에 해당하는 알고리즘을 사용해야 하는지를 빠르게 결정하기 위해서
								// Diameter 종류에 대한 인덱스를 가지고 있게 함.
								// 이 인덱스는 가장 큰 Ball 을 0 으로 하는 내림차순 정렬임.
};

