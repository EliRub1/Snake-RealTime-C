#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <stdlib.h>

#define COLS 80
#define ROWS 25
#define SCREEN_SIZE (COLS * ROWS)

// הגדרת משתני צבע והגדרות למשחק נחש אורך משחק וכו
#define ATTR_BG 0x1F
#define ATTR_SNAKE 0x5D
#define ATTR_SNAKE_HEAD 0x4F
#define ATTR_TARGET 0x2A     
#define ATTR_SHIELD 0x6E     
#define MAX_SNAKE 200
#define INIT_LEN 6
#define APPLE_WIDTH 3
#define APPLE_HEIGHT 2
#define WALL_HEIGHT 5
#define MESSAGE_DURATION 45
#define WIN_SCORE 12

// פונקציות ***
void putch_attr(unsigned int x, unsigned int y, unsigned char ch, unsigned char attr);
void clear_screen(void);
void draw_background(void);
void draw_hud(void);
void draw_snake(void);
void draw_target(void);
void draw_v_wall(int base_x, int base_y, int mirror);
void move_snake(void);
void make_beep(int ms);
void game_over_screen(void);
void init_game(void);
void spawn_target(void);
void update_level(void);
unsigned char get_text_attr(void);

// וקטורים
void interrupt far newInt8(void);
void interrupt far newInt9(void);
void interrupt far (*oldInt8)(void);
void interrupt far (*oldInt9)(void);


unsigned char far *screen = (unsigned char far *)0xB8000000L;

// משתני משחק 
int snake_x[MAX_SNAKE], snake_y[MAX_SNAKE];
int snake_len;
int dir;
int score;
volatile int remaining_time;
volatile int game_over_flag;
volatile int tick_cnt;
int target_x, target_y, target_w;
int shield_side;
int shield_mirrored;
volatile int bonus_timer = 0;
volatile int level_timer = 0;
volatile int speed_msg_timer = 0;
int last_speed_percent = 0;
int current_level = 1;
volatile int last_scan = 0;
int sched_point = 0;
int cycle_len = 12;

// הצבעים לשלבים
unsigned char get_text_attr() {
    switch (current_level) {
        case 1: return 0x1F;
        case 2: return 0x6F;
        case 3: return 0x2F;
        case 4: return 0x0F;
        default: return 0x1F;
    }
}
void putch_attr(unsigned int x, unsigned int y, unsigned char ch, unsigned char attr) {
    unsigned int off = (y * COLS + x) * 2;
    if (x >= 0 && x < COLS && y >= 0 && y < ROWS) {
        screen[off] = ch;
        screen[off + 1] = attr;
    }
}

void clear_screen(void) {
    unsigned int i;
    for (i = 0; i < SCREEN_SIZE; i++) {
        screen[i * 2] = ' ';
        screen[i * 2 + 1] = get_text_attr();
    }
}

void draw_background(void) {
    clear_screen();
}

// התצוגה עם הזמן והניקוד
void draw_hud(void) {
    char buf[50];
    int i;

    sprintf(buf, "Time:%3d Score:%3d Len:%2d", remaining_time, score, snake_len);
    for (i = 0; buf[i] != 0 && i < COLS; i++)
        putch_attr(i, 0, buf[i], get_text_attr());

    if (bonus_timer > 0) {
        putch_attr(8, 0, '+', ATTR_TARGET);
        putch_attr(9, 0, '5', ATTR_TARGET);
    }

    if (level_timer > 0) {
        char level_buf[20];
        sprintf(level_buf, "LEVEL %d", current_level);
        for (i = 0; level_buf[i]; i++)
            putch_attr(i, 1, level_buf[i], get_text_attr());

        if (speed_msg_timer > 0) {
            char speed_buf[20];
            sprintf(speed_buf, "+speed %d%%", last_speed_percent);
            for (i = 0; speed_buf[i]; i++)
                putch_attr(i, 2, speed_buf[i], ATTR_TARGET);
        }
    }
}

// הבנייה של הנחש עם הצבע והמספרים
void draw_snake(void) {
    int i;
    for (i = 0; i < snake_len; i++) {
        if (i == 0)
            putch_attr(snake_x[i], snake_y[i], '>', ATTR_SNAKE_HEAD);
        else {
            char ch = (i < 10) ? ('0' + i) : '*';
            unsigned char attr = (i % 2 == 0) ? 0x4F : 0x0C;
            putch_attr(snake_x[i], snake_y[i], ch, attr);
        }
    }
}

//בניית החומה כמו ראש חץ 
void draw_v_wall(int base_x, int base_y, int mirror) {
    int i;
    if (!mirror) {
        for (i = 0; i < WALL_HEIGHT; i++)
            putch_attr(base_x + i, base_y + i, 'v', ATTR_SHIELD);
        for (i = WALL_HEIGHT - 2; i >= 0; i--)
            putch_attr(base_x + (WALL_HEIGHT - 1 - i), base_y + WALL_HEIGHT - 1 + i, 'v', ATTR_SHIELD);
    } else {
        for (i = 0; i < WALL_HEIGHT; i++)
            putch_attr(base_x - i, base_y + i, 'v', ATTR_SHIELD);
        for (i = WALL_HEIGHT - 2; i >= 0; i--)
            putch_attr(base_x - (WALL_HEIGHT - 1 - i), base_y + WALL_HEIGHT - 1 + i, 'v', ATTR_SHIELD);
    }
}

void draw_target(void) {
    int dx, dy;
    for (dy = 0; dy < APPLE_HEIGHT; dy++)
        for (dx = 0; dx < APPLE_WIDTH; dx++)
            putch_attr(target_x + dx, target_y + dy, '*', ATTR_TARGET);

    if (shield_side == 1)
        draw_v_wall(target_x - 2, target_y - 2, shield_mirrored);
    else if (shield_side == 2)
        draw_v_wall(target_x + APPLE_WIDTH + 1, target_y - 2, shield_mirrored);
}

void spawn_target(void) {
    int valid = 0, dx, dy;
    target_w = APPLE_WIDTH;
    while (!valid) {
        target_x = 5 + rand() % (COLS - 15);
        target_y = 3 + rand() % (ROWS - 5);
        dx = abs(target_x - snake_x[0]);
        dy = abs(target_y - snake_y[0]);
        if (dx + dy > 10) valid = 1;
    }

    if (snake_x[0] < target_x) {
        shield_side = 1;
        shield_mirrored = 1;
    } else {
        shield_side = 2;
        shield_mirrored = 0;
    }
}
void update_level(void) {
    int prev_cycle = cycle_len;
    int new_level = current_level;

    if (score >= 10 && current_level < 4) {
        new_level = 4; cycle_len = 6;
    } else if (score >= 6 && current_level < 3) {
        new_level = 3; cycle_len = 8;
    } else if (score >= 3 && current_level < 2) {
        new_level = 2; cycle_len = 10;
    }

    if (new_level != current_level) {
        int percent = 100 - (cycle_len * 100 / prev_cycle);
        last_speed_percent = percent;
        speed_msg_timer = MESSAGE_DURATION;
        level_timer = MESSAGE_DURATION;
        current_level = new_level;
    }
}
void move_snake(void) {
    int i;
    int newx = snake_x[0];
    int newy = snake_y[0];
    unsigned int off;

    if (dir == 0) newy--;
    else if (dir == 1) newx++;
    else if (dir == 2) newy++;
    else if (dir == 3) newx--;

    // בדיקה גבולות מסך
    if (newx < 0 || newx >= COLS || newy < 1 || newy >= ROWS) {
        game_over_flag = 1;
        return;
    }

    // בדיקה התנגשות עם הנחש עצמו
    for (i = 0; i < snake_len; i++) {
        if (snake_x[i] == newx && snake_y[i] == newy) {
            game_over_flag = 1;
            return;
        }
    }

    // בדיקה התנגשות עם החומה 
    off = (newy * COLS + newx) * 2;
    if (screen[off] == 'v') {
        game_over_flag = 1;
        return;
    }

    // הזזת הגוף
    for (i = snake_len - 1; i > 0; i--) {
        snake_x[i] = snake_x[i - 1];
        snake_y[i] = snake_y[i - 1];
    }
    snake_x[0] = newx;
    snake_y[0] = newy;

    // בדיקה אם אכל את התפוח
    if (newy >= target_y && newy < target_y + APPLE_HEIGHT &&
        newx >= target_x && newx < target_x + APPLE_WIDTH) {
        score++;
        if (snake_len < MAX_SNAKE - 1) {
            snake_x[snake_len] = snake_x[snake_len - 1];
            snake_y[snake_len] = snake_y[snake_len - 1];
            snake_len++;
        }
        make_beep(120);
        remaining_time += 5;
        bonus_timer = MESSAGE_DURATION;
        spawn_target();
        update_level();

        // תנאי ניצחון
        if (score >= WIN_SCORE) {
            game_over_flag = 2;
        }
    }
}
// רעש משתנה לפי ניקוד
void make_beep(int ms) {
    sound(800 + (score * 50));
    delay(ms);
    nosound();
}
//מסך הפסד לפי שלבים
void game_over_screen(void) {
    char buf[40];
    int cx = COLS / 2 - 10, cy = ROWS / 2, i;
    sprintf(buf, "GAME OVER Score: %d", score);
    for (i = 0; buf[i]; i++) {
        putch_attr(cx + i, cy, buf[i], get_text_attr());
    }
}

void init_game(void) {
    int i;
    snake_len = INIT_LEN;
    dir = 1;
    for (i = 0; i < snake_len; i++) {
        snake_x[i] = 5 - i;
        snake_y[i] = 5;
    }
    score = 0;
    remaining_time = 60;
    game_over_flag = 0;
    tick_cnt = 0;
    current_level = 1;
    cycle_len = 12;
    bonus_timer = 0;
    level_timer = 0;
    speed_msg_timer = 0;
    last_speed_percent = 0;
    spawn_target();
}

// ISR למקלדת
void interrupt far newInt9(void) {
    unsigned char sc = inp(0x60);

    if (sc == 0x48 && dir != 2) dir = 0; // Up
    else if (sc == 0x4D && dir != 3) dir = 1; // Right
    else if (sc == 0x50 && dir != 0) dir = 2; // Down
    else if (sc == 0x4B && dir != 1) dir = 3; // Left
    else if (sc == 0x01) game_over_flag = 1; // ESC - יציאה

    last_scan = sc;
    outp(0x20, 0x20); 
}

// ISR טיימר וחישובי תנועה וזמנים
void interrupt far newInt8(void) {
    tick_cnt++;
    if (tick_cnt >= 18) { // אמור להספיק
        tick_cnt = 0;
        if (remaining_time > 0)
            remaining_time--;
    }
    if (bonus_timer > 0) bonus_timer--;
    if (level_timer > 0) level_timer--;
    if (speed_msg_timer > 0) speed_msg_timer--;

    sched_point++;
    if (sched_point >= cycle_len) sched_point = 0;

    if (sched_point % 4 == 0) move_snake();

    if (sched_point % 3 == 0) {
        draw_background();
        draw_hud();
        draw_target();
        draw_snake();
    }
    (*oldInt8)();
}

void main(void) {
    char buf[40];
    int cx, cy, k;

    oldInt8 = getvect(8);
    oldInt9 = getvect(9);
    setvect(8, newInt8);
    setvect(9, newInt9);

    draw_background();
    init_game();
    draw_hud();
    draw_target();
    draw_snake();

    while (!game_over_flag && remaining_time > 0)
        delay(10);

    clear_screen();
    cx = COLS / 2 - 10;
    cy = ROWS / 2;

    if (game_over_flag == 2) {
        // רקע ירוק בניצחון
        unsigned int i;
        for (i = 0; i < SCREEN_SIZE; i++) {
            screen[i * 2 + 1] = 0x2A; 
        }

        sprintf(buf, "YOU WIN!! Score: %d", score);
        for (k = 0; buf[k]; k++)
            putch_attr(cx + k, cy, buf[k], 0x2A);
    } else {
        // רקע מתאים לשלב בשאר המסכים
        clear_screen();
        sprintf(buf, "GAME OVER Score: %d", score);
        for (k = 0; buf[k]; k++)
            putch_attr(cx + k, cy, buf[k], get_text_attr());
    }

    setvect(8, oldInt8);
    setvect(9, oldInt9);

    gotoxy(1, ROWS);
    printf("\nPress any key to exit...");
    getch();
}
