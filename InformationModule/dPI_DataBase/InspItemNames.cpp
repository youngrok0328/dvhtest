#include "StdAfx.h"
#include "InspItemNames.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CInspItemNames::CInspItemNames(void)
{
	//static_assert((sizeof(gl_szStrInspectItem) / sizeof(LPCTSTR)) == enumInspectionItemConstants::INSPECT_END_, "Enumerator mismatch");
}

CInspItemNames::~CInspItemNames(void)
{
}
