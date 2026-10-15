"""Compare the modular firmware against the committed working baseline.

Run from an x64 MSVC developer command prompt: python tests/run_regression.py
The historical source is read from Git into a temporary build directory;
no duplicate project/backup is stored in the repository.
"""
from pathlib import Path
import re
import shutil
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BASELINE = "a01b34162f16ca036cab11fd1f001bc98421704f"


def main():
    if not shutil.which("cl"):
        raise SystemExit("Use an x64 MSVC developer command prompt (cl.exe required).")
    source = subprocess.check_output(
        ["git", "show", f"{BASELINE}:SF6/main.c"], cwd=ROOT
    ).decode("utf-8", errors="replace").replace("\r\n", "\n").replace("\r", "\n")
    # Comments in the historical file have mixed encoding; remove only comments.
    source = re.sub(r"//[^\n]*", "", source)
    symbols = ["main", "debug_in", "tick_flag", "test_flag", "GPIO_Init_All",
               "ReadInputs", "Output_Update", "SysTick_Init", "SysTick_Handler",
               "Logic_Process", "EnqueueAction", "sp1_key", "sp2_key", "sp3_key",
               "sp4_key", "super_key", "action_queue", "queue_head", "queue_tail",
               "current_action"]
    for symbol in symbols:
        source = re.sub(r"\b" + symbol + r"\b", "old_" + symbol, source)
    with tempfile.TemporaryDirectory(prefix="sf6-regression-") as directory:
        build = Path(directory)
        legacy = build / "baseline.c"
        legacy.write_text(source, encoding="utf-8")
        sources = [ROOT / "tests/regression.c", legacy]
        for folder in ("Core", "Board", "Profiles"):
            sources.extend(sorted((ROOT / "SF6" / folder).glob("*.c")))
        subprocess.run(["cl", "/nologo", "/std:c11", "/W3", "/O2",
                        f"/I{ROOT / 'tests/stubs'}", f"/I{ROOT / 'SF6'}",
                        *map(str, sources), "/Fe:regression.exe"], cwd=build, check=True)
        subprocess.run([str(build / "regression.exe")], cwd=build, check=True)


if __name__ == "__main__":
    main()
