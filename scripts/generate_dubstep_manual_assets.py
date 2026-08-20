#!/usr/bin/env python3
"""Generate deterministic SVG figures for Drift's Unreleased Dubstep/Bass bank.

The switch diagrams reuse the established Drift manual geometry. Algorithm
figures visualize implemented contracts rather than oscilloscope captures.

SPDX-License-Identifier: CC-BY-NC-4.0
"""
from __future__ import annotations

import math
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


def write(name: str, body: str, title: str, view: str = "0 0 1056 455") -> None:
    svg = (
        f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="{view}" role="img" aria-label="{title}">\n'
        f'<title>{title}</title>\n{body}\n</svg>\n'
    )
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
{sw(100, dip1, 1)}
{sw(245, dip2, 2)}
<text x="225" y="248" text-anchor="middle" font-size="31" font-weight="700" fill="{DARK}">{name}</text>'''
    write(f"config-{name.lower()}.svg", body, f"{name} configuration switches", view="0 0 450 288")


def wobble() -> None:
    phrase = [0, 1, 0, 2, 1, 3, 0, 2]
    rates = [
        ["1", "1", "1", "1"],
        ["1", "2", "1", "2"],
        ["1", "4/3", "2", "3/2"],
        ["1", "2", "3", "4"],
    ]
    yrows = [155, 220, 285, 350]
    body = [f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Wobble — one bar, four rate vocabularies</text>
<text x="34" y="87" class="subtitle">The eight eighth-note phrase cells stay deterministic; Texture selects how aggressively their carrier rates differ</text>''']
    x0, cw = 255, 91
    for cell, sym in enumerate(phrase):
        x = x0 + cell * cw
        body.append(f'<text x="{x+cw/2:.1f}" y="119" text-anchor="middle" class="small">{cell+1}</text>')
        body.append(f'<text x="{x+cw/2:.1f}" y="139" text-anchor="middle" class="small">s{sym}</text>')
    for region, y in enumerate(yrows):
        body.append(f'<text x="34" y="{y+7}" class="label">Texture {region}</text>')
        for cell, sym in enumerate(phrase):
            x = x0 + cell*cw
            label = rates[region][sym]
            fill = ORANGE if region == 3 else (ORANGE2 if region >= 1 else "white")
            stroke = ORANGE if region >= 1 else GREY
            body.append(f'<rect x="{x+5}" y="{y-24}" width="{cw-10}" height="42" rx="7" fill="{fill}" stroke="{stroke}" stroke-width="2"/>')
            body.append(f'<text x="{x+cw/2:.1f}" y="{y+4}" text-anchor="middle" font-family="Ubuntu,Arial,sans-serif" font-size="17" font-weight="600" fill="{"white" if region>=1 else DARK}">{label}×</text>')
    body.append(f'<text x="255" y="417" class="small">Carrier phase remains continuous when the rate changes; Texture is latched at the bar boundary.</text>')
    write("wobble-rate-phrase.svg", "\n".join(body), "Wobble deterministic rate phrase")


def triangle(x: float) -> float:
    x = x % 1.0
    return 1.0 - abs(2.0*x - 1.0)


def growl_value(phi: float, tau: float) -> float:
    a = 0.75*tau
    b = 0.5*tau*tau
    return (triangle(phi) + a*triangle(2*phi+0.25) + b*triangle(3*phi+0.125))/(1+a+b)


def polyline(values: list[float], x: float, y: float, w: float, h: float) -> str:
    pts=[]
    for i,v in enumerate(values):
        px=x+w*(i/(len(values)-1))
        py=y+h*(1-v)
        pts.append(f"{px:.1f},{py:.1f}")
    return " ".join(pts)


def growl() -> None:
    n=241
    low=[growl_value(i/(n-1),0.0) for i in range(n)]
    mid=[growl_value(i/(n-1),0.55) for i in range(n)]
    high=[growl_value(i/(n-1),1.0) for i in range(n)]
    body=f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Growl — one CV gesture, increasing internal structure</text>
<text x="34" y="87" class="subtitle">Texture adds normalized 2× and 3× phase-related triangle components; the output remains bounded</text>
<line x1="95" y1="345" x2="1015" y2="345" stroke="{LIGHT}" stroke-width="2"/>
<polyline points="{polyline(low,95,130,920,215)}" fill="none" stroke="{GREY}" stroke-width="4"/>
<polyline points="{polyline(mid,95,130,920,215)}" fill="none" stroke="{ORANGE2}" stroke-width="4"/>
<polyline points="{polyline(high,95,130,920,215)}" fill="none" stroke="{ORANGE}" stroke-width="5"/>
<text x="95" y="388" class="small">grey: Texture 0 — fundamental triangle</text>
<text x="390" y="388" class="small">light orange: mid Texture</text>
<text x="700" y="388" class="small">orange: max Texture</text>
<text x="95" y="418" class="small">This is modulation CV, not audio synthesis: use it to animate wavetable, filter, FM, fold or drive destinations.</text>'''
    write("growl-contour.svg", body, "Growl compound timbral motion contour")


def chop() -> None:
    anchors={0,8}
    candidates=[3,11,6,14,2,10,7,15]
    dense=anchors|set(candidates)
    def row(y: int, hits: set[int], label: str) -> str:
        out=[f'<text x="34" y="{y+6}" class="label">{label}</text>']
        for i in range(16):
            x=205+i*49
            active=i in hits
            is_anchor=i in anchors
            fill=ORANGE if is_anchor else (ORANGE2 if active else "white")
            stroke=ORANGE if active else GREY
            out.append(f'<rect x="{x-15}" y="{y-23}" width="30" height="46" rx="6" fill="{fill}" stroke="{stroke}" stroke-width="3"/>')
            if i%4==0:
                out.append(f'<line x1="{x-24}" y1="{y-34}" x2="{x-24}" y2="{y+34}" stroke="{LIGHT}" stroke-width="2"/>')
        return '\n'.join(out)
    body=f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Chop — deterministic syncopation, not probability</text>
<text x="34" y="87" class="subtitle">Steps 0 and 8 are permanent anchors; Texture adds candidates in a fixed musical order</text>
{row(180, anchors, "Texture 0")}
{row(290, dense, "Texture max")}
<text x="205" y="373" class="small">dark orange = structural anchor · light orange = Texture-added onset</text>
<path d="M 705 395 L 755 395 L 755 365 L 835 365 L 915 395" fill="none" stroke="{ORANGE}" stroke-width="4"/>
<text x="705" y="425" class="small">active-step contour: half hold, half linear decay</text>'''
    write("chop-phrase.svg", body, "Chop deterministic sixteen-step articulation phrase")


def smoothstep(u: float) -> float:
    return 3*u*u - 2*u*u*u


def build() -> None:
    n=360
    vals=[]
    micro_phase=0.0
    for i in range(n):
        u=i/(n-1)
        stage=min(3,int(u*4))
        rate=2**stage
        # Integrate approximately over normalized phrase for illustration only.
        micro_phase += rate*4/n
        q=triangle(micro_phase)
        vals.append(smoothstep(u)*(0.25+0.75*q))
    pts=polyline(vals,95,128,920,215)
    macro=polyline([smoothstep(i/(n-1)) for i in range(n)],95,128,920,215)
    body=[f'''{STYLE}<rect width="1056" height="455" fill="white"/>
<text x="34" y="52" class="title">Build — macro rise with four acceleration stages</text>
<text x="34" y="87" class="subtitle">Texture selects an 8/4/2/1-bar phrase; within it the micro triangle advances at quarter, eighth, sixteenth, then thirty-second rate</text>
<line x1="95" y1="343" x2="1015" y2="343" stroke="{LIGHT}" stroke-width="2"/>''']
    for frac,label in [(0.25,'1/4'),(0.5,'1/8'),(0.75,'1/16')]:
        x=95+920*frac
        body.append(f'<line x1="{x:.1f}" y1="115" x2="{x:.1f}" y2="352" stroke="{LIGHT}" stroke-width="2" stroke-dasharray="6 7"/>')
        body.append(f'<text x="{x-55:.1f}" y="383" class="small">{label}</text>')
    body.append('<text x="925" y="383" class="small">1/32</text>')
    body.append(f'<polyline points="{macro}" fill="none" stroke="{GREY}" stroke-width="3" stroke-dasharray="8 7"/>')
    body.append(f'<polyline points="{pts}" fill="none" stroke="{ORANGE}" stroke-width="5"/>')
    body.append('<text x="95" y="420" class="small">dashed grey = smoothstep macro envelope · orange = composite output · phrase end intentionally resets</text>')
    write("build-escalation.svg", "\n".join(body), "Build macro rise and micro-rate escalation")


def update_covers() -> None:
    front=OUT/'drift-front-cover.svg'
    s=front.read_text(encoding='utf-8')
    s=s.replace(
        'Classic · Organic · Generative · Ambient · Electronica · Percussion · 24 algorithms',
        'Classic · Organic · Generative · Ambient · Electronica · Percussion · Dubstep/Bass · 28 algorithms')
    front.write_text(s,encoding='utf-8')

    back=OUT/'drift-back-cover.svg'
    s=back.read_text(encoding='utf-8')
    # Reflow the seven-bank list as a compact block while retaining the original visual language.
    replacements={
        'y="1460" font-size="31" font-weight="300">Algorithm banks':'y="1435" font-size="30" font-weight="300">Algorithm banks',
        'y="1510" font-size="29" font-weight="600">Classic':'y="1480" font-size="27" font-weight="600">Classic',
        'y="1555" font-size="29" font-weight="600">Organic':'y="1520" font-size="27" font-weight="600">Organic',
        'y="1600" font-size="29" font-weight="600">Generative':'y="1560" font-size="27" font-weight="600">Generative',
        'y="1645" font-size="29" font-weight="600">Ambient':'y="1600" font-size="27" font-weight="600">Ambient',
        'y="1690" font-size="29" font-weight="600">Electronica':'y="1640" font-size="27" font-weight="600">Electronica',
        'y="1735" font-size="29" font-weight="600">Percussion':'y="1680" font-size="27" font-weight="600">Percussion',
        'y="1815" font-size="31" font-weight="300">Manual licence':'y="1800" font-size="30" font-weight="300">Manual licence',
        'y="1865" font-size="38" font-weight="600">CC BY-NC 4.0':'y="1845" font-size="36" font-weight="600">CC BY-NC 4.0',
        'y="1940" font-size="31" font-weight="300">Independent, unofficial manual.':'y="1915" font-size="29" font-weight="300">Independent, unofficial manual.',
        'y="1990" font-size="31" font-weight="300">Not affiliated with Free Modular.':'y="1960" font-size="29" font-weight="300">Not affiliated with Free Modular.',
    }
    for old,new in replacements.items():
        s=s.replace(old,new)
    if 'Dubstep/Bass  Wobble' not in s:
        line='    <text x="110" y="1680" font-size="27" font-weight="600">Percussion  Euclid · Repeat · Probability · Humanize</text>'
        s=s.replace(line, line+'\n    <text x="110" y="1720" font-size="27" font-weight="600">Dubstep/Bass  Wobble · Growl · Chop · Build</text>')
    back.write_text(s,encoding='utf-8')


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    config('Wobble', False, False)
    config('Growl', True, False)
    config('Chop', False, True)
    config('Build', True, True)
    wobble(); growl(); chop(); build(); update_covers()
    print('generated Dubstep/Bass manual assets')

if __name__ == '__main__':
    main()
