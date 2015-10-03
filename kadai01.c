#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//2点p、qを結ぶ直線上のy座標がyであるような点のx座標を返す関数
//eg)
//double p[2] = (1.0, 2.0);
double func1(double *p, *q, y){
    double x;
    if(p[1] == q[1]){
        //解なし
        perror("三角形の分割ができない。");
    }
    double r[2];
    else{
        if(p[1] > q[1]){
            x = ((p[0] * (y - q[1])) + (q[0] * (p[1] - y))) / (p[1] - q[1]);
        }
        if(p[1] < q[1]){
            x = ((q[0] * (y - p[1])) + (p[0] * (q[1] - y))) / (q[1] - p[1]);
        }
    }
    return x;
}

//頂点座標の配列verに透視投影を行う関数
void perspective_pro(double ver[][3], int ver_num){
    for(int i = 0; i < ver_num; i++){
        double xp = ver[i][0];
        double yp = ver[i][1];
        double zp = ver[i][2];
        double zi = 256.0;
        
        double xp2 = xp * (zi / zp);
        double yp2 = yp * (zi / zp);
        double zp2 = zi;
        
        ver[i][0] = xp2;
        ver[i][1] = yp2;
        ver[i][2] = zp2;
    }
}

//投影された三角形abcにラスタライズ、クリッピングでシェーディングを行う関数
//引数は投影平面上の3点
//eg)
//double a = {1.0, 2.0};
void shading(double *a, double *b, double *c, double image[][][3]){
    //3点が1直線上に並んでいるときはシェーディングができない
    if(){
    }
    else{
        //三角形を分割
        //y座標の値が真ん中点をp、その他の点をq、rとする
        //y座標の大きさはq >= p >= rの順
        double p[2],q[2],r[2];
        if(b[1] <= a[1] && a[1] <= c[1]){
            p = a;
            q = c;
            r = b;
        }
        else{
            if(c[1] <= a[1] && a[1] <= b[1]){
                p = a;
                q = b;
                r = c;
            }
            else{
                if(a[1] <= b[1] && b[1] <= c[1]){
                    p = b;
                    q = c;
                    r = a;
                }
                else{
                    if(c[1] <= b[1] && b[1] <= a[1]){
                        p = b;
                        q = a;
                        r = c;
                    }
                    else{
                        if(b[1] <= c[1] && c[1] <= a[1]){
                            p = c;
                            q = a;
                            r = b;
                        }
                        else{
                            if(a[1] <= c[1] && c[1] <= b[1]){
                                p = c;
                                q = b;
                                r = a;
                            }
                            else{perror(NULL);}
                        }
                    }
                }
            }
        }
        //三角形を分割
        //分割後の三角形はpp2qとpp2r
        double p2[2];
        
        p2[0] = func1(q, r, p[1]);
        p2[1] = p[1];

        //p2のほうがpのx座標より大きくなるようにする
        if(p2[0] < p[0]){
            double temp[2];
            temp = p2;
            p2 = p;
            p = temp; 
        }
        
        //三角形pp2qをシェーディング
        int i;
        if(0 <= p[1]){
            i = floor(p[1]);
        }
        else{
            i = floor(p[1] + 1);
        }

        for(i; p[1] <= i && i <= q[1]; i++;){
            double x1 = func1(p, q, i);
            double x2 = func1(p2, q, i);
            int j;
            
            if(0 <= x1){
                j = floor(x1);
            }
            else{
                j = floor(x1 + 1)
            }
            for(j; p[0] <= j && j <= p2[0]; j++){
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
        //=====================================================================
        //ポリゴンデータ
        //av1.wrl
        //point
        double ver[5][3] = {
            {0, 0, 400},
            {-200, 0, 500},
            {0, 150, 500},
            {200, 0, 500},
            {0, -150, 500}
        };
        //coordIndex
        int sur[4][3] = {
            {0, 1, 2},
            {0, 2, 3},
            {0, 3, 4},
            {0, 4, 1}
        };
        //diffuseColor
        duble diffuse_color[3] = {1.0, 1.0, 0.0};
        //投影する頂点の数
        const int ver_num = 5;
        //投影する三角形の数
        const int sur_num = 4;
        //=====================================================================
  
        //メモリ内に画像の描画領域を確保
        duble image[256][256][3];
        //初期化
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
        perspective_pro(ver, ver_num);
        //投影後のverから冗長なz座標を消去したx_y_verを作る
        double x_y_ver[ver_num][2];
        for(int i = 0; i < ver_num; i++){
            x_y_ver[i][0] = ver[i][0];
            x_y_ver[i][1] = ver[i][1];
        }

        //シェーディング
        for(int i = 0; i < sur_num; i++){
            doubel a[2], b[2], c[2];
            a[0] = x_y_ver[(sur[i][0])][0];
            a[1] = x_y_ver[(sur[i][0])][1];
            
            b[0] = x_y_ver[(sur[i][1])][0];
            b[1] = x_y_ver[(sur[i][1])][1];
            
            c[0] = x_y_ver[(sur[i][2])][0];
            c[1] = x_y_ver[(sur[i][2])][1];
            
            shading(a, b, c, image);
        }

        //imageの出力
        //初期化
        for(int i = 0; i < 256; i++){
            for(int j = 0; j < 256; j++){
                for(int k = 0; k < 3; k++){
                    char ver_x[256];
                    char ver_y[256];
                    char ver_z[256];
                    char str[1024];
                    
                    sprintf(ver_x, "%lf", image[i][j][0]);
                    sprintf(ver_y, "%lf", image[i][j][1]);
                    sprintf(ver_z, "%lf", image[i][j][2]);
                    sprintf(str, "%s %s %s\n", ver_x, ver_y, ver_z);
                    fputs(str, fp); 
                }
            }
        }
        

      
    }
    fclose(fp);

    printf("ppmファイル %s の作成が完了しました.\n", fname );
    return 0;
}



