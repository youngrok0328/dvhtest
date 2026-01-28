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

Get-ChildItem -Path ".\" -Recurse -File -Filter "*.h" | ForEach-Object {
    $file = $_
    if (Select-String -Path $file.FullName -Pattern "//HDR_0") {
        & $clangFormatExe -i --verbose --style=file:$clangFormatFile $file.FullName
    }
}

Get-ChildItem -Path ".\" -Recurse -File -Filter "*.cpp" | ForEach-Object {
    $file = $_
    if (Select-String -Path $file.FullName -Pattern "//CPP_0") {
        & $clangFormatExe -i --verbose --style=file:$clangFormatFile $file.FullName
    }
}

Get-ChildItem -Path ".\" -Recurse -File -Filter "*.cuh" | ForEach-Object {
    $file = $_
    if (Select-String -Path $file.FullName -Pattern "//HDR_0") {
        & $clangFormatExe -i --verbose --style=file:$clangFormatFile $file.FullName
    }
}

Get-ChildItem -Path ".\" -Recurse -File -Filter "*.cu" | ForEach-Object {
    $file = $_
    if (Select-String -Path $file.FullName -Pattern "//CPP_0") {
        & $clangFormatExe -i --verbose --style=file:$clangFormatFile $file.FullName
    }
}

