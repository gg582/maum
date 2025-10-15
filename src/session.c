#include "session.h"

#include "log.h"
#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#define COMPONENT "session"

struct session_manager {
    unsigned int simulated_sessions;
};

session_manager_t *session_manager_create(void)
{
    session_manager_t *manager = calloc(1, sizeof(*manager));
    if (manager == NULL) {
        return NULL;
    }
    return manager;
}

void session_manager_destroy(session_manager_t *manager)
{
    if (manager == NULL) {
        return;
    }
    free(manager);
}

static void print_lines(const char *const *lines)
{
    if (lines == NULL) {
        return;
    }

    for (size_t i = 0; lines[i] != NULL; ++i) {
        puts(lines[i]);
    }
}

static void print_main_menu(void)
{
    menu_context_t menu;
    menu_init(&menu);
    menu_render_banner(&menu);
    menu_render_main(&menu);
}

static void simulate_telnet_new_user(session_manager_t *manager)
{
    static const char *const pre_menu[] = {
        "",
        "──────────────────────────────────────────────",
        "[TELNET] Session 1: Legacy terminal - new user onboarding",
        "──────────────────────────────────────────────",
        "$ telnet maum.example 2323",
        "Trying 203.0.113.42...",
        "Connected to maum.example.",
        "Escape character is '^]'.",
        "",
        "Maum BBS (마음) - Classic Korean Community",
        "Login (guest/new/handle): new",
        "Enter desired handle: StarSeeker",
        "Set password: ********",
        "Confirm password: ********",
        "Email (optional): starseeker@example.net",
        "Registration complete. 환영합니다, StarSeeker!",
        "",
        "Press ENTER to continue...",
        NULL
    };

    static const char *const post_menu[] = {
        "StarSeeker> 1",
        "",
        "Loading Bulletin Boards...",
        "[1] 자유 게시판 (General)    [Unread: 5]",
        "[2] 기술 토론장 (Tech)      [Unread: 2]",
        "[3] 취미 갤러리 (Hobby)     [Unread: 0]",
        "[4] 자료실 (Files)          [New uploads: 1]",
        "[B] 돌아가기 (Back)",
        "Selection: 2",
        "",
        "┌──────────────────────────────────────────────┐",
        "│  기술 토론장 (Tech)                          │",
        "├──────────────────────────────────────────────┤",
        "│  #42  RISC-V SBC 추천해주세요            Yuna │",
        "│  #41  ANSI Art 그리는 팁                 Jisu │",
        "│  #40  SSH 접속 팁 공유합니다             Neo │",
        "└──────────────────────────────────────────────┘",
        NULL
    };

    LOG_INFO(COMPONENT, "%s", "Simulating TELNET onboarding session");
    manager->simulated_sessions++;

    print_lines(pre_menu);
    print_main_menu();
    print_lines(post_menu);
}

static void simulate_telnet_returning_user(session_manager_t *manager)
{
    static const char *const transcript[] = {
        "",
        "──────────────────────────────────────────────",
        "[TELNET] Session 2: Returning user checks messages",
        "──────────────────────────────────────────────",
        "$ telnet bbs.maum.kr 2323",
        "Trying 198.51.100.77...",
        "Connected to bbs.maum.kr.",
        "Escape character is '^]'.",
        "",
        "Maum BBS (마음) - Login",
        "Handle: Neo",
        "Password: ********",
        "Last login from 198.51.100.202 at 2025-02-01 21:48 KST",
        "You have 2 new private messages.",
        "",
        "Press ENTER to open the main menu...",
        NULL
    };

    static const char *const post_menu[] = {
        "Neo> 4",
        "",
        "┌──────────────────────────────────────────────┐",
        "│  쪽지함 (Private Messages)                   │",
        "├──────────────────────────────────────────────┤",
        "│  * 새 쪽지 *  Jisu → Neo  (오늘 20:11)        │",
        "│  제목: 모임 일정 확인해주세요                │",
        "│                                              │",
        "│  * 새 쪽지 *  StarSeeker → Neo (오늘 19:52)   │",
        "│  제목: 새 ANSI 아트 공유합니다               │",
        "└──────────────────────────────────────────────┘",
        "",
        "Neo> B",
        "Neo> 7",
        "도움말: 메뉴 이동은 번호, 종료는 Q",
        NULL
    };

    LOG_INFO(COMPONENT, "%s", "Simulating TELNET returning user session");
    manager->simulated_sessions++;

    print_lines(transcript);
    print_main_menu();
    print_lines(post_menu);
}

static void simulate_ssh_key_login(session_manager_t *manager)
{
    static const char *const transcript[] = {
        "",
        "──────────────────────────────────────────────",
        "[SSH] Session 3: Key-based login from desktop",
        "──────────────────────────────────────────────",
        "$ ssh -p 2222 yuna@maum.example",
        "Authenticating with public key "
        "'~/.ssh/id_ed25519'",
        "Last login: Sat Feb  8 10:14:03 2025 from 203.0.113.74",
        "──────────────────────────────────────────────",
        "📜 오늘의 공지",
        " - 정기 시스템 점검: 2월 10일 02:00-04:00 KST",
        " - 신규 ANSI 아트 콘테스트: 제출 마감 2월 말",
        NULL
    };

    static const char *const post_menu[] = {
        "yuna@maum ~> open motd",
        "──────────────────────────────────────────────",
        "마음 BBS에 오신 것을 환영합니다!",
        "오늘의 추천 게시판: 취미 갤러리",
        "──────────────────────────────────────────────",
        "yuna@maum ~> bbs",
        NULL
    };

    LOG_INFO(COMPONENT, "%s", "Simulating SSH key login session");
    manager->simulated_sessions++;

    print_lines(transcript);
    print_main_menu();
    print_lines(post_menu);
}

static void simulate_ssh_mobile_user(session_manager_t *manager)
{
    static const char *const transcript[] = {
        "",
        "──────────────────────────────────────────────",
        "[SSH] Session 4: Mobile user joins real-time chat",
        "──────────────────────────────────────────────",
        "$ ssh -p 2222 -o PreferredAuthentications=password guest@maum.example",
        "guest@maum.example's password: ********",
        "Welcome back, guest! (mobile mode enabled)",
        "",
        "Type 'menu' to return here at any time.",
        "Type 'chat' to join the real-time lounge.",
        "",
        "guest@maum ~> chat",
        "Connecting to 대화방 (Lounge)...",
        "[20:35] Neo: 오늘 밤 ANSI 파티 참여하실 분?",
        "[20:35] Yuna: 저는 가능해요!",
        "[20:36] guest: 모바일에서도 잘 보여요 :)",
        "[20:37] StarSeeker: 새 파일 업로드했습니다!",
        NULL
    };

    LOG_INFO(COMPONENT, "%s", "Simulating SSH mobile chat session");
    manager->simulated_sessions++;

    print_lines(transcript);
}

void session_manager_simulate(session_manager_t *manager)
{
    if (manager == NULL) {
        return;
    }

    LOG_INFO(COMPONENT, "%s", "Simulating interactive sessions");

    simulate_telnet_new_user(manager);
    simulate_telnet_returning_user(manager);
    simulate_ssh_key_login(manager);
    simulate_ssh_mobile_user(manager);

    LOG_INFO(COMPONENT, "Simulated sessions: %u", manager->simulated_sessions);
}
