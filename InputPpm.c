#include <stdio.h>
#include <stdlib.h>/*exitを使用するためのヘッダファイル*/
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <math.h>
#define MAXOFARRAYSIZE 20000
#define FILENAME "./sample/data2.txt"/*数値データの入ったファイル名*/
#define MAX 1024



typedef void(*ARRAY_FUNC)(long array[],int x,int y);
//関数ポインタを要素に持つ配列の宣言を簡略化するため

struct list{
    int num;
    int index;
    struct list *next;
}list;

typedef struct list LIST;



LIST *add_list(int num, int index, LIST *tail);

void print_array(long array[],int arraysize){
    int i;
    printf("生成された配列の要素は順に{\n");
    for (i = 0;i < arraysize;i++){
        if (i == 0){
            printf("%4ld ",array[0]);
        }
        else {
            if (i%10){
                printf("%4ld ",array[i]);
            }
            else {	
                printf("\n%4ld ",array[i]);
            }
        }
    }
    printf("\n}です。\n");
}


//debug
void show_list(LIST *p)
{
	while (p != NULL) { 
		printf("%3d %d\n", p->num, p->index);
		p = p->next;
	}
}

void free_list(LIST *p)
{
	LIST *p2;

	while (p != NULL) { 
		p2 = p->next;
		free(p);
		p = p2;
	}
}


//////////////////////////////////////
//////////////////////////////////////
//////////////////////////////////////
int main(void){  
    char *fname = FILENAME;
    FILE *ip;
    ip = fopen(fname,"r");
    if(ip == NULL){ 
        fprintf(stderr, "%sを正常に開くことが出来ませんでした.\n" ,FILENAME);
        exit(1);/*異常終了*/
    }
    
    else{
        printf("loading %s...\n",FILENAME);


        //=============================================================
        char buf[MAX];

        //実装上読み込むppmの形式を以下のように制限する
        /* P3\n */
        /* WIDTH HEIGHT\n */ //(空白で区切る)
        /* 255\n */
        /* ..... */
        //と指定
        //画素値についてはrgbの3つの数値の間に改行を挟むことを許さない
        //仕様書通り幅と高さは空白含めて70文字までとする
        //コメントは実装しない
        
        //マジックナンバーを取得========================
        fgets(buf,70,ip);
        char *magic_num = strtok(buf, "\n");
        printf("magic number is %s.\n", magic_num);
        //===========================================
        
        //WIDTH、HEIGHTを取得=========================
        fgets(buf,70,ip);
        int ppm_width = atoi(strtok(buf, " "));
        printf("width is %d.\n", ppm_width);
        int ppm_height = atoi(strtok(NULL, "\n"));
        printf("height is %d.\n", ppm_height);
        //===========================================
        
        //上限値を取得=================================
        fgets(buf,70,ip);
        int ppm_max = atoi(strtok(buf, "\n"));
        printf("max is %d.\n", ppm_max);
        //===========================================

        //リストの先頭ポインタ
        LIST *head, *tail;
        head = NULL;
        tail = NULL;
  
        int num;
        int index = 0;

        //=============================================================
        while (1){
            num = fgetc(ip);
            if(num == EOF){
                break;
            }
            char char_buf[256];
            char reset[] = "";
            //空白判定
            //空白のとき
            if(isspace(num) != 0){
                //先頭、最後尾をセット
                if(index == 0){
                    LIST *p;
                    /* 記憶領域の確保 */
                    if ((p = (LIST *) malloc(sizeof(LIST))) == NULL) {
                        printf("malloc error\n");
                        exit(EXIT_FAILURE);
                    }
                    /* リストにデータを登録 */
                    p->num = atoi(char_buf);
                    p->index = index;
                    /* ポインタのつなぎ換え */
                    p->next = NULL;
                    tail = p;
                    head = p;
                }
                else{
                    tail = add_list(atoi(char_buf), index, tail);
                }
                index ++;
                memcpy(char_buf, reset, sizeof(char) * 256);
            }
            //空白以外のとき（数字のはず）
            else{
                sprintf(buf, "%c", num);
                strcat(char_buf, buf);
            }   
        }
        //=============================================================          
        fclose(ip);
        printf("completed processing %s\n",FILENAME);
        
        //debug
        //show_list(head);
        
        //取り込んだppmの保存領域内を確保
        int input_ppm[ppm_height][ppm_width][3];


        
        //LISTを通常の配列に変換==================================
        LIST *p = head;
        //debug
        printf("input ppm is...\n");
        while (p->next != NULL) {
            div_t d1 = div(p->index, 3);
            div_t d2 = div(d1.quot, ppm_width);
                
            input_ppm[d2.quot][d2.rem][d1.rem] = p->num;
            //debug
            printf("input_ppm[%d][%d][%d] = %d\n",
                   d2.quot, d2.rem, d1.rem,
                   input_ppm[d2.quot][d2.rem][d1.rem]);
            
            p = p->next;
        }
        //=====================================================

        
        free_list(head);
        return 0;
    }
}

LIST *add_list(int num, int index, LIST *tail){
	LIST *p;

	/* 記憶領域の確保 */
	if ((p = (LIST *) malloc(sizeof(LIST))) == NULL) {
		printf("malloc error\n");
		exit(EXIT_FAILURE);
	}
	
	/* リストにデータを登録 */
	p->num = num;
	p->index = index;	
	/* ポインタのつなぎ換え */
	p->next = NULL;
	tail->next = p;
    //最後尾はpになる
	return p;
}



