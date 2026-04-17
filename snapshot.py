import sys
from pathlib import Path

sys.stdout = open("snapshot.txt", "wt", encoding='utf-8')

REPO_DIR = Path(".")

target_ext = (
    "h", "hpp", "cpp", "md", "ino", "yml", "sh", "py", "ini", "json", "MD",
)

target_dirs = (
    "DJC-Firmware/src", ".github"
)

def _write(f: Path):
    if f.stem == "compile_commands":
        return
    
    try:
        print(f"{file.relative_to(REPO_DIR)}")
        print(f"```{f.suffix}")
        print(file.read_text(encoding='utf-8', errors='ignore').rstrip())
        print("```\n")
    except:
        pass

_write(REPO_DIR / "makefile")

for ext in target_ext:
    for file in REPO_DIR.glob(f"*.{ext}"):
        _write(file)

for d in target_dirs:
    _dir = REPO_DIR / d
    for ext in target_ext:
        for file in _dir.rglob(f"*.{ext}"):
            _write(file)


