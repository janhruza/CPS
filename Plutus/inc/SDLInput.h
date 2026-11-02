//
// Created by jan on 11/02/26.
// SDLInput.h  –  SDL2 interactive input widget toolkit for Plutus chart windows.
//
// PURPOSE
//   Provides self-contained, stateful input widgets (button, text box, checkbox,
//   slider, number spinner, dropdown) that can replace the TUI parameter-entry
//   phase for SDL chart micro-apps.  All widgets are rendered with the same dark
//   design language as SDLWidgets.h and use the embedded bitmap font from Font.h.
//
// USAGE MODEL (event-driven, retained-mode)
//   1. Declare widget state structs on the stack or in a containing struct.
//   2. Call the appropriate WI_Init_* function once to configure each widget.
//   3. Each frame:
//      a. Call WIPollEvents()  — feeds raw SDL events into every widget.
//      b. Call WIDraw_*()      — renders each widget with its current state.
//   4. Read results via the fields of the widget state struct (value, text, etc.)
//      or with the provided accessor helpers.
//   5. Check WIFormDone() to know when the user has submitted the form.
//
// PORTABILITY
//   Pure SDL2 + C23.  No platform-specific code; no runtime font assets.
//

#ifndef PLUTUS_SDLINPUT_H
#define PLUTUS_SDLINPUT_H

#include <stdbool.h>
#include <stddef.h>
#include <SDL.h>

#include "Font.h"
#include "SDLWidgets.h"

/* ============================================================
 *  Shared widget state flags
 * ============================================================ */

typedef enum {
    WI_STATE_NORMAL   = 0,   /* default resting state */
    WI_STATE_HOVER    = 1,   /* mouse is over the widget */
    WI_STATE_FOCUSED  = 2,   /* keyboard focus (Tab to cycle) */
    WI_STATE_PRESSED  = 4,   /* mouse button held down */
    WI_STATE_DISABLED = 8    /* widget ignores input, rendered dimmed */
} WIState;

/* ============================================================
 *  Event context  (pass SDL_Event + mouse pos to every WI call)
 * ============================================================ */

typedef struct tagWIEvent {
    const SDL_Event *pEvent;  /* raw SDL event (may be NULL for draw-only calls) */
    int              mouseX;
    int              mouseY;
} WIEvent;

/* ============================================================
 *  Button
 * ============================================================ */

#define WI_BUTTON_LABEL_MAX 64

typedef struct tagWIButton {
    SDL_Rect    rect;                       /* bounding box in window pixels    */
    char        szLabel[WI_BUTTON_LABEL_MAX];
    SDL_Color   colAccent;                  /* border / fill accent colour      */
    WIState     eState;
    bool        bClicked;                   /* true for exactly one frame after click */

    /* private */
    bool        _bPressedPrev;
} WIButton;

/* Initialise a button.  rect is the pixel bounding box. */
void WIInitButton(WIButton *pBtn, SDL_Rect rect,
                  const char *pszLabel, SDL_Color colAccent);

/* Process one SDL event.  Updates bClicked, eState. */
void WIFeedButton(WIButton *pBtn, WIEvent ev);

/* Draw the button into ren. */
void WIDrawButton(SDL_Renderer *ren, const WIButton *pBtn);

/* ============================================================
 *  Text Box  (single-line UTF-8 input)
 * ============================================================ */

#define WI_TEXTBOX_CAP 256   /* maximum codepoints stored */

typedef struct tagWITextBox {
    SDL_Rect rect;
    char     szLabel[64];                   /* optional label drawn above the box */
    char     szPlaceholder[WI_TEXTBOX_CAP]; /* shown when buffer is empty         */
    char     szText[WI_TEXTBOX_CAP];        /* current user text (UTF-8)          */
    int      iCursor;                       /* byte offset of cursor              */
    WIState  eState;
    bool     bChanged;                      /* true for one frame after any edit  */

    /* private */
    Uint32   _blinkTick;
    bool     _bCursorVisible;
} WITextBox;

void WIInitTextBox(WITextBox *pTb, SDL_Rect rect,
                   const char *pszLabel, const char *pszPlaceholder);

void WIFeedTextBox(WITextBox *pTb, WIEvent ev);
void WIDrawTextBox(SDL_Renderer *ren, const WITextBox *pTb);

/* Clear content and reset cursor. */
void WIClearTextBox(WITextBox *pTb);

/* ============================================================
 *  Number Spinner
 *  Like a text box but constrained to a numeric range.
 *  Includes + / – step buttons on the right.
 * ============================================================ */

typedef struct tagWISpinner {
    SDL_Rect rect;
    char     szLabel[64];
    double   dValue;
    double   dMin;
    double   dMax;
    double   dStep;          /* increment per button click                    */
    int      iDecimals;      /* decimal places shown and accepted             */
    WIState  eState;
    bool     bChanged;

    /* private text box for direct keyboard editing */
    WITextBox _tb;
    bool      _bEditMode;
    WIButton  _btnPlus;
    WIButton  _btnMinus;
} WISpinner;

void WIInitSpinner(WISpinner *pSp, SDL_Rect rect, const char *pszLabel,
                   double dInitial, double dMin, double dMax,
                   double dStep, int iDecimals);

void WIFeedSpinner(WISpinner *pSp, WIEvent ev);
void WIDrawSpinner(SDL_Renderer *ren, const WISpinner *pSp);

/* ============================================================
 *  Checkbox / Toggle
 * ============================================================ */

typedef struct tagWICheckbox {
    SDL_Rect rect;
    char     szLabel[128];
    bool     bChecked;
    WIState  eState;
    bool     bChanged;       /* true for one frame after toggle */
} WICheckbox;

void WIInitCheckbox(WICheckbox *pCb, SDL_Rect rect,
                    const char *pszLabel, bool bInitial);

void WIFeedCheckbox(WICheckbox *pCb, WIEvent ev);
void WIDrawCheckbox(SDL_Renderer *ren, const WICheckbox *pCb);

/* ============================================================
 *  Horizontal Slider
 * ============================================================ */

typedef struct tagWISlider {
    SDL_Rect rect;
    char     szLabel[64];
    double   dValue;         /* current value in [dMin, dMax]   */
    double   dMin;
    double   dMax;
    int      iDecimals;      /* decimal places in value label    */
    SDL_Color colFill;       /* filled-track accent colour       */
    WIState  eState;
    bool     bChanged;

    /* private */
    bool _bDragging;
} WISlider;

void WIInitSlider(WISlider *pSl, SDL_Rect rect, const char *pszLabel,
                  double dInitial, double dMin, double dMax,
                  int iDecimals, SDL_Color colFill);

void WIFeedSlider(WISlider *pSl, WIEvent ev);
void WIDrawSlider(SDL_Renderer *ren, const WISlider *pSl);

/* ============================================================
 *  Dropdown / Select
 * ============================================================ */

#define WI_DROPDOWN_MAX_OPTIONS 16
#define WI_DROPDOWN_OPTION_CAP  64

typedef struct tagWIDropdown {
    SDL_Rect rect;
    char     szLabel[64];
    char     szOptions[WI_DROPDOWN_MAX_OPTIONS][WI_DROPDOWN_OPTION_CAP];
    int      iOptionCount;
    int      iSelected;      /* index of the selected option    */
    WIState  eState;
    bool     bChanged;

    /* private */
    bool     _bOpen;         /* true while the dropdown list is expanded */
    int      _iHovered;      /* hovered option index when open (-1 = none) */
} WIDropdown;

void WIInitDropdown(WIDropdown *pDd, SDL_Rect rect, const char *pszLabel,
                    const char * const *ppOptions, int iCount, int iInitial);

void WIFeedDropdown(WIDropdown *pDd, WIEvent ev);
void WIDrawDropdown(SDL_Renderer *ren, const WIDropdown *pDd);

/* ============================================================
 *  Form helpers
 *  A "form" is just a named group of widgets managed together.
 *  These helpers handle Tab/Shift-Tab focus cycling, OK/Cancel
 *  buttons, and a top-level event dispatch.
 * ============================================================ */

/* Maximum widgets in a single form */
#define WI_FORM_MAX_WIDGETS 32

/* Widget type tag — used by the form dispatcher */
typedef enum {
    WI_KIND_BUTTON,
    WI_KIND_TEXTBOX,
    WI_KIND_SPINNER,
    WI_KIND_CHECKBOX,
    WI_KIND_SLIDER,
    WI_KIND_DROPDOWN
} WIKind;

/* A generic widget slot: kind + pointer to the concrete struct */
typedef struct tagWISlot {
    WIKind  eKind;
    void   *pWidget;   /* points to WIButton, WITextBox, etc. */
    int     iTabIndex; /* 0 = not focusable; >0 = Tab order   */
} WISlot;

typedef struct tagWIForm {
    char   szTitle[64];
    char   szSubtitle[96];
    int    winW;
    int    winH;
    WISlot slots[WI_FORM_MAX_WIDGETS];
    int    iCount;
    int    iFocused;   /* index into slots[] of the focused widget */

    /* Standard OK / Cancel buttons — populated by WIFormInit */
    WIButton btnOK;
    WIButton btnCancel;

    bool bSubmitted;   /* true when the user clicks OK or presses Enter */
    bool bCancelled;   /* true when the user clicks Cancel or presses ESC */
} WIForm;

/*
 * Initialise a form and position OK/Cancel buttons at the bottom of
 * the given window area (winW × winH).
 */
void WIFormInit(WIForm *pForm, int winW, int winH);

/* Set an optional title and subtitle rendered at the top of the modal form */
void WIFormSetHeader(WIForm *pForm, const char *pszTitle, const char *pszSubtitle);

/* Register a widget with the form so it receives focus and Tab cycling. */
void WIFormAdd(WIForm *pForm, WIKind eKind, void *pWidget, int iTabIndex);

/*
 * Feed one SDL event to all widgets in the form, handle Tab focus cycling,
 * and update bSubmitted / bCancelled.
 */
void WIFormFeed(WIForm *pForm, const SDL_Event *pEvent, int mouseX, int mouseY);

/* Draw all registered widgets and the OK/Cancel buttons. */
void WIFormDraw(SDL_Renderer *ren, const WIForm *pForm);

/*
 * Run a modal form loop: opens a new SDL window (or uses the provided ren),
 * renders WIFormDraw each frame, processes events via WIFormFeed, and returns
 * true when submitted (OK) or false when cancelled (ESC/Cancel).
 *
 * If ppWin / ppRen are NULL the function creates its own temporary window.
 */
bool WIFormRunModal(WIForm *pForm, SDL_Window **ppWin, SDL_Renderer **ppRen,
                    const char *pszTitle, int iW, int iH);

#endif /* PLUTUS_SDLINPUT_H */

