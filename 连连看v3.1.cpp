#include <graphics.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <conio.h>

#define ROW 8
#define COL 10
#define BLOCK_SIZE 50
#define INFO_HEIGHT 50
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

int hint_r1 = -1,hint_c1 = -1;
int hint_r2 = -1,hint_c2 = -1;

typedef struct{
    char name[20];
    int level;
    int score;
}Player;

Player player;

int is_win(){
    for(int i=0;i<ROW;i++)
        for(int j=0;j<COL;j++)
            if(map[i][j]!=-1)
                return 0;
    return 1;
}

void init_map(int diff){
    srand((unsigned)time(NULL));
    for(int i=0;i<ROW;i++)
        for(int j=0;j<COL;j++)
            map[i][j] = -1;

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
    outtextxy(x+15,y+10,patterns[map[r][c]]);
}

void draw_info(){
    setfillstyle(SOLID_FILL,DARKGRAY);
    bar(0,0,COL*BLOCK_SIZE,INFO_HEIGHT);
    setcolor(YELLOW);
    setfont(20,0,_T("宋体"));

    char buf[100];
    sprintf(buf,"关卡:%d 玩家:%s 分数:%d | H提示 Z撤销 ESC退出",nowLevel,player.name,player.score);
    outtextxy(10,15,buf);
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

    while(1){
        if(kbhit()){
            char ch = getch();
            if(ch==27) break;
            if(ch=='z'||ch=='Z') undo();
            if(ch=='h'||ch=='H') get_hint();
        }

        MOUSEMSG m = GetMouseMsg();
        if(m.uMsg!=WM_LBUTTONUP){
            delay_ms(1);
            continue;
        }

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

            if(!(old_r==r&&old_c==c)&&map[old_r][old_c]==map[r][c]){
                top++;
                stack[top].r1=old_r;stack[top].c1=old_c;
                stack[top].r2=r;stack[top].c2=c;
                stack[top].val1=map[old_r][old_c];
                stack[top].val2=map[r][c];

                map[old_r][old_c]=-1;
                map[r][c]=-1;
                player.score +=10;
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
                setfont(40,0,_T("宋体"));
                outtextxy(70,150,_T("本关通关"));

                if(nowLevel >= MAX_LEVEL){
                    outtextxy(40,210,_T("全部关卡通关!"));
                    delay_ms(3000);
                    closegraph();
                    return 0;
                }else{
                    outtextxy(40,210,_T("按N进入下一关"));
                    delay_ms(2000);

                    while(1){
                        if(kbhit()){
                            char key = getch();
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
        delay_ms(1);
    }

    savePlayer();
    closegraph();
    return 0;
}
