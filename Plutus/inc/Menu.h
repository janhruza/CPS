//
// Created by jan on 10/19/26.
//

#ifndef PLUTUS_MENU_H
#define PLUTUS_MENU_H

typedef enum {
    /* A standard menu row. When selectable == 1 the cursor can land on it
       and Enter/Space returns its id. When selectable == 0 it renders dimmed
       and the cursor skips it (legacy "disabled" behaviour). */
    MENU_ITEM,

    /* A thin horizontal rule (├────┤). label is ignored. */
    MENU_SEPARATOR,

    /* A non-selectable section heading rendered with a coloured accent and
       decorating rule. Use to group related items inside the same menu.
       The label is drawn in a distinctive style; cursor always skips it. */
    MENU_HEADING,

    /* A non-selectable annotation row shown in a faint italic style.
       Intended for short hints or status lines beneath a heading or item.
       The cursor always skips it. */
    MENU_NOTE
} MenuItemType;

typedef struct tagMenuItem {
    int          id;          /* returned by MenuSelect() when chosen */
    const char  *label;       /* text to display; unused for separators */
    int          selectable;  /* 0 = cursor skips this row */
    MenuItemType type;        /* controls rendering style */
} MenuItem;

/* Convenience constructors for non-item rows */
#define MENU_SEPARATOR_ITEM         { 0, NULL,  0, MENU_SEPARATOR }
#define MENU_HEADING_ITEM(label)    { 0, (label), 0, MENU_HEADING }
#define MENU_NOTE_ITEM(label)       { 0, (label), 0, MENU_NOTE    }

/* Draws the boxed menu with a title, items, and the cursor on index `cursor`. */
void DrawMenu(const char *title, const MenuItem items[], int count, int cursor);

/* Runs the interactive menu. Returns the id of the item chosen with Space,
 * or -1 if the user quit with 'q'. */
int MenuSelect(const char *title, const MenuItem items[], int count);

/* Convenience macro: pass a fixed-size array without counting it. */
#define MENU_SELECT(title, arr) \
    MenuSelect((title), (arr), (int)(sizeof(arr) / sizeof((arr)[0])))


#endif //PLUTUS_MENU_H
