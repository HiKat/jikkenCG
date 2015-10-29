#include <stdio.h>
#include <stdlib.h>/*exitを使用するためのヘッダファイル*/
#include <string.h>
#include <float.h>
#define MAXOFARRAYSIZE 20000
#define FILENAME "sorting.data"/*数値データの入ったファイル名*/
#define MAX 32767

typedef void(*ARRAY_FUNC)(long array[],int x,int y);
//関数ポインタを要素に持つ配列の宣言を簡略化するため


///////////////////////////////////////
int make_array(FILE *ip,long array[]){
    if(ip == NULL){ 
        fprintf(stderr,
                "%sを正常に開くことが出来ませんでした.\n"
                ,FILENAME);
        exit(1);/*異常終了*/
    } 
    printf("%sの数値を読み込みます。\n",FILENAME);
    char buf[MAX];
    char *tp;
    char *buf_p;
    int i = 0;
    while(1){
        if(fgets(buf,sizeof(buf),ip) == NULL){
            break;
        }
        else{
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
    fclose(ip);
    printf("%sからの数値の読み込みが完了しました。\n",FILENAME);
    return i;
}
//////////////////////////////////////
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
    /*読み込み用にmylecture.txtを開く*/
    char *fname = FILENAME;
    FILE *ip;
    ip = fopen(fname,"r");
    long array[MAXOFARRAYSIZE];
    int arraysize = make_array(ip,array);
    print_array(array,arraysize);
    printf("すべての操作が完了しました。\n");
}
//////////////////////////////////
