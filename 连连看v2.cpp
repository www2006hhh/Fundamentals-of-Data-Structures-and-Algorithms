#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ROW 8
#define COL 10
#define BLOCK_SIZE 50
#define INFO_HEIGHT 50  // 专门留给操作说明的高度

int map[ROW][COL];
const char* patterns[] = { "1","2","3","4","5","6","7","8" };
int selected_r = -1, selected_c = -1;

// 撤销用栈
typedef struct {
    int r1, c1;
    int r2, c2;
    int val1, val2;
} Step;

Step stack[1000];
int top = -1;

int hint_r1 = -1, hint_c1 = -1;
int hint_r2 = -1, hint_c2 = -1;

// 判断胜利
int is_win() {
    for (int i = 0; i < ROW; i++)
        for (int j = 0; j < COL; j++)
            if (map[i][j] != -1)
                return 0;
    return 1;
}

// 初始化地图
void init_map() {
    srand((unsigned)time(NULL));
    for (int i = 0; i < ROW; i++)
        for (int j = 0; j < COL; j++)
            map[i][j] = -1;

    int total_pairs = (ROW * COL) / 2;
    for (int p = 0; p < total_pairs; p++) {
        int num = rand() % 8;
        int r1, c1;
        do { r1 = rand() % ROW; c1 = rand() % COL; } while (map[r1][c1] != -1);
        map[r1][c1] = num;

        int r2, c2;
        do { r2 = rand() % ROW; c2 = rand() % COL; } while (map[r2][c2] != -1);
        map[r2][c2] = num;
    }
}

// 画单个方块（整体下移INFO_HEIGHT像素，避开上方文字区）
void draw_block(int r, int c) {
    int x = c * BLOCK_SIZE;
    int y = r * BLOCK_SIZE + INFO_HEIGHT;

    setfillstyle(SOLID_FILL, BLACK);
    bar(x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);

    if (map[r][c] == -1) return;

    if ((r == hint_r1 && c == hint_c1) || (r == hint_r2 && c == hint_c2))
        setcolor(BLUE);
    else if (r == selected_r && c == selected_c)
        setcolor(RED);
    else
        setcolor(WHITE);

    rectangle(x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);
    outtextxy(x + 15, y + 10, patterns[map[r][c]]);
}

// 画顶部操作说明（单独一个深色条，完全不碰游戏区）
void draw_info() {
    // 画信息栏背景
    setfillstyle(SOLID_FILL, DARKGRAY);
    bar(0, 0, COL * BLOCK_SIZE, INFO_HEIGHT);

    // 写黄色文字，清晰不挡
    setcolor(YELLOW);
    setfont(14, 0, "宋体");
    outtextxy(10, 10, "操作说明：鼠标点击消除 | H提示 | Z撤销 | ESC退出");
}

// 全屏绘制
void draw_all() {
    cleardevice();
    draw_info();
    for (int i = 0; i < ROW; i++)
        for (int j = 0; j < COL; j++)
            draw_block(i, j);
}

// 撤销
void undo() {
    if (top < 0) return;
    Step s = stack[top--];
    map[s.r1][s.c1] = s.val1;
    map[s.r2][s.c2] = s.val2;
    draw_all();
}

// 提示
void get_hint() {
    hint_r1 = hint_c1 = hint_r2 = hint_c2 = -1;
    for (int i = 0; i < ROW * COL; i++) {
        int r1 = i / COL;
        int c1 = i % COL;
        if (map[r1][c1] == -1) continue;

        for (int j = i + 1; j < ROW * COL; j++) {
            int r2 = j / COL;
            int c2 = j % COL;
            if (map[r2][c2] == -1) continue;
            if (map[r1][c1] == map[r2][c2]) {
                hint_r1 = r1; hint_c1 = c1;
                hint_r2 = r2; hint_c2 = c2;
                draw_all();
                return;
            }
        }
    }
}

int main() {
    // 窗口高度=游戏区+信息栏高度
    initgraph(COL * BLOCK_SIZE, ROW * BLOCK_SIZE + INFO_HEIGHT);
    init_map();
    draw_all();

    while (1) {
        if (kbhit()) {
            char ch = getch();
            if (ch == 27) break;
            if (ch == 'z' || ch == 'Z') undo();
            if (ch == 'h' || ch == 'H') get_hint();
        }

        MOUSEMSG m = GetMouseMsg();
        if (m.uMsg != WM_LBUTTONUP) { delay_ms(1); continue; }

        // 鼠标坐标减去信息栏高度，转换为游戏格子坐标
        int r = (m.y - INFO_HEIGHT) / BLOCK_SIZE;
        int c = m.x / BLOCK_SIZE;
        if (r < 0 || r >= ROW || c < 0 || c >= COL) continue;
        if (map[r][c] == -1) continue;

        hint_r1 = hint_c1 = hint_r2 = hint_c2 = -1;

        if (selected_r == -1) {
            selected_r = r; selected_c = c;
            draw_all();
        } else {
            int old_r = selected_r;
            int old_c = selected_c;

            if (!(old_r == r && old_c == c) && map[old_r][old_c] == map[r][c]) {
                top++;
                stack[top].r1 = old_r; stack[top].c1 = old_c;
                stack[top].r2 = r; stack[top].c2 = c;
                stack[top].val1 = map[old_r][old_c];
                stack[top].val2 = map[r][c];

                map[old_r][old_c] = -1;
                map[r][c] = -1;
            }

            selected_r = -1; selected_c = -1;
            draw_all();

            if (is_win()) {
                cleardevice();
                setcolor(YELLOW);
                setfont(40, 0, "微软雅黑");
                outtextxy(70, 150, "恭喜通关！");
                delay_ms(3000);
                break;
            }
        }
        delay_ms(1);
    }

    closegraph();
    return 0;
}
