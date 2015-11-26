# jikkenCG  
グローバル変数使いまくります。。。。orz

## 主なファイル構成
### EvalKadai*.sh
EvalKadai01.sh等のファイルはそれぞれ対応するファイルkadai*.cを実行するファイル.

### imagesディレクトリ
作成した画像etc.

### InputPpm.c
PPMファイルを読み込むcファイル.  
ただし、対応するPPMファイルに制限あり.
（コメントアウト非対応等）

### Kadai*.c
VRML形式のデータを受け取ってPPMファイルに描画するプログラム群.
なお、使用上kadai01.cについては内部で定数として与えたVRMLデータで
レンダリングを行う.
また、kadai04-2.cはkadai04.cにカメラ位置の変更
要素を加えた試作ファイル.

### vrml.h, vrml.c
VRMLの読み込みルーチン.  
(@author 飯山先生)

### sampleディレクトリ
VRML形式のファイル（拡張子.wrl）などの入っているディレクトリ.
