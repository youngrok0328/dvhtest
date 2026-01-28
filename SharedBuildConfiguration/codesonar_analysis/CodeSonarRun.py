# -*- coding: utf-8 -*-
"""
run_codesonar_from_build_config.py

- SCRIPT_DIR 내 build_configs.json + BuildConfigLoader를 사용
- .sln(.slnx/.slnf) 경로 탐색, VS 버전 감지 → MSBuild 경로 선택
- CodeSonar analyze 실행
- use_parallel_build가 true면 /m 옵션 적용
- language/driver/dotnet 옵션을 통해 cs-dotnet-scan 기반 C# 분석도 지원

전제:
- BuildConfigLoader.load_json()은 컴포넌트들을 평탄화하여 self.components(list[dict])에 채우거나,
  최소한 self.build_data = [{"components":[...]}] 형태를 제공한다고 가정.
"""

import os
import sys
import shutil
import subprocess
import time

from pathlib import Path
from datetime import datetime

from CodeSonarBuildConfigLoader import BuildConfigLoader  # 레포 기준 import

def log(msg: str, lvl: str = "*") -> None:
    print(f"[{datetime.now().strftime('%H:%M:%S')}] [{lvl}] {msg}", flush=True)

def run(cmd: list[str], cwd: Path | None = None) -> int:
    shown = " ".join(f'"{c}"' if (" " in c and not c.startswith('"')) else c for c in cmd)
    log("RUN: " + shown, "+")
    # subprocess.run is safer and handles waiting automatically
    # check=False allows us to handle return code manually
    result = subprocess.run(cmd, cwd=str(cwd) if cwd else None, check=False)
    return result.returncode

def as_bool(v, default: bool = False) -> bool:
    if v is None:
        return default
    if isinstance(v, bool):
        return v
    s = str(v).strip().lower()
    if s in ("1", "true", "t", "yes", "y", "on"):
        return True
    if s in ("0", "false", "f", "no", "n", "off"):
        return False
    return default

def force_delete_forever(path: Path, delay: float = 1.0, timeout: float = 60.0):
    """폴더가 완전히 없어질 때까지 반복 삭제 (최대 timeout 초 대기)"""
    start_time = time.time()
    while path.exists():
        if time.time() - start_time > timeout:
            log(f"삭제 시간 초과({timeout}s): {path}", "!")
            break
        try:
            if path.is_dir():
                shutil.rmtree(path)
            else:
                # 혹시 파일인 경우를 대비
                path.unlink()

            if not path.exists():
                log(f"이전 산출물 삭제 완료: {path}", "+")
                break
        except Exception as e:
            log(f"삭제 실패, 재시도: {e}", "!")
        time.sleep(delay)

def canonical_sln_for_analysis(sln_path: str | Path) -> str:
    """Return the path we should inspect/build when handed a solution-like file."""
    path = Path(sln_path)
    if path.suffix.lower() in {".slnx", ".slnf"}:
        sibling = path.with_suffix(".sln")
        if sibling.exists():
            return str(sibling)
    return str(path)


def guess_dotnet_mode(sln_path: str | Path) -> bool:
    """Heuristically determine whether the solution is managed (.NET/C#)."""

    # Many configs do not label language; fall back to checking the projects inside
    # the solution.  If we see .csproj/.vbproj and no native VC++ projects we can
    # safely assume it needs cs-dotnet-scan.
    try:
        with open(canonical_sln_for_analysis(sln_path), "r", encoding="utf-8", errors="ignore") as f:
            content = f.read()
    except OSError:
        return False

    content_lower = content.lower()
    has_cs = ".csproj" in content_lower or ".vbproj" in content_lower or ".fsproj" in content_lower
    has_cpp = ".vcxproj" in content_lower or ".vcproj" in content_lower

    if has_cs and not has_cpp:
        return True
    if has_cpp and not has_cs:
        return False

    # Mixed or unknown → keep the current decision (default False so caller keeps msbuild)
    return False


def main() -> int:
    # ── 컨텍스트
    script_dir = Path(__file__).resolve().parent
    cwd = Path.cwd().resolve()
    workspace = Path(os.getenv("BUILD_ROOT", cwd)).resolve()

    repo_full = os.getenv("GITHUB_REPOSITORY", "").strip()
    log(f"repo_full={repo_full}")
    if not repo_full:
        # 로컬 실행 시: git 리포 있으면 remote에서 이름 추출
        try:
            import subprocess
            remote_url = subprocess.check_output(
                ["git", "config", "--get", "remote.origin.url"],
                stderr=subprocess.DEVNULL
            ).decode().strip()
            if remote_url:
                repo_full = remote_url.split("/")[-1].replace(".git", "")
        except Exception:
            pass

    # repo_full이 아직도 비어 있으면 현재 폴더명 사용
    repo_name = repo_full.split("/")[-1] if repo_full else workspace.name
    log(f"repo_name={repo_name}")
    # ✅ configs 고정(SCRIPT_DIR)
    build_json = (script_dir / "CodeSonarBuildConfigs.json").resolve()

    # ── CodeSonar/공유 파일 경로
    codesonar_exe = Path(os.getenv(
        "CODESONAR_BIN",
        r"C:\Program Files\CodeSecure\CodeSonar\codesonar\bin\codesonar.exe"))
    conf_src = Path(os.getenv("CODESONAR_CONF_FILE",
                              workspace / "SharedBuildConfiguration/CodeSonarRuleSet.conf")).resolve()
    pw_file = Path(os.getenv(
        "CODESONAR_PW_FILE",
        workspace / "SharedBuildConfiguration/CodeSonarPassword.txt")).resolve()
    lock_file = Path(os.getenv(
        "CODESONAR_LOCK_FILE",
        workspace / "SharedBuildConfiguration" / "CodeSonarAnalysisInProgress.lock"
    )).resolve()

    # ── CodeSonar/실행 플래그 파일 경로
    flag_file = Path(os.getenv(
        "CODESONAR_FLAG_FILE",
        workspace / "push.event.enable"
    )).resolve()

    hub_user = os.getenv("CODESONAR_HUB_USER", "Administrator")
    foreground = as_bool(os.getenv("CODESONAR_FOREGROUND", "1"), True)
    preset_env = os.getenv("CODESONAR_PRESET", "").strip()
    hub_url = os.getenv("CODESONAR_HUB_URL", "").strip()

    dotnet_scan_default = Path(os.getenv(
        "CODESONAR_DOTNET_SCAN",
        codesonar_exe.parent / "cs-dotnet-scan.exe" if codesonar_exe.suffix.lower() == ".exe" else
        codesonar_exe.parent / "cs-dotnet-scan"
    ))
    default_cs_framework = os.getenv("CODESONAR_CSHARP_FRAMEWORK", "net48").strip()
    default_cs_verbosity = os.getenv("CODESONAR_CSHARP_VERBOSITY", "5").strip()
    default_cs_memory = os.getenv("CODESONAR_CSHARP_MEMORY", "16384").strip()

    # ── 로그
    log(f"CWD={cwd}")
    log(f"WORKSPACE={workspace}")
    log(f"SCRIPT_DIR={script_dir}")
    log(f"REPO_NAME={repo_name}")
    log(f"BUILD_JSON={build_json}")

    # ── 필수 파일 체크
    musts = [
        (build_json, "build_configs.json"),
        (codesonar_exe, "CodeSonar EXE"),
        (conf_src, "CodeSonar conf"),
        (pw_file, "Hub PW file"),
    ]
    for p, name in musts:
        if not Path(p).exists():
            log(f"{name} 없음: {p}", "!")
            return 2

    # ── 설정 로드 (현재 repo에 해당하는 항목만 추출)
    try:
        loader = BuildConfigLoader(str(build_json))
        loader.load_json(repo_name)
        loader.build_solution_index(str(workspace))
    except Exception as e:
        log(f"build_configs 로드 실패: {e}", "!")
        return 2

    # ── 컴포넌트 수집 (loader.components 우선, 없으면 build_data에서 추출)
    components = []
    comps_attr = getattr(loader, "components", None)
    if isinstance(comps_attr, list) and comps_attr:
        components = comps_attr
    else:
        bd = getattr(loader, "build_data", []) or []
        for entry in bd:
            comps = entry.get("components", [])
            if isinstance(comps, list):
                components.extend(comps)

    if not components:
        log("configs에서 컴포넌트 목록을 찾지 못함.", "!")
        return 2

    if not repo_name:
        log("repo 식별 불가", "!")
        return 2

    # ── 대상 빌드 목록 생성
    targets = []
    for comp in components:
        name = (comp.get("name") or "component").lower()

        # repo 필터: 있으면 현재 레포와 일치하는 것만, 없으면 포함
        repo_field = (comp.get("repo","") or "").replace(".git","")
        base = repo_field.split("/")[-1] if repo_field else ""
        if base.lower() != repo_name.lower():
            continue

        # sln 값 그대로 사용 (확장자 없으면 .sln → .slnx → .slnf 시도는 BuildConfigLoader.find_sln_path에서 처리)
        sln_name = (comp.get("sln", "") or "").strip()
        if not sln_name:
            log(f"[{comp.get('name','component')}] sln 미기재. 스킵", "-")
            continue

        # 인덱스로 즉시 해소 (못 찾으면 바로 스킵; 풀스캔 호출 안 함)
        sln_path = loader._resolve_sln_with_index(sln_name)
        if not sln_path:
            # 필요시 한번만 경고 찍고 넘어가거나, 아주 조용히 스킵
            log(f"[skip] 워크스페이스에 존재하지 않는 sln: {sln_name}", "-")
            continue

        # VS 버전 감지 → MSBuild 경로
        vs_ver = loader.detect_vs_version(sln_path)
        msbuild = loader.get_msbuild_path(vs_ver)
        if not msbuild:
            log(f"[{name}] VS{vs_ver} MSBuild 경로 매핑 실패", "!")
            continue

        raw_language = str(comp.get("language") or comp.get("analysis_language") or "").strip().lower()
        raw_driver = str(comp.get("driver") or comp.get("build_driver") or "").strip().lower()
        dotnet_opts = comp.get("dotnet") if isinstance(comp.get("dotnet"), dict) else {}
        is_csharp = any([
            raw_language in {"csharp", "cs", "c#", "dotnet", ".net"},
            raw_driver in {"dotnet", "csharp", "cs-dotnet-scan", "c#"},
            bool(dotnet_opts),
        ])
        
        if not is_csharp:
            is_csharp = guess_dotnet_mode(sln_path)

        # build_config: "x64 Debug" 등
        bc = (comp.get("build_config", "") or "").strip()
        platform = ""
        configuration = ""
        if bc:
            parts = bc.split(maxsplit=1)
            platform = parts[0]
            configuration = parts[1] if len(parts) > 1 else ""

        platform_override = comp.get("platform") or comp.get("msbuild_platform")
        configuration_override = comp.get("configuration") or comp.get("msbuild_configuration")
        if platform_override:
            platform = platform_override
        if configuration_override:
            configuration = configuration_override

        if not bc and not (platform and configuration) and not is_csharp:
            log(f"[{name}] build_config 없음. 스킵", "-")
            continue

        use_parallel = as_bool(comp.get("use_parallel_build", True), True)
        preset = (comp.get("preset") or comp.get("codesonar_preset") or preset_env or "").strip()
        dotnet_driver = comp.get("dotnet_driver") or dotnet_opts.get("driver")
        
        # name 필드가 원본 JSON에 명시적으로 있었는지 확인
        # BuildConfigLoader의 로직: "name" not in c일 때만 자동으로 sln stem 추가
        # 따라서 name이 있고 sln stem과 달라야 원본에 있던 것
        # 하지만 같아도 원본에 명시할 수 있으므로, 원본 JSON 직접 확인 필요
        # 임시 해결: name이 있고 multi_component이면 명시적으로 간주
        sln_stem_lower = Path(sln_path).stem.lower()
        
        # BuildConfigLoader 코드 분석: "name" not in c일 때만 추가
        # 즉, comp에 "name"이 있다면 원본 JSON에 있었던 것임
        # 하지만 sln stem과 같으면 자동생성과 구별 불가
        # → 원본 JSON 재확인 필요
        
        # 간단한 해결: comp 원본 dict 체크 (BuildConfigLoader에서 추가 전)
        # 하지만 이미 처리된 상태이므로... 다른 방법 필요
        
        # 최선의 방법: name != sln_stem이면 명시적, 같으면 미명시로 간주
        # 단, 사용자가 명시적으로 sln stem과 같은 이름을 쓸 수도 있음
        # → 이 경우 구별 불가, 원본 JSON 직접 읽기 필요
        
        # 원본 JSON 직접 확인
        import json
        has_explicit_name = False
        try:
            with open(build_json, 'r', encoding='utf-8') as f:
                original_data = json.load(f)
                for repo_entry in original_data:
                    if isinstance(repo_entry, dict) and repo_entry.get("repo") == comp.get("repo"):
                        for orig_comp in repo_entry.get("components", []):
                            if orig_comp.get("sln") == comp.get("sln"):
                                has_explicit_name = "name" in orig_comp
                                break
        except Exception:
            # 실패시 기존 로직 사용
            has_explicit_name = name != sln_stem_lower

        targets.append({
            "name": name,
            "has_explicit_name": has_explicit_name,
            "sln_path": sln_path,
            "msbuild": msbuild,
            "platform": platform,
            "configuration": configuration,
            "use_parallel": use_parallel,
            "mode": "dotnet" if is_csharp else "msbuild",
            "preset": preset,
            "dotnet_opts": dotnet_opts,
            "dotnet_driver": dotnet_driver,
        })

    if not targets:
        log(f"현재 repo({repo_name})에 매칭되는 컴포넌트가 없음.", "!")
        return 3

    # ✅ 다중 component 여부 확인
    multi_component = len(targets) > 1
    log(f"분석 대상: {len(targets)}개 component")
    if multi_component:
        log(f"다중 component 모드: 각 빌드 전 Clean 자동 수행", "*")
    else:
        log(f"단일 component 모드: 빌드 타겟에 따라 Clean 결정")

    # ── 락
    log("Lock 검사...", "*")
    if lock_file.exists():
        log(f"Lock 존재: {lock_file}", "!")
        return 1
    lock_file.parent.mkdir(parents=True, exist_ok=True)
    lock_file.write_text("LOCKED", encoding="utf-8")

    try:
        # ── 대상별 CodeSonar analyze
        for idx, t in enumerate(targets):
            name = t["name"]
            sln_path = t["sln_path"]
            msbuild = t["msbuild"]
            configuration = t["configuration"]
            platform = t["platform"]
            use_parallel = t["use_parallel"]
            has_explicit_name = t.get("has_explicit_name", False)

            # ✅ 프로젝트명 생성 로직 (이전 버전 호환)
            # - name 필드 없음: repo 이름 그대로 사용 (이전 방식)
            # - name 필드 있음: name 값을 그대로 프로젝트명으로 사용
            if has_explicit_name:
                # name 필드가 명시되어 있으면 name 값을 그대로 사용
                project_name = name
                log(f"CodeSonar Project Name: {project_name} (name 필드 사용)")
            else:
                # name 필드 없으면 repo 이름 그대로 (이전 호환)
                project_name = repo_name
                log(f"CodeSonar Project Name: {project_name} (repo 이름 사용)")
            prj_files_dir = workspace / f"{project_name}.prj_files"
            prj_conf_file = workspace / f"{project_name}.conf"

            # conf 복사
            shutil.copy2(conf_src, prj_conf_file)
            if not prj_conf_file.exists():
                log(f"conf 복사 실패: {prj_conf_file}", "!")
                return 1

            # 이전 산출물 삭제
            if prj_files_dir.exists():
                force_delete_forever(prj_files_dir)

            # .slnx/.slnf → 같은 base의 .sln이 있으면 그걸로 빌드(환경 안전)
            use_path = canonical_sln_for_analysis(sln_path)
            if use_path != sln_path:
                log(f"Using .sln instead of {Path(sln_path).suffix.lower()} for MSBuild: {use_path}", "-")

            mode = t.get("mode", "msbuild")
            preset = t.get("preset") or preset_env

            log_suffix = f"{Path(use_path).name}"
            if platform or configuration:
                log_suffix += f", {platform or '?'} / {configuration or '?'}"
            if mode == "msbuild":
                log_suffix += f", parallel={use_parallel}"
            log(f"=" * 80)
            log(f"Component {idx+1}/{len(targets)}: {name}")
            log(f"  - Project: {project_name}")
            log(f"  - SLN: {Path(use_path).name}")
            log(f"  - Mode: {mode}")
            log(f"  - Config: {platform} / {configuration}")
            log(f"=" * 80)

            # 🔹 빌드 타겟 결정
            is_scheduled = False
            if len(sys.argv) > 1:
                #첫번째 인자를 빌드 타겟으로 사용
                build_target = sys.argv[1].strip()
                #두번째 인자를 is schedule 플래그로 사용 "schedule"일 때만 True
                is_scheduled = len(sys.argv) > 2 and sys.argv[2].strip().lower() == "schedule"
            else:
                build_target = "build"

            # 🔹 Hub 업로드용 프로젝트 이름 생성 (Prefix 추가)
            # build_target이 'rebuild'일 때만 prefix를 붙여서 Hub에 업로드
            hub_project_name = ""
            if is_scheduled == True:
                project_tree_prefix = "SCHEDULED-ANALYSIS"
                prefix_clean = project_tree_prefix.replace("\\", "/")
                hub_project_name = f"{prefix_clean}/{project_name}"
                log(f"CodeSonar Project Name (Hub): {hub_project_name}")
            
            # ✅ MSBuild 모드에서는 무조건 rebuild 강제 (증분 빌드 방지)
            if mode == "msbuild":
                if build_target.lower() != "rebuild":
                    reason = "다중 component" if multi_component else "증분 빌드 방지"
                    log(f"[빌드 타겟] {reason} → 'rebuild' 강제 적용", "*")
                    build_target = "rebuild"
            
            log(f"MSBuild Target: {build_target}")

            cmd = [str(codesonar_exe), "analyze", project_name]
            if hub_project_name:
                cmd.extend(["-project", hub_project_name])
            if foreground:
                cmd.append("-foreground")
            if preset:
                cmd.extend(["-preset", preset])
            cmd.extend(["-conf-file", str(prj_conf_file), "-hubuser", hub_user, "-hubpwfile", str(pw_file)])
            if hub_url:
                cmd.append(hub_url)

            if mode == "dotnet":
                dotnet_opts = t.get("dotnet_opts") or {}
                dotnet_driver = t.get("dotnet_driver") or dotnet_opts.get("driver")
                driver_path = Path(dotnet_driver) if dotnet_driver else dotnet_scan_default
                if not driver_path.exists():
                    log(f"[{name}] cs-dotnet-scan 실행 파일을 찾을 수 없음: {driver_path}", "!")
                    return 2

                cmd.append(str(driver_path))
                cmd.extend(["-msbuild-solution", use_path])
                cmd.extend(["-msbuild-location", str(Path(msbuild).parent)])

                # C# 분석 시에는 구성/플랫폼 옵션을 전달하지 않는다.

                framework = (dotnet_opts.get("framework")
                             or dotnet_opts.get("target_framework")
                             or default_cs_framework)
                if framework:
                    cmd.extend(["-framework", str(framework)])

                verbosity = (dotnet_opts.get("verbosity")
                             or dotnet_opts.get("cs_verbosity")
                             or default_cs_verbosity)
                if verbosity:
                    cmd.extend(["-cs-verbosity", str(verbosity)])

                cs_mem = (dotnet_opts.get("analysis_memory")
                          or dotnet_opts.get("csharp_analysis_memory")
                          or default_cs_memory)
                if cs_mem:
                    cmd.extend(["-csharp-analysis-memory", str(cs_mem)])

                extra_args = dotnet_opts.get("extra_args") or []
                if isinstance(extra_args, str):
                    extra_args = extra_args.split()
                if extra_args:
                    cmd.extend(map(str, extra_args))
            else:
                cmd.extend([
                    str(msbuild), use_path,
                    f"/t:{build_target}",
                    f"/p:Configuration={configuration}",
                    f"/p:Platform={platform}",
                ])
                if use_parallel:
                    cmd.append("/m")

            rc = run(cmd, cwd=workspace)
            if rc != 0:
                log(f"CodeSonar 실패({name}): exit {rc}", "!")
                return rc

        log("모든 컴포넌트 분석 완료.", "+")
        return 0

    finally:
        # Lock 파일은 분석이 완료되면 항상 삭제한다
        if lock_file.exists():
            try:
                lock_file.unlink()
                log("Lock 제거 완료.", "+")
            except Exception as e:
                log(f"Lock 제거 실패: {e}", "!")

        # 스케쥴 분석을 판별하는 플래그는 수동 분석에선 삭제하지 않는다.
        event_name = os.getenv("GITHUB_EVENT_NAME", "").strip().lower()
        if event_name == "schedule" and flag_file.exists():
            try:
                flag_file.unlink()
                log("Flag 제거 완료(스케줄).", "+")
            except Exception as e:
                log(f"Flag 제거 실패(스케줄): {e}", "!")
        else:
            log(f"Flag 유지 (event={event_name or 'unknown'})", "~")
          
if __name__ == "__main__":
    sys.exit(main())
