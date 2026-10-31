//
// Created by jan on 10/31/26.
//

#ifndef PLUTUS_ANSI_H
#define PLUTUS_ANSI_H

/* ANSI escape codes for terminal output.
   Each macro is a string literal, so codes can be joined with adjacent strings:
   printf(ANSI_BOLD ANSI_FG_RED "error" ANSI_RESET "\n"); */

#define ANSI_ESC "\x1b"
#define ANSI_CSI "\x1b["

/* ---------- Reset ---------- */
#define ANSI_RESET          "\x1b[0m"

/* ---------- Text attributes (set) ---------- */
#define ANSI_BOLD           "\x1b[1m"
#define ANSI_DIM            "\x1b[2m"      /* faint, not supported everywhere */
#define ANSI_ITALIC         "\x1b[3m"      /* not supported everywhere */
#define ANSI_UNDERLINE      "\x1b[4m"
#define ANSI_BLINK          "\x1b[5m"      /* slow blink */
#define ANSI_BLINK_FAST     "\x1b[6m"      /* rapid blink, rarely supported */
#define ANSI_INVERSE        "\x1b[7m"      /* swaps foreground and background */
#define ANSI_HIDDEN         "\x1b[8m"      /* conceal text */
#define ANSI_STRIKETHROUGH  "\x1b[9m"

/* ---------- Text attributes (reset individually) ---------- */
#define ANSI_NO_BOLD_DIM    "\x1b[22m"     /* turns off bold and dim */
#define ANSI_NO_ITALIC      "\x1b[23m"
#define ANSI_NO_UNDERLINE   "\x1b[24m"
#define ANSI_NO_BLINK       "\x1b[25m"
#define ANSI_NO_INVERSE     "\x1b[27m"
#define ANSI_NO_HIDDEN      "\x1b[28m"
#define ANSI_NO_STRIKE      "\x1b[29m"

/* ---------- Standard foreground colors (30-37) ---------- */
#define ANSI_FG_BLACK       "\x1b[30m"
#define ANSI_FG_RED         "\x1b[31m"
#define ANSI_FG_GREEN       "\x1b[32m"
#define ANSI_FG_YELLOW      "\x1b[33m"
#define ANSI_FG_BLUE        "\x1b[34m"
#define ANSI_FG_MAGENTA     "\x1b[35m"
#define ANSI_FG_CYAN        "\x1b[36m"
#define ANSI_FG_WHITE       "\x1b[37m"
#define ANSI_FG_DEFAULT     "\x1b[39m"

/* ---------- Standard background colors (40-47) ---------- */
#define ANSI_BG_BLACK       "\x1b[40m"
#define ANSI_BG_RED         "\x1b[41m"
#define ANSI_BG_GREEN       "\x1b[42m"
#define ANSI_BG_YELLOW      "\x1b[43m"
#define ANSI_BG_BLUE        "\x1b[44m"
#define ANSI_BG_MAGENTA     "\x1b[45m"
#define ANSI_BG_CYAN        "\x1b[46m"
#define ANSI_BG_WHITE       "\x1b[47m"
#define ANSI_BG_DEFAULT     "\x1b[49m"

/* ---------- Bright foreground colors (90-97) ---------- */
#define ANSI_FG_BRIGHT_BLACK    "\x1b[90m"   /* usually gray */
#define ANSI_FG_BRIGHT_RED      "\x1b[91m"
#define ANSI_FG_BRIGHT_GREEN    "\x1b[92m"
#define ANSI_FG_BRIGHT_YELLOW   "\x1b[93m"
#define ANSI_FG_BRIGHT_BLUE     "\x1b[94m"
#define ANSI_FG_BRIGHT_MAGENTA  "\x1b[95m"
#define ANSI_FG_BRIGHT_CYAN     "\x1b[96m"
#define ANSI_FG_BRIGHT_WHITE    "\x1b[97m"

/* ---------- Bright background colors (100-107) ---------- */
#define ANSI_BG_BRIGHT_BLACK    "\x1b[100m"
#define ANSI_BG_BRIGHT_RED      "\x1b[101m"
#define ANSI_BG_BRIGHT_GREEN    "\x1b[102m"
#define ANSI_BG_BRIGHT_YELLOW   "\x1b[103m"
#define ANSI_BG_BRIGHT_BLUE     "\x1b[104m"
#define ANSI_BG_BRIGHT_MAGENTA  "\x1b[105m"
#define ANSI_BG_BRIGHT_CYAN     "\x1b[106m"
#define ANSI_BG_BRIGHT_WHITE    "\x1b[107m"

/* ---------- 256-color and truecolor (use with a number) ----------
   Build with a format string, for example:
   printf(ANSI_FG_256(208) "orange" ANSI_RESET);
   printf(ANSI_FG_RGB(255, 128, 0) "orange" ANSI_RESET); */
#define ANSI_FG_256(n)          "\x1b[38;5;" #n "m"
#define ANSI_BG_256(n)          "\x1b[48;5;" #n "m"
#define ANSI_FG_RGB(r, g, b)    "\x1b[38;2;" #r ";" #g ";" #b "m"
#define ANSI_BG_RGB(r, g, b)    "\x1b[48;2;" #r ";" #g ";" #b "m"

/* ---------- Cursor movement ---------- */
#define ANSI_CURSOR_HOME        "\x1b[H"          /* row 1, column 1 */
#define ANSI_CURSOR_UP(n)       "\x1b[" #n "A"
#define ANSI_CURSOR_DOWN(n)     "\x1b[" #n "B"
#define ANSI_CURSOR_FORWARD(n)  "\x1b[" #n "C"
#define ANSI_CURSOR_BACK(n)     "\x1b[" #n "D"
#define ANSI_CURSOR_POS(r, c)   "\x1b[" #r ";" #c "H"
#define ANSI_CURSOR_COL(n)      "\x1b[" #n "G"
#define ANSI_CURSOR_SAVE        "\x1b[s"
#define ANSI_CURSOR_RESTORE     "\x1b[u"
#define ANSI_CURSOR_HIDE        "\x1b[?25l"
#define ANSI_CURSOR_SHOW        "\x1b[?25h"

/* ---------- Erasing ---------- */
#define ANSI_ERASE_LINE         "\x1b[2K"         /* whole line */
#define ANSI_ERASE_TO_EOL       "\x1b[0K"         /* cursor to end of line */
#define ANSI_ERASE_TO_BOL       "\x1b[1K"         /* start of line to cursor */
#define ANSI_CLEAR_SCREEN       "\x1b[2J"
#define ANSI_CLEAR_TO_EOS       "\x1b[0J"         /* cursor to end of screen */
#define ANSI_CLEAR_SCROLLBACK   "\x1b[3J"

/* ---------- Alternate screen (full-screen apps) ---------- */
#define ANSI_ALT_SCREEN_ON      "\x1b[?1049h"
#define ANSI_ALT_SCREEN_OFF     "\x1b[?1049l"

#endif //PLUTUS_ANSI_H