## src/ ツール概要

### test_runner
- 位置: `src/test_runner.cpp`
- 目的: `test/cases/*.json` を読み込み、構文解析 → 正規化出力 → 期待値比較。

#### 使い方
```
./test_runner            # 比較のみ
./test_runner --update   # 期待値を更新（生成/上書き）
```

#### 入出力ディレクトリ
- `test/cases`: 入力 JSON（`.json`）
- `test/expected`: 期待出力（`.out`）。`--update` で自動生成/更新。
- `test/out`: 実行時に生成される出力（`.out`）

#### 処理の流れ
1. `test/cases` を列挙し、`.json` を対象にする
2. `json::value::load` でパース
3. `json::to_string` で正規化文字列を生成
4. `test/out` に保存し、`test/expected` と比較
5. 差分があれば失敗。`--update` 指定時は期待値に反映


