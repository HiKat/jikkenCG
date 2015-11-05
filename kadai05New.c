#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "vrml.h"
#include <string.h>
#include <math.h>
#include <float.h>

//=====================================================================
//必要なデータ
#define FILENAME "image2.ppm"
#define MAGICNUM "P3"
#define WIDTH 256
#define WIDTH_STRING "256"
#define HEIGHT 256
#define HEIGHT_STRING "256"
#define MAX 255
#define MAX_STRING "255"
#define FOCUS 256.0
#define Z_BUF_MAX
#define MAP_FILENAME "./sample/spheremap1 copy.ppm"

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
//環境マップを格納するリストの構造体の定義
struct list{
    int num;
    int index;
    struct list *next;
}list;
typedef struct list LIST;

//環境マッピング用の画像の縦横幅、上限値
int ppm_width, ppm_height, ppm_max;


/* ppm取り込み用関数 */============================================
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

void free_list(LIST *p)
{
	LIST *p2;

	while (p != NULL) { 
		p2 = p->next;
		free(p);
		p = p2;
	}
}
//==============================================================



/* シェーディングに関する関数 */=================================================
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


void shading(double *a, double *b, double *c, double *n, double *A, int input_ppm[ppm_height][ppm_width][3]){
    //3点が1直線上に並んでいるときはシェーディングができない
    if(lineOrNot(a, b, c) == 1){
        //塗りつぶす点が無いので何もしない.
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
            //2パターンの三角形を特定
            if(p[1] == r[1]){
                //debug
                //printf("\np[1] == r[1]\n");
                //x座標が p <= r となるように調整
                if(r[0] <  p[0]){
                    double temp[2];
                    memcpy(temp, r, sizeof(double) * 2);
                    memcpy(r, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);
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


                            //鏡面反射を適用==========================================================================
                            //======================================================================================
                            //描画する点の投影前の空間内の座標.
                            double p_or[3];
                               
                            p_or[0] =
                                (n[0]*(j-(WIDTH/2)))
                                *
                                ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                /
                                ((n[0]*(j-(WIDTH/2))) + (n[1]*(i-(HEIGHT/2))) + n[2]*FOCUS);
                               
                            p_or[1] =
                                (n[1]*(i-(HEIGHT/2)))
                                *
                                ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                /
                                ((n[0]*(j-(WIDTH/2))) + (n[1]*(i-(HEIGHT/2))) + n[2]*FOCUS);
                               
                            p_or[2] =
                                FOCUS
                                *
                                ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                /
                                ((n[0]*(j-(WIDTH/2))) + (n[1]*(i-(HEIGHT/2))) + n[2]*FOCUS);


                            //反射ベクトルの計算=======================================
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
                            
                            //法線ベクトルnと光源方向ベクトルの内積
                            double ip =
                                (n[0] * light_dir_vec[0]) +
                                (n[1] * light_dir_vec[1]) +
                                (n[2] * light_dir_vec[2]);
                            
                            //uは単位視線方向ベクトル
                            //視点方向は原点に固定
                            double u[3];
                            u[0] = p_or[0];
                            u[1] = p_or[1];
                            u[2] = p_or[2];
                            double length_u =
                                sqrt(pow(u[0], 2.0) +
                                     pow(u[1], 2.0) +
                                     pow(u[2], 2.0));
                            
                            /* u[0] = (u[0] / length_u); */
                            /* u[1] = (u[1] / length_u); */
                            /* u[2] = (u[2] / length_u); */

                            /* u[0] = 0; */
                            /* u[1] = 0; */
                            /* u[2] = -1; */

                            /* double n_[3]; */
                            /* double length_fu = sqrt(pow(f[0]+u[0], 2.0)+pow(f[1]+u[1], 2.0)+pow(f[2]+u[2], 2.0)); */
                            /* n_[0] = (f[0]+u[0])/length_fu; */
                            /* n_[1] = (f[1]+u[1])/length_fu; */
                            /* n_[2] = (f[2]+u[2])/length_fu; */

                            //法線ベクトルと視線ベクトルの内積
                            double nu = (n[0]*u[0])+(n[1]*u[1])+(n[2]*u[2]);

                            double f[3];
                            f[0] = u[0] - 2*n[0]*nu;
                            f[1] = u[1] - 2*n[1]*nu;
                            f[2] = u[2] - 2*n[2]*nu;
                            double length_f =
                                sqrt(pow(f[0], 2.0) +
                                     pow(f[1], 2.0) +
                                     pow(f[2], 2.0));
                            
                            /* f[0] = (f[0] / length_f); */
                            /* f[1] = (f[1] / length_f); */
                            /* f[2] = (f[2] / length_f); */

                            //========================================================

                            //Sphere mapとの対応=======================================
                            double m = 2*sqrt(pow(f[0], 2.0)+
                                              pow(f[1], 2.0)+
                                              pow((f[2] + 1), 2.0));
                            
                            int s_x = (int)round((0.5 + (f[0]/m)) * ppm_width);
                            int t_y = (int)round((0.5 - (f[1]/m)) * ppm_height);
                            printf("s = %d t = %d\n", s_x, t_y);

                               
                            //環境マップから対応する画素値を取り出す
                            int env_r, env_g, env_b;

                            int debug = 0;
                            //環境マップ外なら描写しない
                            if(s_x < 0 || t_y < 0 ||
                               s_x > (ppm_width-1) || t_y > (ppm_height-1)){
                                env_r = 0;
                                env_g = 0;
                                env_b = 0;
                                
                                //debug
                                debug = 1;
                            }
                            else{
                                env_r = input_ppm[t_y][s_x][0];
                                env_g = input_ppm[t_y][s_x][1];
                                env_b = input_ppm[t_y][s_x][2];
                                
                                /* env_r = 128; */
                                /* env_g = 128; */
                                /* env_b = 128; */
                            }
                               
                            //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                            if(z_buf[i][j] < p_or[2]){}
                               
                            else{
                                /* image[i][j][0] = -1 * ip * env_r * light_rgb[0]; */
                                
                                /* image[i][j][1] = -1 * ip * env_g * light_rgb[1]; */
                                
                                /* image[i][j][2] = -1 * ip * env_b * light_rgb[2]; */
                                
                                image[i][j][0] =  env_r;
                                
                                image[i][j][1] =  env_g;
                                
                                image[i][j][2] =  env_b;


                            //zバッファの更新
                            z_buf[i][j] = p_or[2];
                            //debug
                            //debug
                            if(debug == 1){
                                printf("at 2100");
                                printf("\n s_x = %d, t_y = %d\n", s_x, t_y);
                                printf("%f %f\n",((0.5 + (f[1]/2)) * ppm_width), ((0.5 - (f[0]/2)) * ppm_height));

                                //exit(0);
                            }
                        }
                    }
                }
                //はみ出ている場合は描画しない
                else{}
            }
                
        }
            
        if(p[1] == q[1]){
            if(q[0] <  p[0]){
                double temp[2];
                memcpy(temp, q, sizeof(double) * 2);
                memcpy(q, p, sizeof(double) * 2);
                memcpy(p, temp, sizeof(double) * 2);
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
                r[1] <= i && i <= p[1];
                i++){

                //撮像部分からはみ出ていないかのチェック
                if( 0 <= i &&
                    i <= (HEIGHT - 1)){
                    double x1 = func1(p, r, i);
                    double x2 = func1(q, r, i);
                        
                    int j;
                    j = ceil(x1);
                        
                    //debug
                    //printf("\nj = %d\nx1 = %f\nx2 = %f\ni = %d\n",j ,x1, x2, i);
                        
                    for(j;
                        x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                        j++){

                        //鏡面反射を適用==========================================================================
                        //======================================================================================
                        //描画する点の投影前の空間内の座標.
                        double p_or[3];
                            
                        p_or[0] =
                            (n[0]*(j-(MAX/2)))
                            *
                            ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                            /
                            ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                            
                        p_or[1] =
                            (n[1]*(i-(MAX/2)))
                            *
                            ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                            /
                            ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                            
                        p_or[2] =
                            FOCUS
                            *
                            ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                            /
                            ((n[0]*(j-(MAX/2))) + (n[1]*(i-(MAX/2))) + n[2]*FOCUS);
                        //反射ベクトルの計算=======================================
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
                            
                        //法線ベクトルnと光源方向ベクトルの内積
                        double ip =
                            (n[0] * light_dir_vec[0]) +
                            (n[1] * light_dir_vec[1]) +
                            (n[2] * light_dir_vec[2]);
                            
                        //uは単位視線方向ベクトル
                        //視点方向は原点に固定
                        double u[3];
                        u[0] = p_or[0];
                        u[1] = p_or[1];
                        u[2] = p_or[2];
                        double length_u =
                            sqrt(pow(u[0], 2.0) +
                                 pow(u[1], 2.0) +
                                 pow(u[2], 2.0));
                        
                        /* u[0] = (u[0] / length_u); */
                        /* u[1] = (u[1] / length_u); */
                        /* u[2] = (u[2] / length_u); */


                        //法線ベクトルと視線ベクトルの内積
                        double nu = (n[0]*u[0])+(n[1]*u[1])+(n[2]*u[2]);

                        double f[3];
                        f[0] = u[0] - 2*n[0]*nu;
                        f[1] = u[1] - 2*n[1]*nu;
                        f[2] = u[2] - 2*n[2]*nu;
                        double length_f =
                            sqrt(pow(f[0], 2.0) +
                                 pow(f[1], 2.0) +
                                 pow(f[2], 2.0));
                        
                        /* f[0] = (f[0] / length_f); */
                        /* f[1] = (f[1] / length_f); */
                        /* f[2] = (f[2] / length_f); */
                        
                        //Sphere mapとの対応=======================================
                        double m = 2*sqrt(pow(f[0], 2.0)+
                                          pow(f[1], 2.0)+
                                          pow((f[2] + 1), 2.0));
                           

                        int s_x = (int)round((0.5 + (f[0]/m)) * ppm_width);
                        int t_y = (int)round((0.5 - (f[1]/m)) * ppm_height);

                               
                        //環境マップから対応する画素値を取り出す
                        int env_r, env_g, env_b;
                        //環境マップ外なら描写しない
                        if(s_x < 0 || t_y < 0 ||
                           s_x > (ppm_width-1) || t_y > (ppm_height-1)){
                            env_r = 0;
                            env_g = 0;
                            env_b = 0;
                        }
                        else{
                            env_r = input_ppm[t_y][s_x][0];
                            env_g = input_ppm[t_y][s_x][1];
                            env_b = input_ppm[t_y][s_x][2];

                            /* env_r = 128; */
                            /* env_g = 128; */
                            /* env_b = 128; */
                        }
                               
                               

                            
                        //======================================================================================
                        //======================================================================================

                            
                        //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                        if(z_buf[i][j] < p_or[2]){}
                        
                        else{
                            /* image[i][j][0] = -1 * ip * env_r * light_rgb[0]; */
                                
                            /* image[i][j][1] = -1 * ip * env_g * light_rgb[1]; */
                                
                            /* image[i][j][2] = -1 * ip * env_b * light_rgb[2]; */

                            image[i][j][0] =  env_r;
                                
                            image[i][j][1] =  env_g;
                                
                            image[i][j][2] =  env_b;

                              

                        //zバッファの更新
                        z_buf[i][j] = p_or[2];
                        //debug
                        if(env_r == 0 && env_g == 0 && env_b == 0){
                            //printf("at 2102\n");
                            //printf("\n s_x = %d, t_y = %d\n", s_x, t_y);
                        }
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
     //p2のほうがpのx座標より大きくなるようにする
     if(p2[0] < p[0]){
         double temp[2];
         memcpy(temp, p2, sizeof(double) * 2);
         memcpy(p2, p, sizeof(double) * 2);
         memcpy(p, temp, sizeof(double) * 2);
     }
     shading(p, p2, q, n, A, input_ppm);
     shading(p, p2, r, n, A, input_ppm);
 }
}
}


