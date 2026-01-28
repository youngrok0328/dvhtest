#pragma once

//HDR_0_________________________________ Configuration header
//HDR_1_________________________________ This project's headers
//HDR_2_________________________________ Other projects' headers
//HDR_3_________________________________ External library headers
//HDR_4_________________________________ Standard library headers
//HDR_5_________________________________ Forward declarations
//HDR_6_________________________________ Header body
//
class ImageFileParser
{
public:
    static CString GetFileName(LPCTSTR filePath);
    static CString GetFolder(LPCTSTR filePath);
    static long GetTrayID(LPCTSTR filePath);
    static long GetScanID(LPCTSTR filePath);
};
