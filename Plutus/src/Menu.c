//
// Created by jan on 10/19/26.
//

/* menu.c - boxed menu TUI
 * Keys: Up/Down or k/j to move, Space to select, q to quit
 */
#include "../inc/Menu.h"
#include "../inc/Ansi.h"

#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>

/* ---------- Palette (named here, built from the ANSI header) ---------- */

#define MENU_BORDER    ANSI_FG_RGB(60, 110, 200)
#define MENU_TITLE     ANSI_BOLD ANSI_FG_RGB(176, 96, 0)
#define MENU_HL_BG     ANSI_BG_RGB(40, 110, 220)
#define MENU_HL_FG     ANSI_FG_RGB(255, 255, 255)
#define MENU_NORMAL    ANSI_FG_DEFAULT
#define MENU_DISABLED  ANSI_FG_RGB(128, 128, 128)

/* ---------- Box drawing characters (UTF-8) ---------- */

#define BOX_H  "─"
#define BOX_V  "│"
#define BOX_TL "┌"
#define BOX_TR "┐"
#define BOX_BL "└"
#define BOX_BR "┘"
#define BOX_LT "├"
#define BOX_RT "┤"

#define BOX_MAX_INNER 60
#define BOX_MIN_INNER 10

/* ---------- Helpers ---------- */

static int term_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0)
        return ws.ws_col;
    return 80;
}

/* Width of the space between the left and right edge characters. */
static int box_inner_width(void)
{
    int w = term_width() - 4;
    if (w > BOX_MAX_INNER) w = BOX_MAX_INNER;
    if (w < BOX_MIN_INNER) w = BOX_MIN_INNER;
    return w;
}

/* Left margin that centers a box of inner width `w`. */
static int box_margin(int w)
{
    int m = (term_width() - (w + 2)) / 2;
    return m < 0 ? 0 : m;
}

static void repeat(const char *s, int n)
{
    while (n-- > 0)
        fputs(s, stdout);
}

static void indent(int n)
{
    if (n > 0)
        printf("%*s", n, "");
}

static void vbar(void)
{
    printf(MENU_BORDER BOX_V ANSI_RESET);
}

/* Top, middle (separator), or bottom border line. */
static void draw_rule(int m, int w, const char *left, const char *fill, const char *right)
{
    indent(m);
    printf(MENU_BORDER "%s", left);
    repeat(fill, w);
    printf("%s" ANSI_RESET "\r\n", right);
}

/* A row of centered text inside the box, clipped to the inner width. */
static void draw_text_row(int m, int w, const char *color, const char *text)
{
    int len = (int)strlen(text);
    if (len > w)
        len = w;
    int left = (w - len) / 2;
    int right = w - len - left;

    indent(m);
    vbar();
    printf("%s%*s%.*s%*s" ANSI_RESET, color, left, "", len, text, right, "");
    vbar();
    printf("\r\n");
}

static int next_selectable(const MenuItem items[], int count, int from, int dir)
{
    for (int i = from + dir; i >= 0 && i < count; i += dir)
        if (items[i].selectable)
            return i;
    return from;
}

static int first_selectable(const MenuItem items[], int count)
{
    for (int i = 0; i < count; i++)
        if (items[i].selectable)
            return i;
    return 0;
}

/* ---------- Draw ---------- */

void DrawMenu(const char *title, const MenuItem items[], int count, int cursor)
{
    int w = box_inner_width();
    int m = box_margin(w);

    /* home, clear, hide cursor, top gap */
    printf(ANSI_CURSOR_HOME ANSI_CLEAR_SCREEN ANSI_CURSOR_HIDE "\r\n");

    /* header: top border, title, rule under title */
    draw_rule(m, w, BOX_TL, BOX_H, BOX_TR);
    draw_text_row(m, w, MENU_TITLE, title ? title : "");
    draw_rule(m, w, BOX_LT, BOX_H, BOX_RT);

    /* body */
    for (int i = 0; i < count; i++) {
        if (items[i].type == MENU_SEPARATOR) {
            draw_rule(m, w, BOX_LT, BOX_H, BOX_RT);
            continue;
        }

        int lim = w - 2;                       /* two spaces of left padding */
        int len = (int)strlen(items[i].label);
        if (len > lim)
            len = lim;
        int pad = w - 2 - len;

        indent(m);
        vbar();
        if (i == cursor) {
            printf(MENU_HL_BG MENU_HL_FG "  %.*s%*s" ANSI_RESET,
                   len, items[i].label, pad, "");
        } else if (items[i].selectable) {
            printf(MENU_NORMAL "  %.*s%*s" ANSI_RESET,
                   len, items[i].label, pad, "");
        } else {
            printf(MENU_DISABLED "  %.*s%*s" ANSI_RESET,
                   len, items[i].label, pad, "");
        }
        vbar();
        printf("\r\n");
    }

    /* footer: rule, hint text, bottom border */
    draw_rule(m, w, BOX_LT, BOX_H, BOX_RT);
    draw_text_row(m, w, MENU_DISABLED, "Up/Down: move  Enter/Space: select  q: quit");
    draw_rule(m, w, BOX_BL, BOX_H, BOX_BR);

    fflush(stdout);
}

/* ---------- Select ---------- */

int MenuSelect(const char *title, const MenuItem items[], int count)
{
    if (count <= 0)
        return -1;

    struct termios orig, raw;
    tcgetattr(STDIN_FILENO, &orig);
    raw = orig;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN]  = 1;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    int cursor = first_selectable(items, count);
    int result = -1;

    for (;;) {
        DrawMenu(title, items, count, cursor);

        unsigned char c;
        if (read(STDIN_FILENO, &c, 1) != 1)
            break;

        if (c == 'q' || c == 'Q') {
            break;
        } else if (c == '\r' || c == '\n' || c == ' ') {
            if (items[cursor].selectable) {
                result = items[cursor].id;
                break;
            }
        } else if (c == 'k') {
            cursor = next_selectable(items, count, cursor, -1);
        } else if (c == 'j') {
            cursor = next_selectable(items, count, cursor, +1);
        } else if (c == 27) {                  /* ESC sequence (arrow keys) */
            char seq[2];
            if (read(STDIN_FILENO, seq, 2) == 2 && seq[0] == '[') {
                if (seq[1] == 'A')
                    cursor = next_selectable(items, count, cursor, -1);
                else if (seq[1] == 'B')
                    cursor = next_selectable(items, count, cursor, +1);
            }
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &orig);
    printf(ANSI_RESET ANSI_CLEAR_SCREEN ANSI_CURSOR_HOME ANSI_CURSOR_SHOW);
    fflush(stdout);

    return result;
}