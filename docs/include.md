## include/ API 概要 (Version 1)

このディレクトリには JSON 値クラス `json::value`、オブジェクト用の `json::object_t`、および内部ユーティリティが含まれます。

### 前提条件 (Prerequisites)
- **C++ 標準規格**: **C++20 以上**が必要です。
  - コンセプト (`std::floating_point` などの `requires` 句) が随所で使用されています。
- **コンパイラ**: C++20 を十分にサポートするコンパイラ (GCC 10+, Clang 10+, MSVC 19.29+ など) が必要です。

### ファイル構成
- **json.hpp** — メインエントリ。`value` クラス、直列化・パース API。末尾で `json/object_t_impl.hpp` をインクルード。
- **json/internal.hpp** — トークン型、ハッシュ・比較、空白判定などパーサ用の内部定義。
- **json/object_t.hpp** — 挿入順序を保持する Object 型 `object_t` の宣言。
- **json/object_t_impl.hpp** — `object_t` のうち `value` の完全定義に依存するメンバの実装。

### 型定義
- `json::value::types`: `Null | Boolean | Number | String | Array | Object`
- `json::value::Array_t`: `std::vector<value>`
- `json::value::Object_t`: `json::object_t`（内部は `internal::umap<value>` 相当で、キーの挿入順序を保持）
- `json::value::json_t`: 内部の `std::variant<nullptr_t, bool, double, std::string, Array_t, Object_t>`

### 構築
- `value()`, `explicit value(types)`
- 基本型: `nullptr_t`, `bool`, 数値（整数/浮動小数 → `double`）
- 文字列: `std::string`, `std::string_view`, `const char*`, `const char[N]`（末尾 NUL は除去）, `char8_t*`, `std::u8string`, `std::u8string_view`
- 配列: `Array_t`, `std::vector<T>`, `std::array<T,N>`, `std::initializer_list<T>`（いずれも `T` から `value` 構築可能）
- オブジェクト: `Object_t`, `std::initializer_list<pair<string,T>>`, `std::unordered_map<string,T,...>`, `std::map<string,T,...>`, `std::vector<pair<string,T>>`
- 直接: `json_t`（内部 `std::variant`）

### 情報取得とアクセス
- `types type() const` / `bool is<T>() const` / `bool is(types t) const`
- `size_t size() const`, `std::vector<std::string> keys() const`
- インデクサ:
  - `value& operator[](size_t index)` （const 版あり）
  - キー指定: `value& operator[](KeyType&& key)` （const 版あり）
    - ※ライブラリ内部ではテンプレートで実装されていますが、ユーザー側からは `std::string`, `std::string_view`, `const char*` の3つの型に対するコピーおよびムーブのオーバーロードが存在するものとみなして問題ありません。
- `get<T>()`
- `null()`, `boolean()`, `integer()`（`int64_t`）, `num()` / `fp()`, `str()`, `array()`, `object()`
  - ※型が不一致の場合には `std::runtime_error` 例外がスローされます。

### 変更・代入
- `clear()`, `swap(value&)`
- `operator=(const value&)`, `operator=(value&&)`, `operator=(T&&)`（`json_t` に代入可能な `T`）

### 直列化とパース
- 文字列化: `std::string json::to_string(const value&)`
- 読みやすい改行付き: `std::string json::to_readable(const value&, int indent_size = 2)`
- ストリーム出力: `void json::write(std::ostream&, const value&)`, `operator<<(std::ostream&, const value&)`
- パース: `static value value::load(const char*, const char* end)`, `static value value::load(const std::string&)`, `static value value::load(std::string_view)`
  - 文字列のエスケープ: `\"`, `\\`, `\b`, `\f`, `\n`, `\r`, `\t`, `\uXXXX`（サロゲートペア対応, UTF-8 出力）

### object_t の補足
- キーの挿入順序を保持し、`keys_in_order()` でその順のキーリストを取得可能。
- 直列化（`write` / `to_string` / `to_readable`）ではこの挿入順でメンバが出力される。
- キー指定には `std::string`, `std::string_view`, `const char*` （コピー/ムーブ）が使用可能です。

### 内部（internal.hpp 抜粋）
- `internal::token` と `internal::token_type`
- ハッシュ・比較（透過型）: `internal::hash`, `internal::equal`
- マップ型: `internal::umap<T>` ≡ `std::unordered_map<std::string, T, hash, equal>`
- ユーティリティ: `internal::isws(char)`
