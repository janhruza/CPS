//
// Created by jan on 10/19/26.
//

#ifndef PLUTUS_MENU_H
#define PLUTUS_MENU_H

typedef enum {
    MENU_ITEM,       /* normal entry: selectable or dimmed */
    MENU_SEPARATOR   /* horizontal rule spanning the box */
} MenuItemType;

typedef struct tagMenuItem {
    int          id;          /* returned by MenuSelect() when chosen */
    const char  *label;       /* unused for separators */
    int          selectable;  /* 0 = shown dimmed, cursor skips it */
    MenuItemType type;        /* MENU_ITEM (default) or MENU_SEPARATOR */
} MenuItem;

/* Use inside an item array to insert a separator line. */
#define MENU_SEPARATOR_ITEM  { 0, NULL, 0, MENU_SEPARATOR }

/* Draws the boxed menu with a title, items, and the cursor on index `cursor`. */
void DrawMenu(const char *title, const MenuItem items[], int count, int cursor);

/* Runs the interactive menu. Returns the id of the item chosen with Space,
 * or -1 if the user quit with 'q'. */
int MenuSelect(const char *title, const MenuItem items[], int count);

/* Convenience macro: pass a fixed-size array without counting it. */
#define MENU_SELECT(title, arr) \
    MenuSelect((title), (arr), (int)(sizeof(arr) / sizeof((arr)[0])))


#endif //PLUTUS_MENU_H
