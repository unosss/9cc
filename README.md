# 概要
[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)の成果物です。


### src
- コンパイラのソースコード。

### subprod
- 当コンパイラ作成時に外部関数を使用して動作確認をした。その際に作成したソースコード等を配置した。

### test
- テストのソースコードおよびテストケース。
- テストケースの入力（コード）は`input.txt`, 想定出力結果は`result.txt`に配置する。
- `test.sh`は当コンパイラ作成当初に使用していたもの。


## 使用方法
rootで以下のコマンドを実行。

- `make`
  - コンパイラの実行ファイルを作成。
- `make prepare`
  - テストの実行ファイルを作成。
- `make test`
  - テストを実行。
- `make clean`
  - 不要なファイルを削除。
 
## TODO
- refactoring
- 複数テストケースの対応
- self-hosting
