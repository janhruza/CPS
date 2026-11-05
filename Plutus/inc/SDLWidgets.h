//
// Created on 10/31/26.
// SDLWidgets.h - Reusable SDL2 drawing primitives, event helpers, and chart widgets.
//               All chart-drawing code should use these instead of duplicating SDL calls.
//
// Design rules:
//  - All functions take SDL_Renderer* as first argument.
//  - Colors are SDL_Color (r,g,b,a).
//  - No globals: callers pass all state in.
//  - Header declares; SDLWidgets.c implements (no inline bodies here).
//

#ifndef PLUTUS_SDLWIDGETS_H
#define PLUTUS_SDLWIDGETS_H

#include <stdbool.h>
#include <stddef.h>
#include <SDL.h>

#include "Font.h"

/* ============================================================
 *  Modern color palette — dark theme
 * ============================================================ */

/* Background & surface layers */
#define WC_BG           ((SDL_Color){15,  17,  23,  255})   /* #0F1117 — window background */
#define WC_SURFACE      ((SDL_Color){22,  27,  34,  255})   /* #16161D — chart plot area */
#define WC_BORDER       ((SDL_Color){48,  54,  61,  255})   /* #30363D — panel borders */
#define WC_GRID         ((SDL_Color){30,  33,  48,  255})   /* #1E2130 — grid lines */

/* Text */
#define WC_TEXT         ((SDL_Color){226, 232, 240, 255})   /* #E2E8F0 — primary text */
#define WC_TEXT_DIM     ((SDL_Color){100, 116, 139, 255})   /* #64748B — secondary / axis text */
#define WC_TEXT_FAINT   ((SDL_Color){51,  65,  85,  255})   /* #334155 — very dim labels */

/* Accent palette (8 distinct hues, good for category charts) */
#define WC_BLUE         ((SDL_Color){59,  130, 246, 255})   /* #3B82F6 */
#define WC_CYAN         ((SDL_Color){34,  211, 238, 255})   /* #22D3EE */
#define WC_GREEN        ((SDL_Color){52,  211, 153, 255})   /* #34D399 */
#define WC_LIME         ((SDL_Color){163, 230, 53,  255})   /* #A3E635 */
#define WC_YELLOW       ((SDL_Color){251, 191, 36,  255})   /* #FBBF24 */
#define WC_ORANGE       ((SDL_Color){251, 146, 60,  255})   /* #FB923C */
#define WC_RED          ((SDL_Color){248, 113, 113, 255})   /* #F87171 */
#define WC_PURPLE       ((SDL_Color){167, 139, 250, 255})   /* #A78BFA */

/* Transparent / alpha variants */
#define WC_ALPHA(c, a)  ((SDL_Color){(c).r, (c).g, (c).b, (a)})

/* Canonical 8-color palette array — used by pie/legend helpers */
extern const SDL_Color g_wPalette[8];

/* ============================================================
 *  Window lifecycle
 * ============================================================ */

/*
 * Opens an SDL window + hardware-accelerated renderer.
 * Falls back to software renderer if HW is unavailable.
 * Returns false and prints to stderr on failure.
 */
bool WOpen(const char *pszTitle, int iW, int iH,
           SDL_Window **ppWin, SDL_Renderer **ppRen);

/*
 * Destroys renderer and window, calls SDL_Quit.
 * Safe to call with NULL pointers.
 */
void WClose(SDL_Renderer **ppRen, SDL_Window **ppWin);

/* ============================================================
 *  Event loop
 * ============================================================ */

/*
 * Drains the SDL event queue.
 * Updates *pMouseX / *pMouseY when the mouse moves (may be NULL).
 * Returns false when the user presses ESC, Q, or closes the window.
 */
bool WPollEvents(int *pMouseX, int *pMouseY);

/* ============================================================
 *  Primitive drawing helpers
 * ============================================================ */

/* Set draw color from an SDL_Color struct. */
void WSetColor(SDL_Renderer *ren, SDL_Color col);

/* Fill a rectangle. */
void WFillRect(SDL_Renderer *ren, SDL_Rect rect, SDL_Color col);

/* Draw a 1-px border rectangle. */
void WDrawRect(SDL_Renderer *ren, SDL_Rect rect, SDL_Color col);

/* Draw a solid line. */
void WDrawLine(SDL_Renderer *ren, int x0, int y0, int x1, int y1, SDL_Color col);

/*
 * Draw a dashed line from (x0,y0) to (x1,y1).
 * iDash = dash length in pixels, iGap = gap length in pixels.
 */
void WDrawDashedLine(SDL_Renderer *ren,
                     int x0, int y0, int x1, int y1,
                     SDL_Color col, int iDash, int iGap);

/* Filled circle (midpoint algorithm). */
void WFillCircle(SDL_Renderer *ren, int cx, int cy, int r, SDL_Color col);

/* Filled pie slice from angle a0 to a1 (radians, clockwise from right). */
void WFillPieSlice(SDL_Renderer *ren, int cx, int cy, int r,
                   double dA0, double dA1, SDL_Color col);

/*
 * Draw a rounded rectangle outline.
 * iRadius — corner radius in pixels (clamped to half the shorter side).
 */
void WDrawRoundRect(SDL_Renderer *ren, SDL_Rect rect, int iRadius, SDL_Color col);

/*
 * Fill a rounded rectangle.
 */
void WFillRoundRect(SDL_Renderer *ren, SDL_Rect rect, int iRadius, SDL_Color col);

/* ============================================================
 *  Chart background helpers
 * ============================================================ */

/*
 * Fills the window background (WC_BG) and draws the plot area surface
 * (WC_SURFACE) and border.
 * padL/padT/padR/padB — padding around the plot area in pixels.
 * Writes the actual plot-area rect into *pPlot if non-NULL.
 */
void WDrawChartBg(SDL_Renderer *ren, int winW, int winH,
                  int padL, int padT, int padR, int padB,
                  SDL_Rect *pPlot);

/*
 * Draw horizontal grid lines inside the plot area.
 * nLines — number of lines (including top and bottom).
 */
void WDrawHGrid(SDL_Renderer *ren, SDL_Rect plot, int nLines, SDL_Color col);

/*
 * Draw vertical grid lines inside the plot area.
 */
void WDrawVGrid(SDL_Renderer *ren, SDL_Rect plot, int nLines, SDL_Color col);

/*
 * Draw X-axis label strings below the plot area.
 * pLabels — array of nLabels C strings.
 * They are distributed evenly across the plot width.
 */
void WDrawXLabels(SDL_Renderer *ren, SDL_Rect plot, int yBaseline,
                  const char * const *pLabels, int nLabels,
                  SDL_Color col, FontSize eFontSize);

/*
 * Draw Y-axis label strings to the left of the plot area.
 * pLabels — array of nLabels C strings, ordered bottom-to-top.
 */
void WDrawYLabels(SDL_Renderer *ren, SDL_Rect plot, int xRight,
                  const char * const *pLabels, int nLabels,
                  SDL_Color col, FontSize eFontSize);

/* ============================================================
 *  Text drawing (delegates to Font.h)
 * ============================================================ */

/*
 * Render a string at (x, y) using the given FontSize.
 * Returns the x position after the last character.
 */
int WText(SDL_Renderer *ren, int x, int y, const char *pszText,
          SDL_Color col, FontSize eFontSize);

/*
 * Render a string centered horizontally in [xLeft, xRight].
 */
void WTextCentered(SDL_Renderer *ren, int xLeft, int xRight, int y,
                   const char *pszText, SDL_Color col, FontSize eFontSize);

/*
 * Render a string right-aligned at xRight.
 */
void WTextRight(SDL_Renderer *ren, int xRight, int y,
                const char *pszText, SDL_Color col, FontSize eFontSize);

/* ============================================================
 *  Chart title + subtitle
 * ============================================================ */

/*
 * Draw a large title and smaller subtitle centred at the top of the window.
 * Returns the y coordinate of the bottom of the subtitle (usable as padT).
 */
int WDrawTitle(SDL_Renderer *ren, int winW,
               const char *pszTitle, const char *pszSubtitle);

/* ============================================================
 *  Crosshair & tooltip
 * ============================================================ */

/*
 * Draw a vertical crosshair line at x within the plot area.
 */
void WDrawCrosshair(SDL_Renderer *ren, SDL_Rect plot, int x, SDL_Color col);

/*
 * A tooltip is a small rounded card rendered at (x, y).
 * pLines — array of nLines C strings shown one per row.
 * The card auto-sizes to the widest string.
 * Flips left/right and up/down if too close to window edges.
 */
void WDrawTooltip(SDL_Renderer *ren, int winW, int winH,
                  int x, int y, const char * const *pLines, int nLines,
                  FontSize eFontSize);

/* ============================================================
 *  Legend
 * ============================================================ */

/*
 * Draw a horizontal legend strip.
 * Each entry has a coloured 10×10 square followed by a label.
 * Returns the y position below the legend.
 */
int WDrawLegend(SDL_Renderer *ren, int xLeft, int y, int winW,
                const char * const *pLabels, const SDL_Color *pColors,
                int nEntries, FontSize eFontSize);

/* ============================================================
 *  Summary stat box
 * ============================================================ */

/*
 * Draw a stat card: a rounded box containing a label + value pair.
 * Returns the right edge of the card (so the caller can chain them).
 */
int WDrawStatCard(SDL_Renderer *ren, int x, int y,
                  const char *pszLabel, const char *pszValue,
                  SDL_Color colAccent);

/* ============================================================
 *  Progress / loading bar (SDL version)
 * ============================================================ */

/*
 * Draw a horizontal filled progress bar inside rect.
 * dFraction — 0.0 to 1.0 (clamped).
 */
void WDrawProgressBar(SDL_Renderer *ren, SDL_Rect rect,
                      double dFraction, SDL_Color colFill, SDL_Color colBg);

/* ============================================================
 *  Help strip (keyboard hint at bottom of window)
 * ============================================================ */

/*
 * Draw a 1-line help bar at the very bottom of the window.
 * pszText — e.g. "  ESC / Q — close    S — screenshot"
 */
void WDrawHelpStrip(SDL_Renderer *ren, int winW, int winH,
                    const char *pszText);

/* ============================================================
 *  Formatting helpers
 * ============================================================ */

/* Format a currency value compactly into buf, e.g. "$1.25M", "$50.0k", "$350" */
void WFormatCurrency(double val, char *buf, size_t size);

#endif /* PLUTUS_SDLWIDGETS_H */

