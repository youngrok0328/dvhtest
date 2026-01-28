# 변환할 프로젝트 루트 경로 설정
$projectPath = ".\"

# 변환할 확장자 목록 (필요 시 추가 가능)
$extensions = @("*.h", "*.cpp", "*.cuh", "*.cu")

# 각 확장자에 대해 변환 실행
foreach ($ext in $extensions) {
    Get-ChildItem -Path $projectPath -Recurse -Include $ext | ForEach-Object {
        Write-Host "Converting: $($_.FullName)"
        $content = Get-Content $_.FullName
        Set-Content -Path $_.FullName -Value $content -Encoding UTF8
    }
}

