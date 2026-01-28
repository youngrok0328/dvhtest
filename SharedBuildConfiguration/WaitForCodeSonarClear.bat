@echo off
setlocal ENABLEDELAYEDEXPANSION

set SCRIPT_DIR=%~dp0
set LOCK_FILE=%SCRIPT_DIR%CodeSonarAnalysisInProgress.lock
set CHECK_COUNT=0

echo [*] Waiting for CodeSonar analysis to complete...
echo [*] Looking for lock file: %LOCK_FILE%

:WAIT_LOOP
if exist "%LOCK_FILE%" (
    set /a CHECK_COUNT+=1
    set /a MOD_RESULT=!CHECK_COUNT! %% 10

    if !MOD_RESULT! EQU 0 (
        echo [!] Lock file still exists. Waiting...
    )

    powershell -command "Start-Sleep -Seconds 30"
    goto WAIT_LOOP
)

echo [+] No active lock found. Proceeding...
exit /b 0