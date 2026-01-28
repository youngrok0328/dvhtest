import os, json, re, chardet, subprocess

from collections import defaultdict
from pathlib import Path

class BuildConfigLoader:
    def __init__(self, json_path):
        self.json_path = json_path
        self.build_data = []      # ← 원문 유지(호환용)
        self.components = []      # ← 우리가 쓸 평탄화된 컴포넌트 리스트
        self._sln_index = {}

    def load_json(self, target_repo: str | None = None):
        """Load and flatten build configuration.

        Parameters
        ----------
        target_repo: str | None
            If provided, only components whose ``repo`` matches (case-insensitive)
            will be loaded. This prevents scanning unrelated entries when the
            JSON contains configurations for multiple repositories.
        """

        print(f"[DEBUG] Entering load_json for path: {self.json_path}")
        self.build_data = []
        self.components = []

        if not os.path.exists(self.json_path):
            print(f"JSON file not found: {self.json_path}")
            print(f"[DEBUG] JSON file not found: {self.json_path}. Exiting.")
            return

        try:
            with open(self.json_path, "rb") as rb:
                raw = rb.read()
            enc = chardet.detect(raw).get("encoding") or "utf-8"
            data = json.loads(raw.decode(enc, errors="replace"))
        except json.JSONDecodeError as e:
            print(f"An error occurred while parsing the JSON file: {e}")
            print(f"[DEBUG] JSON parsing error: {e}. Exiting.")
            return
        except Exception as e:
            print(f"An unexpected error occurred while reading the JSON file: {e}")
            print(f"[DEBUG] Unexpected error reading JSON: {e}. Exiting.")
            return

        # ── 평탄화하면서 부모 repo를 각 컴포넌트에 주입 ──
        comps = []

        def repo_matches(repo_field: str | None) -> bool:
            if not target_repo:
                return True
            base = (repo_field or "").replace(".git", "").split("/")[-1].lower()
            return base == target_repo.lower()

        if isinstance(data, list):
            for entry in data:
                if isinstance(entry, dict) and isinstance(entry.get("components"), list):
                    parent_repo = entry.get("repo")
                    if not repo_matches(parent_repo):
                        continue
                    for comp in entry["components"]:
                        if not isinstance(comp, dict):
                            raise ValueError("Component in 'components' must be an object")
                        c = dict(comp)  # 복사
                        if parent_repo and "repo" not in c:
                            c["repo"] = parent_repo
                        if "name" not in c and "sln" in c:
                            c["name"] = Path(c["sln"]).stem
                        if repo_matches(c.get("repo")):
                            comps.append(c)
                elif isinstance(entry, dict):
                    if not repo_matches(entry.get("repo")):
                        continue
                    c = dict(entry)
                    if "name" not in c and "sln" in c:
                        c["name"] = Path(c["sln"]).stem
                    comps.append(c)
                else:
                    raise ValueError("Invalid item in top-level list; expected object.")
        elif isinstance(data, dict):
            if isinstance(data.get("components"), list):
                parent_repo = data.get("repo")
                if repo_matches(parent_repo):
                    for comp in data["components"]:
                        if not isinstance(comp, dict):
                            raise ValueError("Component in 'components' must be an object")
                        c = dict(comp)
                        if parent_repo and "repo" not in c:
                            c["repo"] = parent_repo
                        if "name" not in c and "sln" in c:
                            c["name"] = Path(c["sln"]).stem
                        if repo_matches(c.get("repo")):
                            comps.append(c)
            elif isinstance(data.get("models"), list):
                for m in data["models"]:
                    if isinstance(m, dict) and isinstance(m.get("components"), list):
                        parent_repo = m.get("repo")
                        if not repo_matches(parent_repo):
                            continue
                        for comp in m["components"]:
                            if not isinstance(comp, dict):
                                raise ValueError("Component in 'components' must be an object")
                            c = dict(comp)
                            if parent_repo and "repo" not in c:
                                c["repo"] = parent_repo
                            if "name" not in c and "sln" in c:
                                c["name"] = Path(c["sln"]).stem
                            if repo_matches(c.get("repo")):
                                comps.append(c)
            else:
                # dict가 바로 component인 경우
                if "sln" in data or "build_config" in data or "repo" in data:
                    if repo_matches(data.get("repo")):
                        c = dict(data)
                        if "name" not in c and "sln" in c:
                            c["name"] = Path(c["sln"]).stem
                        comps = [c]
                else:
                    raise ValueError("Unrecognized schema: no components/models and not a component.")
        else:
            raise ValueError("Top-level JSON must be array or object.")

        # 최소 필드 체크
        for i, c in enumerate(comps):
            if "sln" not in c:
                raise ValueError(f"Component #{i} missing 'sln'")

        self.components = comps
        self.build_data = [{"components": comps}]  # 호환용
        print(f"Loaded {len(self.components)} components (flattened).")
        print(f"[DEBUG] components: {self.components}")

    def build_solution_index(self, search_root: str):
        print(f"[DEBUG] Building solution index under: {search_root}")
        idx = defaultdict(list)
        exts = {".sln", ".slnx", ".slnf"}
        for root, _, files in os.walk(search_root):
            for f in files:
                if os.path.splitext(f)[1].lower() in exts:
                    idx[f.lower()].append(os.path.join(root, f))
        self._sln_index = dict(idx)
        print(f"[DEBUG] Indexed {sum(len(v) for v in self._sln_index.values())} solution files.")

    def _resolve_sln_with_index(self, sln_value: str):
        if not self._sln_index:
            return None
        name = (sln_value or "").strip()
        if not name:
            return None
        base, ext = os.path.splitext(name)
        candidates = [name] if ext else [f"{name}.sln", f"{name}.slnx", f"{name}.slnf"]
        for c in candidates:
            hits = self._sln_index.get(c.lower())
            if hits:
                return hits[0]  # 동일 파일명 여러 개면 첫 번째(필요시 규칙 추가)
        return None

    def find_sln_path(self, sln_value, search_root):
        # 인덱스가 있으면 먼저 인덱스로 즉시 해결 (풀스캔 방지)
        hit = self._resolve_sln_with_index(sln_value)
        if hit:
            print(f"[DEBUG] Found SLN via index: {hit}")
            return hit

        # (인덱스가 없거나 초기화 전이면만) 기존 느린 경로
        print(f"[DEBUG] Entering find_sln_path for sln: {sln_value}, root: {search_root}")
        name = (sln_value or "").strip()
        if not name:
            print(f"[DEBUG] Empty sln_value")
            return None
        base, ext = os.path.splitext(name)
        candidates = [name] if ext else [f"{name}.sln", f"{name}.slnx", f"{name}.slnf"]
        cand_lower = {c.lower(): c for c in candidates}
        for root, _, files in os.walk(search_root):
            lower_map = {f.lower(): f for f in files}
            for want in cand_lower:
                if want in lower_map:
                    found = os.path.join(root, lower_map[want])
                    print(f"[DEBUG] Found SLN path: {found}")
                    return found
        print(f"[DEBUG] SLN file not found. Tried: {candidates} under {search_root}.")
        return None

    def detect_vs_version(self, sln_path):
        print(f"[DEBUG] Entering detect_vs_version for sln: {sln_path}")
        _, ext = os.path.splitext(sln_path)
        ext = ext.lower()
        target = sln_path
        if ext in (".slnx", ".slnf"):
            guess = os.path.splitext(sln_path)[0] + ".sln"
            if os.path.exists(guess):
                target = guess
                print(f"[DEBUG] Using adjacent .sln for version detect: {target}")
            else:
                print(f"[DEBUG] No adjacent .sln. Fallback to VS2022 (17).")
                return 17
        with open(target, 'r', encoding='utf-8') as f:
            content = f.read()
        m = re.search(r"VisualStudioVersion\s*=\s*(\d+)\.", content)
        if m:
            v = int(m.group(1)); print(f"[DEBUG] Detected VS version: {v}"); return v
        print(f"[DEBUG] Failed to detect VS version from {target}. Fallback to 17.")
        return 17

    
    @staticmethod
    def _vswhere_path_candidates():
        cands = [
            Path(r"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"),
            Path(r"C:\Program Files\Microsoft Visual Studio\Installer\vswhere.exe"),
        ]
        for p in os.environ.get("PATH", "").split(os.pathsep):
            exe = Path(p) / "vswhere.exe"
            if exe.exists():
                cands.append(exe)
        seen, out = set(), []
        for p in cands:
            if p and p.exists():
                sp = str(p.resolve())
                if sp not in seen:
                    seen.add(sp); out.append(Path(sp))
        return out

    @staticmethod
    def _query_vs_instances():
        # vswhere가 있으면 설치된 모든 인스턴스 JSON 반환
        for vsw in BuildConfigLoader._vswhere_path_candidates():
            try:
                cmd = [
                    str(vsw), "-all",
                    "-products", "*",
                    "-requires", "Microsoft.Component.MSBuild",
                    "-format", "json"
                ]
                cp = subprocess.run(cmd, check=True, capture_output=True, text=True, encoding="utf-8")
                data = json.loads(cp.stdout or "[]")
                if isinstance(data, list):
                    return data
            except Exception:
                continue
        return []

    @staticmethod
    def _pick_msbuild_from_install_path(install_path: str):
        ip = Path(install_path)
        cands = [
            ip / "MSBuild" / "Current" / "Bin" / "MSBuild.exe",
            ip / "MSBuild" / "17.0" / "Bin" / "MSBuild.exe",
            ip / "MSBuild" / "16.0" / "Bin" / "MSBuild.exe",
            ip / "MSBuild" / "15.0" / "Bin" / "MSBuild.exe",
        ]
        for c in cands:
            if c.exists():
                return str(c)
        msbuild_root = ip / "MSBuild"
        if msbuild_root.exists():
            for exe in msbuild_root.rglob("MSBuild.exe"):
                return str(exe)
        return None

    def get_msbuild_path(self, version_major):
        print(f"[DEBUG] Entering get_msbuild_path for VS major: {version_major}")

        # 0) 사용자가 명시한 경로가 있으면 그걸 최우선
        env_bin = os.getenv("MSBUILD_BIN")
        if env_bin and Path(env_bin).exists():
            print(f"[DEBUG] MSBuild via env MSBUILD_BIN: {env_bin}")
            return env_bin

        # 1) vswhere로 설치 인스턴스 조회
        def parse_ver(s: str):
            try:
                return tuple(int(x) for x in s.split("."))
            except Exception:
                return (0,)

        instances = self._query_vs_instances()
        maj = None
        try:
            maj = int(version_major)
        except Exception:
            maj = None

        if instances:
            # 요청 메이저에 맞는 인스턴스 우선
            matched = []
            for inst in instances:
                iv = str(inst.get("installationVersion", "0"))
                try:
                    installed_major = int(iv.split(".")[0])
                except Exception:
                    installed_major = 0
                if maj is None or installed_major == maj:
                    matched.append(inst)

            # 우선 matched(요청 메이저)에서 최신 선택, 없으면 전체에서 최신 선택
            for pool in (matched, instances):
                if not pool:
                    continue
                pool = sorted(pool, key=lambda i: parse_ver(str(i.get("installationVersion","0"))), reverse=True)
                for inst in pool:
                    msb = self._pick_msbuild_from_install_path(inst.get("installationPath",""))
                    if msb:
                        print(f"[DEBUG] MSBuild via vswhere: {msb}")
                        return msb

        # 2) vswhere 실패/미탑재 시 보수적 폴백
        editions = ["Community", "Professional", "Enterprise", "BuildTools"]
        program_files = [
            os.environ.get("ProgramFiles(x86)", r"C:\Program Files (x86)"),
            os.environ.get("ProgramFiles", r"C:\Program Files"),
        ]
        guesses = []
        if maj is None or maj >= 17:
            for pf in program_files:
                for ed in editions:
                    guesses.append(Path(pf) / "Microsoft Visual Studio" / "2022" / ed / "MSBuild" / "Current" / "Bin" / "MSBuild.exe")
        if maj == 16:
            for ed in editions:
                guesses.append(Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2019") / ed / "MSBuild" / "Current" / "Bin" / "MSBuild.exe")
        if maj == 15:
            for ed in editions:
                guesses.append(Path(r"C:\Program Files (x86)\Microsoft Visual Studio\2017") / ed / "MSBuild" / "15.0" / "Bin" / "MSBuild.exe")

        for g in guesses:
            if g.exists():
                msb = str(g)
                print(f"[DEBUG] MSBuild via hardcoded guess: {msb}")
                return msb

        print("[DEBUG] MSBuild path could not be resolved.")
        return None