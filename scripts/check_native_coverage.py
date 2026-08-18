#!/usr/bin/env python3
import json,sys,xml.etree.ElementTree as ET
from pathlib import Path
policy=json.loads(Path('scripts/native_coverage_policy.json').read_text())
root=ET.parse(sys.argv[1]).getroot();line=float(root.attrib['line-rate'])*100.0;branch=float(root.attrib['branch-rate'])*100.0
print(f'coverage: lines={line:.2f}% branches={branch:.2f}%')
if line<policy['line_min_percent'] or branch<policy['branch_min_percent']:raise SystemExit(1)
