#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "vrml.h"




//=====================================================================
//必要なデータ
#define MAGICNUM "P3"
#define WIDTH 256
#define WIDTH_STRING "256"
#define HEIGHT 256
#define HEIGHT_STRING "256"
#define MAX 255
#define MAX_STRING "255"
#define FOCUS 256.0
#define Z_BUF_MAX
#define ENV_LIGHT 1.0

//diffuseColorを格納する配列
double diffuse_color[3];
//shinessを格納する変数
double shininess;
//specularColorを格納する変数
double specular_color[3];

//光源モデルは平行光源

//光源方向
const double light_dir[3] = {-1.0, -1.0, 2.0};
//光源明るさ
const double light_rgb[3] = {1.0, 1.0, 1.0};

//カメラ位置は原点であるものとして投影を行う.

//=====================================================================


//メモリ内に画像の描画領域を確保
double image[HEIGHT][WIDTH][3];
//zバッファ用の領域を確保
double z_buf[HEIGHT][WIDTH];

//投影された後の2次元平面上の各点の座標を格納する領域
//double projected_ver[VER_NUM][2];
double projected_ver_buf[3][2];


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


//引数は3点の座標とRGBと3点の空間内の座標、3点で形成される空間内の平面の法線ベクトルとする
void shading(double *a, double *b, double *c,
             double *rgb_a, double *rgb_b, double *rgb_c, 
             double *A, double *B, double *C, 
             double *poly_i_n_vec){
    //3点が1直線上に並んでいるときはシェーディングができない
    if(lineOrNot(a, b, c) == 1){}
    else{
        //y座標の値が真ん中点をp、その他の点をq、rとする
        //y座標の大きさはr <= p <= qの順
        double p[2], q[2], r[2];
        //法線ベクトルも名前を変更する
        double rgb_p[3], rgb_q[3], rgb_r[3];
        //空間内での元の座標についても名前を変更する
        double P[3], Q[3], R[3];

        if(b[1] <= a[1] && a[1] <= c[1]){
            memcpy(p, a, sizeof(double) * 2);
            memcpy(q, c, sizeof(double) * 2);
            memcpy(r, b, sizeof(double) * 2);
            
            memcpy(rgb_p, rgb_a, sizeof(double) * 3);
            memcpy(rgb_q, rgb_c, sizeof(double) * 3);
            memcpy(rgb_r, rgb_b, sizeof(double) * 3);

            memcpy(P, A, sizeof(double) * 3);
            memcpy(Q, C, sizeof(double) * 3);
            memcpy(R, B, sizeof(double) * 3);
        }
        else{
            if(c[1] <= a[1] && a[1] <= b[1]){
                memcpy(p, a, sizeof(double) * 2);
                memcpy(q, b, sizeof(double) * 2);
                memcpy(r, c, sizeof(double) * 2);
                
                memcpy(rgb_p, rgb_a, sizeof(double) * 3);
                memcpy(rgb_q, rgb_b, sizeof(double) * 3);
                memcpy(rgb_r, rgb_c, sizeof(double) * 3);

                memcpy(P, A, sizeof(double) * 3);
                memcpy(Q, B, sizeof(double) * 3);
                memcpy(R, C, sizeof(double) * 3);

            }
            else{
                if(a[1] <= b[1] && b[1] <= c[1]){
                    memcpy(p, b, sizeof(double) * 2);
                    memcpy(q, c, sizeof(double) * 2);
                    memcpy(r, a, sizeof(double) * 2);
                    
                    memcpy(rgb_p, rgb_b, sizeof(double) * 3);
                    memcpy(rgb_q, rgb_c, sizeof(double) * 3);
                    memcpy(rgb_r, rgb_a, sizeof(double) * 3);

                    memcpy(P, B, sizeof(double) * 3);
                    memcpy(Q, C, sizeof(double) * 3);
                    memcpy(R, A, sizeof(double) * 3);
                }
                else{
                    if(c[1] <= b[1] && b[1] <= a[1]){
                        memcpy(p, b, sizeof(double) * 2);
                        memcpy(q, a, sizeof(double) * 2);
                        memcpy(r, c, sizeof(double) * 2);
                        
                        memcpy(rgb_p, rgb_b, sizeof(double) * 3);
                        memcpy(rgb_q, rgb_a, sizeof(double) * 3);
                        memcpy(rgb_r, rgb_c, sizeof(double) * 3);

                        memcpy(P, B, sizeof(double) * 3);
                        memcpy(Q, A, sizeof(double) * 3);
                        memcpy(R, C, sizeof(double) * 3);
                    }
                    else{
                        if(b[1] <= c[1] && c[1] <= a[1]){
                            memcpy(p, c, sizeof(double) * 2);
                            memcpy(q, a, sizeof(double) * 2);
                            memcpy(r, b, sizeof(double) * 2);

                            memcpy(rgb_p, rgb_c, sizeof(double) * 3);
                            memcpy(rgb_q, rgb_a, sizeof(double) * 3);
                            memcpy(rgb_r, rgb_b, sizeof(double) * 3);

                            memcpy(P, C, sizeof(double) * 3);
                            memcpy(Q, A, sizeof(double) * 3);
                            memcpy(R, B, sizeof(double) * 3);
                        }
                        else{
                            if(a[1] <= c[1] && c[1] <= b[1]){
                                memcpy(p, c, sizeof(double) * 2);
                                memcpy(q, b, sizeof(double) * 2);
                                memcpy(r, a, sizeof(double) * 2);

                                memcpy(rgb_p, rgb_c, sizeof(double) * 3);
                                memcpy(rgb_q, rgb_b, sizeof(double) * 3);
                                memcpy(rgb_r, rgb_a, sizeof(double) * 3);

                                memcpy(P, C, sizeof(double) * 3);
                                memcpy(Q, B, sizeof(double) * 3);
                                memcpy(R, A, sizeof(double) * 3);

                            }
                            else{
                                printf("エラーat2055\n");
                                printf("\na[1]=%f\tb[1]=%f\tc[1]=%f\n", a[1], b[1], c[1]);
                                perror(NULL);
                            }
                        }
                    }
                }
            }
        }        
        //分割可能な三角形かを判定
        if(p[1] == r[1] || p[1] == q[1]){
            //分割できない
            
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
                            
            //2パターンの三角形を特定
            //Type 1
            if(p[1] == r[1]){
                //debug
                //printf("\np[1] == r[1]\n");
                //x座標が p <= r となるように調整
                if(r[0] <  p[0]){
                    double temp[2];
                    double temp_rgb[3];
                    memcpy(temp, r, sizeof(double) * 2);
                    memcpy(r, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);

                    memcpy(temp_rgb, rgb_r, sizeof(double) * 3);
                    memcpy(rgb_r, rgb_p, sizeof(double) * 3);
                    memcpy(rgb_p, temp_rgb, sizeof(double) * 3);
                }
                
                //debug
                if(r[0] == p[0]){
                  perror("エラーat958");
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

                //zバッファを確認しながら3点pqrについて先にシェーディングで色をぬる
                int temp_p0 = ceil(p[0]);
                int temp_p1 = ceil(p[1]);             
                if(z_buf[temp_p1][temp_p0] < P[2]){
                    //描画しない
                }
                else{
                    image[temp_p1][temp_p0][0] = rgb_p[0];
                    image[temp_p1][temp_p0][1] = rgb_p[1];
                    image[temp_p1][temp_p0][2] = rgb_p[2];
                }

                int temp_q0 = ceil(q[0]);
                int temp_q1 = ceil(q[1]);             
                if(z_buf[temp_q1][temp_q0] < Q[2]){
                    //描画しない
                }
                else{
                    image[temp_q1][temp_q0][0] = rgb_q[0];
                    image[temp_q1][temp_q0][1] = rgb_q[1];
                    image[temp_q1][temp_q0][2] = rgb_q[2];
                }

                int temp_r0 = ceil(r[0]);
                int temp_r1 = ceil(r[1]);             
                if(z_buf[temp_r1][temp_r0] < R[2]){
                    //描画しない
                }
                else{
                    image[temp_r1][temp_r0][0] = rgb_r[0];
                    image[temp_r1][temp_r0][1] = rgb_r[1];
                    image[temp_r1][temp_r0][2] = rgb_r[2];
                }
                
                
                int i;
                i = ceil(p[1]);
                for(i;
                    p[1] <= i && i <= q[1];
                    i++){

                    //撮像平面からはみ出ていないかのチェック
                    if(0 <= i
                       &&
                       i <= (HEIGHT - 1)){
                           double x1 = func1(p, q, i);
                           double x2 = func1(r, q, i);
                           int j;
                           j = ceil(x1);
                           
                           for(j;
                               x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                               j++){
                               
      
                               //===========================================
                               //p[1] == r[1]
                               //描画する点の空間内でのz座標を計算
                               //計算時の法線ベクトルは
                               double p_z =
                                   FOCUS
                                   *
                                   ((poly_i_n_vec[0]*A[0]) +
                                    (poly_i_n_vec[1]*A[1]) +
                                    (poly_i_n_vec[2]*A[2]))
                                   /
                                   ((poly_i_n_vec[0]*(j-(MAX/2))) +
                                    (poly_i_n_vec[1]*(i-(MAX/2))) +
                                    (poly_i_n_vec[2]*FOCUS));
                               
                               //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                               if(z_buf[i][j] < p_z){}

                               //Type 1
                               else{
                                   image[i][j][0] =
                                       (
                                        ((x2-j) / (x2-x1))
                                        *
                                        ((rgb_p[0]*(q[1]-i) + rgb_q[0]*(i-p[1])) / (q[1]-p[1]))
                                        )
                                       +
                                       (
                                        ((j-x1) / (x2-x1))
                                        *
                                        ((rgb_r[0]*(q[1]-i) + rgb_q[0]*(i-r[1])) / (q[1]-r[1]))
                                        );
                                       
                                   image[i][j][1] =
                                        (
                                        ((x2-j) / (x2-x1))
                                        *
                                        ((rgb_p[1]*(q[1]-i) + rgb_q[1]*(i-p[1])) / (q[1]-p[1]))
                                        )
                                       +
                                       (
                                        ((j-x1) / (x2-x1))
                                        *
                                        ((rgb_r[1]*(q[1]-i) + rgb_q[1]*(i-r[1])) / (q[1]-r[1]))
                                        );
                                   
                                   image[i][j][2] =
                                       (
                                        ((x2-j) / (x2-x1))
                                        *
                                        ((rgb_p[2]*(q[1]-i) + rgb_q[2]*(i-p[1])) / (q[1]-p[1]))
                                        )
                                       +
                                       (
                                        ((j-x1) / (x2-x1))
                                        *
                                        ((rgb_r[2]*(q[1]-i) + rgb_q[2]*(i-r[1])) / (q[1]-r[1]))
                                        );
                                   
                                   //zバッファの更新
                                   z_buf[i][j] = p_z;
                               }
                           }
                    }
                    //はみ出ている場合は描画しない
                    else{}
                }
                
            }
            
            if(p[1] == q[1]){
                //x座標が p < q となるように調整
                if(q[0] <  p[0]){
                    double temp[2];
                    double temp_rgb[3];                   
                    memcpy(temp, q, sizeof(double) * 2);
                    memcpy(q, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);

                    memcpy(temp_rgb, rgb_q, sizeof(double) * 3);
                    memcpy(rgb_q, rgb_p, sizeof(double) * 3);
                    memcpy(rgb_p, temp_rgb, sizeof(double) * 3);  
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
                
                //zバッファを確認しながら3点pqrについて先にシェーディングで色をぬる
                int temp_p0 = ceil(p[0]);
                int temp_p1 = ceil(p[1]);             
                if(z_buf[temp_p1][temp_p0] < P[2]){
                    //描画しない
                }
                else{
                    image[temp_p1][temp_p0][0] = rgb_p[0];
                    image[temp_p1][temp_p0][1] = rgb_p[1];
                    image[temp_p1][temp_p0][2] = rgb_p[2];
                }

                int temp_q0 = ceil(q[0]);
                int temp_q1 = ceil(q[1]);             
                if(z_buf[temp_q1][temp_q0] < Q[2]){
                    //描画しない
                }
                else{
                    image[temp_q1][temp_q0][0] = rgb_q[0];
                    image[temp_q1][temp_q0][1] = rgb_q[1];
                    image[temp_q1][temp_q0][2] = rgb_q[2];
                }

                int temp_r0 = ceil(r[0]);
                int temp_r1 = ceil(r[1]);             
                if(z_buf[temp_r1][temp_r0] < R[2]){
                    //描画しない
                }
                else{
                    image[temp_r1][temp_r0][0] = rgb_r[0];
                    image[temp_r1][temp_r0][1] = rgb_r[1];
                    image[temp_r1][temp_r0][2] = rgb_r[2];
                }
                
                int i;
                i = ceil(r[1]);
                for(i;
                    r[1] <= i && i <= p[1];
                    i++){

                    //撮像部分からはみ出ていないかのチェック
                    if( 0 <= i &&
                        i <= (HEIGHT - 1)){
                        double x1 = func1(p, r, i);
                        double x2 = func1(q, r, i);
                        
                        int j;
                        j = ceil(x1);

                        for(j;
                            x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                            j++){
                            
                            //=============================================================
                            double p_z =
                                FOCUS
                                *
                                ((poly_i_n_vec[0]*A[0]) +
                                 (poly_i_n_vec[1]*A[1]) +
                                 (poly_i_n_vec[2]*A[2]))
                                /
                                ((poly_i_n_vec[0]*(j-(MAX/2))) +
                                 (poly_i_n_vec[1]*(i-(MAX/2))) +
                                 poly_i_n_vec[2]*FOCUS);

                            //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                            if(z_buf[i][j] < p_z){}
                        
                            else{
                                //Type 2                         
                                image[i][j][0] =
                                    (
                                     ((x2-j) / (x2-x1))
                                     *
                                     ((rgb_p[0]*(i-r[1]) + rgb_r[0]*(p[1]-i)) / (p[1]-r[1]))
                                     )
                                    +
                                    (
                                     ((j-x1) / (x2-x1))
                                     *
                                     ((rgb_r[0]*(q[1]-i) + rgb_q[0]*(i-r[1])) / (q[1]-r[1]))
                                     );
                                
                                image[i][j][1] =
                                    (
                                     ((x2-j) / (x2-x1))
                                     *
                                     ((rgb_p[1]*(i-r[1]) + rgb_r[1]*(p[1]-i)) / (p[1]-r[1]))
                                     )
                                    +
                                    (
                                     ((j-x1) / (x2-x1))
                                     *
                                     ((rgb_r[1]*(q[1]-i) + rgb_q[1]*(i-r[1])) / (q[1]-r[1]))
                                     );
                                
                                image[i][j][2] =
                                    (
                                     ((x2-j) / (x2-x1))
                                     *
                                     ((rgb_p[2]*(i-r[1]) + rgb_r[2]*(p[1]-i)) / (p[1]-r[1]))
                                     )
                                    +
                                    (
                                     ((j-x1) / (x2-x1))
                                     *
                                     ((rgb_r[2]*(q[1]-i) + rgb_q[2]*(i-r[1])) / (q[1]-r[1]))
                                     );
                                //zバッファの更新
                                z_buf[i][j] = p_z;
                            }
                        }
                    }
                    //撮像平面からはみ出る部分は描画しない
                    else{}      
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
            
            double P2[3];
            P2[0] =
                (poly_i_n_vec[0]*(p2[0]-(MAX/2)))
                *
                ((poly_i_n_vec[0]*A[0]) +
                 (poly_i_n_vec[1]*A[1]) +
                 (poly_i_n_vec[2]*A[2]))
                /
                ((poly_i_n_vec[0]*(p2[0]-(MAX/2))) +
                 (poly_i_n_vec[1]*(p2[1]-(MAX/2))) +
                 poly_i_n_vec[2]*FOCUS);
            
            P2[1] =
                (poly_i_n_vec[1]*(p2[1]-(MAX/2)))
                *
                ((poly_i_n_vec[0]*A[0]) +
                 (poly_i_n_vec[1]*A[1]) +
                 (poly_i_n_vec[2]*A[2]))
                /
                ((poly_i_n_vec[0]*(p2[0]-(MAX/2))) +
                 (poly_i_n_vec[1]*(p2[1]-(MAX/2))) +
                 poly_i_n_vec[2]*FOCUS);
            
            P2[2] =
                FOCUS
                *
                ((poly_i_n_vec[0]*A[0]) +
                 (poly_i_n_vec[1]*A[1]) +
                 (poly_i_n_vec[2]*A[2]))
                /
                ((poly_i_n_vec[0]*(p2[0]-(MAX/2))) +
                 (poly_i_n_vec[1]*(p2[1]-(MAX/2))) +
                 poly_i_n_vec[2]*FOCUS);
            

            double rgb_p2[3];
            for(int i = 0; i < 3; i++){
                rgb_p2[i]
                    =
                    rgb_q[i] * ((p[1]-r[1])/(q[1]-r[1]))
                    +
                    rgb_r[i] * ((q[1]-p[1])/(q[1]-r[1]));
            }
                
            
            
            //p2のほうがpのx座標より大きくなるようにする
            if(p2[0] < p[0]){
                double temp[2];
                double temp_rgb[3];
                
                memcpy(temp, p2, sizeof(double) * 2);
                memcpy(p2, p, sizeof(double) * 2);
                memcpy(p, temp, sizeof(double) * 2);

                memcpy(temp_rgb, rgb_p2, sizeof(double) * 2);
                memcpy(rgb_p2, rgb_p, sizeof(double) * 2);
                memcpy(rgb_p, temp_rgb, sizeof(double) * 2);
            }
            //分割しても同一平面上なので法線ベクトルと
            //平面上の任意の点は同じものを使える.
            //求める必要があるのはrgb_p2とP2
            
            shading(p, p2, q, rgb_p, rgb_p2, rgb_q, P, P2, Q, poly_i_n_vec);
            shading(p, p2, r, rgb_p, rgb_p2, rgb_r, P, P2, R, poly_i_n_vec);
        }
    }
}

/*
/////////////////////////////////////////////////////////////////
*/
#define MWS 256

static int strindex( char *s, char *t)
{
    int         i, j, k;

    for (i = 0; s[i] != '\0'; i++) {
        for (j = i, k = 0;  t[k] != '\0' && s[j] == t[k]; j++, k++) ;
        if (k > 0 && t[k] == '\0')
            return i;
    }
    return -1;
}

static int getword(
                   FILE *fp,
                   char word[],
                   int sl)
{
    int i,c;

    while ( (c = fgetc(fp)) != EOF && ( isspace(c) || c == '#')) {
        if ( c == '#' ) {
            while ( (c = fgetc(fp)) != EOF && c != '\n') ;
            if ( c == EOF ) return (0);
        }
    }
    if ( c == EOF )
        return (0);
    ungetc(c,fp);

    for ( i = 0 ; i < sl - 1 ; i++) {
        word[i] = fgetc(fp);
        if ( isspace(word[i]) )
            break;
    }
    word[i] = '\0';

    return i;
}

static int read_material(
                         FILE *fp,
                         Surface *surface,
                         char *b)
{
    while (getword(fp,b,MWS)>0) {
        if      (strindex(b,"}")>=0) break;
        else if (strindex(b,"diffuseColor") >= 0) {
            getword(fp,b,MWS);
            surface->diff[0] = atof(b);
            getword(fp,b,MWS);
            surface->diff[1] = atof(b);
            getword(fp,b,MWS);
            surface->diff[2] = atof(b);
        }
        else if (strindex(b,"ambientIntensity") >= 0) {
            getword(fp,b,MWS);
            surface->ambi = atof(b);
        }
        else if (strindex(b,"specularColor") >= 0) {
            getword(fp,b,MWS);
            surface->spec[0] = atof(b);
            getword(fp,b,MWS);
            surface->spec[1] = atof(b);
            getword(fp,b,MWS);
            surface->spec[2] = atof(b);
        }
        else if (strindex(b,"shininess") >= 0) {
            getword(fp,b,MWS);
            surface->shine = atof(b);
        }
    }
    return 1;
}

static int count_point(
                       FILE *fp,
                       char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            num++;
        }
    }
    if ( num %3 != 0 ) {
        fprintf(stderr,"invalid file type[number of points mismatch]\n");
    }
    return num/3;
}

static int read_point(
                      FILE *fp,
                      Polygon *polygon,
                      char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            polygon->vtx[num++] = atof(b);
        }
    }
    return num/3;
}

static int count_index(
                       FILE *fp,
                       char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            num++;
        }
    }
    if ( num %4 != 0 ) {
        fprintf(stderr,"invalid file type[number of indices mismatch]\n");
    }
    return num/4;
}

static int read_index(
                      FILE *fp,
                      Polygon *polygon,
                      char *b)
{
    int num=0;
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"[")>=0) break;
    }
    while (getword(fp,b,MWS)>0) {
        if (strindex(b,"]")>=0) break;
        else {
            polygon->idx[num++] = atoi(b);
            if (num%3 == 0) getword(fp,b,MWS);
        }
    }
    return num/3;
}

int read_one_obj(
                 FILE *fp,
                 Polygon *poly,
                 Surface *surface)
{
    char b[MWS];
    int flag_material = 0;
    int flag_point = 0;
    int flag_index = 0;

    /* initialize surface */
    surface->diff[0] = 1.0;
    surface->diff[1] = 1.0;
    surface->diff[2] = 1.0;
    surface->spec[0] = 0.0;
    surface->spec[1] = 0.0;
    surface->spec[2] = 0.0;
    surface->ambi = 0.0;
    surface->shine = 0.2;

    if ( getword(fp,b,MWS) <= 0) return 0;

    poly->vtx_num = 0;
    poly->idx_num = 0;

    while (flag_material==0 || flag_point==0 || flag_index==0) {
        if      (strindex(b,"Material")>=0) {
            getword(fp,b,MWS);
            flag_material = 1;
        }
        else if (strindex(b,"point")>=0) {
            fprintf(stderr,"Counting... [point]\n");
            poly->vtx_num = count_point(fp, b);
            flag_point = 1;
        }
        else if (strindex(b,"coordIndex")>=0) {
            fprintf(stderr,"Counting... [coordIndex]\n");
            poly->idx_num = count_index(fp, b);
            flag_index = 1;
        }
        else if (getword(fp,b,MWS) <= 0) return 0;
    }

    flag_material = 0;
    flag_point = 0;
    flag_index = 0;

    fseek(fp, 0, SEEK_SET);
    poly->vtx = (double *)malloc(sizeof(double)*3*poly->vtx_num);
    poly->idx = (int *)malloc(sizeof(int)*3*poly->idx_num);
    while (flag_material==0 || flag_point==0 || flag_index==0) {
        if      (strindex(b,"Material")>=0) {
            fprintf(stderr,"Reading... [Material]\n");
            read_material(fp,surface,b);
            flag_material = 1;
        }
        else if (strindex(b,"point")>=0) {
            fprintf(stderr,"Reading... [point]\n");
            read_point(fp,poly,b);
            flag_point = 1;
        }
        else if (strindex(b,"coordIndex")>=0) {
            fprintf(stderr,"Reading... [coordIndex]\n");
            read_index(fp,poly,b);
            flag_index = 1;
        }
        else if (getword(fp,b,MWS) <= 0) return 0;
    }

    return 1;
}		 


int main (int argc, char *argv[])
{
    int i;
    FILE *fp;
    Polygon poly;
    Surface surface;

    fp = fopen(argv[1], "r");
    read_one_obj(fp, &poly, &surface);

    fprintf(stderr,"%d vertice are found.(poly.vtx_num)\n",poly.vtx_num);
    fprintf(stderr,"%d triangles are found.(poly.idx_num)\n",poly.idx_num);

    //i th vertex
    printf("\npoly.vtx[i*3+0,1,2]\n");
    for ( i = 0 ; i < poly.vtx_num ; i++ ) {
        fprintf(stdout,"%f %f %f # %d th vertex\n",
                poly.vtx[i*3+0], poly.vtx[i*3+1], poly.vtx[i*3+2],
                i);
    }

    //i th triangle
    printf("\npoly.idx[i*3+0,1,2]\n");
    for ( i = 0 ; i < poly.idx_num ; i++ ) {
        fprintf(stdout,"%d %d %d # %d th triangle\n",
                poly.idx[i*3+0], poly.idx[i*3+1], poly.idx[i*3+2],
                i);
    }

    /* material info */
    fprintf(stderr, "diffuseColor %f %f %f\n", surface.diff[0], surface.diff[1], surface.diff[2]);
    fprintf(stderr, "specularColor %f %f %f\n", surface.spec[0], surface.spec[1], surface.spec[2]);
    fprintf(stderr, "ambientIntensity %f\n", surface.ambi);
    fprintf(stderr, "shininess %f\n", surface.shine);

    //===================================================================

    FILE *fp_ppm;
    char *fname = argv[2];

    
    fp_ppm = fopen( fname, "w" );
    //ファイルが開けなかったとき
    if( fp_ppm == NULL ){
        printf("%sファイルが開けません.\n", fname);
        return -1;
    }
    
    //ファイルが開けたとき
    else{
        //描画領域を初期化
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                image[i][j][0] = 0.0 * MAX;
                image[i][j][1] = 0.0 * MAX;
                image[i][j][2] = 0.0 * MAX;
            }
        }

         //zバッファを初期化
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                z_buf[i][j] = DBL_MAX;
            }
        }

        //diffuse_colorの格納 
        diffuse_color[0] = surface.diff[0];
        diffuse_color[1] = surface.diff[1];
        diffuse_color[2] = surface.diff[2];

        //shininessの格納
        //！！！！！！！！！！！！！注意！！！！！！！！！！！！！！！！
        //（実験ページの追加情報を参照）
        //各ファイルのshininessの値は
        //av4 0.5
        //av5 0.5
        //iiyama1997 1.0
        //aa053 1.0
        //av007 0.34
        
        shininess = surface.shine * 128;

        //speculorColorの格納
        specular_color[0] = surface.spec[0];
        specular_color[1] = surface.spec[1];
        specular_color[2] = surface.spec[2];

        //各頂点の法線ベクトルを求める
        //三角形iの法線ベクトルを求めて配列に格納する（グローバル領域に保存）
        double poly_n[poly.idx_num * 3];


        //=======================================================================
        //三角形iは3点A、B、Cからなる
        //この3点で形成される三角形の法線ベクトルを求めてpoly_nに格納していく
        for(int i = 0; i < poly.idx_num; i++){
            //三角形iの各頂点の座標
            double A[3], B[3], C[3];
            A[0] = poly.vtx[(poly.idx[i*3+0])*3 + 0];
            A[1] = poly.vtx[(poly.idx[i*3+0])*3 + 1];
            A[2] = poly.vtx[(poly.idx[i*3+0])*3 + 2];
            
            B[0] = poly.vtx[(poly.idx[i*3+1])*3 + 0];
            B[1] = poly.vtx[(poly.idx[i*3+1])*3 + 1];
            B[2] = poly.vtx[(poly.idx[i*3+1])*3 + 2];
            
            C[0] = poly.vtx[(poly.idx[i*3+2])*3 + 0];
            C[1] = poly.vtx[(poly.idx[i*3+2])*3 + 1];
            C[2] = poly.vtx[(poly.idx[i*3+2])*3 + 2];
            
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
            
            poly_n[i*3 + 0] = n[0];
            poly_n[i*3 + 1] = n[1];
            poly_n[i*3 + 2] = n[2];
        }
        //=======================================================================


        //三角形iの法線ベクトルがpoly_nに格納された.
        //debug
        printf("\npoly_n\n");
        for(int i = 0 ; i < poly.idx_num ; i++){
            fprintf(stdout,"%f %f %f # %d th triangle\n",
                    poly_n[i*3+0], poly_n[i*3+1], poly_n[i*3+2],
                    i);
        }

        //各点の平均、正規化した法線ベクトルを求める==================================================
        //点iの法線ベクトルをもとめて専用の配列に格納する
        //頂点iの法線ベクトルは
        //(poly_ave_i[i*3+0], poly_ave_i[i*3+1], poly_ave_i[i*3+2])
        double poly_ave_i[poly.vtx_num];
        //点iが隣接する平面を探索
        for(int i = 0; i < poly.vtx_num; i++){
            double sum_vec[3] = {0.0, 0.0, 0.0};
            int count = 0;
            //三角形jの中に頂点iが含まれるかを判定
            for(int j = 0; j < poly.idx_num; j++){
                //プログラムの可読性を保つためバラして書く
                if(poly.idx[j*3+0] == i||
                   poly.idx[j*3+1] == i||
                   poly.idx[j*3+2] == i){
                    sum_vec[0] =  sum_vec[0] + poly_n[j*3+0];
                    sum_vec[1] =  sum_vec[1] + poly_n[j*3+1];
                    sum_vec[2] =  sum_vec[2] + poly_n[j*3+2];
                    count++;
                }
            }
            //点iの法線ベクトルを隣接平面の法線ベクトルの平均を正規化して計算する
            double ni_vec[3];
            if(count == 0){
                printf("\n warning!! 1128\n");
                printf("\n i = %d\n" ,i);
                exit(0);
            }
            ni_vec[0] = sum_vec[0] / count;
            ni_vec[1] = sum_vec[1] / count;
            ni_vec[2] = sum_vec[2] / count;
            
            double length_ni_vec =
                sqrt(pow(ni_vec[0], 2.0)+
                     pow(ni_vec[1], 2.0)+
                     pow(ni_vec[2], 2.0));
            if(length_ni_vec == 0){
                printf("\n warning!! 1129\n");
                exit(0);
            }
            ni_vec[0] = ni_vec[0] / length_ni_vec;
            ni_vec[1] = ni_vec[1] / length_ni_vec;
            ni_vec[2] = ni_vec[2] / length_ni_vec;

            //頂点iの法線ベクトルを格納
            poly_ave_i[i*3+0] = ni_vec[0];
            poly_ave_i[i*3+1] = ni_vec[1];
            poly_ave_i[i*3+2] = ni_vec[2];

            //debug
            double length_ply_ave =
                sqrt(pow(poly_ave_i[i*3+0], 2.0)+
                     pow(poly_ave_i[i*3+1], 2.0)+
                     pow(poly_ave_i[i*3+2], 2.0));
            if(length_ply_ave == 0){
                printf("\n warning!! 1151 \n");
                exit(0);
            }
        }
        //======================================================================================


        //点iの法線ベクトルがpoly_ave_iに格納された.
        //debug
        printf("\npoly_ave_i\n");
        for(int i = 0 ; i < poly.idx_num ; i++){
            fprintf(stdout,"%f %f %f # %d th vertex\n",
                    poly_ave_i[i*3+0], poly_ave_i[i*3+1], poly_ave_i[i*3+2],
                    i);
        }



        
        //各点の輝度値を決定する===================================================================
        //点iの輝度値を専用の配列に格納
        double rgb_i[poly.vtx_num*3];
        //点iの輝度値は
        //(rgb_i[i*3+0], rgb_i[i*3+0], rgb_i[i*3+0],)
        
        //点iのiベクトルは平行光源を使うと全ての点において
        //同じになるので予め用意する==============
        double i_vec[3];
        i_vec[0] = light_dir[0];
        i_vec[1] = light_dir[1];
        i_vec[2] = light_dir[2];
        double length_i =
            sqrt(pow(i_vec[0], 2.0) + pow(i_vec[1], 2.0) + pow(i_vec[2], 2.0));
        if(length_i == 0){
                printf("\n warning! 11403\n");
                exit(0);
            }
        i_vec[0] = (i_vec[0] / length_i);
        i_vec[1] = (i_vec[1] / length_i);
        i_vec[2] = (i_vec[2] / length_i);
        //====================================
        
        for(int i = 0; i < poly.vtx_num; i++){
            //eベクトル=========================
            double e[3];
            e[0] = -1 * poly.vtx[i*3+0];
            e[1] = -1 * poly.vtx[i*3+1];
            e[2] = -1 * poly.vtx[i*3+2];
            double length_e =
                sqrt(pow(e[0], 2.0) + pow(e[1], 2.0) + pow(e[2], 2.0));
            if(length_e == 0){
                printf("\n warning! 11400\n");
                exit(0);
            }
            e[0] = (e[0] / length_e);
            e[1] = (e[1] / length_e);
            e[2] = (e[2] / length_e);
            //=================================

            //sベクトル=========================
            double s[3];
            s[0] = e[0] - i_vec[0];
            s[1] = e[1] - i_vec[1];
            s[2] = e[2] - i_vec[2];
            double length_s =
                sqrt(pow(s[0], 2.0) + pow(s[1], 2.0) + pow(s[2], 2.0));
            if(length_s == 0){
                printf("\n warning! 11401\n");
                exit(0);
            }
            s[0] = (s[0] / length_s);
            s[1] = (s[1] / length_s);
            s[2] = (s[2] / length_s);
            //=================================


            //iベクトルとnベクトルの内積を計算
            double ip =
                (poly_ave_i[i*3+0] * i_vec[0]) +
                (poly_ave_i[i*3+1] * i_vec[1]) +
                (poly_ave_i[i*3+2] * i_vec[2]);
            
            if(0 <= ip){
                ip = 0;
            }

            //内積sn
            double sn
                = ((s[0] * poly_ave_i[i*3+0]) +
                   (s[1] * poly_ave_i[i*3+1]) +
                   (s[2] * poly_ave_i[i*3+2]));
            if(sn <= 0){
                sn = 0;
            }

            //頂点iの輝度値を計算
            rgb_i[i*3+0] =
                //拡散反射
                (-1 * ip * diffuse_color[0] * light_rgb[0] * MAX)
                //鏡面反射
                + (pow(sn, shininess) * specular_color[0] * light_rgb[0] * MAX)
                //環境反射
                + surface.ambi * ENV_LIGHT * MAX
                ;
            
            rgb_i[i*3+1] =
                //拡散反射
                (-1 * ip * diffuse_color[1] * light_rgb[1] * MAX)
                //鏡面反射
                + (pow(sn, shininess) * specular_color[1] * light_rgb[1] * MAX)
                //環境反射
                + surface.ambi * ENV_LIGHT * MAX
                ;
            
            rgb_i[i*3+2] =
                //拡散反射
                (-1 * ip * diffuse_color[2] * light_rgb[2] * MAX)
                //鏡面反射
                + (pow(sn, shininess) * specular_color[2] * light_rgb[2] * MAX)
                //環境反射
                + surface.ambi * ENV_LIGHT * MAX
                ;


            //debug
            printf("\nrgb_i=(%f\t%f\t%f), i=%d\n", rgb_i[i*3+0], rgb_i[i*3+1], rgb_i[i*3+2], i);
        }
        //======================================================================================
        

        
                                
        //シェーディング
        //ポリゴンiをシェーディング================================================
        
        for(int i = 0; i < poly.idx_num; i++){
            //三角形の各点の透視投影処理===========================================
            for(int j = 0; j < 3; j++){ 
                double xp = poly.vtx[(poly.idx[i*3+j])*3 + 0];
                double yp = poly.vtx[(poly.idx[i*3+j])*3 + 1];
                double zp = poly.vtx[(poly.idx[i*3+j])*3 + 2];
                double zi = FOCUS;

                //debug
                //printf("\nxp = %f\typ = %f\tzp = %f\n", xp, yp, zp);

                //debug 
                if(zp == 0){
                    printf("\n(%f\t%f\t%f) i=%d, j=%d\n", xp, yp, zp, i, j);
                    perror("\nエラー0934\n");
                    exit(0);
                    //break;
                }
                
                double xp2 = xp * (zi / zp);
                double yp2 = yp * (zi / zp);
                double zp2 = zi;
                
                //座標軸を平行移動
                projected_ver_buf[j][0] = (MAX / 2) + xp2;
                projected_ver_buf[j][1] = (MAX / 2) + yp2;
            }
 
            
            double a[2], b[2], c[2];
            a[0] = projected_ver_buf[0][0];
            a[1] = projected_ver_buf[0][1];
            b[0] = projected_ver_buf[1][0];
            b[1] = projected_ver_buf[1][1];
            c[0] = projected_ver_buf[2][0];
            c[1] = projected_ver_buf[2][1];
            //===================================================================

            //点a、b、cがそれぞれ何番目の頂点かを参照

            int index_a = poly.idx[i*3+0];
            int index_b = poly.idx[i*3+1];
            int index_c = poly.idx[i*3+2];

            //点iの輝度値を参照する
            double rgb_a[3], rgb_b[3], rgb_c[3];
            rgb_a[0] = rgb_i[index_a*3+0];
            rgb_a[1] = rgb_i[index_a*3+1];
            rgb_a[2] = rgb_i[index_a*3+2];
            
            rgb_b[0] = rgb_i[index_b*3+0];
            rgb_b[1] = rgb_i[index_b*3+1];
            rgb_b[2] = rgb_i[index_b*3+2];
            
            rgb_c[0] = rgb_i[index_c*3+0];
            rgb_c[1] = rgb_i[index_c*3+1];
            rgb_c[2] = rgb_i[index_c*3+2];
            
            //関数shadingの中では3点の空間内での座標も必要
            double A[3], B[3], C[3];
            A[0] = poly.vtx[index_a*3 + 0];
            A[1] = poly.vtx[index_a*3 + 1];
            A[2] = poly.vtx[index_a*3 + 2];
            
            B[0] = poly.vtx[index_b*3 + 0];
            B[1] = poly.vtx[index_b*3 + 1];
            B[2] = poly.vtx[index_b*3 + 2];
            
            C[0] = poly.vtx[index_c*3 + 0];
            C[1] = poly.vtx[index_c*3 + 1];
            C[2] = poly.vtx[index_c*3 + 2];
            //三角形iのシェーディングを行う
            
            //三角形iの（本来の）法線ベクトルは
            //(poly_n[i*3+0], poly_n[i*3+1], poly_n[i*3+2])
            double poly_i_n_vec[3]
                = {poly_n[i*3+0], poly_n[i*3+1], poly_n[i*3+2]};

            shading(a, b, c, rgb_a, rgb_b, rgb_c, A, B, C, poly_i_n_vec);
        }


        
     
        //ヘッダー出力
        fputs(MAGICNUM, fp_ppm);
        fputs("\n", fp_ppm);
        fputs(WIDTH_STRING, fp_ppm);
        fputs(" ", fp_ppm);
        fputs(HEIGHT_STRING, fp_ppm);
        fputs("\n", fp_ppm);
        fputs(MAX_STRING, fp_ppm);
        fputs("\n" ,fp_ppm);

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
                fputs(str, fp_ppm); 
            }
        }
    }
    fclose(fp_ppm);
    fclose(fp);
    
    printf("\nppmファイル %s の作成が完了しました.\n", fname );
    return 1;
}

