$clangFormatExe = "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Tools\Llvm\x64\bin\clang-format.exe"

if((Test-Path -Path $clangFormatExe) -eq $false)
{
    throw "clang-format.exe 파일을 찾을 수 없습니다."
}

$clangFormatFile = ".\.clang-format"

if((Test-Path -Path $clangFormatFile) -eq $false)
{
    throw ".clang-format 파일을 찾을 수 없습니다."
}

# 변환할 확장자 목록 (필요 시 추가 가능)
$header_extensions = @("*.h", "*.cuh")

foreach ($ext in $header_extensions) {
    Get-ChildItem -Path ".\" -Recurse -File -Filter $ext | ForEach-Object {
        $file = $_
        if (Select-String -Path $file.FullName -Pattern "//HDR_0") {
            & $clangFormatExe -i --verbose --style=file:$clangFormatFile $file.FullName
        }
    }
}

$source_extensions = @("*.cpp", "*.cu")

foreach ($ext in $source_extensions) {
    Get-ChildItem -Path ".\" -Recurse -File -Filter $ext | ForEach-Object {
        $file = $_
        if (Select-String -Path $file.FullName -Pattern "//CPP_0") {
            & $clangFormatExe -i --verbose --style=file:$clangFormatFile $file.FullName
        }
    }
}

