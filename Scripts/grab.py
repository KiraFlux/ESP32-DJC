from pathlib import Path

DIR = Path("../DJC-Firmware/src")

for file in DIR.rglob("*.hpp"):
    try:
        print(f"{file.relative_to(DIR)}")
        print("```cpp")
        print(file.read_text(encoding='utf-8', errors='ignore').rstrip())
        print("```\n")
    except:
        pass

for file in DIR.rglob("*.cpp"):
    try:
        print(f"{file.relative_to(DIR)}")
        print("```cpp")
        print(file.read_text(encoding='utf-8', errors='ignore').rstrip())
        print("```\n")
    except:
        pass
