#!/usr/bin/env python3
"""Generate deterministic SVG figures for Drift's Ambient bank.

The switch diagrams reuse the established Drift manual switch geometry. The
algorithm figures are explanatory visualisations of the implemented control
contracts; they are not oscilloscope captures.

SPDX-License-Identifier: CC-BY-NC-4.0
"""
from __future__ import annotations

from pathlib import Path
import math
import random

OUT = Path(__file__).resolve().parents[1] / "docs" / "manual" / "assets"
ORANGE = "#e64a2b"
ORANGE2 = "#f27962"
GREY = "#8d8d8d"
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


def polyline(values: list[float], x: float, y: float, w: float, h: float, vmin: float, vmax: float) -> str:
    span = max(1e-12, vmax-vmin)
    pts=[]
    for i,v in enumerate(values):
        px=x+w*(i/(len(values)-1 if len(values)>1 else 1))
        py=y+h*(1-(v-vmin)/span)
        pts.append(f"{px:.1f},{py:.1f}")
    return " ".join(pts)


def smoothstep(x: float) -> float:
    x=max(0.0,min(1.0,x))
    return x*x*(3.0-2.0*x)


def soft_triangle(phase: float) -> float:
    p=phase%1.0
    tri=2*p if p<0.5 else 2*(1-p)
    return 2*smoothstep(tri)-1


def current_asset() -> str:
    n=220
    phases=(0.0,1/3,2/3)
    ratios=(1.0,362/256,414/256)
    low=(768,192,64)
    high=(512,320,192)
    low_out=[]; high_out=[]
    for i in range(n):
        t=i/42.0
        vals=[soft_triangle(phases[j]+t*ratios[j]) for j in range(3)]
        low_out.append(sum(vals[j]*low[j] for j in range(3))/1024)
        high_out.append(sum(vals[j]*high[j] for j in range(3))/1024)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Current long-form component weighting">
<title>Current long-form component weighting</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Current — three slow currents, one evolving contour</text>
<text x="34" y="87" class="subtitle">Fixed non-harmonic rate ratios prevent a short obvious loop; Texture redistributes a constant total weight</text>
<line x1="110" y1="209" x2="1015" y2="209" stroke="#e6e6e6"/>
<polyline points="{polyline(low_out,110,118,905,180,-1,1)}" fill="none" stroke="{GREY}" stroke-width="4" stroke-linejoin="round"/>
<polyline points="{polyline(high_out,110,118,905,180,-1,1)}" fill="none" stroke="{ORANGE}" stroke-width="5" stroke-linejoin="round"/>
<text x="42" y="336" class="label">Low Texture</text><line x1="165" y1="330" x2="215" y2="330" stroke="{GREY}" stroke-width="4"/>
<text x="275" y="336" class="label">High Texture</text><line x1="408" y1="330" x2="458" y2="330" stroke="{ORANGE}" stroke-width="5"/>
<text x="42" y="392" class="small">Implementation ratios: 1 : 362/256 : 414/256. The digital trajectory is finite and ultimately repeats, but not on an obvious musical cycle.</text>
</svg>'''


def triangular(rng: random.Random) -> float:
    return (rng.random()+rng.random()-1.0)


def anchor_asset() -> str:
    rng1=random.Random(0xA11CE); rng2=random.Random(0xA11CE)
    def trace(rng,spread):
        x=0.0; out=[]
        a=0.94
        for _ in range(150):
            x=a*x+triangular(rng)*spread
            x=max(-1,min(1,x)); out.append(x)
        return out
    low=trace(rng1,0.07); high=trace(rng2,0.20)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Anchor mean-reverting stochastic motion">
<title>Anchor mean-reverting stochastic motion</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Anchor — stochastic motion with a home</text>
<text x="34" y="87" class="subtitle">A restoring term continually pulls the state toward the midpoint; Texture sets the target spread around that anchor</text>
<line x1="110" y1="246" x2="1015" y2="246" stroke="#d6d6d6" stroke-width="2" stroke-dasharray="8 8"/>
<text x="42" y="251" class="small">centre</text>
<polyline points="{polyline(low,110,118,905,255,-1,1)}" fill="none" stroke="{GREY}" stroke-width="4" stroke-linejoin="round"/>
<polyline points="{polyline(high,110,118,905,255,-1,1)}" fill="none" stroke="{ORANGE}" stroke-width="4" stroke-linejoin="round" opacity="0.92"/>
<text x="110" y="413" class="small">grey: lower Texture / narrower spread</text>
<text x="455" y="413" class="small">orange: higher Texture / wider spread</text>
</svg>'''


def breath_curve(duration: float, amplitude: float, skew: float, samples: int=90) -> list[tuple[float,float]]:
    vals=[]
    for i in range(samples):
        t=duration*i/(samples-1)
        p=t/duration
        if p<skew:
            e=smoothstep(p/skew)
        else:
            e=1-smoothstep((p-skew)/(1-skew))
        vals.append((t,e*amplitude))
    return vals


def breath_asset() -> str:
    specs=[(1.05,.80,.36),(0.82,.98,.28),(1.18,.70,.48)]
    total=sum(s[0] for s in specs); x0=90; w=930; cursor=0.0
    paths=[]; boundaries=[]
    for idx,(dur,amp,skew) in enumerate(specs):
        pts=[]
        for t,v in breath_curve(dur,amp,skew):
            x=x0+w*(cursor+t)/total; y=358-225*v
            pts.append(f"{x:.1f},{y:.1f}")
        paths.append(f'<polyline points="{" ".join(pts)}" fill="none" stroke="{ORANGE if idx==1 else GREY}" stroke-width="5" stroke-linejoin="round"/>')
        cursor+=dur
        if idx<2:
            bx=x0+w*cursor/total
            boundaries.append(f'<line x1="{bx:.1f}" y1="116" x2="{bx:.1f}" y2="370" stroke="#e4e4e4" stroke-width="2" stroke-dasharray="7 7"/>')
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Breath cycle-to-cycle variation">
<title>Breath cycle-to-cycle variation</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Breath — one gesture, varied once per cycle</text>
<text x="34" y="87" class="subtitle">Every cycle returns to baseline; Texture varies duration, height and peak position only at the rollover</text>
<line x1="90" y1="358" x2="1020" y2="358" stroke="#dedede" stroke-width="2"/>
{''.join(boundaries)}{''.join(paths)}
<text x="90" y="414" class="small">baseline</text>
<text x="845" y="414" class="small">cycle identity remains: baseline → one peak → baseline</text>
</svg>'''


def fog_kernel(u: float) -> float:
    if u<=0 or u>=1: return 0.0
    return 16*u*u*(1-u)*(1-u)


def fog_asset() -> str:
    n=220
    clouds=[(0.05,0.42,.72),(0.28,0.38,-.58),(0.48,0.45,.88),(0.70,0.27,-.70)]
    indiv=[]; total=[]
    for i in range(n):
        t=i/(n-1)
        vals=[]
        for start,dur,amp in clouds:
            u=(t-start)/dur
            vals.append(amp*fog_kernel(u))
        indiv.append(vals); total.append(sum(vals))
    paths=[]
    for j in range(len(clouds)):
        vals=[v[j] for v in indiv]
        paths.append(f'<polyline points="{polyline(vals,110,125,905,190,-1.2,1.2)}" fill="none" stroke="{GREY}" stroke-width="2.5" opacity="0.55"/>')
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Fog overlapping bipolar cloudlets">
<title>Fog overlapping bipolar cloudlets</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Fog — overlapping soft bipolar cloudlets</text>
<text x="34" y="87" class="subtitle">Up to four finite-support voices overlap; Texture targets average occupancy rather than simply raising output level</text>
<line x1="110" y1="220" x2="1015" y2="220" stroke="#e0e0e0" stroke-width="2"/>
{''.join(paths)}
<polyline points="{polyline(total,110,125,905,190,-1.2,1.2)}" fill="none" stroke="{ORANGE}" stroke-width="5" stroke-linejoin="round"/>
<text x="110" y="365" class="small">grey: individual cloudlets</text>
<text x="398" y="365" class="small">orange: summed bipolar modulation before 0–10 V projection</text>
<text x="110" y="413" class="small">Each cloudlet uses g(u) = 16u²(1−u)² and is exactly zero outside its finite support.</text>
</svg>'''


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    assets={
        "config-current.svg": switch_asset("Current",False,False),
        "config-anchor.svg": switch_asset("Anchor",True,False),
        "config-breath.svg": switch_asset("Breath",False,True),
        "config-fog.svg": switch_asset("Fog",True,True),
        "current-long-form.svg": current_asset(),
        "anchor-mean-reversion.svg": anchor_asset(),
        "breath-cycle-variation.svg": breath_asset(),
        "fog-cloudlets.svg": fog_asset(),
    }
    for name,content in assets.items():
        (OUT/name).write_text(content,encoding="utf-8")
    print(f"generated {len(assets)} Ambient manual assets")

if __name__ == "__main__":
    main()
