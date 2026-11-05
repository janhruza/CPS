//
// Created on 10/31/26.
// SDLWidgets.c - Implementation of reusable SDL2 chart primitives.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL.h>

#include "../inc/Font.h"
#include "../inc/SDLWidgets.h"

/* ============================================================
 *  Color palette (canonical 8-color cycle for charts)
 * ============================================================ */

const SDL_Color g_wPalette[8] = {
    {59,  130, 246, 255},
    {52,  211, 153, 255},
    {251, 146, 60,  255},
    {167, 139, 250, 255},
    {34,  211, 238, 255},
    {251, 191, 36,  255},
    {248, 113, 113, 255},
    {163, 230, 53,  255},
};

/* ============================================================
 *  Window lifecycle
 * ============================================================ */

bool WOpen(const char *pszTitle, int iW, int iH,
           SDL_Window **ppWin, SDL_Renderer **ppRen)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return false;
    }
    *ppWin = SDL_CreateWindow(pszTitle,
                              SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                              iW, iH,
                              SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (!*ppWin) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }
    *ppRen = SDL_CreateRenderer(*ppWin, -1,
                                SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*ppRen)
        *ppRen = SDL_CreateRenderer(*ppWin, -1, 0);   /* SW fallback */
    if (!*ppRen) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(*ppWin);
        *ppWin = NULL;
        SDL_Quit();
        return false;
    }
    SDL_SetRenderDrawBlendMode(*ppRen, SDL_BLENDMODE_BLEND);
    return true;
}

void WClose(SDL_Renderer **ppRen, SDL_Window **ppWin)
{
    if (ppRen && *ppRen) { SDL_DestroyRenderer(*ppRen); *ppRen = NULL; }
    if (ppWin && *ppWin) { SDL_DestroyWindow(*ppWin);   *ppWin = NULL; }
    SDL_Quit();
}

/* ============================================================
 *  Event loop
 * ============================================================ */

bool WPollEvents(int *pMouseX, int *pMouseY)
{
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) return false;
        if (e.type == SDL_KEYDOWN &&
            (e.key.keysym.sym == SDLK_ESCAPE || e.key.keysym.sym == SDLK_q))
            return false;
        if (e.type == SDL_MOUSEMOTION) {
            if (pMouseX) *pMouseX = e.motion.x;
            if (pMouseY) *pMouseY = e.motion.y;
        }
    }
    return true;
}

/* ============================================================
 *  Primitive helpers
 * ============================================================ */

void WSetColor(SDL_Renderer *ren, SDL_Color col)
{
    SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
}

void WFillRect(SDL_Renderer *ren, SDL_Rect rect, SDL_Color col)
{
    WSetColor(ren, col);
    SDL_RenderFillRect(ren, &rect);
}

void WDrawRect(SDL_Renderer *ren, SDL_Rect rect, SDL_Color col)
{
    WSetColor(ren, col);
    SDL_RenderDrawRect(ren, &rect);
}

void WDrawLine(SDL_Renderer *ren, int x0, int y0, int x1, int y1, SDL_Color col)
{
    WSetColor(ren, col);
    SDL_RenderDrawLine(ren, x0, y0, x1, y1);
}

void WDrawDashedLine(SDL_Renderer *ren,
                     int x0, int y0, int x1, int y1,
                     SDL_Color col, int iDash, int iGap)
{
    if (iDash <= 0) iDash = 6;
    if (iGap  <= 0) iGap  = 4;
    double dx = x1 - x0;
    double dy = y1 - y0;
    double len = sqrt(dx * dx + dy * dy);
    if (len < 1.0) return;
    dx /= len; dy /= len;
    double t = 0.0;
    bool bDraw = true;
    WSetColor(ren, col);
    while (t < len) {
        double seg = bDraw ? iDash : iGap;
        double t2  = t + seg;
        if (t2 > len) t2 = len;
        if (bDraw) {
            SDL_RenderDrawLine(ren,
                x0 + (int)(dx * t),  y0 + (int)(dy * t),
                x0 + (int)(dx * t2), y0 + (int)(dy * t2));
        }
        t = t2;
        bDraw = !bDraw;
    }
}

void WFillCircle(SDL_Renderer *ren, int cx, int cy, int r, SDL_Color col)
{
    WSetColor(ren, col);
    for (int dy = -r; dy <= r; dy++) {
        int dx = (int)sqrt((double)(r * r - dy * dy));
        SDL_RenderDrawLine(ren, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void WFillPieSlice(SDL_Renderer *ren, int cx, int cy, int r,
                   double dA0, double dA1, SDL_Color col)
{
    if (dA1 <= dA0) return;
    WSetColor(ren, col);
    /* Use scanline fill: for each row in the bounding box, clip to the sector. */
    const int steps = (int)(fabs(dA1 - dA0) / (2.0 * M_PI) * 720.0) + 4;
    for (int s = 0; s <= steps; s++) {
        double a = dA0 + (dA1 - dA0) * (double)s / (double)steps;
        int ex = cx + (int)((double)r * cos(a));
        int ey = cy + (int)((double)r * sin(a));
        SDL_RenderDrawLine(ren, cx, cy, ex, ey);
    }
}

/* ---- Rounded rect helpers ---- */

static void _DrawCircleQuadrant(SDL_Renderer *ren, int cx, int cy, int r, int q)
{
    /* q: 0=top-right, 1=top-left, 2=bottom-left, 3=bottom-right */
    int x = 0, y = r, d = 1 - r;
    while (x <= y) {
        int pts[4][2] = {
            { cx + x, cy - y }, { cx + y, cy - x },
            { cx - x, cy - y }, { cx - y, cy - x },
        };
        int off[4][2] = { {0,0},{0,0},{0,0},{0,0} };
        (void)off; (void)pts; (void)q;
        /* Draw single points on the arc */
        switch (q) {
            case 0: SDL_RenderDrawPoint(ren, cx + x, cy - y);
                    SDL_RenderDrawPoint(ren, cx + y, cy - x); break;
            case 1: SDL_RenderDrawPoint(ren, cx - x, cy - y);
                    SDL_RenderDrawPoint(ren, cx - y, cy - x); break;
            case 2: SDL_RenderDrawPoint(ren, cx - x, cy + y);
                    SDL_RenderDrawPoint(ren, cx - y, cy + x); break;
            case 3: SDL_RenderDrawPoint(ren, cx + x, cy + y);
                    SDL_RenderDrawPoint(ren, cx + y, cy + x); break;
        }
        if (d < 0) d += 2 * x + 3;
        else { d += 2 * (x - y) + 5; y--; }
        x++;
    }
}

void WDrawRoundRect(SDL_Renderer *ren, SDL_Rect rect, int iRadius, SDL_Color col)
{
    int hw = rect.w / 2, hh = rect.h / 2;
    if (iRadius > hw) iRadius = hw;
    if (iRadius > hh) iRadius = hh;
    WSetColor(ren, col);
    int x = rect.x, y = rect.y, w = rect.w, h = rect.h, r = iRadius;
    _DrawCircleQuadrant(ren, x + r,     y + r,     r, 1);
    _DrawCircleQuadrant(ren, x + w - r, y + r,     r, 0);
    _DrawCircleQuadrant(ren, x + r,     y + h - r, r, 2);
    _DrawCircleQuadrant(ren, x + w - r, y + h - r, r, 3);
    SDL_RenderDrawLine(ren, x + r,     y,         x + w - r, y);
    SDL_RenderDrawLine(ren, x + r,     y + h,     x + w - r, y + h);
    SDL_RenderDrawLine(ren, x,         y + r,     x,         y + h - r);
    SDL_RenderDrawLine(ren, x + w,     y + r,     x + w,     y + h - r);
}

void WFillRoundRect(SDL_Renderer *ren, SDL_Rect rect, int iRadius, SDL_Color col)
{
    int hw = rect.w / 2, hh = rect.h / 2;
    if (iRadius > hw) iRadius = hw;
    if (iRadius > hh) iRadius = hh;
    WSetColor(ren, col);
    int x = rect.x, y = rect.y, w = rect.w, h = rect.h, r = iRadius;
    /* Fill the three non-overlapping strips */
    SDL_Rect mid = { x,     y + r,     w,     h - 2 * r };
    SDL_Rect top = { x + r, y,         w - 2 * r, r     };
    SDL_Rect bot = { x + r, y + h - r, w - 2 * r, r     };
    SDL_RenderFillRect(ren, &mid);
    SDL_RenderFillRect(ren, &top);
    SDL_RenderFillRect(ren, &bot);
    /* Fill corner circles */
    WFillCircle(ren, x + r,     y + r,     r, col);
    WFillCircle(ren, x + w - r, y + r,     r, col);
    WFillCircle(ren, x + r,     y + h - r, r, col);
    WFillCircle(ren, x + w - r, y + h - r, r, col);
}

/* ============================================================
 *  Chart background
 * ============================================================ */

void WDrawChartBg(SDL_Renderer *ren, int winW, int winH,
                  int padL, int padT, int padR, int padB,
                  SDL_Rect *pPlot)
{
    /* Window background */
    WFillRect(ren, (SDL_Rect){0, 0, winW, winH}, WC_BG);

    /* Plot surface */
    SDL_Rect plot = { padL, padT, winW - padL - padR, winH - padT - padB };
    WFillRect(ren, plot, WC_SURFACE);
    WDrawRect(ren, plot, WC_BORDER);

    if (pPlot) *pPlot = plot;
}

void WDrawHGrid(SDL_Renderer *ren, SDL_Rect plot, int nLines, SDL_Color col)
{
    if (nLines < 2) return;
    for (int i = 0; i <= nLines; i++) {
        int y = plot.y + (int)((double)i / (double)nLines * (double)plot.h);
        WDrawLine(ren, plot.x, y, plot.x + plot.w, y, col);
    }
}

void WDrawVGrid(SDL_Renderer *ren, SDL_Rect plot, int nLines, SDL_Color col)
{
    if (nLines < 2) return;
    for (int i = 0; i <= nLines; i++) {
        int x = plot.x + (int)((double)i / (double)nLines * (double)plot.w);
        WDrawLine(ren, x, plot.y, x, plot.y + plot.h, col);
    }
}

void WDrawXLabels(SDL_Renderer *ren, SDL_Rect plot, int yBaseline,
                  const char * const *pLabels, int nLabels,
                  SDL_Color col, FontSize eFontSize)
{
    if (!pLabels || nLabels <= 0) return;
    for (int i = 0; i < nLabels; i++) {
        if (!pLabels[i]) continue;
        int cx = plot.x + (int)((double)i / (double)(nLabels - 1) * (double)plot.w);
        WTextCentered(ren, cx - 40, cx + 40, yBaseline, pLabels[i], col, eFontSize);
    }
}

void WDrawYLabels(SDL_Renderer *ren, SDL_Rect plot, int xRight,
                  const char * const *pLabels, int nLabels,
                  SDL_Color col, FontSize eFontSize)
{
    if (!pLabels || nLabels <= 0) return;
    int ch = FontCellHeight(eFontSize);
    for (int i = 0; i < nLabels; i++) {
        if (!pLabels[i]) continue;
        /* i=0 → bottom, i=nLabels-1 → top */
        int y = plot.y + plot.h - (int)((double)i / (double)(nLabels - 1) * (double)plot.h) - ch / 2;
        WTextRight(ren, xRight, y, pLabels[i], col, eFontSize);
    }
}

/* ============================================================
 *  Text helpers
 * ============================================================ */

int WText(SDL_Renderer *ren, int x, int y, const char *pszText,
          SDL_Color col, FontSize eFontSize)
{
    return FontDrawText(ren, x, y, pszText, col, eFontSize);
}

void WTextCentered(SDL_Renderer *ren, int xLeft, int xRight, int y,
                   const char *pszText, SDL_Color col, FontSize eFontSize)
{
    if (!pszText) return;
    int tw = FontTextWidth(pszText, eFontSize);
    int cx = (xLeft + xRight) / 2 - tw / 2;
    FontDrawText(ren, cx, y, pszText, col, eFontSize);
}

void WTextRight(SDL_Renderer *ren, int xRight, int y,
                const char *pszText, SDL_Color col, FontSize eFontSize)
{
    if (!pszText) return;
    int tw = FontTextWidth(pszText, eFontSize);
    FontDrawText(ren, xRight - tw, y, pszText, col, eFontSize);
}

/* ============================================================
 *  Chart title
 * ============================================================ */

int WDrawTitle(SDL_Renderer *ren, int winW,
               const char *pszTitle, const char *pszSubtitle)
{
    int y = 18;
    WTextCentered(ren, 0, winW, y, pszTitle,    WC_TEXT,     FONT_LG);
    y += FontCellHeight(FONT_LG) + 4;
    if (pszSubtitle && pszSubtitle[0]) {
        WTextCentered(ren, 0, winW, y, pszSubtitle, WC_TEXT_DIM, FONT_SM);
        y += FontCellHeight(FONT_SM) + 4;
    }
    return y + 8;  /* suggested padT */
}

/* ============================================================
 *  Crosshair & tooltip
 * ============================================================ */

void WDrawCrosshair(SDL_Renderer *ren, SDL_Rect plot, int x, SDL_Color col)
{
    if (x < plot.x || x > plot.x + plot.w) return;
    WSetColor(ren, col);
    SDL_RenderDrawLine(ren, x, plot.y, x, plot.y + plot.h);
}

void WDrawTooltip(SDL_Renderer *ren, int winW, int winH,
                  int x, int y, const char * const *pLines, int nLines,
                  FontSize eFontSize)
{
    if (!pLines || nLines <= 0) return;
    const int cw = FontMetricsFor(eFontSize).cw;
    const int ch = FontCellHeight(eFontSize);
    const int pad = 8, gap = 3, radius = 6;

    /* Find the widest line */
    int maxW = 0;
    for (int i = 0; i < nLines; i++) {
        if (!pLines[i]) continue;
        int w = (int)strlen(pLines[i]) * cw;
        if (w > maxW) maxW = w;
    }
    int boxW = maxW + pad * 2;
    int boxH = nLines * (ch + gap) - gap + pad * 2;

    /* Flip so it stays on screen */
    int bx = x + 12;
    int by = y - boxH / 2;
    if (bx + boxW > winW - 4) bx = x - boxW - 12;
    if (by < 4)               by = 4;
    if (by + boxH > winH - 4) by = winH - boxH - 4;

    /* Shadow */
    WFillRoundRect(ren, (SDL_Rect){bx+3, by+3, boxW, boxH}, radius,
                   (SDL_Color){0,0,0,100});
    /* Card */
    WFillRoundRect(ren, (SDL_Rect){bx, by, boxW, boxH}, radius,
                   (SDL_Color){28, 35, 50, 240});
    WDrawRoundRect(ren, (SDL_Rect){bx, by, boxW, boxH}, radius, WC_BORDER);

    /* Lines */
    for (int i = 0; i < nLines; i++) {
        if (!pLines[i]) continue;
        SDL_Color col = (i == 0) ? WC_TEXT : WC_TEXT_DIM;
        FontDrawText(ren, bx + pad, by + pad + i * (ch + gap), pLines[i], col, eFontSize);
    }
}

/* ============================================================
 *  Legend
 * ============================================================ */

int WDrawLegend(SDL_Renderer *ren, int xLeft, int y, int winW,
                const char * const *pLabels, const SDL_Color *pColors,
                int nEntries, FontSize eFontSize)
{
    const int ch = FontCellHeight(eFontSize);
    const int dotSz = 10;
    const int gap   = 6;
    const int entryGap = 20;

    /* Compute total width to center the legend */
    int totalW = 0;
    for (int i = 0; i < nEntries; i++) {
        if (pLabels[i])
            totalW += dotSz + gap + FontTextWidth(pLabels[i], eFontSize) + entryGap;
    }
    totalW -= entryGap; /* remove trailing gap */

    int x = (xLeft + winW) / 2 - totalW / 2;
    int dotY = y + (ch - dotSz) / 2;

    for (int i = 0; i < nEntries; i++) {
        if (!pLabels[i]) continue;
        WFillRoundRect(ren, (SDL_Rect){x, dotY, dotSz, dotSz}, 3, pColors[i]);
        x += dotSz + gap;
        FontDrawText(ren, x, y, pLabels[i], WC_TEXT_DIM, eFontSize);
        x += FontTextWidth(pLabels[i], eFontSize) + entryGap;
    }
    return y + ch + 6;
}

/* ============================================================
 *  Stat card
 * ============================================================ */

int WDrawStatCard(SDL_Renderer *ren, int x, int y,
                  const char *pszLabel, const char *pszValue,
                  SDL_Color colAccent)
{
    const int pad = 10, radius = 8, gap = 4;
    int lw = FontTextWidth(pszLabel, FONT_SM);
    int vw = FontTextWidth(pszValue, FONT_MD);
    int cardW = (lw > vw ? lw : vw) + pad * 2;
    int cardH = FontCellHeight(FONT_SM) + gap + FontCellHeight(FONT_MD) + pad * 2;

    /* Accent stripe on the left */
    WFillRoundRect(ren, (SDL_Rect){x, y, cardW, cardH}, radius,
                   (SDL_Color){28, 35, 50, 220});
    WFillRect(ren, (SDL_Rect){x, y + radius, 3, cardH - radius * 2}, colAccent);

    FontDrawText(ren, x + pad, y + pad,
                 pszLabel, WC_TEXT_DIM, FONT_SM);
    FontDrawText(ren, x + pad, y + pad + FontCellHeight(FONT_SM) + gap,
                 pszValue, WC_TEXT, FONT_MD);

    return x + cardW + 10;
}

/* ============================================================
 *  Progress bar (SDL)
 * ============================================================ */

void WDrawProgressBar(SDL_Renderer *ren, SDL_Rect rect,
                      double dFraction, SDL_Color colFill, SDL_Color colBg)
{
    if (dFraction < 0.0) dFraction = 0.0;
    if (dFraction > 1.0) dFraction = 1.0;
    WFillRoundRect(ren, rect, rect.h / 2, colBg);
    if (dFraction > 0.0) {
        SDL_Rect fill = { rect.x, rect.y, (int)(rect.w * dFraction), rect.h };
        if (fill.w < rect.h) fill.w = rect.h; /* keep corners round */
        WFillRoundRect(ren, fill, rect.h / 2, colFill);
    }
}

/* ============================================================
 *  Help strip
 * ============================================================ */

void WDrawHelpStrip(SDL_Renderer *ren, int winW, int winH,
                    const char *pszText)
{
    const int h = FontCellHeight(FONT_SM) + 10;
    WFillRect(ren, (SDL_Rect){0, winH - h, winW, h},
              (SDL_Color){10, 12, 18, 200});
    WDrawLine(ren, 0, winH - h, winW, winH - h, WC_BORDER);
    if (pszText)
        FontDrawText(ren, 12, winH - h + 5, pszText, WC_TEXT_DIM, FONT_SM);
}

void WFormatCurrency(double val, char *buf, size_t size)
{
    if (!buf || size == 0) return;
    double absVal = fabs(val);
    const char *sign = (val < 0.0) ? "-" : "";
    if (absVal >= 1e9) {
        snprintf(buf, size, "%s$%.2lfB", sign, absVal / 1e9);
    } else if (absVal >= 1e6) {
        snprintf(buf, size, "%s$%.2lfM", sign, absVal / 1e6);
    } else if (absVal >= 1e4) {
        snprintf(buf, size, "%s$%.1lfk", sign, absVal / 1e3);
    } else {
        snprintf(buf, size, "%s$%.0lf", sign, absVal);
    }
}

