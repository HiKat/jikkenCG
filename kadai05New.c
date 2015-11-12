#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>


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
#define ENV_LIGHT 1.0
#define MAP_FILENAME "./sample/spheremap1.ppm"

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
//カメラ位置
double camera_xyz[3];
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


//引数は3点の座標とRGBと3点の空間内の座標、3点で形成される空間内の平面の法線ベクトルとする
void shading(double *a, double *b, double *c,
             double *n_a, double *n_b, double *n_c, 
             double *A, double *B, double *C, double *poly_i_n_vec,
             int input_ppm[ppm_height][ppm_width][3]){
    //3点が1直線上に並んでいるときはシェーディングができない
    if(lineOrNot(a, b, c) == 1){
        //塗りつぶす点が無いので何もしない.       
    }
    else{
        //y座標の値が真ん中点をp、その他の点をq、rとする
        //y座標の大きさはr <= p <= qの順
        double p[2], q[2], r[2];
        //法線ベクトルも名前を変更する
        double n_p[3], n_q[3], n_r[3];
        //空間内での元の座標についても名前を変更する
        double P[3], Q[3], R[3];

        if(b[1] <= a[1] && a[1] <= c[1]){
            memcpy(p, a, sizeof(double) * 2);
            memcpy(q, c, sizeof(double) * 2);
            memcpy(r, b, sizeof(double) * 2);
            
            memcpy(n_p, n_a, sizeof(double) * 3);
            memcpy(n_q, n_c, sizeof(double) * 3);
            memcpy(n_r, n_b, sizeof(double) * 3);

            memcpy(P, A, sizeof(double) * 3);
            memcpy(Q, C, sizeof(double) * 3);
            memcpy(R, B, sizeof(double) * 3);
        }
        else{
            if(c[1] <= a[1] && a[1] <= b[1]){
                memcpy(p, a, sizeof(double) * 2);
                memcpy(q, b, sizeof(double) * 2);
                memcpy(r, c, sizeof(double) * 2);
                
                memcpy(n_p, n_a, sizeof(double) * 3);
                memcpy(n_q, n_b, sizeof(double) * 3);
                memcpy(n_r, n_c, sizeof(double) * 3);

                memcpy(P, A, sizeof(double) * 3);
                memcpy(Q, B, sizeof(double) * 3);
                memcpy(R, C, sizeof(double) * 3);

            }
            else{
                if(a[1] <= b[1] && b[1] <= c[1]){
                    memcpy(p, b, sizeof(double) * 2);
                    memcpy(q, c, sizeof(double) * 2);
                    memcpy(r, a, sizeof(double) * 2);
                    
                    memcpy(n_p, n_b, sizeof(double) * 3);
                    memcpy(n_q, n_c, sizeof(double) * 3);
                    memcpy(n_r, n_a, sizeof(double) * 3);

                    memcpy(P, B, sizeof(double) * 3);
                    memcpy(Q, C, sizeof(double) * 3);
                    memcpy(R, A, sizeof(double) * 3);
                }
                else{
                    if(c[1] <= b[1] && b[1] <= a[1]){
                        memcpy(p, b, sizeof(double) * 2);
                        memcpy(q, a, sizeof(double) * 2);
                        memcpy(r, c, sizeof(double) * 2);
                        
                        memcpy(n_p, n_b, sizeof(double) * 3);
                        memcpy(n_q, n_a, sizeof(double) * 3);
                        memcpy(n_r, n_c, sizeof(double) * 3);

                        memcpy(P, B, sizeof(double) * 3);
                        memcpy(Q, A, sizeof(double) * 3);
                        memcpy(R, C, sizeof(double) * 3);
                    }
                    else{
                        if(b[1] <= c[1] && c[1] <= a[1]){
                            memcpy(p, c, sizeof(double) * 2);
                            memcpy(q, a, sizeof(double) * 2);
                            memcpy(r, b, sizeof(double) * 2);

                            memcpy(n_p, n_c, sizeof(double) * 3);
                            memcpy(n_q, n_a, sizeof(double) * 3);
                            memcpy(n_r, n_b, sizeof(double) * 3);

                            memcpy(P, C, sizeof(double) * 3);
                            memcpy(Q, A, sizeof(double) * 3);
                            memcpy(R, B, sizeof(double) * 3);
                        }
                        else{
                            if(a[1] <= c[1] && c[1] <= b[1]){
                                memcpy(p, c, sizeof(double) * 2);
                                memcpy(q, b, sizeof(double) * 2);
                                memcpy(r, a, sizeof(double) * 2);

                                memcpy(n_p, n_c, sizeof(double) * 3);
                                memcpy(n_q, n_b, sizeof(double) * 3);
                                memcpy(n_r, n_a, sizeof(double) * 3);

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
                            
            //2パターンの三角形を特定
            //Type 1
            if(p[1] == r[1]){
                //debug
                //printf("\np[1] == r[1]\n");
                //x座標が p <= r となるように調整
                if(r[0] <  p[0]){
                    double temp[2];
                    double temp_n[3];
                    memcpy(temp, r, sizeof(double) * 2);
                    memcpy(r, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);

                    memcpy(temp_n, n_r, sizeof(double) * 3);
                    memcpy(n_r, n_p, sizeof(double) * 3);
                    memcpy(n_p, temp_n, sizeof(double) * 3);
                    
                    //debug
                    /* printf("\n交換後の3点の座標は\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n" */
                    /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
                    
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


                /* 点(j, i)のシェーディング========================================================================== */
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


                               /* 点(j, i)の空間座標を求める. */
                               double p_or[3];
                               double k =
                                   ((poly_i_n_vec[0] * (A[0] - camera_xyz[0])) +
                                    (poly_i_n_vec[1] * (A[1] - camera_xyz[1])) +
                                    (poly_i_n_vec[2] * (A[2] - camera_xyz[2])))
                                   /
                                   ((poly_i_n_vec[0] * ((j-(WIDTH/2)) - camera_xyz[0])) +
                                    (poly_i_n_vec[1] * ((i-(HEIGHT/2)) - camera_xyz[1])) +
                                    (poly_i_n_vec[2] * (FOCUS - camera_xyz[2])));
                               
                               p_or[0] = k * ((j-(WIDTH/2)) - camera_xyz[0]) + camera_xyz[0];
                               p_or[1] = k * ((i-(HEIGHT/2)) - camera_xyz[1]) + camera_xyz[1];             
                               p_or[2] = k * (FOCUS - camera_xyz[2]) + camera_xyz[2];


                               /* 点(J, i)の法線ベクトルを線形補間によって求める */
                               double n_ji[3];
                               n_ji[0] =
                                       (
                                        ((x2-j) / (x2-x1))
                                        *
                                        ((n_p[0]*(q[1]-i) + n_q[0]*(i-p[1])) / (q[1]-p[1]))
                                        )
                                       +
                                       (
                                        ((j-x1) / (x2-x1))
                                        *
                                        ((n_r[0]*(q[1]-i) + n_q[0]*(i-r[1])) / (q[1]-r[1]))
                                        );
                                       
                               n_ji[1] =
                                        (
                                        ((x2-j) / (x2-x1))
                                        *
                                        ((n_p[1]*(q[1]-i) + n_q[1]*(i-p[1])) / (q[1]-p[1]))
                                        )
                                       +
                                       (
                                        ((j-x1) / (x2-x1))
                                        *
                                        ((n_r[1]*(q[1]-i) + n_q[1]*(i-r[1])) / (q[1]-r[1]))
                                        );
                                   
                               n_ji[2] =
                                       (
                                        ((x2-j) / (x2-x1))
                                        *
                                        ((n_p[2]*(q[1]-i) + n_q[2]*(i-p[1])) / (q[1]-p[1]))
                                        )
                                       +
                                       (
                                        ((j-x1) / (x2-x1))
                                        *
                                        ((n_r[2]*(q[1]-i) + n_q[2]*(i-r[1])) / (q[1]-r[1]))
                                        );
                               double length_n_ji =
                                   sqrt(pow(n_ji[0], 2.0) +
                                        pow(n_ji[1], 2.0) +
                                        pow(n_ji[2], 2.0));
                               n_ji[0] = (n_ji[0] / length_n_ji);
                               n_ji[1] = (n_ji[1] / length_n_ji);
                               n_ji[2] = (n_ji[2] / length_n_ji);

                               
                               /* 視線方向ベクトルを求める */
                               double u[3];
                               u[0] = p_or[0] - camera_xyz[0];
                               u[1] = p_or[1] - camera_xyz[1];
                               u[2] = p_or[2] - camera_xyz[2];
                               double length_u =
                                   sqrt(pow(u[0], 2.0) +
                                        pow(u[1], 2.0) +
                                        pow(u[2], 2.0));                             
                               u[0] = (u[0] / length_u);
                               u[1] = (u[1] / length_u);
                               u[2] = (u[2] / length_u);

                               /* 反射ベクトルを求める */
                               double nu = (n_ji[0]*u[0])+(n_ji[1]*u[1])+(n_ji[2]*u[2]);
                               double f[3];
                               f[0] = u[0] - 2*n_ji[0]*nu;
                               f[1] = u[1] - 2*n_ji[1]*nu;
                               f[2] = u[2] - 2*n_ji[2]*nu;
                               double length_f =
                                   sqrt(pow(f[0], 2.0) +
                                        pow(f[1], 2.0) +
                                        pow(f[2], 2.0));                         
                               f[0] = (f[0] / length_f);
                               f[1] = (f[1] / length_f);
                               f[2] = (f[2] / length_f);

                               double m = 2*sqrt(pow(f[0], 2.0)+
                                                 pow(f[1], 2.0)+
                                                 pow(f[2], 2.0));
                                                             
                               int s_x = (int)round((0.5 + (f[0]/m)) * ppm_width);
                               int t_y = (int)round((0.5 - (f[1]/m)) * ppm_height);
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
                               }
                               
                               
                               //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                               if(z_buf[i][j] < p_or[2]){}
                               //小さいとき
                               else{
                                   image[i][j][0] =  env_r;
                                   
                                   image[i][j][1] =  env_g;
                                   
                                   image[i][j][2] =  env_b;
                                   
                                   //zバッファの更新
                                   z_buf[i][j] = p_or[2];
                               }
                           }
                    }
                    //はみ出ている場合は描画しない
                    else{}
                }
                /* 点(j, i)のシェーディングここまで========================================================================== */
                
            }
            
            if(p[1] == q[1]){
                //debug
                //printf("\np[1] == q[1]\n");
                //debug
                /* printf("\n三角形\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n\n" */
                /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
                //x座標が p < q となるように調整
                if(q[0] <  p[0]){
                    double temp[2];
                    double temp_n[3];                   
                    memcpy(temp, q, sizeof(double) * 2);
                    memcpy(q, p, sizeof(double) * 2);
                    memcpy(p, temp, sizeof(double) * 2);

                    memcpy(temp_n, n_q, sizeof(double) * 3);
                    memcpy(n_q, n_p, sizeof(double) * 3);
                    memcpy(n_p, temp_n, sizeof(double) * 3);
                    
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


                /* 点(j, i)のシェーディング============================================== */
                int i;
                i = ceil(r[1]);
                //debug
                /* printf("\ni = %d\n", i); */
                /* printf("\nr[1] = %f\n", r[1]); */
                /* printf("\np[1] = %f\n", p[1]); */
                //debug
                /* printf("\n三角形\npの座標(%f, %f)\nqの座標(%f, %f)\nrの座標(%f, %f)\n\n" */
                /*        ,p[0], p[1], q[0], q[1], r[0], r[1]); */
            
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

                              /* 点(j, i)の空間座標を求める. */
                               double p_or[3];                           
                               double k =
                                   ((poly_i_n_vec[0] * (A[0] - camera_xyz[0])) +
                                    (poly_i_n_vec[1] * (A[1] - camera_xyz[1])) +
                                    (poly_i_n_vec[2] * (A[2] - camera_xyz[2])))
                                   /
                                   ((poly_i_n_vec[0] * ((j-(WIDTH/2)) - camera_xyz[0])) +
                                    (poly_i_n_vec[1] * ((i-(HEIGHT/2)) - camera_xyz[1])) +
                                    (poly_i_n_vec[2] * (FOCUS - camera_xyz[2])));
                               
                               p_or[0] = k * ((j-(WIDTH/2)) - camera_xyz[0]) + camera_xyz[0];
                               p_or[1] = k * ((i-(HEIGHT/2)) - camera_xyz[1]) + camera_xyz[1];             
                               p_or[2] = k * (FOCUS - camera_xyz[2]) + camera_xyz[2];
                               
                               /* 点(J, i)の法線ベクトルを線形補間によって求める */
                               double n_ji[3];
                               n_ji[0] =
                                   (
                                    ((x2-j) / (x2-x1))
                                    *
                                    ((n_p[0]*(i-r[1]) + n_r[0]*(p[1]-i)) / (p[1]-r[1]))
                                    )
                                   +
                                   (
                                    ((j-x1) / (x2-x1))
                                    *
                                    ((n_r[0]*(q[1]-i) + n_q[0]*(i-r[1])) / (q[1]-r[1]))
                                    );
                                
                               n_ji[1] =
                                   (
                                    ((x2-j) / (x2-x1))
                                    *
                                    ((n_p[1]*(i-r[1]) + n_r[1]*(p[1]-i)) / (p[1]-r[1]))
                                    )
                                   +
                                   (
                                    ((j-x1) / (x2-x1))
                                    *
                                    ((n_r[1]*(q[1]-i) + n_q[1]*(i-r[1])) / (q[1]-r[1]))
                                    );
                                
                               n_ji[2] =
                                   (
                                    ((x2-j) / (x2-x1))
                                    *
                                    ((n_p[2]*(i-r[1]) + n_r[2]*(p[1]-i)) / (p[1]-r[1]))
                                    )
                                   +
                                   (
                                    ((j-x1) / (x2-x1))
                                    *
                                    ((n_r[2]*(q[1]-i) + n_q[2]*(i-r[1])) / (q[1]-r[1]))
                                    );
                               double length_n_ji =
                                   sqrt(pow(n_ji[0], 2.0) +
                                        pow(n_ji[1], 2.0) +
                                        pow(n_ji[2], 2.0));
                               n_ji[0] = (n_ji[0] / length_n_ji);
                               n_ji[1] = (n_ji[1] / length_n_ji);
                               n_ji[2] = (n_ji[2] / length_n_ji);

                                   
                               /* 視線方向ベクトルを求める */
                               double u[3];
                               u[0] = p_or[0] - camera_xyz[0];
                               u[1] = p_or[1] - camera_xyz[1];
                               u[2] = p_or[2] - camera_xyz[2];
                               double length_u =
                                   sqrt(pow(u[0], 2.0) +
                                        pow(u[1], 2.0) +
                                        pow(u[2], 2.0));                             
                               u[0] = (u[0] / length_u);
                               u[1] = (u[1] / length_u);
                               u[2] = (u[2] / length_u);
                               
                               /* 反射ベクトルを求める */
                               double nu =
                                   (n_ji[0]*u[0])+(n_ji[1]*u[1])+(n_ji[2]*u[2]);
                               double f[3];
                               f[0] = u[0] - 2*n_ji[0]*nu;
                               f[1] = u[1] - 2*n_ji[1]*nu;
                               f[2] = u[2] - 2*n_ji[2]*nu;
                               double length_f =
                                   sqrt(pow(f[0], 2.0) +
                                        pow(f[1], 2.0) +
                                        pow(f[2], 2.0));                         
                               f[0] = (f[0] / length_f);
                               f[1] = (f[1] / length_f);
                               f[2] = (f[2] / length_f);

                               double m = 2*sqrt(pow(f[0], 2.0)+
                                                 pow(f[1], 2.0)+
                                                 pow(f[2], 2.0));
                               
                               
                               int s_x = (int)round((0.5 + (f[0]/m)) * ppm_width);
                               int t_y = (int)round((0.5 - (f[1]/m)) * ppm_height);
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
                               }
                               


                               
                            //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                            if(z_buf[i][j] < p_or[2]){}
                        
                            else{
                                image[i][j][0] =  env_r;
                                
                                image[i][j][1] =  env_g;
                                
                                image[i][j][2] =  env_b;
                                
                                z_buf[i][j] = p_or[2];
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
            double k =
                ((poly_i_n_vec[0] * (A[0] - camera_xyz[0])) +
                 (poly_i_n_vec[1] * (A[1] - camera_xyz[1])) +
                 (poly_i_n_vec[2] * (A[2] - camera_xyz[2])))
                /
                ((poly_i_n_vec[0] * ((p2[0]-(WIDTH/2)) - camera_xyz[0])) +
                 (poly_i_n_vec[1] * ((p2[1]-(HEIGHT/2)) - camera_xyz[1])) +
                 (poly_i_n_vec[2] * (FOCUS - camera_xyz[2])));
            
            P2[0] = k * ((p2[0]-(WIDTH/2)) - camera_xyz[0]) + camera_xyz[0];
            P2[1] = k * ((p2[1]-(HEIGHT/2)) - camera_xyz[1]) + camera_xyz[1];             
            P2[2] = k * (FOCUS - camera_xyz[2]) + camera_xyz[2];

            double n_p2[3];
            for(int i = 0; i < 3; i++){
                n_p2[i]
                    =
                    n_q[i] * ((p[1]-r[1])/(q[1]-r[1]))
                    +
                    n_r[i] * ((q[1]-p[1])/(q[1]-r[1]));
            }
                
            
            
            //p2のほうがpのx座標より大きくなるようにする
            if(p2[0] < p[0]){
                double temp[2];
                double temp_n[3];
                
                memcpy(temp, p2, sizeof(double) * 2);
                memcpy(p2, p, sizeof(double) * 2);
                memcpy(p, temp, sizeof(double) * 2);

                memcpy(temp_n, n_p2, sizeof(double) * 2);
                memcpy(n_p2, n_p, sizeof(double) * 2);
                memcpy(n_p, temp_n, sizeof(double) * 2);
            }
            shading(p, p2, q, n_p, n_p2, n_q, P, P2, Q, poly_i_n_vec, input_ppm);
            shading(p, p2, r, n_p, n_p2, n_r, P, P2, R, poly_i_n_vec, input_ppm);
        }
    }
}





/* VRML 2.0 Reader 
 *
 * ver1.1 2005/10/06 Masaaki IIYAMA (bug fix)
 * ver1.0 2005/09/27 Masaaki IIYAMA
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "vrml.h"


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


int main (int argc, char *argv[]){
    /* VRML読み込み ==========================================================*/
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

    /* VRML読み込みここまで =======================================================*/
    
    
    /* 環境マップppmファイルの読み込み================================= */
    char *map_fname = MAP_FILENAME;
    FILE *ip;
    ip = fopen(map_fname,"r");
    if(ip == NULL){ 
        fprintf(stderr, "%sを正常に開くことが出来ませんでした.\n" ,MAP_FILENAME);
        exit(1);/*異常終了*/
    }
    
    printf("loading %s...\n",MAP_FILENAME);

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
    ppm_width = atoi(strtok(buf, " "));
    printf("width is %d.\n", ppm_width);
    ppm_height = atoi(strtok(NULL, "\n"));
    printf("height is %d.\n", ppm_height);
    //===========================================
        
    //上限値を取得=================================
    fgets(buf,70,ip);
    ppm_max = atoi(strtok(buf, "\n"));
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

    while (1){
        num = fgetc(ip);
        char reset[] = "";
        //空白判定
        //空白のとき
        if(isspace(num) != 0 || num == EOF){
            //直前に読み込んだ文字が空白のとき
            if(flag == 1){
                /* ループから抜ける */
                if(num == EOF){
                    break;
                }
            }
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
    fclose(ip);
    printf("completed processing %s\n",MAP_FILENAME);
        
    //debug
    //show_list(head);
        
    //取り込んで環境マッピングに使用するppmの保存領域内を確保
    int input_ppm[ppm_height][ppm_width][3];
    //LISTを通常の配列に変換==================================
    LIST *p = head;

    //debug
    //printf("input ppm is...\n");
    //printf("head->index = %d\ttail->index = %d\n", head->index, tail->index);
        
        
    while (p->next != NULL) {
        //通常の画像viewerはヘッダを見て256*256であれば
        //それ以降の余分な数値は無視する
        int max_index = (ppm_height*ppm_width*3)-1;
        if(max_index < (p->index)){
            break;
        }
        div_t d1 = div(p->index, 3);
        div_t d2 = div(d1.quot, ppm_width);
                
        input_ppm[d2.quot][d2.rem][d1.rem] = p->num;

        /* printf("input_ppm[%d][%d][%d] = %d\n", */
        /*        d2.quot, d2.rem, d1.rem, */
        /*        input_ppm[d2.quot][d2.rem][d1.rem]); */
            
        p = p->next;
    }
    /* 環境マップppmファイルの読み込みここまで================================= */

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

        //カメラ位置の取り込み
        camera_xyz[0] = atoi(argv[3]);
        camera_xyz[1] = 0.0;
        camera_xyz[2] = 0.0;
        printf("camera is at (%f, %f, %f)\n", camera_xyz[0], camera_xyz[1], camera_xyz[2]);

        /* 全三角形の法線ベクトルを格納======================================================= */
        
        //各頂点の法線ベクトルを求める
        //三角形iの法線ベクトルを求めて配列に格納する（グローバル領域に保存）
        double poly_n[poly.idx_num * 3];
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

            //debug
            /* printf("\n三角形%dの3点の座標は\n(%f,\t%f,\t%f)\n(%f,\t%f,\t%f)\n(%f,\t%f,\t%f)\nです\n", i , */
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
            
            poly_n[i*3 + 0] = n[0];
            poly_n[i*3 + 1] = n[1];
            poly_n[i*3 + 2] = n[2];
        }
  
        //三角形iの法線ベクトルがpoly_nに格納された.
        //debug
        printf("\npoly_n\n");
        for(int i = 0 ; i < poly.idx_num ; i++){
            fprintf(stdout,"%f %f %f # %d th triangle\n",
                    poly_n[i*3+0], poly_n[i*3+1], poly_n[i*3+2],
                    i);
        }
        /* ここまで全三角形の法線ベクトルを格納======================================================= */



        
        /* 各点の平均、正規化した法線ベクトルを格納==================================================*/
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
        //点iの法線ベクトルがpoly_ave_iに格納された.
        //debug
        printf("\npoly_ave_i\n");
        for(int i = 0 ; i < poly.idx_num ; i++){
            fprintf(stdout,"%f %f %f # %d th vertex\n",
                    poly_ave_i[i*3+0], poly_ave_i[i*3+1], poly_ave_i[i*3+2],
                    i);
        }
        /* 各点の平均、正規化した法線ベクトルを格納ここまで===============================================*/
                                
        //シェーディング
        //ポリゴンiをシェーディング=======================================================================
        //============================================================================================
        
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
                
                double xp2 = xp * ((zi - camera_xyz[2]) / (zp - camera_xyz[2]));
                double yp2 = yp * ((zi - camera_xyz[2]) / (zp - camera_xyz[2]));
                double zp2 = zi;
                
                //座標軸を平行移動
                projected_ver_buf[j][0] = (WIDTH / 2) + xp2;
                projected_ver_buf[j][1] = (HEIGHT / 2) + yp2;
            }
 
            
            double a[2], b[2], c[2];
            a[0] = projected_ver_buf[0][0];
            a[1] = projected_ver_buf[0][1];
            b[0] = projected_ver_buf[1][0];
            b[1] = projected_ver_buf[1][1];
            c[0] = projected_ver_buf[2][0];
            c[1] = projected_ver_buf[2][1];

            //点a、b、cがそれぞれ何番目の頂点かを参照

            int index_a = poly.idx[i*3+0];
            int index_b = poly.idx[i*3+1];
            int index_c = poly.idx[i*3+2];
            
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

            double n_a[3], n_b[3], n_c[3];
            n_a[0] = poly_ave_i[index_a*3+0];
            n_a[1] = poly_ave_i[index_a*3+1];
            n_a[2] = poly_ave_i[index_a*3+2];
            n_b[0] = poly_ave_i[index_b*3+0];
            n_b[1] = poly_ave_i[index_b*3+1];
            n_b[2] = poly_ave_i[index_b*3+2];
            n_c[0] = poly_ave_i[index_c*3+0];
            n_c[1] = poly_ave_i[index_c*3+1];
            n_c[2] = poly_ave_i[index_c*3+2];

            //三角形iの（本来の）法線ベクトルは
            //(poly_n[i*3+0], poly_n[i*3+1], poly_n[i*3+2])
            double poly_i_n_vec[3]
                = {poly_n[i*3+0], poly_n[i*3+1], poly_n[i*3+2]};
            
            shading(a, b, c, n_a, n_b, n_c, A, B, C, poly_i_n_vec, input_ppm);
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

