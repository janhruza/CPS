/*
 * SDLInput.c  –  SDL2 interactive input widget toolkit for Plutus.
 *
 * Implements all widgets declared in inc/SDLInput.h:
 *   WIButton, WITextBox, WISpinner, WICheckbox, WISlider, WIDropdown, WIForm.
 *
 * Design notes:
 *   - Dark-theme colours reuse the WC_* palette from SDLWidgets.h.
 *   - Font rendering uses FontDrawText() / FontDrawChar() from Font.h (bitmap, no SDL_ttf).
 *   - SDL_StartTextInput / SDL_StopTextInput gate Unicode entry for WITextBox.
 *   - WIFormRunModal creates a temporary SDL window if none is supplied.
 *   - All pixel dimensions are in logical window pixels; no DPI scaling.
 */

#include "inc/SDLInput.h"
#include "inc/Font.h"
#include "inc/SDLWidgets.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* ============================================================
 *  Internal colour constants  (RGBA)
 * ============================================================ */

/* Background colours */
#define CI_BG           { 15,  17,  23, 255}   /* WC_BG      */
#define CI_SURFACE      { 22,  22,  29, 255}   /* WC_SURFACE */
#define CI_BORDER       { 48,  54,  61, 255}   /* WC_BORDER  */
#define CI_BORDER_FOCUS { 88, 166, 255, 255}   /* blue focus ring */
#define CI_LABEL        {200, 210, 220, 255}   /* label text */
#define CI_TEXT         {230, 237, 243, 255}   /* input text */
#define CI_PLACEHOLDER  { 90, 100, 110, 255}   /* placeholder text */
#define CI_CURSOR       {255, 255, 255, 200}   /* text cursor */
#define CI_HOVER_OVERLAY  { 255, 255, 255,  18}    /* semi-transparent hover tint */
#define CI_PRESS_OVERLAY  { 255, 255, 255,  40}    /* semi-transparent press tint */
#define CI_DISABLED_FG  { 80,  90, 100, 255}
#define CI_DISABLED_BG  { 18,  20,  26, 255}
#define CI_CHECK_FILL   { 35, 134, 100, 255}   /* checkbox tick green */
#define CI_SLIDER_TRACK { 40,  46,  56, 255}
#define CI_SLIDER_THUMB {200, 210, 220, 255}

/* Cursor blink period */
#define CURSOR_BLINK_MS  600u

/* Button height when auto-placed by WIFormInit */
#define FORM_BTN_H       36
#define FORM_BTN_W      100
#define FORM_BTN_GAP     12
#define FORM_MARGIN      16

/* ============================================================
 *  Small SDL helpers
 * ============================================================ */

static void ci_set(SDL_Renderer *ren, SDL_Color c)
{
    SDL_SetRenderDrawColor(ren, c.r, c.g, c.b, c.a);
}

static bool ci_point_in(SDL_Rect r, int x, int y)
{
    return (x >= r.x && x < r.x + r.w && y >= r.y && y < r.y + r.h);
}

/* Draw a filled rect */
static void ci_fill(SDL_Renderer *ren, SDL_Rect r, SDL_Color c)
{
    ci_set(ren, c);
    SDL_RenderFillRect(ren, &r);
}

/* Draw an outlined rect */
static void ci_rect(SDL_Renderer *ren, SDL_Rect r, SDL_Color c)
{
    ci_set(ren, c);
    SDL_RenderDrawRect(ren, &r);
}

/* Filled rounded rectangle — simple approximation */
static void ci_fill_round(SDL_Renderer *ren, SDL_Rect r, int rad, SDL_Color c)
{
    if (rad <= 0) { ci_fill(ren, r, c); return; }
    WFillRoundRect(ren, r, rad, c);
}

static void ci_rect_round(SDL_Renderer *ren, SDL_Rect r, int rad, SDL_Color c)
{
    if (rad <= 0) { ci_rect(ren, r, c); return; }
    WDrawRoundRect(ren, r, rad, c);
}

/* Draw text with the MD font */
static void ci_text(SDL_Renderer *ren, int x, int y, const char *sz, SDL_Color col)
{
    FontDrawText(ren, x, y, sz, col, FONT_MD);
}

/* Draw label (SM font, dimmer) */
static void ci_label(SDL_Renderer *ren, int x, int y, const char *sz)
{
    SDL_Color c = (SDL_Color)CI_LABEL;
    FontDrawText(ren, x, y, sz, c, FONT_SM);
}

/* Overlay tint for hover / press */
static void ci_tint(SDL_Renderer *ren, SDL_Rect r, SDL_Color c)
{
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);
    ci_set(ren, c);
    SDL_RenderFillRect(ren, &r);
    SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_NONE);
}

/* Font metrics shorthand */
static int ci_fmh(void) { return FontMetricsFor(FONT_MD).ch; }
static int ci_fmw(void) { return FontMetricsFor(FONT_MD).cw; }
static int ci_fsh(void) { return FontMetricsFor(FONT_SM).ch; }

/* ============================================================
 *  WIButton
 * ============================================================ */

void WIInitButton(WIButton *pBtn, SDL_Rect rect,
                  const char *pszLabel, SDL_Color colAccent)
{
    memset(pBtn, 0, sizeof(*pBtn));
    pBtn->rect      = rect;
    pBtn->colAccent = colAccent;
    snprintf(pBtn->szLabel, WI_BUTTON_LABEL_MAX, "%s", pszLabel ? pszLabel : "");
    pBtn->eState    = WI_STATE_NORMAL;
}

void WIFeedButton(WIButton *pBtn, WIEvent ev)
{
    if (pBtn->eState == WI_STATE_DISABLED) return;

    pBtn->bClicked = false;

    if (!ev.pEvent) return;

    bool bOver = ci_point_in(pBtn->rect, ev.mouseX, ev.mouseY);

    switch (ev.pEvent->type) {
        case SDL_MOUSEMOTION:
            if (!(pBtn->eState & WI_STATE_PRESSED))
                pBtn->eState = bOver ? WI_STATE_HOVER : WI_STATE_NORMAL;
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (ev.pEvent->button.button == SDL_BUTTON_LEFT && bOver) {
                pBtn->eState      = WI_STATE_PRESSED;
                pBtn->_bPressedPrev = true;
            }
            break;

        case SDL_MOUSEBUTTONUP:
            if (ev.pEvent->button.button == SDL_BUTTON_LEFT && pBtn->_bPressedPrev) {
                pBtn->_bPressedPrev = false;
                pBtn->bClicked  = bOver;
                pBtn->eState    = bOver ? WI_STATE_HOVER : WI_STATE_NORMAL;
            }
            break;

        default: break;
    }
}

void WIDrawButton(SDL_Renderer *ren, const WIButton *pBtn)
{
    SDL_Color colBg     = (SDL_Color)CI_SURFACE;
    SDL_Color colBorder = pBtn->colAccent;
    SDL_Color colText   = (SDL_Color)CI_TEXT;

    if (pBtn->eState == WI_STATE_DISABLED) {
        colBg     = (SDL_Color)CI_DISABLED_BG;
        colBorder = (SDL_Color)CI_DISABLED_FG;
        colText   = (SDL_Color)CI_DISABLED_FG;
    } else if (pBtn->eState & WI_STATE_PRESSED) {
        /* slightly lighter surface */
        colBg = (SDL_Color){30, 33, 42, 255};
    } else if (pBtn->eState == WI_STATE_FOCUSED) {
        colBorder = (SDL_Color)CI_BORDER_FOCUS;
    }

    ci_fill_round(ren, pBtn->rect, 5, colBg);
    ci_rect_round(ren, pBtn->rect, 5, colBorder);

    if (pBtn->eState == WI_STATE_HOVER)
        ci_tint(ren, pBtn->rect, (SDL_Color)CI_HOVER_OVERLAY);
    else if (pBtn->eState & WI_STATE_PRESSED)
        ci_tint(ren, pBtn->rect, (SDL_Color)CI_PRESS_OVERLAY);

    /* Centred label */
    int tw = FontTextWidth(pBtn->szLabel, FONT_MD);
    int tx = pBtn->rect.x + (pBtn->rect.w - tw) / 2;
    int ty = pBtn->rect.y + (pBtn->rect.h - ci_fmh()) / 2;
    ci_text(ren, tx, ty, pBtn->szLabel, colText);
}

/* ============================================================
 *  WITextBox
 * ============================================================ */

void WIInitTextBox(WITextBox *pTb, SDL_Rect rect,
                   const char *pszLabel, const char *pszPlaceholder)
{
    memset(pTb, 0, sizeof(*pTb));
    pTb->rect = rect;
    snprintf(pTb->szLabel,       sizeof(pTb->szLabel),       "%s", pszLabel       ? pszLabel       : "");
    snprintf(pTb->szPlaceholder, sizeof(pTb->szPlaceholder), "%s", pszPlaceholder ? pszPlaceholder : "");
    pTb->eState = WI_STATE_NORMAL;
}

void WIFeedTextBox(WITextBox *pTb, WIEvent ev)
{
    if (pTb->eState == WI_STATE_DISABLED) return;

    pTb->bChanged = false;

    bool bOver = ci_point_in(pTb->rect, ev.mouseX, ev.mouseY);

    if (ev.pEvent) {
        switch (ev.pEvent->type) {
            case SDL_MOUSEBUTTONDOWN:
                if (ev.pEvent->button.button == SDL_BUTTON_LEFT) {
                    bool wasFocused = (pTb->eState == WI_STATE_FOCUSED);
                    pTb->eState = bOver ? WI_STATE_FOCUSED : WI_STATE_NORMAL;
                    if (bOver && !wasFocused) SDL_StartTextInput();
                    if (!bOver && wasFocused) SDL_StopTextInput();
                }
                break;

            case SDL_MOUSEMOTION:
                if (pTb->eState != WI_STATE_FOCUSED)
                    pTb->eState = bOver ? WI_STATE_HOVER : WI_STATE_NORMAL;
                break;

            case SDL_TEXTINPUT:
                if (pTb->eState == WI_STATE_FOCUSED) {
                    size_t addLen = strlen(ev.pEvent->text.text);
                    size_t curLen = strlen(pTb->szText);
                    if (curLen + addLen < WI_TEXTBOX_CAP - 1) {
                        /* Insert at cursor position */
                        memmove(pTb->szText + pTb->iCursor + addLen,
                                pTb->szText + pTb->iCursor,
                                curLen - (size_t)pTb->iCursor + 1);
                        memcpy(pTb->szText + pTb->iCursor, ev.pEvent->text.text, addLen);
                        pTb->iCursor += (int)addLen;
                        pTb->bChanged = true;
                    }
                }
                break;

            case SDL_KEYDOWN:
                if (pTb->eState == WI_STATE_FOCUSED) {
                    SDL_Keycode sym = ev.pEvent->key.keysym.sym;
                    int  len = (int)strlen(pTb->szText);

                    if (sym == SDLK_BACKSPACE && pTb->iCursor > 0) {
                        /* Skip back over a UTF-8 multibyte lead byte */
                        int back = 1;
                        while (pTb->iCursor - back > 0 &&
                               (pTb->szText[pTb->iCursor - back] & 0xC0) == 0x80)
                            back++;
                        memmove(pTb->szText + pTb->iCursor - back,
                                pTb->szText + pTb->iCursor,
                                (size_t)(len - pTb->iCursor + 1));
                        pTb->iCursor -= back;
                        pTb->bChanged = true;
                    } else if (sym == SDLK_DELETE && pTb->iCursor < len) {
                        int fwd = 1;
                        while (pTb->iCursor + fwd < len &&
                               (pTb->szText[pTb->iCursor + fwd] & 0xC0) == 0x80)
                            fwd++;
                        memmove(pTb->szText + pTb->iCursor,
                                pTb->szText + pTb->iCursor + fwd,
                                (size_t)(len - pTb->iCursor - fwd + 1));
                        pTb->bChanged = true;
                    } else if (sym == SDLK_LEFT && pTb->iCursor > 0) {
                        pTb->iCursor--;
                        while (pTb->iCursor > 0 &&
                               (pTb->szText[pTb->iCursor] & 0xC0) == 0x80)
                            pTb->iCursor--;
                    } else if (sym == SDLK_RIGHT && pTb->iCursor < len) {
                        pTb->iCursor++;
                        while (pTb->iCursor < len &&
                               (pTb->szText[pTb->iCursor] & 0xC0) == 0x80)
                            pTb->iCursor++;
                    } else if (sym == SDLK_HOME) {
                        pTb->iCursor = 0;
                    } else if (sym == SDLK_END) {
                        pTb->iCursor = len;
                    }
                }
                break;

            default: break;
        }
    }

    /* Cursor blink update */
    if (pTb->eState == WI_STATE_FOCUSED) {
        Uint32 t = SDL_GetTicks();
        pTb->_bCursorVisible = ((t / CURSOR_BLINK_MS) % 2) == 0;
    }
}

void WIDrawTextBox(SDL_Renderer *ren, const WITextBox *pTb)
{
    bool bFocused  = (pTb->eState == WI_STATE_FOCUSED);
    bool bHover    = (pTb->eState == WI_STATE_HOVER);
    bool bDisabled = (pTb->eState == WI_STATE_DISABLED);

    SDL_Color colBg     = bDisabled ? (SDL_Color)CI_DISABLED_BG : (SDL_Color)CI_SURFACE;
    SDL_Color colBorder = bFocused  ? (SDL_Color)CI_BORDER_FOCUS
                        : bHover    ? (SDL_Color){70, 80, 94, 255}
                        :             (SDL_Color)CI_BORDER;

    /* Label above */
    if (pTb->szLabel[0]) {
        ci_label(ren,
                 pTb->rect.x,
                 pTb->rect.y - ci_fsh() - 3,
                 pTb->szLabel);
    }

    ci_fill_round(ren, pTb->rect, 4, colBg);
    ci_rect_round(ren, pTb->rect, 4, colBorder);

    if (bHover && !bFocused)
        ci_tint(ren, pTb->rect, (SDL_Color)CI_HOVER_OVERLAY);

    /* Text or placeholder */
    int tx = pTb->rect.x + 8;
    int ty = pTb->rect.y + (pTb->rect.h - ci_fmh()) / 2;

    bool bEmpty = (pTb->szText[0] == '\0');
    if (bEmpty && pTb->szPlaceholder[0]) {
        ci_text(ren, tx, ty, pTb->szPlaceholder, (SDL_Color)CI_PLACEHOLDER);
    } else {
        SDL_Color colText = bDisabled ? (SDL_Color)CI_DISABLED_FG : (SDL_Color)CI_TEXT;
        ci_text(ren, tx, ty, pTb->szText, colText);
    }

    /* Cursor */
    if (bFocused && pTb->_bCursorVisible) {
        /* Measure text up to cursor */
        char tmp[WI_TEXTBOX_CAP];
        int cursorByte = pTb->iCursor;
        if (cursorByte > (int)sizeof(tmp) - 1) cursorByte = (int)sizeof(tmp) - 1;
        memcpy(tmp, pTb->szText, (size_t)cursorByte);
        tmp[cursorByte] = '\0';
        int cx = tx + FontTextWidth(tmp, FONT_MD);
        SDL_SetRenderDrawColor(ren, 255, 255, 255, 200);
        SDL_RenderDrawLine(ren, cx, ty, cx, ty + ci_fmh() - 1);
    }
}

void WIClearTextBox(WITextBox *pTb)
{
    memset(pTb->szText, 0, sizeof(pTb->szText));
    pTb->iCursor  = 0;
    pTb->bChanged = false;
}

/* ============================================================
 *  WISpinner
 * ============================================================ */

/* Button width for +/- */
#define SP_BTN_W 28

void WIInitSpinner(WISpinner *pSp, SDL_Rect rect, const char *pszLabel,
                   double dInitial, double dMin, double dMax,
                   double dStep, int iDecimals)
{
    memset(pSp, 0, sizeof(*pSp));
    pSp->rect      = rect;
    pSp->dValue    = dInitial;
    pSp->dMin      = dMin;
    pSp->dMax      = dMax;
    pSp->dStep     = dStep;
    pSp->iDecimals = iDecimals;
    snprintf(pSp->szLabel, sizeof(pSp->szLabel), "%s", pszLabel ? pszLabel : "");

    /* Inner text box occupies the rect minus two button columns */
    SDL_Rect tbRect = {
        rect.x,
        rect.y,
        rect.w - SP_BTN_W * 2 - 4,
        rect.h
    };
    char placeholder[32];
    snprintf(placeholder, sizeof(placeholder), "%.*f", iDecimals, dInitial);
    WIInitTextBox(&pSp->_tb, tbRect, "", placeholder);
    snprintf(pSp->_tb.szText, WI_TEXTBOX_CAP, "%.*f", iDecimals, dInitial);
    pSp->_tb.iCursor = (int)strlen(pSp->_tb.szText);

    /* Minus button */
    SDL_Rect btnMinusRect = {
        rect.x + rect.w - SP_BTN_W * 2 - 2,
        rect.y,
        SP_BTN_W,
        rect.h
    };
    SDL_Color accentGray = {80, 90, 104, 255};
    WIInitButton(&pSp->_btnMinus, btnMinusRect, "\xe2\x88\x92", accentGray); /* − */

    /* Plus button */
    SDL_Rect btnPlusRect = {
        rect.x + rect.w - SP_BTN_W,
        rect.y,
        SP_BTN_W,
        rect.h
    };
    WIInitButton(&pSp->_btnPlus, btnPlusRect, "+", accentGray);
}

static void sp_clamp_sync(WISpinner *pSp)
{
    if (pSp->dValue < pSp->dMin) pSp->dValue = pSp->dMin;
    if (pSp->dValue > pSp->dMax) pSp->dValue = pSp->dMax;
    snprintf(pSp->_tb.szText, WI_TEXTBOX_CAP, "%.*f", pSp->iDecimals, pSp->dValue);
    pSp->_tb.iCursor = (int)strlen(pSp->_tb.szText);
}

void WIFeedSpinner(WISpinner *pSp, WIEvent ev)
{
    if (pSp->eState == WI_STATE_DISABLED) return;
    pSp->bChanged = false;

    WIFeedButton(&pSp->_btnMinus, ev);
    WIFeedButton(&pSp->_btnPlus,  ev);
    WIFeedTextBox(&pSp->_tb, ev);
    pSp->eState = pSp->_tb.eState;

    if (pSp->_btnMinus.bClicked) {
        pSp->dValue -= pSp->dStep;
        sp_clamp_sync(pSp);
        pSp->bChanged = true;
    }
    if (pSp->_btnPlus.bClicked) {
        pSp->dValue += pSp->dStep;
        sp_clamp_sync(pSp);
        pSp->bChanged = true;
    }

    /* When text box loses focus or Enter/arrow is pressed, update value */
    if (ev.pEvent && ev.pEvent->type == SDL_KEYDOWN) {
        SDL_Keycode sym = ev.pEvent->key.keysym.sym;
        if (pSp->_tb.eState == WI_STATE_FOCUSED) {
            if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
                pSp->dValue = atof(pSp->_tb.szText);
                sp_clamp_sync(pSp);
                pSp->bChanged = true;
            } else if (sym == SDLK_UP) {
                pSp->dValue += pSp->dStep;
                sp_clamp_sync(pSp);
                pSp->bChanged = true;
            } else if (sym == SDLK_DOWN) {
                pSp->dValue -= pSp->dStep;
                sp_clamp_sync(pSp);
                pSp->bChanged = true;
            }
        }
    }
    if (pSp->_tb.bChanged) {
        pSp->bChanged = true;
        /* Live-parse while typing */
        pSp->dValue = atof(pSp->_tb.szText);
        if (pSp->dValue < pSp->dMin) pSp->dValue = pSp->dMin;
        if (pSp->dValue > pSp->dMax) pSp->dValue = pSp->dMax;
    }
}

void WIDrawSpinner(SDL_Renderer *ren, const WISpinner *pSp)
{
    /* Label above */
    if (pSp->szLabel[0])
        ci_label(ren, pSp->rect.x, pSp->rect.y - ci_fsh() - 3, pSp->szLabel);

    WIDrawTextBox(ren, &pSp->_tb);
    WIDrawButton(ren, &pSp->_btnMinus);
    WIDrawButton(ren, &pSp->_btnPlus);
}

/* ============================================================
 *  WICheckbox
 * ============================================================ */

#define CB_BOX_SIZE 18

void WIInitCheckbox(WICheckbox *pCb, SDL_Rect rect,
                    const char *pszLabel, bool bInitial)
{
    memset(pCb, 0, sizeof(*pCb));
    pCb->rect     = rect;
    pCb->bChecked = bInitial;
    snprintf(pCb->szLabel, sizeof(pCb->szLabel), "%s", pszLabel ? pszLabel : "");
    pCb->eState = WI_STATE_NORMAL;
}

void WIFeedCheckbox(WICheckbox *pCb, WIEvent ev)
{
    if (pCb->eState == WI_STATE_DISABLED) return;
    pCb->bChanged = false;

    if (!ev.pEvent) return;
    bool bOver = ci_point_in(pCb->rect, ev.mouseX, ev.mouseY);

    switch (ev.pEvent->type) {
        case SDL_MOUSEMOTION:
            pCb->eState = bOver ? WI_STATE_HOVER : WI_STATE_NORMAL;
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (bOver && ev.pEvent->button.button == SDL_BUTTON_LEFT)
                pCb->eState = WI_STATE_PRESSED;
            break;
        case SDL_MOUSEBUTTONUP:
            if (ev.pEvent->button.button == SDL_BUTTON_LEFT && bOver &&
                pCb->eState == WI_STATE_PRESSED) {
                pCb->bChecked = !pCb->bChecked;
                pCb->bChanged = true;
                pCb->eState   = WI_STATE_HOVER;
            } else {
                pCb->eState = bOver ? WI_STATE_HOVER : WI_STATE_NORMAL;
            }
            break;
        default: break;
    }
}

void WIDrawCheckbox(SDL_Renderer *ren, const WICheckbox *pCb)
{
    SDL_Color colBorder = (pCb->eState == WI_STATE_FOCUSED)
                        ? (SDL_Color)CI_BORDER_FOCUS
                        : (SDL_Color)CI_BORDER;

    SDL_Rect box = {
        pCb->rect.x,
        pCb->rect.y + (pCb->rect.h - CB_BOX_SIZE) / 2,
        CB_BOX_SIZE,
        CB_BOX_SIZE
    };

    ci_fill_round(ren, box, 3, (SDL_Color)CI_SURFACE);
    if (pCb->bChecked)
        ci_fill_round(ren, box, 3, (SDL_Color)CI_CHECK_FILL);
    ci_rect_round(ren, box, 3, colBorder);

    if (pCb->eState == WI_STATE_HOVER)
        ci_tint(ren, box, (SDL_Color)CI_HOVER_OVERLAY);

    /* Checkmark */
    if (pCb->bChecked) {
        SDL_Color cw = {255, 255, 255, 255};
        int cx = box.x, cy = box.y;
        /* Simple tick: two lines */
        ci_set(ren, cw);
        SDL_RenderDrawLine(ren, cx + 3,  cy + 9,  cx + 7,  cy + 13);
        SDL_RenderDrawLine(ren, cx + 7,  cy + 13, cx + 14, cy + 4);
        SDL_RenderDrawLine(ren, cx + 3,  cy + 10, cx + 7,  cy + 14);
        SDL_RenderDrawLine(ren, cx + 7,  cy + 14, cx + 14, cy + 5);
    }

    /* Label to the right */
    int lx = box.x + CB_BOX_SIZE + 8;
    int ly = pCb->rect.y + (pCb->rect.h - ci_fmh()) / 2;
    SDL_Color colText = (pCb->eState == WI_STATE_DISABLED)
                      ? (SDL_Color)CI_DISABLED_FG
                      : (SDL_Color)CI_TEXT;
    ci_text(ren, lx, ly, pCb->szLabel, colText);
}

/* ============================================================
 *  WISlider
 * ============================================================ */

#define SL_TRACK_H  6
#define SL_THUMB_R  9

void WIInitSlider(WISlider *pSl, SDL_Rect rect, const char *pszLabel,
                  double dInitial, double dMin, double dMax,
                  int iDecimals, SDL_Color colFill)
{
    memset(pSl, 0, sizeof(*pSl));
    pSl->rect      = rect;
    pSl->dValue    = dInitial;
    pSl->dMin      = dMin;
    pSl->dMax      = dMax;
    pSl->iDecimals = iDecimals;
    pSl->colFill   = colFill;
    snprintf(pSl->szLabel, sizeof(pSl->szLabel), "%s", pszLabel ? pszLabel : "");
    pSl->eState = WI_STATE_NORMAL;
}

/* Map value -> x pixel in track */
static int sl_val_to_x(const WISlider *pSl)
{
    int trackX = pSl->rect.x + SL_THUMB_R;
    int trackW = pSl->rect.w - SL_THUMB_R * 2;
    double t   = (pSl->dValue - pSl->dMin) / (pSl->dMax - pSl->dMin);
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return trackX + (int)(t * (double)trackW);
}

/* Map x pixel -> value */
static double sl_x_to_val(const WISlider *pSl, int x)
{
    int trackX = pSl->rect.x + SL_THUMB_R;
    int trackW = pSl->rect.w - SL_THUMB_R * 2;
    double t   = (double)(x - trackX) / (double)trackW;
    if (t < 0.0) t = 0.0;
    if (t > 1.0) t = 1.0;
    return pSl->dMin + t * (pSl->dMax - pSl->dMin);
}

void WIFeedSlider(WISlider *pSl, WIEvent ev)
{
    if (pSl->eState == WI_STATE_DISABLED) return;
    pSl->bChanged = false;

    if (!ev.pEvent) return;
    bool bOver = ci_point_in(pSl->rect, ev.mouseX, ev.mouseY);

    switch (ev.pEvent->type) {
        case SDL_MOUSEMOTION:
            if (pSl->_bDragging) {
                pSl->dValue   = sl_x_to_val(pSl, ev.mouseX);
                pSl->bChanged = true;
            } else {
                pSl->eState = bOver ? WI_STATE_HOVER : WI_STATE_NORMAL;
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if (bOver && ev.pEvent->button.button == SDL_BUTTON_LEFT) {
                pSl->_bDragging = true;
                pSl->eState     = WI_STATE_PRESSED;
                pSl->dValue     = sl_x_to_val(pSl, ev.mouseX);
                pSl->bChanged   = true;
            }
            break;
        case SDL_MOUSEBUTTONUP:
            if (ev.pEvent->button.button == SDL_BUTTON_LEFT && pSl->_bDragging) {
                pSl->_bDragging = false;
                pSl->eState     = bOver ? WI_STATE_HOVER : WI_STATE_NORMAL;
            }
            break;
        default: break;
    }
}

void WIDrawSlider(SDL_Renderer *ren, const WISlider *pSl)
{
    /* Label above + value on right */
    if (pSl->szLabel[0]) {
        ci_label(ren, pSl->rect.x, pSl->rect.y - ci_fsh() - 3, pSl->szLabel);
        char valBuf[32];
        snprintf(valBuf, sizeof(valBuf), "%.*f", pSl->iDecimals, pSl->dValue);
        int vw = FontTextWidth(valBuf, FONT_SM);
        FontDrawText(ren,
                     pSl->rect.x + pSl->rect.w - vw,
                     pSl->rect.y - ci_fsh() - 3,
                     valBuf,
                     (SDL_Color)CI_TEXT,
                     FONT_SM);
    }

    /* Track background */
    int trackY = pSl->rect.y + pSl->rect.h / 2 - SL_TRACK_H / 2;
    SDL_Rect trackBg = {pSl->rect.x, trackY, pSl->rect.w, SL_TRACK_H};
    ci_fill_round(ren, trackBg, 3, (SDL_Color)CI_SLIDER_TRACK);

    /* Filled portion */
    int thumbX = sl_val_to_x(pSl);
    SDL_Rect trackFill = {pSl->rect.x, trackY, thumbX - pSl->rect.x, SL_TRACK_H};
    if (trackFill.w > 0)
        ci_fill_round(ren, trackFill, 3, pSl->colFill);

    /* Thumb */
    SDL_Color colThumb = (pSl->eState & WI_STATE_PRESSED)
                       ? (SDL_Color){255, 255, 255, 255}
                       : (SDL_Color)CI_SLIDER_THUMB;
    int thumbY = pSl->rect.y + pSl->rect.h / 2;

    /* Draw thumb as filled circle */
    WFillCircle(ren, thumbX, thumbY, SL_THUMB_R, colThumb);

    /* Focus ring */
    if (pSl->eState == WI_STATE_FOCUSED) {
        SDL_Color fr = (SDL_Color)CI_BORDER_FOCUS;
        ci_set(ren, fr);
        SDL_Rect outer = {pSl->rect.x - 2, pSl->rect.y - 2,
                          pSl->rect.w + 4, pSl->rect.h + 4};
        ci_rect(ren, outer, fr);
    }
}

/* ============================================================
 *  WIDropdown
 * ============================================================ */

void WIInitDropdown(WIDropdown *pDd, SDL_Rect rect, const char *pszLabel,
                    const char * const *ppOptions, int iCount, int iInitial)
{
    memset(pDd, 0, sizeof(*pDd));
    pDd->rect        = rect;
    pDd->iOptionCount = iCount < WI_DROPDOWN_MAX_OPTIONS ? iCount : WI_DROPDOWN_MAX_OPTIONS;
    pDd->iSelected    = (iInitial >= 0 && iInitial < pDd->iOptionCount) ? iInitial : 0;
    pDd->_iHovered    = -1;
    snprintf(pDd->szLabel, sizeof(pDd->szLabel), "%s", pszLabel ? pszLabel : "");

    for (int i = 0; i < pDd->iOptionCount; i++) {
        snprintf(pDd->szOptions[i], WI_DROPDOWN_OPTION_CAP, "%s",
                 ppOptions && ppOptions[i] ? ppOptions[i] : "");
    }
}

void WIFeedDropdown(WIDropdown *pDd, WIEvent ev)
{
    if (pDd->eState == WI_STATE_DISABLED) return;
    pDd->bChanged = false;

    if (!ev.pEvent) return;

    bool bOverHeader = ci_point_in(pDd->rect, ev.mouseX, ev.mouseY);
    int rowH = pDd->rect.h;
    SDL_Rect listRect = {
        pDd->rect.x,
        pDd->rect.y + pDd->rect.h,
        pDd->rect.w,
        rowH * pDd->iOptionCount
    };
    bool bOverList = pDd->_bOpen && ci_point_in(listRect, ev.mouseX, ev.mouseY);

    switch (ev.pEvent->type) {
        case SDL_MOUSEMOTION:
            if (!pDd->_bOpen)
                pDd->eState = bOverHeader ? WI_STATE_HOVER : WI_STATE_NORMAL;
            if (pDd->_bOpen && bOverList) {
                pDd->_iHovered = (ev.mouseY - listRect.y) / rowH;
                if (pDd->_iHovered >= pDd->iOptionCount)
                    pDd->_iHovered = pDd->iOptionCount - 1;
            } else {
                pDd->_iHovered = -1;
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
            if (ev.pEvent->button.button == SDL_BUTTON_LEFT) {
                if (bOverHeader) {
                    pDd->_bOpen = !pDd->_bOpen;
                    pDd->eState = pDd->_bOpen ? WI_STATE_PRESSED : WI_STATE_NORMAL;
                } else if (pDd->_bOpen && bOverList) {
                    int sel = (ev.mouseY - listRect.y) / rowH;
                    if (sel >= 0 && sel < pDd->iOptionCount) {
                        pDd->iSelected = sel;
                        pDd->bChanged  = true;
                    }
                    pDd->_bOpen = false;
                    pDd->eState = WI_STATE_NORMAL;
                } else if (pDd->_bOpen) {
                    /* Click outside — close */
                    pDd->_bOpen = false;
                    pDd->eState = WI_STATE_NORMAL;
                }
            }
            break;

        case SDL_KEYDOWN:
            if (pDd->eState == WI_STATE_FOCUSED) {
                SDL_Keycode sym = ev.pEvent->key.keysym.sym;
                if (sym == SDLK_RETURN || sym == SDLK_SPACE) {
                    pDd->_bOpen = !pDd->_bOpen;
                } else if (sym == SDLK_UP && pDd->iSelected > 0) {
                    pDd->iSelected--;
                    pDd->bChanged = true;
                } else if (sym == SDLK_DOWN && pDd->iSelected < pDd->iOptionCount - 1) {
                    pDd->iSelected++;
                    pDd->bChanged = true;
                } else if (sym == SDLK_ESCAPE) {
                    pDd->_bOpen = false;
                }
            }
            break;

        default: break;
    }
}

void WIDrawDropdown(SDL_Renderer *ren, const WIDropdown *pDd)
{
    bool bFocused = (pDd->eState == WI_STATE_FOCUSED || pDd->_bOpen);
    bool bHover   = (pDd->eState == WI_STATE_HOVER);

    SDL_Color colBorder = bFocused ? (SDL_Color)CI_BORDER_FOCUS
                        : bHover   ? (SDL_Color){70, 80, 94, 255}
                        :            (SDL_Color)CI_BORDER;

    /* Label */
    if (pDd->szLabel[0])
        ci_label(ren, pDd->rect.x, pDd->rect.y - ci_fsh() - 3, pDd->szLabel);

    /* Header box */
    ci_fill_round(ren, pDd->rect, 4, (SDL_Color)CI_SURFACE);
    ci_rect_round(ren, pDd->rect, 4, colBorder);
    if (bHover)
        ci_tint(ren, pDd->rect, (SDL_Color)CI_HOVER_OVERLAY);

    /* Selected text */
    const char *pszSel = (pDd->iSelected >= 0 && pDd->iSelected < pDd->iOptionCount)
                       ? pDd->szOptions[pDd->iSelected] : "";
    int ty = pDd->rect.y + (pDd->rect.h - ci_fmh()) / 2;
    ci_text(ren, pDd->rect.x + 8, ty, pszSel, (SDL_Color)CI_TEXT);

    /* Arrow indicator (▾) */
    const char *arrow = pDd->_bOpen ? "\xe2\x96\xb4" : "\xe2\x96\xbe"; /* ▴ / ▾ */
    int aw = ci_fmw();
    ci_text(ren, pDd->rect.x + pDd->rect.w - aw - 8, ty, arrow, (SDL_Color)CI_LABEL);

    /* Dropdown list (drawn on top — caller must draw dropdowns last) */
    if (pDd->_bOpen) {
        int rowH = pDd->rect.h;
        SDL_Rect listBg = {
            pDd->rect.x,
            pDd->rect.y + pDd->rect.h,
            pDd->rect.w,
            rowH * pDd->iOptionCount + 4
        };
        ci_fill_round(ren, listBg, 4, (SDL_Color)CI_SURFACE);
        ci_rect_round(ren, listBg, 4, (SDL_Color)CI_BORDER_FOCUS);

        for (int i = 0; i < pDd->iOptionCount; i++) {
            SDL_Rect row = {
                pDd->rect.x + 2,
                pDd->rect.y + pDd->rect.h + 2 + i * rowH,
                pDd->rect.w - 4,
                rowH
            };

            if (i == pDd->iSelected)
                ci_fill_round(ren, row, 3, (SDL_Color){30, 80, 160, 100});
            else if (i == pDd->_iHovered)
                ci_fill_round(ren, row, 3, (SDL_Color)CI_HOVER_OVERLAY);

            int oty = row.y + (row.h - ci_fmh()) / 2;
            ci_text(ren, row.x + 8, oty, pDd->szOptions[i], (SDL_Color)CI_TEXT);
        }
    }
}

/* ============================================================
 *  WIForm
 * ============================================================ */

void WIFormInit(WIForm *pForm, int winW, int winH)
{
    memset(pForm, 0, sizeof(*pForm));
    pForm->winW = winW;
    pForm->winH = winH;

    /* OK button — bottom-right */
    SDL_Rect okRect = {
        winW - FORM_MARGIN - FORM_BTN_W,
        winH - FORM_MARGIN - FORM_BTN_H,
        FORM_BTN_W,
        FORM_BTN_H
    };
    SDL_Color colOK = {35, 134, 100, 255};  /* green */
    WIInitButton(&pForm->btnOK, okRect, "OK", colOK);

    /* Cancel button — left of OK */
    SDL_Rect cancelRect = {
        winW - FORM_MARGIN - FORM_BTN_W * 2 - FORM_BTN_GAP,
        winH - FORM_MARGIN - FORM_BTN_H,
        FORM_BTN_W,
        FORM_BTN_H
    };
    SDL_Color colCancel = {130, 40, 40, 255};  /* red */
    WIInitButton(&pForm->btnCancel, cancelRect, "Cancel", colCancel);
}

void WIFormSetHeader(WIForm *pForm, const char *pszTitle, const char *pszSubtitle)
{
    snprintf(pForm->szTitle,    sizeof(pForm->szTitle),    "%s", pszTitle    ? pszTitle    : "");
    snprintf(pForm->szSubtitle, sizeof(pForm->szSubtitle), "%s", pszSubtitle ? pszSubtitle : "");
}

static WIState *slot_state(WISlot *s);

void WIFormAdd(WIForm *pForm, WIKind eKind, void *pWidget, int iTabIndex)
{
    if (pForm->iCount >= WI_FORM_MAX_WIDGETS) return;
    pForm->slots[pForm->iCount].eKind    = eKind;
    pForm->slots[pForm->iCount].pWidget  = pWidget;
    pForm->slots[pForm->iCount].iTabIndex = iTabIndex;

    /* If first focusable widget, set it focused by default */
    if (iTabIndex > 0) {
        bool bAnyFocused = false;
        for (int i = 0; i < pForm->iCount; i++) {
            if (pForm->slots[i].iTabIndex > 0) {
                bAnyFocused = true;
                break;
            }
        }
        if (!bAnyFocused) {
            pForm->iFocused = pForm->iCount;
            WIState *pSt = slot_state(&pForm->slots[pForm->iCount]);
            if (pSt) *pSt = WI_STATE_FOCUSED;
        }
    }

    pForm->iCount++;
}

/* Internal: get WIState pointer from a slot */
static WIState *slot_state(WISlot *s)
{
    switch (s->eKind) {
        case WI_KIND_BUTTON:   return &((WIButton   *)s->pWidget)->eState;
        case WI_KIND_TEXTBOX:  return &((WITextBox  *)s->pWidget)->eState;
        case WI_KIND_SPINNER:  return &((WISpinner  *)s->pWidget)->_tb.eState;
        case WI_KIND_CHECKBOX: return &((WICheckbox *)s->pWidget)->eState;
        case WI_KIND_SLIDER:   return &((WISlider   *)s->pWidget)->eState;
        case WI_KIND_DROPDOWN: return &((WIDropdown *)s->pWidget)->eState;
    }
    return NULL;
}

/* Internal: dispatch feed to a slot */
static void slot_feed(WISlot *s, WIEvent ev)
{
    switch (s->eKind) {
        case WI_KIND_BUTTON:   WIFeedButton  ((WIButton   *)s->pWidget, ev); break;
        case WI_KIND_TEXTBOX:  WIFeedTextBox ((WITextBox  *)s->pWidget, ev); break;
        case WI_KIND_SPINNER:  WIFeedSpinner ((WISpinner  *)s->pWidget, ev); break;
        case WI_KIND_CHECKBOX: WIFeedCheckbox((WICheckbox *)s->pWidget, ev); break;
        case WI_KIND_SLIDER:   WIFeedSlider  ((WISlider   *)s->pWidget, ev); break;
        case WI_KIND_DROPDOWN: WIFeedDropdown((WIDropdown *)s->pWidget, ev); break;
    }
}

/* Internal: dispatch draw to a slot */
static void slot_draw(SDL_Renderer *ren, const WISlot *s)
{
    switch (s->eKind) {
        case WI_KIND_BUTTON:   WIDrawButton  (ren, (WIButton   *)s->pWidget); break;
        case WI_KIND_TEXTBOX:  WIDrawTextBox (ren, (WITextBox  *)s->pWidget); break;
        case WI_KIND_SPINNER:  WIDrawSpinner (ren, (WISpinner  *)s->pWidget); break;
        case WI_KIND_CHECKBOX: WIDrawCheckbox(ren, (WICheckbox *)s->pWidget); break;
        case WI_KIND_SLIDER:   WIDrawSlider  (ren, (WISlider   *)s->pWidget); break;
        case WI_KIND_DROPDOWN: WIDrawDropdown(ren, (WIDropdown *)s->pWidget); break;
    }
}

void WIFormFeed(WIForm *pForm, const SDL_Event *pEvent, int mouseX, int mouseY)
{
    WIEvent ev = {pEvent, mouseX, mouseY};

    /* Tab / Shift-Tab focus cycling */
    if (pEvent && pEvent->type == SDL_KEYDOWN) {
        SDL_Keycode sym = pEvent->key.keysym.sym;
        if (sym == SDLK_TAB) {
            bool shift = (pEvent->key.keysym.mod & KMOD_SHIFT) != 0;

            /* Find next focusable slot */
            int dir   = shift ? -1 : 1;
            int start = pForm->iFocused;
            int cur   = (start + dir + pForm->iCount) % pForm->iCount;
            while (cur != start) {
                if (pForm->slots[cur].iTabIndex > 0) {
                    /* Defocus old */
                    WIState *pOld = slot_state(&pForm->slots[pForm->iFocused]);
                    if (pOld && *pOld == WI_STATE_FOCUSED) {
                        *pOld = WI_STATE_NORMAL;
                        /* Stop text input if leaving a textbox */
                        if (pForm->slots[pForm->iFocused].eKind == WI_KIND_TEXTBOX ||
                            pForm->slots[pForm->iFocused].eKind == WI_KIND_SPINNER)
                            SDL_StopTextInput();
                    }
                    pForm->iFocused = cur;
                    WIState *pNew = slot_state(&pForm->slots[cur]);
                    if (pNew) *pNew = WI_STATE_FOCUSED;
                    /* Start text input if entering a textbox/spinner */
                    if (pForm->slots[cur].eKind == WI_KIND_TEXTBOX ||
                        pForm->slots[cur].eKind == WI_KIND_SPINNER)
                        SDL_StartTextInput();
                    break;
                }
                cur = (cur + dir + pForm->iCount) % pForm->iCount;
            }
        }

        /* Escape cancels */
        if (sym == SDLK_ESCAPE)
            pForm->bCancelled = true;
    }

    /* Feed all slots first */
    for (int i = 0; i < pForm->iCount; i++)
        slot_feed(&pForm->slots[i], ev);

    /* Enter submits only if not handled by an open dropdown */
    if (pEvent && pEvent->type == SDL_KEYDOWN) {
        SDL_Keycode sym = pEvent->key.keysym.sym;
        if (sym == SDLK_RETURN || sym == SDLK_KP_ENTER) {
            bool bDdOpen = false;
            for (int i = 0; i < pForm->iCount; i++) {
                if (pForm->slots[i].eKind == WI_KIND_DROPDOWN) {
                    const WIDropdown *pDd = (const WIDropdown *)pForm->slots[i].pWidget;
                    if (pDd->_bOpen) { bDdOpen = true; break; }
                }
            }
            if (!bDdOpen)
                pForm->bSubmitted = true;
        }
    }

    /* Feed OK / Cancel */
    WIFeedButton(&pForm->btnOK,     ev);
    WIFeedButton(&pForm->btnCancel, ev);

    if (pForm->btnOK.bClicked)     pForm->bSubmitted = true;
    if (pForm->btnCancel.bClicked) pForm->bCancelled = true;
}

void WIFormDraw(SDL_Renderer *ren, const WIForm *pForm)
{
    /* Title and subtitle if set */
    if (pForm->szTitle[0]) {
        WDrawTitle(ren, pForm->winW > 0 ? pForm->winW : 520,
                   pForm->szTitle, pForm->szSubtitle);
    }

    /* Draw non-dropdown widgets first */
    for (int i = 0; i < pForm->iCount; i++) {
        if (pForm->slots[i].eKind != WI_KIND_DROPDOWN)
            slot_draw(ren, &pForm->slots[i]);
    }
    /* Draw OK/Cancel */
    WIDrawButton(ren, &pForm->btnOK);
    WIDrawButton(ren, &pForm->btnCancel);
    /* Dropdowns last (overlay on top of everything) */
    for (int i = 0; i < pForm->iCount; i++) {
        if (pForm->slots[i].eKind == WI_KIND_DROPDOWN)
            slot_draw(ren, &pForm->slots[i]);
    }
}

/* ============================================================
 *  WIFormRunModal
 * ============================================================ */

bool WIFormRunModal(WIForm *pForm, SDL_Window **ppWin, SDL_Renderer **ppRen,
                    const char *pszTitle, int iW, int iH)
{
    if (!SDL_WasInit(SDL_INIT_VIDEO)) {
        if (SDL_Init(SDL_INIT_VIDEO) != 0) {
            fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
            return false;
        }
    }

    pForm->winW = iW;
    pForm->winH = iH;

    bool bOwned = (ppWin == NULL || *ppWin == NULL);
    SDL_Window   *win = NULL;
    SDL_Renderer *ren = NULL;

    if (!bOwned) {
        win = *ppWin;
        ren = *ppRen;
    } else {
        win = SDL_CreateWindow(
            pszTitle ? pszTitle : "Input",
            SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
            iW, iH,
            SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
        if (!win) return false;

        ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
        if (!ren) { SDL_DestroyWindow(win); return false; }

        if (ppWin) *ppWin = win;
        if (ppRen) *ppRen = ren;
    }

    /* Activate text input if initially focused widget is text/spinner */
    if (pForm->iCount > 0 && pForm->iFocused < pForm->iCount) {
        WIKind k = pForm->slots[pForm->iFocused].eKind;
        if (k == WI_KIND_TEXTBOX || k == WI_KIND_SPINNER)
            SDL_StartTextInput();
    }

    SDL_Color colBg = (SDL_Color)CI_BG;
    bool bDone = false;

    while (!bDone) {
        SDL_Event e;
        int mx = 0, my = 0;
        SDL_GetMouseState(&mx, &my);

        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                pForm->bCancelled = true;
            }
            if (e.type == SDL_WINDOWEVENT &&
                e.window.event == SDL_WINDOWEVENT_CLOSE) {
                pForm->bCancelled = true;
            }
            WIFormFeed(pForm, &e, mx, my);
        }

        /* Redraw */
        SDL_SetRenderDrawColor(ren, colBg.r, colBg.g, colBg.b, colBg.a);
        SDL_RenderClear(ren);
        WIFormDraw(ren, pForm);
        SDL_RenderPresent(ren);

        SDL_Delay(16); /* ~60 fps */

        if (pForm->bSubmitted || pForm->bCancelled)
            bDone = true;
    }

    SDL_StopTextInput();

    if (bOwned) {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(win);
        if (ppWin) *ppWin = NULL;
        if (ppRen) *ppRen = NULL;
    }

    return pForm->bSubmitted;
}
