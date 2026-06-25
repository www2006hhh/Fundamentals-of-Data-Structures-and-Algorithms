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
#define INFO_HEIGHT 70
#define MAX_LEVEL 3

int map[ROW][COL];
const char* patterns[] = {"1","2","3","4","5","6","7","8"};
int selected_r = -1, selected_c = -1;
int nowLevel = 1;
int levelPassed = 0;

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
}Player;
Player player;

//==================== V5 BFS寻路模块 ====================
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
        int curIdx = q->front;
        PathNode cur = DeQueue(&q);
        for (int d = 0; d < 4; d++) {
            int nr = cur.r + dir[d][0];
            int nc = cur.c + dir[d][1];
            if (nr == endR && nc == endC) {
                targetIdx = curIdx;
                findFlag = 1;
                goto endSearch;
            }
            if (IsPassable(nr, nc) && !vis[nr][nc]) {
                vis[nr][nc] = 1;
                PathNode next = {nr, nc, curIdx};
                EnQueue(&q, next);
            }
        }
    }
endSearch:
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
    // 反转路径
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

void DrawPath() {
    setlinewidth(3);
    setcolor(RED);
    for (int i = 0; i < pathLen - 1; i++) {
        int x1 = pathC[i] * BLOCK_SIZE + BLOCK_SIZE / 2;
        int y1 = pathR[i] * BLOCK_SIZE + BLOCK_SIZE / 2 + INFO_HEIGHT;
        int x2 = pathC[i+1] * BLOCK_SIZE + BLOCK_SIZE / 2;
        int y2 = pathR[i+1] * BLOCK_SIZE + BLOCK_SIZE / 2 + INFO_HEIGHT;
        line(x1, y1, x2, y2);
    }
}
//=========================================================

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
    snapTop=-1;
}

void draw_block(int r,int c){
    int x = c*BLOCK_SIZE;
    int y = r*BLOCK_SIZE+INFO_HEIGHT;
    setfillstyle(SOLID_FILL,BLACK);
    bar(x,y,x+BLOCK_SIZE,y+BLOCK_SIZE);
    if(map[r][c]==-1) return;
    if((r==hint_r1&&c==hint_c1)||(r==hint_r2&&c==hint_c2))
        setcolor(BLUE);
    else if(r==selected_r&&c==selected_c)
        setcolor(RED);
    else
        setcolor(WHITE);
    rectangle(x,y,x+BLOCK_SIZE,y+BLOCK_SIZE);
    setfont(16,0,"宋体");
    outtextxy(x+15,y+10,patterns[map[r][c]]);
}

void draw_info(){
    setfillstyle(SOLID_FILL,DARKGRAY);
    bar(0,0,COL*BLOCK_SIZE,INFO_HEIGHT);
    setcolor(YELLOW);
    setfont(16,0,"宋体");
    char buf1[80],buf2[80];
    sprintf(buf1,"关卡:%d 玩家:%s 分数:%d",nowLevel,player.name,player.score);
    sprintf(buf2,"H提示  Z单步撤销  U整盘回溯  ESC退出");
    outtextxy(10,12,buf1);
    outtextxy(10,38,buf2);
}

void draw_all(){
    cleardevice();
    draw_info();
    for(int i=0;i<ROW;i++)
        for(int j=0;j<COL;j++)
            draw_block(i,j);
}

void undo(){
    if(top<0) return;
    Step s = stack[top--];
    map[s.r1][s.c1] = s.val1;
    map[s.r2][s.c2] = s.val2;
    draw_all();
}

void undoAllSnap(){
    loadSnap();
    draw_all();
}

void get_hint(){
    hint_r1=hint_c1=hint_r2=hint_c2=-1;
    for(int i=0;i<ROW*COL;i++){
        int r1 = i/COL;
        int c1 = i%COL;
        if(map[r1][c1]==-1) continue;
        for(int j=i+1;j<ROW*COL;j++){
            int r2 = j/COL;
            int c2 = j%COL;
            if(map[r2][c2]==-1) continue;
            if(map[r1][c1]==map[r2][c2]){
                hint_r1=r1;hint_c1=c1;
                hint_r2=r2;hint_c2=c2;
                draw_all();
                return;
            }
        }
    }
}

void savePlayer(){
    FILE* fp = fopen("player.txt","w");
    if(fp){
        fprintf(fp,"%s %d %d",player.name,player.level,player.score);
        fclose(fp);
    }
}

void loadPlayer(){
    FILE* fp = fopen("player.txt","r");
    if(fp){
        fscanf(fp,"%s %d %d",player.name,&player.level,&player.score);
        fclose(fp);
    }else{
        strcpy(player.name,"游客");
        player.level = 1;
        player.score = 0;
    }
}

int main(){
    initgraph(COL*BLOCK_SIZE,ROW*BLOCK_SIZE+INFO_HEIGHT);
    loadPlayer();
    nowLevel = player.level;
    init_map(nowLevel+2);
    draw_all();

    DWORD pathShowStart = 0;
    int showPathFlag = 0;

    while(1){
        // 非阻塞路径显示计时，替代Sleep卡死
        if(showPathFlag){
            DWORD now = GetTickCount();
            if(now - pathShowStart < 800){
                DrawPath();
            }else{
                showPathFlag = 0;
                draw_all();
            }
        }

        if(_kbhit()){
            char ch = _getch();
            if(ch==27) break;
            if(ch=='z'||ch=='Z') undo();
            if(ch=='h'||ch=='H') get_hint();
            if(ch=='u'||ch=='U') undoAllSnap();
        }

        MOUSEMSG m;
        if (PeekMouseMsg(&m) && m.uMsg == WM_LBUTTONUP)
        {
            FlushMouseMsgBuffer();
            int r = (m.y-INFO_HEIGHT)/BLOCK_SIZE;
            int c = m.x/BLOCK_SIZE;
            if(r<0||r>=ROW||c<0||c>=COL) continue;
            if(map[r][c]==-1) continue;

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
                        // 开启路径计时显示，不阻塞主线程
                        draw_all();
                        showPathFlag = 1;
                        pathShowStart = GetTickCount();

                        saveSnap();
                        top++;
                        stack[top].r1=old_r;stack[top].c1=old_c;
                        stack[top].r2=r;stack[top].c2=r;
                        stack[top].val1=map[old_r][old_c];
                        stack[top].val2=map[r][c];
                        map[old_r][old_c]=-1;
                        map[r][c]=-1;
                        player.score +=10;
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
                    savePlayer();
                    cleardevice();
                    setcolor(YELLOW);
                    setfont(40,0,"宋体");
                    outtextxy(70,150,"本关通关");
                    if(nowLevel >= MAX_LEVEL){
                        outtextxy(40,210,"全部关卡通关!");
                        Sleep(3000);
                        closegraph();
                        return 0;
                    }else{
                        outtextxy(40,210,"按N进入下一关");
                        Sleep(2000);
                        while(1){
                            if(_kbhit()){
                                char key = _getch();
                                if(key == 'n' || key == 'N'){
                                    nowLevel++;
                                    init_map(nowLevel+2);
                                    draw_all();
                                    break;
                                }
                                if(key == 27){
                                    closegraph();
                                    return 0;
                                }
                            }
                        }
                    }
                }
            }
        }
        delay_ms(2); // 延长循环间隔，降低CPU占用
    }
    savePlayer();
    closegraph();
    return 0;
}
