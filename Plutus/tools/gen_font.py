#!/usr/bin/env python3
"""
gen_font.py – Bake bitmap fonts into a C header for the Plutus project.

Fonts baked:
  SM   – Inter 11pt  → target cell 7×14
  MD   – Inter 15pt  → target cell 9×18
  LG   – Inter 20pt  → target cell 12×24
  MONO – SF Mono / Cascadia Code Regular 11pt → target cell 7×14

Output: /home/jan/CLionProjects/CPS/Plutus/inc/Font.h
"""

import math
import os
import sys
from PIL import Image, ImageFont, ImageDraw

# ---------------------------------------------------------------------------
# Font paths
# ---------------------------------------------------------------------------
INTER_PATH = "/usr/local/share/fonts/i/Inter_VariableFont_opsz,wght.ttf"

MONO_REGULAR_CANDIDATES = [
    "/usr/share/fonts/OTF/SF-Mono-Regular.otf",
    "/usr/share/fonts/OTF/CascadiaCode-Regular.otf",
]

# We only use ONE mono font (Regular weight) for FONT_MONO
def find_font(candidates):
    for p in candidates:
        if os.path.exists(p):
            return p
    raise FileNotFoundError(f"None of {candidates} found")

MONO_PATH = find_font(MONO_REGULAR_CANDIDATES)
print(f"# Using mono font: {MONO_PATH}", file=sys.stderr)

# ---------------------------------------------------------------------------
# ASCII printable range
# ---------------------------------------------------------------------------
FIRST_CHAR = 32
LAST_CHAR  = 126
NUM_CHARS  = LAST_CHAR - FIRST_CHAR + 1  # 95

# ---------------------------------------------------------------------------
# Font spec: (name, font_path, target_cw, target_ch)
# We iterate pt size until ch matches target_ch.
# ---------------------------------------------------------------------------
FONT_SPECS = [
    ("SM",   INTER_PATH, 7,  14),
    ("MD",   INTER_PATH, 9,  18),
    ("LG",   INTER_PATH, 12, 24),
    ("MONO", MONO_PATH,  7,  14),
]

def find_pt_for_height(font_path, target_h, start_pt=8, max_pt=80):
    """Binary-search for the PIL point size that produces glyphs of height == target_h."""
    # We measure height as the ascent+descent from getmetrics() or bbox of rendered chars
    best_pt = start_pt
    best_diff = 9999
    for pt in range(start_pt, max_pt + 1):
        try:
            font = ImageFont.truetype(font_path, size=pt)
        except Exception:
            continue
        # measure a tall character
        img = Image.new("L", (200, 200), 0)
        draw = ImageDraw.Draw(img)
        draw.text((10, 10), "Ag|", font=font, fill=255)
        bbox = img.getbbox()
        if bbox is None:
            continue
        h = bbox[3] - bbox[1]
        diff = abs(h - target_h)
        if diff < best_diff:
            best_diff = diff
            best_pt = pt
        if h >= target_h and diff <= 1:
            break
    return best_pt


def render_glyph(font, ch, cw, ch_height):
    """Render a single character into a 1-bit bitmap of size (cw, ch_height)."""
    img = Image.new("L", (cw * 4, ch_height * 4), 0)
    draw = ImageDraw.Draw(img)
    # Get the offset to center the glyph
    bbox = font.getbbox(ch)  # (left, top, right, bottom) relative to origin
    if bbox is None:
        return [[0] * cw for _ in range(ch_height)]
    glyph_w = bbox[2] - bbox[0]
    glyph_h = bbox[3] - bbox[1]
    # Offset so glyph is top-left aligned at (0,0) in image
    x_off = -bbox[0]
    y_off = -bbox[1]
    img2 = Image.new("L", (cw, ch_height), 0)
    draw2 = ImageDraw.Draw(img2)
    draw2.text((x_off, y_off), ch, font=font, fill=255)
    # threshold
    pixels = img2.load()
    bitmap = []
    for row in range(ch_height):
        rowdata = []
        for col in range(cw):
            rowdata.append(1 if pixels[col, row] >= 128 else 0)
        bitmap.append(rowdata)
    return bitmap


def pack_row(bits):
    """Pack a list of 0/1 bits into bytes, MSB first, padding to byte boundary."""
    n_bytes = math.ceil(len(bits) / 8)
    result = []
    for b in range(n_bytes):
        byte_val = 0
        for bit_idx in range(8):
            pixel_idx = b * 8 + bit_idx
            if pixel_idx < len(bits) and bits[pixel_idx]:
                byte_val |= (1 << (7 - bit_idx))
        result.append(byte_val)
    return result


def measure_actual_cell(font_path, pt):
    """Return (actual_cw, actual_ch) for a given font+pt by measuring rendered glyphs."""
    font = ImageFont.truetype(font_path, size=pt)
    max_w = 0
    max_h = 0
    for code in range(FIRST_CHAR, LAST_CHAR + 1):
        ch = chr(code)
        bbox = font.getbbox(ch)
        if bbox is None:
            continue
        w = bbox[2] - bbox[0]
        h = bbox[3] - bbox[1]
        if w > max_w:
            max_w = w
        if h > max_h:
            max_h = h
    return max_w, max_h


def align8(n):
    """Round up to next multiple of 8."""
    return ((n + 7) // 8) * 8


# ---------------------------------------------------------------------------
# Build font data
# ---------------------------------------------------------------------------
results = []  # list of dicts

for name, font_path, target_cw, target_ch in FONT_SPECS:
    print(f"# Processing FONT_{name} (target {target_cw}x{target_ch})...", file=sys.stderr)
    pt = find_pt_for_height(font_path, target_ch)
    print(f"#   Found pt={pt}", file=sys.stderr)

    font = ImageFont.truetype(font_path, size=pt)

    # Measure actual cell bounds across all printable chars
    max_w = 0
    max_h = 0
    for code in range(FIRST_CHAR, LAST_CHAR + 1):
        ch = chr(code)
        try:
            bbox = font.getbbox(ch)
        except Exception:
            bbox = None
        if bbox is None:
            continue
        gw = bbox[2] - bbox[0]
        gh = bbox[3] - bbox[1]
        if gw > max_w: max_w = gw
        if gh > max_h: max_h = gh

    # Use target dimensions as cell (clamp actual if bigger)
    cw = max(target_cw, max_w)
    ch_px = max(target_ch, max_h)
    # Round width up to 8-px alignment
    cw_aligned = align8(cw)
    # Clamp to ≤16 px (max 2 bytes per row)
    if cw_aligned > 16:
        cw_aligned = 16
    # Use the target cell height exactly (clip/pad)
    ch_final = target_ch

    bytes_per_row = cw_aligned // 8

    print(f"#   Cell: {cw_aligned}x{ch_final}, bytesPerRow={bytes_per_row}", file=sys.stderr)

    # Render all glyphs
    glyphs = []
    for code in range(FIRST_CHAR, LAST_CHAR + 1):
        ch_char = chr(code)
        bitmap = render_glyph(font, ch_char, cw_aligned, ch_final)
        packed_rows = [pack_row(row) for row in bitmap]
        glyphs.append(packed_rows)

    results.append({
        "name": name,
        "cw": cw_aligned,
        "ch": ch_final,
        "bytes_per_row": bytes_per_row,
        "pt": pt,
        "font_path": font_path,
        "glyphs": glyphs,
    })


# ---------------------------------------------------------------------------
# Emit the C header
# ---------------------------------------------------------------------------

def emit_header(results):
    lines = []
    a = lines.append

    a("/* ============================================================")
    a(" * Font.h – Auto-generated bitmap font header for Plutus")
    a(" * Generated by tools/gen_font.py")
    a(" *")
    for r in results:
        a(f" * FONT_{r['name']}: {r['font_path']} pt={r['pt']} cell={r['cw']}x{r['ch']} bytesPerRow={r['bytes_per_row']}")
    a(" * ============================================================")
    a(" * ASCII 32-126 (95 glyphs), 1bpp, MSB-first packed rows")
    a(" * ============================================================ */")
    a("")
    a("#ifndef PLUTUS_FONT_H")
    a("#define PLUTUS_FONT_H")
    a("")
    a("#include <SDL.h>")
    a("#include <stdint.h>")
    a("#include <string.h>")
    a("")
    a("/* ---- Font size enum ---- */")
    a("typedef enum {")
    a("    FONT_SM   = 0,")
    a("    FONT_MD   = 1,")
    a("    FONT_LG   = 2,")
    a("    FONT_MONO = 3")
    a("} FontSize;")
    a("")
    a("/* ---- Per-size metrics ---- */")
    a("typedef struct tagFontMetrics {")
    a("    int cw;          /* cell width  (pixels) */")
    a("    int ch;          /* cell height (pixels) */")
    a("    int bytesPerRow; /* ceil(cw/8)           */")
    a("} FontMetrics;")
    a("")
    a("static const FontMetrics g_fontMetrics[4] = {")
    for r in results:
        a(f"    {{ {r['cw']:2d}, {r['ch']:2d}, {r['bytes_per_row']} }},  /* FONT_{r['name']} */")
    a("};")
    a("")

    # Emit glyph arrays
    for r in results:
        name       = r["name"]
        cw         = r["cw"]
        ch         = r["ch"]
        bpr        = r["bytes_per_row"]
        glyphs     = r["glyphs"]
        array_name = f"g_fontGlyphs_{name}"
        a(f"/* ---- FONT_{name}: {cw}x{ch}, {bpr} byte(s)/row ---- */")
        a(f"static const uint8_t {array_name}[{NUM_CHARS}][{ch}][{bpr}] = {{")
        for idx, packed_rows in enumerate(glyphs):
            ch_char = chr(FIRST_CHAR + idx)
            display = ch_char if ch_char.isprintable() and ch_char not in ('\\', '/') else f"\\x{FIRST_CHAR+idx:02x}"
            a(f"    /* [{idx:2d}] '{display}' */ {{")
            for row_bytes in packed_rows:
                hex_vals = ", ".join(f"0x{b:02X}" for b in row_bytes)
                a(f"        {{ {hex_vals} }},")
            a("    },")
        a("};")
        a("")

    # Inline functions
    a("/* ================================================================")
    a(" * Inline rendering helpers")
    a(" * ================================================================ */")
    a("")
    a("/* Returns the pixel width of a string rendered at the given size. */")
    a("static inline int FontTextWidth(const char *s, FontSize size)")
    a("{")
    a("    return (int)strlen(s) * g_fontMetrics[(int)size].cw;")
    a("}")
    a("")
    a("/* Returns the cell height for a given size. */")
    a("static inline int FontCellHeight(FontSize size)")
    a("{")
    a("    return g_fontMetrics[(int)size].ch;")
    a("}")
    a("")
    a("/* Draw a single character; returns the advance width in pixels. */")
    a("static inline int FontDrawChar(SDL_Renderer *ren, int x, int y, char c,")
    a("                               SDL_Color col, FontSize size)")
    a("{")
    a("    int idx = (unsigned char)c - 32;")
    a("    const FontMetrics *m = &g_fontMetrics[(int)size];")
    a("    const uint8_t (*glyphs)[m->ch][m->bytesPerRow];  /* VLA-ptr trick */")
    a("    if (idx < 0 || idx >= 95) idx = 0;  /* clamp to printable range */")
    a("")
    a("    switch (size) {")

    for r in results:
        a(f"        case FONT_{r['name']}: glyphs = g_fontGlyphs_{r['name']}; break;")

    a("        default: glyphs = g_fontGlyphs_SM; break;")
    a("    }")
    a("")
    a("    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);")
    a("    for (int row = 0; row < m->ch; ++row) {")
    a("        for (int byte_i = 0; byte_i < m->bytesPerRow; ++byte_i) {")
    a("            uint8_t byte_val = glyphs[idx][row][byte_i];")
    a("            for (int bit = 7; bit >= 0; --bit) {")
    a("                int col_px = byte_i * 8 + (7 - bit);")
    a("                if (col_px >= m->cw) break;")
    a("                if (byte_val & (uint8_t)(1u << (unsigned)bit)) {")
    a("                    SDL_RenderDrawPoint(ren, x + col_px, y + row);")
    a("                }")
    a("            }")
    a("        }")
    a("    }")
    a("    return m->cw;")
    a("}")
    a("")
    a("/* Draw a null-terminated string; returns the final x position. */")
    a("static inline int FontDrawText(SDL_Renderer *ren, int x, int y, const char *s,")
    a("                               SDL_Color col, FontSize size)")
    a("{")
    a("    while (*s) {")
    a("        x += FontDrawChar(ren, x, y, *s++, col, size);")
    a("    }")
    a("    return x;")
    a("}")
    a("")
    a("#endif /* PLUTUS_FONT_H */")
    a("")

    return "\n".join(lines)


header_text = emit_header(results)

OUT_PATH = "/home/jan/CLionProjects/CPS/Plutus/inc/Font.h"
os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)
with open(OUT_PATH, "w") as f:
    f.write(header_text)

print(f"Wrote {OUT_PATH}  ({len(header_text)} bytes)", file=sys.stderr)

# Print summary to stdout
for r in results:
    print(f"FONT_{r['name']}: cw={r['cw']} ch={r['ch']} bytesPerRow={r['bytes_per_row']} pt={r['pt']}")
