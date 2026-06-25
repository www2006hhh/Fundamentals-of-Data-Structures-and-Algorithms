#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

#define ROW 8
#define COL 10
#define BLOCK_SIZE 50
#define INFO_HEIGHT 90
#define MAX_LEVEL 3

int map[ROW][COL];
const char* patterns[] = {"1","2","3","4","5","6","7","8"};
int selected_r = -1, selected_c = -1;
int nowLevel = 1;
int levelPassed = 0;
int remainingPairs = 0;
int gameTime = 0;
int timeCounter = 0;

typedef struct{
    int r1,c1;
    int r2,c2;
    int val1,val2;
}Step;
Step stack[1000];
int top = -1;

typedef struct{
    int data[ROW][COL];
}MapSnap;
MapSnap snapStack[500];
int snapTop = -1;

int hint_r1 = -1,hint_c1 = -1;
int hint_r2 = -1,hint_c2 = -1;

typedef struct{
    char name[20];
    int level;
    int score;
    int bestTime;
}Player;
Player player;

// ---------- BFS 寻路 ----------
typedef struct PathNode {
    int r, c;
    int preIdx;
} PathNode;

typedef struct Queue {
    PathNode data[1000];
    int front, rear;
} Queue;

int pathR[100], pathC[100];
int pathLen;

void InitQueue(Queue *q) {
    q->front = q->rear = 0;
}
int IsEmpty(Queue *q) {
    return q->front == q->rear;
}
void EnQueue(Queue *q, PathNode node) {
    q->data[q->rear++] = node;
}
PathNode DeQueue(Queue *q) {
    return q->data[q->front++];
}

int IsPassable(int r, int c) {
    if (r < 0 || r >= ROW || c < 0 || c >= COL)
        return 0;
    return map[r][c] == -1;
}

int FindShortPath(int startR, int startC, int endR, int endC) {
    if (startR == endR && startC == endC) return 0;

    Queue q;
    InitQueue(&q);
    int vis[ROW][COL] = {0};
    int dir[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};

    PathNode start = {startR, startC, -1};
    EnQueue(&q, start);
    vis[startR][startC] = 1;
    int findFlag = 0;
    int targetIdx = -1;

    while (!IsEmpty(&q)) {
        int curIdx = q.front;
        PathNode cur = DeQueue(&q);
        for (int d = 0; d < 4; d++) {
            int nr = cur.r + dir[d][0];
            int nc = cur.c + dir[d][1];
            if (nr == endR && nc == endC) {
                targetIdx = curIdx;
                findFlag = 1;
                break;
            }
            if (IsPassable(nr, nc) && !vis[nr][nc]) {
                vis[nr][nc] = 1;
                PathNode next = {nr, nc, curIdx};
                EnQueue(&q, next);
            }
        }
        if (findFlag) break;
    }

    if (!findFlag) return 0;

    pathLen = 0;
    int curIdx = targetIdx;
    while (curIdx != -1) {
        PathNode p = q.data[curIdx];
        pathR[pathLen] = p.r;
        pathC[pathLen] = p.c;
        curIdx = p.preIdx;
        pathLen++;
    }
    for (int i = 0; i < pathLen / 2; i++) {
        int tr = pathR[i], tc = pathC[i];
        pathR[i] = pathR[pathLen - 1 - i];
        pathC[i] = pathC[pathLen - 1 - i];
    }
    pathR[pathLen] = endR;
    pathC[pathLen] = endC;
    pathLen++;
    return 1;
}

int GetTurnCount() {
    if (pathLen <= 2) return 0;
    int turn = 0;
    int preDr = pathR[1] - pathR[0];
    int preDc = pathC[1] - pathC[0];
    for (int i = 2; i < pathLen; i++) {
        int curDr = pathR[i] - pathR[i-1];
        int curDc = pathC[i] - pathC[i-1];
        if (curDr != preDr || curDc != preDc) {
            turn++;
            preDr = curDr;
            preDc = curDc;
        }
    }
    return turn;
}

// ---------- 绘制 ----------
void DrawPath() {
    setlinewidth(4);
    setcolor(RGB(255, 50, 50));
    for (int i = 0; i < pathLen - 1; i++) {
        int x1 = pathC[i] * BLOCK_SIZE + BLOCK_SIZE / 2;
        int y1 = pathR[i] * BLOCK_SIZE + BLOCK_SIZE / 2 + INFO_HEIGHT;
        int x2 = pathC[i+1] * BLOCK_SIZE + BLOCK_SIZE / 2;
        int y2 = pathR[i+1] * BLOCK_SIZE + BLOCK_SIZE / 2 + INFO_HEIGHT;
        setcolor(RGB(255, 200, 200));
        setlinewidth(2);
        line(x1-1, y1-1, x2-1, y2-1);
        line(x1+1, y1+1, x2+1, y2+1);
        setcolor(RGB(255, 80, 80));
        setlinewidth(4);
        line(x1, y1, x2, y2);
    }
    setlinewidth(1);
}

int is_win(){
    for(int i=0;i<ROW;i++)
        for(int j=0;j<COL;j++)
            if(map[i][j]!=-1)
                return 0;
    return 1;
}

void saveSnap(){
    snapTop++;
    for(int i=0;i<ROW;i++)
        memcpy(snapStack[snapTop].data[i], map[i], sizeof(map[i]));
}

void loadSnap(){
    if(snapTop<0)return;
    for(int i=0;i<ROW;i++)
        memcpy(map[i], snapStack[snapTop].data[i], sizeof(map[i]));
    snapTop--;
}

int countRemainingPairs() {
    int count = 0;
    for(int i=0;i<ROW;i++)
        for(int j=0;j<COL;j++)
            if(map[i][j] != -1)
                count++;
    return count / 2;
}

void init_map(int diff){
    srand((unsigned)time(NULL));
    memset(map, -1, sizeof(map));
    int pairNum = (ROW*COL)/2;
    for(int p=0;p<pairNum;p++){
        int num = rand()%diff;
        int r1,c1;
        do{
            r1 = rand()%ROW;
            c1 = rand()%COL;
        }while(map[r1][c1]!=-1);
        map[r1][c1] = num;
        int r2,c2;
        do{
            r2 = rand()%ROW;
            c2 = rand()%COL;
        }while(map[r2][c2]!=-1);
        map[r2][c2] = num;
    }
    levelPassed = 0;
    snapTop = -1;
    top = -1;
    gameTime = 0;
    timeCounter = 0;
    remainingPairs = countRemainingPairs();
}

void draw_block(int r,int c){
    int x = c*BLOCK_SIZE;
    int y = r*BLOCK_SIZE+INFO_HEIGHT;
    setfillstyle(SOLID_FILL, RGB(20,20,30));
    bar(x,y,x+BLOCK_SIZE,y+BLOCK_SIZE);
    if(map[r][c]==-1) return;

    if((r==hint_r1&&c==hint_c1)||(r==hint_r2&&c==hint_c2))
        setcolor(RGB(0, 200, 255));
    else if(r==selected_r&&c==selected_c)
        setcolor(RGB(255, 50, 50));
    else
        setcolor(RGB(180,180,180));
    rectangle(x,y,x+BLOCK_SIZE,y+BLOCK_SIZE);

    setfont(24,0,"宋体");
    if((r==hint_r1&&c==hint_c1)||(r==hint_r2&&c==hint_c2))
        setcolor(RGB(0, 200, 255));
    else if(r==selected_r&&c==selected_c)
        setcolor(RGB(255, 50, 50));
    else
        setcolor(WHITE);
    outtextxy(x+15, y+12, patterns[map[r][c]]);
}

void draw_info(){
    setfillstyle(SOLID_FILL, RGB(40,40,60));
    bar(0,0,COL*BLOCK_SIZE,INFO_HEIGHT);
    setcolor(RGB(100,100,150));
    line(0, INFO_HEIGHT-1, COL*BLOCK_SIZE, INFO_HEIGHT-1);

    setcolor(YELLOW);
    setfont(14,0,"宋体");
    char buf1[80], buf2[80], buf3[80];
    sprintf(buf1,"关卡:%d  玩家:%s  分数:%d", nowLevel, player.name, player.score);
    sprintf(buf2,"剩余配对:%d  时间:%02d:%02d", remainingPairs, gameTime/60, gameTime%60);
    sprintf(buf3,"H提示  Z撤销  U回溯  ESC退出");
    outtextxy(10,10,buf1);
    outtextxy(10,35,buf2);
    outtextxy(10,60,buf3);

    if(player.bestTime > 0) {
        char best[40];
        sprintf(best,"最佳:%02d:%02d", player.bestTime/60, player.bestTime%60);
        setcolor(RGB(100,255,100));
        outtextxy(COL*BLOCK_SIZE-130, 10, best);
    }
}

void draw_all(){
    cleardevice();
    draw_info();
    for(int i=0;i<ROW;i++)
        for(int j=0;j<COL;j++)
            draw_block(i,j);
}

// ---------- 操作 ----------
void undo(){
    if(top<0) {
        MessageBox(NULL,"没有可撤销的操作！","提示",MB_OK);
        return;
    }
    Step s = stack[top--];
    map[s.r1][s.c1] = s.val1;
    map[s.r2][s.c2] = s.val2;
    remainingPairs = countRemainingPairs();
    draw_all();
}

void undoAllSnap(){
    if(snapTop<0) {
        MessageBox(NULL,"没有可回溯的盘面！","提示",MB_OK);
        return;
    }
    loadSnap();
    remainingPairs = countRemainingPairs();
    draw_all();
}

void get_hint(){
    hint_r1=hint_c1=hint_r2=hint_c2=-1;
    int bestTurn = 999;
    int bestR1=-1,bestC1=-1,bestR2=-1,bestC2=-1;

    for(int i=0;i<ROW*COL;i++){
        int r1 = i/COL;
        int c1 = i%COL;
        if(map[r1][c1]==-1) continue;
        for(int j=i+1;j<ROW*COL;j++){
            int r2 = j/COL;
            int c2 = j%COL;
            if(map[r2][c2]==-1) continue;
            if(map[r1][c1]==map[r2][c2]){
                if(FindShortPath(r1, c1, r2, c2)) {
                    int turn = GetTurnCount();
                    if(turn < bestTurn) {
                        bestTurn = turn;
                        bestR1 = r1; bestC1 = c1;
                        bestR2 = r2; bestC2 = c2;
                        if(turn == 0) goto found;
                    }
                }
            }
        }
    }
found:
    if(bestR1 != -1) {
        hint_r1 = bestR1; hint_c1 = bestC1;
        hint_r2 = bestR2; hint_c2 = bestC2;
        draw_all();
    } else {
        MessageBox(NULL,"没有可消除的对！","提示",MB_OK);
    }
}

// ---------- 存档 ----------
void savePlayer(){
    FILE* fp = fopen("player.txt","w");
    if(fp){
        fprintf(fp,"%s %d %d %d", player.name, player.level, player.score, player.bestTime);
        fclose(fp);
    }
}

void loadPlayer(){
    FILE* fp = fopen("player.txt","r");
    if(fp){
        fscanf(fp,"%s %d %d %d", player.name, &player.level, &player.score, &player.bestTime);
        fclose(fp);
    }else{
        strcpy(player.name,"游客");
        player.level = 1;
        player.score = 0;
        player.bestTime = 0;
    }
}

// ========== 主函数 ==========
int main(){
    initgraph(COL*BLOCK_SIZE, ROW*BLOCK_SIZE+INFO_HEIGHT);
    setbkcolor(RGB(20,20,30));
    loadPlayer();
    nowLevel = player.level;
    init_map(nowLevel+2);
    draw_all();

    DWORD pathShowStart = 0;
    int showPathFlag = 0;
    DWORD lastTimeUpdate = GetTickCount();

    // 用于 Windows API 鼠标检测的状态
    static int lastLButton = 0;

    while(1){
        DWORD now = GetTickCount();

        // 更新时间
        if(!showPathFlag && !is_win()) {
            if(now - lastTimeUpdate >= 1000) {
                gameTime++;
                lastTimeUpdate = now;
                draw_info();
            }
        }

        // 路径动画
        if(showPathFlag){
            if(now - pathShowStart < 800){
                DrawPath();
            }else{
                showPathFlag = 0;
                draw_all();
            }
        }

        // 键盘
        if(_kbhit()){
            char ch = _getch();
            if(ch==27) break;
            if(ch=='z'||ch=='Z') { undo(); lastTimeUpdate = GetTickCount(); }
            if(ch=='h'||ch=='H') { get_hint(); lastTimeUpdate = GetTickCount(); }
            if(ch=='u'||ch=='U') { undoAllSnap(); lastTimeUpdate = GetTickCount(); }
        }

        // ---------- 鼠标处理（使用 Windows API，兼容 EGE） ----------
        int curLButton = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1 : 0;
        if (curLButton && !lastLButton) {   // 检测到单击（按下瞬间）
            POINT pt;
            GetCursorPos(&pt);
            ScreenToClient(getHWnd(), &pt);   // ★ 修改点：GetHwsnd() → getHWnd()
            int r = (pt.y - INFO_HEIGHT) / BLOCK_SIZE;
            int c = pt.x / BLOCK_SIZE;

            if (r >= 0 && r < ROW && c >= 0 && c < COL && map[r][c] != -1) {
                hint_r1=hint_c1=hint_r2=hint_c2=-1;
                if(selected_r==-1){
                    selected_r=r;
                    selected_c=c;
                    draw_all();
                }else{
                    int old_r=selected_r;
                    int old_c=selected_c;
                    if(!(old_r==r&&old_c==c)){
                        if(map[old_r][old_c]==map[r][c]){
                            int connect = FindShortPath(old_r, old_c, r, c);
                            if(!connect){
                                MessageBox(NULL,"两点无法连通！","消除失败",MB_OK);
                                selected_r=-1;selected_c=-1;
                                draw_all();
                                continue;
                            }
                            int turn = GetTurnCount();
                            if(turn>3){
                                MessageBox(NULL,"连线超过3段折线，无法消除！","消除失败",MB_OK);
                                selected_r=-1;selected_c=-1;
                                draw_all();
                                continue;
                            }
                            draw_all();
                            showPathFlag = 1;
                            pathShowStart = GetTickCount();

                            saveSnap();
                            top++;
                            stack[top].r1=old_r;stack[top].c1=old_c;
                            stack[top].r2=r;stack[top].c2=c;
                            stack[top].val1=map[old_r][old_c];
                            stack[top].val2=map[r][c];
                            map[old_r][old_c]=-1;
                            map[r][c]=-1;
                            player.score +=10;
                            remainingPairs = countRemainingPairs();
                        }else{
                            MessageBox(NULL,"图案不一致，不能消除","提示",MB_OK);
                        }
                    }
                    selected_r=-1;
                    selected_c=-1;
                    draw_all();

                    if(is_win()){
                        levelPassed = 1;
                        player.level = nowLevel;
                        if(player.bestTime == 0 || gameTime < player.bestTime) {
                            player.bestTime = gameTime;
                        }
                        savePlayer();

                        cleardevice();
                        setcolor(YELLOW);
                        setfont(40,0,"宋体");
                        outtextxy(80,130,"?? 本关通关 ??");
                        char timeStr[50];
                        sprintf(timeStr,"用时: %02d:%02d", gameTime/60, gameTime%60);
                        setfont(24,0,"宋体");
                        setcolor(RGB(100,255,100));
                        outtextxy(120,190,timeStr);

                        if(nowLevel >= MAX_LEVEL){
                            setfont(28,0,"宋体");
                            setcolor(RGB(255,200,50));
                            outtextxy(40,240,"?? 全部关卡通关! ??");
                            outtextxy(80,290,"按任意键退出");
                            Sleep(2000);
                            while(!_kbhit()) { Sleep(10); }
                            closegraph();
                            return 0;
                        }else{
                            setfont(20,0,"宋体");
                            setcolor(WHITE);
                            outtextxy(100,250,"按 N 进入下一关  按 ESC 退出");
                            while(1){
                                if(_kbhit()){
                                    char key = _getch();
                                    if(key == 'n' || key == 'N'){
                                        nowLevel++;
                                        init_map(nowLevel+2);
                                        draw_all();
                                        lastTimeUpdate = GetTickCount();
                                        break;
                                    }
                                    if(key == 27){
                                        closegraph();
                                        return 0;
                                    }
                                }
                                Sleep(10);
                            }
                        }
                    }
                }
            }
        }
        lastLButton = curLButton;
        Sleep(5);
    }
    savePlayer();
    closegraph();
    return 0;
}
