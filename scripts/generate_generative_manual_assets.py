#!/usr/bin/env python3
"""Generate deterministic SVG figures for Drift's Generative bank.

The figures visualise the implemented Generative control laws and reuse the
same switch geometry and manual palette as the existing Classic/Organic
assets.

SPDX-License-Identifier: CC-BY-NC-4.0
"""
from __future__ import annotations

from pathlib import Path
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


def slot_icon(label: str, switch1_on: bool, switch2_on: bool) -> str:
    def slider(x: int, is_on: bool) -> str:
        y = 35 if is_on else 62
        color = ORANGE if is_on else GREY
        return f'<rect x="{x+8}" y="{y}" width="34" height="18" rx="3" fill="{color}"/>'
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 220 120" role="img" aria-label="DIP slot {label}">
<title>DIP slot {label}</title>
<rect width="220" height="120" fill="white"/>
<rect x="20" y="15" width="180" height="78" rx="8" fill="#f1f1f1" stroke="#1a1a1a" stroke-width="4"/>
<rect x="44" y="28" width="50" height="60" rx="5" fill="white" stroke="#1a1a1a" stroke-width="3"/>
{slider(44, switch1_on)}
<rect x="126" y="28" width="50" height="60" rx="5" fill="white" stroke="#1a1a1a" stroke-width="3"/>
{slider(126, switch2_on)}
<text x="69" y="112" text-anchor="middle" font-family="Ubuntu,Arial,sans-serif" font-size="13" fill="#6e6e6e">1</text>
<text x="151" y="112" text-anchor="middle" font-family="Ubuntu,Arial,sans-serif" font-size="13" fill="#6e6e6e">2</text>
</svg>'''


def polyline(values: list[float], x: float, y: float, w: float, h: float, vmin: float, vmax: float) -> str:
    if not values:
        return ""
    span = max(1e-12, vmax-vmin)
    pts=[]
    for i,v in enumerate(values):
        px=x+w*(i/(len(values)-1 if len(values)>1 else 1))
        py=y+h*(1-(v-vmin)/span)
        pts.append(f"{px:.1f},{py:.1f}")
    return " ".join(pts)


def turing_asset() -> str:
    seed = 0xA5C3
    def advance(reg: int, mutate: bool=False) -> int:
        bit=(reg & 1) ^ (1 if mutate else 0)
        return ((reg>>1) | (bit<<15)) & 0xFFFF
    locked=[]; reg=seed
    for _ in range(33):
        locked.append((reg>>4)/4095)
        reg=advance(reg)
    changed=[]; reg=seed
    mutation_step=11
    for i in range(33):
        changed.append((reg>>4)/4095)
        reg=advance(reg, i==mutation_step)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Turing locked loop and mutation">
<title>Turing locked loop and mutation</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Turing — repetition that can slowly rewrite itself</text>
<text x="34" y="87" class="subtitle">Texture 0 locks the 16-bit register; mutation changes the future loop rather than drawing a new sequence</text>
<text x="42" y="174" class="label">Locked</text><line x1="190" y1="196" x2="1015" y2="196" stroke="#e6e6e6"/>
<polyline points="{polyline(locked,190,120,825,110,0,1)}" fill="none" stroke="{GREY}" stroke-width="5" stroke-linejoin="round"/>
<text x="42" y="331" class="label">One mutation</text><line x1="190" y1="352" x2="1015" y2="352" stroke="#e6e6e6"/>
<polyline points="{polyline(changed,190,276,825,110,0,1)}" fill="none" stroke="{ORANGE}" stroke-width="5" stroke-linejoin="round"/>
<line x1="{190+825*mutation_step/32:.1f}" y1="270" x2="{190+825*mutation_step/32:.1f}" y2="395" stroke="{ORANGE2}" stroke-width="2" stroke-dasharray="7 7"/>
<text x="{190+825*mutation_step/32+8:.1f}" y="410" class="small">feedback bit mutated</text>
</svg>'''


def markov_asset() -> str:
    # Deterministic example using the implemented structured probabilities.
    rng=random.Random(0x4D4B)
    state=0; states=[]
    for _ in range(38):
        states.append(state)
        r=rng.randrange(8)
        if r < 4: nxt=state
        elif r < 6: nxt=(state+1)&7
        elif r == 6: nxt=(state+7)&7
        else: nxt=(state+4)&7
        state=nxt
    xs=[]
    for i,s in enumerate(states):
        x=165+i*(850/(len(states)-1)); y=375-s*34
        xs.append(f"{x:.1f},{y:.1f}")
    grid=''.join(f'<line x1="155" y1="{375-s*34}" x2="1015" y2="{375-s*34}" stroke="#eeeeee"/><text x="120" y="{380-s*34}" text-anchor="end" class="small">S{s}</text>' for s in range(8))
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Markov recurring state vocabulary">
<title>Markov recurring state vocabulary</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Markov — familiar states, changing routes</text>
<text x="34" y="87" class="subtitle">The next value depends on the current state; Texture moves from a structured grammar toward free exploration</text>
{grid}
<polyline points="{' '.join(xs)}" fill="none" stroke="{ORANGE}" stroke-width="5" stroke-linecap="round" stroke-linejoin="round"/>
<text x="155" y="430" class="small">Example structured-state path; physical voltages are a fixed shuffled eight-level vocabulary</text>
</svg>'''


def motif_asset() -> str:
    before=[0.18,0.76,0.34,0.92,0.54,0.24,0.68,0.43]
    after=before.copy(); after[2],after[4]=after[4],after[2]
    def steps(vals,y,color):
        parts=[]
        for i,v in enumerate(vals):
            x=205+i*100; yy=y+80*(1-v)
            parts.append(f'<circle cx="{x}" cy="{yy:.1f}" r="8" fill="{color}"/><text x="{x}" y="{y+105}" text-anchor="middle" class="small">{i+1}</text>')
            if i:
                px=205+(i-1)*100; pyy=y+80*(1-vals[i-1])
                parts.append(f'<line x1="{px}" y1="{pyy:.1f}" x2="{x}" y2="{yy:.1f}" stroke="{color}" stroke-width="4"/>')
        return ''.join(parts)
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Motif phrase mutation">
<title>Motif phrase mutation</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Motif — preserve identity, alter structure</text>
<text x="34" y="87" class="subtitle">One edit is allowed only at an eight-step phrase boundary; most edits keep the complete voltage vocabulary</text>
<text x="42" y="178" class="label">Phrase A</text>{steps(before,120,GREY)}
<text x="42" y="345" class="label">Next cycle</text>{steps(after,287,ORANGE)}
<path d="M405 254 C455 225 535 225 585 254" fill="none" stroke="{ORANGE2}" stroke-width="3" stroke-dasharray="7 7"/>
<text x="495" y="235" text-anchor="middle" class="small">example: reverse three</text>
</svg>'''


def urn_asset() -> str:
    baseline=[32]*8
    fav=[32,32,32,32,160,32,32,32]
    relaxed=[32,32,32,32,86,32,32,32]
    groups=[(baseline,"Equal",GREY),(fav,"Reinforced",ORANGE),(relaxed,"Later",ORANGE2)]
    bars=[]
    for gi,(vals,label,color) in enumerate(groups):
        gx=120+gi*305
        bars.append(f'<text x="{gx+105}" y="136" text-anchor="middle" class="label">{label}</text>')
        for i,v in enumerate(vals):
            h=160*v/160; x=gx+i*26; y=350-h
            bars.append(f'<rect x="{x}" y="{y:.1f}" width="18" height="{h:.1f}" rx="3" fill="{color}"/><text x="{x+9}" y="372" text-anchor="middle" class="small">{i}</text>')
    return f'''<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 1056 455" role="img" aria-label="Urn reinforcement and relaxation">
<title>Urn reinforcement and relaxation</title>{STYLE}
<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Urn — temporary preferences emerge and fade</text>
<text x="34" y="87" class="subtitle">A selected state gains Texture-controlled weight; every draw also relaxes all weights back toward baseline</text>
<line x1="100" y1="350" x2="960" y2="350" stroke="#d8d8d8" stroke-width="2"/>
{''.join(bars)}
<text x="120" y="417" class="small">State weights, not output amplitudes. The output vocabulary itself remains fixed at eight levels.</text>
</svg>'''


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    assets={
        "config-turing.svg": switch_asset("Turing",False,False),
        "config-markov.svg": switch_asset("Markov",True,False),
        "config-motif.svg": switch_asset("Motif",False,True),
        "config-urn.svg": switch_asset("Urn",True,True),
        "dip-slot-00.svg": slot_icon("OFF / OFF",False,False),
        "dip-slot-10.svg": slot_icon("ON / OFF",True,False),
        "dip-slot-01.svg": slot_icon("OFF / ON",False,True),
        "dip-slot-11.svg": slot_icon("ON / ON",True,True),
        "turing-mutation.svg": turing_asset(),
        "markov-vocabulary.svg": markov_asset(),
        "motif-evolution.svg": motif_asset(),
        "urn-reinforcement.svg": urn_asset(),
    }
    for name,content in assets.items():
        (OUT/name).write_text(content+"\n",encoding="utf-8")
        print(OUT/name)
    return 0

if __name__ == '__main__':
    raise SystemExit(main())
