#include "StdAfx.h"
#include "PackageSpec_BallMapSource.h"

#include "../../VisionHostCommon/DBObject.h"

#include <algorithm>
#include <fstream>
#include <functional>
#include <map>
#include <set>

#include "Utility/ThreadedProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

PackageSpec_BallTypeDefinition::PackageSpec_BallTypeDefinition(const double diameter_um, const double height_um)
	: m_diameter_um(diameter_um)
	, m_height_um(height_um)
{

}

BOOL PackageSpec_BallTypeDefinition::LinkDataBase(BOOL bSave, CiDataBase &db)
{
	db[_T("{2559511F-14C6-4BD5-8A80-561C8069AE81}")].Link(bSave, m_diameter_um);
	db[_T("{C6F1A3E3-3F06-44B3-A7EF-E704A151A250}")].Link(bSave, m_height_um);

	return TRUE;
}

PackageSpec_BallMapSource::PackageSpec_BallMapSource()
	: m_validBallMap(FALSE)
	, m_ballMapFlipX(FALSE)
	, m_ballMapFlipY(FALSE)
	, m_ballMapRotation(RotationIndex::Deg_0)
{
}

PackageSpec_BallMapSource::PackageSpec_BallMapSource(LPCTSTR pathName, CString &errorString)
	: m_validBallMap(FALSE)
	, m_ballMapFlipX(FALSE)
	, m_ballMapFlipY(FALSE)
	, m_ballMapRotation(RotationIndex::Deg_0)
{
	IPVM::ThreadedProgress progress(_T("Load ball map file"));

	CFile file;

	if (!file.Open(pathName, CFile::modeRead))
	{
		errorString = _T("Cannot open file !!!");
		return;
	}

	if (file.GetLength() < 10)
	{
		errorString = _T("File length is too short !!!");
		return;
	}

	// Check Encoding
	char temp[10];
	file.Read(temp, 10);
	file.Close();

	// Read
	bool isWindowsUnicode = (temp[0] == -1 && temp[1] == -2)		// BOM Check
		&& !(temp[3] | temp[5] | temp[7] | temp[9]);				// 영문자, 숫자, 쉼표만 있으므로 0 이어야 한다.

	if (isWindowsUnicode)
	{
		errorString = _T("Invalid file encoding");
		return;
	}

	typedef std::basic_string<char>::size_type temp_string_size_type;

	std::ifstream infile(pathName);
	bool headerRead = false;

	char buffer[1000];

	enum fixColumn
	{
		FX_NAME,
		FX_X,
		FX_Y,
		FX_DIAMETER,
		FX_HEIGHT,
		FX_END,
	};

	long ballIndex = 0;
	long columnLink[FX_END];
	long headerColumnCount = 0;
	long rowIndex = 0;

	std::map<CStringA, long> mapDieNameToIndex;

	while (infile.getline(buffer, 1000))
	{
		rowIndex++;

		std::basic_string<char> temp(buffer);

		temp_string_size_type findStart = 0;
		temp_string_size_type findEnd = 0;

		if (!headerRead)
		{
			//---------------------------------------------------------------------------------------
			// Read Header
			//---------------------------------------------------------------------------------------

			std::map<CStringA, long> headerIndexMap;
			long index = 0;
			while (1)
			{
				findEnd = temp.find_first_of(',', findStart);
				CStringA text;
				if (findEnd == std::basic_string<char>::npos)
				{
					text.SetString(buffer + findStart, temp.length() - findStart);
					headerIndexMap[text] = index;
					break;
				}
				else
				{
					text.SetString(buffer + findStart, findEnd - findStart);
					headerIndexMap[text] = index;
					findStart = findEnd + 1;
					index++;
				}
			}

			// 헤더 정보중 필수 항목들이 모두 들어왔는지 확인한다

			for (long item = 0; item < FX_END; item++)
			{
				CStringA columnName;
				switch (item)
				{
				case FX_NAME:			columnName = _T("PinName");		break;
				case FX_X:				columnName = _T("Xcoord(mm)");	break;
				case FX_Y:				columnName = _T("Ycoord(mm)");	break;
				case FX_DIAMETER:		columnName = _T("Diameter");	break;
				case FX_HEIGHT:			columnName = _T("Height");		break;
				}

				if (columnName.IsEmpty()) break;

				if (headerIndexMap.find(columnName) == headerIndexMap.end())
				{
					errorString.Format(_T("'%s' Column not found."), LPCTSTR(CString(columnName)));
					return;
				}

				columnLink[item] = headerIndexMap[columnName];
			}

			headerColumnCount = long(headerIndexMap.size());
			headerRead = true;
			continue;
		}

		std::vector<CStringA> dataList;
		while (1)
		{
			findEnd = temp.find_first_of(',', findStart);
			CStringA text;
			if (findEnd == std::basic_string<char>::npos)
			{
				text.SetString(buffer + findStart, temp.length() - findStart);
				dataList.push_back(text);
				break;
			}
			else
			{
				text.SetString(buffer + findStart, findEnd - findStart);
				dataList.push_back(text);
				findStart = findEnd + 1;
			}
		}

		if (dataList.size())
		{
			if (dataList.size() != headerColumnCount)
			{
				// 헤더 컬럼과 데이터 컴럼의 수가 다르다				
				errorString.Format(_T("The number of columns is different. (%d Line)"), rowIndex);
				return;
			}

			m_balls.emplace_back
			(
				dataList[columnLink[FX_NAME]],
				ballIndex,
				::atof(dataList[columnLink[FX_X]]) * 1000.,
				::atof(dataList[columnLink[FX_Y]]) * 1000.,
				::atof(dataList[columnLink[FX_DIAMETER]]) * 1000.,
				::atof(dataList[columnLink[FX_HEIGHT]]) * 1000.
			);

			ballIndex++;
		}
	}

	if (m_balls.size() <= 0)
	{
		errorString = _T("No balls !!!");
		return;
	}

	progress.Post(_T("Consistency check"));

	// Diameter 종류 체크
	std::set<float> diameters;

	for (const auto &ball : m_balls)
	{
		diameters.insert(ball.m_diameter_um);
	}

	if (diameters.size() > 10)
	{
		errorString = _T("Too many diameters !!!");
		return;
	}

	// 아이디 중복 체크
	{
		std::map<CString, long> indexCounts;

		for (const auto &ball : m_balls)
		{
			indexCounts[ball.m_name]++;
		}

		CString strDuplicatedIDs(_T("Ball ID duplicated!!\n\n"));
		long duplicatedIDCounts = 0;

		for (const auto &pair : indexCounts)
		{
			if (pair.second > 1)
			{
				if (duplicatedIDCounts > 10)
				{
					strDuplicatedIDs += _T(".....");
					break;
				}

				CString str;
				str.Format(_T("%s\n"), LPCTSTR(pair.first));

				strDuplicatedIDs += str;
				duplicatedIDCounts++;
			}
		}

		if (duplicatedIDCounts > 0)
		{
			errorString = strDuplicatedIDs;
			return;
		}
	}

	// 볼 좌표 중복 체크
	{
		std::map<std::pair<double, double>, long> coordCounts;

		for (const auto &ball : m_balls)
		{
			coordCounts[std::make_pair(ball.m_posX_um, ball.m_posY_um)]++;
		}

		CString strDuplicatedBallCoords(_T("Ball coordination duplicated!!\n\n"));
		long duplicatedBallCounts = 0;

		for (const auto &pair : coordCounts)
		{
			if (pair.second > 1)
			{
				if (duplicatedBallCounts > 10)
				{
					strDuplicatedBallCoords += _T(".....");
					break;
				}

				CString str;
				str.Format(_T("X:%.2lf Y:%.2lf Count:%d\n"), pair.first.first, pair.first.second, pair.second);

				strDuplicatedBallCoords += str;
				duplicatedBallCounts++;
			}
		}

		if (duplicatedBallCounts > 0)
		{
			errorString = strDuplicatedBallCoords;
			return;
		}
	}

	progress.Post(_T("Post processing"));

	// Diameter 리스트를 만들고, 어떤 Ball 이 어떤 Diameter Index 를 가지는지 정리한다.
	UpdateTypeInfo();

	m_validBallMap = true;

	return;
}

void PackageSpec_BallMapSource::UpdateTypeInfo()
{
	// Diameter Index 생성
	for (const auto &ball : m_balls)
	{
		bool found = false;

		for (long j = 0; j < (long)m_ballTypes.size(); j++)
		{
			if (ball.m_diameter_um == m_ballTypes[j].m_diameter_um)
			{
				found = true;
				break;
			}
		}

		if (found == false)
		{
			m_ballTypes.emplace_back(ball.m_diameter_um, ball.m_height_um);
		}
	}

	// 큰 녀석 기준으로 정렬
	std::sort(m_ballTypes.begin(), m_ballTypes.end(), [](const PackageSpec_BallTypeDefinition &lhs, const PackageSpec_BallTypeDefinition &rhs)
	{
		return lhs.m_diameter_um > rhs.m_diameter_um;
	});

	// 각 범프 정보에 Diameter Index 를 넣어준다.
	for (auto &ball : m_balls)
	{
		for (long j = 0; j < (long)m_ballTypes.size(); j++)
		{
			if (ball.m_diameter_um == m_ballTypes[j].m_diameter_um)
			{
				ball.m_typeIndex = j;
				break;
			}
		}
	}

	return;
}

BOOL PackageSpec_BallMapSource::LinkDataBase(BOOL bSave, CiDataBase &db)
{
	// m_balls
	{
		long count = long(m_balls.size());

		db[_T("{7406DD17-E425-4396-9DCD-1203B09E299A}")].Link(bSave, count);

		m_balls.resize(count, PackageSpec_BallSource(nullptr, 0, 0., 0., 0., 0.));

		for (long n = 0; n < count; n++)
		{
			m_balls[n].LinkDataBase(bSave, db.GetSubDBFmt(_T("{7406DD17-E425-4396-9DCD-1203B09E299A}_%d"), n));
		}
	}

	// m_ballTypes
	{
		long count = long(m_ballTypes.size());

		db[_T("{4255FCF6-3146-4A0A-9F40-EB33C3CFE99B}")].Link(bSave, count);

		m_ballTypes.resize(count, PackageSpec_BallTypeDefinition(0., 0.));

		for (long n = 0; n < count; n++)
		{
			m_ballTypes[n].LinkDataBase(bSave, db.GetSubDBFmt(_T("{4255FCF6-3146-4A0A-9F40-EB33C3CFE99B}_%d"), n));
		}
	}

	db[_T("{84158BBE-3969-4584-A93C-7A1D7C101212}")].Link(bSave, m_validBallMap);
	db[_T("{A7307333-D758-4393-80BD-108C37B42C48}")].Link(bSave, m_ballMapFlipX);
	db[_T("{AF4CB709-14EC-494B-B3E8-6C5C3B8CF7AD}")].Link(bSave, m_ballMapFlipY);
	db[_T("{16953E1A-2524-4AD3-9F0C-D67CD55FC7D1}")].Link(bSave, *(long *)&m_ballMapRotation);

	return TRUE;
}

BOOL PackageSpec_BallMapSource::Export(LPCTSTR pathName)
{
	CFile file;

	if (!file.Open(pathName, CFile::modeCreate|CFile::modeWrite))
	{
		return FALSE;
	}

	file.Close();

	typedef std::basic_string<char>::size_type temp_string_size_type;

	std::ofstream outfile(pathName);

	outfile << "PinName,Xcoord(mm),Ycoord(mm),Diameter,Height" << std::endl;

	for (const auto &ball : m_balls)
	{
		outfile << CStringA(ball.m_name).GetBuffer() << "," << ball.m_posX_um * 0.001 << "," << ball.m_posY_um * 0.001 << "," << ball.m_diameter_um * 0.001 << "," << ball.m_height_um * 0.001 << std::endl;
	}

	return TRUE;
}

CArchive &operator<<(CArchive &ar, const PackageSpec_BallMapSource &Src)
{
	long nVersion = 20180308;

	ar << nVersion;

	switch (nVersion)
	{
	case 20180308:
		ar << Src.m_validBallMap;
		ar << Src.m_ballMapFlipX;
		ar << Src.m_ballMapFlipY;
		ar << (long)Src.m_ballMapRotation;
		ar << (int)Src.m_balls.size();
		for (long n = 0; n < (long)Src.m_balls.size(); n++)
		{
			ar << Src.m_balls[n].m_name;
			ar << Src.m_balls[n].m_posX_um;
			ar << Src.m_balls[n].m_posY_um;
			ar << Src.m_balls[n].m_diameter_um;
			ar << Src.m_balls[n].m_height_um;
			ar << Src.m_balls[n].m_index;
			ar << Src.m_balls[n].m_typeIndex;
		}
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Ball Map Origin"));
		break;
	}

	return ar;
}

CArchive &operator>>(CArchive &ar, PackageSpec_BallMapSource &Dst)
{
	long nVersion = 20180308;

	//ar >> nVersion;

	switch (nVersion)
	{
	case 20180308:
		int nSize;
		long ballRotation;
		ar >> Dst.m_validBallMap;
		ar >> Dst.m_ballMapFlipX;
		ar >> Dst.m_ballMapFlipY;
		ar >> ballRotation;
		Dst.m_ballMapRotation = PackageSpec_BallMapSource::RotationIndex(ballRotation);
		ar >> nSize;
		if (nSize > 0)
		{
			Dst.m_balls.resize(nSize, PackageSpec_BallSource(nullptr, 0, 0., 0., 0., 0.));
			for (long n = 0; n < nSize; n++)
			{
				ar >> Dst.m_balls[n].m_name;
				ar >> Dst.m_balls[n].m_posX_um;
				ar >> Dst.m_balls[n].m_posY_um;
				ar >> Dst.m_balls[n].m_diameter_um;
				ar >> Dst.m_balls[n].m_height_um;
				ar >> Dst.m_balls[n].m_index;
				ar >> Dst.m_balls[n].m_typeIndex;
			}
		}
		break;
	default:
		::AfxThrowArchiveException(CArchiveException::badSchema, _T("Ball Map Origin"));
		break;
	}

	return ar;
}