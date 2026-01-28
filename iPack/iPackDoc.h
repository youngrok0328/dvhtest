#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class CiPackDoc : public CDocument
{
protected: // serialization에서만 만들어집니다.
    CiPackDoc();
    DECLARE_DYNCREATE(CiPackDoc)

    // 특성입니다.

public:
    // 작업입니다.

public:
    // 재정의입니다.

public:
    virtual BOOL OnNewDocument();
    virtual void Serialize(CArchive& ar);
#ifdef SHARED_HANDLERS
    virtual void InitializeSearchContent();
    virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

    // 구현입니다.

public:
    virtual ~CiPackDoc();
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    // 생성된 메시지 맵 함수

protected:
    DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
    // 검색 처리기에 대한 검색 콘텐츠를 설정하는 도우미 함수
    void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
