#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define FILENAME "test.ppm"
#define MAGICNUM "P3"
#define WIDTH 256
#define WIDTH_STRING "256"
#define HEIGHT 256
#define HEIGHT_STRING "256"
#define MAX 255
#define MAX_STRING "255"
#define FOCUS 256.0



//投影する頂点の数
//パターン1
//#define VER_NUM 5
//パターン2
#define VER_NUM 6

//ポリゴンの三角形面の数erm
//パターン1
//#define SUR_NUM 4
//パターン2
#define SUR_NUM 2



//メモリ内に画像の描画領域を確保
double image[256][256][3];

//投影された後の2次元平面上の各点の座標を格納する
double projected_ver[VER_NUM][2];


//=====================================================================
//ポリゴンデータ
//av1.wrl
//point

//example
//パターン1
/* double ver[VER_NUM][3] = { */
/*     {0, 0, 400}, */
/*     {-200, 0, 500}, */
/*     {0, 150, 500}, */
/*     {200, 0, 500}, */
/*     {0, -150, 500} */
/* }; */

//パターン2
double ver[VER_NUM][3] = {
    {-200, 0, 500},
    {200, -100, 500},
    {100, -200, 400},
    {-100, -100, 500},
    {50, 200,  400},
    {100, 100, 500}
};

//double ver[VER_NUM][3];//ランダムな座標を格納するための領域を確保

//coordIndex
//パターン1
/* const int sur[SUR_NUM][3] = { */
/*     {0, 1, 2}, */
/*     {0, 2, 3}, */
/*     {0, 3, 4}, */
/*     {0, 4, 1} */
/* }; */

//パターン2
const int sur[SUR_NUM][3] = {
    {0, 1, 2},
    {3, 4, 5},
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
        double zi = FOCUS;
        
        double xp2 = xp * (zi / zp);
        double yp2 = yp * (zi / zp);
        double zp2 = zi;
        
        //projected_ver[i][0] = xp2;
        //projected_ver[i][1] = yp2;
        projected_ver[i][0] = (MAX / 2) + xp2;
        projected_ver[i][1] = (MAX / 2) + yp2;
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
        //debug
        printf("\n3点\naの座標(%f,\t%f)\nbの座標(%f,\t%f)\ncの座標(%f,\t%f)\nは一直線上の3点です\n"
               ,a[0], a[1], b[0], b[1], c[0], c[1]);
    }
    else{
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
                                printf("エラー\n");
                                perror(NULL);
                            }
                        }
                    }
                }
            }
        }
        printf("\n3点の座標は\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n"
               ,p[0], p[1], q[0], q[1], r[0], r[1]);
        //分割可能な三角形かを判定
        if(p[1] == r[1] || p[1] == q[1]){
            //分割できない
            printf("\n三角形\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\nは分割できないのでこのままシェーディング\n"
                   ,p[0], p[1], q[0], q[1], r[0], r[1]);

            
            //2パターンの三角形を特定
            if(p[1] == r[1]){
                //x座標が p <= r となるように調整
                if(r[0] <  p[0]){
                    double temp[2];
                    memcpy(temp, r, sizeof(double) * 2);
                    memcpy(r, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);
                    //debug
                    printf("\n交換後の3点の座標は\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n"
                           ,p[0], p[1], q[0], q[1], r[0], r[1]);
                }
                //debug
                if(r[0] == p[0]){
                    perror("エラー958");
                }
                //シェーディング処理
                //シェーディングの際に画面からはみ出した部分をどう扱うか
                //以下の実装はxy座標の範囲を0 <= x, y <= 256として実装している
                //三角形pqrをシェーディング
                //y座標はp <= r
                //debug
                if(r[1] < p[1]){
                    perror("エラーat1855");
                }
                int i;
                i = ceil(p[1]);         
                for(i;
                    p[1] <= i && i <= q[1] && 0 <= i && i <= (HEIGHT - 1);
                    i++){
                    double x1 = func1(p, q, i);          
                    double x2 = func1(r, q, i);        
                    int j;
                    j = ceil(x1);
                    
                    for(j;
                        x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                        j++){
                        image[i][j][0] = 1.0 * MAX;
                        image[i][j][1] = 1.0 * MAX;
                        image[i][j][2] = 0.0 * MAX;
                    }
                }
                
            }
            
            if(p[1] == q[1]){
                //x座標が p < q となるように調整
                if(q[0] <  p[0]){
                    double temp[2];
                    memcpy(temp, q, sizeof(double) * 2);
                    memcpy(q, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);
                    //debug
                    printf("\n交換後の3点の座標は\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n"
                           ,p[0], p[1], q[0], q[1], r[0], r[1]);
                }
                //debug
                if(q[0] == p[0]){
                    perror("エラーat1011");
                }
                //シェーディング処理
                //三角形pqrをシェーディング
                //y座標はp <= q
                //debug
                if(q[1] < p[1]){
                    perror("エラーat1856");
                }
                int i;
                i = ceil(r[1]);

                
                for(i;
                    r[1] <= i && i <= p[1] && 0 <= i && i <= (HEIGHT - 1);
                    i++){       
                    double x1 = func1(p, r, i);          
                    double x2 = func1(q, r, i);

                    int j;
                    j = ceil(x1);
                    
                    for(j;
                        x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                        j++){
                        image[i][j][0] = 1.0 * MAX;
                        image[i][j][1] = 1.0 * MAX;
                        image[i][j][2] = 0.0 * MAX;
                    }
                }
            }
            
        }
        //分割できる
        //分割してそれぞれ再帰的に処理
        //分割後の三角形はpp2qとpp2r
        else{
            double p2[2];

            p2[0] = func1(q, r, p[1]);
            p2[1] = p[1];
            //p2のほうがpのx座標より大きくなるようにする
            if(p2[0] < p[0]){
                double temp[2];
                memcpy(temp, p2, sizeof(double) * 2);
                memcpy(p2, p, sizeof(double) * 2);
                memcpy(p, temp, sizeof(double) * 2); 
            }
            shading(p, p2, q);
            shading(p, p2, r);
        }   
    }
}

int main(void){
    FILE *fp;
    char *fname = FILENAME;
    
    fp = fopen( fname, "w" );
    //ファイルが開けなかったとき
    if( fp == NULL ){
        printf("%sファイルが開けません.\n", fname);
        return -1;
    }
  
    //ファイルが開けたとき
    else{
        //頂点座標をランダムに設定
        /* srand(10); */
        /* for(int i = 0; i < VER_NUM; i++){ */
        /*     ver[i][0] = rand() % 256 + 1; */
        /*     ver[i][1] = rand() % 256 + 1; */
        /*     ver[i][2] = rand() % 256 + 1; */
        /* } */

        printf("\n初期の頂点座標は以下\n");
        for(int i = 0; i < VER_NUM; i++){
            printf("%f\t%f\t%f\n", ver[i][0], ver[i][1], ver[i][2]);
        }
        printf("\n");
        
        //描画領域を初期化=======================================
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                image[i][j][0] = 0.0 * MAX;
                image[i][j][1] = 0.0 * MAX;
                image[i][j][2] = 0.0 * MAX;
            }
        }
        //=====================================================
     
        //ヘッダー出力
        fputs(MAGICNUM, fp);
        fputs("\n", fp);
        fputs(WIDTH_STRING, fp);
        fputs(" ", fp);
        fputs(HEIGHT_STRING, fp);
        fputs("\n", fp);
        fputs(MAX_STRING, fp);
        fputs("\n" ,fp);

        //各点の透視投影処理
        perspective_pro();
       
        printf("\n撮像領域上の各点の座標のprojected_verの値\n");
        for(int i = 0; i < VER_NUM; i++){
            printf("%f\t%f\n", projected_ver[i][0], projected_ver[i][1]);
        }
        printf("\n");

        //シェーディング
        for(int i = 0; i < SUR_NUM; i++){
            double a[2], b[2], c[2];
            
            a[0] = projected_ver[(sur[i][0])][0];
            a[1] = projected_ver[(sur[i][0])][1];            
            b[0] = projected_ver[(sur[i][1])][0];
            b[1] = projected_ver[(sur[i][1])][1];            
            c[0] = projected_ver[(sur[i][2])][0];
            c[1] = projected_ver[(sur[i][2])][1];
            //debug
            printf("\n3点\naの座標(%f,\t%f)\nbの座標(%f,\t%f)\ncの座標(%f,\t%f)\nのシェーディングを行います.\n"
                   ,a[0], a[1], b[0], b[1], c[0], c[1]);
            shading(a, b, c);
        }

        //imageの出力
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                char r[256];
                char g[256];
                char b[256];
                char str[1024];
                    
                sprintf(r, "%d", (int)round(image[i][j][0]));
                sprintf(g, "%d", (int)round(image[i][j][1]));
                sprintf(b, "%d", (int)round(image[i][j][2]));
                sprintf(str, "%s\t%s\t%s\n", r, g, b);
                fputs(str, fp); 
            }
        }
        
        
      
    }
    fclose(fp);

    printf("\nppmファイル %s の作成が完了しました.\n", fname );
    return 0;
}



