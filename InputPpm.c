#include <stdio.h>
#include <stdlib.h>/*exitを使用するためのヘッダファイル*/
#include <string.h>
#include <float.h>
#define MAXOFARRAYSIZE 20000
#define FILENAME "./sample/data.txt"/*数値データの入ったファイル名*/
#define MAX 1024

typedef void(*ARRAY_FUNC)(long array[],int x,int y);
//関数ポインタを要素に持つ配列の宣言を簡略化するため



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
//////////////////////////////////////
//////////////////////////////////////
//////////////////////////////////////
int main(void){  
    char *fname = FILENAME;
    FILE *ip;
    long array[MAXOFARRAYSIZE];  
    ip = fopen(fname,"r");
    if(ip == NULL){ 
        fprintf(stderr, "%sを正常に開くことが出来ませんでした.\n" ,FILENAME);
        exit(1);/*異常終了*/
    }
    
    else{
        printf("%sの数値を読み込みます。\n",FILENAME);


        //=============================================================
        char buf[MAX];
        char *tp;
        char *buf_p;
        int i = 0;

        //全て改行で区切られているものと制限する
        //コメントはエラーが出てしまう
        //マジックナンバーを取得========================
        char magic_num[3];
        fgets(buf,sizeof(buf[MAX]),ip);
        magic_num = buf;
        //===========================================
        
        //WIDTH、HEIGHTを取得=========================
        int ppm_width, ppm_height;
        fgets(buf,sizeof(buf),ip);
        ppm_width= atoi(buf);
        fgets(buf,sizeof(buf),ip);
        ppm_height= atoi(buf);
        //===========================================
        
        //上限値を取得=================================
        int ppm_max;
        fgets(buf,sizeof(buf[MAX]),ip);
        ppm_masx = atoi(buf);
        //===========================================

        
        while(1){
            char *buf_add = fgets(buf,sizeof(buf),ip);
            if(buf_add == NULL){
                break;
            }
            else{
                //bufの中に改行が含まれているとき
                if(strchr(buf, '\n')){
                    
                }
                //bufの中に改行が含まれていないとき
                /*strtokでbufから数値列を空白で分解していく*/
                buf_p = buf;
                tp = strtok(buf_p," ");
                array[i] = atoi(tp);
                while(tp != NULL){
                    tp = strtok(NULL," ");
                    i++;
                    if(tp != NULL){
                        array[i] = atoi(tp);
                    }
                }
            }
        }
         //=============================================================

        
        fclose(ip);
        printf("%sからの数値の読み込みが完了しました。\n",FILENAME);
        print_array(array,i);
        printf("すべての操作が完了しました。\n");
    }
}
//////////////////////////////////
