#ifndef MENU_H
#define MENU_H

typedef struct {
    unsigned int active_index;
} menu_context_t;

void menu_init(menu_context_t *menu);
void menu_render_banner(const menu_context_t *menu);
void menu_render_main(const menu_context_t *menu);

#endif // MENU_H
