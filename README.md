# jpp

軽量な C++ JSON 値クラスと簡易パーサ/シリアライザ、およびテストツール。

## 特徴
- `json::value` による直感的な JSON 値表現（`Null/Boolean/Number/String/Array/Object`）
- オブジェクトは `json::object_t` でキーの挿入順序を保持し、直列化時もその順で出力
- 文字列化/パース対応（`\uXXXX` サロゲートペア → UTF-8 変換）、`to_readable()` でインデント付き出力
- 各種コンテナ・文字列型からの容易な構築（`u8` 系含む）
- テストランナーで入出力を自動検証

## ドキュメント
- include API・ファイル構成: `docs/include.md`
- ツール概要: `docs/src.md`

## テストの実行
1. テストケースを `test/cases/*.json` に配置．期待する出力を `test/expected/*.out` に配置．
2. ビルドして実行
```
make test
```

## JSON specification
https://tex2e.github.io/rfc-translater/html/rfc8259.html