#pragma once

#ifdef __DPI_DATABASE_EXPORTS__
#define DPI_DATABASE_API __declspec(dllexport)
#else
#define DPI_DATABASE_API __declspec(dllimport)
#endif

class CFigure;

class DPI_DATABASE_API CFigureFactory
{
public:
	static CFigure* Produce(const GUID &guid);
};
