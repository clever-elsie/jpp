## include/ API 概要

このディレクトリには JSON 値クラス `json::value` と内部ユーティリティが含まれます。

### 型定義
- `json::value::types`: `Null | Boolean | Number | String | Array | Object`
- `json::value::Array_t`: `std::vector<value>`
- `json::value::Object_t`: `std::unordered_map<std::string, value, internal::hash, internal::equal>`

### 構築
- `value()`, `explicit value(types)`
- 基本型: `nullptr_t`, `bool`, 数値（整数/浮動小数 → `double`）
- 文字列: `std::string`, `std::string_view`, `const char*`, `const char[N]`（末尾NULは除去）, `char8_t*`, `std::u8string`, `std::u8string_view`
- 配列: `Array_t`, `std::vector<T>`, `std::array<T,N>`, `std::initializer_list<T>`（いずれも `T` から `value` 構築可能）
- オブジェクト: `Object_t`, `std::initializer_list<pair<string,T>>`, `std::unordered_map<string,T,...>`, `std::map<string,T,...>`, `std::vector<pair<string,T>>`
- 直接: `json_t`（内部 `std::variant`）

### 情報取得とアクセス
- `types type() const` / `bool is<T>() const`
- `size_t size() const`, `std::vector<std::string> keys() const`
- インデクサ: `value& operator[](size_t)`, `value& operator[](string_like)`
- `get<T>()`, `null()`, `boolean()`, `num()`, `str()`, `array()`, `object()`

### 直列化とパース
- 文字列化: `std::string json::to_string(const value&)`
- ストリーム出力: `void json::write(std::ostream&, const value&)`, `operator<<(std::ostream&, const value&)`
- パース: `static value value::load(const char*, const char* end)`, `static value value::load(const std::string&)`, `static value value::load(std::string_view)`
  - 文字列のエスケープ: `\"`, `\\`, `\b`, `\f`, `\n`, `\r`, `\t`, `\uXXXX`（サロゲートペア対応, UTF-8 出力）

### 内部（internal.hpp 抜粋）
- `internal::token` と `token_type`
- ハッシュ・比較（透過型）: `internal::hash`, `internal::equal`
- ユーティリティ: `internal::isws`


