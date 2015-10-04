#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



//投影する頂点の数
#define VER_NUM 5
//ポリゴンの三角形面の数
#define SUR_NUM 4


//メモリ内に画像の描画領域を確保
double image[256][256][3];

//投影された後の2次元平面上の各点の座標を格納する
double projected_ver[VER_NUM][2];

int array[4];

//=====================================================================
//ポリゴンデータ
//av1.wrl
//point

//example
/* double ver[VER_NUM][3] = { */
/*     {0, 0, 400}, */
/*     {-200, 0, 500}, */
/*     {0, 150, 500}, */
/*     {200, 0, 500}, */
/*     {0, -150, 500} */
/* }; */
double ver[VER_NUM][3];

//coordIndex
const int sur[SUR_NUM][3] = {
    {0, 1, 2},
    {0, 2, 3},
    {0, 3, 4},
    {0, 4, 1}
};
//diffuseColor
double diffuse_color[3] = {1.0, 1.0, 0.0};
//=====================================================================



//2点p、qを結ぶ直線上のy座標がyであるような点のx座標を返す関数
//eg)
//double p[2] = (1.0, 2.0);
double func1(double *p, double *q, double y){
    double x;
    if(p[1] > q[1]){
        x = ((p[0] * (y - q[1])) + (q[0] * (p[1] - y))) / (p[1] - q[1]);
    }
    if(p[1] < q[1]){
        x = ((q[0] * (y - p[1])) + (p[0] * (q[1] - y))) / (q[1] - p[1]);
    }
    if(p[1] == q[1]){
        //解なし
        printf("2点\n(%f, %f)\n(%f, %f)\nはy座標が同じ.", p[0], p[1], q[0], q[1]);
        perror(NULL);
        return -1;
    }
    //printf("check x = %f\n", x);
    //printf("check p[0] = %f\n", p[0]);
    return x;
}

//3点a[2] = {x, y},,,が1直線上にあるかどうかを判定する関数
//1直線上に無ければreturn 0;
//1直線上にあればreturn 1;
int lineOrNot(double *a, double *b, double *c){
    if(a[0] == b[0]){
        if(a[0] == c[0]){
            return 1;
        }
        else{
            return 0;
        }
    }
    else{
        if(c[1] == a[1] + ((b[1] - a[1]) / (b[0] - a[0])) * (c[0] - a[0])){
            return 1;
        }
        else{
            return 0;
        }
    }
}

//頂点座標の配列verに透視投影を行う関数
void perspective_pro(){
    for(int i = 0; i < VER_NUM; i++){
        double xp = ver[i][0];
        double yp = ver[i][1];
        double zp = ver[i][2];
        double zi = 256.0;
        
        double xp2 = xp * (zi / zp);
        double yp2 = yp * (zi / zp);
        double zp2 = zi;
        
        projected_ver[i][0] = xp2;
        projected_ver[i][1] = yp2;
    }
}

//投影された三角形abcにラスタライズ、クリッピングでシェーディングを行う関数
//引数は投影平面上の3点
//eg)
//double a = {1.0, 2.0};
void shading(double *a, double *b, double *c){
    //3点が1直線上に並んでいるときはシェーディングができない
    if(lineOrNot(a, b, c) == 1){
        //塗りつぶす点が無いので何もしない.
    }
    else{
        //三角形を分割
        //y座標の値が真ん中点をp、その他の点をq、rとする
        //y座標の大きさはr <= p <= qの順
        double p[2], q[2], r[2];
        if(b[1] <= a[1] && a[1] <= c[1]){
            memcpy(p, a, sizeof(double) * 2);
            memcpy(q, c, sizeof(double) * 2);
            memcpy(r, b, sizeof(double) * 2);
        }
        else{
            if(c[1] <= a[1] && a[1] <= b[1]){
                memcpy(p, a, sizeof(double) * 2);
                memcpy(q, b, sizeof(double) * 2);
                memcpy(r, c, sizeof(double) * 2);
            }
            else{
                if(a[1] <= b[1] && b[1] <= c[1]){
                    memcpy(p, b, sizeof(double) * 2);
                    memcpy(q, c, sizeof(double) * 2);
                    memcpy(r, a, sizeof(double) * 2);
                }
                else{
                    if(c[1] <= b[1] && b[1] <= a[1]){
                        memcpy(p, b, sizeof(double) * 2);
                        memcpy(q, a, sizeof(double) * 2);
                        memcpy(r, c, sizeof(double) * 2);
                    }
                    else{
                        if(b[1] <= c[1] && c[1] <= a[1]){
                            memcpy(p, c, sizeof(double) * 2);
                            memcpy(q, a, sizeof(double) * 2);
                            memcpy(r, b, sizeof(double) * 2);
                        }
                        else{
                            if(a[1] <= c[1] && c[1] <= b[1]){
                                memcpy(p, c, sizeof(double) * 2);
                                memcpy(q, b, sizeof(double) * 2);
                                memcpy(r, a, sizeof(double) * 2);
                            }
                            else{
                                printf("エラー発生\n");
                                perror(NULL);
                            }
                        }
                    }
                }
            }
        }
        //三角形を分割
        //分割後の三角形はpp2qとpp2r
        double p2[2];
        //printf("check2!!!!\n");
        //printf("p[0] = %f\n", p[0]);
        //printf("p[1] = %f\n", p[1]);
        
        p2[0] = func1(q, r, p[1]);
        p2[1] = p[1];

        //p2のほうがpのx座標より大きくなるようにする
        if(p2[0] < p[0]){
            double temp[2];
            memcpy(temp, p2, 2);
            memcpy(p2, p, 2);
            memcpy(p, temp, 2); 
        }
        
        //三角形pp2qをシェーディング
        //y座標はp <= q
        int i;
        i = ceil(p[1]);
        
        for(i; p[1] <= i && i <= q[1]; i++){
            double x1 = func1(p, q, i);
            double x2 = func1(p2, q, i);
            int j;
            j = ceil(x1);
            
            for(j; x1 <= j && j <= x2; j++){
                image[i][j][0] = 1.0;
                image[i][j][1] = 1.0;
                image[i][j][2] = 0.0;
            }
        }


        //三角形pp2rをシェーディング
        i = ceil(r[1]);
        
        for(i; r[1] <= i && i <= p[1]; i++){
            double x1 = func1(p, r, i);
            double x2 = func1(p2, r, i);
            int j;
            j = ceil(x1);
            
            for(j; x1 <= j && j <= x2; j++){
                image[i][j][0] = 1.0;
                image[i][j][1] = 1.0;
                image[i][j][2] = 0.0;
            }
        } 
    }
}

int main(void){
    FILE *fp;
    char *fname = "test.txt";
    
    fp = fopen( fname, "w" );
    //ファイルが開けなかったとき
    if( fp == NULL ){
        printf("%sファイルが開けません.\n", fname );
        return -1;
    }
  
    //ファイルが開けたとき
    else{
        //ポリゴンデータに乱数を代入
        srand(10);
        //頂点座標をランダムに設定
        for(int i = 0; i < VER_NUM; i++){
            ver[i][0] = rand() % 256 + 1;
            ver[i][1] = rand() % 256 + 1;
            ver[i][2] = rand() % 256 + 1;
        }

        printf("\n頂点座標は以下\n");
        for(int i = 0; i < VER_NUM; i++){
            printf("%f\t%f\t%f\n", ver[i][0], ver[i][1], ver[i][2]);
        }
        printf("\n");
        
        //描画領域を初期化
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                for(int k = 0; k < 3; k++){
                    image[i][j][k] = 0.0;
                }
            }
        }
     
        //ヘッダー出力
        char *magic_num = "P3\n";
        char *size = "256 256\n";
        char *color = "256\n";
        fputs(magic_num, fp);
        fputs(size, fp);
        fputs(color, fp);

        //各点の透視投影処理
        perspective_pro();
       
        printf("\n透視投影後のprojected_verの値\n");
        for(int i = 0; i < VER_NUM; i++){
            printf("%f\t%f\n", projected_ver[i][0], projected_ver[i][1]);
        }
        printf("\n");

        //シェーディング
        for(int i = 0; i < SUR_NUM; i++){
            double a[2], b[2], c[2];
            printf("shading debug!\n");
            printf("i = %d\n", i);
            
            a[0] = projected_ver[(sur[i][0])][0];
            a[1] = projected_ver[(sur[i][0])][1];
            printf("sur[i][0] = %d\n", sur[i][0]);
            printf("projected_ver[(sur[i][0])])[0] = %f\n", projected_ver[(sur[i][0])][0]);
            
            b[0] = projected_ver[(sur[i][1])][0];
            b[1] = projected_ver[(sur[i][1])][1];
            printf("sur[i][1] = %d\n", sur[i][1]);
            printf("projected_ver[(sur[i][1])])[0] = %f\n", projected_ver[(sur[i][1])][0]);
            
            c[0] = projected_ver[(sur[i][2])][0];
            c[1] = projected_ver[(sur[i][2])][1];


            //printf("dee %d\n", sur[i][0], projected_ver[(sur[i][0])][0]);
            //printf("%f\t%f\t%d\n", a[0], a[1], i);
            shading(a, b, c);
            //printf("%f\t%f\t%d\n", a[0], a[1], i);
        }

        //imageの出力
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                char ver_x[256];
                char ver_y[256];
                char ver_z[256];
                char str[1024];
                    
                sprintf(ver_x, "%lf", image[i][j][0]);
                sprintf(ver_y, "%lf", image[i][j][1]);
                sprintf(ver_z, "%lf", image[i][j][2]);
                sprintf(str, "%s\t%s\t%s\n", ver_x, ver_y, ver_z);
                fputs(str, fp); 
            }
        }
        
        
      
    }
    fclose(fp);

    printf("ppmファイル %s の作成が完了しました.\n", fname );
    return 0;
}



