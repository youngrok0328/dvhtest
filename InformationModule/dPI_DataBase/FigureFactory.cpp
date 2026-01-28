// Figure.cpp: implementation of the CFigure class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FigureFactory.h"

#include "Figure_Ellipse.h"
#include "Figure_Rectangle.h"
#include "Figure_Polygon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFigure* CFigureFactory::Produce(const GUID &guid)
{
// 	if(guid == CEllipse::GetGUIDStatic())
// 	{
// 		return new CEllipse;
// 	}
// 	else if(guid == CRectangle::GetGUIDStatic())
// 	{
// 		return new CRectangle;
// 	}
// 	else if(guid == CPolygon::GetGUIDStatic())
// 	{
// 		return new CPolygon;
// 	}
// 	else
// 	{
		return NULL;
//	}
}