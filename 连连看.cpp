#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define ROW 8
#define COL 10
#define BLOCK_SIZE 50

int map[ROW][COL];
const LPCSTR patterns[] = { "1","2","3","4","5","6","7","8" ,"9","0"};
int selected_r = -1, selected_c = -1;

int is_win() {
    for (int i = 0; i < ROW; i++)
        for (int j = 0; j < COL; j++)
            if (map[i][j] != -1)
                return 0;
    return 1;
}

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

void draw_block(int r, int c) {
    int x = c * BLOCK_SIZE;
    int y = r * BLOCK_SIZE;

    setfillstyle(SOLID_FILL, BLACK);
    bar(x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);

    if (map[r][c] == -1) return;

    setcolor((r == selected_r && c == selected_c) ? RED : WHITE);
    rectangle(x, y, x + BLOCK_SIZE, y + BLOCK_SIZE);
    outtextxy(x + 15, y + 10, patterns[map[r][c]]);
}

void draw_all() {
    cleardevice();
    for (int i = 0; i < ROW; i++)
        for (int j = 0; j < COL; j++)
            draw_block(i, j);
}

int main() {
    initgraph(COL * BLOCK_SIZE, ROW * BLOCK_SIZE);
    init_map();
    draw_all();

    while (1) {
        if (kbhit()) break;

        MOUSEMSG m = GetMouseMsg();
        if (m.uMsg != WM_LBUTTONUP) {
            delay_ms(1);
            continue;
        }

        int r = m.y / BLOCK_SIZE;
        int c = m.x / BLOCK_SIZE;
        if (r < 0 || r >= ROW || c < 0 || c >= COL) continue;
        if (map[r][c] == -1) continue;

        if (selected_r == -1) {
            selected_r = r;
            selected_c = c;
            draw_block(r, c);
        } else {
            int old_r = selected_r;
            int old_c = selected_c;

            if (!(old_r == r && old_c == c) && map[old_r][old_c] == map[r][c]) {
                map[old_r][old_c] = -1;
                map[r][c] = -1;
            }

            selected_r = -1;
            selected_c = -1;

            draw_block(old_r, old_c);
            draw_block(r, c);

            if (is_win()) {
                cleardevice();
                setfont(40, 0, "微软雅黑"); 
                setcolor(YELLOW);
                outtextxy(80, 150, "恭喜通关！");
                outtextxy(60, 220, "全部消除成功！");
                delay_ms(3000);
                break;
            }
        }
        delay_ms(1);
    }

    closegraph();
    return 0;
}
