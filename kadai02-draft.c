#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>



//=====================================================================
//必要なデータ
#define FILENAME "image.ppm"
#define MAGICNUM "P3"
#define WIDTH 256
#define WIDTH_STRING "256"
#define HEIGHT 256
#define HEIGHT_STRING "256"
#define MAX 255
#define MAX_STRING "255"
#define FOCUS 256.0

//パターン1=======================
/* #define VER_NUM 5 */
/* #define SUR_NUM 4 */
/* const double ver[VER_NUM][3] = { */
/*     {0, 0, 400}, */
/*     {-200, 0, 500}, */
/*     {0, 150, 500}, */
/*     {200, 0, 500}, */
/*     {0, -150, 500} */
/* }; */
/* const int sur[SUR_NUM][3] = { */
/*     {0, 1, 2}, */
/*     {0, 2, 3}, */
/*     {0, 3, 4}, */
/*     {0, 4, 1} */
/* }; */
//================================


//パターン2=======================
/* #define VER_NUM 6 */
/* #define SUR_NUM 2 */
/* const double ver[VER_NUM][3] = { */
/*     {-200, 0, 500}, */
/*     {200, -100, 500}, */
/*     {100, -200, 400}, */
/*     {-100, -100, 500}, */
/*     {50, 200,  400}, */
/*     {100, 100, 500} */
/* }; */
/* const int sur[SUR_NUM][3] = { */
/*     {0, 1, 2}, */
/*     {3, 4, 5}, */
/* }; */
//================================



//パターン3（ランダム座標）=======
#define VER_NUM 5
#define SUR_NUM 4

//ランダムな座標を格納するための領域を確保
//頂点座標はmain関数内で格納
double ver[VER_NUM][3];

const int sur[SUR_NUM][3] = {
    {0, 1, 2},
    {0, 2, 3},
    {0, 3, 4},
    {0, 4, 1}
};
//================================


//diffuseColor
const double diffuse_color[3] = {0.0, 1.0, 0.0};

//光源モデルは平行光源
//光源方向
const double light_dir[3] = {-1.0, -1.0, 2.0};
//光源明るさ
const double light_rgb[3] = {1.0, 1.0, 1.0};
//=====================================================================


//メモリ内に画像の描画領域を確保
double image[HEIGHT][WIDTH][3];

//投影された後の2次元平面上の各点の座標を格納する領域
double projected_ver[VER_NUM][2];



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
        printf("\n引数が不正です.\n2点\n(%f, %f)\n(%f, %f)\nはy座標が同じです.\n"
               , p[0], p[1], q[0], q[1]);
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

        //座標軸を平行移動
        //projected_ver[i][0] = xp2;
        //projected_ver[i][1] = yp2;
        projected_ver[i][0] = (MAX / 2) + xp2;
        projected_ver[i][1] = (MAX / 2) + yp2;
    }
}

//投影された三角形abcにラスタライズ、クリッピングでシェーディングを行う関数
//引数a, b, cは投影平面上の3点
//eg)
//double a = {1.0, 2.0};
//nは法線ベクトル
void shading(double *a, double *b, double *c, double *n){
    //3点が1直線上に並んでいるときはシェーディングができない
    if(lineOrNot(a, b, c) == 1){
        //塗りつぶす点が無いので何もしない.
        
        //debug
        /* printf("\n3点\naの座標(%f,\t%f)\nbの座標(%f,\t%f)\ncの座標(%f,\t%f)\nは一直線上の3点です\n" */
        /*        ,a[0], a[1], b[0], b[1], c[0], c[1]); */
        
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

        //debug
        /* printf("\n3点の座標は\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n" */
        /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
        
        //分割可能な三角形かを判定
        if(p[1] == r[1] || p[1] == q[1]){
            //分割できない

            //debug
            /* printf("\n三角形\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\nは分割できないのでこのままシェーディング\n" */
            /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */

            
            //長さが1の光源方向ベクトルを作成する
            //光源方向ベクトルの長さ
            double length_l =
                sqrt(pow(light_dir[0], 2.0) +
                     pow(light_dir[1], 2.0) +
                     pow(light_dir[2], 2.0));
            
            double light_dir_vec[3];
            light_dir_vec[0] = light_dir[0] / length_l;
            light_dir_vec[1] = light_dir[1] / length_l;
            light_dir_vec[2] = light_dir[2] / length_l;
            
            // 法線ベクトルnと光源方向ベクトルの内積
            double ip =
                (n[0] * light_dir_vec[0]) +
                (n[1] * light_dir_vec[1]) +
                (n[2] * light_dir_vec[2]);
                            
            //2パターンの三角形を特定
            if(p[1] == r[1]){
                //x座標が p <= r となるように調整
                if(r[0] <  p[0]){
                    double temp[2];
                    memcpy(temp, r, sizeof(double) * 2);
                    memcpy(r, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);
                    
                    //debug
                    /* printf("\n交換後の3点の座標は\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n" */
                    /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
                    
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
                    
                        image[i][j][0] =
                            -1 * ip * diffuse_color[0] *
                            light_rgb[0] * MAX;
                        image[i][j][1] =
                            -1 * ip * diffuse_color[1] *
                            light_rgb[1] * MAX;
                        image[i][j][2] =
                            -1 * ip * diffuse_color[2] *
                            light_rgb[2] * MAX;
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
                    /* printf("\n交換後の3点の座標は\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n" */
                    /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
                    
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

                        image[i][j][0] =
                            -1 * ip * diffuse_color[0] *
                            light_rgb[0] * MAX;
                        image[i][j][1] =
                            -1 * ip * diffuse_color[1] *
                            light_rgb[1] * MAX;
                        image[i][j][2] =
                            -1 * ip * diffuse_color[2] *
                            light_rgb[2] * MAX;
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
            //分割しても法線ベクトルは同一
            shading(p, p2, q, n);
            shading(p, p2, r, n);
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
        srand(10);
        for(int i = 0; i < VER_NUM; i++){
            ver[i][0] = rand() % 80;
            ver[i][1] = rand() % 80;
            ver[i][2] = rand() % 50 + 30;
        }

        fprintf(stderr, "\n初期の頂点座標は以下\n");
        for(int i = 0; i < VER_NUM; i++){
            fprintf(stderr, "%f\t%f\t%f\n", ver[i][0], ver[i][1], ver[i][2]);
        }
        fprintf(stderr, "\n");
        
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
       
        /* printf("\n撮像領域上の各点の座標のprojected_verの値\n"); */
        /* for(int i = 0; i < VER_NUM; i++){ */
        /*     printf("%f\t%f\n", projected_ver[i][0], projected_ver[i][1]); */
        /* } */
        /* printf("\n"); */

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
            /* printf("\n3点\naの座標(%f,\t%f)\nbの座標(%f,\t%f)\ncの座標(%f,\t%f)\nのシェーディングを行います.\n" */
            /*        ,a[0], a[1], b[0], b[1], c[0], c[1]); */


            //冗長な処理
            //透視投影処理の際に法線ベクトル、
            //光源からの距離を同時に求めておけばよかったが
            //今更変更できないのでここで再び求める
            
            //法線ベクトルを計算
            //投影前の3点の座標を取得
            double A[3], B[3], C[3];
            A[0] = ver[(sur[i][0])][0];
            A[1] = ver[(sur[i][0])][1];
            A[2] = ver[(sur[i][0])][2];
            
            B[0] = ver[(sur[i][1])][0];
            B[1] = ver[(sur[i][1])][1];
            B[2] = ver[(sur[i][1])][2];
            
            C[0] = ver[(sur[i][2])][0];
            C[1] = ver[(sur[i][2])][1];
            C[2] = ver[(sur[i][2])][2];

            //debug
            /* printf("\n3次元空間内の3点の座標は\n(%f,\t%f,\t%f)\n(%f,\t%f,\t%f)\n(%f,\t%f,\t%f)\nです\n", */
            /*        A[0], A[1], A[2], */
            /*        B[0], B[1], B[2], */
            /*        C[0], C[1], C[2]); */
            
            //ベクトルAB, ACから外積を計算して
            //法線ベクトルnを求める
            double AB[3], AC[3], n[3];
            AB[0] = B[0] - A[0];
            AB[1] = B[1] - A[1];
            AB[2] = B[2] - A[2];
            
            AC[0] = C[0] - A[0];
            AC[1] = C[1] - A[1];
            AC[2] = C[2] - A[2];

            n[0] = (AB[1] * AC[2]) - (AB[2] * AC[1]);
            n[1] = (AB[2] * AC[0]) - (AB[0] * AC[2]);
            n[2] = (AB[0] * AC[1]) - (AB[1] * AC[0]);

            //長さを1に調整
            double length_n =
                sqrt(pow(n[0], 2.0) +
                     pow(n[1], 2.0) +
                     pow(n[2], 2.0));
        
            n[0] = n[0] / length_n;
            n[1] = n[1] / length_n;
            n[2] = n[2] / length_n;

            //debug
            /* printf("\n法線ベクトルは\n(%f,\t%f,\t%f)\nです\n", */
            /*        n[0], n[1], n[2]); */

            
            //平面iの投影先の三角形をシェーディング
            shading(a, b, c, n);
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



