# jpp

軽量な C++ JSON 値クラスと簡易パーサ/シリアライザ、およびテストツール。

## 特徴
- `json::value` による直感的な JSON 値表現（`Null/Boolean/Number/String/Array/Object`）
- 文字列化/パース対応（`\uXXXX` サロゲートペア → UTF-8 変換）
- 各種コンテナ・文字列型からの容易な構築（`u8` 系含む）
- テストランナーで入出力を自動検証

## ドキュメント
- include API: `docs/include.md`
- ツール概要: `docs/src.md`

## テストの実行
1. テストケースを `test/cases/*.json` に配置
2. ビルドして実行
```
g++ -std=c++20 -O2 -Iinclude src/test_runner.cpp -o test_runner
./test_runner           # 比較
./test_runner --update  # 期待値更新
```

# JSON parser
https://tex2e.github.io/rfc-translater/html/rfc8259.html