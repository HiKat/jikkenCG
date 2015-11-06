# jikkenCG

レンダリング本体の関数のshadingを別のモジュールにする必要が
全く無かった.  
↑のせいでmainグローバル変数を乱用する羽目になった.  

これからは先に無理にモジュール化をしようとして変なところで  
ぶった切るより、先ずはmain関数内で処理する形で書いて  
必要に応じてモジュール化を考えるようにしたい  

シェーディング時のループのi, jはそれぞれ
もとのxyz空間内の座標には

```c

x = j-(MAX/2)
y = i-(MAX/2)

```

として戻すことに留意.  

発展課題でカメラ位置を変化させるのは避けたほうがいいかも.  
視点方向ベクトルなどわざわざカメラ位置(0, 0, 0)を引く計算を明示化していないので  
どこでバグが起こるかわからない  


各ファイルシェーディングで点の塗りつぶしのループで
jの値が撮像平面からはみ出している時の対処はされているが
iの値が撮像平面からはみ出している場合の対処がなされていない？？    


課題2から拡散反射の際のベクトルiの正負が逆になっている可能性  


ppmの読み込みルーチンについて  
読み込むppmは  
最初のヘッダ部分は全て改行で区切らないとエラーが出る  
コメントは非対応  



0 == 1/2のミスに気付かずかなり戸惑った

課題５の疑似コード  
```

int main(int argc, char *argv[]){
  ppm読み込み;
  ファイルオープン;

  double poly_n[poly.idx_num * 3];

  全ての三角形の法線ベクトルを求めてpoly_nに格納;

  double poly_ave_i[poly.vtx_num];

  全ての頂点の法線ベクトルを求めてpoly_ave_iに格納;
  //頂点iの法線ベクトルは(poly_ave_i[i*3+0], poly_ave_i[i*3+1], poly_ave_i[i*3+2])となる.


  //三角形iのシェーディング;
  for(int i = 0; i < poly.idx_num; i++){
    //3点a、b、cはそれぞれ頂点番号poly.idx[i*3+0]、poly.idx[i*3+0]、poly.idx[i*3+0]




    double n_a[3], n_b[3], n_c[3];
    3点の法線ベクトルをn_a、n_b、n_cに格納;

    double poly_i_n_vec[3];
    三角形iの法線ベクトルをpoly_i_n_vecに格納;

    shading(a, b, c, n_a, n_b, n_c, A, B, C);

  }
  ファイルクローズ;
  出力;
}


void shading(double *a, double *b, double *c, double *n_a, double *n_b, double *n_c, double *A, double *B, double *C){

  //パターン1のシェーディング
  for(点(j, i)をシェーディング){
    double p_or[3];
    点(j, i)の空間座標をp_orに格納;

    double n_ji[3];
    p_orを用いて計算した点(j, i)での法線ベクトルをn_jiに格納;

    double u[3];
    p_orを用いて計算した視線ベクトルをuに格納;

    double f[3];
    n_jiとuを用いて反射ベクトルをfに格納;

    int s_x, t_y;
    s_x、t_yの値をfから求めて格納;

    int env_r, env_g, env_b;
    環境マップの値を取り出す;

    zバッファ判定;
    imageを決定;

  }

}





```
