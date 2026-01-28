#include "StdAfx.h"
#include "PassiveJob.h"
#include "../../dA_Modules/dA_Base/ChipDB.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CPassiveJobInfo::CPassiveJobInfo()
{
	Open();
}

void CPassiveJobInfo::AddType()
{
	ChipDB ChipDB;
	
	ChipDB.strModelName = _T("Untile");
	ChipDB.fChipWidth = 0.f;
	ChipDB.fChipLength = 0.f;

	srand(time(NULL));
	ChipDB.GroupColor = RGB(rand()%255, rand()%255, rand()%255);

	m_vecsChipData.push_back(ChipDB);
}

void CPassiveJobInfo::Save()
{
	CString strDestFilePath(CHIP_DB_DIRECTORY _T("ChipDataBase.cdb"));

	CFile File;
	if(TRUE == File.Open(strDestFilePath, CFile::modeCreate|CFile::modeWrite))
	{
		CArchive ar(&File, CArchive::store);

		long nDataNum = (long)m_vecsChipData.size();
		ar << nDataNum;

		for(long nData=0 ; nData<nDataNum ; nData++)
		{
			ar << m_vecsChipData[nData];
		}

		ar.Close();
		File.Close();
	}
}

void CPassiveJobInfo::Open()
{
	CString strDestFilePath(CHIP_DB_DIRECTORY _T("ChipDataBase.cdb"));

	CFile File;
	if(TRUE == File.Open(strDestFilePath, CFile::modeRead))
	{
		CArchive ar(&File, CArchive::load);

		long nDataNum;

		ar >> nDataNum;
		m_vecsChipData.resize(nDataNum);

		for(long nData=0 ; nData<nDataNum ; nData++)
		{
			ar >> m_vecsChipData[nData];
		}

		ar.Close();
		File.Close();
	}
}