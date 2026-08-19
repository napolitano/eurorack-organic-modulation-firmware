#!/usr/bin/env python3
"""Generate deterministic SVG figures for Drift's Percussion bank.

The switch diagrams reuse the established Drift manual switch geometry. The
algorithm figures visualise the implemented rhythm/clock contracts and are not
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
STYLE = '''<style>
.title{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:29px;font-weight:700;fill:#1a1a1a}
.subtitle{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:18px;font-weight:300;fill:#6e6e6e}
.label{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:18px;font-weight:500;fill:#1a1a1a}
.small{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif;font-size:15px;font-weight:300;fill:#6e6e6e}
</style>'''

def write(name: str, body: str, title: str, aria: str | None = None, view: str = "0 0 1056 455") -> None:
    aria = aria or title
    svg = f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="{view}" role="img" aria-label="{aria}">\n<title>{title}</title>\n{body}\n</svg>\n'
    (OUT / name).write_text(svg, encoding="utf-8")

def config(name: str, dip1: bool, dip2: bool) -> None:
    def sw(x: int, on: bool, number: int) -> str:
        y = 61 if on else 103
        fill = ORANGE if on else GREY
        return f'''<rect x="{x}" y="49" width="78" height="93" rx="9" fill="white" stroke="{DARK}" stroke-width="5"/>
<rect x="{x+12}" y="{y}" width="54" height="30" rx="5" fill="{fill}"/>
<text x="{x+39}" y="190" text-anchor="middle" font-size="25" font-weight="300" fill="{GREY}">{number}</text>'''
    body = f'''<style>text{{font-family:Ubuntu,"Ubuntu Sans",Arial,sans-serif}}</style>
<rect width="450" height="288" fill="white"/>
<rect x="80" y="30" width="290" height="135" rx="12" fill="#f1f1f1" stroke="{DARK}" stroke-width="6"/>
{sw(100,dip1,1)}
{sw(245,dip2,2)}
<text x="225" y="248" text-anchor="middle" font-size="31" font-weight="700" fill="{DARK}">{name}</text>'''
    write(f"config-{name.lower()}.svg", body, f"{name} configuration switches", view="0 0 450 288")

def euclid() -> None:
    # Canonical E(5,16)-style example plus a phrase-end tail fill.
    base = {0,3,6,9,12}
    fill = base | {13,14,15}
    def row(y: int, hits: set[int], label: str) -> str:
        out=[f'<text x="34" y="{y+6}" class="label">{label}</text>']
        for i in range(16):
            x=190+i*50
            active=i in hits
            out.append(f'<circle cx="{x}" cy="{y}" r="15" fill="{ORANGE if active else "white"}" stroke="{ORANGE if active else GREY}" stroke-width="3"/>')
            if i%4==0:
                out.append(f'<line x1="{x-25}" y1="{y-28}" x2="{x-25}" y2="{y+28}" stroke="{LIGHT}" stroke-width="2"/>')
        return '\n'.join(out)
    body=f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Euclid — stable pattern, phrase-end fill</text>
<text x="34" y="87" class="subtitle">Texture selects E(k,16) density and the shared 4/8/12/16-bar phrase activity</text>
{row(190,base,"normal bar")}
{row(305,fill,"fill bar")}
<text x="190" y="385" class="small">16-step bar; fill only adds tail hits and never removes the Euclidean core</text>'''
    write("euclid-pattern.svg", body, "Euclid pattern and phrase fill")

def repeat() -> None:
    body=f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Repeat — one anchor, optional ratchets</text>
<text x="34" y="87" class="subtitle">Repeats stay inside the first half of a quarter note; phrase fills force stronger tail rolls</text>
<line x1="130" y1="340" x2="965" y2="340" stroke="{LIGHT}" stroke-width="3"/>
<text x="38" y="155" class="label">single</text><line x1="210" y1="125" x2="210" y2="185" stroke="{ORANGE}" stroke-width="8"/>
<text x="38" y="245" class="label">double</text><line x1="210" y1="215" x2="210" y2="275" stroke="{ORANGE}" stroke-width="8"/><line x1="385" y1="215" x2="385" y2="275" stroke="{ORANGE2}" stroke-width="7"/>
<text x="38" y="335" class="label">quad</text><line x1="210" y1="305" x2="210" y2="365" stroke="{ORANGE}" stroke-width="8"/><line x1="300" y1="305" x2="300" y2="365" stroke="{ORANGE2}" stroke-width="7"/><line x1="390" y1="305" x2="390" y2="365" stroke="{ORANGE2}" stroke-width="7"/><line x1="480" y1="305" x2="480" y2="365" stroke="{ORANGE2}" stroke-width="7"/>
<line x1="210" y1="118" x2="210" y2="382" stroke="{GREY}" stroke-width="2" stroke-dasharray="8 8"/>
<line x1="560" y1="118" x2="560" y2="382" stroke="{GREY}" stroke-width="2" stroke-dasharray="8 8"/>
<text x="210" y="410" class="small" text-anchor="middle">quarter start</text><text x="560" y="410" class="small" text-anchor="middle">half-beat limit for ratchets</text>'''
    write("repeat-ratchets.svg", body, "Repeat ratchet placement")

def probability() -> None:
    body=f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Probability — metric skeleton plus optional detail</text>
<text x="34" y="87" class="subtitle">Primary quarter notes are guaranteed; secondary and ghost positions become more likely with Texture</text>'''
    parts=[body]
    for i in range(16):
        x=145+i*54
        if i%4==0: kind,fill,stroke="P",ORANGE,ORANGE
        elif i%2==0: kind,fill,stroke="S",ORANGE2,ORANGE2
        else: kind,fill,stroke="G","white",GREY
        parts.append(f'<rect x="{x-18}" y="160" width="36" height="115" rx="8" fill="{fill}" stroke="{stroke}" stroke-width="3"/>')
        parts.append(f'<text x="{x}" y="305" text-anchor="middle" class="small">{i}</text>')
        parts.append(f'<text x="{x}" y="228" text-anchor="middle" font-family="Ubuntu,Arial,sans-serif" font-size="17" font-weight="700" fill="{DARK if fill=="white" else "white"}">{kind}</text>')
    parts.append(f'<text x="145" y="370" class="small">P: always · S: probability ∝ Texture · G: probability ∝ Texture² / 2</text>')
    parts.append(f'<text x="145" y="400" class="small">Fill bars boost optional probabilities, especially in the final quarter; primary hits stay fixed</text>')
    write("probability-grid.svg", '\n'.join(parts), "Probability metric classes")

def humanize() -> None:
    body=f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Humanize — bounded deviation around a fixed grid</text>
<text x="34" y="87" class="subtitle">Eight nominal eighth notes remain; Texture adds up to ±12 ms timing shift and bounded pulse-level variation</text>
<line x1="115" y1="315" x2="1000" y2="315" stroke="{LIGHT}" stroke-width="3"/>'''
    parts=[body]
    nominal=[145+i*112 for i in range(8)]
    offsets=[-10,12,-4,8,-13,5,0,11]
    heights=[95,125,105,140,112,132,100,120]
    for i,x in enumerate(nominal):
        parts.append(f'<line x1="{x}" y1="130" x2="{x}" y2="330" stroke="{GREY}" stroke-width="2" stroke-dasharray="6 7"/>')
        ax=x+offsets[i]
        parts.append(f'<line x1="{ax}" y1="{315-heights[i]}" x2="{ax}" y2="315" stroke="{ORANGE}" stroke-width="8"/>')
    parts.append('<text x="145" y="375" class="small">grey: nominal grid · orange: emitted event</text>')
    parts.append('<text x="145" y="407" class="small">Every offset is measured from its own nominal time, so timing error never accumulates into tempo drift</text>')
    write("humanize-timing.svg", '\n'.join(parts), "Humanize timing and amplitude variation")

def update_covers() -> None:
    front=OUT/'drift-front-cover.svg'
    s=front.read_text(encoding='utf-8')
    s=s.replace('Classic · Organic · Generative · Ambient · Electronica · 20 algorithms',
                'Classic · Organic · Generative · Ambient · Electronica · Percussion · 24 algorithms')
    front.write_text(s,encoding='utf-8')

    back=OUT/'drift-back-cover.svg'
    s=back.read_text(encoding='utf-8')
    # Compress the bank list slightly so six rows and the licence block fit without clipping.
    s=s.replace('y="1490" font-size="31" font-weight="300">Algorithm banks', 'y="1460" font-size="31" font-weight="300">Algorithm banks')
    replacements={
      'y="1540" font-size="31" font-weight="600">Classic':'y="1510" font-size="29" font-weight="600">Classic',
      'y="1590" font-size="31" font-weight="600">Organic':'y="1555" font-size="29" font-weight="600">Organic',
      'y="1640" font-size="31" font-weight="600">Generative':'y="1600" font-size="29" font-weight="600">Generative',
      'y="1690" font-size="31" font-weight="600">Ambient':'y="1645" font-size="29" font-weight="600">Ambient',
      'y="1740" font-size="31" font-weight="600">Electronica':'y="1690" font-size="29" font-weight="600">Electronica',
    }
    for a,b in replacements.items(): s=s.replace(a,b)
    electronica_line='    <text x="110" y="1690" font-size="29" font-weight="600">Electronica  Pump · Acid · Shuffle · Polymeter</text>'
    if 'Percussion  Euclid' not in s:
        s=s.replace(electronica_line, electronica_line+'\n    <text x="110" y="1735" font-size="29" font-weight="600">Percussion  Euclid · Repeat · Probability · Humanize</text>')
    s=s.replace('y="1825" font-size="31" font-weight="300">Manual licence','y="1815" font-size="31" font-weight="300">Manual licence')
    s=s.replace('y="1875" font-size="38" font-weight="600">CC BY-NC 4.0','y="1865" font-size="38" font-weight="600">CC BY-NC 4.0')
    s=s.replace('y="1950" font-size="31" font-weight="300">Independent, unofficial manual.','y="1940" font-size="31" font-weight="300">Independent, unofficial manual.')
    s=s.replace('y="2000" font-size="31" font-weight="300">Not affiliated with Free Modular.','y="1990" font-size="31" font-weight="300">Not affiliated with Free Modular.')
    back.write_text(s,encoding='utf-8')

def main() -> None:
    OUT.mkdir(parents=True,exist_ok=True)
    config('Euclid',False,False)
    config('Repeat',True,False)
    config('Probability',False,True)
    config('Humanize',True,True)
    euclid(); repeat(); probability(); humanize(); update_covers()
    print('generated Percussion manual assets')
if __name__=='__main__': main()
