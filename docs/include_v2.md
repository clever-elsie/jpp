## include/ API 概要 (Version 2)

このディレクトリには、例外処理を排除し、`std::expected` による安全なエラーハンドリングやアサートによる契約プログラミングに対応した JSON 値クラス `json::value`、オブジェクト用の `json::object_t`、およびユーティリティが含まれます。

### 前提条件 (Prerequisites)
- **C++ 標準規格**: **C++23 以上**が必要です。
  - C++23 で導入された `std::expected` などの標準ライブラリ機能を活用しています。
- **コンパイラ**: C++23 をサポートする主要コンパイラ (GCC 13+, Clang 16+ など) が必要です。
- **標準ライブラリ制限**: オブジェクトのキー挿入順序を高速に保持・走査するため、GCC の拡張データ構造である Policy-Based Data Structures (`pb_ds` / `<ext/pb_ds/assoc_container.hpp>`) を使用しています。このため、標準ライブラリとして **GNU `libstdc++` が必須**です。
  - ※ Clang 等でビルドする場合も、標準ライブラリには `libc++` ではなく GNU `libstdc++` を指定する必要があります（例: `-stdlib=libstdc++`）。

### ファイル構成
- **json_v2.hpp** — メインエントリ。`ELSIE_JSON_LIBRARY_VERSION 2` を定義し、必要なヘッダ群をインクルードします。
- **json_v2/image.hpp** — `value` クラス本体、および参照ラッパー `ref_t` の定義。
- **json_v2/image_array.hpp** / **image_object.hpp** — 配列（`array_t`）およびオブジェクト（`object_t`）のデータ構造。
- **json_v2/parse.hpp** — 例外をスローせず `std::expected` を返す JSON パーサの実装。
- **json_v2/generate.hpp** — JSON 直列化（シリアライザ）の実装。

---

### 型定義
- `json::types`: `Null | Bool | Int | Uint | Float | Str | Array | Object`
- `json::value::null_t`: `std::nullptr_t`
- `json::value::bool_t`: `bool`
- `json::value::int_t`: `int64_t`
- `json::value::uint_t`: `uint64_t`
- `json::value::float_t`: `double`
- `json::value::str_t`: `std::string`
- `json::value::array_t`: `data_structure::array_t<value>` (キーの挿入順序を保持する配列)
- `json::value::object_t`: `data_structure::object_map_t<value>` (キーの挿入順序を保持するオブジェクト)
- `json::value::json_t`: 内部の `std::variant<null_t, bool_t, int_t, uint_t, float_t, str_t, array_t, object_t>`

---

### 構築
- `value()`, `explicit value(types)`
- **基本型からの構築**: `nullptr_t`, `bool`, 各種数値型（整数 / 浮動小数点数）
- **文字列からの構築**: `std::string`, `std::string_view`, `const char*`, `const char[N]` (末尾 NUL は除去), `const char8_t*`, `std::u8string`, `std::u8string_view`
- **配列からの構築**: `array_t`, `std::initializer_list<T>`, `std::vector<T>`, `std::array<T, N>` (要素 `T` が `value` に変換可能な場合)
- **オブジェクトからの構築**: `object_t`, `std::initializer_list<std::pair<std::string, T>>`, `std::unordered_map<std::string, T>`, `std::map<std::string, T>`, `std::vector<std::pair<std::string, T>>`

---

### 情報取得と検証
- `types type() const noexcept` — 現在保持している JSON の型タイプを返します。
- `bool is(types t) const noexcept` — 指定された JSON の型タイプと一致するか検証します。
- `template<types T> bool is() const noexcept` — テンプレートパラメータで指定された JSON の型タイプと一致するか検証します。
- `template<class T> bool is() const noexcept` — テンプレートパラメータで指定された C++ の内部表現型（`str_t` 等）と一致するか検証します。
- `size_t size() const noexcept` — 配列・オブジェクトの場合は要素数、それ以外は `1` を返します。
- `std::expected<std::vector<std::string>, std::domain_error> keys() const` — オブジェクトのキーの一覧を挿入順で取得します。オブジェクトでない場合はエラーを返します。

---

### アクセス

#### 1. インデクサ (`operator[]`)
型安全性をバイパスする高速なアクセス手段です。
- `value& operator[](size_t index)` (const 版あり)
- `value& operator[](KeyType&& key)` (const 版あり)
  - ※ `KeyType` には `std::string`, `std::string_view`, `const char*` のコピーまたはムーブが指定可能です。
  - **昇格ルール**: 現在の値が `null_t` の場合、インデクサの呼び出し（添字が数値か文字列キーか）に応じて、自動的に空の配列 (`array_t`) または空のオブジェクト (`object_t`) へ型が自動昇格します。
  - **自動挿入ルール**: 非constのインデクサで、指定された要素やキーが存在しない場合は、自動的に新規追加・リサイズされてその要素の参照を返します。
  - **制約**: すでに型が決定しており、それが配列（またはオブジェクト）ではない状態で対応するインデクサを呼び出すと、アサート（`assert`）で失敗します。

#### 2. 安全な範囲アクセス (`at`)
範囲外アクセスや型不一致によるエラーを、例外を投げずに `std::expected` で安全に処理します。
- `std::expected<value&, std::out_of_range> at(size_t index)` (const 版あり)
- `std::expected<value&, std::out_of_range> at(KeyType&& key)` (const 版あり)
  - ※ `KeyType` には `std::string`, `std::string_view`, `const char*` のコピーまたはムーブが指定可能です。
  - ※ 戻り値の参照ラッパーについての詳細は、後述の [注記] を参照してください。

---

### ゲッター (型ごとの値抽出)

安全なエラーハンドリングを行う小文字の「安全アクセス」と、型前提が保証されている状況で高速に呼び出せる大文字の「契約アクセス」の 2 系統が提供されます。

#### 1. 安全アクセス (小文字ゲッター, `std::expected` 返却)
型が異なる場合は例外をスローせず、`std::unexpected` にエラー情報 (`std::domain_error`) を包んで返します。

| メソッド | 戻り値の型 (非const / const) | エラー条件 |
| :--- | :--- | :--- |
| `null()` | `std::expected<std::nullptr_t, std::domain_error>` | `Null` 型以外の場合 |
| `boolean()` | `std::expected<bool, std::domain_error>` | `Bool` 型以外の場合 |
| `sint()` | `std::expected<int64_t, std::domain_error>` | `Int` 型以外の場合 |
| `uint()` | `std::expected<uint64_t, std::domain_error>` | `Uint` 型以外の場合 |
| `fp()` | `std::expected<double, std::domain_error>` | `Float` 型以外の場合 |
| `str()` | `std::expected<std::string&, std::domain_error>` / `...<const std::string&, ...>` | `Str` 型以外の場合 |
| `array()` | `std::expected<array_t&, std::domain_error>` / `...<const array_t&, ...>` | `Array` 型以外の場合 |
| `object()` | `std::expected<object_t&, std::domain_error>` / `...<const object_t&, ...>` | `Object` 型以外の場合 |
| `get<T>()` | `std::expected<T&, std::domain_error>` / `...<const T&, ...>` | 内部型 `T` と不一致の場合 |

> [!NOTE]
> **戻り値の参照型について (`ref_t<T>` ラッパー)**
>
> 戻り値の型として `T&` と記載されている部分（`str()`, `array()`, `object()`, `at()`, `get()` の戻り値など）は、ライブラリ内部では `json::ref_t<T>` を返します。
> 
> これは C++ の標準仕様において、`std::expected` や `std::optional` が生の参照型（`T&`）を直接要素として保持できないという言語制約を解決するために実装されたカスタム参照ラッパーです。
> `ref_t<T>` は、暗黙 of 型変換演算子（`operator T&()`）、参照外し演算子（`operator*()`）、およびポインタアクセス演算子（`operator->()`）をオーバーロードしているため、**通常の C++ の参照（`T&`）とまったく同等に透過的に使用することができます**。

#### 2. 契約アクセス (大文字ゲッター, `assert` による前提検証)
値の型が一致していることをアサート（`assert`）で事前検証します。型が一致していれば直接値（または生の参照）を返し、例外スローやラッパー包みを行わないため、最も高速にアクセスできます。

- `std::nullptr_t Null() const`
- `bool Boolean() const`
- `int64_t Sint() const`
- `uint64_t Uint() const`
- `double Fp() const`
- `std::string& Str()` (const 版は `const std::string&`)
- `array_t& Array()` (const 版は `const array_t&`)
- `object_t& Object()` (const 版は `const object_t&`)

---

### 変更・代入
- `clear() noexcept` — 内部データを `null` にクリアします。
- `swap(value& other) noexcept` — 別の `value` オブジェクトとデータを高速に入れ替えます。
- `operator=(const value&)`, `operator=(value&&)` — コピー・ムーブ代入。
- `template<class T> value& operator=(T&& v)` — `json_t` に代入可能な値 `v` を直接代入します。

---

### 直列化とパース

#### 1. 直列化 (シリアライズ)
- `std::string json::to_string(const value& v)` — JSON 値をコンパクトな文字列にシリアライズします。
- `std::string json::to_readable(const value& v, int indent_size = 2)` — インデントを付けた人間が読みやすい形式でシリアライズします。
- `void json::write(std::ostream& os, const value& v)` — ストリームに JSON 文字列を直接書き込みます。
- `std::ostream& operator<<(std::ostream& os, const value& v)` — ストリーム出力演算子。

#### 2. パース (デシリアライズ)
例外をスローせず、パースエラーの情報を `std::expected` で返します。
- `static std::expected<value, std::runtime_error> value::load(const char* begin, const char* const end)`
- `static std::expected<value, std::runtime_error> value::load(const std::string& s)`
- `static std::expected<value, std::runtime_error> value::load(const std::string_view& s)`
  - ※ パースに失敗した場合は `std::unexpected(std::runtime_error("エラー内容"))` が返されます。
  - ※ エスケープシーケンス（`\"`, `\\`, `\b`, `\f`, `\n`, `\r`, `\t`）および `\uXXXX`（サロゲートペア対応、UTF-8 出力）を正しくハンドリングします。
