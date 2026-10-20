#!/usr/bin/env python3
"""Generate the ENTRO V instrument logo marks, lockups and app icons.

The SVG files are the design source.  Raster exports use macOS QuickLook
(qlmanage) and sips; .ico files are written with Pillow.  Run on macOS:

    python3 scripts/generate-logo-assets.py

Generated files are committed so CI does not need these tools.
"""

from __future__ import annotations

import math
import shutil
import subprocess
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
LOGO = ROOT / "assets" / "logos"
ICONS = ROOT / "assets" / "icons"
PNG = LOGO / "png"
TMP = Path("/tmp/entro-logo-render")

SILVER = "#EDE7DC"
MUTED = "#8F8C85"
DARK = "#121316"

INSTRUMENTS = {
    "matriarch": {
        "name": "MATRIARCH",
        "subtitle": "SEMI-MODULAR ANALOG SYNTHESIZER",
        "accent": "#CF4B2E",
        "title": "MATRIARCH V",
    },
    "polybrute": {
        "name": "POLYBRUTE",
        "subtitle": "SIX-VOICE ANALOG SYNTHESIZER",
        "accent": "#3F93E8",
        "title": "POLYBRUTE V",
    },
    "one": {
        "name": "ONE",
        "subtitle": "POLYPHONIC ANALOG SYNTHESIZER",
        "accent": "#D9A441",
        "title": "ONE V",
    },
}


def arc(cx: float, cy: float, r: float, a0: float, a1: float) -> str:
    """SVG arc from angle a0 to a1, clockwise, in degrees."""
    x0 = cx + r * math.cos(math.radians(a0))
    y0 = cy + r * math.sin(math.radians(a0))
    x1 = cx + r * math.cos(math.radians(a1))
    y1 = cy + r * math.sin(math.radians(a1))
    sweep = 1
    large = 1 if (a1 - a0) % 360.0 > 180.0 else 0
    return f"M {x0:.3f} {y0:.3f} A {r:.3f} {r:.3f} 0 {large} {sweep} {x1:.3f} {y1:.3f}"


def mark_svg(kind: str) -> str:
    accent = INSTRUMENTS[kind]["accent"]
    rail = (
        f'<path d="M16 96 H50 L64 106 L78 96 H112" fill="none" '
        f'stroke="{accent}" stroke-width="6" stroke-linecap="round" stroke-linejoin="round"/>'
    )

    if kind == "matriarch":
        bars = [
            (30, 26),
            (52, 48),
            (76, 48),
            (98, 26),
        ]
        body = "\n".join(
            f'    <line x1="{x}" y1="94" x2="{x}" y2="{y}"/>' for x, y in bars
        )
        art = f"""  <g stroke="{SILVER}" stroke-width="10" stroke-linecap="round">
{body}
  </g>
  {rail}"""

    elif kind == "polybrute":
        bars = [(22, 62), (38, 42), (54, 30), (70, 34), (86, 50), (102, 66)]
        body = "\n".join(
            f'    <line x1="{x}" y1="94" x2="{x}" y2="{y}"/>'
            for x, y in bars
            if x != 54
        )
        art = f"""  <g stroke="{SILVER}" stroke-width="7" stroke-linecap="round">
{body}
  </g>
  <line x1="54" y1="94" x2="54" y2="30" stroke="{accent}" stroke-width="7" stroke-linecap="round"/>
  {rail}"""

    elif kind == "one":
        arcs = " ".join(
            arc(64, 60, 31, a0, a1)
            for a0, a1 in ((108, 192), (228, 312), (348, 432))
        )
        art = f"""  <path d="{arcs}" fill="none" stroke="{SILVER}" stroke-width="9" stroke-linecap="round"/>
  <circle cx="64" cy="60" r="7" fill="{accent}"/>"""

    else:
        raise ValueError(kind)

    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="1024" height="1024" viewBox="0 0 128 128">
{art}
</svg>
"""


def lockup_svg(kind: str) -> str:
    info = INSTRUMENTS[kind]
    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="620" height="160" viewBox="0 0 620 160">
  <g transform="translate(16,16)">
{mark_svg(kind).split('<svg', 1)[1].rsplit('</svg>', 1)[0].replace('  ', '    ').strip()}
  </g>
  <text x="168" y="70" fill="{SILVER}" font-family="Helvetica Neue, Arial, sans-serif"
        font-size="46" font-weight="700" letter-spacing="4">{info['name']} <tspan fill="{info['accent']}">V</tspan></text>
  <text x="170" y="102" fill="{MUTED}" font-family="Helvetica Neue, Arial, sans-serif"
        font-size="9.5" font-weight="500" letter-spacing="3.2">{info['subtitle']}</text>
</svg>
"""


def icon_svg(kind: str) -> str:
    info = INSTRUMENTS[kind]
    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="1024" height="1024" viewBox="0 0 1024 1024">
  <defs>
    <linearGradient id="bg" x1="0" y1="0" x2="0" y2="1">
      <stop offset="0" stop-color="#24252a"/>
      <stop offset="1" stop-color="#0c0d0f"/>
    </linearGradient>
    <radialGradient id="glow" cx="0.5" cy="0.45" r="0.5">
      <stop offset="0" stop-color="{info['accent']}" stop-opacity="0.12"/>
      <stop offset="1" stop-color="{info['accent']}" stop-opacity="0"/>
    </radialGradient>
  </defs>
  <rect width="1024" height="1024" rx="224" fill="url(#bg)"/>
  <rect width="1024" height="1024" rx="224" fill="url(#glow)"/>
  <rect x="26" y="26" width="972" height="972" rx="202" fill="none"
        stroke="#ffffff" stroke-opacity="0.10" stroke-width="3"/>
  <g transform="translate(192,192) scale(5)">
{mark_svg(kind).split('<svg', 1)[1].rsplit('</svg>', 1)[0].strip()}
  </g>
</svg>
"""


def family_svg() -> str:
    marks = []
    labels = []
    x = 54
    for kind in ("matriarch", "polybrute", "one"):
        info = INSTRUMENTS[kind]
        body = mark_svg(kind).split('<svg', 1)[1].rsplit('</svg>', 1)[0].strip()
        marks.append(f'  <g transform="translate({x},150) scale(1.35)">\n{body}\n  </g>')
        labels.append(
            f'  <text x="{x + 86}" y="342" fill="{SILVER}" text-anchor="middle" '
            f'font-family="Helvetica Neue, Arial, sans-serif" font-size="17" '
            f'font-weight="700" letter-spacing="2">{info["title"]}</text>\n'
            f'  <rect x="{x + 44}" y="354" width="84" height="3" fill="{info["accent"]}"/>'
        )
        x += 252

    return f"""<svg xmlns="http://www.w3.org/2000/svg" width="900" height="420" viewBox="0 0 900 420">
  <rect width="900" height="420" fill="{DARK}"/>
  <text x="450" y="58" fill="{SILVER}" text-anchor="middle"
        font-family="Helvetica Neue, Arial, sans-serif" font-size="34"
        font-weight="700" letter-spacing="7">ENTRO <tspan fill="#CF4B2E">V</tspan> COLLECTION</text>
  <text x="450" y="84" fill="{MUTED}" text-anchor="middle"
        font-family="Helvetica Neue, Arial, sans-serif" font-size="9"
        font-weight="500" letter-spacing="4.4">THREE VIRTUAL-ANALOG INSTRUMENTS</text>
{chr(10).join(marks)}
{chr(10).join(labels)}
</svg>
"""


CHROME = "/Applications/Google Chrome.app/Contents/MacOS/Google Chrome"


def render(svg: Path, width: int, height: int, out: Path) -> None:
    TMP.mkdir(parents=True, exist_ok=True)
    subprocess.run(
        [CHROME, "--headless", "--disable-gpu", "--no-sandbox", "--hide-scrollbars",
         "--default-background-color=00000000", f"--window-size={width},{height}",
         f"--screenshot={out}", f"file://{svg}"],
        check=True,
        stdout=subprocess.DEVNULL,
        stderr=subprocess.DEVNULL,
    )
    if not out.exists():
        raise RuntimeError(f"Chrome did not produce {out}")


def build_icns(kind: str, source: Path) -> None:
    iconset = TMP / f"{kind}.iconset"
    if iconset.exists():
        shutil.rmtree(iconset)
    iconset.mkdir(parents=True)
    for size in (16, 32, 128, 256, 512):
        for scale, suffix in ((1, ""), (2, "@2x")):
            px = size * scale
            subprocess.run(
                ["sips", "-z", str(px), str(px), str(source), "--out",
                 str(iconset / f"icon_{size}x{size}{suffix}.png")],
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
    subprocess.run(
        ["iconutil", "-c", "icns", str(iconset), "-o", str(ICONS / f"{kind}.icns")],
        check=True,
    )


def build_ico(kind: str, source: Path) -> None:
    image = Image.open(source).convert("RGBA")
    sizes = [(16, 16), (24, 24), (32, 32), (48, 48), (64, 64), (128, 128), (256, 256)]
    image.save(ICONS / f"{kind}.ico", sizes=sizes)


def main() -> None:
    LOGO.mkdir(parents=True, exist_ok=True)
    ICONS.mkdir(parents=True, exist_ok=True)
    PNG.mkdir(parents=True, exist_ok=True)

    for kind in INSTRUMENTS:
        (LOGO / f"{kind}-v-mark.svg").write_text(mark_svg(kind), encoding="utf-8")
        (LOGO / f"{kind}-v-lockup.svg").write_text(lockup_svg(kind), encoding="utf-8")
        render(LOGO / f"{kind}-v-lockup.svg", 620, 160, PNG / f"{kind}-v-lockup.png")
        (LOGO / f"{kind}-v-icon.svg").write_text(icon_svg(kind), encoding="utf-8")

        icon = ICONS / f"{kind}-1024.png"
        render(LOGO / f"{kind}-v-icon.svg", 1024, 1024, icon)

        mark1024 = PNG / f"{kind}-v-mark-1024.png"
        render(LOGO / f"{kind}-v-mark.svg", 1024, 1024, mark1024)

        for size in (512, 256, 128, 64, 32):
            subprocess.run(
                ["sips", "-z", str(size), str(size), str(mark1024), "--out",
                 str(PNG / f"{kind}-v-mark-{size}.png")],
                check=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

        build_icns(kind, icon)
        build_ico(kind, icon)

    family = LOGO / "entro-v-collection.svg"
    family.write_text(family_svg(), encoding="utf-8")
    render(family, 900, 420, LOGO / "entro-v-collection.png")

    print("Generated logo marks, lockups, icons and family image.")


if __name__ == "__main__":
    main()
