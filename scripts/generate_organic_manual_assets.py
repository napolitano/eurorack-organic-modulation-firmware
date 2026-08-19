#!/usr/bin/env python3
"""Generate deterministic SVG figures for the Organic Drift algorithm bank.

The figures are explanatory visualisations derived from the documented production
control laws. They are not oscilloscope captures.

SPDX-License-Identifier: GPL-3.0-or-later
"""
from __future__ import annotations

import math
from pathlib import Path
from typing import Iterable

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "docs" / "manual" / "assets"
ORANGE = "#e64a2b"
ORANGE2 = "#e8722a"
YELLOW = "#efc93a"
DARK = "#1a1a1a"
GREY = "#9a9a9a"
LIGHT = "#eeeeee"

STYLE = '''<style>
  text { font-family: Ubuntu, "Ubuntu Sans", Arial, sans-serif; }
  .title { font-size: 34px; font-weight: 700; fill: #1a1a1a; }
  .subtitle { font-size: 24px; font-weight: 300; fill: #8d8d8d; }
  .label { font-size: 21px; font-weight: 500; fill: #1a1a1a; }
  .small { font-size: 17px; font-weight: 300; fill: #777; }
</style>'''


def points(values: Iterable[float], x0: float, y0: float, width: float, height: float,
           lo: float | None = None, hi: float | None = None) -> str:
    vals = list(values)
    if not vals:
        return ""
    min_v = min(vals) if lo is None else lo
    max_v = max(vals) if hi is None else hi
    span = max(max_v - min_v, 1e-9)
    return " ".join(
        f"{x0 + width * i / max(1, len(vals)-1):.1f},{y0 + height * (1.0 - (v-min_v)/span):.1f}"
        for i, v in enumerate(vals)
    )


def gradient_noise(x: float, gradients: list[float]) -> float:
    i = math.floor(x)
    t = x - i
    g0 = gradients[i % len(gradients)]
    g1 = gradients[(i + 1) % len(gradients)]
    fade = 6*t**5 - 15*t**4 + 10*t**3
    return (1-fade) * g0 * t + fade * g1 * (t-1)


def fractal_asset() -> str:
    gradients = [0.92, -0.61, 0.44, -0.88, 0.73, 0.31, -0.47, 0.84, -0.28, 0.57, -0.76, 0.39]
    xs = [i * 0.018 for i in range(520)]
    low = [gradient_noise(x, gradients) for x in xs]
    high = [
        (512*gradient_noise(x, gradients)
         + 320*gradient_noise(4*x + 0.37, gradients)
         + 192*gradient_noise(16*x + 0.71, gradients)) / 1024.0
        for x in xs
    ]
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 430" role="img" aria-label="Fractal texture comparison">
<title>Fractal texture comparison</title>{STYLE}
<rect width="1056" height="430" fill="white"/>
<text x="34" y="52" class="title">Fractal — detail across three scales</text>
<text x="34" y="87" class="subtitle">Texture redistributes constant gain from macro motion into 4× and 16× detail</text>
<line x1="46" y1="196" x2="1010" y2="196" stroke="#e6e6e6" stroke-width="2"/>
<polyline points="{points(low,46,115,964,145,-0.8,0.8)}" fill="none" stroke="{GREY}" stroke-width="5" stroke-linecap="round" stroke-linejoin="round"/>
<text x="46" y="285" class="label">Texture 0% — 1× macro layer only</text>
<line x1="46" y1="350" x2="1010" y2="350" stroke="#e6e6e6" stroke-width="2"/>
<polyline points="{points(high,46,273,964,125,-0.8,0.8)}" fill="none" stroke="{ORANGE}" stroke-width="5" stroke-linecap="round" stroke-linejoin="round"/>
<text x="590" y="414" class="label">Texture 100% — 512 / 320 / 192</text>
</svg>'''


def tri(phase: float) -> float:
    p = phase % 1.0
    return -1.0 + 4.0*p if p < 0.5 else 3.0 - 4.0*p


def vector_asset() -> str:
    xp, yp = 0.0, 0.25
    trajectory: list[tuple[float,float]] = []
    projected: list[float] = []
    base_x = 0.0065
    base_y = base_x * 0.75
    coupling = 0.25
    for _ in range(760):
        xw, yw = tri(xp), tri(yp)
        xp = (xp + base_x * (1 + coupling*yw)) % 1.0
        yp = (yp + base_y * (1 - coupling*xw)) % 1.0
        xw, yw = tri(xp), tri(yp)
        trajectory.append((xw, yw))
        projected.append((xw+yw)/2)
    traj_pts = " ".join(
        f"{68 + (x+1)*205:.1f},{128 + (1-(y+1)/2)*250:.1f}" for x,y in trajectory
    )
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Vector coupled two-dimensional flow">
<title>Vector coupled two-dimensional flow</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Vector — coupled two-dimensional flow</text>
<text x="34" y="87" class="subtitle">Texture bends the trajectory by cross-coupling two forward-moving toroidal phases</text>
<rect x="68" y="128" width="410" height="250" rx="8" fill="#fafafa" stroke="#dedede" stroke-width="2"/>
<line x1="273" y1="128" x2="273" y2="378" stroke="#e6e6e6"/><line x1="68" y1="253" x2="478" y2="253" stroke="#e6e6e6"/>
<polyline points="{traj_pts}" fill="none" stroke="{ORANGE}" stroke-width="3.2" stroke-linecap="round" stroke-linejoin="round" opacity="0.92"/>
<text x="68" y="413" class="label">Internal x/y trajectory</text>
<line x1="560" y1="253" x2="1015" y2="253" stroke="#e6e6e6" stroke-width="2"/>
<polyline points="{points(projected[:520],560,135,455,235,-1,1)}" fill="none" stroke="{ORANGE2}" stroke-width="5" stroke-linecap="round" stroke-linejoin="round"/>
<text x="560" y="413" class="label">Output = projection of x and y</text>
</svg>'''


def lfsr_next(states: tuple[int,int]) -> tuple[tuple[int,int], int]:
    primary, secondary = states
    def adv(state: int, taps: tuple[int,...]) -> int:
        bit = 0
        for pos in taps:
            bit ^= state >> (16-pos)
        return ((state >> 1) | ((bit & 1) << 15)) & 0xFFFF
    primary = adv(primary, (16,14,13,11))
    secondary = adv(secondary, (16,15,13,4))
    return (primary, secondary), primary ^ secondary


def rain_trace(density: int, speed: int, count: int = 560) -> list[float]:
    seed = 0x5A17
    swapped = ((seed >> 8) & 0xFF) | ((seed << 8) & 0xFF00)
    states = (seed, (~swapped) & 0xFFFF)
    env = 0
    residual = 0
    cutoff = (min(density,1023) ** 2) >> 6
    alpha = 4 + (min(speed,1023) << 3)
    out: list[float] = []
    for _ in range(count):
        if env == 0:
            residual = 0
        else:
            full = env * alpha + residual
            decay = full >> 16
            residual = full & 0xFFFF
            if decay >= env:
                env, residual = 0, 0
            else:
                env -= decay
        states, rnd = lfsr_next(states)
        if rnd < cutoff:
            states, amp_rnd = lfsr_next(states)
            amp = 4096 + (amp_rnd & 0x3FFF)
            env = min(0xFFFF, env + amp)
        out.append(env / 65535.0)
    return out


def rain_asset() -> str:
    rows = [(190, "Low Density"), (520, "Medium Density"), (900, "High Density")]
    colors = [GREY, ORANGE2, ORANGE]
    ytops = [118, 230, 342]
    pieces = []
    for (density,label),color,y in zip(rows,colors,ytops):
        trace = rain_trace(density, 520)
        pieces.append(f'<line x1="220" y1="{y+72}" x2="1015" y2="{y+72}" stroke="#e6e6e6" stroke-width="2"/>')
        pieces.append(f'<polyline points="{points(trace,220,y,795,72,0,1)}" fill="none" stroke="{color}" stroke-width="4" stroke-linecap="round" stroke-linejoin="round"/>')
        pieces.append(f'<text x="42" y="{y+45}" class="label">{label}</text>')
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Rain density comparison">
<title>Rain density comparison</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Rain — Density controls event rate</text>
<text x="34" y="87" class="subtitle">Random impulses accumulate into one decaying envelope; Speed controls the tail</text>
{''.join(pieces)}
</svg>'''


def henon_fixed(count: int = 900, a_q: int = 22938, b_q: int = 4915) -> list[tuple[float,float]]:
    x, y = 0, 0
    pts: list[tuple[float,float]] = []
    for i in range(count + 80):
        x_sq = (x*x + (1<<13)) >> 14
        scaled = (x_sq*a_q + (1<<13)) >> 14
        nx = 16384 + y - scaled
        ny = (x*b_q) >> 14
        nx = max(-32768, min(32767, nx)); ny = max(-32768, min(32767, ny))
        x,y = nx,ny
        if i >= 80:
            pts.append((x/16384.0, y/16384.0))
    return pts


def attractor_asset() -> str:
    # Texture ~= 75% maps to a ~= 1.35 and produces a long fixed-point orbit,
    # making the implemented nonlinear structure easier to see than the much
    # shorter full-scale cycle.
    pts = henon_fixed(a_q=22119)
    xs=[p[0] for p in pts]
    minx,maxx=min(p[0] for p in pts),max(p[0] for p in pts)
    miny,maxy=min(p[1] for p in pts),max(p[1] for p in pts)
    cloud="".join(
        f'<circle cx="{65+(x-minx)/(maxx-minx)*420:.1f}" cy="{130+(1-(y-miny)/(maxy-miny))*245:.1f}" r="1.35" fill="{ORANGE}" opacity="0.68"/>'
        for x,y in pts[::2]
    )
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Hénon attractor structure and output projection">
<title>Hénon attractor structure and output projection</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Attractor — deterministic nonlinear structure</text>
<text x="34" y="87" class="subtitle">Shown at Texture ≈75%: fixed-point Hénon map with a ≈ 1.35 and b ≈ 0.30</text>
<rect x="65" y="130" width="420" height="245" rx="8" fill="#fafafa" stroke="#dedede" stroke-width="2"/>
{cloud}
<text x="65" y="413" class="label">Map states in x/y space</text>
<line x1="560" y1="253" x2="1015" y2="253" stroke="#e6e6e6" stroke-width="2"/>
<polyline points="{points(xs[:520],560,135,455,235,-2,2)}" fill="none" stroke="{ORANGE2}" stroke-width="4.5" stroke-linecap="round" stroke-linejoin="round"/>
<text x="560" y="413" class="label">Firmware interpolates the x projection</text>
</svg>'''


def bank_overview_asset() -> str:
    classic = ["Perlin", "Brownian", "Bézier", "LFO"]
    organic = ["Fractal", "Vector", "Rain", "Attractor"]
    dips = ["OFF / OFF", "ON / OFF", "OFF / ON", "ON / ON"]
    cells=[]
    for col,(dip,c,o) in enumerate(zip(dips,classic,organic)):
        x=218+col*202
        cells.append(f'<text x="{x}" y="132" text-anchor="middle" class="small">{dip}</text>')
        cells.append(f'<rect x="{x-88}" y="152" width="176" height="68" rx="9" fill="#f4f4f4"/><text x="{x}" y="194" text-anchor="middle" class="label">{c}</text>')
        cells.append(f'<rect x="{x-88}" y="260" width="176" height="68" rx="9" fill="#fff1ed"/><text x="{x}" y="302" text-anchor="middle" class="label">{o}</text>')
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 390" role="img" aria-label="Classic and Organic bank DIP mapping">
<title>Classic and Organic bank DIP mapping</title>{STYLE}
<rect width="1056" height="390" fill="white"/>
<text x="34" y="52" class="title">Same DIP positions, different compile-time bank</text>
<text x="34" y="87" class="subtitle">The flashed firmware chooses the bank; the rear switches choose one of its four slots at startup</text>
<text x="38" y="193" class="label">Classic</text><text x="38" y="301" class="label" fill="{ORANGE}">Organic</text>
{''.join(cells)}
</svg>'''



def switch_asset(name: str, switch1_on: bool, switch2_on: bool) -> str:
    def slider(x: int, is_on: bool) -> str:
        y = 61 if is_on else 103
        color = ORANGE if is_on else GREY
        return f'<rect x="{x+12}" y="{y}" width="54" height="30" rx="5" fill="{color}"/>'

    return f"""<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 450 288" role="img" aria-label="{name} configuration switches">
<title>{name} configuration switches</title>
<style>text{{font-family:Ubuntu,\"Ubuntu Sans\",Arial,sans-serif}}</style>
<rect width="450" height="288" fill="white"/>
<rect x="80" y="30" width="290" height="135" rx="12" fill="#f1f1f1" stroke="#1a1a1a" stroke-width="6"/>
<rect x="100" y="49" width="78" height="93" rx="9" fill="white" stroke="#1a1a1a" stroke-width="5"/>
{slider(100, switch1_on)}
<text x="139" y="190" text-anchor="middle" font-size="25" font-weight="300" fill="#8d8d8d">1</text>
<rect x="245" y="49" width="78" height="93" rx="9" fill="white" stroke="#1a1a1a" stroke-width="5"/>
{slider(245, switch2_on)}
<text x="284" y="190" text-anchor="middle" font-size="25" font-weight="300" fill="#8d8d8d">2</text>
<text x="225" y="248" text-anchor="middle" font-size="31" font-weight="700" fill="#1a1a1a">{name}</text>
</svg>"""

def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    assets = {
        "organic-bank-overview.svg": bank_overview_asset(),
        "config-fractal.svg": switch_asset("Fractal", False, False),
        "config-vector.svg": switch_asset("Vector", True, False),
        "config-rain.svg": switch_asset("Rain", False, True),
        "config-attractor.svg": switch_asset("Attractor", True, True),
        "fractal-texture.svg": fractal_asset(),
        "vector-flow.svg": vector_asset(),
        "rain-density.svg": rain_asset(),
        "attractor-henon.svg": attractor_asset(),
    }
    for name, content in assets.items():
        (OUT / name).write_text(content + "\n", encoding="utf-8")
        print(OUT / name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
