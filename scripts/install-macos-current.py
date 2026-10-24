#!/usr/bin/env python3
"""Install a complete tested build, backing up installed bundles before replacement."""
import argparse
import datetime
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
PRODUCTS = ("MATRIARCH V", "POLYBRUTE V", "ONE V", "CONTINUUM V")

def run(*args):
    subprocess.run(args, check=True, capture_output=True)

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--source", type=Path, default=ROOT / "dist")
    parser.add_argument("--backup", type=Path, default=ROOT.parents[1] / "work" / "install-backup")
    args = parser.parse_args()
    if sys.platform != "darwin":
        parser.error("macOS installer requires macOS")
    stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    backup = args.backup / stamp
    formats = (("vst3", Path.home() / "Library/Audio/Plug-Ins/VST3"),
               ("component", Path.home() / "Library/Audio/Plug-Ins/Components"),
               ("app", Path("/Applications")))
    items = [(args.source / f"{name}.{ext}", folder / f"{name}.{ext}", ext)
             for name in PRODUCTS for ext, folder in formats]
    staged, replaced = [], []
    try:
        # Preflight all 12 sources before changing any installed product.
        for src, dst, ext in items:
            if not src.is_dir():
                raise RuntimeError(f"Missing bundle: {src}")
            run("codesign", "--force", "--deep", "--sign", "-", str(src))
            run("codesign", "--verify", "--deep", "--strict", str(src))
        for src, dst, ext in items:
            dst.parent.mkdir(parents=True, exist_ok=True)
            temp = dst.parent / ("." + dst.name + ".installing-" + stamp)
            run("ditto", str(src), str(temp))
            staged.append(temp)
            run("codesign", "--verify", "--deep", "--strict", str(temp))
            if dst.exists():
                old = backup / ext / dst.name
                old.parent.mkdir(parents=True, exist_ok=True)
                run("ditto", str(dst), str(old))
        for (src, dst, ext), temp in zip(items, staged):
            replaced.append((dst, backup / ext / dst.name))
            if dst.exists():
                shutil.rmtree(dst)
            temp.rename(dst)
            run("codesign", "--verify", "--deep", "--strict", str(dst))
            for executable in (src / "Contents/MacOS").iterdir():
                installed = dst / "Contents/MacOS" / executable.name
                if hashlib.sha256(executable.read_bytes()).digest() != hashlib.sha256(installed.read_bytes()).digest():
                    raise RuntimeError(f"Installed executable differs: {installed}")
            print("Installed:", dst, flush=True)
    except Exception:
        for dst, old in reversed(replaced):
            if dst.exists():
                shutil.rmtree(dst)
            if old.exists():
                run("ditto", str(old), str(dst))
        raise
    finally:
        for temp in staged:
            if temp.exists():
                shutil.rmtree(temp)
    print("Previous versions:", backup)
    print("Restart the DAW/standalone process to load the installed binaries.")

if __name__ == "__main__":
    main()
