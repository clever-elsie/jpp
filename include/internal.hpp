#pragma once
#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>
#include <unordered_map>

namespace json{ namespace internal{

template<class S, class T>
concept pure_fit = std::same_as<std::decay_t<S>,std::decay_t<T>>;

template<class T>
concept is_string_like = pure_fit<std::string_view,T> || pure_fit<std::string,T> || pure_fit<const char*,T>;

struct hash{
  using is_transparent = void;
  template<is_string_like T>
  constexpr static size_t operator()(T&&v)noexcept{ return std::hash<std::decay_t<T>>{}(v); }
};

struct equal{
  using is_transparent = void;
  template<is_string_like S, is_string_like T>
  constexpr static bool operator()(S&&v1, T&&v2)noexcept{ return v1 == v2; }
};

template<class T>
using umap=std::unordered_map<std::string,T,hash,equal>;

enum class token_type{
  begin_array, begin_object, end_array, end_object,
  name_separator, value_separator, String, Number, Boolean, Null, EOF_token
};

struct token{
  token_type type;
  std::string word;
  token():type(token_type::EOF_token),word(""){}
  token(token_type type, const std::string&word):type(type),word(word){}
  token(token_type type, std::string&&word):type(type),word(std::move(word)){}
};

constexpr inline bool isws(char c){
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}


} // namespace internal
} // namespace json