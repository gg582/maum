#include "menu.h"

#include <stdio.h>

void menu_init(menu_context_t *menu)
{
    if (menu == NULL) {
        return;
    }
    menu->active_index = 0;
}

void menu_render_banner(const menu_context_t *menu)
{
    (void)menu;
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                   마음 (Maum) BBS                          ║\n");
    printf("║              Classic Korean BBS Revival                     ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
}

void menu_render_main(const menu_context_t *menu)
{
    (void)menu;
    printf("║  [1] Bulletin Boards (게시판)                               ║\n");
    printf("║  [2] File Libraries (자료실)                                ║\n");
    printf("║  [3] Chat Rooms (대화방)                                    ║\n");
    printf("║  [4] Private Messages (쪽지함)                              ║\n");
    printf("║  [5] User Profile (내 정보)                                 ║\n");
    printf("║  [6] System Info (시스템 정보)                              ║\n");
    printf("║  [7] Help (도움말)                                          ║\n");
    printf("║  [Q] Quit (종료)                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\nSelect [1-7, Q]: _\n");
}
