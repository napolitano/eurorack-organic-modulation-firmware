#!/usr/bin/env python3
"""Generate deterministic SVG figures for Drift's Electronica bank.

The switch diagrams reuse the established Drift manual switch geometry. The
algorithm figures visualise the implemented control contracts and are not
oscilloscope captures.

SPDX-License-Identifier: CC-BY-NC-4.0
"""
from __future__ import annotations

from pathlib import Path

OUT = Path(__file__).resolve().parents[1] / "docs" / "manual" / "assets"
ORANGE = "#e64a2b"
ORANGE2 = "#f27962"
GREY = "#8d8d8d"
LIGHT = "#dedede"
DARK = "#1a1a1a"
STYLE = '''
<style>
.title{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:29px;font-weight:700;fill:#1a1a1a}
.subtitle{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:18px;font-weight:300;fill:#6e6e6e}
.label{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:18px;font-weight:500;fill:#1a1a1a}
.small{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:15px;font-weight:300;fill:#6e6e6e}
</style>'''


def switch_asset(name: str, switch1_on: bool, switch2_on: bool) -> str:
    def slider(x: int, is_on: bool) -> str:
        y = 61 if is_on else 103
        color = ORANGE if is_on else GREY
        return f'<rect x="{x+12}" y="{y}" width="54" height="30" rx="5" fill="{color}"/>'
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 450 288" role="img" aria-label="{name} configuration switches">
<title>{name} configuration switches</title>
<style>text{{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif}}</style>
<rect width="450" height="288" fill="white"/>
<rect x="80" y="30" width="290" height="135" rx="12" fill="#f1f1f1" stroke="#1a1a1a" stroke-width="6"/>
<rect x="100" y="49" width="78" height="93" rx="9" fill="white" stroke="#1a1a1a" stroke-width="5"/>
{slider(100, switch1_on)}
<text x="139" y="190" text-anchor="middle" font-size="25" font-weight="300" fill="#8d8d8d">1</text>
<rect x="245" y="49" width="78" height="93" rx="9" fill="white" stroke="#1a1a1a" stroke-width="5"/>
{slider(245, switch2_on)}
<text x="284" y="190" text-anchor="middle" font-size="25" font-weight="300" fill="#8d8d8d">2</text>
<text x="225" y="248" text-anchor="middle" font-size="31" font-weight="700" fill="#1a1a1a">{name}</text>
</svg>'''


def smoothstep(x: float) -> float:
    x = max(0.0, min(1.0, x))
    return x*x*(3.0 - 2.0*x)


def polyline(values: list[float], x: float, y: float, w: float, h: float, vmin: float, vmax: float) -> str:
    span = max(1e-12, vmax-vmin)
    pts=[]
    for i,v in enumerate(values):
        px=x+w*(i/(len(values)-1 if len(values)>1 else 1))
        py=y+h*(1-(v-vmin)/span)
        pts.append(f"{px:.1f},{py:.1f}")
    return " ".join(pts)


def pump_asset() -> str:
    n=220
    def curve(endpoint: float) -> list[float]:
        out=[]
        for i in range(n):
            p=i/(n-1)
            out.append(1.0 if p>=endpoint else smoothstep(p/endpoint))
        return out
    fast=curve(0.25); long=curve(15/16)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Pump duck and recovery contours">
<title>Pump duck and recovery contours</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Pump — reset, then recover</text>
<text x="34" y="87" class="subtitle">Every internal quarter note starts at zero; Texture moves the recovery endpoint from 1/4 to 15/16 of the beat</text>
<line x1="110" y1="337" x2="1015" y2="337" stroke="{LIGHT}" stroke-width="2"/>
<line x1="110" y1="118" x2="110" y2="337" stroke="{ORANGE}" stroke-width="3"/>
<polyline points="{polyline(fast,110,118,905,219,0,1)}" fill="none" stroke="{GREY}" stroke-width="4"/>
<polyline points="{polyline(long,110,118,905,219,0,1)}" fill="none" stroke="{ORANGE}" stroke-width="5"/>
<text x="110" y="385" class="small">beat boundary / instant duck</text>
<text x="390" y="385" class="small">grey: low Texture / fast recovery</text>
<text x="718" y="385" class="small">orange: high Texture / late recovery</text>
</svg>'''


def acid_asset() -> str:
    levels=[1024+128*((5*n+3)%16) for n in range(16)]
    accent=[(n%4==0) or (n%7==0) for n in range(16)]
    slide=[((5*n)%16)<4 for n in range(16)]
    x0=75; y0=345; w=920; step=w/16
    items=[]
    for n,lvl in enumerate(levels):
        x=x0+n*step
        y=y0-(lvl-900)/(3900-900)*205
        items.append(f'<line x1="{x:.1f}" y1="{y0}" x2="{x:.1f}" y2="{y:.1f}" stroke="{ORANGE if accent[n] else GREY}" stroke-width="{7 if accent[n] else 5}"/>')
        items.append(f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{7 if accent[n] else 5}" fill="{ORANGE if accent[n] else GREY}"/>')
        if slide[n]:
            items.append(f'<path d="M {x+7:.1f} {y-15:.1f} q 12 -15 24 0" fill="none" stroke="{ORANGE2}" stroke-width="3"/>')
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Acid deterministic sixteen-step contour grammar">
<title>Acid deterministic sixteen-step contour grammar</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Acid — fixed 16-step CV grammar</text>
<text x="34" y="87" class="subtitle">The project-defined permutation supplies the levels; marked steps receive accent and/or Texture-controlled slide</text>
<line x1="75" y1="345" x2="995" y2="345" stroke="{LIGHT}" stroke-width="2"/>
{''.join(items)}
<text x="75" y="394" class="small">orange = accent</text>
<path d="M 245 389 q 12 -15 24 0" fill="none" stroke="{ORANGE2}" stroke-width="3"/><text x="280" y="394" class="small">curve mark = slide</text>
<text x="640" y="394" class="small">levels repeat exactly after 16 sixteenth-note steps</text>
</svg>'''


def shuffle_asset() -> str:
    x0=105; pair=820
    straight=[x0, x0+pair*0.5, x0+pair]
    swung=[x0, x0+pair*0.75, x0+pair]
    def marks(xs,y,color):
        return ''.join(f'<line x1="{x:.1f}" y1="{y-20}" x2="{x:.1f}" y2="{y+20}" stroke="{color}" stroke-width="5"/>' for x in xs)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Shuffle straight to three-to-one timing">
<title>Shuffle straight to three-to-one timing</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Shuffle — move the second onset, not the pair</text>
<text x="34" y="87" class="subtitle">Pair duration stays fixed while Texture delays the second sixteenth from 1/2 to 3/4 of the pair</text>
<text x="34" y="172" class="label">Straight</text><line x1="105" y1="164" x2="925" y2="164" stroke="{LIGHT}" stroke-width="4"/>{marks(straight,164,GREY)}
<text x="34" y="278" class="label">3:1 max</text><line x1="105" y1="270" x2="925" y2="270" stroke="{LIGHT}" stroke-width="4"/>{marks(swung,270,ORANGE)}
<path d="M 105 324 L 720 324" stroke="{ORANGE}" stroke-width="5"/><path d="M 720 324 L 925 324" stroke="{GREY}" stroke-width="5"/>
<text x="300" y="358" class="small">long = 3/4 pair</text><text x="765" y="358" class="small">short = 1/4 pair</text>
<text x="105" y="414" class="small">The two intervals always sum to one complete pair, so Shuffle introduces no cumulative tempo drift.</text>
</svg>'''


def polymeter_asset() -> str:
    # Show the 4-against-7 case because it demonstrates a long but readable 28-step recurrence.
    n=28; x0=80; w=930; step=w/(n-1)
    lines=[]
    for i in range(n):
        x=x0+i*step
        a=(i%4==0); b=(i%7==0)
        if a or b:
            h=72 if a and b else 46
            col=ORANGE if a and b else (ORANGE2 if a else GREY)
            lines.append(f'<line x1="{x:.1f}" y1="320" x2="{x:.1f}" y2="{320-h}" stroke="{col}" stroke-width="5"/>')
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Polymeter four against seven recurrence">
<title>Polymeter four against seven recurrence</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Polymeter — two cycle lengths, one shared grid</text>
<text x="34" y="87" class="subtitle">Example: the fixed four-step anchor against Texture-selected seven-step cycle realigns after 28 sixteenths</text>
<line x1="80" y1="320" x2="1010" y2="320" stroke="{LIGHT}" stroke-width="2"/>
{''.join(lines)}
<text x="80" y="370" class="small">grey: 4-step start</text><text x="285" y="370" class="small">light orange: 7-step start</text><text x="570" y="370" class="small">dark orange: coincidence / strongest accent</text>
<text x="80" y="414" class="small">Other Texture regions select 3, 5 or 9 steps, giving exact recurrences of 12, 20 or 36 sixteenths.</text>
</svg>'''


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    assets={
        "config-pump.svg": switch_asset("Pump",False,False),
        "config-acid.svg": switch_asset("Acid",True,False),
        "config-shuffle.svg": switch_asset("Shuffle",False,True),
        "config-polymeter.svg": switch_asset("Polymeter",True,True),
        "pump-ducking.svg": pump_asset(),
        "acid-contour.svg": acid_asset(),
        "shuffle-timing.svg": shuffle_asset(),
        "polymeter-cycle.svg": polymeter_asset(),
    }
    for name,data in assets.items():
        (OUT/name).write_text(data,encoding="utf-8")
    print(f"generated {len(assets)} Electronica manual assets")

if __name__ == "__main__":
    main()
