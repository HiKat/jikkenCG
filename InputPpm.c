#include <stdio.h>
#include <stdlib.h>/*exitを使用するためのヘッダファイル*/
#include <string.h>
#include <float.h>
#include <ctype.h>
#include <math.h>
//#define MAP_FILENAME "./sample/spheremap1.ppm"
//#define MAP_FILENAME "./sample/MapSample.txt"
//#define MAP_FILENAME "./sample/spheremap2.ppm"
#define MAP_FILENAME "./Kadai02ForHead.txt"

#define MAX 1024
#define OUTPUTNAME "inputMap.ppm"
//#define OUTPUTNAME "inputMap.txt"



struct list{
    int num;
    int index;
    struct list *next;
}list;

typedef struct list LIST;

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
    printf("index = %d num = %d\n", p->index, p->num);
	/* ポインタのつなぎ換え */
	p->next = NULL;
	tail->next = p;
    //最後尾はpになる
	return p;
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
    char *map_fname = MAP_FILENAME;
    FILE *ip;
    ip = fopen(map_fname,"r");
    if(ip == NULL){ 
        fprintf(stderr, "%sを正常に開くことが出来ませんでした.\n" ,MAP_FILENAME);
        exit(1);/*異常終了*/
    }
    
    else{
        printf("loading %s...\n",MAP_FILENAME);


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


        //==============================================================================
        printf("magic number is %s.\n", magic_num);
        //==============================================================================

        
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
        char char_buf[256];
        int flag = 0;
        //=============================================================
        while (1){
            num = fgetc(ip);
            char reset[] = "";
            //空白判定
            //空白のとき
            if(isspace(num) != 0 || num == EOF){
                //直前に読み込んだ文字が空白のとき
                if(flag == 1){}
                //直前に読み込んだ文字が空白でないとき
                else{
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
                        printf("index = %d num = %d\n", p->index, p->num);
                        
                        /* ポインタのつなぎ換え */
                        p->next = NULL;
                        tail = p;
                        head = p;
                        /* ループから抜ける */
                        if(num == EOF){
                            break;
                        }
                    }
                    else{
                    tail = add_list(atoi(char_buf), index, tail);
                    }
                    index ++;
                    memcpy(char_buf, reset, sizeof(char) * 256);
                    flag = 1;
                }
            }
            //空白以外のとき（数字のはず）
            else{
                flag = 0;
                sprintf(buf, "%c", num);
                strcat(char_buf, buf);
            }   
        }
        //=============================================================          
        fclose(ip);
        printf("completed processing %s\n",MAP_FILENAME);
        
        //debug
        //show_list(head);
        
        //取り込んだppmの保存領域内を確保
        int input_ppm[ppm_height][ppm_width][3];


        
        //LISTを通常の配列に変換==================================
        LIST *p = head;

        //debug
        printf("input ppm is...\n");
        printf("head->index = %d\ttail->index = %d\n", head->index, tail->index);

        int max_index = (ppm_height*ppm_width*3)-1;
        while (p->next != NULL) {
            //通常の画像viewerはヘッダを見て256*256であれば
            //それ以降の余分な数値は無視する
            if(max_index < (p->index)){
                break;
            }
            div_t d1 = div(p->index, 3);
            div_t d2 = div(d1.quot, ppm_width);
                
            input_ppm[d2.quot][d2.rem][d1.rem] = p->num;
            /* rintf("input_ppm[%d][%d][%d] = %d\n", */
            /*       d2.quot, d2.rem, d1.rem, */
            /*       input_ppm[d2.quot][d2.rem][d1.rem]); */
            
            p = p->next;
        }
        div_t d3 = div(max_index, 3);
        div_t d4 = div(d3.quot, ppm_width);
        input_ppm[d4.quot][d4.rem][d3.rem] = p->num;
        
        /* printf("input_ppm[%d][%d][%d] = %d\n", */
        /*        d4.quot, d4.rem, d3.rem, */
        /*        input_ppm[d4.quot][d4.rem][d3.rem]); */
        
        //=====================================================

        
        free_list(head);

        //=====================================================
        FILE *out_ppm;
        char *out_fname = OUTPUTNAME;
        out_ppm = fopen( out_fname, "w" );
        //ファイルが開けなかったとき
        if( out_ppm == NULL ){
            printf("%sファイルが開けません.\n", out_fname);
            return -1;
        }
        
        //出力
        else{
            //ヘッダ出力=========================================
            //printf("magic number is %s.\n", magic_num);
            char str[1024];
            /* sprintf(str, "%s\n", magic_num); */
            /* fputs(str, out_ppm); */
            
            fputs("P3\n", out_ppm);
            sprintf(str, "%d\t%d\n", ppm_width, ppm_height);
            fputs(str, out_ppm);
            sprintf(str, "%d\n", ppm_max);
            fputs(str, out_ppm);
            //==================================================

            //輝度値出力=========================================
            for(int i = 0; i < ppm_height; i++){
                for(int j = 0; j < ppm_width; j++){
                    char r[256];
                    char g[256];
                    char b[256];
                    sprintf(r, "%d", (int)round(input_ppm[i][j][0]));
                    sprintf(g, "%d", (int)round(input_ppm[i][j][1]));
                    sprintf(b, "%d", (int)round(input_ppm[i][j][2]));
                    sprintf(str, "%s\t%s\t%s\n", r, g, b);
                    fputs(str, out_ppm); 
                }
            }
            fclose(out_ppm);
        }
        //=====================================================
        return 0;
    }
}



