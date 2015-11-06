#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <ctype.h>
#include "vrml.h"



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
#define Z_BUF_MAX 

//diffuseColorを格納する配列
double diffuse_color[3];

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

//投影された三角形abcにラスタライズ、クリッピングでシェーディングを行う関数
//引数a, b, cは投影平面上の3点
//eg)
//double a = {1.0, 2.0};
//nは法線ベクトル
//Aは投影前の3点からなる三角形平面上の任意の点の座標.
//(3点A、B、Cのうちいずれでも良いがmain関数内のAを使うものとする.)
void shading(double *a, double *b, double *c, double *n, double *A){
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

            if(0 <= ip){
                ip = 0;
            }
                            
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
                               
                               //描画する点の空間内のz座標. 
                               double z =
                                   FOCUS * ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                   /
                                   ((n[0]*(j-(WIDTH/2))) + (n[1]*(i-(HEIGHT/2))) + n[2]*FOCUS);
                               
                               //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                               if(z_buf[i][j] < z){}
                               
                               else{
                                   image[i][j][0] =
                                       -1 * ip * diffuse_color[0] *
                                       light_rgb[0] * MAX;                                 
                                   image[i][j][1] =
                                       -1 * ip * diffuse_color[1] *
                                       light_rgb[1] * MAX;
                                   image[i][j][2] =
                                       -1 * ip * diffuse_color[2] *
                                       light_rgb[2] * MAX;
                                   
                                   //zバッファの更新
                                   z_buf[i][j] = z;
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
                        
                        for(j;
                            x1 <= j && j <= x2 && 0 <= j && j <= (WIDTH - 1);
                            j++){
                            
                            //描画する点の空間内のz座標. 
                            double z =
                                FOCUS * ((n[0]*A[0]) + (n[1]*A[1]) + (n[2]*A[2]))
                                /
                                ((n[0]*(j-(WIDTH/2))) + (n[1]*(i-(HEIGHT/2))) + n[2]*FOCUS);
                            
                            //zがzバッファの該当する値より大きければ描画を行わない（何もしない）
                            if(z_buf[i][j] < z){}
                        
                            else{                   
                                image[i][j][0] =
                                    -1 * ip * diffuse_color[0] *
                                    light_rgb[0] * MAX;
                                image[i][j][1] =
                                    -1 * ip * diffuse_color[1] *
                                    light_rgb[1] * MAX;
                                image[i][j][2] =
                                    -1 * ip * diffuse_color[2] *
                                    light_rgb[2] * MAX;

                                //zバッファの更新
                                z_buf[i][j] = z;
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
            //分割して処理
            //分割しても同一平面上なので法線ベクトルと
            //平面上の任意の点は同じものを使える.
            shading(p, p2, q, n, A);
            shading(p, p2, r, n, A);
        }
    }
}

/* VRMLの読み込み */
/* ======================================================================= */
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
/* ======================================================================= */

int main (int argc, char *argv[]){
    /* VRML読み込み========================================================= */
    int i;
    FILE *fp;
    Polygon poly;
    Surface surface;

    fp = fopen(argv[1], "r");
    read_one_obj(fp, &poly, &surface);

    printf("\npoly.vtx_num\n");
    fprintf(stderr,"%d vertice are found.\n",poly.vtx_num);
    printf("\npoly.idx_num\n");
    fprintf(stderr,"%d triangles are found.\n",poly.idx_num);

    /* i th vertex */
    printf("\npoly.vtx[i*3+0, 2, 3]\n");
    for ( i = 0 ; i < poly.vtx_num ; i++ ) {
        fprintf(stdout,"%f %f %f # %d th vertex\n", 
                poly.vtx[i*3+0], poly.vtx[i*3+1], poly.vtx[i*3+2],
                i);
    }

    /* i th triangle */
    printf("\npoly.idx[i*3+0, 2, 3]\n");
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
    /* VRML読み込みここまで========================================================= */

    FILE *fp_ppm;
    char *fname = FILENAME;
    fp_ppm = fopen(argv[2], "w" );
    
    //ファイルが開けなかったとき
    if( fp_ppm == NULL ){
        printf("%sファイルが開けません.\n", fname);
        return -1;
    }
  
    //ファイルが開けたとき
    else{
        fprintf(stderr, "\n初期の頂点座標は以下\n");
        for(int i = 0; i < poly.vtx_num; i++){
            fprintf(stderr, "%f\t%f\t%f\n", poly.vtx[i*3+0], poly.vtx[i*3+1], poly.vtx[i*3+2]);
        }
        fprintf(stderr, "\n");
        
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

        //シェーディング
        //三角形ごとのループ
        for(int i = 0; i < poly.idx_num; i++){
            //各点の透視投影処理
            for(int j = 0; j < 3; j++){ 
                double xp = poly.vtx[(poly.idx[i*3+j])*3 + 0];
                double yp = poly.vtx[(poly.idx[i*3+j])*3 + 1];
                double zp = poly.vtx[(poly.idx[i*3+j])*3 + 2];
                double zi = FOCUS;

                //debug 
                if(zp == 0){
                    printf("\n(%f\t%f\t%f) i=%d, j=%d\n", xp, yp, zp, i, j);
                    perror("\nエラー0934\n");
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
            
            //平面iの投影先の三角形をシェーディング
            shading(a, b, c, n, A);
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
    
    printf("\nppmファイル %s に画像を出力しました.\n", argv[2]);
    return 1;
}

