//CPP_0_________________________________ Precompiled header
#include "stdafx.h"

//CPP_1_________________________________ Main header
#include "iPackDoc.h"

//CPP_2_________________________________ This project's headers
//CPP_3_________________________________ Other projects' headers
//CPP_4_________________________________ External library headers
//CPP_5_________________________________ Standard library headers
//CPP_6_________________________________ Preprocessor macros
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//CPP_7_________________________________ Implementation body
//
IMPLEMENT_DYNCREATE(CiPackDoc, CDocument)

BEGIN_MESSAGE_MAP(CiPackDoc, CDocument)
END_MESSAGE_MAP()

// CiPackDoc 생성/소멸

CiPackDoc::CiPackDoc()
{
    // TODO: 여기에 일회성 생성 코드를 추가합니다.
}

CiPackDoc::~CiPackDoc()
{
}

BOOL CiPackDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
        return FALSE;

    // TODO: 여기에 재초기화 코드를 추가합니다.
    // SDI 문서는 이 문서를 다시 사용합니다.

    return TRUE;
}

// CiPackDoc serialization

void CiPackDoc::Serialize(CArchive& ar)
{
    if (ar.IsStoring())
    {
        // TODO: 여기에 저장 코드를 추가합니다.
    }
    else
    {
        // TODO: 여기에 로딩 코드를 추가합니다.
    }
}

#ifdef SHARED_HANDLERS

// 축소판 그림을 지원합니다.
void CiPackDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
    // 문서의 데이터를 그리려면 이 코드를 수정하십시오.
    dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

    CString strText = _T("TODO: implement thumbnail drawing here");
    LOGFONT lf;

    CFont* pDefaultGUIFont = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
    pDefaultGUIFont->GetLogFont(&lf);
    lf.lfHeight = 36;

    CFont fontDraw;
    fontDraw.CreateFontIndirect(&lf);

    CFont* pOldFont = dc.SelectObject(&fontDraw);
    dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
    dc.SelectObject(pOldFont);
}

// 검색 처리기를 지원합니다.
void CiPackDoc::InitializeSearchContent()
{
    CString strSearchContent;
    // 문서의 데이터에서 검색 콘텐츠를 설정합니다.
    // 콘텐츠 부분은 ";"로 구분되어야 합니다.

    // 예: strSearchContent = _T("point;rectangle;circle;ole object;");
    SetSearchContent(strSearchContent);
}

void CiPackDoc::SetSearchContent(const CString& value)
{
    if (value.IsEmpty())
    {
        RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
    }
    else
    {
        CMFCFilterChunkValueImpl* pChunk = NULL;
        ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
        if (pChunk != NULL)
        {
            pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
            SetChunkValue(pChunk);
        }
    }
}

#endif // SHARED_HANDLERS

// CiPackDoc 진단

#ifdef _DEBUG
void CiPackDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void CiPackDoc::Dump(CDumpContext& dc) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

// CiPackDoc 명령
