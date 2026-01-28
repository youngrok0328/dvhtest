#include "stdafx.h"

#include "PackageSpec_CustomMaster.h"
#include "FigureFactory.h"
#include "Figure_Ellipse.h"
#include "Figure_Rectangle.h"
#include "Figure_Polygon.h"

#include "../../VisionHostCommon/DBObject.h"

#include <algorithm>
#include <iomanip>
#include <fstream>
#include <string>
#include <cmath>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SCustomInfoMaster::SCustomInfoMaster()
: strLayerName(SCustomInfoMaster::GetDefault_strLayerName())
, fOffsetX(0.f)
, fOffsetY(0.f)
{
}

SCustomInfoMaster::SCustomInfoMaster(const SCustomInfoMaster &Src)
: strLayerName(LPCTSTR(Src.strLayerName))
, fOffsetX(Src.fOffsetX)
, fOffsetY(Src.fOffsetY)
{
	Copy(Src);
}

SCustomInfoMaster::~SCustomInfoMaster()
{
	Init();
}

SCustomInfoMaster &SCustomInfoMaster::operator=(const SCustomInfoMaster &Src)
{
	Init();

	Copy(Src);

	return *this;
}

void SCustomInfoMaster::Init()
{
	std::vector<CFigure*>::iterator itr = vecElement.begin();

	while(itr != vecElement.end())
	{
		delete *itr;
		itr++;
	}

	vecElement.clear();

	strLayerName = SCustomInfoMaster::GetDefault_strLayerName();

	fOffsetX = 0.f;
	fOffsetY = 0.f;
}

void SCustomInfoMaster::Copy(const SCustomInfoMaster &Src)
{
	vecElement.reserve((long)Src.vecElement.size());

	std::vector<CFigure*>::const_iterator itr = Src.vecElement.begin();

	while(itr != Src.vecElement.end())
	{
		vecElement.push_back((*itr)->Clone());
		itr++;
	}

	strLayerName = LPCTSTR(Src.strLayerName);

	fOffsetX = Src.fOffsetX;
	fOffsetY = Src.fOffsetY;
}

void SCustomInfoMaster::ImportGerberTxt(LPCTSTR szPathName)
{
	float x, y, p1, p2;

	std::ifstream stream(szPathName);

	if(!stream)
	{
		return;
	}

	std::string key;

	while(stream.good())
	{
		stream >> key;

		if(key == "CIRCLE")
		{
			stream >> x >> y >> p1;		// mils
			CEllipse *pEllipse = (CEllipse *)CFigureFactory::Produce(CEllipse::GetGUIDStatic());
			pEllipse->fCenX = x;
			pEllipse->fCenY = y;
			pEllipse->fAxisLenX = p1;
			pEllipse->fAxisLenY = p1;
			vecElement.push_back(pEllipse);
		}
		else if(key == "RECTANGLE")
		{
			stream >> x >> y >> p1 >> p2;
			CRectangle *pRectangle = (CRectangle *)CFigureFactory::Produce(CRectangle::GetGUIDStatic());
			pRectangle->fCenX = x;
			pRectangle->fCenY = y;
			pRectangle->fAxisLenX = p1;
			pRectangle->fAxisLenY = p2;
			vecElement.push_back(pRectangle);
		}
		else if(key == "OBROUND")
		{
			// 임시로 Rectangle 취급함.
			stream >> x >> y >> p1 >> p2;
			CRectangle *pRectangle = (CRectangle *)CFigureFactory::Produce(CRectangle::GetGUIDStatic());
			pRectangle->fCenX = x;
			pRectangle->fCenY = y;
			pRectangle->fAxisLenX = p1;
			pRectangle->fAxisLenY = p2;
			vecElement.push_back(pRectangle);
		}
		else if(key == "POLYGON")
		{
			long nVertexCount = 0;
			stream >> nVertexCount;

			std::vector<float> vecx(nVertexCount);
			std::vector<float> vecy(nVertexCount);

			for(long nVertex = 0 ; nVertex < nVertexCount ; nVertex++)
			{
				stream >> vecx[nVertex] >> vecy[nVertex];
			}

			if(nVertexCount == 4)
			{
				// Rectangle 로 변환 가능한 경우에 Rectangle 로 저장
				float fInnerProduct = 0.f;
				fInnerProduct += fabs((vecx[1] - vecx[0]) * (vecx[2] - vecx[1]) + (vecy[1] - vecy[0]) * (vecy[2] - vecy[1]));
				fInnerProduct += fabs((vecx[2] - vecx[1]) * (vecx[3] - vecx[2]) + (vecy[2] - vecy[1]) * (vecy[3] - vecy[2]));
				fInnerProduct += fabs((vecx[3] - vecx[2]) * (vecx[0] - vecx[3]) + (vecy[3] - vecy[2]) * (vecy[0] - vecy[3]));
				fInnerProduct += fabs((vecx[0] - vecx[3]) * (vecx[1] - vecx[0]) + (vecy[0] - vecy[3]) * (vecy[1] - vecy[0]));

				if(fInnerProduct < 0.000000001)
				{
					float fMinX = min(min(vecx[0], vecx[1]), min(vecx[2], vecx[3]));
					float fMaxX = max(max(vecx[0], vecx[1]), min(vecx[2], vecx[3]));
					float fMinY = min(min(vecy[0], vecy[1]), min(vecy[2], vecy[3]));
					float fMaxY = max(max(vecy[0], vecy[1]), min(vecy[2], vecy[3]));
					float p1 = fMaxX - fMinX;
					float p2 = fMaxY - fMinY;

					CRectangle *pRectangle = (CRectangle *)CFigureFactory::Produce(CRectangle::GetGUIDStatic());
					pRectangle->fCenX = 0.5f * (fMinX + fMaxX);
					pRectangle->fCenY = 0.5f * (fMinY + fMaxY);
					pRectangle->fAxisLenX = p1;
					pRectangle->fAxisLenY = p2;
					vecElement.push_back(pRectangle);
				}
				else
				{
					CPolygon *pPolygon = (CPolygon *)CFigureFactory::Produce(CPolygon::GetGUIDStatic());

					pPolygon->vecx = vecx;
					pPolygon->vecy = vecy;

					vecElement.push_back(pPolygon);
				}
			}
			else
			{
				CPolygon *pPolygon = (CPolygon *)CFigureFactory::Produce(CPolygon::GetGUIDStatic());

				pPolygon->vecx = vecx;
				pPolygon->vecy = vecy;

				vecElement.push_back(pPolygon);
			}
		}
	}
}

LPCTSTR SCustomInfoMaster::GetDefault_strLayerName()
{
	return _T("Untitled Layer");
}

// Save & Load...
BOOL SCustomInfoMaster::LinkDataBase(BOOL bSave, CiDataBase &db, long nCount)
{
	std::vector<CFigure*> vecElement;

	CString strTemp;

	strTemp.Format(_T("CustomInfoMaster_%d_"), nCount);

	if(!db[strTemp + _T("Layer Name")].Link(bSave, strLayerName)) strLayerName = _T("Untitled Layer");
	if(!db[strTemp + _T("Offset X")].Link(bSave, fOffsetX)) fOffsetX = 0.f;
	if(!db[strTemp + _T("Offset Y")].Link(bSave, fOffsetY)) fOffsetY = 0.f;

	//if(!bSave)
	//{
	//	strTemp.Format(_T("CustomInfoMaster_%d_Element_Size"), nCount);
	//	if(!db[strTemp].Link(bSave, nSize)) nSize = 0;
	//}
	//else
	//{
	//	nSize = (long)(vecElement.size());
	//	if(!db[strTemp].Link(bSave, nSize)) nSize = 0;
	//}
	//for(long i = 0 ; i < nSize ; i++)
	//{
	//	strTemp.Format(_T("CustomInfoMaster_%d_Element_%d"), nCount, i);
	//	if(!db[strTemp].Link(bSave, vecElement[i])) vecElement[i] = 0;
	//}

	return TRUE;
}

CArchive &operator<<(CArchive &ar, const SCustomInfoMaster &Src)
{
	ar << long(Src.vecElement.size());

	std::vector<CFigure*>::const_iterator itr = Src.vecElement.begin();

	GUID tempGUID;

	while(itr != Src.vecElement.end())
	{
		tempGUID = (*itr)->GetGUID();

		ar.Write(&tempGUID, sizeof(tempGUID));		

		(*itr)->Store(ar);
		itr++;
	}

	ar << Src.strLayerName;
	ar << Src.fOffsetX;
	ar << Src.fOffsetY;

	return ar;
}

CArchive &operator>>(CArchive &ar, SCustomInfoMaster &Dst)
{
	Dst.Init();

	long nCount = 0;

	ar >> nCount;

	Dst.vecElement.resize(nCount);

	std::vector<CFigure*>::iterator itr = Dst.vecElement.begin();

	GUID tempGUID;

	while(itr != Dst.vecElement.end())
	{
		ar.Read(&tempGUID, sizeof(tempGUID));

		*itr = CFigureFactory::Produce(tempGUID);

		(*itr)->Load(ar);
		itr++;
	}

	ar >> Dst.strLayerName;
	ar >> Dst.fOffsetX;
	ar >> Dst.fOffsetY;

	return ar;
}