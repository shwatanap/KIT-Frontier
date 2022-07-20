# KIT-Frontier
TdZddを用いてs-t問題を解く

## 実行方法
### s-tパス問題
make NUM=[n] </br>
(./path grid/grid$(NUM)x$(NUM))

解の数のみを出力する場合は`solutionOutput`をコメントアウトしてください


### ハミルトン閉路問題
make exech NUM=[n] </br>
(./hamilton grid/grid$(NUM)x$(NUM))

解の数のみを出力する場合は`solutionOutput`をコメントアウトしてください


### ZDDの描画
make dot(doth) NUM=4 </br>
(./path(hamilton) grid/grid$(NUM)x$(NUM).grh > output.dot && dot -Tpng output.dot -o output.png)

ZDDを描画する際は`solutionOutput`と`解の出力`をコメントアウトしてください
