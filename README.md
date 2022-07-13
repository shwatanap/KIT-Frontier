# KIT-Frontier
TdZddを用いてs-t問題を解く

## How to exec
### s-tパス問題
make NUM=[n] </br>
(./path grid/grid$(NUM)x$(NUM))

### ZDDの描画
make dot NUM=4 </br>
(./path grid/grid$(NUM)x$(NUM).grh > output.dot && dot -Tpng output.dot -o output.png)
