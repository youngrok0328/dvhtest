param(
    [string]$PackageName,
    [string]$SolutionFileName,
    [string]$InstallDir,
    [string]$BuildRoot
)

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.IO.Compression.FileSystem

function Log {
    param ([string]$message)
    Write-Output $message
    Add-Content -Path $logFile -Value $message
}

function Get-VersionInfoFromHeader($headerPath) {
    if (!(Test-Path $headerPath)) {
        throw "VersionInfo.h 파일이 존재하지 않습니다: $headerPath"
    }

    $lines = Get-Content $headerPath

    # 숫자 매크로 값 읽기 함수
    function GetValue($macro) {
        $line = $lines | Where-Object { $_ -match "^\s*#define\s+$macro" }
        if (-not $line) { return "" }
        return ($line -replace '[^\d]', '').Trim() -replace '^\s+|\s+$', ''
    }

    # 문자열 매크로 값 읽기 함수 (따옴표 안 내용 추출, \0 제거)
    function GetStringValue($macro) {
        $line = $lines | Where-Object { $_ -match "^\s*#define\s+$macro" }
        if (-not $line) { return "" }
        $match = ($line -match '"(.*?)"') | Out-Null
        $value = $matches[1].Trim()
        return ($value -replace '\\0$', '')   # \0 제거
    }

    # 숫자 버전 읽기
    $model  = GetValue 'MODEL_MAJOR_VERSION'
    $socket = GetValue 'MODEL_MINOR_VERSION'
    $major  = GetValue 'MODEL_REVISION'
    $minor  = GetValue 'MODEL_BUILD_NUMBER'

    if (-not ($model -and $socket -and $major -and $minor)) {
        throw "VersionInfo.h에서 모든 숫자 버전 값을 추출하지 못했습니다."
    }

    $versionString = "$model.$socket.$major.$minor"

    # 문자열 버전 읽기
    $versionStr = GetStringValue 'VER_FILE_VERSION_STR'
    if (-not $versionStr) {
        $versionStr = $versionString  # 문자열 버전이 없으면 숫자 버전 사용
    }

    return @{
        VersionShort = $versionString
        VersionString = $versionStr
    }
}

# --- 초기 변수 설정 ---
#$configPath = Join-Path $PSScriptRoot "BuildScript_Config.psd1"
#$configPath = Join-Path (Split-Path $PSScriptRoot -Parent) "BuildScript_Config.psd1"
#$config = Import-PowerShellDataFile -Path $configPath

$PackageName      = $config.PackageName
$SolutionFileName = $config.SolutionFileName
$InstallDir       = $config.InstallDir
$buildRoot        = $config.BuildRoot
$IsCI             = $config.IsCI

# buildRoot 폴더가 없으면 생성
if (!(Test-Path $buildRoot)) {
    New-Item -Path $buildRoot -ItemType Directory | Out-Null
}

$outputRoot = $buildRoot

$timestamp = Get-Date -Format "yyyyMMdd_HHmmss"
$logFile = Join-Path $outputRoot ("${PackageName}_${timestamp}_BuildLog.txt")
Log "=== 빌드 자동화 시작: $timestamp ==="

# --- 1. 폴더 선택 ---
#$folderBrowser = New-Object System.Windows.Forms.FolderBrowserDialog
#$folderBrowser.Description = "빌드할 원본 소스 폴더를 선택하세요"
#$folderBrowser.ShowNewFolderButton = $false
#
#if ($folderBrowser.ShowDialog() -eq [System.Windows.Forms.DialogResult]::OK) {
#    $sourcePath = $folderBrowser.SelectedPath
#    Log "선택한 경로: $sourcePath"
#} else {
#    Log "폴더 선택이 취소됨. 종료."
#    exit 1
#}

# 폴더 선택을 생략하고 현재 스크립트 실행 위치를 sourcePath로 지정
#$sourcePath = $PSScriptRoot
$sourcePath = Split-Path $PSScriptRoot -Parent
Log "선택한 경로(현재 폴더): $sourcePath"

# --- 2. 빌드 타입 ---
$buildTypes = @("Release", "Debug")

# --- 3. 버전 정보 (VersionInfo,h) ---
$versionHeaderPath = Join-Path $sourcePath "VersionInfo.h"

try {
    $versionInfo = Get-VersionInfoFromHeader $versionHeaderPath
    $fileVersionShort = $versionInfo.VersionShort
    $fileVersionString = $versionInfo.VersionString

    Log "숫자 기반 버전: $fileVersionShort"
    Log "문자열 버전: $fileVersionString"
} catch {
    Log "버전 정보 추출 실패: $_"
    exit 1
}

# --- 3-1. 빌드 전 실행 중인 프로세스 확인 ---
$targetProcessName = "iPack_Launcher"

try {
    $runningProc = Get-Process -Name $targetProcessName -ErrorAction SilentlyContinue
    if ($runningProc) {
        Log "프로세스 '$targetProcessName.exe'가 실행 중입니다. 빌드를 중단합니다."
        exit 1
    }
    Log "프로세스 '$targetProcessName.exe'가 실행 중이지 않음. 빌드 계속 진행."
} catch {
    Log "프로세스 확인 중 오류 발생: $_"
    exit 1
}

# --- 4. 빌드 디렉토리 설정 및 복사 ---
$buildDir = Join-Path $outputRoot ("${PackageName}_${timestamp}_${fileVersionString}")
Log "빌드 디렉토리: $buildDir"
try {
    Copy-Item $sourcePath -Destination $buildDir -Recurse -Force -ErrorAction Stop
    Log "소스 복사 완료"
} catch {
    Log "소스 복사 중 오류 발생: $_"
    exit 1
}

# --- 5. 불필요한 파일 제거 ---
$excludes = @(".git", ".vs", "Debug", "Release", "*.user", "*.suo", "*.log")
foreach ($exclude in $excludes) {
    Get-ChildItem -Path $buildDir -Recurse -Force -Include $exclude | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
}
Log "불필요한 파일 제거 완료"

# --- 6. Visual Studio 빌드 ---
$slnFilePath = Join-Path $buildDir $SolutionFileName
$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Professional\Common7\IDE\devenv.exe"
if (!(Test-Path $vsPath)) {
    Log "Visual Studio 경로 오류"
    exit 1
}

foreach ($buildType in $buildTypes) {
    Log "빌드 타입: $buildType"

    Log "빌드 시작: $slnFilePath [$buildType]"
    $buildResult = & $vsPath $slnFilePath /Build $buildType 2>&1 | Tee-Object -FilePath $logFile -Append

    if ($LASTEXITCODE -ne 0) {
        Log "$buildType 빌드 실패"
        exit 1
    }
    Log "$buildType 빌드 완료"
}

# --- 7. 압축 작업 ---
$buildConfigs = @("Release", "Debug")

foreach ($buildType in $buildConfigs) {

    $releasePath = Join-Path $buildDir "x64\$buildType"
    if (!(Test-Path $releasePath)) {
        Log "$buildType 빌드 결과 경로가 존재하지 않음."
        continue
    }

    # Binary.zip
    $binaryFiles = Get-ChildItem $releasePath -Include *.exe, *.dll, ReleaseNotes.txt -Recurse
    $tempBinaryFolder = Join-Path $buildDir "TempBinary_$buildType"
    if ($binaryFiles) {
        New-Item -Path $tempBinaryFolder -ItemType Directory | Out-Null
        $binaryFiles | ForEach-Object { Copy-Item $_.FullName -Destination $tempBinaryFolder }
        $binaryZip = Join-Path $outputRoot ("${PackageName}_${timestamp}_${fileVersionString}_${buildType}_Binary.zip")
        [System.IO.Compression.ZipFile]::CreateFromDirectory($tempBinaryFolder, $binaryZip)
        Remove-Item $tempBinaryFolder -Recurse -Force
        Log "$buildType Binary.zip 생성 완료"
    }

    # Pdb.zip
    $pdbFiles = Get-ChildItem $releasePath -Include *.pdb -Recurse
    $tempPdbFolder = Join-Path $buildDir "TempPdb_$buildType"
    if ($pdbFiles) {
        New-Item -Path $tempPdbFolder -ItemType Directory | Out-Null
        $pdbFiles | ForEach-Object { Copy-Item $_.FullName -Destination $tempPdbFolder }
        $pdbZip = Join-Path $outputRoot ("${PackageName}_${timestamp}_${fileVersionString}_${buildType}_Pdb.zip")
        [System.IO.Compression.ZipFile]::CreateFromDirectory($tempPdbFolder, $pdbZip)
        Remove-Item $tempPdbFolder -Recurse -Force
        Log "$buildType Pdb.zip 생성 완료"
    }
}

# 불필요한 파일 제거
$excludes = @(".git", ".vs", "Debug", "Release", "*.user", "*.suo", "*.log")
foreach ($exclude in $excludes) {
    Get-ChildItem -Path $buildDir -Recurse -Force -Include $exclude | Remove-Item -Recurse -Force -ErrorAction SilentlyContinue
}
Log "불필요한 파일 제거 완료"

# SourceCode.zip
Get-ChildItem -Path $buildDir -Directory -Recurse -Force | Where-Object { $_.Name -ieq "x64" } | ForEach-Object { Remove-Item $_.FullName -Recurse -Force }
#$sourceCodeZip = Join-Path $outputRoot ("${fileVersionShort}_${timestamp}_SourceCode.zip")
$sourceCodeZip = Join-Path $outputRoot ("${PackageName}_${timestamp}_${fileVersionString}_SourceCode.zip")
[System.IO.Compression.ZipFile]::CreateFromDirectory($buildDir, $sourceCodeZip)
Log "SourceCode.zip 생성 완료"

# 빌드 디렉토리 삭제
if (Test-Path $buildDir) {
    Remove-Item $buildDir -Recurse -Force
    Log "빌드 디렉토리 삭제 완료: $buildDir"
} else {
    Log "삭제할 빌드 디렉토리가 존재하지 않음: $buildDir"
}

# --- 8. NSIS 스크립트 생성 ---
$nsisPath = "C:\Program Files (x86)\NSIS\makensis.exe"

$buildConfigs = @("Release", "Debug")

foreach ($buildType in $buildConfigs) {

    $nsiFile = Join-Path $outputRoot "BuildInstaller_${buildType}.nsi"
    $setupExeName = "${PackageName}_${timestamp}_${fileVersionString}_${buildType}.exe"
    $setupExe = Join-Path $outputRoot $setupExeName

    $binaryZipName = "${PackageName}_${timestamp}_${fileVersionString}_${buildType}_Binary.zip"

    $absoluteInstallDir = $InstallDir
    $expandCommand = 'powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "Expand-Archive -Path \"' + "$absoluteInstallDir\\$binaryZipName" + '\" -DestinationPath \"' + "$absoluteInstallDir" + '\" -Force"'

    $nsiContent = @"
Outfile "$setupExe"
InstallDir "$InstallDir"
RequestExecutionLevel admin
ShowInstDetails show

Page Directory
Page InstFiles

!include "WinVer.nsh"

Section "Install"
    ; PowerShell 명령을 명시적으로 cmd.exe 경유 실행
    nsExec::ExecToStack 'cmd /C powershell.exe -Command "[System.DateTime]::Now.ToString(\"yyyyMMdd_HHmmss\")"'
    Pop `$0        ; return code
    Pop `$1        ; 문자열

    StrCpy `$R4 "`$INSTDIR`_backup_`$1"

    ; 확인용 출력
    DetailPrint "백업 이름: `$1"
    DetailPrint "백업 경로: `$R4"

    ; 메모리 해제 (더 이상 $0을 참조하지 않으므로)
    System::Free `$0

    ; 기존 백업 폴더가 있을 경우 삭제
    IfFileExists "`$R4\*.*" 0 +2
        RMDir /r "`$R4"

    ; 기존 설치 폴더 존재 시 -> 리네임
    IfFileExists "`$INSTDIR\*.*" 0 SkipRename
        Rename "`$INSTDIR" "`$R4"
        IfErrors 0 ContinueInstall
        MessageBox MB_ICONSTOP "기존 폴더 이름 변경 실패. 설치를 중단합니다."
        Abort

SkipRename:
    CreateDirectory "`$INSTDIR"

ContinueInstall:
    SetOutPath "`$INSTDIR"
    File "$binaryZipName"

    nsExec::Exec '$expandCommand'

    Delete "`$INSTDIR\$binaryZipName"

SectionEnd
"@

    # UTF-8 BOM 포함 저장
    [System.IO.File]::WriteAllText($nsiFile, $nsiContent, [System.Text.UTF8Encoding]::new($true))
    Log "NSIS 스크립트 작성 완료: $nsiFile"

    if (!(Test-Path $nsiFile)) {
        Log "$buildType NSIS 스크립트 파일이 존재하지 않습니다."
        continue
    }

    if (!(Test-Path $nsisPath)) {
        Log "NSIS 경로 오류"
        exit 1
    }

    Log "$buildType NSIS 컴파일 시작"
    $output = & $nsisPath $nsiFile 2>&1
    $outputText = $output -join "`r`n"   # 줄바꿈 포함 문자열로 변환
    Log "NSIS 출력:`r`n$outputText"

    if (Test-Path $setupExe) {
        Log "$buildType 설치 파일 생성 완료: $setupExe"
    } else {
        Log "$buildType 설치 파일 생성 실패"
        Log "NSIS 에러 로그:`r`n$outputText"
        exit 1
    }

    Start-Sleep -Seconds 1
}

Log "=== 빌드 자동화 완료 ==="

# Bitbucket 환경에서는 메시지 박스 띄우지 않음
if (-not $IsCI) {
    [System.Windows.Forms.MessageBox]::Show("Setup.exe 파일 생성 완료 (Release/Debug)", "완료", "OK", "Information")
    $result = [System.Windows.Forms.MessageBox]::Show("빌드 자동화가 완료되었습니다. PowerShell을 종료하시겠습니까?", "종료 확인", "YesNo", "Question")
    if ($result -eq [System.Windows.Forms.DialogResult]::Yes) {
        Stop-Process -Id $PID
    } else {
        Log "사용자가 PowerShell을 계속 열기로 선택함."
    }
}
