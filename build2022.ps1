# 빌드 경로 설정
$ScriptRoot = Split-Path -Parent $MyInvocation.MyCommand.Path
$BUILD_PATH = Join-Path $ScriptRoot "x64\Release"

Write-Host "빌드 시작..."

# MSBuild 실행
& "C:\Program Files\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\msbuild.exe" `
    "$ScriptRoot\iPack.sln" `
    /t:rebuild `
    /p:Configuration="release" `
    /p:Platform=x64 `
    /m:48

# GitHub Actions 환경 변수로 설정
"OUTPUT_PATH=$BUILD_PATH" | Out-File -FilePath $env:GITHUB_ENV -Encoding utf8 -Append
