import csv
import math
from pathlib import Path

with Path("benchmark.csv").open(newline="", encoding="utf-8") as source:
    rows = list(csv.DictReader(source))
if not rows:
    raise SystemExit("benchmark.csv is empty; run `make benchmark` first")

width, height = 900, 560
left, right, top, bottom = 100, 35, 55, 85
plot_w, plot_h = width-left-right, height-top-bottom
xs = [int(row["elements"]) for row in rows]
ys = [float(row[key]) for row in rows for key in ("insert_ms", "find_ms", "erase_ms")]
x0, x1 = math.log10(min(xs)), math.log10(max(xs))
y0, y1 = math.log10(min(ys)), math.log10(max(ys))
if x0 == x1: x1 += 1
if y0 == y1: y1 += 1
def px(x): return left + (math.log10(x)-x0)/(x1-x0)*plot_w
def py(y): return top + plot_h - (math.log10(y)-y0)/(y1-y0)*plot_h

colors = {"insert_ms": "#2563eb", "find_ms": "#16a34a", "erase_ms": "#dc2626"}
labels = {"insert_ms": "Вставка", "find_ms": "Поиск", "erase_ms": "Удаление"}
svg = [f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}" viewBox="0 0 {width} {height}">',
       '<rect width="100%" height="100%" fill="white"/>',
       '<text x="450" y="30" text-anchor="middle" font-family="sans-serif" font-size="20">AVL-дерево: время базовых операций</text>']
for i in range(6):
    x = left + plot_w*i/5
    svg.append(f'<line x1="{x:.1f}" y1="{top}" x2="{x:.1f}" y2="{top+plot_h}" stroke="#ddd"/>')
    value = 10**(x0+(x1-x0)*i/5)
    svg.append(f'<text x="{x:.1f}" y="{top+plot_h+23}" text-anchor="middle" font-family="sans-serif" font-size="12">{value:.0f}</text>')
    y = top + plot_h - plot_h*i/5
    svg.append(f'<line x1="{left}" y1="{y:.1f}" x2="{left+plot_w}" y2="{y:.1f}" stroke="#ddd"/>')
    value = 10**(y0+(y1-y0)*i/5)
    svg.append(f'<text x="{left-10}" y="{y+4:.1f}" text-anchor="end" font-family="sans-serif" font-size="12">{value:.3g}</text>')
for key, color in colors.items():
    points = " ".join(f'{px(int(row["elements"])):.1f},{py(float(row[key])):.1f}' for row in rows)
    svg.append(f'<polyline points="{points}" fill="none" stroke="{color}" stroke-width="2.5"/>')
    for row in rows:
        svg.append(f'<circle cx="{px(int(row["elements"])):.1f}" cy="{py(float(row[key])):.1f}" r="4" fill="{color}"/>')
svg.extend([f'<text x="{left+plot_w/2:.1f}" y="{height-28}" text-anchor="middle" font-family="sans-serif" font-size="14">Количество элементов (логарифмическая шкала)</text>',
            f'<text x="22" y="{top+plot_h/2:.1f}" text-anchor="middle" transform="rotate(-90 22 {top+plot_h/2:.1f})" font-family="sans-serif" font-size="14">Время, мс (логарифмическая шкала)</text>'])
for i, (key, color) in enumerate(colors.items()):
    x = left + i*130
    svg.extend([f'<line x1="{x}" y1="{height-55}" x2="{x+25}" y2="{height-55}" stroke="{color}" stroke-width="3"/>',
                f'<text x="{x+32}" y="{height-51}" font-family="sans-serif" font-size="13">{labels[key]}</text>'])
svg.append('</svg>')
Path("benchmark.svg").write_text("\n".join(svg), encoding="utf-8")
print("Saved benchmark.svg")
